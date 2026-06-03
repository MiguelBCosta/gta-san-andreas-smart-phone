#pragma once
#include "../PhoneApp.h"
#include <IconsFontAwesome5.h>

class ProfileApp : public PhoneApp {
private:
    int m_activeSubmenu = 0; // 0: Main Screen, 1: Jogador, 2: Habilidades, 3: Gang & Respeito, 4: Crimes & Wanted, 5: Progresso

    // Submenu drawing helper functions
    void drawMainMenu();
    void drawJogadorMenu();
    void drawHabilidadesMenu();
    void drawGangMenu();
    void drawCrimesMenu();
    void drawProgressoMenu();

    // Custom UI helpers
    void drawStatRow(const char* label, const char* value, const char* icon = nullptr, ImVec4 iconColor = ImVec4(1,1,1,1));
    void drawStatProgressBar(const char* label, float percentage, const char* icon = nullptr, ImVec4 iconColor = ImVec4(1,1,1,1));

public:
    ProfileApp();
    ~ProfileApp() override = default;

    void onOpen() override;
    void onClose() override;
    bool onBack() override;
    void onDraw() override;
    void onLanguageChange() override { name = TR("profile.title"); }
};
