#pragma once

#include "../hooks.hpp"

HRESULT __stdcall H::EndScene(IDirect3DDevice9* device) {
    if (!G::hooksInitialized) return H::pEndSceneOG(device);

    if (!G::menu->Initialized) {
        auto params = D3DDEVICE_CREATION_PARAMETERS{ };
        device->GetCreationParameters(&params);

        pWndProcOG = (WNDPROC)SetWindowLongPtr(params.hFocusWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
        G::menu->Initialize(params.hFocusWindow);

        ImGui_ImplDX9_Init(device);

        UIGlobals::pDevice = device;
    }

    G::menu->Render();

    return H::pEndSceneOG(device);
}
