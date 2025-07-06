#pragma once

#include <Windows.h>
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


	// TODO: Move body to Hooks/PrintNotification.cpp
	inline void __cdecl PrintNotifDetour(QString* message, int durationMs) {

		// call og func first otherwise in game we will see funny chinese text lmao
		pPrintNotifOG(message, durationMs);

		if (message && M::QString_toUtf8 && M::QByteArray_constData) {
			QByteArray* utf8 = M::QString_toUtf8(message);
			const char* text = M::QByteArray_constData(utf8);
			//if (text)
				//printf("[Notification] %s\n", text);
		}
	}

	using EndSceneFn = HRESULT(__stdcall*)(IDirect3DDevice9* device);
	inline EndSceneFn pEndSceneOG = nullptr;
	HRESULT __stdcall EndScene(IDirect3DDevice9* device);

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


		// Create hooks
		CREATE_HOOK(ChannelSetAttributePtr, &H::ChannelSetAttrDetour, &H::pChannelSetAttrOG, "ChannelSetAttribute");
		CREATE_HOOK(OnDrawPtr, &H::OnDraw, &H::pOnDrawOG, "OnDraw");
		CREATE_HOOK(HasHiddenPtr, &H::HasHidden, &H::pHasHiddenOG, "HasHidden");
		CREATE_HOOK(StartGamePtr, &H::StartGame, &H::pStartGameOG, "StartGame");
		CREATE_HOOK(GetCursorInfoPtr, &H::GetCursorInfoDetour, &H::pGetCursorInfoOG, "GetCursorInfo");
		CREATE_HOOK(PrintPtr, &PrintNotifDetour, &pPrintNotifOG, "PrintNotification");
		if (!isOpenGL) {
			// we are using dx9
			auto pDeviceVTable = *reinterpret_cast<uintptr_t**>(pDevice);
			auto EndScenePtr = pDeviceVTable[42];
			CREATE_HOOK(pDeviceVTable[42], &EndScene, &pEndSceneOG, "EndScene");
			UIGlobals::bIsOpenGL = false;
			UIGlobals::pDevice = pDevice;
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