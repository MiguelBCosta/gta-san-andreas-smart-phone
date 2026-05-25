#include "PhoneStorage.h"
#include <nlohmann/json.hpp>

void PhoneStorage::setStorageProvider(IStorageProvider* provider) {
    m_provider = provider;
}

void PhoneStorage::addApp(PhoneApp* app) {
    if (app) {
        m_apps.push_back(app);
    }
}

void PhoneStorage::onGameSave(int slot) {
    if (!m_provider) return;

    nlohmann::json root;
    root["version"] = 1;
    
    nlohmann::json appsJson = nlohmann::json::object();
    for (auto* app : m_apps) {
        nlohmann::json appData = nlohmann::json::object();
        app->onSave(appData);
        // Only store if the app wrote something, or store it anyway if we want to ensure key exists
        if (!appData.empty()) {
            appsJson[app->id] = appData;
        }
    }
    root["apps"] = appsJson;

    m_provider->WriteSlotData(slot, root.dump(4));
}

void PhoneStorage::onGameLoad(int slot) {
    if (!m_provider) return;

    std::string data = m_provider->ReadSlotData(slot);
    if (data.empty()) {
        // No save exists for this slot, wipe memory to start clean
        onNewGame();
        return;
    }

    try {
        nlohmann::json root = nlohmann::json::parse(data);
        nlohmann::json appsJson;
        if (root.contains("apps")) {
            appsJson = root["apps"];
        }

        for (auto* app : m_apps) {
            if (!appsJson.is_null() && appsJson.contains(app->id)) {
                app->onLoad(appsJson[app->id]);
            } else {
                // If this app has no saved data in this slot, make sure it is wiped/reset
                app->onWipe();
            }
        }
    } catch (const std::exception&) {
        // JSON parsing error, wipe memory to be safe
        onNewGame();
    }
}

void PhoneStorage::onNewGame() {
    for (auto* app : m_apps) {
        app->onWipe();
    }
}
