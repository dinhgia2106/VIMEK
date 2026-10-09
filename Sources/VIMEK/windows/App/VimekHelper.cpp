// VIMEK — Vietnamese input method.
// Copyright (C) 2019 Mai Vu Tuyen
// SPDX-License-Identifier: GPL-3.0-only
// Upstream attribution and modifications: see NOTICE.md.
#include "VimekHelper.h"
#include <stdarg.h>
#include <Shlobj.h>

#include <fstream>
#include <sstream>

#pragma comment(lib, "version.lib")


static BYTE* _regData = 0;

static LPCTSTR sk = TEXT("SOFTWARE\\VIMEK");
static HKEY hKey;
static LPCTSTR _runOnStartupKeyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
static TCHAR _executePath[MAX_PATH];
static bool _hasGetPath = false;

static DWORD _cacheProcessId = 0;
static string _foregroundExeName;
static string _exeNameUtf8 = "TheVIMEKProject";
static string _unknownProgram = "UnknownProgram";

int CF_RTF = RegisterClipboardFormat(_T("Rich Text Format"));
int CF_HTML = RegisterClipboardFormat(_T("HTML Format"));
int CF_VIMEK = RegisterClipboardFormat(_T("VIMEK Format"));

void VimekHelper::openRegistryKey() {
	LONG nError = RegOpenKeyEx(HKEY_CURRENT_USER, sk, NULL, KEY_ALL_ACCESS, &hKey);
	if (nError == ERROR_FILE_NOT_FOUND) 	{
		nError = RegCreateKeyEx(HKEY_CURRENT_USER, sk, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &hKey, NULL);
	}
	if (nError) {
		LOG(L"result %d\n", nError);
	}
}

void VimekHelper::setRegInt(LPCTSTR key, const int & val) {
	openRegistryKey();
	RegSetValueEx(hKey, key, 0, REG_DWORD, (LPBYTE)&val, sizeof(val));
	RegCloseKey(hKey);
}

int VimekHelper::getRegInt(LPCTSTR key, const int & defaultValue) {
	openRegistryKey();
	int val = defaultValue;
	DWORD size = sizeof(val);
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, key, 0, 0, (LPBYTE)&val, &size)) {
		val = defaultValue;
	}
	RegCloseKey(hKey);
	return val;
}

void VimekHelper::setRegBinary(LPCTSTR key, const BYTE * pData, const int & size) {
	openRegistryKey();
	RegSetValueEx(hKey, key, 0, REG_BINARY, pData, size);
	RegCloseKey(hKey);
}

BYTE * VimekHelper::getRegBinary(LPCTSTR key, DWORD& outSize) {
	openRegistryKey();
	if (_regData) {
		delete[] _regData;
		_regData = NULL;
	}
	DWORD size = 0;
	RegQueryValueEx(hKey, key, 0, 0, 0, &size);
	_regData = new BYTE[size];
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, key, 0, 0, _regData, &size)) {
		delete[] _regData;
		_regData = NULL;
	}
	outSize = size;
	RegCloseKey(hKey);
	return _regData;
}

