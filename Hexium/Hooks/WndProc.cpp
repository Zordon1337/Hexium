#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
LRESULT WINAPI H::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (G::menu->IsOpen && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return 1;

	if (H::pWndProcOG)
		return CallWindowProc(H::pWndProcOG, hwnd, msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);

}