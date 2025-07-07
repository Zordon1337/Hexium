#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "platform.hpp"

#pragma comment(lib, "Ws2_32.lib")

#include <sstream>
#include <vector>
#include <libloaderapi.h>

#include "globals.hpp"
#include "Minhook/MinHook.h"

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_dx9.h"

#include "ui.hpp"
#include "memory.hpp"
#include "Utils/Notify.h"
#include "Utils/DX9.hpp"
#include "Utils/Game.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// TODO: move out of hooks??
struct MouseInput {
	int x;
	int y;
	// the ones under are not confirmed, unknown tbh
	int leftButtonDown;
	int rightButtonDown;
	int middleButtonDown;
};

namespace H {

	using GetCursorInfoFn = int(__cdecl*)(MouseInput* a1);
	inline GetCursorInfoFn pGetCursorInfoOG = nullptr;
	int __cdecl GetCursorInfoDetour(MouseInput* a1);


	typedef BOOL(WINAPI* SwapBuffers_t)(HDC);
	inline SwapBuffers_t pSwapBuffersOG = nullptr;
	BOOL WINAPI hk_SwapBuffers(HDC hdc);


	using WndProcFn = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
	inline WndProcFn pWndProcOG = nullptr;
	LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	using StartGameFn = DWORD(__fastcall*)(void* thisptr, void* edx, void* a2);
	inline StartGameFn pStartGameOG = nullptr;
	DWORD __fastcall StartGame(void* thisptr, void* edx, void* a2);


	using HasHiddenFn = bool(__fastcall*)(void* a1);
	inline HasHiddenFn pHasHiddenOG = nullptr;
	bool __fastcall HasHidden(void* a1);


	using OnDrawFn = void(__fastcall*)(void* a1);
	inline OnDrawFn pOnDrawOG = nullptr;
	void __fastcall OnDraw(void* a1);

	using ChannelSetAttrFn = bool(__stdcall*)(LONG handle, int attrib, float value);
	inline ChannelSetAttrFn pChannelSetAttrOG = nullptr;
	bool __stdcall ChannelSetAttrDetour(LONG handle, int attrib, float value);

	using PrintNotifFn = void(__cdecl*)(QString* message, int durationMs);
	inline PrintNotifFn pPrintNotifOG = nullptr;
	void __cdecl PrintNotifDetour(QString* message, int durationMs);

	using EndSceneFn = HRESULT(__stdcall*)(IDirect3DDevice9* device);
	inline EndSceneFn pEndSceneOG = nullptr;
	HRESULT __stdcall EndScene(IDirect3DDevice9* device);

	using DX9ResetFn = HRESULT(__stdcall*)(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
	inline DX9ResetFn pDX9ResetOG = nullptr;
	HRESULT __stdcall DX9Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);

	using DrawBeatmapFn = void(__fastcall*)(void* a1, void* a2, void* a3);
	inline DrawBeatmapFn pDrawBeatmapOG = nullptr;
	void __fastcall DrawBeatmap(void* a1, void* a2, void* a3);

	using GetCursorPosFn = BOOL(WINAPI*)(LPPOINT lpPoint);
	inline GetCursorPosFn pGetCursorPosOG = nullptr;
	BOOL WINAPI GetCursorPosDetour(LPPOINT lpPoint);

