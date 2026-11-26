// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.h"
#include "SuggestWindows.h"
#include "VimekTheme.h"
#include "WindowsInput.h"
#include <winhttp.h>
#include <oleacc.h>
#include <UIAutomation.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#pragma comment(lib, "winhttp")
#pragma comment(lib, "oleacc")
#pragma comment(lib, "oleaut32")

int vAiSuggest = 0;
int vAiSuggestPort = 8765;

namespace {
const UINT WM_SUGGEST_REPLY = WM_APP + 0x51;
const wchar_t POPUP_CLASS[] = L"VimekSuggestPopup";
const size_t SHOWN = 5;    // digits 1-5
const int REQUEST_K = 5;

struct Job {
    std::wstring key, context, screen, raw;
};

struct Reply {
    std::wstring key;
    std::vector<std::wstring> windows;
    RECT caret = {};  // where the caret was when the reply came back
};

SuggestRun theRun;
HWND popup = nullptr;
std::vector<std::wstring> shown;
HFONT textFont = nullptr, numberFont = nullptr;
int scale = 96;

// Shared with the worker thread.
std::mutex mu;
std::condition_variable cv;
std::thread worker;
bool stopping = false, queued = false, hasReply = false;
Job queuedJob;
std::wstring wantedKey;
Reply latest;

int px(int value) { return MulDiv(value, scale, 96); }

std::string post(HINTERNET session, const std::string& body) {
    std::string out;
    HINTERNET connection = WinHttpConnect(session, L"127.0.0.1", (INTERNET_PORT)vAiSuggestPort, 0);
    if (!connection) return out;
    HINTERNET request = WinHttpOpenRequest(connection, L"POST", L"/chunk", nullptr, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (request) {
        const wchar_t headers[] = L"Content-Type: application/json\r\n";
        if (WinHttpSendRequest(request, headers, (DWORD)-1L, (LPVOID)body.data(), (DWORD)body.size(),
                               (DWORD)body.size(), 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD status = 0, size = sizeof(status);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
            DWORD available = 0;
            while (status == 200 && WinHttpQueryDataAvailable(request, &available) && available) {
                std::string chunk(available, '\0');
                DWORD read = 0;
                if (!WinHttpReadData(request, &chunk[0], available, &read) || !read) break;
                out.append(chunk.data(), read);
            }
        }
        WinHttpCloseHandle(request);
    }
    WinHttpCloseHandle(connection);
    return out;
}

// GET /health: true if a server already answers on the port (ours, or one the user started).
bool serverAnswers() {
    HINTERNET session = WinHttpOpen(L"VIMEK", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return false;
    WinHttpSetTimeouts(session, 300, 300, 300, 500);
    bool ok = false;
    if (HINTERNET connection = WinHttpConnect(session, L"127.0.0.1", (INTERNET_PORT)vAiSuggestPort, 0)) {
        if (HINTERNET request = WinHttpOpenRequest(connection, L"GET", L"/health", nullptr, WINHTTP_NO_REFERER,
                                                   WINHTTP_DEFAULT_ACCEPT_TYPES, 0)) {
            DWORD status = 0, size = sizeof(status);
            ok = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                 WinHttpReceiveResponse(request, nullptr) &&
                 WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                     WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX) &&
                 status == 200;
            WinHttpCloseHandle(request);
        }
        WinHttpCloseHandle(connection);
    }
    WinHttpCloseHandle(session);
    return ok;
}

// The bundled server. It lives in a job that closes with VIMEK, so it never outlives VIMEK, even
// when VIMEK is killed. One thread starts and stops it; the keyboard hook only wakes that thread.
namespace server {
const DWORD RESTART_MS = 30000; // a server that exits is started again at most this often

std::mutex mu;
std::condition_variable cv;
std::thread thread;
bool stopping = false, wake = false;
HANDLE job = nullptr, process = nullptr;
ULONGLONG lastStart = 0;

std::wstring exePath() {
    wchar_t self[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, self, MAX_PATH);
    if (!n || n == MAX_PATH) return L"";
    std::wstring dir(self, n);
    dir.resize(dir.find_last_of(L"\\/") + 1);
    return dir + L"suggest\\vimek-suggest.exe";
}

void stop() {
    if (!process) return;
    TerminateProcess(process, 0);
    WaitForSingleObject(process, 2000);
    CloseHandle(process);
    process = nullptr;
}

void start() {
    std::wstring exe = exePath();
    if (exe.empty() || GetFileAttributesW(exe.c_str()) == INVALID_FILE_ATTRIBUTES) return;
    if (!job) {
        job = CreateJobObjectW(nullptr, nullptr);
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (job) SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    }
    std::wstring dir = exe.substr(0, exe.find_last_of(L'\\'));
    std::wstring cmd = L"\"" + exe + L"\" --port " + std::to_wstring(vAiSuggestPort);
    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi = {};
    lastStart = GetTickCount64();
    // Suspended until it is in the job, so not even its first instant runs outside it.
    if (!CreateProcessW(exe.c_str(), &cmd[0], nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, dir.c_str(), &si, &pi))
        return;
    if (job) AssignProcessToJobObject(job, pi.hProcess);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    process = pi.hProcess;
}

bool running() { return process && WaitForSingleObject(process, 0) == WAIT_TIMEOUT; }

void loop() {
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(mu);
            cv.wait_for(lock, std::chrono::milliseconds(RESTART_MS), [] { return stopping || wake; });
            if (stopping) break;
            wake = false;
        }
        if (!vAiSuggest) {
            stop();
            continue;
        }
        if (running()) continue;
        if (process) { // it exited: maybe the port was taken, maybe it crashed
            CloseHandle(process);
            process = nullptr;
            if (GetTickCount64() - lastStart < RESTART_MS) continue;
        }
        if (!serverAnswers()) start();
    }
    stop();
}

void notify() {
    {
        std::lock_guard<std::mutex> lock(mu);
        wake = true;
    }
    cv.notify_all();
}
} // namespace server

// UI Automation, looked up on the worker thread: some applications answer slowly, and the
// keyboard hook must never wait on them. GUIDs are spelled out so MSVC and MinGW link alike.
const CLSID CLSID_UiaClient = {0xff48dba4, 0x60ef, 0x4201, {0xaa, 0x87, 0x54, 0x10, 0x3e, 0xef, 0x59, 0x4e}};
const IID IID_UiaClient = {0x30cbe57d, 0xd9d0, 0x452a, {0xab, 0x13, 0x7a, 0xc5, 0xac, 0x48, 0x25, 0xee}};
const IID IID_UiaTextPattern = {0x32eba289, 0x3583, 0x42c9, {0x9c, 0x59, 0x3b, 0x6d, 0x9a, 0x1e, 0x9b, 0x6a}};
const IID IID_UiaTextPattern2 = {0x506a921a, 0xfcc9, 0x409f, {0xb2, 0x3b, 0x37, 0xeb, 0x74, 0x10, 0x68, 0x72}};

// Last rectangle of a text range (the line the caret is on), in screen pixels.
bool rangeRect(IUIAutomationTextRange* range, RECT& r) {
    SAFEARRAY* rects = nullptr;
    bool ok = false;
    if (SUCCEEDED(range->GetBoundingRectangles(&rects)) && rects) {
        LONG lo = 0, hi = -1;
        SafeArrayGetLBound(rects, 1, &lo);
        SafeArrayGetUBound(rects, 1, &hi);
        double* d = nullptr;
        LONG n = hi - lo + 1;
        if (n >= 4 && SUCCEEDED(SafeArrayAccessData(rects, (void**)&d))) {
            const double* q = d + (n / 4 - 1) * 4;
            r = {(LONG)q[0], (LONG)q[1], (LONG)(q[0] + q[2]), (LONG)(q[1] + q[3])};
            ok = q[3] > 0;
            SafeArrayUnaccessData(rects);
        }
        SafeArrayDestroy(rects);
    }
    return ok;
}

bool uiaCaret(IUIAutomation* uia, RECT& r) {
    IUIAutomationElement* element = nullptr;
    if (!uia || FAILED(uia->GetFocusedElement(&element)) || !element) return false;
    IUIAutomationTextRange* range = nullptr;
    IUIAutomationTextPattern2* text2 = nullptr;
    if (SUCCEEDED(element->GetCurrentPatternAs(UIA_TextPattern2Id, IID_UiaTextPattern2, (void**)&text2)) && text2) {
        BOOL active = FALSE;
        text2->GetCaretRange(&active, &range);
        text2->Release();
    }
    if (!range) {
        IUIAutomationTextPattern* text = nullptr;
        if (SUCCEEDED(element->GetCurrentPatternAs(UIA_TextPatternId, IID_UiaTextPattern, (void**)&text)) && text) {
            IUIAutomationTextRangeArray* selection = nullptr;
            if (SUCCEEDED(text->GetSelection(&selection)) && selection) {
                int length = 0;
                selection->get_Length(&length);
                if (length > 0) selection->GetElement(length - 1, &range);
                selection->Release();
            }
            text->Release();
        }
    }
    bool ok = false;
    if (range) {
        ok = rangeRect(range, r);
        if (!ok) { // an empty range often has no rectangle: measure the character before the caret
            IUIAutomationTextRange* before = nullptr;
            if (SUCCEEDED(range->Clone(&before)) && before) {
                int moved = 0;
                before->MoveEndpointByUnit(TextPatternRangeEndpoint_Start, TextUnit_Character, -1, &moved);
                if (moved && rangeRect(before, r)) {
                    r.left = r.right;
                    ok = true;
                }
                before->Release();
            }
        }
        range->Release();
    }
    element->Release();
    if (ok) r.right = r.left + 1;
    return ok;
}

// Screen rectangle of the caret in the foreground window; the mouse position if unknown.
RECT caretRect(IUIAutomation* uia) {
    RECT r = {};
    // Browsers, Office and modern editors draw their own caret and report it through UI Automation.
    if (uiaCaret(uia, r)) return r;
    HWND foreground = GetForegroundWindow();
    GUITHREADINFO info = {};
    info.cbSize = sizeof(info);
    if (foreground && GetGUIThreadInfo(GetWindowThreadProcessId(foreground, nullptr), &info) && info.hwndCaret &&
        !IsRectEmpty(&info.rcCaret)) {
        // rcCaret is in the caret window's own client units. For a window that is not DPI aware
        // they are 96-DPI units that Windows scales up, so scale them to ours before adding the
        // (already scaled) client origin.
        typedef UINT(WINAPI * DpiForWindow)(HWND);
        static DpiForWindow dpiForWindow =
            (DpiForWindow)GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow");
        UINT windowDpi = dpiForWindow ? dpiForWindow(info.hwndCaret) : 0;
        if (!windowDpi) windowDpi = (UINT)scale;
        POINT origin = {0, 0};
        ClientToScreen(info.hwndCaret, &origin);
        r.left = origin.x + MulDiv(info.rcCaret.left, scale, windowDpi);
        r.top = origin.y + MulDiv(info.rcCaret.top, scale, windowDpi);
        r.right = origin.x + MulDiv(info.rcCaret.right, scale, windowDpi);
        r.bottom = origin.y + MulDiv(info.rcCaret.bottom, scale, windowDpi);
        return r;
    }
    IAccessible* acc = nullptr;
    if (foreground && SUCCEEDED(AccessibleObjectFromWindow(foreground, (DWORD)OBJID_CARET, IID_IAccessible, (void**)&acc)) && acc) {
        long x = 0, y = 0, w = 0, h = 0;
        VARIANT self;
        self.vt = VT_I4;
        self.lVal = CHILDID_SELF;
        HRESULT hr = acc->accLocation(&x, &y, &w, &h, self);
        acc->Release();
        if (SUCCEEDED(hr) && (x || y)) {
            r = {x, y, x + w, y + h};
            return r;
        }
    }
    POINT p;
    GetCursorPos(&p);
    r = {p.x, p.y, p.x + 1, p.y + px(20)};
    return r;
}

void workerLoop() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IUIAutomation* uia = nullptr;
    CoCreateInstance(CLSID_UiaClient, nullptr, CLSCTX_INPROC_SERVER, IID_UiaClient, (void**)&uia);
    HINTERNET session = WinHttpOpen(L"VIMEK", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS, 0);
    // A local server answers in tens of milliseconds; never let a dead one pile up requests.
    if (session) WinHttpSetTimeouts(session, 300, 300, 300, 1000);
    for (;;) {
        Reply reply;
        Job job;
        {
            std::unique_lock<std::mutex> lock(mu);
            cv.wait(lock, [] { return stopping || queued; });
            if (stopping) break;
            job = queuedJob;
            queued = false;
        }
        reply.key = job.key;
        std::vector<std::wstring> candidates;
        if (session && suggestParseCandidates(post(session, suggestChunkJson(job.context, job.screen, job.raw, REQUEST_K)), candidates))
            reply.windows = SuggestRun::distinct(candidates, SHOWN);
        if (!reply.windows.empty()) reply.caret = caretRect(uia);
        {
            std::lock_guard<std::mutex> lock(mu);
            latest = reply;
            hasReply = true;
        }
        cv.notify_all();
        if (popup) PostMessage(popup, WM_SUGGEST_REPLY, 0, 0);
    }
    if (session) WinHttpCloseHandle(session);
    if (uia) uia->Release();
    CoUninitialize();
}

