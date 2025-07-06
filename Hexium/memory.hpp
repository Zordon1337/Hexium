#pragma once
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include "logger.hpp"

#include "Structs/QString.h"
#include "Structs/QByteArray.h"

#include "globals.hpp"

namespace M {
	using ToUtf8Fn = QByteArray * (__stdcall*)(QString* thisPtr);
	using ConstDataFn = const char* (__cdecl*)(QByteArray* thisPtr);
	using ToQString = void(__cdecl*)(QString* newString, const char* str, int len);

	inline ToUtf8Fn QString_toUtf8 = nullptr;
	inline ConstDataFn QByteArray_constData = nullptr;
	inline ToQString Utf8_toQString = nullptr;

	inline bool ParsePattern(const char* pattern, std::vector<uint8_t>& bytes, std::vector<bool>& mask) {
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

	inline uint8_t* PatternScan(const char* module, const char* pattern) {
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
}

namespace U {
	inline void Notification(const char* message, int durationMs) {
		if (!message || !M::Utf8_toQString || !G::memoryInitialized) return;

		using PrintNotifFn = void(__cdecl*)(QString* message, int durationMs);
		PrintNotifFn PrintNotif = nullptr;

		QString qMessage;

		/*
		* caching offset becase:
		* 1. searching for pattern every time is slow asf
		* 2. for some reason when searching for 2nd time it fails.
		*/
		auto static PrintPtr = M::PatternScan("Hexis.exe", "55 8B EC A1 ? ? ? ? 85 C0 74 ? 8D 88 ? ? ? ? 85 C9 74 ? FF 75 ? FF 75 ? 6A ? 6A ? 6A");

		M::Utf8_toQString(&qMessage, message, -1);
		if (!PrintPtr) {
			PrintPtr = M::PatternScan("Hexis.exe", "55 8B EC A1 ? ? ? ? 85 C0 74 ? 8D 88 ? ? ? ? 85 C9 74 ? FF 75 ? FF 75 ? 6A ? 6A ? 6A");
			PrintNotif = reinterpret_cast<PrintNotifFn>(PrintPtr);
			if (PrintNotif) {
				PrintNotif(&qMessage, durationMs);
			}
			else {
				LOG_ERROR("PrintNotification not found!");
			}
			return;
		}
		else {
			PrintNotif = reinterpret_cast<PrintNotifFn>(PrintPtr);
			PrintNotif(&qMessage, durationMs);
		}
	}
}

namespace M {
	inline bool Init() {
		HMODULE qtcore = GetModuleHandleA("Qt5Core.dll");
		if (!qtcore) {
			LOG_ERROR("Qt5Core not loaded");
			return false;
		}

		QString_toUtf8 = reinterpret_cast<ToUtf8Fn>(GetProcAddress(qtcore, "?toUtf8@QString@@QBE?AVQByteArray@@XZ"));
		QByteArray_constData = reinterpret_cast<ConstDataFn>(GetProcAddress(qtcore, "?constData@QByteArray@@QBEPBDXZ"));
		Utf8_toQString = reinterpret_cast<ToQString>(GetProcAddress(qtcore, "?fromUtf8@QString@@SA?AV1@PBDH@Z"));
		if (!QString_toUtf8 || !QByteArray_constData || !Utf8_toQString) {
			LOG_ERROR("Failed to resolve qt5core functions");
			return false;
		}

		G::memoryInitialized = true;
		U::Notification("Memory initialized successfully!", 3000);
		return true;
	}
}
