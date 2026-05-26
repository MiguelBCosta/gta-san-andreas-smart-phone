#pragma once
#include "../PhoneApp.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>

// Simple placeholder apps to populate the home screen and dock.
// Each one just shows a "Coming soon" message in onDraw.








class MusicApp : public PhoneApp {
public:
    MusicApp() {
        id = "music"; icon = ICON_FA_MUSIC; name = "Musica";
        color = ImVec4(0.70f, 0.10f, 0.15f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Musica - Em breve"); }
};



class SettingsApp : public PhoneApp {
public:
    SettingsApp() {
        id = "settings"; icon = ICON_FA_COG; name = "Config";
        color = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Configuracoes - Em breve"); }
};

