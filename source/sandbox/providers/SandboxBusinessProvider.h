#pragma once
#include "../../core/providers/IBusinessProvider.h"
#include "../../core/providers/IStatsProvider.h"
#include "../../core/ServiceContainer.h"
#include "../../core/LocalizationManager.h"
#include <IconsFontAwesome5.h>
#include <vector>
#include <string>
#include <algorithm>

class SandboxBusinessProvider : public IBusinessProvider {
private:
    std::vector<BusinessInfo> m_businesses;
    std::vector<float> m_profitFloats;
    Inject<IStatsProvider> m_statsProvider;

public:
    SandboxBusinessProvider() {
        m_businesses = {
            {"grove", "business.name.grove", "business.desc.grove", ICON_FA_HOME, 0, 10000, 10000, true, 0, true},
            {"robois", "business.name.robois", "business.desc.robois", ICON_FA_SHOPPING_CART, 10000, 2000, 2000, false, 0, true},
            {"zero", "business.name.zero", "business.desc.zero", ICON_FA_HELICOPTER, 30000, 5000, 5000, false, 0, true},
            {"wang", "business.name.wang", "business.desc.wang", ICON_FA_CAR, 50000, 8000, 8000, false, 0, true},
            {"hippy", "business.name.hippy", "business.desc.hippy", ICON_FA_STORE, 12000, 2000, 2000, false, 0, true},
            {"vank_hoff", "business.name.vank_hoff", "business.desc.vank_hoff", ICON_FA_HOTEL, 20000, 2000, 2000, false, 0, true},
            {"rs_haul", "business.name.rs_haul", "business.desc.rs_haul", ICON_FA_TRUCK, 25000, 2000, 2000, false, 0, true},
            {"quarry", "business.name.quarry", "business.desc.quarry", ICON_FA_INDUSTRY, 40000, 10000, 10000, false, 0, true},
            {"airstrip", "business.name.airstrip", "business.desc.airstrip", ICON_FA_PLANE, 80000, 10000, 10000, false, 0, true},
            {"burger", "business.name.burger", "business.desc.burger", ICON_FA_HAMBURGER, 15000, 2000, 2000, false, 0, true}
        };
        m_profitFloats.resize(m_businesses.size(), 0.0f);
    }

    ~SandboxBusinessProvider() override = default;

    std::vector<BusinessInfo> GetBusinesses() override {
        std::vector<BusinessInfo> result = m_businesses;
        for (auto& b : result) {
            b.name = TR(b.name.c_str());
            b.description = TR(b.description.c_str());
        }
        return result;
    }

    void CollectProfit(const std::string& id) override {
        for (size_t i = 0; i < m_businesses.size(); i++) {
            if (m_businesses[i].id == id) {
                if (m_businesses[i].owned && m_businesses[i].currentProfit > 0) {
                    if (m_statsProvider.isValid()) {
                        m_statsProvider->AddPlayerMoney(m_businesses[i].currentProfit);
                    }
                    m_businesses[i].currentProfit = 0;
                    m_profitFloats[i] = 0.0f;
                }
                break;
            }
        }
    }

    void CollectAllProfits() override {
        int total = 0;
        for (size_t i = 0; i < m_businesses.size(); i++) {
            if (m_businesses[i].owned && m_businesses[i].currentProfit > 0) {
                total += m_businesses[i].currentProfit;
                m_businesses[i].currentProfit = 0;
                m_profitFloats[i] = 0.0f;
            }
        }
        if (total > 0 && m_statsProvider.isValid()) {
            m_statsProvider->AddPlayerMoney(total);
        }
    }

    void Update(float dt) override {
        // Accelerate sandbox accumulation by 60x so players can see it rising during manual testing
        float timeMultiplier = 60.0f; 
        for (size_t i = 0; i < m_businesses.size(); i++) {
            auto& b = m_businesses[i];
            if (b.owned && b.currentProfit < b.maxProfit) {
                // Rate: Daily yield accumulated over 1440 seconds (24 minutes)
                m_profitFloats[i] += (b.dailyYield / 1440.0f) * dt * timeMultiplier;
                if (m_profitFloats[i] >= 1.0f) {
                    int whole = static_cast<int>(m_profitFloats[i]);
                    b.currentProfit = (std::min)(b.maxProfit, b.currentProfit + whole);
                    m_profitFloats[i] -= whole;
                }
            }
        }
    }


    // Helper for Sandbox buying simulation
    void SetOwned(const std::string& id, bool owned) {
        for (auto& b : m_businesses) {
            if (b.id == id) {
                b.owned = owned;
                if (!owned) {
                    b.currentProfit = 0;
                }
                break;
            }
        }
    }

    bool CanPurchaseInApp() override { return true; }
    
    void PurchaseBusiness(const std::string& id) override {
        for (auto& b : m_businesses) {
            if (b.id == id && !b.owned) {
                if (m_statsProvider.isValid() && m_statsProvider->GetPlayerStats().money >= b.cost) {
                    m_statsProvider->AddPlayerMoney(-b.cost);
                    b.owned = true;
                }
                break;
            }
        }
    }
};

