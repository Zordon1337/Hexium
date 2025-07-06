#pragma once

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_dx9.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
namespace UIConfig {
    inline bool bDisableHD = false;
    inline bool bTimewarp = false;
    inline float flTimewarpSpeed = 1.0f;
    inline bool bServerSwitcher = false;
	inline int iServerIpIndex = 0;
}
namespace UIGlobals {
    inline bool bIsOpenGL = false;
	inline IDirect3DDevice9* pDevice = nullptr;
	inline HWND hwnd = nullptr;
    inline std::vector<const char*> serverIps;
    inline std::vector<std::string> serverIpStrings;
}
class UI {
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
        
        ImGui::Checkbox("Disable HD (Requires map restart)", &UIConfig::bDisableHD);
        ImGui::Spacing();
        ImGui::Checkbox("Timewarp", &UIConfig::bTimewarp);
        if (UIConfig::bTimewarp) {
            ImGui::SliderFloat("Speed", &UIConfig::flTimewarpSpeed, 0.1f, 2.f);
        }
		ImGui::Checkbox("Server Switcher", &UIConfig::bServerSwitcher);
		if (UIConfig::bServerSwitcher) {
			if (UIGlobals::serverIpStrings.size() < 1)
				ImGui::Text("No server IPs found. Please edit serverIps.txt to add some.");
            int idx = UIConfig::iServerIpIndex;
            if (idx < 0 || idx >= (int)UIGlobals::serverIpStrings.size())
                idx = 0;
            if (ImGui::BeginCombo("Select item", UIGlobals::serverIpStrings[idx].c_str())) {
                for (int n = 0; n < (int)UIGlobals::serverIps.size(); n++) {
                    bool is_selected = (idx == n);
                    if (ImGui::Selectable(UIGlobals::serverIps[n], is_selected))
                    {
                        UIConfig::iServerIpIndex = n;
                        const char* newHost = UIGlobals::serverIps[n];
                        char buffer[16] = { 0 };

                        strncpy_s(buffer, newHost, sizeof(buffer) - 1);

                        uintptr_t targetAddr = reinterpret_cast<uintptr_t>(GetModuleHandleA("Hexis.exe")) + 0x4957F8;

                        DWORD oldProtect;
                        VirtualProtect((LPVOID)targetAddr, 16, PAGE_EXECUTE_READWRITE, &oldProtect);
                        memcpy((void*)targetAddr, buffer, 16);
                        VirtualProtect((LPVOID)targetAddr, 16, oldProtect, &oldProtect);

                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

			if (ImGui::Button("Edit IPs")) {
				system("notepad serverIps.txt"); // it waits for user to quit, should it? idk
                LoadIpsFromFile();
			}
            if (ImGui::Button("Refresh")) {
				LoadIpsFromFile();
				UIConfig::iServerIpIndex = 0;
            }
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