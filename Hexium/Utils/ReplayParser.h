#pragma once
#include <map>
// http://osu.ppy.sh/wiki/en/Client/File_formats/osr_%28file_format%29
struct ReplayFrame {
    int TimeSinceLastAction;
    float MouseX;
    float MouseY;
    int Buttons;
};

namespace ReplayParser {
	inline int iLastGameTime = 0;
	inline std::map<int, ReplayFrame> CurrentReplay;
	void ParseReplay(const char* replayPath);

}