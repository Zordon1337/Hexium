#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
#include "../ui.hpp"
bool __fastcall H::HasHidden(void* a1)
{
	bool result = H::pHasHiddenOG(a1);
	return UIConfig::bDisableHD ? false : result;
}