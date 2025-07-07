#pragma once
#include "../hooks.hpp"
#include "../Utils/ReplayParser.h"
BOOL WINAPI H::GetCursorPosDetour(LPPOINT lpPoint) {

	return H::pGetCursorPosOG(lpPoint);
	
}