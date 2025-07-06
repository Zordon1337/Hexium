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
#include "ui.hpp"
#include "memory.hpp"

class QString {
private:
	char* m_data;
	int m_length;

public:
	// Default constructor
	QString() : m_data(nullptr), m_length(0) {}

	// Constructor from const char*
	QString(const char* str) {
		if (str) {
			m_length = static_cast<int>(std::strlen(str));
			m_data = new char[m_length + 1];
			std::memcpy(m_data, str, m_length + 1);
		}
		else {
			m_data = nullptr;
			m_length = 0;
		}
	}

	// Copy constructor
	QString(const QString& other) {
		m_length = other.m_length;
		if (other.m_data) {
			m_data = new char[m_length + 1];
			std::memcpy(m_data, other.m_data, m_length + 1);
		}
		else {
			m_data = nullptr;
		}
	}

	// Move constructor
	QString(QString&& other) noexcept {
		m_data = other.m_data;
		m_length = other.m_length;
		other.m_data = nullptr;
		other.m_length = 0;
	}

	// Copy assignment
	QString& operator=(const QString& other) {
		if (this != &other) {
			delete[] m_data;
			m_length = other.m_length;
			if (other.m_data) {
				m_data = new char[m_length + 1];
				std::memcpy(m_data, other.m_data, m_length + 1);
			}
			else {
				m_data = nullptr;
			}
		}
		return *this;
	}

	// Move assignment
	QString& operator=(QString&& other) noexcept {
		if (this != &other) {
			delete[] m_data;
			m_data = other.m_data;
			m_length = other.m_length;
			other.m_data = nullptr;
			other.m_length = 0;
		}
		return *this;
	}

	// Destructor
	~QString() {
		delete[] m_data;
	}

	// Length
	int length() const {
		return m_length;
	}

	// toUtf8: mimic Qt API (here just return std::string)
	std::string toUtf8() const {
		return m_data ? std::string(m_data) : std::string();
	}

	// const char* access
	const char* c_str() const {
		return m_data;
	}
};

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
			if (text)
				printf("[Notification] %s\n", text);
		}
	}

	// Initialize all hooks
	inline bool Init() {
		// Attempt to initialize MinHook
		if (MH_Initialize() != MH_OK) {
			return false;
		}

		// Loads the addresses of our hooks
		auto SwapBuffersPtr = GetProcAddress(GetModuleHandleA("gdi32.dll"), "SwapBuffers");
		CHECK_PATTERN(SwapBuffersPtr, "SwapBuffers");
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
		CREATE_HOOK(SwapBuffersPtr, &H::hk_SwapBuffers, &H::pSwapBuffersOG, "SwapBuffers");
		CREATE_HOOK(ChannelSetAttributePtr, &H::ChannelSetAttrDetour, &H::pChannelSetAttrOG, "ChannelSetAttribute");
		CREATE_HOOK(OnDrawPtr, &H::OnDraw, &H::pOnDrawOG, "OnDraw");
		CREATE_HOOK(HasHiddenPtr, &H::HasHidden, &H::pHasHiddenOG, "HasHidden");
		CREATE_HOOK(StartGamePtr, &H::StartGame, &H::pStartGameOG, "StartGame");
		CREATE_HOOK(GetCursorInfoPtr, &H::GetCursorInfoDetour, &H::pGetCursorInfoOG, "GetCursorInfo");
		CREATE_HOOK(PrintPtr, &PrintNotifDetour, &pPrintNotifOG, "PrintNotification");


		// Enable hooks
		if (MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK) {
			LOG_ERROR("Failed to enable hooks!\n");
			return false;
		}

		return true;
	}
}