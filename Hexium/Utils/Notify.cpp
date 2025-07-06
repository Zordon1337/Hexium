#include "Notify.h"
#include "../Structs/QString.h"
#include "../memory.hpp"
#include "../globals.hpp"
#include "../logger.hpp"

void Notify::Send(const char* message, int durationMs) {
	if (!message || !M::Utf8_toQString || !G::memoryInitialized) return;

	using PrintNotifFn = void(__cdecl*)(QString* message, int durationMs);
	PrintNotifFn PrintNotif = nullptr;

	QString qMessage;

	/*
	* caching offset becase:
	* 1. searching for pattern every time is slow asf
	* 2. for some reason when searching for 2nd time it fails.
	*/
	auto static PrintPtr = M::PatternScan("Hexis.exe", "55 8B EC A1 ? ? ? ? 85 C0 74 ? 8D 88 ? ? ? ? 85 C9 74 ? FF 75 ? FF 75 ? 6A ? 6A ? 6A");

	M::Utf8_toQString(&qMessage, message, -1);
	if (!PrintPtr) {
		PrintPtr = M::PatternScan("Hexis.exe", "55 8B EC A1 ? ? ? ? 85 C0 74 ? 8D 88 ? ? ? ? 85 C9 74 ? FF 75 ? FF 75 ? 6A ? 6A ? 6A");
		PrintNotif = reinterpret_cast<PrintNotifFn>(PrintPtr);
		if (PrintNotif) {
			PrintNotif(&qMessage, durationMs);
		}
		else {
			LOG_ERROR("PrintNotification not found!");
		}
		return;
	}
	else {
		PrintNotif = reinterpret_cast<PrintNotifFn>(PrintPtr);
		PrintNotif(&qMessage, durationMs);
	}
}
