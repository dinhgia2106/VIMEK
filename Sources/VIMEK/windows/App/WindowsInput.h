// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <windows.h>
#include <imm.h>
#include <cwchar>

namespace VimekWindowsInput {
// Driver-supplied extra information is not an injected keystroke. Some
// keyboards attach it to physical events, including inside games.
inline bool isInjected(const KBDLLHOOKSTRUCT& key) {
    return (key.flags & LLKHF_INJECTED) != 0;
}

inline bool queryImeOpen(HWND ime) {
    if (!ime) return false;
    DWORD_PTR open = 0;
    // Never let a game's message loop block our keyboard hook or tray.
    // BLOCK also prevents another hook callback re-entering engine state.
    return SendMessageTimeoutW(ime, WM_IME_CONTROL, 0x0005, 0,
        SMTO_BLOCK | SMTO_ABORTIFHUNG | SMTO_ERRORONEXIT, 10, &open) && open;
}

inline bool usesGameTextInput(HWND window) {
    wchar_t name[64] = {};
    GetClassNameW(window, name, 64);
    return wcscmp(name, L"UnityWndClass") == 0;
}
}
