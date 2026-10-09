// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.h"
#include "AppDelegate.h"
#include "VimekTheme.h"
#include "VimekDashboard.h"
#include "../Sources/VIMEK/engine/ModifierShortcut.h"
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

class DashboardActions : public AppDelegate {
public:
    int shortcuts=0,admin=0,startup=0;
    HWND adminOwner=nullptr;
    // Replace OS effects only: native button dispatch and shortcut selection
    // execute unchanged, without writing preferences, startup entries or UAC.
    void onSwitchShortcut(int status) override {++shortcuts;vSwitchKeyStatus=status;}
    void onRunWithWindows(bool enabled) override {++startup;vRunWithWindows=enabled;}
    void onRunAsAdmin(bool enabled,HWND owner) override {++admin;vRunAsAdmin=enabled;adminOwner=owner;}
};
bool captionIs(HWND control,const wchar_t* expected) {
    wchar_t text[128]={};GetWindowTextW(control,text,128);return wcscmp(text,expected)==0;
}
void testDashboard() {
    DashboardActions app;
    VimekDashboard dialog(GetModuleHandle(nullptr));
    HWND root=CreateDialogParam(GetModuleHandle(nullptr),MAKEINTRESOURCE(5000),nullptr,DialogProc,(LPARAM)&dialog);
    check(root!=nullptr,"Dashboard opens");if(!root)return;
    ShowWindow(root,SW_SHOWNORMAL);pump();
    HWND hotkey=GetDlgItem(root,7011),admin=GetDlgItem(root,7006),startup=GetDlgItem(root,7007);
    int savedShortcut=vSwitchKeyStatus,savedAdmin=vRunAsAdmin,savedStartup=vRunWithWindows;
    int spelling=vCheckSpelling,smart=vUseSmartSwitchKey;
    for(unsigned sound:{0u,0x8000u}) {
        vSwitchKeyStatus=int(0xFE0003FEu|sound);dialog.fillData();
        SendMessage(hotkey,BM_CLICK,0,0);pump();
        check(captionIs(hotkey,L"Ctrl + Shift"),"Shortcut click selects Ctrl+Shift");
        check(unsigned(vSwitchKeyStatus)==(0xFE0009FEu|sound),"Shortcut saves Ctrl+Shift and retains sound preference");
        VimekModifierShortcut chord;
        unsigned required=(unsigned(vSwitchKeyStatus)>>8)&15;
        check(!chord.update(1,required)&&!chord.update(9,required)&&chord.update(1,required)&&
            !chord.update(9,required)&&chord.update(1,required),"Selected shortcut supports holding Ctrl and tapping Shift repeatedly");
        SendMessage(hotkey,BM_CLICK,0,0);pump();
        check(captionIs(hotkey,L"Ctrl + Alt")&&unsigned(vSwitchKeyStatus)==(0xFE0003FEu|sound),"Next click saves Ctrl+Alt, retaining sound preference");
    }
    vSwitchKeyStatus=0x5A00855A;dialog.fillData();
    SendMessage(hotkey,BM_CLICK,0,0);pump();
    check(unsigned(vSwitchKeyStatus)==0xFE0083FEu,"Custom shortcut selects Ctrl+Alt without retaining its letter or Win key");
    check(app.shortcuts==5,"Every shortcut click reaches the preference action once");
    vRunAsAdmin=vRunWithWindows=0;dialog.fillData();
    SendMessage(admin,BM_CLICK,0,0);pump();
    check(vRunAsAdmin==1&&app.adminOwner==root&&captionIs(admin,L"Chạy với quyền Admin: Bật"),"Admin toggle enables admin through the shared action");
    SendMessage(startup,BM_CLICK,0,0);pump();
    check(vRunWithWindows==1&&captionIs(startup,L"Khởi động cùng Windows: Bật"),"Startup toggle enables startup through the shared action");
    SendMessage(admin,BM_CLICK,0,0);SendMessage(startup,BM_CLICK,0,0);pump();
    check(!vRunAsAdmin&&!vRunWithWindows&&app.admin==2&&app.startup==2,"Both system toggles turn off on the next click");
    check(vCheckSpelling==spelling&&vUseSmartSwitchKey==smart,"Dashboard system toggles leave spelling and app-mode preferences intact");
    vRunAsAdmin=vRunWithWindows=1;vSwitchKeyStatus=int(0xFE0009FEu);dialog.fillData();
    check(captionIs(admin,L"Chạy với quyền Admin: Bật")&&captionIs(startup,L"Khởi động cùng Windows: Bật")&&
        captionIs(hotkey,L"Ctrl + Shift"),"Dashboard refresh reflects changes made in advanced settings");
    vSwitchKeyStatus=savedShortcut;vRunAsAdmin=savedAdmin;vRunWithWindows=savedStartup;
    DestroyWindow(root);pump();
}
POINT center(HWND window) {
    RECT rect;GetWindowRect(window,&rect);return {(rect.left+rect.right)/2,(rect.top+rect.bottom)/2};
}
HWND enclosingGroup(HWND root,POINT point) {
    for(HWND child=GetWindow(root,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)) {
        wchar_t name[32]={};GetClassNameW(child,name,32);
        if(_wcsicmp(name,L"Button")||(GetWindowLongPtr(child,GWL_STYLE)&BS_TYPEMASK)!=BS_GROUPBOX)continue;
        RECT rect;GetWindowRect(child,&rect);
        if(PtInRect(&rect,point))return child;
    }
    return nullptr;
}
void checkGroupedControls(HWND root,const int* ids,int count) {
    for(int i=0;i<count;++i) {
        HWND control=GetDlgItem(root,ids[i]);
        check(control&&IsWindowVisible(control),"Grouped control exists and is visible");
        if(!control)continue;
        POINT point=center(control);
        HWND group=enclosingGroup(root,point);
        check(group!=nullptr,"Control lies inside its visual group");
        if(!group)continue;
        POINT local=point;ScreenToClient(control,&local);
        HDC dc=GetDC(control);
        check(PtVisible(dc,local.x,local.y)!=FALSE,"Group box cannot clip out its control");ReleaseDC(control,dc);
        local=point;ScreenToClient(group,&local);dc=GetDC(group);
        check(!PtVisible(dc,local.x,local.y),"Group repaint excludes controls in front of it");ReleaseDC(group,dc);
        // Disabled options intentionally do not receive mouse hits.
        if(IsWindowEnabled(control))check(WindowFromPoint(point)==control,"Grouped control receives mouse hit testing");
    }
    if(vimekUsesDarkTheme()) {
        HWND group=enclosingGroup(root,center(GetDlgItem(root,ids[0])));
        if(!group)return;
        RECT rect;GetClientRect(group,&rect);
        HDC screen=GetDC(group),memory=CreateCompatibleDC(screen);
        HBITMAP bitmap=CreateCompatibleBitmap(screen,rect.right,rect.bottom);
        HGDIOBJ old=SelectObject(memory,bitmap);
        FillRect(memory,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
        SendMessage(group,WM_PRINTCLIENT,(WPARAM)memory,PRF_CLIENT|PRF_ERASEBKGND);
        check(GetPixel(memory,3,rect.bottom/2)==RGB(19,21,25),"Group paints its interior with the dark background");
        SelectObject(memory,old);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(group,screen);
    }
}
void testCommonControls(HWND root) {
    const int ids[]={IDC_COMBO_INPUT_TYPE,IDC_COMBO_TABLE_CODE,IDC_CHECK_SWITCH_KEY_CTRL,
        IDC_CHECK_SWITCH_KEY_ALT,IDC_CHECK_SWITCH_KEY_WIN,IDC_CHECK_SWITCH_KEY_SHIFT,
        IDC_SWITCH_KEY_KEY,IDC_CHECK_SWITCH_KEY_BEEP,IDC_RADIO_METHOD_VIETNAMESE,IDC_RADIO_METHOD_ENGLISH};
    for(int repeat=0;repeat<3;++repeat) {
        if(repeat==1) {
            HWND group=enclosingGroup(root,center(GetDlgItem(root,ids[0])));
            RedrawWindow(group,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
        } else if(repeat==2) {
            SendMessage(root,WM_SETTINGCHANGE,0,0);SendMessage(root,WM_THEMECHANGED,0,0);
        }
        RedrawWindow(root,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW);pump();
        checkGroupedControls(root,ids,sizeof(ids)/sizeof(ids[0]));
    }
    SetWindowSubclass(root,observeCommands,1,0);
    for(int id:{IDC_CHECK_SWITCH_KEY_CTRL,IDC_CHECK_SWITCH_KEY_BEEP}) {
        HWND control=GetDlgItem(root,id);
        int beforeCommands=commands,beforeState=(int)SendMessage(control,BM_GETCHECK,0,0);
        RECT rect;GetClientRect(control,&rect);LPARAM point=MAKELPARAM(rect.right/2,rect.bottom/2);
        SendMessage(control,WM_LBUTTONDOWN,MK_LBUTTON,point);SendMessage(control,WM_LBUTTONUP,0,point);pump();
        check(commands==beforeCommands+1,"Grouped checkbox click reaches the dialog");
        check((int)SendMessage(control,BM_GETCHECK,0,0)!=beforeState,"Grouped checkbox changes state");
        SendMessage(control,BM_SETCHECK,beforeState,0);
    }
    RemoveWindowSubclass(root,observeCommands,1);
    for(int id:{IDC_COMBO_INPUT_TYPE,IDC_COMBO_TABLE_CODE}) {
        HWND combo=GetDlgItem(root,id);
        check(SendMessage(combo,CB_GETCOUNT,0,0)>1,"Grouped combo contains its options");
        SendMessage(combo,CB_SHOWDROPDOWN,TRUE,0);pump();
        check(SendMessage(combo,CB_GETDROPPEDSTATE,0,0)!=FALSE,"Grouped combo opens its option list");
        SendMessage(combo,CB_SHOWDROPDOWN,FALSE,0);pump();
    }
}
void testAdvancedDialog() {
    AppDelegate app;
    MainControlDialog dialog(GetModuleHandle(nullptr),IDD_DIALOG_MAIN);
    HWND root=CreateDialogParam(GetModuleHandle(nullptr),MAKEINTRESOURCE(IDD_DIALOG_MAIN),nullptr,DialogProc,(LPARAM)&dialog);
    check(root!=nullptr,"Advanced dialog opens");if(!root)return;
    SetWindowPos(root,nullptr,20,20,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    ShowWindow(root,SW_SHOWNOACTIVATE);pump();
    testCommonControls(root);
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
        if(i==3) {
            RECT pageRect;GetClientRect(page,&pageRect);
            for(HWND child=GetWindow(page,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)) {
                RECT bounds;GetWindowRect(child,&bounds);MapWindowPoints(nullptr,page,(POINT*)&bounds,2);
                check(bounds.left>=0&&bounds.top>=0&&bounds.right<=pageRect.right&&bounds.bottom<=pageRect.bottom,
                    "Information page content fits entirely inside its host");
            }
            RECT buttonRect;GetClientRect(option,&buttonRect);
            wchar_t title[128]={};GetWindowTextW(option,title,128);
            HDC textDc=GetDC(option);
            HGDIOBJ old=SelectObject(textDc,(HFONT)SendMessage(option,WM_GETFONT,0,0));
            RECT measured={};DrawTextW(textDc,title,-1,&measured,DT_CALCRECT|DT_SINGLELINE);
            check(measured.right+8<=buttonRect.right,"License button has room for its complete caption");
            check(PtVisible(textDc,buttonRect.right-2,buttonRect.bottom/2)!=FALSE,"License button right edge is not clipped");
            SelectObject(textDc,old);ReleaseDC(option,textDc);
        }
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
void testConvertGroups() {
    AppDelegate app;
    ConvertToolDialog dialog(GetModuleHandle(nullptr),IDD_DIALOG_CONVERT_TOOL);
    HWND root=CreateDialogParam(GetModuleHandle(nullptr),MAKEINTRESOURCE(IDD_DIALOG_CONVERT_TOOL),nullptr,DialogProc,(LPARAM)&dialog);
    check(root!=nullptr,"Conversion dialog opens");if(!root)return;
    SetWindowPos(root,nullptr,20,20,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    ShowWindow(root,SW_SHOWNOACTIVATE);pump();
    const int ids[]={IDC_CHECK_All_CAPS,IDC_CHECK_NON_ALL_CAPS,IDC_CHECK_REMOVE_MARK,
        IDC_CHECK_CONVERT_CLIPBOARD,IDC_CHECK_SWITCH_KEY_CTRL,IDC_SWITCH_KEY_KEY,
        IDC_COMBO_TABLE_CODE_SRC,IDC_COMBO_TABLE_CODE_DST};
    checkGroupedControls(root,ids,sizeof(ids)/sizeof(ids[0]));
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
    testConvertGroups();
    testDashboard();
    SetThreadDesktop(previous);CloseDesktop(desktop);
    std::cout<<"Dashboard shortcuts and system toggles, grouped controls and 20 native tab switches; "<<failures<<" failures\n";
    return failures?1:0;
}
