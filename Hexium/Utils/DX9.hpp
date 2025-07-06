#pragma once
#pragma comment(lib, "d3d9.lib")
#include <d3d9.h>

namespace DX9 {
	inline HWND hwnd = nullptr;
    IDirect3DDevice9* GetD3D9Device();
}
