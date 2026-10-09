// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#include "SystemTrayHelper.h"
#include "AppDelegate.h"

#define WM_TRAYMESSAGE (WM_USER + 1)
#define TRAY_ICONUID 100
#define WM_REFRESH_TRAY (WM_APP + 20)
#define TRAY_RETRY_TIMER 101

#define POPUP_VIET_ON_OFF 900
#define POPUP_SPELLING 901
#define POPUP_SMART_SWITCH 902
#define POPUP_USE_MACRO 903

#define POPUP_TELEX 910
#define POPUP_VNI 911
#define POPUP_SIMPLE_TELEX 912
#define POPUP_SIMPLE_TELEX_2 913

#define POPUP_UNICODE 930
#define POPUP_TCVN3 931
#define POPUP_VNI_WINDOWS 932
#define POPUP_UNICODE_COMPOUND 933
#define POPUP_VN_LOCALE_1258 934

#define POPUP_CONVERT_TOOL 980
#define POPUP_QUICK_CONVERT 981

#define POPUP_MACRO_TABLE 990

#define POPUP_CONTROL_PANEL 1000
#define POPUP_ABOUT_VIMEK 1010
#define POPUP_VIMEK_EXIT 2000

#define MODIFY_MENU(MENU, COMMAND, DATA) ModifyMenu(MENU, COMMAND, \
											MF_BYCOMMAND | (DATA ? MF_CHECKED : MF_UNCHECKED), \
											COMMAND, \
											menuData[COMMAND]);

static HMENU popupMenu;
static HMENU menuInputType, codeMenu, settingsMenu, toolsMenu;
static HMENU otherCode;

static NOTIFYICONDATA nid;
static bool iconAdded = false;
#ifdef VIMEK_TRAY_TESTING
extern BOOL WINAPI VimekTestNotifyIcon(DWORD, PNOTIFYICONDATA);
#endif
static BOOL notifyIcon(DWORD message) {
#ifdef VIMEK_TRAY_TESTING
	return VimekTestNotifyIcon(message, &nid);
#else
	return Shell_NotifyIcon(message, &nid);
#endif
}

