#include <Windows.h>
#include "Minhook/MinHook.h"
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>

#include "globals.h"

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ParsePattern(const char* pattern, std::vector<uint8_t>& bytes, std::vector<bool>& mask) {
	std::istringstream iss(pattern);
	std::string byteStr;
	while (iss >> byteStr) {
		if (byteStr == "?" || byteStr == "??") {
			bytes.push_back(0);
			mask.push_back(true);
		}
		else {
			if (byteStr.size() != 2 || !isxdigit(byteStr[0]) || !isxdigit(byteStr[1]))
				return false;
			bytes.push_back(static_cast<uint8_t>(strtoul(byteStr.c_str(), nullptr, 16)));
			mask.push_back(false);
		}
	}
	return !bytes.empty();
}

uint8_t* PatternScan(const char* module, const char* pattern) {
	HMODULE hModule = GetModuleHandleA(module);
	if (!hModule) return nullptr;

	auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

	auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(hModule) + dosHeader->e_lfanew);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return nullptr;

	auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto base = reinterpret_cast<uint8_t*>(hModule);

	std::vector<uint8_t> patternBytes;
	std::vector<bool> patternMask;
	if (!ParsePattern(pattern, patternBytes, patternMask))
		return nullptr;

	for (size_t i = 0; i < sizeOfImage - patternBytes.size(); ++i) {
		bool found = true;
		for (size_t j = 0; j < patternBytes.size(); ++j) {
			if (!patternMask[j] && base[i + j] != patternBytes[j]) {
				found = false;
				break;
			}
		}
		if (found)
			return &base[i];
	}

	return nullptr;
}

using ChannelSetAttrFn = bool(__stdcall*)(LONG handle, int attrib, float value);
inline ChannelSetAttrFn pChannelSetAttrOG = nullptr;
bool __stdcall ChannelSetAttrDetour(LONG handle, int attrib, float value)
{
	if (attrib == 65536 && CFG::bTimewarp) {
		return pChannelSetAttrOG(handle, attrib, (CFG::flTimewarpSpeed * 100) - 100);
	}
	return pChannelSetAttrOG(handle, attrib, value);
}

using OnDrawFn = void(__fastcall*)(void* a1);
inline OnDrawFn pOnDrawOG = nullptr;
void __fastcall OnDraw(void* a1)
{
	G::isDrawing = true;
	pOnDrawOG(a1);
	G::isDrawing = false; // refactor this shit, setting isDrawing before function is completed
}

using HasHiddenFn = bool(__fastcall*)(void* a1);
inline HasHiddenFn pHasHiddenOG = nullptr;
bool __fastcall HasHidden(void* a1)
{

	bool result = pHasHiddenOG(a1);

	return CFG::bDisableHD ? false : result;
}
#include <chrono>

// Global
inline std::chrono::steady_clock::time_point g_GameStartTime;
using StartGameFn = DWORD(__fastcall*)(void* thisptr, void* edx, void* a2);
inline StartGameFn pStartGameOG = nullptr;
DWORD __fastcall StartGame(void* thisptr, void* edx, void* a2)
{
	G::isPlaying = true;
	printf("Playing!\n");
	g_GameStartTime = std::chrono::steady_clock::now();
	if (pStartGameOG)
		return pStartGameOG(thisptr, edx, a2);
	else
	{
		printf("pStartGameOG is NULL!");
		return 0;
	}
}


using WndProcFn = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
inline WndProcFn pWndProcOG = nullptr;

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (G::isMenuOpen && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return 1;

	return CallWindowProc(pWndProcOG, hwnd, msg, wParam, lParam);
}

