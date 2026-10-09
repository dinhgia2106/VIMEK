// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#include "AppDelegate.h"
#include "VimekDashboard.h"
#include <Shlobj.h>

static AppDelegate* _instance;

//see document in Engine.h
int vLanguage = 1;
int vInputType = 0;
int vFreeMark = 0;
int vCodeTable = 0;
int vCheckSpelling = 1;
int vUseModernOrthography = 1;
int vQuickTelex = 0;
int vSwitchKeyStatus = VIMEK_DEFAULT_SWITCH_STATUS;
int vRestoreIfWrongSpelling = 1;
int vFixRecommendBrowser = 0;
int vUseMacro = 1;
int vUseMacroInEnglishMode = 1;
int vAutoCapsMacro = 0;
int vSendKeyStepByStep = 1;
int vUseSmartSwitchKey = 1;
int vUpperCaseFirstChar = 0;
int vTempOffSpelling = 0;
int vAllowConsonantZFWJ = 0;
int vQuickStartConsonant = 0;
int vQuickEndConsonant = 0;
int vOtherLanguage = 1;
int vRememberCode = 1;
int vTempOffVimek = 0;

int vUseGrayIcon = 0;
int vShowOnStartUp = 0;
int vRunWithWindows = 0;

int vSupportMetroApp = 1;
int vCreateDesktopShortcut = 0;
int vRunAsAdmin = 0;
int vCheckNewVersion = 0;
//beta feature
int vFixChromiumBrowser = 0; //new on version 2.0

bool AppDelegate::isDialogMsg(MSG & msg) const {
	return (mainDialog != NULL && IsDialogMessage(mainDialog->getHwnd(), &msg)) ||
		(advancedDialog != NULL && IsDialogMessage(advancedDialog->getHwnd(), &msg)) ||
		(macroDialog != NULL && IsDialogMessage(macroDialog->getHwnd(), &msg)) || 
		(convertDialog != NULL && IsDialogMessage(convertDialog->getHwnd(), &msg)) || 
		(aboutDialog != NULL && IsDialogMessage(aboutDialog->getHwnd(), &msg));
}

void AppDelegate::checkUpdate() {
	// Disabled until VIMEK owns a release feed and updater.
}

AppDelegate::AppDelegate() {
	_instance = this;
}

AppDelegate * AppDelegate::getInstance() {
	return _instance;
}

