// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "Suggest.h"

#include <algorithm>

static bool isLetter(wchar_t c) {
    // Latin letters and the precomposed Vietnamese range (Latin-1 to Latin Extended Additional).
    return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= 0xC0 && c <= 0x1EF9 && c != 0xD7 && c != 0xF7);
}

void SuggestRun::reset() {
    screen_.clear();
    typed_.clear();
    locked_ = 0;
}

void SuggestRun::append(wchar_t ch) {
    screen_ += ch;
    typed_ += ch;
    trim();
}

void SuggestRun::trim() {
    if (screen_.size() <= MaxChars) return;
    size_t n = screen_.size() - MaxChars;
    screen_.erase(0, n);
    typed_.erase(0, n);
    locked_ = locked_ > n ? locked_ - n : 0;
}

void SuggestRun::erase(size_t count) {
    if (count > screen_.size()) {
        reset();
        return;
    }
    screen_.resize(screen_.size() - count);
    typed_.resize(typed_.size() - count);
    if (locked_ > screen_.size()) locked_ = screen_.size();
}

void SuggestRun::lockAll() {
    locked_ = screen_.size();
}

bool SuggestRun::endsInWord() const {
    return screen_.size() > locked_ && isLetter(screen_.back());
}

size_t SuggestRun::windowStart() const {
    size_t i = screen_.size(), start = screen_.size();
    int words = 0;
    while (i > locked_ && words < WindowWords) {
        while (i > locked_ && screen_[i - 1] == L' ') --i;
        if (i == locked_) break;
        while (i > locked_ && screen_[i - 1] != L' ') --i;
        start = i;
        ++words;
    }
    return start;
}

bool SuggestRun::windowTypedWithMarks() const {
    for (size_t i = windowStart(); i < typed_.size(); ++i)
        if ((unsigned long)typed_[i] > 0x7F) return true;
    return false;
}

std::wstring SuggestRun::requestText() const {
    size_t start = windowStart();
    return screen_.substr(0, start) + typed_.substr(start);
}

std::vector<std::wstring> SuggestRun::windows(const std::wstring& request, size_t start,
                                              const std::vector<std::wstring>& candidates, size_t limit) {
    std::vector<std::wstring> out;
    for (const std::wstring& c : candidates) {
        if (c.size() != request.size() || start > c.size()) continue;
        std::wstring w = c.substr(start);
        if (std::find(out.begin(), out.end(), w) == out.end()) out.push_back(w);
        if (out.size() >= limit) break;
    }
    return out;
}

bool SuggestRun::replaceWindow(const std::wstring& text, size_t& backspaces, std::wstring& insert) {
    size_t start = windowStart();
    if (text.size() != screen_.size() - start) return false;
    size_t same = 0;
    while (same < text.size() && text[same] == screen_[start + same]) ++same;
    backspaces = text.size() - same;
    insert = text.substr(same);
    screen_.replace(start, text.size(), text);
    return true;
}

// ---------------------------------------------------------------- JSON

static void putUtf8(std::string& out, unsigned long cp) {
    if (cp < 0x80) {
        out += (char)cp;
    } else if (cp < 0x800) {
        out += (char)(0xC0 | (cp >> 6));
        out += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += (char)(0xE0 | (cp >> 12));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    } else {
        out += (char)(0xF0 | (cp >> 18));
        out += (char)(0x80 | ((cp >> 12) & 0x3F));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
}

static void putWide(std::wstring& out, unsigned long cp) {
    if (sizeof(wchar_t) == 2 && cp >= 0x10000) {
        cp -= 0x10000;
        out += (wchar_t)(0xD800 + (cp >> 10));
        out += (wchar_t)(0xDC00 + (cp & 0x3FF));
    } else {
        out += (wchar_t)cp;
    }
}

std::string suggestRequestJson(const std::wstring& text, int k) {
    static const char hex[] = "0123456789abcdef";
    std::string out = "{\"text\":\"";
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned long cp = (unsigned long)text[i];
        if (sizeof(wchar_t) == 2 && cp >= 0xD800 && cp < 0xDC00 && i + 1 < text.size()) {
            unsigned long lo = (unsigned long)text[i + 1];
            if (lo >= 0xDC00 && lo < 0xE000) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                ++i;
            }
        }
        if (cp == '"' || cp == '\\') {
            out += '\\';
            out += (char)cp;
        } else if (cp < 0x20) {
            out += "\\u00";
            out += hex[cp >> 4];
            out += hex[cp & 15];
        } else {
            putUtf8(out, cp);
        }
    }
    out += "\",\"k\":" + std::to_string(k) + "}";
    return out;
}

