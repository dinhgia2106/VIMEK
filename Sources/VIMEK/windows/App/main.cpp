// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#include "stdafx.h"
#include "AppDelegate.h"
#include "VimekDashboard.h"
#include <Shlobj.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
						_In_opt_ HINSTANCE hPrevInstance,
						_In_ LPWSTR    lpCmdLine,
						_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	if(wcsstr(lpCmdLine,L"--render-previews")) {
		CreateDirectoryW(L"build",nullptr);CreateDirectoryW(L"build/ui",nullptr);
		return VimekDashboard::renderPreview(false,L"build/ui/dashboard-light.png") &&
			VimekDashboard::renderPreview(true,L"build/ui/dashboard-dark.png") ? 0 : 1;
	}
	// UI review without installing global keyboard/mouse hooks or a tray item.
	if(wcsstr(lpCmdLine,L"--preview")) {
		AppDelegate preview;
		return preview.runPreview(hInstance);
	}
	if (wcsncmp(lpCmdLine, L"--restart-from ", 15) == 0) {
		DWORD previousId = wcstoul(lpCmdLine + 15, nullptr, 10);
		HANDLE previous = OpenProcess(SYNCHRONIZE, FALSE, previousId);
		if (previous) {
			DWORD result = WaitForSingleObject(previous, 10000);
			CloseHandle(previous);
			if (result != WAIT_OBJECT_0) return 1;
		}
	}
	
#if NDEBUG
	//check the program is run as administrator mode
	APP_GET_DATA(vRunAsAdmin, 0);
	if (vRunAsAdmin && !IsUserAnAdmin()) {
		//create admin process
		ShellExecute(0, L"runas", VimekHelper::getFullPath().c_str(), 0, 0, SW_SHOWNORMAL);
		return 1;
	}
#endif
	AppDelegate app;
	return app.run(hInstance);
}
