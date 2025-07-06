#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
void __fastcall H::OnDraw(void* a1)
{
	G::isDrawing = true;
	H::pOnDrawOG(a1);
	G::isDrawing = false; // refactor this shit, setting isDrawing before function is completed
}