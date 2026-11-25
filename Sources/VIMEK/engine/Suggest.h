// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
// Model suggestions next to Telex: platform-independent state.
//
// The user types a chunk (the text since the last space): one word without marks, or several
// words without spaces (`homnaytroidepqua`). A local model proposes the segmented, marked forms
// and the user takes one with a digit; Space is an ordinary space. SuggestRun mirrors the text the input
// method has put on screen since the last break, so the platform layer knows exactly which
// characters to replace, and keeps the keys the user pressed for the chunk: the Telex engine may
// turn letters of an unspaced chunk into marks (`anhsang` shows as `ánhang`).
#ifndef Suggest_h
#define Suggest_h

#include <string>
#include <vector>

class SuggestRun {
public:
    // Longer runs drop their oldest characters; the model was trained on single sentences.
    static const size_t MaxChars = 192;
    // Context sent with a request: the last characters before the chunk.
    static const size_t ContextChars = 64;

    void reset();
    // A character the input method put on screen (typed, or sent by the Telex engine).
    void append(wchar_t ch);
    // Characters removed from the end of the screen text. Removing more than the run holds
    // means the screen no longer matches, so the run resets.
    void erase(size_t count);
    // A key the user pressed in the chunk: a character, L'\b' for Backspace, L' ' ends the chunk.
    void key(wchar_t ch);
    // Nothing before the current end may change any more (a choice was made, or Esc).
    void lockAll();

    const std::wstring& screen() const { return screen_; }
    const std::wstring& raw() const { return raw_; }
    bool empty() const { return screen_.empty(); }
    // True while the last character belongs to a word, i.e. the user is typing a chunk.
    bool endsInWord() const;
    // First character a suggestion may change: the start of the chunk (after the last space),
    // never before the locked part.
    size_t windowStart() const;
    std::wstring window() const { return screen_.substr(windowStart()); }
    // Up to ContextChars of screen text before the chunk.
    std::wstring context() const;
    // Identifies one request: context, chunk on screen and keys.
    std::wstring requestKey() const;

    // Distinct non-empty candidates, at most `limit`, best first.
    static std::vector<std::wstring> distinct(const std::vector<std::wstring>& candidates, size_t limit);

    // Show `text` in place of the chunk; its length may differ (spaces are added). Returns the
    // number of backspaces and the characters to send; the shared prefix is left alone.
    void replaceWindow(const std::wstring& text, size_t& backspaces, std::wstring& insert);

private:
    std::wstring screen_; // what is on screen
    std::wstring raw_;    // keys pressed for the current chunk
    size_t locked_ = 0;
    void trim();
};

// JSON for POST /chunk of virestore.serve, and the reply's "candidates" list.
std::string suggestChunkJson(const std::wstring& context, const std::wstring& screen, const std::wstring& raw, int k);
bool suggestParseCandidates(const std::string& json, std::vector<std::wstring>& out);

#endif /* Suggest_h */
