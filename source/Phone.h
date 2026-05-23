#pragma once
#include <vector>
#include <algorithm>
#include <imgui.h>
#include "PhoneApp.h"

class Phone {
public:
    // ---- State ----
    bool visible = false;
    PhoneApp* currentApp = nullptr;
    std::vector<PhoneApp*> apps;

    // ---- Layout constants (from MoonPhone.lua) ----
    static constexpr float PH_PADDING_X = 20.0f;
    static constexpr float PH_PADDING_Y = 20.0f;
    static constexpr float PH_W  = 320.0f;
    static constexpr float PH_H  = 620.0f;
    static constexpr float BEZEL = 12.0f;
    static constexpr float WIN_R = 36.0f;
    static constexpr float SCR_R = 26.0f;

    // Notch (Dynamic Island)
    static constexpr float NI_W = 108.0f;
    static constexpr float NI_H = 26.0f;
    static constexpr float NI_R = 13.0f;

    // App grid
    static constexpr float ICON_SZ      = 58.0f;
    static constexpr float ICON_R       = 14.0f;
    static constexpr int   GRID_COLS    = 4;
    static constexpr float GRID_GAP_Y   = 18.0f;
    static constexpr float LABEL_H      = 16.0f;
    static constexpr float GRID_START_Y = 88.0f;

    // Derived horizontal layout
    static constexpr float SCREEN_W    = PH_W - 2.0f * BEZEL;           // 296
    static constexpr float INNER_PAD_X = 8.0f;
    static constexpr float INNER_W     = SCREEN_W - 2.0f * INNER_PAD_X; // 280
    static constexpr float GRID_GAP_X  = (INNER_W - GRID_COLS * ICON_SZ) / (GRID_COLS - 1);
    static constexpr float GRID_START_X = BEZEL + INNER_PAD_X;

    // Dock
    static constexpr float DOCK_H       = 90.0f;
    static constexpr float DOCK_ICON_SZ = 52.0f;
    static constexpr float DOCK_ICON_R  = 12.0f;
    static constexpr float DOCK_GAP     = 16.0f;
    static constexpr float DOCK_Y       = PH_H - BEZEL - DOCK_H;

    // ---- Public API ----
    void registerApp(PhoneApp* app);
    void openApp(PhoneApp* app);
    void closeApp();
    void closePhone();

    // Call this every frame from EndScene when visible == true
    void draw();

private:
    void drawHome(ImDrawList* draw, ImVec2 winPos);
    void drawCurrentApp(ImDrawList* draw, ImVec2 winPos);
    bool drawIcon(PhoneApp* app, ImDrawList* draw, ImVec2 winPos,
                  float curX, float curY, float sz, float rounding, const char* btnId);
};