static int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool readHex4(const std::string& s, size_t& i, unsigned long& v) {
    if (i + 4 > s.size()) return false;
    v = 0;
    for (int n = 0; n < 4; ++n) {
        int h = hexValue(s[i++]);
        if (h < 0) return false;
        v = (v << 4) | (unsigned long)h;
    }
    return true;
}

// Reads a JSON string starting at the opening quote; leaves i after the closing quote.
static bool readString(const std::string& s, size_t& i, std::wstring& out) {
    if (i >= s.size() || s[i] != '"') return false;
    ++i;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"') {
            ++i;
            return true;
        }
        if (c == '\\') {
            if (++i >= s.size()) return false;
            char e = s[i++];
            switch (e) {
            case '"': out += L'"'; break;
            case '\\': out += L'\\'; break;
            case '/': out += L'/'; break;
            case 'b': out += L'\b'; break;
            case 'f': out += L'\f'; break;
            case 'n': out += L'\n'; break;
            case 'r': out += L'\r'; break;
            case 't': out += L'\t'; break;
            case 'u': {
                unsigned long cp;
                if (!readHex4(s, i, cp)) return false;
                if (cp >= 0xD800 && cp < 0xDC00 && i + 1 < s.size() && s[i] == '\\' && s[i + 1] == 'u') {
                    size_t j = i + 2;
                    unsigned long lo;
                    if (readHex4(s, j, lo) && lo >= 0xDC00 && lo < 0xE000) {
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        i = j;
                    }
                }
                putWide(out, cp);
                break;
            }
            default: return false;
            }
            continue;
        }
        // UTF-8 sequence
        int extra = c < 0x80 ? 0 : (c >> 5) == 6 ? 1 : (c >> 4) == 14 ? 2 : (c >> 3) == 30 ? 3 : -1;
        if (extra < 0) return false;
        unsigned long cp = extra == 0 ? c : extra == 1 ? (c & 0x1F) : extra == 2 ? (c & 0x0F) : (c & 0x07);
        ++i;
        for (int n = 0; n < extra; ++n, ++i) {
            if (i >= s.size() || ((unsigned char)s[i] >> 6) != 2) return false;
            cp = (cp << 6) | ((unsigned char)s[i] & 0x3F);
        }
        putWide(out, cp);
    }
    return false;
}

static void skipSpace(const std::string& s, size_t& i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) ++i;
}

bool suggestParseCandidates(const std::string& json, std::vector<std::wstring>& out) {
    out.clear();
    size_t i = json.find("\"candidates\"");
    if (i == std::string::npos) return false;
    i += 12;
    skipSpace(json, i);
    if (i >= json.size() || json[i] != ':') return false;
    ++i;
    skipSpace(json, i);
    if (i >= json.size() || json[i] != '[') return false;
    ++i;
    skipSpace(json, i);
    if (i < json.size() && json[i] == ']') return true;
    while (i < json.size()) {
        std::wstring item;
        if (!readString(json, i, item)) return false;
        out.push_back(item);
        skipSpace(json, i);
        if (i < json.size() && json[i] == ',') {
            ++i;
            skipSpace(json, i);
            continue;
        }
        return i < json.size() && json[i] == ']';
    }
    return false;
}
