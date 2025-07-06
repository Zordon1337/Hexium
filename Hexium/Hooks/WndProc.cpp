#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
LRESULT WINAPI H::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// TODO: fix keyboard not even calling WndProc
    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();

        if (G::menu->IsOpen && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
            return true; 
        }

    }

    return CallWindowProc(H::pWndProcOG, hwnd, msg, wParam, lParam);
}
