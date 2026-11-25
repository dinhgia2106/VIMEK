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

// Type text as the user would: each character is a key and appears on screen unchanged.
static void type(SuggestRun& run, const std::wstring& text) {
    for (wchar_t c : text) {
        run.key(c);
        run.append(c);
    }
}

int main() {
    SuggestRun run;
    type(run, L"Hôm qua toi ");
    type(run, L"homnaytroi");
    check(run.endsInWord(), "ends in word");
    check(run.window() == L"homnaytroi", "the chunk is everything since the last space");
    check(run.context() == L"Hôm qua toi ", "context is the text before the chunk");
    check(run.raw() == L"homnaytroi", "keys of the chunk");

    // The chosen candidate is longer than the chunk: spaces are added.
    size_t bs = 0;
    std::wstring ins;
    run.replaceWindow(L"hôm nay trời", bs, ins);
    check(bs == 9 && ins == L"ôm nay trời", "only the changed tail is retyped");
    check(run.screen() == L"Hôm qua toi hôm nay trời" && run.raw().empty(), "screen after replacement");
    check(run.window() == L"trời", "after replacement the chunk is the last word on screen");

    // A space ends the chunk; the next one starts empty.
    type(run, L" ");
    check(!run.endsInWord() && run.raw().empty(), "space ends the chunk");

    // The Telex engine shows marks the keys do not: the keys are kept apart from the screen.
    run.reset();
    for (wchar_t c : std::wstring(L"anhs")) run.key(c);
    for (wchar_t c : std::wstring(L"ánh")) run.append(c);
    run.key(L'a');
    run.append(L'a');
    check(run.window() == L"ánha" && run.raw() == L"anhsa", "screen and keys differ under Telex");
    run.key(L'\b');
    run.erase(1);
    check(run.window() == L"ánh" && run.raw() == L"anhs", "backspace removes one key");

    // A choice locks everything typed so far.
    run.lockAll();
    check(run.window().empty() && !run.endsInWord() && run.raw().empty(), "locked run has an empty chunk");

    // Erasing more than the run holds means the screen is unknown.
    run.erase(1000);
    check(run.empty(), "erasing more than the run resets it");

    // Long runs keep the newest characters; the context is bounded.
    for (int i = 0; i < 100; ++i) type(run, L"ab ");
    check(run.screen().size() == SuggestRun::MaxChars, "run is bounded");
    type(run, L"xyz");
    check(run.context().size() == SuggestRun::ContextChars, "context is bounded");

    check(SuggestRun::distinct({L"a", L"", L"a", L"b", L"c"}, 2) == std::vector<std::wstring>({L"a", L"b"}), "distinct");

    // JSON round trip.
    std::string json = suggestChunkJson(L"Hôm ", L"n\"ay", L"na\\y", 5);
    check(json == "{\"context\":\"H\xc3\xb4m \",\"screen\":\"n\\\"ay\",\"raw\":\"na\\\\y\",\"k\":5}", "request escaping");
    std::vector<std::wstring> parsed;
    check(suggestParseCandidates("{\"candidates\": [\"h\xc3\xb4m nay\", \"h\\u00f4m\\\"\"], \"telex\": false, \"ms\": 3.1}", parsed),
          "parse reply");
    check(parsed.size() == 2 && parsed[0] == L"hôm nay" && parsed[1] == L"hôm\"", "parsed values");
    check(suggestParseCandidates("{\"candidates\":[],\"telex\":true}", parsed) && parsed.empty(), "empty list");
    check(!suggestParseCandidates("{\"detail\":\"no\"}", parsed), "error reply");

    std::cout << "suggest tests; " << failures << " failures\n";
    return failures ? 1 : 0;
}
