// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#pragma once
#include "stdafx.h"

class VimekManager {
public:
	static unsigned short _lastKeyCode;
	static vector<LPCTSTR>& getInputType();
	static vector<LPCTSTR>& getTableCode();

	static void initEngine();
	static void freeEngine();

	static bool checkUpdate(string& newVersion);

	static void createDesktopShortcut();
};