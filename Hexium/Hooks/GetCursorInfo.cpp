#pragma once 
#include "../hooks.hpp"
int __cdecl H::GetCursorInfoDetour(MouseInput* a1) {
    int result = H::pGetCursorInfoOG(a1);

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