struct Colors {
    COLORREF background, text, number, border;
};
Colors colors() {
    if (vimekUsesDarkTheme()) return {RGB(32, 32, 36), RGB(236, 236, 240), RGB(110, 168, 254), RGB(80, 80, 88)};
    return {RGB(255, 255, 255), RGB(24, 24, 28), RGB(37, 99, 235), RGB(196, 196, 204)};
}

// Lays the items out left to right; returns the size needed. Draws when dc is given a paint target.
SIZE layout(HDC dc, bool draw) {
    const int pad = px(8), gapNumber = px(4), gapItem = px(14);
    Colors c = colors();
    int x = pad, height = 0;
    for (size_t i = 0; i < shown.size(); ++i) {
        std::wstring number = std::to_wstring(i + 1);
        SIZE s;
        SelectObject(dc, numberFont);
        GetTextExtentPoint32W(dc, number.c_str(), (int)number.size(), &s);
        if (draw) {
            SetTextColor(dc, c.number);
            TextOutW(dc, x, pad, number.c_str(), (int)number.size());
        }
        x += s.cx + gapNumber;
        height = max(height, (int)s.cy);
        SelectObject(dc, textFont);
        GetTextExtentPoint32W(dc, shown[i].c_str(), (int)shown[i].size(), &s);
        if (draw) {
            SetTextColor(dc, c.text);
            TextOutW(dc, x, pad, shown[i].c_str(), (int)shown[i].size());
        }
        x += s.cx + (i + 1 < shown.size() ? gapItem : 0);
        height = max(height, (int)s.cy);
    }
    SIZE size = {x + pad, height + 2 * pad};
    return size;
}

