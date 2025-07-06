#pragma once

#include "ui.hpp"

#define CHECK_PATTERN(PATTERN, NAME) \
    if (!PATTERN) { \
        printf("%s not found!\n", NAME); \
        MH_Uninitialize(); \
        return false; \
    } else { \
        printf("%s found at: %p\n", NAME, PATTERN); \
    }

#define CREATE_HOOK(TARGET, DETOUR, ORIGINAL, NAME) \
    do { \
        if (MH_CreateHook(reinterpret_cast<LPVOID>(TARGET), reinterpret_cast<LPVOID>(DETOUR), reinterpret_cast<LPVOID*>(ORIGINAL)) != MH_OK) { \
            printf("Failed to create %s hook!\n", NAME); \
            return false; \
        } \
    } while (0)

namespace G {
    inline bool isDrawing = false;
    inline bool isPlaying = false;
    inline std::shared_ptr<UI> menu;
}