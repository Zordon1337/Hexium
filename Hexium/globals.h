#pragma once
#define CHECK_PATTERN(PATTERN, NAME) \
    if (!PATTERN) { \
        printf("%s not found!\n", NAME); \
        MH_Uninitialize(); \
        return false; \
    } else { \
        printf("%s found at: %p\n", NAME, PATTERN); \
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
