// VIMEK native dashboard. GPL-3.0.
#pragma once
#include "BaseDialog.h"
class VimekDashboard : public BaseDialog {
    bool dark = false;
    float scale = 1;
    HFONT bodyFont = nullptr, labelFont = nullptr;
    HBRUSH background = nullptr;
    void updateTheme();
    void paint(HDC dc, RECT client);
    void button(int id, const wchar_t* title, int x, int y, int width, int height);
    void text(HDC dc, const wchar_t* title, int x, int y, int width, int height, HFONT font, COLORREF color);
    void drawButton(const DRAWITEMSTRUCT* item, const wchar_t* label = nullptr);
    INT_PTR eventProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) override;
    bool hasCustomTheme() const override { return true; }
public:
    explicit VimekDashboard(HINSTANCE instance) : BaseDialog(instance, 5000) {}
    ~VimekDashboard() override;
    void fillData() override;
    static bool renderPreview(bool dark, const wchar_t* path);
};
