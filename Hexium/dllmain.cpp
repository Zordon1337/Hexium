#include <Windows.h>
#include "Minhook/MinHook.h"
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>

#include "globals.hpp"
#include "hooks.hpp"
#include "ui.hpp"

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"

DWORD WINAPI Entry(LPVOID lpParam)
{
#if _DEBUG
	AllocConsole();
	FILE* pFile;
	freopen_s(&pFile, "CONOUT$", "w", stdout);
	SetConsoleTitleA("Hexium Debug Console");
#endif
	
	G::menu = std::make_shared<UI>();

	if (!LoadHooks()) return -1;

	while (true) {
		if (GetAsyncKeyState(VK_INSERT) & 1) {
			G::menu->IsOpen = !G::menu->IsOpen;
		}
		Sleep(5); 
	}
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);
		HANDLE hThread = CreateThread(NULL, 0, Entry, NULL, 0, NULL);
		if (hThread) {
			CloseHandle(hThread);
		}
	}
	break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