int AppDelegate::run(HINSTANCE hInstance) {
	this->hInstance = hInstance;

	//check app has already run or not
	HWND previousInstance = FindWindow(APP_CLASS, NULL);
	if (previousInstance) {
		MessageBeep(MB_OK);
		SendMessage(previousInstance, WM_USER + 2019, 0, 0);
		PostQuitMessage(0);
		return 0;
	}

	//init Vimek Engine
	VimekManager::initEngine();

	//create system tray
	SystemTrayHelper::createSystemTrayIcon(hInstance);
	SystemTrayHelper::updateData();

	//create main control
	if (vShowOnStartUp)
		createMainDialog();
	MessageBeep(MB_OK);

	//check update
	if (vCheckNewVersion)
		checkUpdate();

	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))	{
		if (msg.message == WM_KEYDOWN) {
			VimekManager::_lastKeyCode = (UINT16)msg.wParam;
		}
		if (!isDialogMsg(msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

void AppDelegate::createMainDialog() {
	if (mainDialog == NULL) {
		mainDialog = new VimekDashboard(hInstance);
		mainDialog->show();
	} else {
		mainDialog->bringOnTop();
	}
}

int AppDelegate::runPreview(HINSTANCE instance) {
	hInstance=instance;preview=true;createMainDialog();
	MSG msg;
	while(GetMessage(&msg,nullptr,0,0)>0){if(!isDialogMsg(msg)){TranslateMessage(&msg);DispatchMessage(&msg);}}
	return 0;
}

void AppDelegate::closeDialog(BaseDialog * dialog) {
	dialog->closeDialog();
	if (mainDialog == dialog) {
		delete mainDialog;
		mainDialog = NULL;
		if(preview) PostQuitMessage(0);
	} else if (advancedDialog == dialog) {
		delete advancedDialog;
		advancedDialog = NULL;
		if (mainDialog) mainDialog->fillData();
	} else if (aboutDialog == dialog) {
		delete aboutDialog;
		aboutDialog = NULL;
	} else if (macroDialog == dialog) {
		delete macroDialog;
		macroDialog = NULL;
	} else if (convertDialog == dialog) {
		delete convertDialog;
		convertDialog = NULL;
	}
}

void AppDelegate::onInputMethodChangedFromHotKey() {
	APP_SET_DATA(vLanguage, vLanguage);
	if (mainDialog) {
		mainDialog->fillData();
	}
	SystemTrayHelper::requestUpdate();
}

void AppDelegate::onDefaultConfig() {
	APP_SET_DATA(vLanguage, 1);
	APP_SET_DATA(vInputType, 0);
	vFreeMark = 0;
	APP_SET_DATA(vCodeTable, 0);
	APP_SET_DATA(vCheckSpelling, 1);
	APP_SET_DATA(vUseModernOrthography, 0);
	APP_SET_DATA(vQuickTelex, 0);
	APP_SET_DATA(vSwitchKeyStatus, VIMEK_DEFAULT_SWITCH_STATUS);
	APP_SET_DATA(vRestoreIfWrongSpelling, 1);
	APP_SET_DATA(vFixRecommendBrowser, 1);
	APP_SET_DATA(vUseMacro, 0);
	APP_SET_DATA(vUseMacroInEnglishMode, 0);
	APP_SET_DATA(vSendKeyStepByStep, 1);
	APP_SET_DATA(vUseSmartSwitchKey, 1);
	APP_SET_DATA(vUpperCaseFirstChar, 0);
	APP_SET_DATA(vAllowConsonantZFWJ, 0);
	APP_SET_DATA(vTempOffSpelling, 0);

	APP_SET_DATA(vUseGrayIcon, 0);
	APP_SET_DATA(vShowOnStartUp, 1);
	APP_SET_DATA(vRunWithWindows, 0);

	APP_SET_DATA(vSupportMetroApp, 1);
	APP_SET_DATA(vRememberCode, 1);
	APP_SET_DATA(vOtherLanguage, 1);
	APP_SET_DATA(vTempOffVimek, 0);
	APP_SET_DATA(vFixChromiumBrowser, 0);

	if (mainDialog) {
		mainDialog->fillData();
	}
	SystemTrayHelper::updateData();
}

void AppDelegate::onToggleVietnamese() {
	APP_SET_DATA(vLanguage, vLanguage ? 0 : 1);
	startNewSession();
	if (vUseSmartSwitchKey) {
		string& exe = VimekHelper::getLastAppExecuteName();
		setAppInputMethodStatus(exe, vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
	if (HAS_BEEP(vSwitchKeyStatus)) MessageBeep(MB_OK);
	if (mainDialog) mainDialog->fillData();
	SystemTrayHelper::requestUpdate();
}

void AppDelegate::onToggleCheckSpelling() {
	APP_SET_DATA(vCheckSpelling, vCheckSpelling ? 0 : 1);
	if (mainDialog) {
		mainDialog->fillData();
	}
	vSetCheckSpelling();
}

void AppDelegate::onToggleUseSmartSwitchKey() {
	APP_SET_DATA(vUseSmartSwitchKey, vUseSmartSwitchKey ? 0 : 1);
	if (mainDialog) {
		mainDialog->fillData();
	}
}

void AppDelegate::onToggleUseMacro() {
	APP_SET_DATA(vUseMacro, vUseMacro ? 0 : 1);
	if (mainDialog) {
		mainDialog->fillData();
	}
}

void AppDelegate::onMacroTable() {
	if (macroDialog == NULL) {
		macroDialog = new MacroDialog(hInstance, IDD_DIALOG_MACRO);
		macroDialog->show();
	} else {
		macroDialog->bringOnTop();
	}
}

void AppDelegate::onConvertTool() {
	if (convertDialog == NULL) {
		convertDialog = new ConvertToolDialog(hInstance, IDD_DIALOG_CONVERT_TOOL);
		convertDialog->show();
	} else {
		convertDialog->bringOnTop();
	}
}

void AppDelegate::onQuickConvert() {
	if (VimekHelper::quickConvert()) {
		//alert when complete
		if (!convertToolDontAlertWhenCompleted) {
			TCHAR msg[256];
			LoadString(hInstance, IDS_STRING_CONVERT_COMPLETED, msg, 256);
			MessageBox(NULL, msg, _T("VIMEK"), MB_OK);
		}
	}
}

void AppDelegate::onInputType(const int & type) {
	if (!vimekIsValidInputMethod(type)) return;
	APP_SET_DATA(vInputType, type);
	startNewSession();
	if (mainDialog) {
		mainDialog->fillData();
	}
}

void AppDelegate::onTableCode(const int & code) {
	APP_SET_DATA(vCodeTable, code);
	if (mainDialog) {
		mainDialog->fillData();
	}
	if (vRememberCode) {
		setAppInputMethodStatus(VimekHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
}

void AppDelegate::onControlPanel() {
	createMainDialog();
}

void AppDelegate::onSwitchShortcut(int status) {
	APP_SET_DATA(vSwitchKeyStatus, status);
	if (advancedDialog) advancedDialog->fillData();
}

void AppDelegate::onRunWithWindows(bool enabled) {
	if (VimekHelper::registerRunOnStartup(enabled ? 1 : 0)) {
		APP_SET_DATA(vRunWithWindows, enabled ? 1 : 0);
	} else {
		MessageBoxW(mainDialog ? mainDialog->getHwnd() : nullptr,
			L"Không thể thay đổi tùy chọn khởi động cùng Windows. Vui lòng thử lại.", L"VIMEK", MB_OK | MB_ICONERROR);
	}
	if (mainDialog) mainDialog->fillData();
	if (advancedDialog) advancedDialog->fillData();
}

void AppDelegate::onRunAsAdmin(bool enabled, HWND owner) {
	APP_SET_DATA(vRunAsAdmin, enabled ? 1 : 0);
	if (enabled && !IsUserAnAdmin()) {
		if (MessageBoxW(owner, L"Khởi động lại VIMEK với quyền Admin ngay bây giờ?\nNếu chọn Không, thay đổi sẽ áp dụng ở lần mở tiếp theo.",
			L"VIMEK", MB_ICONQUESTION | MB_YESNO) == IDYES) {
			// The elevated instance waits for this one to exit before checking
			// for an existing instance. Cancelling UAC leaves VIMEK running.
			wstring arguments = L"--restart-from " + std::to_wstring(GetCurrentProcessId());
			if ((INT_PTR)ShellExecuteW(owner, L"runas", VimekHelper::getFullPath().c_str(), arguments.c_str(), nullptr, SW_SHOWNORMAL) > 32)
				PostQuitMessage(0);
			else {
				APP_SET_DATA(vRunAsAdmin, 0);
				VimekHelper::registerRunOnStartup(vRunWithWindows);
			}
		}
	} else {
		VimekHelper::registerRunOnStartup(false);
		VimekHelper::registerRunOnStartup(vRunWithWindows);
	}
	if (mainDialog) mainDialog->fillData();
	if (advancedDialog) advancedDialog->fillData();
}

void AppDelegate::onAdvancedSettings() {
	if (!advancedDialog) {
		advancedDialog = new MainControlDialog(hInstance, IDD_DIALOG_MAIN);
		advancedDialog->show();
		SetWindowText(advancedDialog->getHwnd(), L"VIMEK · Nâng cao");
	} else {
		advancedDialog->fillData();
		advancedDialog->bringOnTop();
	}
}

void AppDelegate::onVimekAbout() {
	if (aboutDialog == NULL) {
		aboutDialog = new AboutDialog(hInstance, IDD_ABOUTBOX);
		aboutDialog->show();
	} else {
		aboutDialog->bringOnTop();
	}
}

void AppDelegate::onVimekExit() {
	VimekManager::freeEngine();
	SystemTrayHelper::removeSystemTray();
	PostQuitMessage(0);
}
