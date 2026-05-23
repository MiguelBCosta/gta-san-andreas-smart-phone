#pragma once
#include "PhoneApp.h"
#include <imgui.h>

// Simple placeholder apps to populate the home screen and dock.
// Each one just shows a "Coming soon" message in onDraw.

class CalculatorApp : public PhoneApp {
public:
    CalculatorApp() {
        id = "calculator"; name = "Calc";
        color = ImVec4(0.35f, 0.35f, 0.40f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Calculadora - Em breve"); }
};

class CameraApp : public PhoneApp {
public:
    CameraApp() {
        id = "camera"; name = "Camera";
        color = ImVec4(0.30f, 0.30f, 0.35f, 1.0f);
        dock = true; dockOrder = 1;
    }
    void onDraw() override { ImGui::Text("Camera - Em breve"); }
};

class ClockApp : public PhoneApp {
public:
    ClockApp() {
        id = "clock"; name = "Relogio";
        color = ImVec4(0.85f, 0.48f, 0.08f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Relogio - Em breve"); }
};

class GarageApp : public PhoneApp {
public:
    GarageApp() {
        id = "garage"; name = "Garagem";
        color = ImVec4(0.20f, 0.65f, 0.85f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Garagem - Em breve"); }
};

class MapsApp : public PhoneApp {
public:
    MapsApp() {
        id = "maps"; name = "Mapa";
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Mapa - Em breve"); }
};

class MessagesApp : public PhoneApp {
public:
    MessagesApp() {
        id = "messages"; name = "SMS";
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
        dock = true; dockOrder = 2;
    }
    void onDraw() override { ImGui::Text("Mensagens - Em breve"); }
};

class MusicApp : public PhoneApp {
public:
    MusicApp() {
        id = "music"; name = "Musica";
        color = ImVec4(0.70f, 0.10f, 0.15f, 1.0f);
        dock = true; dockOrder = 3;
    }
    void onDraw() override { ImGui::Text("Musica - Em breve"); }
};

class PhoneCallApp : public PhoneApp {
public:
    PhoneCallApp() {
        id = "phone"; name = "Ligar";
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
        dock = true; dockOrder = 4;
    }
    void onDraw() override { ImGui::Text("Telefone - Em breve"); }
};

class SettingsApp : public PhoneApp {
public:
    SettingsApp() {
        id = "settings"; name = "Config";
        color = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Configuracoes - Em breve"); }
};

class WeatherApp : public PhoneApp {
public:
    WeatherApp() {
        id = "weather"; name = "Clima";
        color = ImVec4(0.40f, 0.70f, 0.90f, 1.0f);
    }
    void onDraw() override { ImGui::Text("Clima - Em breve"); }
};
