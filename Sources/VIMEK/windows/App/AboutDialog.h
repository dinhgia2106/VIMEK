// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#pragma once
#include "BaseDialog.h"

class AboutDialog :	public BaseDialog {
protected:
	INT_PTR eventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void initDialog();
public:
	AboutDialog(const HINSTANCE & hInstance, const int & resourceId);
	~AboutDialog();
	virtual void fillData() override;
private:
	HWND hUpdateButton;
	void onUpdateButton();
};