typedef BOOL(WINAPI* SwapBuffers_t)(HDC);
SwapBuffers_t o_SwapBuffers = nullptr;
BOOL WINAPI hk_SwapBuffers(HDC hdc) {
	static bool initialized = false;
	if (!initialized && wglGetCurrentContext()) {
		HWND hwnd = WindowFromDC(hdc);
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplOpenGL3_Init();
		initialized = true;

		// why the fuck it even works
		pWndProcOG = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		auto io = ImGui::GetIO();

		io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf",16.f);
	}

	if (initialized && G::isMenuOpen) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::StyleColorsLight();
		ImGui::Begin("Hexium - Internal Cheat for Hexis");
		
		ImGui::Checkbox("Disable HD (Requires map restart)", &CFG::bDisableHD);
		ImGui::Spacing();
		ImGui::Checkbox("Timewarp", &CFG::bTimewarp);
		if (CFG::bTimewarp) {
			ImGui::SliderFloat("Speed", &CFG::flTimewarpSpeed, 0.1f, 2.f);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	return o_SwapBuffers(hdc);
}
struct MouseInput {
	int x;
	int y;
	int leftButtonDown;
	int rightButtonDown;
	int middleButtonDown;
};

using GetCursorInfoFn = int(__cdecl*)(MouseInput* a1);
inline GetCursorInfoFn pGetCursorInfoOG = nullptr;
int __cdecl GetCursorInfoDetour(MouseInput* a1) {
	int result = pGetCursorInfoOG(a1);
	//printf("Cursor Position: %i, %i\n", a1->x, a1->y);
	a1->x = 500;
	a1->y = 360;
	return result;
}
DWORD WINAPI Entry(LPVOID lpParam)
{
#if _DEBUG
	AllocConsole();
	FILE* pFile;
	freopen_s(&pFile, "CONOUT$", "w", stdout);
	SetConsoleTitleA("Hexium Debug Console");
#endif

	if (MH_Initialize() != MH_OK) {
		return 1;
	}

	auto BASS_ChannelSetAttribute = GetProcAddress(GetModuleHandleA("bass.dll"), "BASS_ChannelSetAttribute");
	CHECK_PATTERN(BASS_ChannelSetAttribute, "BASS_ChannelSetAttribute");
	auto OnDrawPtr = PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 89 4D ? 8B 59");
	CHECK_PATTERN(OnDrawPtr, "OnDraw");
	auto HasHiddenPtr = PatternScan("Hexis.exe", "8A 41 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC 33 C0 39 41");
	CHECK_PATTERN(HasHiddenPtr, "HasHidden");
	auto StartGamePtr = PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 8B F9 89 7D ? 33 DB 8D B7");
	CHECK_PATTERN(StartGamePtr, "StartGame");
	auto GetCursorInfoPtr = PatternScan("Hexis.exe", "55 8B EC 83 EC ? 8D 45 ? 0F 57 C0");
	CHECK_PATTERN(GetCursorInfoPtr, "GetCursorInfo");

	MH_CreateHook(
		BASS_ChannelSetAttribute,
		&ChannelSetAttrDetour,
		reinterpret_cast<LPVOID*>(&pChannelSetAttrOG)
	);
	MH_CreateHook(
		OnDrawPtr,
		&OnDraw,
		reinterpret_cast<LPVOID*>(&pOnDrawOG)
	);

	MH_CreateHook(
		HasHiddenPtr,
		&HasHidden,
		reinterpret_cast<LPVOID*>(&pHasHiddenOG)
	);
	MH_CreateHook(
		StartGamePtr,
		&StartGame,
		reinterpret_cast<LPVOID*>(&pStartGameOG)
	);
	MH_CreateHook(
		GetCursorInfoPtr,
		&GetCursorInfoDetour,
		reinterpret_cast<LPVOID*>(&pGetCursorInfoOG)
	);
	HMODULE hGDI = GetModuleHandleA("gdi32.dll");
	void* pSwapBuffers = GetProcAddress(hGDI, "SwapBuffers");

	if (MH_CreateHook(pSwapBuffers, &hk_SwapBuffers, reinterpret_cast<void**>(&o_SwapBuffers)) != MH_OK) {
		printf("Failed to create SwapBuffers hook\n");
		return -1;
	}

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
		printf("Failed to enable hooks!\n");
	}
	
	while(true) {

		if (GetAsyncKeyState(VK_INSERT) & 1) {
			G::isMenuOpen = !G::isMenuOpen;
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

