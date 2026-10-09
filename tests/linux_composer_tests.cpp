// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "Composer.h"
#include <iostream>
#include <stdexcept>
int main() {
    int failures=0,checks=0;
    auto check=[&](bool condition,const char* label) {
        ++checks;
        if(!condition){++failures;std::cerr<<"FAIL: "<<label<<'\n';}
    };
    struct Case {int method;const char* keys;const char* expected;};
    const Case cases[]={
        {0,"tieengs","tiếng"},{1,"tie6ng1","tiếng"},{2,"tieengs","tiếng"},{3,"tieengs","tiếng"},
        {0,"Vieetj","Việt"},{1,"Vie6t5","Việt"},{0,"dduwowngf","đường"},{1,"d9u7o7ng2","đường"},
        {0,"as","á"},{0,"af","à"},{0,"ar","ả"},{0,"ax","ã"},{0,"aj","ạ"},
        {0,"aa","â"},{0,"aw","ă"},{0,"ow","ơ"},{0,"uw","ư"},{0,"dd","đ"},
        {0,"aas","ấ"},{1,"a61","ấ"},{0,"aaa","aa"},{0,"ass","as"},
        {0,"asz","a"},{1,"a10","a"},{0,"as\b",""},{0,"as\bf","f"},
        {0,"tieengs\b","tiến"},{0,"1234567890","1234567890"},
        {0,"hoaf","hòa"},
    };
    for(const auto& test:cases) {
        vimek::Composer composer({test.method,true,true,false});
        for(const char* key=test.keys;*key;++key)composer.press(*key);
        if(composer.text()!=test.expected) {++failures;std::cerr<<"FAIL: "<<test.keys<<" -> "<<composer.text()<<" expected "<<test.expected<<'\n';}
    }
    vimek::Composer first,second({1,true,true,false});
    first.press('a');second.press('a');first.press('s');second.press('1');
    check(first.text()=="á"&&second.text()=="á","Independent Telex and VNI contexts");
    check(first.finish()=="á"&&first.empty(),"Commit finishes and clears preedit");
    check(first.finish().empty(),"Repeated focus out cannot commit twice");
    first.press('a');first.press('a');first.clear();first.press('o');first.press('w');
    check(first.text()=="ơ","Reset cancels prior word");
    first.configure({0,true,true,true});for(char c:std::string("hoaf"))first.press(c);
    check(first.text()=="hoà","Tone placement follows preferences");
    first.configure({0,false,false,false});
    for(char c:std::string("zax"))first.press(c);
    check(first.text()=="zã","Spelling can be disabled");
    first.configure({0,true,true,false});
    for(int i=0;i<200;++i)first.press('b');
    check(first.finish()==std::string(200,'b'),"Long words survive engine buffer boundaries");
    for(char c:std::string("hello-world"))first.press(c);
    check(first.finish()=="hello-world","Invalid English words restore when committed");
    std::cout<<sizeof(cases)/sizeof(cases[0])+checks<<" Linux composition scenarios; "<<failures<<" failures\n";
    return failures?1:0;
}
