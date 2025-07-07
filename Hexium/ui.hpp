#pragma once

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_dx9.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <commdlg.h>
#include "Utils/ReplayParser.h"
namespace UIConfig {
    inline bool bDisableHD = false;
    inline bool bTimewarp = false;
    inline float flTimewarpSpeed = 1.0f;

    inline bool bServerSwitcher = false;
	inline int iServerIpIndex = 0;

    inline bool bReplayBot = false;
    inline char srReplayPath[256] = "";
}
namespace UIGlobals {
    inline bool bIsOpenGL = false;
	inline IDirect3DDevice9* pDevice = nullptr;
	inline HWND hwnd = nullptr;
    inline std::vector<const char*> serverIps;
    inline std::vector<std::string> serverIpStrings;
}
class UI {
    private:
    bool IsWindowFullscreen(HWND hwnd) {
        if (!hwnd)
            return false;
        RECT windowRect;
        if (!GetWindowRect(hwnd, &windowRect))
            return false;
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (!monitor)
            return false;
        MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
        if (!GetMonitorInfo(monitor, &monitorInfo))
            return false;

        RECT& monitorRect = monitorInfo.rcMonitor;
        int tolerance = 2;
        bool isFullscreen =
            abs(windowRect.left - monitorRect.left) <= tolerance &&
            abs(windowRect.top - monitorRect.top) <= tolerance &&
            abs(windowRect.right - monitorRect.right) <= tolerance &&
            abs(windowRect.bottom - monitorRect.bottom) <= tolerance;
        return isFullscreen;
    }
    public:
    bool Initialized = false;
    bool IsOpen = true;

    void LoadIpsFromFile() {
        std::ifstream file("serverIps.txt");
        if (!file.is_open()) {
            UIGlobals::serverIpStrings.clear();
            UIGlobals::serverIps.clear();

            UIGlobals::serverIpStrings = { "127.0.0.1", "192.168.0.1" };
            return;
        }

        UIGlobals::serverIpStrings.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty())
                UIGlobals::serverIpStrings.push_back(line);
        }

        UIGlobals::serverIps.clear();
        for (auto& str : UIGlobals::serverIpStrings) {
            UIGlobals::serverIps.push_back(str.c_str());
        }


    }

    void Initialize(HWND hwnd) {
        if (Initialized) return;

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);

		if (UIGlobals::bIsOpenGL) {
			ImGui_ImplOpenGL3_Init();
		}
        Initialized = true;

        auto io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // optional
        io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf",16.f);
        LoadIpsFromFile();
    }

    void Render() {
        if (!Initialized || !IsOpen) return;

        ImGui_ImplWin32_NewFrame();
        if (UIGlobals::bIsOpenGL) {
            ImGui_ImplOpenGL3_NewFrame();
        }
        else {
			ImGui_ImplDX9_NewFrame();
        }
        ImGui::NewFrame();
        ImGui::StyleColorsLight();
        ImGui::Begin("Hexium - Internal Cheat for Hexis");

        if (ImGui::BeginTabBar("Tabs")) {

            if (ImGui::BeginTabItem("Gameplay")) {
                ImGui::Checkbox("Timewarp", &UIConfig::bTimewarp);
                if (UIConfig::bTimewarp) {
                    ImGui::SliderFloat("Speed", &UIConfig::flTimewarpSpeed, 0.1f, 2.f);
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuals")) {
                ImGui::Checkbox("Disable HD (Requires map restart)", &UIConfig::bDisableHD);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Osu replay bot")) {

				if (!IsWindowFullscreen(UIGlobals::hwnd)) {
					ImGui::TextWrapped("Warning! Replay bot works only in fullscreen mode!");
					ImGui::TextWrapped("Please switch to fullscreen mode to use it.");
                    UIConfig::bReplayBot = false;
                }
                else {
                    ImGui::Checkbox("Toggle", &UIConfig::bReplayBot);

                    if (UIConfig::bReplayBot) {
                        ImGui::TextWrapped("Warning! Replays must be first unpacked & converted via osuReplayExtractor");
                        ImGui::TextWrapped("Get it here: https://github.com/MateusPevidor/osuReplayExtractor/tree/master");

                        ImGui::InputText("Replay Path", UIConfig::srReplayPath, IM_ARRAYSIZE(UIConfig::srReplayPath));

                        if (ImGui::Button("Select Replay")) {
                            char filename[MAX_PATH] = "";

                            OPENFILENAMEA ofn = { 0 };
                            ofn.lStructSize = sizeof(ofn);
                            ofn.hwndOwner = nullptr;
                            ofn.lpstrFile = filename;
                            ofn.nMaxFile = MAX_PATH;
                            ofn.lpstrFilter = "Replay Files\0*.osr\0All Files\0*.*\0";
                            ofn.nFilterIndex = 1;
                            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

                            if (GetOpenFileNameA(&ofn)) {
                                strncpy_s(UIConfig::srReplayPath, filename, IM_ARRAYSIZE(UIConfig::srReplayPath) - 1);
                                UIConfig::srReplayPath[IM_ARRAYSIZE(UIConfig::srReplayPath) - 1] = '\0';

                            }
                        }
                    }
                }
                

                ImGui::EndTabItem();
            }


            if (ImGui::BeginTabItem("Misc")) {
                ImGui::Checkbox("Server Switcher", &UIConfig::bServerSwitcher);
                if (UIConfig::bServerSwitcher) {
                    if (UIGlobals::serverIpStrings.empty())
                        ImGui::Text("No server IPs found. Please edit serverIps.txt to add some.");

                    int idx = UIConfig::iServerIpIndex;
                    if (idx < 0 || idx >= (int)UIGlobals::serverIpStrings.size())
                        idx = 0;

                    if (!UIGlobals::serverIpStrings.empty() &&
                        ImGui::BeginCombo("Select item", UIGlobals::serverIpStrings[idx].c_str())) {

                        for (int n = 0; n < (int)UIGlobals::serverIps.size(); n++) {
                            bool is_selected = (idx == n);
                            if (ImGui::Selectable(UIGlobals::serverIps[n], is_selected)) {
                                UIConfig::iServerIpIndex = n;
                                const char* newHost = UIGlobals::serverIps[n];
                                char buffer[16] = { 0 };
                                strncpy_s(buffer, newHost, sizeof(buffer) - 1);

                                uintptr_t targetAddr = reinterpret_cast<uintptr_t>(GetModuleHandleA("Hexis.exe")) + 0x4957F8;

                                DWORD oldProtect;
                                if (VirtualProtect((LPVOID)targetAddr, sizeof(buffer), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                                    memcpy((void*)targetAddr, buffer, sizeof(buffer));
                                    VirtualProtect((LPVOID)targetAddr, sizeof(buffer), oldProtect, &oldProtect);
                                }
                            }

                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    if (ImGui::Button("Edit IPs")) {
                        system("notepad serverIps.txt");
                        LoadIpsFromFile();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Refresh")) {
                        LoadIpsFromFile();
                        UIConfig::iServerIpIndex = 0;
                    }
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
		if (UIGlobals::bIsOpenGL) {
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
        else {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
    }
};