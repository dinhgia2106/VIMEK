// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.h"
#include "AppDelegate.h"
#include <iostream>

INT_PTR CALLBACK DialogProc(HWND,UINT,WPARAM,LPARAM);
namespace {
int failures=0,commands=0;
void check(bool condition,const char* message) {
    if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}
}
void pump() {
    MSG message;
    while(PeekMessage(&message,nullptr,0,0,PM_REMOVE)) {
        TranslateMessage(&message);DispatchMessage(&message);
    }
}
LRESULT CALLBACK observeCommands(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR) {
    // Exercise native input without saving test choices to user preferences.
    if(message==WM_COMMAND&&HIWORD(wp)==BN_CLICKED) {++commands;return 0;}
    return DefSubclassProc(window,message,wp,lp);
}
HWND findOption(HWND root,int id) {
    for(HWND child=GetWindow(root,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)) {
        HWND option=GetDlgItem(child,id);
        if(option)return option;
    }
    return nullptr;
}
POINT center(HWND window) {
    RECT rect;GetWindowRect(window,&rect);return {(rect.left+rect.right)/2,(rect.top+rect.bottom)/2};
}
void testAdvancedDialog() {
    AppDelegate app;
    MainControlDialog dialog(GetModuleHandle(nullptr),IDD_DIALOG_MAIN);
    HWND root=CreateDialogParam(GetModuleHandle(nullptr),MAKEINTRESOURCE(IDD_DIALOG_MAIN),nullptr,DialogProc,(LPARAM)&dialog);
    check(root!=nullptr,"Advanced dialog opens");if(!root)return;
    SetWindowPos(root,nullptr,20,20,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    ShowWindow(root,SW_SHOWNOACTIVATE);pump();
    HWND tab=GetDlgItem(root,IDC_TAB_CONTROL);
    const int options[]={IDC_CHECK_SPELLING,IDC_CHECK_USE_MACRO,IDC_CHECK_SHOW_ON_STARTUP,IDC_BUTTON_GO_SOURCE_CODE};
    for(int repeat=0;repeat<5;++repeat) for(int i=0;i<4;++i) {
        RECT item;TabCtrl_GetItemRect(tab,i,&item);
        LPARAM pointOnTab=MAKELPARAM((item.left+item.right)/2,(item.top+item.bottom)/2);
        SendMessage(tab,WM_LBUTTONDOWN,MK_LBUTTON,pointOnTab);
        SendMessage(tab,WM_LBUTTONUP,0,pointOnTab);
        pump();
        check(TabCtrl_GetCurSel(tab)==i,"Mouse click selects the requested tab");
        HWND option=findOption(root,options[i]);
        check(option&&IsWindowVisible(option)&&IsWindowEnabled(option),"Selected page option is visible and enabled");
        if(!option)continue;
        HWND page=GetParent(option);
        POINT point=center(option);
        check(WindowFromPoint(point)==option,"Mouse hit reaches page option instead of tab");
        // Native window DCs must clip children/siblings during real WM_PAINT.
        POINT pagePoint=point;ScreenToClient(page,&pagePoint);
        HDC pageDc=GetDC(page);
        check(!PtVisible(pageDc,pagePoint.x,pagePoint.y),"Page background cannot repaint over its option");ReleaseDC(page,pageDc);
        POINT rootPoint=point;ScreenToClient(root,&rootPoint);
        HDC rootDc=GetDC(root);
        check(!PtVisible(rootDc,rootPoint.x,rootPoint.y),"Dialog background cannot repaint over page controls");ReleaseDC(root,rootDc);
        POINT tabPoint=point;ScreenToClient(tab,&tabPoint);
        HDC tabDc=GetDC(tab);
        check(!PtVisible(tabDc,tabPoint.x,tabPoint.y),"Tab background cannot repaint over selected page");ReleaseDC(tab,tabDc);
        RedrawWindow(root,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);pump();
        InvalidateRect(page,nullptr,TRUE);UpdateWindow(page);pump();
        InvalidateRect(tab,nullptr,TRUE);UpdateWindow(tab);pump();
        check(WindowFromPoint(point)==option,"Option remains reachable after independent repaints");
        SetWindowSubclass(page,observeCommands,1,0);
        int beforeCommands=commands;
        int beforeState=(int)SendMessage(option,BM_GETCHECK,0,0);
        POINT local=point;ScreenToClient(option,&local);
        SendMessage(option,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(local.x,local.y));
        SendMessage(option,WM_LBUTTONUP,0,MAKELPARAM(local.x,local.y));pump();
        check(commands==beforeCommands+1,"Click reaches page command handler");
        if(i<3)check((int)SendMessage(option,BM_GETCHECK,0,0)!=beforeState,"Native checkbox changes when clicked");
        RemoveWindowSubclass(page,observeCommands,1);
        if(i<3)SendMessage(option,BM_SETCHECK,beforeState,0);
    }
    DestroyWindow(root);pump();
}
}
int main() {
    // All shown test windows live on a separate desktop; never switch the user
    // to it, install input hooks or change persisted settings.
    HDESK previous=GetThreadDesktop(GetCurrentThreadId());
    wchar_t name[64];swprintf(name,64,L"VIMEK-UI-tests-%lu",GetCurrentProcessId());
    HDESK desktop=CreateDesktopW(name,nullptr,nullptr,0,DESKTOP_CREATEWINDOW|DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS|DESKTOP_ENUMERATE,nullptr);
    if(!desktop||!SetThreadDesktop(desktop)) {
        std::cerr<<"Cannot create isolated UI test desktop: "<<GetLastError()<<'\n';if(desktop)CloseDesktop(desktop);return 2;
    }
    INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_WIN95_CLASSES|ICC_LINK_CLASS};InitCommonControlsEx(&controls);
    testAdvancedDialog();
    SetThreadDesktop(previous);CloseDesktop(desktop);
    std::cout<<"20 native tab switches, repaints and option clicks; "<<failures<<" failures\n";
    return failures?1:0;
}
