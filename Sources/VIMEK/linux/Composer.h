// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <string>
namespace vimek {
struct Options {
    int method=0;
    bool spelling=true, restore=true, modern=false;
};
// Keep each input context's unfinished word separate. The shared engine uses
// global state, so replay only this bounded word before handling an edit.
class Composer {
    Options options_;
    std::string keys_;
    std::wstring text_;
    std::wstring render() const;
public:
    explicit Composer(Options options={}) : options_(options) {}
    void configure(Options options) {clear();options_=options;}
    void clear() {keys_.clear();text_.clear();}
    void press(char key);
    bool empty() const {return text_.empty();}
    bool full() const {return keys_.size()>=256;}
    unsigned length() const {return unsigned(text_.size());}
    std::string text() const;
    std::string finish();
};
}
