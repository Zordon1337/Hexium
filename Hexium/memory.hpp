#pragma once
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

namespace M {
	// Pattern parsing + scanning
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