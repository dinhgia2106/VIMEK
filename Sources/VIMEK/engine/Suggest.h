// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
// Model suggestions next to Telex: platform-independent state.
//
// The user types words without marks (or with Telex marks), a local model proposes the marked
// form of the last few words, and the user takes one with Space (the first) or a digit.
// SuggestRun mirrors the text the input method has put on screen since the last break, so the
// platform layer knows exactly which characters to replace.
#ifndef Suggest_h
#define Suggest_h

#include <string>
#include <vector>

class SuggestRun {
public:
    // Longer runs drop their oldest characters; the model was trained on single sentences.
    static const size_t MaxChars = 192;
    // How many trailing words a suggestion may change: only the word being typed, from the start
    // of the sentence or the last space. Earlier words stay as they are.
    static const int WindowWords = 1;

    void reset();
    // A character the input method put on screen (typed, or sent by the Telex engine).
    void append(wchar_t ch);
    // Characters removed from the end of the screen text. Removing more than the run holds
    // means the screen no longer matches, so the run resets.
    void erase(size_t count);
    // Nothing before the current end may change any more (a choice was made, or Esc).
    void lockAll();

    const std::wstring& screen() const { return screen_; }
    bool empty() const { return screen_.empty(); }
    // True while the last character belongs to a word, i.e. the user is typing one.
    bool endsInWord() const;
    // First character a suggestion may change: the start of the last WindowWords words,
    // never before the locked part.
    size_t windowStart() const;
    std::wstring window() const { return screen_.substr(windowStart()); }
    // The user already marked the window with Telex (or typed a marked letter): no suggestion.
    bool windowTypedWithMarks() const;
    // What the model sees: earlier text as shown, the window as the user typed it.
    std::wstring requestText() const;

    // Distinct windows (at most `limit`) cut from candidates for `request`, best first.
    // Candidates whose length differs from the request are skipped: the tagger keeps length.
    static std::vector<std::wstring> windows(const std::wstring& request, size_t start,
                                             const std::vector<std::wstring>& candidates, size_t limit);

    // Show `text` in place of the current window. Returns the number of backspaces and the
    // characters to send; the shared prefix is left alone. False if `text` does not fit.
    bool replaceWindow(const std::wstring& text, size_t& backspaces, std::wstring& insert);

private:
    std::wstring screen_; // what is on screen
    std::wstring typed_;  // same length: the screen without the model's replacements
    size_t locked_ = 0;
    void trim();
};

// JSON for POST /candidates of virestore.serve, and the reply's "candidates" list.
std::string suggestRequestJson(const std::wstring& text, int k);
bool suggestParseCandidates(const std::string& json, std::vector<std::wstring>& out);

#endif /* Suggest_h */
