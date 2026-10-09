// VIMEK native dashboard. GPL-3.0.
#include "VimekDashboard.h"
#include "AppDelegate.h"
#include <dwmapi.h>
#include <gdiplus.h>
#include <algorithm>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
enum { VI=7000, EN, METHOD0, METHOD1, METHOD2, METHOD3, SPELL, SMART, MACRO, ADVANCED, CONVERT, HOTKEY };
struct Control {int id;const wchar_t* title;int x,y,width,height;};
static const Control controls[]={
    {VI,L"Tiếng Việt",32,200,302,40},{EN,L"English",346,200,302,40},
    {METHOD0,L"Telex",32,288,145,40},{METHOD1,L"VNI",189,288,145,40},
    {METHOD2,L"Simple Telex 1",346,288,145,40},{METHOD3,L"Simple Telex 2",503,288,145,40},
    {HOTKEY,L"Ctrl + Alt",456,387,192,36},{SPELL,L"Bật",558,445,90,36},{SMART,L"Bật",558,494,90,36},
    {MACRO,L"Gõ tắt",32,559,192,36},{CONVERT,L"Chuyển mã",236,559,192,36},{ADVANCED,L"Nâng cao",440,559,208,36}
};
static COLORREF rgb(bool dark, COLORREF light, COLORREF night) { return dark ? night : light; }
VimekDashboard::~VimekDashboard() {
    DeleteObject(bodyFont); DeleteObject(labelFont); DeleteObject(titleFont); DeleteObject(background);
}
void VimekDashboard::updateTheme() {
    DWORD light=1, size=sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    dark = !light;
    if(wcsstr(GetCommandLineW(),L"--preview-dark"))dark=true;
    if(wcsstr(GetCommandLineW(),L"--preview-light"))dark=false;
    DeleteObject(background);
    background = CreateSolidBrush(rgb(dark, RGB(248,249,251), RGB(19,21,25)));
    BOOL value=dark;
    DwmSetWindowAttribute(hDlg, 20, &value, sizeof(value));
    InvalidateRect(hDlg, nullptr, TRUE);
    for (HWND child=GetWindow(hDlg,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)) InvalidateRect(child,nullptr,TRUE);
}
void VimekDashboard::button(int id, const wchar_t* title, int x, int y, int width, int height) {
    HWND child=CreateWindowExW(0,L"BUTTON",title,WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,
        int(x*scale),int(y*scale),int(width*scale),int(height*scale),hDlg,(HMENU)(INT_PTR)id,hInstance,nullptr);
    SendMessage(child,WM_SETFONT,(WPARAM)bodyFont,TRUE);
}
void VimekDashboard::text(HDC dc,const wchar_t* title,int x,int y,int width,int height,HFONT font,COLORREF color) {
    RECT r={int(x*scale),int(y*scale),int((x+width)*scale),int((y+height)*scale)};
    auto old=SelectObject(dc,font); SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);
    DrawTextW(dc,title,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
    SelectObject(dc,old);
}
void VimekDashboard::paint(HDC dc, RECT client) {
    FillRect(dc,&client,background);
    COLORREF ink=rgb(dark,RGB(28,32,39),RGB(243,244,247));
    COLORREF muted=rgb(dark,RGB(98,105,117),RGB(153,162,177));
    COLORREF line=rgb(dark,RGB(222,226,233),RGB(46,51,61));
    text(dc,L"V I M E K",32,20,500,30,labelFont,ink);
    text(dc,L"Bộ gõ của bạn.",32,51,500,26,bodyFont,muted);
    // An outlined glyph, drawn as a native vector rather than a font bitmap.
    Graphics g(dc);g.SetSmoothingMode(SmoothingModeAntiAlias);
    Color color(255,GetRValue(ink),GetGValue(ink),GetBValue(ink));Pen pen(color,1.6f*scale);
    GraphicsPath path;
    PointF v[]={{14,13},{23,13},{32,40},{41,13},{50,13},{36,51},{28,51}};
    PointF e[]={{17,13},{48,13},{48,20},{25,20},{25,29},{45,29},{45,36},{25,36},{25,44},{48,44},{48,51},{17,51}};
    path.AddPolygon(vLanguage?v:e,vLanguage?7:12);
    Matrix m; m.Scale(1.45f*scale,1.45f*scale);m.Translate(24*scale,88*scale,MatrixOrderAppend);path.Transform(&m);g.DrawPath(&pen,&path);
    text(dc,vLanguage?L"Tiếng Việt":L"English",130,105,400,43,titleFont,ink);
    text(dc,vLanguage?L"Sẵn sàng viết điều bạn muốn.":L"Gõ nguyên bản, không thêm dấu.",130,150,500,28,bodyFont,muted);
    text(dc,L"KIỂU GÕ",32,254,600,24,labelFont,muted);
    const wchar_t* sample = vInputType==1?L"Ví dụ: tie6ng1 Vie6t5 → tiếng Việt":L"Ví dụ: tieengs Vieetj → tiếng Việt";
    text(dc,sample,32,333,610,26,bodyFont,muted);
    text(dc,L"Phím chuyển Việt / Anh",32,391,350,28,bodyFont,ink);
    text(dc,L"Kiểm tra chính tả",32,448,350,28,bodyFont,ink);
    text(dc,L"Nhớ chế độ theo ứng dụng",32,497,420,28,bodyFont,ink);
    wchar_t footer[128];const wchar_t* codes[]={L"Unicode",L"TCVN3",L"VNI Windows",L"Unicode tổ hợp",L"CP1258"};
    swprintf(footer,128,L"%s  ·  Tự theo giao diện hệ điều hành",codes[(vCodeTable>=0&&vCodeTable<5)?vCodeTable:0]);
    text(dc,footer,32,620,610,22,bodyFont,muted);
    HPEN p=CreatePen(PS_SOLID,1,line);auto old=SelectObject(dc,p);
    for(int y:{378,434,543,608}){MoveToEx(dc,int(32*scale),int(y*scale),nullptr);LineTo(dc,int(648*scale),int(y*scale));}
    SelectObject(dc,old);DeleteObject(p);
}
void VimekDashboard::drawButton(const DRAWITEMSTRUCT* item, const wchar_t* suppliedLabel) {
    int id=item->CtlID;
    bool active=(id==VI&&vLanguage)||(id==EN&&!vLanguage)||(id>=METHOD0&&id<=METHOD3&&vInputType==id-METHOD0)||
        (id==SPELL&&vCheckSpelling)||(id==SMART&&vUseSmartSwitchKey);
    COLORREF fill=active?rgb(dark,RGB(35,40,49),RGB(237,240,245)):rgb(dark,RGB(255,255,255),RGB(28,32,39));
    if(item->itemState&ODS_SELECTED)fill=rgb(dark,RGB(94,104,120),RGB(92,103,122));
    COLORREF ink=active?rgb(dark,RGB(255,255,255),RGB(26,30,37)):rgb(dark,RGB(48,54,65),RGB(226,231,240));
    HPEN p=CreatePen(PS_SOLID,1,rgb(dark,RGB(218,224,233),RGB(53,60,72)));
    HBRUSH b=CreateSolidBrush(fill);auto oldP=SelectObject(item->hDC,p);auto oldB=SelectObject(item->hDC,b);
    RECT r=item->rcItem;FillRect(item->hDC,&r,background);RoundRect(item->hDC,r.left+1,r.top+1,r.right-1,r.bottom-1,int(12*scale),int(12*scale));
    SelectObject(item->hDC,oldP);SelectObject(item->hDC,oldB);DeleteObject(p);DeleteObject(b);
    wchar_t label[128];if(suppliedLabel)wcsncpy(label,suppliedLabel,127);else GetWindowTextW(item->hwndItem,label,128);label[127]=0;
    if(id==SPELL)wcscpy(label,vCheckSpelling?L"Bật":L"Tắt");
    if(id==SMART)wcscpy(label,vUseSmartSwitchKey?L"Bật":L"Tắt");
    auto old=SelectObject(item->hDC,bodyFont);SetBkMode(item->hDC,TRANSPARENT);SetTextColor(item->hDC,ink);
    DrawTextW(item->hDC,label,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);SelectObject(item->hDC,old);
    if(item->itemState&ODS_FOCUS){InflateRect(&r,-int(4*scale),-int(4*scale));DrawFocusRect(item->hDC,&r);}
}
void VimekDashboard::fillData() {
    if(!hDlg)return;
    SetDlgItemText(hDlg,SPELL,vCheckSpelling?L"Kiểm tra chính tả: Bật":L"Kiểm tra chính tả: Tắt");
    SetDlgItemText(hDlg,SMART,vUseSmartSwitchKey?L"Nhớ chế độ theo ứng dụng: Bật":L"Nhớ chế độ theo ứng dụng: Tắt");
    // Show the saved custom shortcut accurately; clicking restores Ctrl+Alt.
    std::wstring shortcut;
    if(HAS_CONTROL(vSwitchKeyStatus))shortcut+=L"Ctrl + ";
    if(HAS_OPTION(vSwitchKeyStatus))shortcut+=L"Alt + ";
    if(HAS_COMMAND(vSwitchKeyStatus))shortcut+=L"Win + ";
    if(HAS_SHIFT(vSwitchKeyStatus))shortcut+=L"Shift + ";
    if(GET_SWITCH_KEY(vSwitchKeyStatus)!=0xFE){wchar_t name[64]={};GetKeyNameTextW(MapVirtualKeyW(GET_SWITCH_KEY(vSwitchKeyStatus),MAPVK_VK_TO_VSC)<<16,name,64);shortcut+=name;}
    else if(shortcut.size()>=3)shortcut.resize(shortcut.size()-3);
    SetDlgItemText(hDlg,HOTKEY,shortcut.empty()?L"Chưa đặt":shortcut.c_str());
    InvalidateRect(hDlg,nullptr,TRUE);
    for(HWND c=GetWindow(hDlg,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))InvalidateRect(c,nullptr,TRUE);
}
INT_PTR VimekDashboard::eventProc(HWND window,UINT message,WPARAM wp,LPARAM lp) {
    switch(message){
    case WM_INITDIALOG:{
        hDlg=window;
        auto windowDpi=(UINT(WINAPI*)(HWND))GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForWindow");
        HDC windowDc=GetDC(window);
        scale=(windowDpi?windowDpi(window):GetDeviceCaps(windowDc,LOGPIXELSX))/96.f;
        ReleaseDC(window,windowDc);
        RECT desired={0,0,int(680*scale),int(656*scale)};
        AdjustWindowRectEx(&desired,(DWORD)GetWindowLongPtr(window,GWL_STYLE),FALSE,(DWORD)GetWindowLongPtr(window,GWL_EXSTYLE));
        MONITORINFO monitor={sizeof(MONITORINFO)};GetMonitorInfo(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor);
        int width=desired.right-desired.left,height=desired.bottom-desired.top;
        SetWindowPos(window,nullptr,monitor.rcWork.left+(monitor.rcWork.right-monitor.rcWork.left-width)/2,
            monitor.rcWork.top+(monitor.rcWork.bottom-monitor.rcWork.top-height)/2,width,height,SWP_NOZORDER);
        RECT r;GetClientRect(window,&r);scale=float(r.right)/680;
        bodyFont=CreateFontW(-int(15*scale),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        labelFont=CreateFontW(-int(13*scale),0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        titleFont=CreateFontW(-int(34*scale),0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        static ULONG_PTR token=0;if(!token){GdiplusStartupInput input;GdiplusStartup(&token,&input,nullptr);}
        updateTheme();SET_DIALOG_ICON(IDI_APP_ICON);
        for(const auto& c:controls)button(c.id,c.title,c.x,c.y,c.width,c.height);
        createToolTip(GetDlgItem(hDlg,HOTKEY),L"Bấm để dùng Ctrl + Alt. Đặt phím khác trong Nâng cao.");
        fillData();return TRUE;}
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(window,&ps);RECT r;GetClientRect(window,&r);paint(dc,r);EndPaint(window,&ps);return TRUE;}
    case WM_ERASEBKGND:return TRUE;
    case WM_DRAWITEM:drawButton((const DRAWITEMSTRUCT*)lp);return TRUE;
    case WM_CTLCOLORDLG:return (INT_PTR)background;
    case WM_SETTINGCHANGE:case WM_THEMECHANGED:updateTheme();return TRUE;
    case WM_ACTIVATE:if(LOWORD(wp)!=WA_INACTIVE)fillData();break;
    case WM_COMMAND: {
        if(HIWORD(wp)!=BN_CLICKED)break;int id=LOWORD(wp);auto app=AppDelegate::getInstance();
        if(id==VI||id==EN){if(vLanguage!=(id==VI)){app->onToggleVietnamese();startNewSession();}}
        else if(id>=METHOD0&&id<=METHOD3)app->onInputType(id-METHOD0);
        else if(id==SPELL)app->onToggleCheckSpelling();
        else if(id==SMART)app->onToggleUseSmartSwitchKey();
        else if(id==MACRO)app->onMacroTable();
        else if(id==CONVERT)app->onConvertTool();
        else if(id==ADVANCED)app->onAdvancedSettings();
        else if(id==HOTKEY){APP_SET_DATA(vSwitchKeyStatus,VIMEK_DEFAULT_SWITCH_STATUS);}
        else if(id==IDCANCEL){app->closeDialog(this);return TRUE;}
        SystemTrayHelper::updateData();fillData();return TRUE;}
    case WM_CLOSE:AppDelegate::getInstance()->closeDialog(this);return TRUE;
    }return FALSE;
}

bool VimekDashboard::renderPreview(bool night, const wchar_t* path) {
    // Render the same paint/control geometry to an offscreen bitmap. This is
    // a design artifact, not a desktop screenshot or an interaction test.
    GdiplusStartupInput input;ULONG_PTR token;GdiplusStartup(&token,&input,nullptr);
    bool success=false;
    {
        VimekDashboard panel(nullptr);panel.dark=night;panel.scale=1;
        panel.background=CreateSolidBrush(rgb(night,RGB(248,249,251),RGB(19,21,25)));
        panel.bodyFont=CreateFontW(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        panel.labelFont=CreateFontW(-13,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        panel.titleFont=CreateFontW(-34,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        Bitmap bitmap(680,656,PixelFormat32bppARGB);Graphics g(&bitmap);g.Clear(Color::White);HDC dc=g.GetHDC();
        RECT client={0,0,680,656};panel.paint(dc,client);
        for(const auto& c:controls){DRAWITEMSTRUCT item={};item.CtlID=c.id;item.hDC=dc;item.rcItem={c.x,c.y,c.x+c.width,c.y+c.height};panel.drawButton(&item,c.title);}
        g.ReleaseHDC(dc);
        const CLSID png={0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
        success=bitmap.Save(path,&png,nullptr)==Ok;
    }
    GdiplusShutdown(token);return success;
}
