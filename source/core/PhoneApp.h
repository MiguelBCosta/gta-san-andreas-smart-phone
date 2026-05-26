#pragma once
#include <string>
#include <imgui.h>
#include <nlohmann/json.hpp>

// Base class for all phone apps.
// Each app must implement onDraw(). onOpen/onClose are optional.
class PhoneApp {
public:
    std::string id;
    std::string icon = "";
    std::string name;
    ImVec4 color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    bool dock = false;
    int dockOrder = 99;
    int badgeCount = 0;

    virtual ~PhoneApp() = default;

    // Whether the app requires mouse interaction
    virtual bool requiresMouse() const { return true; }

    // Whether the app draws a solid background overlay when open
    virtual bool hasSolidBackground() const { return true; }

    // Whether the phone chassis should be completely hidden when this app is open
    virtual bool hidePhoneChassis() const { return false; }

    // Called when the app is opened
    virtual void onOpen() {}

    // Called when the native back button is clicked.
    // Returns true if the app handled it internally, false to let the phone close the app.
    virtual bool onBack() { return false; }

    // Called when the app is closed (back button or phone closed)
    virtual void onClose() {}

    // Called every frame while the app is the active app.
    // Draw your ImGui widgets here.
    virtual void onDraw() = 0;

    // Called every frame on the main game loop thread.
    virtual void update(float dt) {}

    // ---- Persistence callbacks (optional) ----
    
    // Called when the game/session is saved. Write your data into 'out'.
    virtual void onSave(nlohmann::json& out) {}

    // Called when a saved game/session is loaded. Read your data from 'in'.
    virtual void onLoad(const nlohmann::json& in) {}

    // Called when a new game starts or the device is wiped. Reset your app's state.
    virtual void onWipe() {}
};