void showNearCaret(RECT caret) {
    HDC dc = GetDC(popup);
    SIZE size = layout(dc, false);
    ReleaseDC(popup, dc);
    // Some toolkits (Tk, several games) report a caret only a few pixels tall: assume a line.
    if (caret.bottom - caret.top < px(16)) caret.bottom = caret.top + px(24);
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(MonitorFromRect(&caret, MONITOR_DEFAULTTONEAREST), &mi);
    int x = caret.left, y = caret.bottom + px(4);
    if (y + size.cy > mi.rcWork.bottom) y = caret.top - size.cy - px(4);
    if (x + size.cx > mi.rcWork.right) x = mi.rcWork.right - size.cx;
    if (x < mi.rcWork.left) x = mi.rcWork.left;
    SetWindowPos(popup, HWND_TOPMOST, x, y, size.cx, size.cy, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(popup, nullptr, TRUE);
}

void refresh() {
    if (!VimekSuggest::active() || !theRun.endsInWord()) {
        VimekSuggest::hide();
        return;
    }
    std::vector<std::wstring> items = VimekSuggest::current(0);
    std::wstring now = theRun.window();
    bool differs = false;
    for (const std::wstring& w : items) differs = differs || w != now;
    if (!differs) {
        // Keep what is on screen while a newer reply is on its way, if it still fits.
        if (items.empty() && VimekSuggest::visible()) return;
        VimekSuggest::hide();
        return;
    }
    RECT caret;
    {
        std::lock_guard<std::mutex> lock(mu);
        caret = latest.caret;
    }
    shown = items;
    showNearCaret(caret);
}

LRESULT CALLBACK popupProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SUGGEST_REPLY:
        refresh();
        return 0;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(window, &ps);
        RECT rc;
        GetClientRect(window, &rc);
        Colors c = colors();
        HBRUSH background = CreateSolidBrush(c.background);
        FillRect(dc, &rc, background);
        DeleteObject(background);
        HBRUSH border = CreateSolidBrush(c.border);
        FrameRect(dc, &rc, border);
        DeleteObject(border);
        SetBkMode(dc, TRANSPARENT);
        HGDIOBJ old = SelectObject(dc, textFont);
        layout(dc, true);
        SelectObject(dc, old);
        EndPaint(window, &ps);
        return 0;
    }
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
} // namespace

