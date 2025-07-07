#pragma once
#include "ReplayParser.h"
#include <fstream>
#include "../logger.hpp"
#include <sstream>
#include <vector>
#include "../ui.hpp"

void ReplayParser::ParseReplay(const char* replayPath)
{
    std::ifstream file(replayPath);
    if (!file) {
		LOG_ERROR("Failed to open replay file: %s\n", replayPath);
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();

	std::string content = ss.str();

    std::vector<std::string> parts;
    std::stringstream ss2(content);
    std::string token;

    while (std::getline(ss2, token, ',')) {
        parts.push_back(token);
    }
	float currTime = 0;
    for (const auto& part : parts) {
		ReplayFrame frame;
		std::stringstream partStream(part);
		std::string value;

		std::getline(partStream, value, '|');
		frame.TimeSinceLastAction = std::stoi(value);


		std::getline(partStream, value, '|');
		frame.MouseX = std::stof(value);

		std::getline(partStream, value, '|');
		frame.MouseY = std::stof(value);

		std::getline(partStream, value, '|');
		frame.Buttons = std::stoi(value);



		// i could just use the ones provided by dx9 or opengl but i dont want to risk with nullptrs
        RECT rect;
		HWND hwnd = UIGlobals::hwnd;

		// dont touch. works
        if (hwnd && GetClientRect(hwnd, &rect)) {


			float screenWidth = static_cast<float>(rect.right - rect.left);
			float screenHeight = static_cast<float>(rect.bottom - rect.top);
			constexpr float OSU_WIDTH = 512.0f;
			constexpr float OSU_HEIGHT = 384.0f;
			float PLAYFIELD_WIDTH = 1550.0f * (screenWidth / 2560.f);
			float PLAYFIELD_HEIGHT = 1150.0f * (screenHeight / 1440.f);

			constexpr float PLAYFIELD_X = 500.0f;
			constexpr float PLAYFIELD_Y = 150.0f;

			float scaleX = PLAYFIELD_WIDTH / OSU_WIDTH;
			float scaleY = PLAYFIELD_HEIGHT / OSU_HEIGHT;
			float scale = std::min(scaleX, scaleY);

			float mappedX = frame.MouseX * scale + PLAYFIELD_X;
			float mappedY = frame.MouseY * scale + PLAYFIELD_Y;

			frame.MouseX = mappedX;
			frame.MouseY = mappedY;

        }




		currTime += frame.TimeSinceLastAction;

		CurrentReplay[currTime] = frame;
    }
}