static bool runStartupTask(const wstring& arguments) {
	wchar_t systemDirectory[MAX_PATH] = {};
	GetSystemDirectoryW(systemDirectory, MAX_PATH);
	wstring executable = wstring(systemDirectory) + L"\\schtasks.exe";
	wstring command = L"\"" + executable + L"\" " + arguments;
	STARTUPINFOW startup = {}; startup.cb = sizeof(startup);
	PROCESS_INFORMATION process = {};
	if (CreateProcessW(executable.c_str(), &command[0], nullptr, nullptr, FALSE,
		CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) {
		// Finish deletion before creating a replacement task.
		DWORD result = WaitForSingleObject(process.hProcess, 10000), exitCode = 1;
		if (result == WAIT_OBJECT_0) GetExitCodeProcess(process.hProcess, &exitCode);
		CloseHandle(process.hThread); CloseHandle(process.hProcess);
		return result == WAIT_OBJECT_0 && exitCode == 0;
	}
	return false;
}

bool VimekHelper::registerRunOnStartup(const int& val) {
	HKEY runKey = nullptr;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, _runOnStartupKeyPath, 0, nullptr, 0, KEY_SET_VALUE,
		nullptr, &runKey, nullptr) != ERROR_SUCCESS) return false;
	// A task created by an elevated process avoids a UAC prompt at each logon.
	// If elevation was deferred, the Run entry launches VIMEK normally and the
	// app requests UAC at startup, just as it does on a manual launch.
	bool taskCreated = false;
	if (val && vRunAsAdmin && IsUserAnAdmin()) {
		// Preserve Unicode and quote the executable inside the task argument.
		taskCreated = runStartupTask(L"/create /sc onlogon /tn VIMEK /rl highest /it /tr \"\\\"" + getFullPath() + L"\\\"\" /f");
	}
	LONG result;
	if (val && !taskCreated) {
		wstring path = L"\"" + getFullPath() + L"\"";
		result = RegSetValueExW(runKey, L"VIMEK", 0, REG_SZ, reinterpret_cast<const BYTE*>(path.c_str()), DWORD((path.size() + 1) * sizeof(wchar_t)));
	} else {
		result = RegDeleteValueW(runKey, L"VIMEK");
		if (result == ERROR_FILE_NOT_FOUND) result = ERROR_SUCCESS;
	}
	RegCloseKey(runKey);
	if (!val) runStartupTask(L"/delete /tn VIMEK /f");
	return result == ERROR_SUCCESS;
}

LPTSTR VimekHelper::getExecutePath() {
	if (!_hasGetPath) {
		HMODULE hModule = GetModuleHandleW(NULL);
		GetModuleFileNameW(hModule, _executePath, MAX_PATH);
		_hasGetPath = true;
	}
	return _executePath;
}

string& VimekHelper::getForegroundAppExecuteName() {
	DWORD processId = 0;
	GetWindowThreadProcessId(GetForegroundWindow(), &processId);
	if (!processId) return _unknownProgram;
	if (processId == _cacheProcessId) return _foregroundExeName;
	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (!process) return _unknownProgram;
	wchar_t path[32768] = {};
	DWORD length = sizeof(path) / sizeof(path[0]);
	BOOL found = QueryFullProcessImageNameW(process, 0, path, &length);
	CloseHandle(process);
	if (!found) return _unknownProgram;
	const wchar_t* name = wcsrchr(path, L'\\');
	name = name ? name + 1 : path;
	int bytes = WideCharToMultiByte(CP_UTF8, 0, name, -1, nullptr, 0, nullptr, nullptr);
	if (bytes <= 1) return _unknownProgram;
	string result(bytes, '\0');
	WideCharToMultiByte(CP_UTF8, 0, name, -1, &result[0], bytes, nullptr, nullptr);
	result.pop_back();
	_foregroundExeName = result;
	_cacheProcessId = processId;
	return _foregroundExeName;
}

string& VimekHelper::getFrontMostAppExecuteName() {
	string& foreground = getForegroundAppExecuteName();
	if (foreground == _unknownProgram) return _unknownProgram;
	// Tray and VIMEK windows retain the last input app for explicit UI choices,
	// but must remain identifiable to the foreground-change handler.
	if (_cacheProcessId != GetCurrentProcessId() && _stricmp(foreground.c_str(), "explorer.exe") != 0)
		_exeNameUtf8 = foreground;
	return _exeNameUtf8;
}

string & VimekHelper::getLastAppExecuteName() {
	if (!vUseSmartSwitchKey)
		return getFrontMostAppExecuteName();
	return _exeNameUtf8;
}

wstring VimekHelper::getFullPath() {
	HMODULE hModule = GetModuleHandle(NULL);
	TCHAR path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);
	wstring rs(path);
	return rs;
}

