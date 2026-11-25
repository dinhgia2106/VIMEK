// VIMEK model-suggestion tests. GPL-3.0, see LICENSE.
#include "Suggest.h"
#include <iostream>

static int failures = 0;
static void check(bool ok, const char* what) {
    if (!ok) {
        ++failures;
        std::cerr << "FAIL " << what << '\n';
    }
}

static void type(SuggestRun& run, const std::wstring& text) {
    for (wchar_t c : text) run.append(c);
}

int main() {
    SuggestRun run;
    type(run, L"toi di hoc ve nha an com");
    check(run.endsInWord(), "ends in word");
    check(run.window() == L"com", "window is only the word being typed");

    // Space applies the first suggestion to that word; earlier words are never touched.
    std::wstring request = run.requestText();
    std::vector<std::wstring> cands = {L"tôi đi học về nhà ăn cơm", L"tôi đi học về nhà ăn côm", L"tôi đi học về nhà an cơm", L"short"};
    auto wins = SuggestRun::windows(request, run.windowStart(), cands, 5);
    check(wins.size() == 2 && wins[0] == L"cơm" && wins[1] == L"côm", "distinct windows, wrong lengths skipped");
    size_t bs = 0;
    std::wstring ins;
    check(run.replaceWindow(wins[0], bs, ins), "replace window");
    check(bs == 2 && ins == L"ơm", "only the changed tail is retyped");
    check(run.screen() == L"toi di hoc ve nha an cơm", "screen after replacement");

    // After a space the next word starts its own window.
    run.append(L' ');
    check(!run.endsInWord(), "space ends the word");
    type(run, L"ngon");
    check(run.window() == L"ngon" && run.requestText() == L"toi di hoc ve nha an cơm ngon", "next word");

    // A word the user marked with Telex gets no suggestion.
    run.reset();
    type(run, L"tiếng");
    check(run.windowTypedWithMarks(), "Telex-marked word");
    type(run, L" viet");
    check(!run.windowTypedWithMarks() && run.window() == L"viet", "plain word after a marked one");

    // A choice locks everything typed so far.
    run.lockAll();
    check(run.window().empty() && !run.endsInWord(), "locked run has an empty window");
    type(run, L" nhe");
    check(run.window() == L"nhe" && run.requestText() == L"tiếng viet nhe", "after lock");

    // Backspace past the start of the run means the screen is unknown.
    run.erase(2);
    check(run.screen() == L"tiếng viet n", "erase");
    run.erase(1000);
    check(run.empty(), "erasing more than the run resets it");

    // Long runs keep the newest characters.
    for (int i = 0; i < 100; ++i) type(run, L"ab ");
    check(run.screen().size() == SuggestRun::MaxChars, "run is bounded");

    // JSON round trip.
    std::string json = suggestRequestJson(L"hom \"nay\"\\", 5);
    check(json == "{\"text\":\"hom \\\"nay\\\"\\\\\",\"k\":5}", "request escaping");
    std::vector<std::wstring> parsed;
    check(suggestParseCandidates("{\"candidates\": [\"h\xc3\xb4m nay\", \"h\\u00f4m\\\"\"], \"ms\": 3.1}", parsed),
          "parse reply");
    check(parsed.size() == 2 && parsed[0] == L"hôm nay" && parsed[1] == L"hôm\"", "parsed values");
    check(suggestParseCandidates("{\"candidates\":[]}", parsed) && parsed.empty(), "empty list");
    check(!suggestParseCandidates("{\"detail\":\"no\"}", parsed), "error reply");

    std::cout << "suggest tests; " << failures << " failures\n";
    return failures ? 1 : 0;
}
