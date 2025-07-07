#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
BOOL WINAPI H::hk_SwapBuffers(HDC hdc) {
    if (!G::menu->Initialized && wglGetCurrentContext()) {
        HWND hwnd = WindowFromDC(hdc);
        UIGlobals::hwnd = hwnd;
        pWndProcOG = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
        G::menu->Initialize(hwnd);
    }

    G::menu->Render();

    return H::pSwapBuffersOG(hdc);
}