#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
DWORD __fastcall H::StartGame(void* thisptr, void* edx, void* a2)
{
	G::isPlaying = true;
	G::menu->IsOpen = false; // auto hide menu on game start
	printf("Playing!\n");


	return H::pStartGameOG(thisptr, edx, a2);
}