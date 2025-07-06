#pragma once
#include "../hooks.hpp"
HRESULT __stdcall H::DX9Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();

    HRESULT hr = H::pDX9ResetOG(device, pPresentationParameters);
    
    // actually it still breaks for some reason.
    // TODO: Fix crash after reset

    if (SUCCEEDED(hr)) {
        ImGui_ImplDX9_CreateDeviceObjects();
    }

    return hr;
}
