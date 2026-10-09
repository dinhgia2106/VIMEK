// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#include "VimekManager.h"
#include <shlobj.h>

static vector<LPCTSTR> _inputType = {
#define VIMEK_METHOD_NAME(id, name) _T(name),
	VIMEK_INPUT_METHOD_LIST(VIMEK_METHOD_NAME)
#undef VIMEK_METHOD_NAME
};

static vector<LPCTSTR> _tableCode = {
	_T("Unicode"),
	_T("TCVN3 (ABC)"),
	_T("VNI Windows"),
	_T("Unicode Tổ hợp"),
	_T("Vietnamese Locale CP 1258")
};

/*-----------------------------------------------------------------------*/

extern void VimekInit();
extern void VimekFree();

unsigned short  VimekManager::_lastKeyCode = 0;

vector<LPCTSTR>& VimekManager::getInputType() {
	return _inputType;
}

vector<LPCTSTR>& VimekManager::getTableCode() {
	return _tableCode;
}

void VimekManager::initEngine() {
	VimekInit();
}

void VimekManager::freeEngine() {
	VimekFree();
}

bool VimekManager::checkUpdate(string& newVersion) {
	// VIMEK has no release feed yet; never fetch or install Vimek updates.
	newVersion.clear();
	return false;
}

void VimekManager::createDesktopShortcut() {
	CoInitialize(NULL);
	IShellLink* pShellLink = NULL;
	HRESULT hres;
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL,
							IID_IShellLink, (void**)&pShellLink);
	if (SUCCEEDED(hres)) {
		wstring path = VimekHelper::getFullPath();
		pShellLink->SetPath(path.c_str());
		pShellLink->SetDescription(_T("VIMEK - Bộ gõ Tiếng Việt"));
		pShellLink->SetIconLocation(path.c_str(), 0);

		IPersistFile* pPersistFile;
		hres = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

		if (SUCCEEDED(hres)) {
			wchar_t desktopPath[MAX_PATH + 1];
			wchar_t savePath[MAX_PATH + 10];
			SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath);
			wsprintf(savePath, _T("%s\\VIMEK.lnk"), desktopPath);
			hres = pPersistFile->Save(savePath, TRUE);
			pPersistFile->Release();
			pShellLink->Release();
		}
	}
}
