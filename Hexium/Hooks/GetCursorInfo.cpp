#pragma once 
#include "../hooks.hpp"
#include "../Utils/ReplayParser.h"
int __cdecl H::GetCursorInfoDetour(MouseInput* a1) {
    int result = H::pGetCursorInfoOG(a1);



    

    return result;
}