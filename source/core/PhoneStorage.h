#pragma once
#include <vector>
#include "PhoneApp.h"
#include "providers/IStorageProvider.h"
#include "ServiceContainer.h"

class Phone;

class PhoneStorage {
public:
    PhoneStorage() = default;
    ~PhoneStorage() = default;

    void setPhone(Phone* phone);
    void addApp(PhoneApp* app);

    // Callbacks to trigger save, load or wipe for all registered apps
    void onGameSave(int slot);
    void onGameLoad(int slot);
    void onNewGame();

private:
    Inject<IStorageProvider> m_provider;
    Phone* m_phone = nullptr;
    std::vector<PhoneApp*> m_apps;
};
