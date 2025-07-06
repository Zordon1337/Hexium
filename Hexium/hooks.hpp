#pragma once

#include <sstream>
#include <vector>
#include <libloaderapi.h>
#include "globals.hpp"
#include "Minhook/MinHook.h"

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"
#include "ui.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Pattern parsing + scanning
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


// Hooks
using ChannelSetAttrFn = bool(__stdcall*)(LONG handle, int attrib, float value);
inline ChannelSetAttrFn pChannelSetAttrOG = nullptr;
bool __stdcall ChannelSetAttrDetour(LONG handle, int attrib, float value)
{
	if (attrib == 65536 && UIConfig::bTimewarp) {
		return pChannelSetAttrOG(handle, attrib, (UIConfig::flTimewarpSpeed * 100) - 100);
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
	return UIConfig::bDisableHD ? false : result;
}

using StartGameFn = DWORD(__fastcall*)(void* thisptr, void* edx, void* a2);
inline StartGameFn pStartGameOG = nullptr;
DWORD __fastcall StartGame(void* thisptr, void* edx, void* a2)
{
	G::isPlaying = true;
	G::menu->IsOpen = false; // auto hide menu on game start
	printf("Playing!\n");
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

	if (G::menu->IsOpen && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return 1;

	return CallWindowProc(pWndProcOG, hwnd, msg, wParam, lParam);
}

typedef BOOL(WINAPI* SwapBuffers_t)(HDC);
SwapBuffers_t pSwapBuffersOG = nullptr;
BOOL WINAPI hk_SwapBuffers(HDC hdc) {
    if (!G::menu->Initialized && wglGetCurrentContext()) {
        HWND hwnd = WindowFromDC(hdc);
        pWndProcOG = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
        G::menu->Initialize(hwnd);
    }
    
    G::menu->Render();

    return pSwapBuffersOG(hdc);
}

// TODO: move out of hooks??
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

    uintptr_t hexisBase = (uintptr_t)GetModuleHandleA(NULL);
    uintptr_t ptr = *(uintptr_t*)(hexisBase + 0x515F6C);
    if (ptr) {
        int currentTime = *(int*)(ptr + 0x2B8);
        // todo relax or auto
    }

    if (G::isPlaying) {
        //a1->x = 500;
        //a1->y = 300;
    }

    return result;
}


// Initialize all hooks
bool LoadHooks() {
    // Attempt to initialize MinHook
    if (MH_Initialize() != MH_OK) {
		return false;
	}

    // Loads the addresses of our hooks
	auto SwapBuffersPtr = GetProcAddress(GetModuleHandleA("gdi32.dll"), "SwapBuffers");
    CHECK_PATTERN(SwapBuffersPtr, "SwapBuffers");
    auto ChannelSetAttributePtr = GetProcAddress(GetModuleHandleA("bass.dll"), "BASS_ChannelSetAttribute");
	CHECK_PATTERN(ChannelSetAttributePtr, "BASS_ChannelSetAttribute");
	auto OnDrawPtr = PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 89 4D ? 8B 59");
	CHECK_PATTERN(OnDrawPtr, "OnDraw");
	auto HasHiddenPtr = PatternScan("Hexis.exe", "8A 41 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC 33 C0 39 41");
	CHECK_PATTERN(HasHiddenPtr, "HasHidden");
	auto StartGamePtr = PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 8B F9 89 7D ? 33 DB 8D B7");
	CHECK_PATTERN(StartGamePtr, "StartGame");   
	auto GetCursorInfoPtr = PatternScan("Hexis.exe", "55 8B EC 83 EC ? 8D 45 ? 0F 57 C0");
	CHECK_PATTERN(GetCursorInfoPtr, "GetCursorInfo");

    // Create hooks
	CREATE_HOOK(SwapBuffersPtr, &hk_SwapBuffers, &pSwapBuffersOG, "SwapBuffers");
	CREATE_HOOK(ChannelSetAttributePtr, &ChannelSetAttrDetour, &pChannelSetAttrOG, "ChannelSetAttribute");
	CREATE_HOOK(OnDrawPtr, &OnDraw, &pOnDrawOG, "OnDraw");
	CREATE_HOOK(HasHiddenPtr, &HasHidden, &pHasHiddenOG, "HasHidden");
	CREATE_HOOK(StartGamePtr, &StartGame, &pStartGameOG, "StartGame");
	CREATE_HOOK(GetCursorInfoPtr, &GetCursorInfoDetour, &pGetCursorInfoOG, "GetCursorInfo");

    // Enable hooks
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
		printf("Failed to enable hooks!\n");
        return false;
	}

    return true;
}