#include "PhoneStorage.h"
#include "Phone.h"
#include <nlohmann/json.hpp>
#include <algorithm>

void PhoneStorage::setStorageProvider(IStorageProvider* provider) {
    m_provider = provider;
}

void PhoneStorage::setPhone(Phone* phone) {
    m_phone = phone;
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

    // Save layout ordering
    if (m_phone) {
        nlohmann::json layoutJson = nlohmann::json::object();
        
        nlohmann::json gridOrder = nlohmann::json::array();
        for (auto* app : m_phone->m_apps) {
            gridOrder.push_back(app->id);
        }
        layoutJson["grid"] = gridOrder;

        nlohmann::json dockOrder = nlohmann::json::array();
        for (auto* app : m_phone->m_dockApps) {
            dockOrder.push_back(app->id);
        }
        layoutJson["dock"] = dockOrder;

        root["layout"] = layoutJson;
    }

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

        // Load layout ordering
        if (m_phone && root.contains("layout")) {
            nlohmann::json layoutJson = root["layout"];
            
            if (layoutJson.contains("grid") && layoutJson["grid"].is_array()) {
                std::vector<std::string> gridOrder = layoutJson["grid"].get<std::vector<std::string>>();
                std::vector<PhoneApp*> newOrder;
                for (const auto& id : gridOrder) {
                    auto it = std::find_if(m_phone->m_apps.begin(), m_phone->m_apps.end(), [&](PhoneApp* a) {
                        return a->id == id;
                    });
                    if (it != m_phone->m_apps.end()) {
                        newOrder.push_back(*it);
                    }
                }
                // Append missing/new apps
                for (auto* app : m_phone->m_apps) {
                    if (std::find(newOrder.begin(), newOrder.end(), app) == newOrder.end()) {
                        newOrder.push_back(app);
                    }
                }
                m_phone->m_apps = newOrder;
            }

            if (layoutJson.contains("dock") && layoutJson["dock"].is_array()) {
                std::vector<std::string> dockOrder = layoutJson["dock"].get<std::vector<std::string>>();
                std::vector<PhoneApp*> newDock;
                for (const auto& id : dockOrder) {
                    auto it = std::find_if(m_phone->m_apps.begin(), m_phone->m_apps.end(), [&](PhoneApp* a) {
                        return a->id == id;
                    });
                    if (it != m_phone->m_apps.end()) {
                        newDock.push_back(*it);
                    }
                }
                m_phone->m_dockApps = newDock;
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
    if (m_phone) {
        m_phone->resetDefaultLayout();
    }
}
