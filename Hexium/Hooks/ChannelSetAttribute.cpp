#pragma once
#include "../hooks.hpp"
#include "../globals.hpp"
#include "../ui.hpp"
bool __stdcall H::ChannelSetAttrDetour(LONG handle, int attrib, float value)
{
	if (attrib == 65536 && UIConfig::bTimewarp) {
		return H::pChannelSetAttrOG(handle, attrib, (UIConfig::flTimewarpSpeed * 100) - 100);
	}
	return H::pChannelSetAttrOG(handle, attrib, value);
}