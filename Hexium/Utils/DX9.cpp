#include "DX9.hpp"

namespace DX9 {
    IDirect3DDevice9* GetD3D9Device() {

        IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D) return nullptr;

        D3DPRESENT_PARAMETERS d3dpp = {};
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = GetForegroundWindow();

        IDirect3DDevice9* pDevice = nullptr;
        HRESULT hr = pD3D->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            d3dpp.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &d3dpp,
            &pDevice
        );
		DX9::hwnd = d3dpp.hDeviceWindow;
        pD3D->Release();

        if (FAILED(hr)) return nullptr;
        return pDevice;
    }
}
