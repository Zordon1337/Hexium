#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
#include "../Utils/ReplayParser.h"
DWORD __fastcall H::StartGame(void* thisptr, void* edx, void* a2)
{
	G::isPlaying = true;
	G::menu->IsOpen = false; // auto hide menu on game start
	printf("Playing!\n");

	// don't waste time on parsing replay when replaybot is disabled
	if (!UIConfig::bReplayBot) {
		return H::pStartGameOG(thisptr, edx, a2);
	}

	ReplayParser::ParseReplay(UIConfig::srReplayPath);

	return H::pStartGameOG(thisptr, edx, a2);
} 