map<UINT, LPCTSTR> menuData = {
	{POPUP_VIET_ON_OFF, _T("Bật Tiếng Việt")},
	{POPUP_SPELLING, _T("Bật kiểm tra chính tả")},
	{POPUP_SMART_SWITCH, _T("Nhớ chế độ Việt/Anh theo ứng dụng")},
	{POPUP_USE_MACRO, _T("Bật gõ tắt")},
	{POPUP_TELEX, _T("Kiểu gõ Telex")},
	{POPUP_VNI, _T("Kiểu gõ VNI")},
	{POPUP_SIMPLE_TELEX, _T("Kiểu gõ Simple Telex 1")},
	{POPUP_SIMPLE_TELEX_2, _T("Kiểu gõ Simple Telex 2")},
	{POPUP_UNICODE, _T("Unicode dựng sẵn")},
	{POPUP_TCVN3, _T("TCVN3 (ABC)")},
	{POPUP_VNI_WINDOWS, _T("VNI Windows")},
	{POPUP_UNICODE_COMPOUND, _T("Unicode tổ hợp")},
	{POPUP_VN_LOCALE_1258, _T("Vietnamese locale CP 1258")},
	{POPUP_CONVERT_TOOL, _T("Công cụ chuyển mã...")},
	{POPUP_QUICK_CONVERT, _T("Chuyển mã nhanh")},
	{POPUP_MACRO_TABLE, _T("Cấu hình gõ tắt...")},
	{POPUP_CONTROL_PANEL, _T("Bảng điều khiển...")},
	{POPUP_ABOUT_VIMEK, _T("Giới thiệu VIMEK")},
	{POPUP_VIMEK_EXIT, _T("Thoát")},
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static UINT taskbarCreated;

	switch (message) {
	case WM_CREATE:
		taskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
		// Explorer runs at normal integrity even when VIMEK is elevated.
		ChangeWindowMessageFilterEx(hWnd, taskbarCreated, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(hWnd, WM_TRAYMESSAGE, MSGFLT_ALLOW, nullptr);
		break;
	case WM_USER+2019:
		AppDelegate::getInstance()->onControlPanel();
		break;
	case WM_REFRESH_TRAY:
		SystemTrayHelper::updateData();
		break;
	case WM_TIMER:
		if (wParam == TRAY_RETRY_TIMER) SystemTrayHelper::updateData();
		break;
	case WM_TRAYMESSAGE: {
		if (lParam == WM_LBUTTONDBLCLK) {
			AppDelegate::getInstance()->onControlPanel();
		}
		if (lParam == WM_LBUTTONUP) {
			AppDelegate::getInstance()->onToggleVietnamese();
			SystemTrayHelper::updateData();
		} else if (lParam == WM_RBUTTONDOWN) {
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(hWnd);
			UINT commandId = TrackPopupMenu(
				popupMenu,
				TPM_RETURNCMD | TPM_NONOTIFY,
				curPoint.x,
				curPoint.y,
				0,
				hWnd,
				NULL
			);
			switch (commandId) {
			case POPUP_VIET_ON_OFF:
				AppDelegate::getInstance()->onToggleVietnamese();
				break;
			case POPUP_SPELLING:
				AppDelegate::getInstance()->onToggleCheckSpelling();
				break;
			case POPUP_SMART_SWITCH:
				AppDelegate::getInstance()->onToggleUseSmartSwitchKey();
				break;
			case POPUP_USE_MACRO:
				AppDelegate::getInstance()->onToggleUseMacro();
				break;
			case POPUP_MACRO_TABLE:
				AppDelegate::getInstance()->onMacroTable();
				break;
			case POPUP_CONVERT_TOOL:
				AppDelegate::getInstance()->onConvertTool();
				break;
			case POPUP_QUICK_CONVERT:
				AppDelegate::getInstance()->onQuickConvert();
				break;
			case POPUP_TELEX:
				AppDelegate::getInstance()->onInputType(0);
				break;
			case POPUP_VNI:
				AppDelegate::getInstance()->onInputType(1);
				break;
			case POPUP_SIMPLE_TELEX:
				AppDelegate::getInstance()->onInputType(2);
				break;
			case POPUP_SIMPLE_TELEX_2:
				AppDelegate::getInstance()->onInputType(3);
				break;
			case POPUP_UNICODE:
				AppDelegate::getInstance()->onTableCode(0);
				break;
			case POPUP_TCVN3:
				AppDelegate::getInstance()->onTableCode(1);
				break;
			case POPUP_VNI_WINDOWS:
				AppDelegate::getInstance()->onTableCode(2);
				break;
			case POPUP_UNICODE_COMPOUND:
				AppDelegate::getInstance()->onTableCode(3);
				break;
			case POPUP_VN_LOCALE_1258:
				AppDelegate::getInstance()->onTableCode(4);
				break;
			case POPUP_CONTROL_PANEL:
				AppDelegate::getInstance()->onControlPanel();
				break;
			case POPUP_ABOUT_VIMEK:
				AppDelegate::getInstance()->onVimekAbout();
				break;
			case POPUP_VIMEK_EXIT:
				AppDelegate::getInstance()->onVimekExit();
				break;
			}
			SystemTrayHelper::updateData();
		}
	}
	break;
	default:
		// if the taskbar is restarted, add the system tray icon again
		if (message == taskbarCreated) {
			iconAdded = false;
			SystemTrayHelper::updateData();
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND SystemTrayHelper::createFakeWindow(const HINSTANCE & hIns) {
	//create fake window
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hIns;
	wcex.hIcon = LoadIcon(hIns, MAKEINTRESOURCE(IDI_APP_ICON));
	wcex.hCursor = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = APP_CLASS;
	wcex.hIconSm = NULL;
	ATOM atom = RegisterClassExW(&wcex);
	HWND hWnd = CreateWindowW(APP_CLASS, _T(""), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hIns, nullptr);
	if (!hWnd) {
		return NULL;
	}
	ShowWindow(hWnd, 0);
	UpdateWindow(hWnd);
	return hWnd;
}

void SystemTrayHelper::createPopupMenu() {
	popupMenu = CreatePopupMenu();
	menuInputType=CreatePopupMenu();codeMenu=CreatePopupMenu();settingsMenu=CreatePopupMenu();toolsMenu=CreatePopupMenu();
	AppendMenu(popupMenu,MF_STRING,POPUP_CONTROL_PANEL,L"Mở VIMEK");
	AppendMenu(popupMenu,MF_SEPARATOR,0,nullptr);
	AppendMenu(popupMenu, MF_CHECKED, POPUP_VIET_ON_OFF, menuData[POPUP_VIET_ON_OFF]);
	for(UINT id=POPUP_TELEX;id<=POPUP_SIMPLE_TELEX_2;++id)AppendMenu(menuInputType,MF_STRING,id,menuData[id]);
	AppendMenu(popupMenu,MF_POPUP,(UINT_PTR)menuInputType,L"Kiểu gõ");
	for(UINT id=POPUP_UNICODE;id<=POPUP_VNI_WINDOWS;++id)AppendMenu(codeMenu,MF_STRING,id,menuData[id]);
	otherCode = codeMenu;
	AppendMenu(otherCode, MF_CHECKED, POPUP_UNICODE_COMPOUND, menuData[POPUP_UNICODE_COMPOUND]);
	AppendMenu(otherCode, MF_CHECKED, POPUP_VN_LOCALE_1258, menuData[POPUP_VN_LOCALE_1258]);
	AppendMenu(popupMenu, MF_POPUP, (UINT_PTR)codeMenu, L"Bảng mã");
	for(UINT id=POPUP_SPELLING;id<=POPUP_USE_MACRO;++id)AppendMenu(settingsMenu,MF_STRING,id,menuData[id]);
	AppendMenu(popupMenu,MF_POPUP,(UINT_PTR)settingsMenu,L"Tùy chọn");
	for(UINT id:{POPUP_MACRO_TABLE,POPUP_CONVERT_TOOL,POPUP_QUICK_CONVERT})AppendMenu(toolsMenu,MF_STRING,id,menuData[id]);
	AppendMenu(popupMenu,MF_POPUP,(UINT_PTR)toolsMenu,L"Công cụ");
	AppendMenu(popupMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(popupMenu, MF_UNCHECKED, POPUP_ABOUT_VIMEK, menuData[POPUP_ABOUT_VIMEK]);
	AppendMenu(popupMenu, MF_UNCHECKED, POPUP_VIMEK_EXIT, menuData[POPUP_VIMEK_EXIT]);

	SetMenuDefaultItem(popupMenu, POPUP_CONTROL_PANEL, false);
}

static void loadTrayIcon() {
	int icon = 0;
	if (vLanguage) {
		icon = vUseGrayIcon ? IDI_ICON_STATUS_VIET_10 : IDI_ICON_STATUS_VIET;
		LoadString(GetModuleHandle(0), IDS_TRAY_TITLE_2, nid.szTip, 128);
	}
	else {
		icon = vUseGrayIcon ? IDI_ICON_STATUS_ENG_10 : IDI_ICON_STATUS_ENG;
		LoadString(GetModuleHandle(0), IDS_TRAY_TITLE, nid.szTip, 128);
	}
	nid.hIcon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(icon), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
}

void SystemTrayHelper::updateData() {
	loadTrayIcon();
	if (nid.hWnd) {
		// A hidden taskbar or Explorer restart must not leave an old mode in
		// the Shell. Retry from the current engine state, never an old snapshot.
		bool updated = iconAdded && notifyIcon(NIM_MODIFY);
		if (!updated) updated = notifyIcon(NIM_ADD) || notifyIcon(NIM_MODIFY);
		iconAdded = updated;
		if (updated) KillTimer(nid.hWnd, TRAY_RETRY_TIMER);
		else SetTimer(nid.hWnd, TRAY_RETRY_TIMER, 1000, nullptr);
	}

	MODIFY_MENU(popupMenu, POPUP_VIET_ON_OFF, vLanguage);
	MODIFY_MENU(settingsMenu, POPUP_SPELLING, vCheckSpelling);
	MODIFY_MENU(settingsMenu, POPUP_SMART_SWITCH, vUseSmartSwitchKey);
	MODIFY_MENU(settingsMenu, POPUP_USE_MACRO, vUseMacro);
	MODIFY_MENU(menuInputType, POPUP_TELEX, vInputType == 0);
	MODIFY_MENU(menuInputType, POPUP_VNI, vInputType == 1);
	MODIFY_MENU(menuInputType, POPUP_SIMPLE_TELEX, vInputType == 2);
	MODIFY_MENU(menuInputType, POPUP_SIMPLE_TELEX_2, vInputType == 3);
	MODIFY_MENU(codeMenu, POPUP_UNICODE, vCodeTable == 0);
	MODIFY_MENU(codeMenu, POPUP_TCVN3, vCodeTable == 1);
	MODIFY_MENU(codeMenu, POPUP_VNI_WINDOWS, vCodeTable == 2);
	MODIFY_MENU(otherCode, POPUP_UNICODE_COMPOUND, vCodeTable == 3);
	MODIFY_MENU(otherCode, POPUP_VN_LOCALE_1258, vCodeTable == 4);

	wstring hotkey = L"";
	bool hasAdd = false;
	if (convertToolHotKey & 0x100) {
		hotkey += L"Ctrl";
		hasAdd = true;
	}
	if (convertToolHotKey & 0x200) {
		if (hasAdd)
			hotkey += L" + ";
		hotkey += L"Alt";
		hasAdd = true;
	}
	if (convertToolHotKey & 0x400) {
		if (hasAdd)
			hotkey += L" + ";
		hotkey += L"Win";
		hasAdd = true;
	}
	if (convertToolHotKey & 0x800) {
		if (hasAdd)
			hotkey += L" + ";
		hotkey += L"Shift";
		hasAdd = true;
	}

	unsigned short k = ((convertToolHotKey >> 24) & 0xFF);
	if (k != 0xFE) {
		if (hasAdd)
			hotkey += L" + ";
		if (k == VK_SPACE)
			hotkey += L"Space";
		else
			hotkey += (wchar_t)k;
	}

	wstring hotKeyString = menuData[POPUP_QUICK_CONVERT];
	if (hasAdd) {
		hotKeyString += L" - [";
		hotKeyString += hotkey;
		hotKeyString += L"]";
	}
	ModifyMenu(toolsMenu, POPUP_QUICK_CONVERT, MF_BYCOMMAND | MF_UNCHECKED, POPUP_QUICK_CONVERT, hotKeyString.c_str());
}

void SystemTrayHelper::requestUpdate() {
	// Keep synchronous Explorer calls outside the low-level keyboard hook.
	if (nid.hWnd) PostMessage(nid.hWnd, WM_REFRESH_TRAY, 0, 0);
}

static HINSTANCE ins;
static int recreateCount = 0;

void SystemTrayHelper::_createSystemTrayIcon(const HINSTANCE& hIns) {
	ins=hIns;
	HWND hWnd = createFakeWindow(ins);
	
	if (hWnd == NULL) { //Use timer to create
		if (recreateCount >= 5) {
			PostQuitMessage(0);
			return;
		}
		ins = hIns;
		SetTimer(NULL, 0, 1000 * 3, (TIMERPROC)&WaitToCreateFakeWindow);
		++recreateCount;
		return;
	}
	createPopupMenu();

	//create system tray
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = TRAY_ICONUID;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_TRAYMESSAGE;
	loadTrayIcon();
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	iconAdded = false;
	updateData();
}


void CALLBACK SystemTrayHelper::WaitToCreateFakeWindow(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) {
	_createSystemTrayIcon(ins);
	KillTimer(0, timerId);
}

void SystemTrayHelper::createSystemTrayIcon(const HINSTANCE& hIns) {
	_createSystemTrayIcon(hIns);
}

void SystemTrayHelper::removeSystemTray() {
	if (nid.hWnd) {
		KillTimer(nid.hWnd, TRAY_RETRY_TIMER);
		notifyIcon(NIM_DELETE);
		DestroyWindow(nid.hWnd);
	}
	nid = {};
	iconAdded = false;
	if (popupMenu) DestroyMenu(popupMenu);
	popupMenu = menuInputType = codeMenu = settingsMenu = toolsMenu = otherCode = nullptr;
}