wstring VimekHelper::getClipboardText(const int& type) {
	// Try opening the clipboard
	if (!OpenClipboard(nullptr)) {
		return _T("");
	}

	// Get handle of clipboard object for ANSI text
	HANDLE hData = GetClipboardData(type);
	if (hData == nullptr) {
		return _T("");
	}

	// Lock the handle to get the actual text pointer
	wchar_t * pszText = static_cast<wchar_t*>(GlobalLock(hData));
	if (pszText == nullptr) {
		return _T("");
	}

	// Save text in a string class instance
	wstring text(pszText);

	// Release the lock
	GlobalUnlock(hData);

	// Release the clipboard
	CloseClipboard();

	return text;
}

void VimekHelper::setClipboardText(LPCTSTR data, const int & len, const int& type) {
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(WCHAR));
	memcpy(GlobalLock(hMem), data, len * sizeof(WCHAR));
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(type, hMem);
	CloseClipboard();
}

bool VimekHelper::quickConvert() {
	//read data from clipboard
	//support Unicode raw string, Rich Text Format and HTML

	if (!OpenClipboard(nullptr)) {
		return false;
	}

	string dataHTML, dataRTF;
	wstring dataUnicode;

	char* pHTML = 0, pRTF = 0;
	wchar_t* pUnicode = 0;

	//HTML
	HANDLE hData = GetClipboardData(CF_HTML);
	if (hData) {
		pHTML = static_cast<char*>(GlobalLock(hData));
		GlobalUnlock(hData);
	}
	if (pHTML) {
		dataHTML = pHTML;
		dataHTML = convertUtil(dataHTML);
	}

	//UNICODE
	hData = GetClipboardData(CF_UNICODETEXT);
	if (hData) {
		pUnicode = static_cast<wchar_t*>(GlobalLock(hData));
		GlobalUnlock(hData);
	}
	if (pUnicode) {
		dataUnicode = pUnicode;
		dataUnicode = utf8ToWideString(convertUtil(wideStringToUtf8(dataUnicode)));
	}

	OpenClipboard(0);
	EmptyClipboard();

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (int)(dataHTML.size() + 1) * sizeof(char));
	memcpy(GlobalLock(hMem), dataHTML.c_str(), (int)(dataHTML.size() + 1) * sizeof(char));
	GlobalUnlock(hMem);
	SetClipboardData(CF_HTML, hMem);

	hMem = GlobalAlloc(GMEM_MOVEABLE, (int)(dataUnicode.size() + 1) * sizeof(wchar_t));
	memcpy(GlobalLock(hMem), dataUnicode.c_str(), (int)(dataUnicode.size() + 1) * sizeof(wchar_t));
	GlobalUnlock(hMem);
	SetClipboardData(CF_UNICODETEXT, hMem);

	CloseClipboard();
	return true;
}

DWORD VimekHelper::getVersionNumber() {
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
		return 0;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	UINT dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0) {
		return 0;
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
		return 0;
	}

	LPBYTE lpBuffer = NULL;

	if (VerQueryValue(&data[0], _T("\\"), (VOID FAR * FAR*) & lpBuffer, &dwSize)) {
		if (dwSize) {
			VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
			if (verInfo->dwSignature == 0xfeef04bd) {
				return ((verInfo->dwFileVersionMS >> 16) & 0xffff) |
					(((verInfo->dwFileVersionMS >> 0) & 0xffff) << 8) |
					(((verInfo->dwFileVersionLS >> 16) & 0xffff) << 16);
			}
		}
	}

	return 0;
}

wstring VimekHelper::getVersionString() {
	TCHAR versionBuffer[MAX_PATH];
	DWORD ver = getVersionNumber();
	wsprintfW(versionBuffer, _T("%d.%d.%d"), ver & 0xFF, (ver>>8) & 0xFF, (ver >> 16) & 0xFF);
	return wstring(versionBuffer);

	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
		return _T("");
	}
}
