#pragma once
#include <vector>
#include "PhoneApp.h"
#include "providers/IStorageProvider.h"

class Phone;

class PhoneStorage {
public:
    PhoneStorage() = default;
    ~PhoneStorage() = default;

    void setStorageProvider(IStorageProvider* provider);
    void setPhone(Phone* phone);
    void addApp(PhoneApp* app);

    // Callbacks to trigger save, load or wipe for all registered apps
    void onGameSave(int slot);
    void onGameLoad(int slot);
    void onNewGame();

private:
    IStorageProvider* m_provider = nullptr;
    Phone* m_phone = nullptr;
    std::vector<PhoneApp*> m_apps;
};
