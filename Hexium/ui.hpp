#pragma once

#include "ImGui/imgui_impl_win32.h"
#pragma comment(lib, "opengl32.lib")
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_dx9.h"
namespace UIConfig {
    inline bool bDisableHD = false;
    inline bool bTimewarp = false;
    inline float flTimewarpSpeed = 1.0f;
}
namespace UIGlobals {
    inline bool bIsOpenGL = false;
	inline IDirect3DDevice9* pDevice = nullptr;
	inline HWND hwnd = nullptr;
}
class UI {
    public:
    bool Initialized = false;
    bool IsOpen = true;

    void Initialize(HWND hwnd) {
        if (Initialized) return;

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);
		if (UIGlobals::bIsOpenGL) {
			ImGui_ImplOpenGL3_Init();
		}
        Initialized = true;

        auto io = ImGui::GetIO();

        io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf",16.f);
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