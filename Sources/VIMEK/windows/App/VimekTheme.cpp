// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.h"
#include "VimekTheme.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <algorithm>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

namespace {
const wchar_t* themeProperty = L"Vimek.DialogTheme";
const UINT_PTR subclassId = 0x56494d45;
const COLORREF windowColor = RGB(19,21,25), surfaceColor = RGB(28,32,39);
const COLORREF inkColor = RGB(243,244,247), mutedColor = RGB(153,162,177);
const COLORREF borderColor = RGB(70,78,92);
struct Theme {
    HWND root;
    bool dark = false;
    HBRUSH background = CreateSolidBrush(windowColor);
    HBRUSH surface = CreateSolidBrush(surfaceColor);
    ~Theme() { DeleteObject(background); DeleteObject(surface); }
};
LRESULT CALLBACK themeProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
bool classIs(HWND window, const wchar_t* name) {
    wchar_t actual[64]={}; GetClassNameW(window,actual,64);
    return _wcsicmp(actual,name)==0;
}
int scaled(HWND window, int value) {
    HDC dc=GetDC(window); int dpi=GetDeviceCaps(dc,LOGPIXELSX); ReleaseDC(window,dc);
    return MulDiv(value,dpi,96);
}
void fill(HDC dc, RECT rect, COLORREF color) {
    HBRUSH brush=CreateSolidBrush(color); FillRect(dc,&rect,brush); DeleteObject(brush);
}
void frame(HDC dc, RECT rect, COLORREF color) {
    HBRUSH brush=CreateSolidBrush(color); FrameRect(dc,&rect,brush); DeleteObject(brush);
}
void label(HWND window,HDC dc,const wchar_t* title,RECT& rect,UINT flags,COLORREF color=inkColor) {
    HFONT font=(HFONT)SendMessage(window,WM_GETFONT,0,0);
    HGDIOBJ old=SelectObject(dc,font?font:GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc,TRANSPARENT); SetTextColor(dc,IsWindowEnabled(window)?color:mutedColor);
    DrawTextW(dc,title,-1,&rect,flags); SelectObject(dc,old);
}
void paintButton(HWND window,HDC dc,Theme* theme) {
    RECT rect; GetClientRect(window,&rect);
    wchar_t title[512]; GetWindowTextW(window,title,512);
    DWORD style=(DWORD)GetWindowLongPtr(window,GWL_STYLE);
    int type=style&BS_TYPEMASK, state=(int)SendMessage(window,BM_GETSTATE,0,0);
    // Group boxes sit behind their sibling controls. Their clipped DC paints
    // the remaining interior because the dialog clips out the entire group.
    FillRect(dc,&rect,theme->background);
    bool focused=(GetFocus()==window), pressed=(state&BST_PUSHED)!=0;
    UINT flags=DT_SINGLELINE|DT_VCENTER;
    if (SendMessage(window,WM_QUERYUISTATE,0,0)&UISF_HIDEACCEL) flags|=DT_HIDEPREFIX;
    if (type==BS_GROUPBOX) {
        RECT box=rect; box.top+=scaled(window,7); frame(dc,box,borderColor);
        RECT text=rect; text.left+=scaled(window,10); text.right-=scaled(window,10); text.bottom=scaled(window,17);
        RECT measured=text; label(window,dc,title,measured,DT_CALCRECT|DT_SINGLELINE);
        measured.right+=scaled(window,4); FillRect(dc,&measured,theme->background);
        label(window,dc,title,text,DT_SINGLELINE,mutedColor); return;
    }
    if (type==BS_AUTOCHECKBOX||type==BS_CHECKBOX||type==BS_AUTO3STATE||type==BS_3STATE||type==BS_AUTORADIOBUTTON||type==BS_RADIOBUTTON) {
        bool radio=(type==BS_AUTORADIOBUTTON||type==BS_RADIOBUTTON);
        int size=scaled(window,13);
        RECT mark={rect.left+1,(rect.bottom-size)/2,rect.left+1+size,(rect.bottom+size)/2};
        if (style&BS_LEFTTEXT) { mark.left=rect.right-size-1; mark.right=rect.right-1; rect.right=mark.left-scaled(window,6); }
        else rect.left=mark.right+scaled(window,6);
        HPEN pen=CreatePen(PS_SOLID,1,IsWindowEnabled(window)?borderColor:RGB(53,60,72));
        HGDIOBJ oldPen=SelectObject(dc,pen),oldBrush=SelectObject(dc,theme->surface);
        if(radio) Ellipse(dc,mark.left,mark.top,mark.right,mark.bottom);
        else RoundRect(dc,mark.left,mark.top,mark.right,mark.bottom,scaled(window,3),scaled(window,3));
        SelectObject(dc,oldPen); SelectObject(dc,oldBrush); DeleteObject(pen);
        int check=(int)SendMessage(window,BM_GETCHECK,0,0);
        if(check!=BST_UNCHECKED) {
            InflateRect(&mark,-scaled(window,3),-scaled(window,3));
            if(radio) { HBRUSH b=CreateSolidBrush(IsWindowEnabled(window)?inkColor:mutedColor); oldBrush=SelectObject(dc,b); oldPen=SelectObject(dc,GetStockObject(NULL_PEN)); Ellipse(dc,mark.left,mark.top,mark.right,mark.bottom); SelectObject(dc,oldBrush); SelectObject(dc,oldPen); DeleteObject(b); }
            else if(check==BST_INDETERMINATE) fill(dc,mark,mutedColor);
            else { pen=CreatePen(PS_SOLID,scaled(window,2),IsWindowEnabled(window)?inkColor:mutedColor); oldPen=SelectObject(dc,pen); MoveToEx(dc,mark.left,mark.top+(mark.bottom-mark.top)/2,nullptr); LineTo(dc,mark.left+(mark.right-mark.left)/3,mark.bottom); LineTo(dc,mark.right,mark.top); SelectObject(dc,oldPen); DeleteObject(pen); }
        }
        label(window,dc,title,rect,flags|DT_LEFT);
    } else {
        fill(dc,rect,pressed?RGB(53,60,72):surfaceColor);
        frame(dc,rect,focused||type==BS_DEFPUSHBUTTON?mutedColor:borderColor);
        label(window,dc,title,rect,flags|DT_CENTER);
    }
    if(focused&&!(SendMessage(window,WM_QUERYUISTATE,0,0)&UISF_HIDEFOCUS)) { InflateRect(&rect,-2,-2); DrawFocusRect(dc,&rect); }
}
void paintTab(HWND window,HDC dc,Theme* theme) {
    RECT client; GetClientRect(window,&client); FillRect(dc,&client,theme->background);
    int selected=TabCtrl_GetCurSel(window);
    for(int i=0;i<TabCtrl_GetItemCount(window);++i) {
        RECT rect; TabCtrl_GetItemRect(window,i,&rect);
        if(i==selected) fill(dc,rect,surfaceColor);
        wchar_t title[128]={}; TCITEMW item={}; item.mask=TCIF_TEXT;item.pszText=title;item.cchTextMax=128;
        TabCtrl_GetItem(window,i,&item); label(window,dc,title,rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE,i==selected?inkColor:mutedColor);
        if(i==selected) { RECT line=rect;line.top=line.bottom-scaled(window,2); fill(dc,line,inkColor); }
        if(i==selected&&GetFocus()==window) { InflateRect(&rect,-4,-4); DrawFocusRect(dc,&rect); }
    }
}
void paintCombo(HWND window,HDC dc,Theme* theme) {
    RECT rect; GetClientRect(window,&rect); FillRect(dc,&rect,theme->surface);
    frame(dc,rect,GetFocus()==window?mutedColor:borderColor);
    RECT text=rect; text.left+=scaled(window,6);text.right-=scaled(window,21);
    wchar_t title[256]; GetWindowTextW(window,title,256);
    label(window,dc,title,text,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    int x=rect.right-scaled(window,11),y=(rect.top+rect.bottom)/2;
    HPEN pen=CreatePen(PS_SOLID,1,IsWindowEnabled(window)?inkColor:mutedColor); HGDIOBJ old=SelectObject(dc,pen);
    MoveToEx(dc,x-scaled(window,3),y-scaled(window,1),nullptr); LineTo(dc,x,y+scaled(window,2)); LineTo(dc,x+scaled(window,3),y-scaled(window,1)); SelectObject(dc,old);DeleteObject(pen);
}
BOOL CALLBACK attach(HWND window,LPARAM param) {
    Theme* theme=(Theme*)param;
    // Tab pages are siblings of the tab control. Clip overlapping siblings,
    // and never let a dialog background paint over its own child controls.
    SetWindowLongPtr(window,GWL_STYLE,GetWindowLongPtr(window,GWL_STYLE)|WS_CLIPSIBLINGS);
    // A Win32 group box is a sibling of the controls it surrounds. Leaving
    // it above those controls clips their entire drawing region away.
    if(classIs(window,L"Button")&&(GetWindowLongPtr(window,GWL_STYLE)&BS_TYPEMASK)==BS_GROUPBOX) {
        SetWindowPos(window,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    }
    if(classIs(window,L"#32770")) {
        SetWindowLongPtr(window,GWL_STYLE,GetWindowLongPtr(window,GWL_STYLE)|WS_CLIPCHILDREN);
        SetWindowLongPtr(window,GWL_EXSTYLE,GetWindowLongPtr(window,GWL_EXSTYLE)&~WS_EX_TRANSPARENT);
    }
    SetWindowSubclass(window,themeProc,subclassId,(DWORD_PTR)theme);
    // List view colors are documented independently of the native theme.
    if(classIs(window,WC_LISTVIEWW)) {
        SetWindowTheme(window,theme->dark?L"DarkMode_Explorer":nullptr,nullptr);
        ListView_SetBkColor(window,theme->dark?surfaceColor:GetSysColor(COLOR_WINDOW));
        ListView_SetTextBkColor(window,theme->dark?surfaceColor:GetSysColor(COLOR_WINDOW));
        ListView_SetTextColor(window,theme->dark?inkColor:GetSysColor(COLOR_WINDOWTEXT));
    } else if(classIs(window,L"Edit")||classIs(window,L"ComboBox")||classIs(window,WC_TABCONTROLW)||classIs(window,L"Button")) {
        SetWindowTheme(window,theme->dark?L"":nullptr,theme->dark?L"":nullptr);
        if(classIs(window,L"ComboBox")) {
            COMBOBOXINFO info={sizeof(info)};
            if(GetComboBoxInfo(window,&info)&&info.hwndList) SetWindowSubclass(info.hwndList,themeProc,subclassId,(DWORD_PTR)theme);
        }
    }
    RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME);
    return TRUE;
}
void update(Theme* theme) {
    theme->dark=vimekUsesDarkTheme();
    BOOL dark=theme->dark; DwmSetWindowAttribute(theme->root,20,&dark,sizeof(dark));
    EnumChildWindows(theme->root,attach,(LPARAM)theme);
    RedrawWindow(theme->root,nullptr,nullptr,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN|RDW_FRAME);
}
LRESULT CALLBACK themeProc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR data) {
    Theme* theme=(Theme*)data;
    if(message==WM_NCDESTROY) {
        RemoveWindowSubclass(window,themeProc,subclassId);
        LRESULT result=DefSubclassProc(window,message,wp,lp);
        if(window==theme->root) { RemovePropW(window,themeProperty); delete theme; }
        return result;
    }
    if(window==theme->root&&(message==WM_SETTINGCHANGE||message==WM_THEMECHANGED)) { LRESULT result=DefSubclassProc(window,message,wp,lp);update(theme);return result; }
    if(!theme->dark) return DefSubclassProc(window,message,wp,lp);
    if(message>=WM_CTLCOLORMSGBOX&&message<=WM_CTLCOLORSTATIC) {
        HDC dc=(HDC)wp; HWND control=(HWND)lp;
        SetTextColor(dc,IsWindowEnabled(control)?inkColor:mutedColor);
        bool input=message==WM_CTLCOLOREDIT||message==WM_CTLCOLORLISTBOX||classIs(control,L"Edit");
        SetBkColor(dc,input?surfaceColor:windowColor);SetBkMode(dc,input?OPAQUE:TRANSPARENT);
        return (LRESULT)(input?theme->surface:theme->background);
    }
    bool dialog=classIs(window,L"#32770"),button=classIs(window,L"Button"),tab=classIs(window,WC_TABCONTROLW);
    bool combo=classIs(window,L"ComboBox")&&((GetWindowLongPtr(window,GWL_STYLE)&3)==CBS_DROPDOWNLIST);
    if(message==WM_ERASEBKGND&&(dialog||button||tab||combo)) return TRUE;
    if((message==WM_PAINT||message==WM_PRINTCLIENT)&&(dialog||button||tab||combo)) {
        PAINTSTRUCT ps={}; HDC dc=message==WM_PAINT?BeginPaint(window,&ps):(HDC)wp;
        if(button) paintButton(window,dc,theme);
        else if(tab) paintTab(window,dc,theme);
        else if(combo) paintCombo(window,dc,theme);
        else { RECT rect;GetClientRect(window,&rect);FillRect(dc,&rect,theme->background); }
        if(message==WM_PAINT) EndPaint(window,&ps);return 0;
    }
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    if(message==WM_NCPAINT&&(classIs(window,L"Edit")||classIs(window,WC_LISTVIEWW))) {
        LONG_PTR style=GetWindowLongPtr(window,GWL_STYLE),exStyle=GetWindowLongPtr(window,GWL_EXSTYLE);
        if((style&WS_BORDER)||(exStyle&WS_EX_CLIENTEDGE)) {
            HDC dc=GetWindowDC(window); RECT rect;GetWindowRect(window,&rect);OffsetRect(&rect,-rect.left,-rect.top);
            frame(dc,rect,borderColor);
            if(exStyle&WS_EX_CLIENTEDGE){InflateRect(&rect,-1,-1);frame(dc,rect,surfaceColor);}
            ReleaseDC(window,dc);
        }
    }
    if(button||combo||tab) {
        switch(message) {
        case BM_SETCHECK:case BM_SETSTATE:case WM_ENABLE:case WM_SETFOCUS:case WM_KILLFOCUS:case WM_SETTEXT:
        case WM_LBUTTONDOWN:case WM_LBUTTONUP:case WM_KEYDOWN:case WM_KEYUP:case CB_SETCURSEL:case TCM_SETCURSEL:case WM_UPDATEUISTATE:
            InvalidateRect(window,nullptr,FALSE);break;
        }
    }
    return result;
}
}
bool vimekUsesDarkTheme() {
    HIGHCONTRASTW contrast={sizeof(contrast)};
    if(SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(contrast),&contrast,0)&&(contrast.dwFlags&HCF_HIGHCONTRASTON)) return false;
    if(wcsstr(GetCommandLineW(),L"--preview-dark")) return true;
    if(wcsstr(GetCommandLineW(),L"--preview-light")) return false;
    DWORD light=1,size=sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",L"AppsUseLightTheme",RRF_RT_REG_DWORD,nullptr,&light,&size);
    return !light;
}
void vimekThemeDialog(HWND window) {
    if(!window) return;
    SetWindowLongPtr(window,GWL_STYLE,GetWindowLongPtr(window,GWL_STYLE)|WS_CLIPCHILDREN);
    Theme* theme=(Theme*)GetPropW(window,themeProperty);
    if(!theme) { theme=new Theme;theme->root=window;SetPropW(window,themeProperty,theme);SetWindowSubclass(window,themeProc,subclassId,(DWORD_PTR)theme); }
    update(theme);
}
