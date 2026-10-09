// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#pragma once
#include "stdafx.h"
#include "MainControlDialog.h"
#include "AboutDialog.h"
#include "ConvertToolDialog.h"
#include "MacroDialog.h"

class BaseDialog;
class AppDelegate {
private:
	HINSTANCE hInstance;
	bool preview = false;
	BaseDialog* mainDialog = NULL, *aboutDialog = NULL, *macroDialog = NULL, *convertDialog = NULL, *advancedDialog = NULL;
private:
	bool isDialogMsg(MSG &msg) const;
	void checkUpdate();
public:
	AppDelegate();
	virtual ~AppDelegate() = default;
	static AppDelegate* getInstance();
	int run(HINSTANCE hInstance);
	int runPreview(HINSTANCE hInstance);
	void createMainDialog();
	void closeDialog(BaseDialog* dialog);
public: //event
	void onInputMethodChangedFromHotKey();
	void onDefaultConfig();

	void onToggleVietnamese();
	void onToggleCheckSpelling();
	void onToggleUseSmartSwitchKey();
	void onToggleUseMacro();
	virtual void onSwitchShortcut(int status);
	virtual void onRunWithWindows(bool enabled);
	virtual void onRunAsAdmin(bool enabled, HWND owner);

	void onMacroTable();
	void onConvertTool();
	void onQuickConvert();

	void onInputType(const int& type);
	void onTableCode(const int& code);

	void onControlPanel();
	void onAdvancedSettings();
	void onVimekAbout();
	void onVimekExit();
};
