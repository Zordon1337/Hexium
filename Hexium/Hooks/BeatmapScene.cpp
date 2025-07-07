#pragma once
#include "../hooks.hpp"
#include "../Utils/ReplayParser.h"

static int lastButtons = 0;
static float lastMouseX = -1.f;
static float lastMouseY = -1.f;

void __fastcall H::DrawBeatmap(void* a1, void* a2, void* a3) {
	if (!UIConfig::bReplayBot) {
		return H::pDrawBeatmapOG(a1, a2, a3);
	}
    uintptr_t hexisBase = (uintptr_t)GetModuleHandleA(NULL);
    uintptr_t ptr = *(uintptr_t*)(hexisBase + 0x515F6C);
    if (ptr && a1) {
        int currentTime = *(int*)(ptr + 0x2B8);

        auto it = ReplayParser::CurrentReplay.find(currentTime);
        if (it != ReplayParser::CurrentReplay.end()) {
            const auto& frame = it->second;
            int currentButtons = frame.Buttons;

            std::vector<INPUT> inputs;

            if ((currentButtons & 1) && !(lastButtons & 1)) {
                INPUT input = {};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                inputs.push_back(input);
            }
            else if (!(currentButtons & 1) && (lastButtons & 1)) {
                INPUT input = {};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                inputs.push_back(input);
            }

            if ((currentButtons & 2) && !(lastButtons & 2)) {
                INPUT input = {};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                inputs.push_back(input);
            }
            else if (!(currentButtons & 2) && (lastButtons & 2)) {
                INPUT input = {};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                inputs.push_back(input);
            }

            if ((currentButtons & 4) && !(lastButtons & 4)) {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = 'Z';
                input.ki.dwFlags = 0;
                inputs.push_back(input);
            }
            else if (!(currentButtons & 4) && (lastButtons & 4)) {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = 'Z';
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                inputs.push_back(input);
            }

            if ((currentButtons & 8) && !(lastButtons & 8)) {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = 'X';
                input.ki.dwFlags = 0;
                inputs.push_back(input);
            }
            else if (!(currentButtons & 8) && (lastButtons & 8)) {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = 'X';
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                inputs.push_back(input);
            }

            if (!inputs.empty() && UIConfig::bReplayBotDisableRelax) {
                SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
            }

            lastButtons = currentButtons;

            if ((frame.MouseX != lastMouseX || frame.MouseY != lastMouseY) && UIConfig::bReplayBotDisableAuto) {
                SetCursorPos(static_cast<int>(frame.MouseX), static_cast<int>(frame.MouseY));
                lastMouseX = frame.MouseX;
                lastMouseY = frame.MouseY;
            }
        }
    }

    return pDrawBeatmapOG(a1, a2, a3);
}