	// Initialize all hooks
	inline bool Init() {
		// Attempt to initialize MinHook
		if (MH_Initialize() != MH_OK) {
			return false;
		}
		bool isOpenGL = Game::usesOpenGL();

		// Loads the addresses of our hooks
		FARPROC SwapBuffersPtr = nullptr;
		IDirect3DDevice9* pDevice = nullptr;
		if (isOpenGL) {
			SwapBuffersPtr = GetProcAddress(GetModuleHandleA("gdi32.dll"), "SwapBuffers");
			CHECK_PATTERN(SwapBuffersPtr, "SwapBuffers");
		}
		else
		{
			pDevice = DX9::GetD3D9Device();
			CHECK_PATTERN(pDevice, "D3D9 Device");
		}
		auto ChannelSetAttributePtr = GetProcAddress(GetModuleHandleA("bass.dll"), "BASS_ChannelSetAttribute");
		CHECK_PATTERN(ChannelSetAttributePtr, "BASS_ChannelSetAttribute");
		auto OnDrawPtr = M::PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 89 4D ? 8B 59");
		CHECK_PATTERN(OnDrawPtr, "OnDraw");
		auto HasHiddenPtr = M::PatternScan("Hexis.exe", "8A 41 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC 33 C0 39 41");
		CHECK_PATTERN(HasHiddenPtr, "HasHidden");
		auto StartGamePtr = M::PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 8B F9 89 7D ? 33 DB 8D B7");
		CHECK_PATTERN(StartGamePtr, "StartGame");
		auto GetCursorInfoPtr = M::PatternScan("Hexis.exe", "55 8B EC 83 EC ? 8D 45 ? 0F 57 C0");
		CHECK_PATTERN(GetCursorInfoPtr, "GetCursorInfo");
		auto PrintPtr = M::PatternScan("Hexis.exe", "55 8B EC A1 ? ? ? ? 85 C0 74 ? 8D 88 ? ? ? ? 85 C9 74 ? FF 75 ? FF 75 ? 6A ? 6A ? 6A");
		CHECK_PATTERN(PrintPtr, "PrintNotification");
		auto ConnectToServerLoopPtr = M::PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? 53 56 57 50 8D 45 ? 64 A3 ? ? ? ? 89 65 ? 8B F1");
		CHECK_PATTERN(ConnectToServerLoopPtr, "ConnectToServer");
		auto DrawBeatmapPtr = M::PatternScan("Hexis.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 ? 64 A3 ? ? ? ? 8B D9 89 5D ? 33 C0 89 45 ? F3 0F 10 45");
		CHECK_PATTERN(DrawBeatmapPtr, "DrawBeatmap");
		auto GetCursorPosPtr = GetProcAddress(GetModuleHandleA("user32.dll"), "GetCursorPos");
		CHECK_PATTERN(GetCursorPosPtr, "GetCursorPos");
		// Create hooks
		// Hooking Bass_ChannelSetAttribute for Timewarp
		CREATE_HOOK(ChannelSetAttributePtr, &H::ChannelSetAttrDetour, &H::pChannelSetAttrOG, "ChannelSetAttribute");
		// I Don't even remember why I hooked this lol
		CREATE_HOOK(OnDrawPtr, &H::OnDraw, &H::pOnDrawOG, "OnDraw");
		// Hooking to spoof Mod Value
		CREATE_HOOK(HasHiddenPtr, &H::HasHidden, &H::pHasHiddenOG, "HasHidden");
		// Hooking to know when user started map
		CREATE_HOOK(StartGamePtr, &H::StartGame, &H::pStartGameOG, "StartGame");
		// Hooking GetCursorInfo for possibly rx or auto without touching input outside of game
		CREATE_HOOK(GetCursorInfoPtr, &H::GetCursorInfoDetour, &H::pGetCursorInfoOG, "GetCursorInfo");
		// Hooking PrintNotif
		CREATE_HOOK(PrintPtr, &H::PrintNotifDetour, &H::pPrintNotifOG, "PrintNotification");
		// Hooking DrawBeatmap for Replay bot
		CREATE_HOOK(DrawBeatmapPtr, &H::DrawBeatmap, &H::pDrawBeatmapOG, "DrawBeatmap");
		// Hooking GetCursorPos
		CREATE_HOOK(GetCursorPosPtr, &H::GetCursorPosDetour, &H::pGetCursorPosOG, "GetCursorPos");

		if (!isOpenGL) {
			// we are using dx9
			auto pDeviceVTable = *reinterpret_cast<uintptr_t**>(pDevice);
			auto EndScenePtr = pDeviceVTable[42];
			CREATE_HOOK(pDeviceVTable[42], &EndScene, &pEndSceneOG, "EndScene");
			UIGlobals::bIsOpenGL = false;
			UIGlobals::pDevice = pDevice;
			auto DX9ResetPtr = pDeviceVTable[16];
			CREATE_HOOK(DX9ResetPtr, &DX9Reset, &pDX9ResetOG, "DX9 Reset");
		}
		else {
			// we are using opengl
			CREATE_HOOK(SwapBuffersPtr, &H::hk_SwapBuffers, &H::pSwapBuffersOG, "SwapBuffers");
			UIGlobals::bIsOpenGL = true;
		}

		// Enable hooks
		if (MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
			LOG_ERROR("Failed to enable hooks!\n");
			return false;
		}

		G::hooksInitialized = true;
		Notify::Send("Hooked successfully!", 1000);

		return true;
	}
}