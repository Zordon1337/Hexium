#pragma once
#define CHECK_PATTERN(PATTERN) \
    if (!PATTERN) { \
        printf("Pattern not found!\n"); \
        MH_Uninitialize(); \
        return false; \
    } else { \
        printf("Pattern found at: %p\n", PATTERN); \
    }

namespace G {
    inline bool isDrawing = false;
    inline bool isMenuOpen = true;
    inline bool isPlaying = false;
}

namespace CFG {
    inline bool bDisableHD = false;
    inline bool bTimewarp = false;
    inline float flTimewarpSpeed = 1.0f;
}
