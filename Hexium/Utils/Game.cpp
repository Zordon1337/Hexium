#include "Game.h"
#include <cstdint>
#include "../platform.hpp"
// Note, this is value from Settings, it will change once user changes their render type in settings
// but we don't really care since we use this only on hooking
bool Game::usesOpenGL() {
	auto base = reinterpret_cast<uintptr_t>(GetModuleHandleA("Hexis.exe"));
	bool val = *reinterpret_cast<bool*>(base + 0x518CB5);
	return val;
}