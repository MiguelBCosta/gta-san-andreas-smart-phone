#pragma once
#include <vector>
#include "PhoneApp.h"
#include "providers/IStorageProvider.h"

class PhoneStorage {
public:
    PhoneStorage() = default;
    ~PhoneStorage() = default;

    void setStorageProvider(IStorageProvider* provider);
    void addApp(PhoneApp* app);

    // Callbacks to trigger save, load or wipe for all registered apps
    void onGameSave(int slot);
    void onGameLoad(int slot);
    void onNewGame();

private:
    IStorageProvider* m_provider = nullptr;
    std::vector<PhoneApp*> m_apps;
};
