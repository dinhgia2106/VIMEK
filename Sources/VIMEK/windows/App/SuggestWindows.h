// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
// Model suggestions next to Telex on Windows (experimental).
//
// A local virestore server proposes marked forms of the last words typed. VIMEK starts the bundled
// one (suggestimek-suggest.exe next to VIMEK, see scripts/build-suggest-server.ps1) while
// suggestions are on, unless a server already answers on the port (`python -m virestore.serve`). A small window near the caret lists them: 1-5 take one, Esc keeps what was typed, and
// Space is an ordinary space. Requests run on a worker thread; the keyboard hook never waits on the network
// for longer than a short, bounded time.
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "../../engine/Suggest.h"

extern int vAiSuggest;      // 0: off, 1: on
extern int vAiSuggestPort;  // port of the local server on 127.0.0.1

namespace VimekSuggest {
void init(HINSTANCE instance);
// Also stops the bundled server.
void shutdown();
// Start or stop the bundled server to match vAiSuggest. Never blocks: the work runs on a thread.
void serverChanged();
// Suggestions are on and the current settings support them (Vietnamese, Unicode, Telex family).
bool active();
SuggestRun& run();
// Call after the run changed: asks for new suggestions or hides the window.
void changed();
// Forget the run (caret moved, focus changed, ...).
void reset();
void hide();
bool visible();
// Suggestions for the current run, best first. Waits at most waitMs for a reply that is
// still on its way; empty if none is ready.
std::vector<std::wstring> current(DWORD waitMs);
}