namespace VimekSuggest {

void init(HINSTANCE instance) {
    if (popup) return;
    HDC screen = GetDC(nullptr);
    scale = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(nullptr, screen);
    textFont = CreateFontW(-MulDiv(11, scale, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    numberFont = CreateFontW(-MulDiv(10, scale, 72), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = popupProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = POPUP_CLASS;
    RegisterClassExW(&wc);
    popup = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, POPUP_CLASS, L"", WS_POPUP,
                            0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
    stopping = false;
    worker = std::thread(workerLoop);
    // A --test-input run brings its own server (or none); it never starts the bundled one.
    if (!VimekWindowsInput::testInput()) {
        server::stopping = false;
        server::thread = std::thread(server::loop);
        server::notify();
    }
}

void serverChanged() {
    if (server::thread.joinable()) server::notify();
}

void shutdown() {
    {
        std::lock_guard<std::mutex> lock(mu);
        stopping = true;
    }
    cv.notify_all();
    if (worker.joinable()) worker.join();
    {
        std::lock_guard<std::mutex> lock(server::mu);
        server::stopping = true;
    }
    server::cv.notify_all();
    if (server::thread.joinable()) server::thread.join();
    if (popup) DestroyWindow(popup);
    popup = nullptr;
    if (textFont) DeleteObject(textFont);
    if (numberFont) DeleteObject(numberFont);
    textFont = numberFont = nullptr;
}

bool active() {
    // VNI uses digits for marks, and other code tables are not one character per letter.
    return vAiSuggest && popup && vLanguage == 1 && vCodeTable == 0 && vInputType != 1;
}

SuggestRun& run() { return theRun; }

void changed() {
    // The server decides whether the chunk is a word marked with Telex (no candidates then).
    if (!active() || !theRun.endsInWord()) {
        hide();
        return;
    }
    Job job = {theRun.requestKey(), theRun.context(), theRun.window(), theRun.raw()};
    {
        std::lock_guard<std::mutex> lock(mu);
        if (job.key == wantedKey) return;
        wantedKey = job.key;
        queuedJob = job;
        queued = true;
    }
    cv.notify_all();
}

void reset() {
    theRun.reset();
    hide();
}

void hide() {
    if (popup && IsWindowVisible(popup)) ShowWindow(popup, SW_HIDE);
    shown.clear();
}

bool visible() { return popup && IsWindowVisible(popup); }

std::vector<std::wstring> current(DWORD waitMs) {
    std::wstring key = theRun.requestKey();
    std::unique_lock<std::mutex> lock(mu);
    auto ready = [&] { return hasReply && latest.key == key; };
    if (!ready() && waitMs && key == wantedKey)
        cv.wait_for(lock, std::chrono::milliseconds(waitMs), ready);
    return ready() ? latest.windows : std::vector<std::wstring>();
}

} // namespace VimekSuggest
