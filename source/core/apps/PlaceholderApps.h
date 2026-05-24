#pragma once
#include "../PhoneApp.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>

// Simple placeholder apps to populate the home screen and dock.
// Each one just shows a "Coming soon" message in onDraw.


class CameraApp : public PhoneApp {
public:
    CameraApp() {
        id = "camera"; icon = ICON_FA_CAMERA; name = "Camera";
        color = ImVec4(0.30f, 0.30f, 0.35f, 1.0f);
        dock = true; dockOrder = 1;
    }
    void onDraw() override { ImGui::Text("Camera - Em breve"); }
};



class GarageApp : public PhoneApp {
public:
    GarageApp() {
        id = "garage"; icon = ICON_FA_CAR; name = "Garagem";
        color = ImVec4(0.20f, 0.65f, 0.85f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Garagem - Em breve"); }
};

class MapsApp : public PhoneApp {
public:
    MapsApp() {
        id = "maps"; icon = ICON_FA_MAP_MARKED_ALT; name = "Mapa";
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Mapa - Em breve"); }
};

class MessagesApp : public PhoneApp {
public:
    MessagesApp() {
        id = "messages"; icon = ICON_FA_COMMENT; name = "SMS";
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
        dock = true; dockOrder = 2;
    }
    void onDraw() override { ImGui::Text("Mensagens - Em breve"); }
};

class MusicApp : public PhoneApp {
public:
    MusicApp() {
        id = "music"; icon = ICON_FA_MUSIC; name = "Musica";
        color = ImVec4(0.70f, 0.10f, 0.15f, 1.0f);
        dock = true; dockOrder = 3;
    }
    void onDraw() override { ImGui::Text("Musica - Em breve"); }
};

class PhoneCallApp : public PhoneApp {
public:
    PhoneCallApp() {
        id = "phone"; icon = ICON_FA_PHONE; name = "Ligar";
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
        dock = true; dockOrder = 4;
    }
    void onDraw() override { ImGui::Text("Telefone - Em breve"); }
};

class SettingsApp : public PhoneApp {
public:
    SettingsApp() {
        id = "settings"; icon = ICON_FA_COG; name = "Config";
        color = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Configuracoes - Em breve"); }
};

