// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#include "BaseDialog.h"
#include "VimekTheme.h"
static TCHAR tooltipBuff[1024];

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_INITDIALOG) {
#ifdef _WIN64
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
#else
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
#endif
		BaseDialog*b = (BaseDialog*)lParam;
		INT_PTR result = b->eventProc(hDlg, uMsg, wParam, lParam);
		if (!b->hasCustomTheme()) vimekThemeDialog(hDlg);
		return result;
	}
#ifdef _WIN64
	LONG_PTR attr = GetWindowLongPtr(hDlg, GWLP_USERDATA);
#else
	long attr = GetWindowLong(hDlg, GWL_USERDATA);
#endif
	if (attr != 0) {
		return ((BaseDialog*)attr)->eventProc(hDlg, uMsg, wParam, lParam);
	}
	return FALSE;
}

void BaseDialog::createToolTip(const HWND& control, LPCTSTR toolTipString) {
	if (!control || !toolTipString)
		return;

	HWND tip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_ALWAYSTIP  | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, NULL, hInstance, NULL);

	if (!tip)
		return;

	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)control;
	toolInfo.lpszText = (LPTSTR)toolTipString;
	if (!SendMessage(tip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo)) {
		
	}
	SendMessage(tip, TTM_SETMAXTIPWIDTH, 0, 300);
}

void BaseDialog::createToolTip(const HWND& control, const UINT& stringResId) {
	LoadString(hInstance, stringResId, tooltipBuff, 1024);
	createToolTip(control, tooltipBuff);
}

BaseDialog::BaseDialog(const HINSTANCE& hInstance, const int & resourceId) {
	hDlg = nullptr;
	resId = resourceId;
	this->hInstance = hInstance;
}

void BaseDialog::show() {
	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(resId), 0, DialogProc, (LPARAM)this);
	ShowWindow(hDlg, SW_SHOWNORMAL);
}

void BaseDialog::bringOnTop() {
	SetForegroundWindow(hDlg);
	SetActiveWindow(hDlg);
}

HWND BaseDialog::getHwnd() {
	return hDlg;
}

void BaseDialog::closeDialog() {
#ifdef _WIN64
	SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
#else
	SetWindowLong(hDlg, GWL_USERDATA, 0);
#endif
	DestroyWindow(hDlg);
}
