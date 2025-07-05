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
}