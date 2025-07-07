#pragma once
#include "../hooks.hpp"

void __cdecl H::PrintNotifDetour(QString* message, int durationMs) {
	H::pPrintNotifOG(message, durationMs);
}