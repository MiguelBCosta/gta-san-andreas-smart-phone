#pragma once
#include "../PhoneApp.h"
#include <IconsFontAwesome5.h>
#include <unordered_map>
#include <string>

class StoreApp : public PhoneApp {
private:
    struct InstallProgress {
        bool isInstalling = false;
        float progress = 0.0f;
    };

    std::unordered_map<std::string, InstallProgress> m_installStates;

public:
    StoreApp();

    void onOpen() override;
    void onClose() override;
    void update(float dt) override;
    void onDraw() override;
    void onLanguageChange() override { name = TR("appstore.title"); }
};
