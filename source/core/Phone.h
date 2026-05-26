#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <imgui.h>
#include "PhoneApp.h"
#include "PhoneStorage.h"
#include "providers/IClockProvider.h"
#include "providers/IScreenProvider.h"
#include "providers/ICameraProvider.h"

enum class PhoneAnimMode {
    FORCED,
    SMOOTH
};

class IPhoneCallProvider;
class IAvatarProvider;

class Phone {
public:
    Phone();
    PhoneStorage& getStorage() { return m_storage; }

    // ---- State ----
    
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
    static constexpr float NI_H = 22.0f;
    static constexpr float NI_R = 11.0f;

    // App grid
    static constexpr float ICON_SZ      = 58.0f;
    static constexpr float ICON_R       = 14.0f;
    static constexpr int   GRID_COLS    = 4;
    static constexpr float GRID_GAP_Y   = 18.0f;
    static constexpr float LABEL_H      = 16.0f;
    static constexpr float GRID_START_Y = 65.0f;

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

    // Header & Content Constants
    static constexpr float HEADER_Y_POS     = 44.0f;
    static constexpr float HEADER_BTN_W     = 80.0f;
    static constexpr float HEADER_BTN_H     = 30.0f;
    static constexpr float HEADER_TITLE_Y   = 50.0f;
    static constexpr float HEADER_SEP_Y     = 78.0f;
    static constexpr float CONTENT_Y_OFFSET = 82.0f;
    static constexpr float STATUS_BAR_Y     = BEZEL + 7.0f;

    // ---- Public API ----
    void setClockProvider(IClockProvider* provider);
    IClockProvider* getClockProvider() const { return m_clockProvider; }
    void setScreenProvider(IScreenProvider* provider);
    IScreenProvider* getScreenProvider() const { return m_screenProvider; }
    void setCallProvider(IPhoneCallProvider* provider);
    IPhoneCallProvider* getCallProvider() const { return m_callProvider; }
    void setAvatarProvider(IAvatarProvider* provider);
    IAvatarProvider* getAvatarProvider() const { return m_avatarProvider; }
    void setCameraProvider(ICameraProvider* provider);
    ICameraProvider* getCameraProvider() const { return m_cameraProvider; }

    void registerApp(PhoneApp* app);
    void openApp(PhoneApp* app);
    void closeApp();
    
    // Animation API
    void toggle(PhoneAnimMode mode);
    void open(PhoneAnimMode mode);
    void close(PhoneAnimMode mode);
    void update(float dt);
    void process(float dt);
    bool isVisible() const;
    bool shouldCaptureInput() const;
    bool isOpen() const { return m_isOpen; }
    PhoneApp* getCurrentApp() const { return m_currentApp; }

    // Call this every frame when isVisible() == true
    void draw();

    void resetDefaultLayout();

    struct AppVisualState {
        float x = -1.0f;
        float y = -1.0f;
        bool isDragging = false;
    };

private:
    friend class PhoneStorage;

    std::vector<PhoneApp*> m_apps;
    std::vector<PhoneApp*> m_defaultApps;
    std::vector<PhoneApp*> m_dockApps;
    PhoneApp* m_currentApp = nullptr;
    IClockProvider* m_clockProvider = nullptr;
    IScreenProvider* m_screenProvider = nullptr;
    IPhoneCallProvider* m_callProvider = nullptr;
    IAvatarProvider* m_avatarProvider = nullptr;
    ICameraProvider* m_cameraProvider = nullptr;

    bool m_isOpen = false;
    float m_animProgress = 0.0f;
    PhoneStorage m_storage;

    // Drag and drop / edit mode states
    bool m_editMode = false;
    bool m_isDragging = false;
    std::string m_draggedKey = "";
    ImVec2 m_dragOffset = ImVec2(0, 0);
    std::string m_pressedKey = "";
    float m_pressTimer = 0.0f;
    float m_time = 0.0f;
    bool m_dragStartedThisClick = false;

    std::unordered_map<std::string, AppVisualState> m_visualStates;

    void drawHome(ImDrawList* draw, ImVec2 winPos);
    void drawHomeGrid(ImDrawList* draw, ImVec2 winPos);
    void drawHomeDock(ImDrawList* draw, ImVec2 winPos);
    void drawCurrentApp(ImDrawList* draw, ImVec2 winPos);
    void drawStatusBar();
    bool drawIcon(PhoneApp* app, ImDrawList* draw, ImVec2 winPos,
                  float curX, float curY, float sz, float rounding, const char* btnId, const std::string& key);
};
