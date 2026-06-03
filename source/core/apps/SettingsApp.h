#pragma once
#include "../PhoneApp.h"
#include "../providers/IWallpaperProvider.h"
#include "../ServiceContainer.h"
#include "../LocalizationManager.h"
#include <imgui.h>
#include <string>
#include <vector>

enum class WallpaperType {
    SOLID = 0,
    GRADIENT = 1,
    IMAGE = 2
};

struct WallpaperConfig {
    WallpaperType type = WallpaperType::GRADIENT;
    ImVec4 solidColor = ImVec4(0.08f, 0.35f, 0.18f, 1.0f); // Default solid green
    ImVec4 gradientColor1 = ImVec4(0.08f, 0.35f, 0.18f, 1.0f); // Default top dark green (Grove Street)
    ImVec4 gradientColor2 = ImVec4(0.02f, 0.10f, 0.05f, 1.0f); // Default bottom very dark green
    std::string imageName = "";
};

class SettingsApp : public PhoneApp {
private:
    WallpaperConfig m_config;
    std::vector<std::string> m_availableWallpapers;
    bool m_wallpapersLoaded = false;
    
    // UI Navigation State
    // 0: Settings List, 1: Wallpaper Menu, 2: Language Menu
    int m_menuState = 0;

    // Presets
    static const int PRESET_COUNT = 6;
    static const ImVec4 PRESET_SOLIDS[PRESET_COUNT];
    static const std::pair<ImVec4, ImVec4> PRESET_GRADIENTS[PRESET_COUNT];
    static const char* PRESET_GRADIENT_NAMES[PRESET_COUNT];

public:
    SettingsApp();

    void onOpen() override;
    void onClose() override;
    void onDraw() override;
    bool onBack() override;

    // Persistence
    void onSave(nlohmann::json& out) override;
    void onLoad(const nlohmann::json& in) override;
    void onWipe() override;

    // Wallpaper drawing helper called by Phone chassis renderer
    void DrawWallpaper(ImDrawList* draw, ImVec2 pMin, ImVec2 pMax, float rounding);
};
