#pragma once
#include <string>
#include <imgui.h>

// Base class for all phone apps.
// Each app must implement onDraw(). onOpen/onClose are optional.
class PhoneApp {
public:
    std::string id;
    std::string name;
    ImVec4 color = ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
    bool dock = false;
    int dockOrder = 99;

    virtual ~PhoneApp() = default;

    // Called when the app is opened
    virtual void onOpen() {}

    // Called when the app is closed (back button or phone closed)
    virtual void onClose() {}

    // Called every frame while the app is the active app.
    // Draw your ImGui widgets here.
    virtual void onDraw() = 0;
};
