// VIMEK regression tests. GPL-3.0, see LICENSE.
#include "Engine.h"
#include <iostream>
#include <stdexcept>
#include <string>

int vLanguage = 1, vInputType = 0, vFreeMark = 0, vCodeTable = 0;
int vSwitchKeyStatus = 0, vCheckSpelling = 1, vUseModernOrthography = 0;
int vQuickTelex = 0, vRestoreIfWrongSpelling = 1, vFixRecommendBrowser = 0;
int vUseMacro = 0, vUseMacroInEnglishMode = 0, vAutoCapsMacro = 0;
int vUseSmartSwitchKey = 0, vUpperCaseFirstChar = 0, vTempOffSpelling = 0;
int vAllowConsonantZFWJ = 0, vQuickStartConsonant = 0, vQuickEndConsonant = 0;
int vRememberCode = 0, vOtherLanguage = 0, vTempOffVimek = 0;

static const Uint16 letterKeys[] = {
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z
};
static Uint16 keyFor(char c) {
    if (c >= 'a' && c <= 'z') return letterKeys[c - 'a'];
    if (c >= 'A' && c <= 'Z') return letterKeys[c - 'A'];
    const Uint16 digitKeys[] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9};
    if (c >= '0' && c <= '9') return digitKeys[c - '0'];
    if (c == ' ') return KEY_SPACE;
    if (c == '\b') return KEY_DELETE;
    if (c == '.') return KEY_DOT;
    throw std::runtime_error("Unsupported test key");
}
static wchar_t decode(Uint32 c) {
    if (c & (CHAR_CODE_MASK | PURE_CHARACTER_MASK)) return wchar_t(c & 0xffff);
    for (int i = 0; i < 26; ++i) {
        if ((c & 0xffff) == letterKeys[i]) return wchar_t((c & CAPS_MASK ? 'A' : 'a') + i);
    }
    if ((c & 0xffff) == KEY_SPACE) return L' ';
    const Uint16 digitKeys[] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9};
    for (int i = 0; i < 10; ++i) if ((c & 0xffff) == digitKeys[i]) return L'0' + i;
    throw std::runtime_error("Unsupported engine character");
}
static std::wstring type(const std::string& keys, int method) {
    vInputType = method;
    auto* hook = static_cast<vKeyHookState*>(vKeyInit());
    startNewSession();
    std::wstring text;
    for (char c : keys) {
        bool upper = c >= 'A' && c <= 'Z';
        vKeyHandleEvent(Keyboard, KeyDown, keyFor(c), upper ? 1 : 0, false);
        if (hook->code == vWillProcess || hook->code == vRestore || hook->code == vRestoreAndStartNewSession) {
            if (hook->backspaceCount > text.size()) throw std::runtime_error("Backspace exceeds document");
            text.resize(text.size() - hook->backspaceCount);
            for (int i = int(hook->newCharCount) - 1; i >= 0; --i) text += decode(hook->charData[i]);
            if (hook->code == vRestore || hook->code == vRestoreAndStartNewSession) text += wchar_t(c);
            if (hook->code == vRestoreAndStartNewSession) startNewSession();
        } else if (c == '\b') {
            if (!text.empty()) text.pop_back();
        } else {
            text += wchar_t(c);
        }
    }
    return text;
}
#include "ModifierShortcut.h"
static int shortcutTests() {
    struct Event { unsigned mask; bool key = false, blocked = false, fire = false; };
    const std::vector<std::vector<Event>> cases = {
        {{1},{3},{2,false,false,true},{0}},
        {{2},{3},{1,false,false,true},{0}},
        {{1},{3},{3},{3},{1,false,false,true},{0}},
        {{1},{0}}, {{2},{0}},
        {{1},{3},{3,true},{1},{0}},
        {{1},{3},{11},{3},{2},{0}},
        {{9},{11},{3},{1},{0}},
        {{1},{3},{7},{3},{1},{0}}, // Win/Command also cancels
        {{1},{3,false,true},{1},{0}},
        {{1},{3},{2,false,false,true},{3},{1,false,false,true},{0}},
        {{1},{3},{1,false,false,true},{0},{2},{3},{1,false,false,true},{0}},
        {{1},{3},{3,true},{1},{0},{2},{3},{2,false,false,true},{0}},
        {{1},{3},{1,false,false,true},{3},{1,false,false,true},{3},{1,false,false,true},{0}}, // Hold Control, tap Alt
        {{2},{3},{2,false,false,true},{3},{2,false,false,true},{3},{2,false,false,true},{0}}, // Hold Alt, tap Control
        {{1},{3},{3},{1,false,false,true},{1},{1},{3},{3},{1,false,false,true},{0}}, // Repeats do not toggle twice
        {{1},{3},{1,false,false,true},{3},{3,true},{1},{3},{1},{0}}, // Letter cancels until fully released
        {{1},{3},{1,false,false,true},{3},{11},{3},{1},{0}}, // Extra modifier cancels next tap
        {{1},{3},{1,false,false,true},{3,false,true},{1},{3},{1},{0}} // AltGr cancels next tap
    };
    int failures=0;
    for(const auto& events:cases){VimekModifierShortcut chord;for(const auto& event:events)
        if(chord.update(event.mask,3,event.key,event.blocked)!=event.fire)++failures;}
    VimekModifierShortcut disabled;
    if(disabled.update(3,0)||disabled.update(0,0))++failures;
    std::cout<<cases.size()+1<<" shortcut scenarios; "<<failures<<" failures\n";
    return failures;
}
int main() {
    struct Case { int method; const char* keys; const wchar_t* expected; };
    const Case cases[] = {
        {0,"as",L"á"},{0,"af",L"à"},{0,"ar",L"ả"},{0,"ax",L"ã"},{0,"aj",L"ạ"},
        {0,"aa",L"â"},{0,"aw",L"ă"},{0,"ee",L"ê"},{0,"oo",L"ô"},{0,"ow",L"ơ"},{0,"uw",L"ư"},{0,"dd",L"đ"},
        {1,"a1",L"á"},{1,"a2",L"à"},{1,"a3",L"ả"},{1,"a4",L"ã"},{1,"a5",L"ạ"},
        {1,"a6",L"â"},{1,"a8",L"ă"},{1,"e6",L"ê"},{1,"o6",L"ô"},{1,"o7",L"ơ"},{1,"u7",L"ư"},{1,"d9",L"đ"},
        {0,"tieengs Vieetj",L"tiếng Việt"},{1,"tie6ng1 Vie6t5",L"tiếng Việt"},
        {0,"Tooi yeeu tieengs Vieetj",L"Tôi yêu tiếng Việt"},
        {0,"dduwowngf",L"đường"},{1,"d9u7o7ng2",L"đường"},
        {0,"hoaf",L"hòa"},{1,"hoa2",L"hòa"},
        {0,"aas",L"ấ"},{1,"a61",L"ấ"},{0,"aaa",L"aa"},{0,"ass",L"as"},
        {0,"asz",L"a"},{1,"a10",L"a"},{0,"as\bf",L"f"},
        {0,"hello world ",L"hello world "},{0,"VIMEK 2026",L"VIMEK 2026"},
        {2,"tieengs Vieetj",L"tiếng Việt"},{3,"tieengs Vieetj",L"tiếng Việt"},
        {-1,"as",L"á"},{99,"as",L"á"}
    };
    int failures = shortcutTests();
    for (const auto& c : cases) {
        const auto actual = type(c.keys, c.method);
        if (actual != c.expected) {
            ++failures;
            std::cerr << "FAIL method=" << c.method << " keys=" << c.keys
                      << " actual=" << wideStringToUtf8(actual) << " expected=" << wideStringToUtf8(c.expected) << '\n';
        }
    }
    // A corrupted preference during a session must not index past ProcessingChar.
    vInputType = 999;
    vKeyHandleEvent(Keyboard, KeyDown, KEY_A);
    if (vInputType != 0) ++failures;
    std::cout << (sizeof(cases) / sizeof(cases[0])) << " typing cases; " << failures << " failures\n";
    return failures ? 1 : 0;
}
