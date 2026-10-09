// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "Composer.h"
#include "Engine.h"
#include <stdexcept>

// Linux initially exposes Unicode input. Desktop-only features stay disabled
// here; their settings must not leak in from another input context.
int vLanguage=1,vInputType=0,vFreeMark=0,vCodeTable=0,vSwitchKeyStatus=0;
int vCheckSpelling=1,vUseModernOrthography=0,vQuickTelex=0,vRestoreIfWrongSpelling=1;
int vFixRecommendBrowser=0,vUseMacro=0,vUseMacroInEnglishMode=0,vAutoCapsMacro=0;
int vUseSmartSwitchKey=0,vUpperCaseFirstChar=0,vTempOffSpelling=0,vAllowConsonantZFWJ=0;
int vQuickStartConsonant=0,vQuickEndConsonant=0,vRememberCode=0,vOtherLanguage=0,vTempOffVimek=0;

namespace {
const Uint16 letters[]={KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,
    KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z};
const Uint16 digits[]={KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};
const char punctuation[]=" \b.,/;'\\-=[]`";
const Uint16 punctuationKeys[]={KEY_SPACE,KEY_DELETE,KEY_DOT,KEY_COMMA,KEY_SLASH,
    KEY_SEMICOLON,KEY_QUOTE,KEY_BACK_SLASH,KEY_MINUS,KEY_EQUALS,KEY_LEFT_BRACKET,KEY_RIGHT_BRACKET,KEY_BACKQUOTE};
Uint16 keyFor(char c) {
    if(c>='A'&&c<='Z')c=char(c-'A'+'a');
    if(c>='a'&&c<='z')return letters[c-'a'];
    if(c>='0'&&c<='9')return digits[c-'0'];
    for(unsigned i=0;i<sizeof(punctuation)-1;++i)if(c==punctuation[i])return punctuationKeys[i];
    return KEY_EMPTY;
}
wchar_t decode(Uint32 data) {
    if(data&(CHAR_CODE_MASK|PURE_CHARACTER_MASK))return wchar_t(data&0xffff);
    Uint16 code=data&0xffff;
    for(unsigned i=0;i<26;++i)if(code==letters[i])return wchar_t((data&CAPS_MASK?'A':'a')+i);
    for(unsigned i=0;i<10;++i)if(code==digits[i])return L'0'+i;
    for(unsigned i=0;i<sizeof(punctuation)-1;++i)if(code==punctuationKeys[i])return punctuation[i];
    throw std::runtime_error("Unknown engine output key");
}
}
namespace vimek {
std::wstring Composer::render() const {
    vInputType=options_.method;vCheckSpelling=options_.spelling;
    vRestoreIfWrongSpelling=options_.restore;vUseModernOrthography=options_.modern;
    auto* hook=static_cast<vKeyHookState*>(vKeyInit());startNewSession();
    std::wstring output;
    for(char c:keys_) {
        Uint16 key=keyFor(c);
        if(key==KEY_EMPTY) {startNewSession();output+=wchar_t(c);continue;}
        vKeyHandleEvent(Keyboard,KeyDown,key,c>='A'&&c<='Z'?1:0,false);
        if(hook->code==vWillProcess||hook->code==vRestore||hook->code==vRestoreAndStartNewSession) {
            if(hook->backspaceCount>output.size())throw std::runtime_error("Engine edit exceeds preedit");
            output.resize(output.size()-hook->backspaceCount);
            for(int i=int(hook->newCharCount)-1;i>=0;--i)output+=decode(hook->charData[i]);
            if(hook->code!=vWillProcess)output+=wchar_t(c);
            if(hook->code==vRestoreAndStartNewSession)startNewSession();
        } else if(c=='\b') {if(!output.empty())output.pop_back();}
        else output+=wchar_t(c);
    }
    return output;
}
void Composer::press(char key) {
    keys_+=key;
    text_=render();
    if(text_.empty())keys_.clear();
}
std::string Composer::text() const {return wideStringToUtf8(text_);}
std::string Composer::finish() {
    if(empty())return {};
    press(' ');auto output=text_;output.pop_back();clear();
    return wideStringToUtf8(output);
}
}
