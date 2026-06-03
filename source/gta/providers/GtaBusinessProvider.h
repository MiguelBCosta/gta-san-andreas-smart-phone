#pragma once
#include "../../core/providers/IBusinessProvider.h"
#include "../../core/LocalizationManager.h"
#include <game_sa/CPickups.h>
#include <game_sa/CWorld.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/common.h>
#include <IconsFontAwesome5.h>
#include <algorithm>
#include <limits>

class GtaBusinessProvider : public IBusinessProvider {
private:
    struct PredefinedBusiness {
        std::string id;
        std::string name;
        std::string description;
        const char* icon;
        int cost;
        int maxProfit;
        int dailyYield;
        float x, y; // Only X/Y needed for matching
    };

    std::vector<PredefinedBusiness> m_predefined = {
        {"grove",    "empresas.name.grove",    "empresas.desc.grove",    ICON_FA_HOME,         0,     10000, 10000,  2498.382f, -1686.124f},
        {"robois",   "empresas.name.robois",   "empresas.desc.robois",   ICON_FA_SHOPPING_CART,10000,  2000,  2000,  1356.000f, -1757.813f},
        {"zero",     "empresas.name.zero",     "empresas.desc.zero",     ICON_FA_HELICOPTER,  30000,  5000,  5000, -2242.663f,   136.890f},
        {"wang",     "empresas.name.wang",     "empresas.desc.wang",     ICON_FA_CAR,         50000,  8000,  8000, -1969.186f,   282.167f},
        {"hippy",    "empresas.name.hippy",    "empresas.desc.hippy",    ICON_FA_STORE,       12000,  2000,  2000, -2593.539f,    59.140f},
        {"vank_hoff","empresas.name.vank_hoff","empresas.desc.vank_hoff",ICON_FA_HOTEL,       20000,  2000,  2000, -1753.717f,   958.960f},
        {"rs_haul",  "empresas.name.rs_haul",  "empresas.desc.rs_haul",  ICON_FA_TRUCK,       25000,  2000,  2000,   -79.584f, -1135.762f},
        {"quarry",   "empresas.name.quarry",   "empresas.desc.quarry",   ICON_FA_INDUSTRY,    40000, 10000, 10000,   819.781f,   855.206f},
        {"airstrip", "empresas.name.airstrip", "empresas.desc.airstrip", ICON_FA_PLANE,       80000, 10000, 10000,   426.234f,  2530.609f},
        {"burger",   "empresas.name.burger",   "empresas.desc.burger",   ICON_FA_HAMBURGER,   15000,  2000,  2000,  1888.564f,  2086.109f}
    };

    // Returns the index into m_predefined nearest to the given X/Y position.
    // Uses squared 2D distance — no radius cap — to always find the closest.
    int NearestBusinessIndex(float px, float py) const {
        int best = -1;
        float bestDist = std::numeric_limits<float>::max();
        for (int i = 0; i < static_cast<int>(m_predefined.size()); i++) {
            float dx = px - m_predefined[i].x;
            float dy = py - m_predefined[i].y;
            float d = dx*dx + dy*dy;
            if (d < bestDist) {
                bestDist = d;
                best = i;
            }
        }
        return best;
    }

    // Scans the full global pickup pool (620 slots) and returns the type-16
    // pickup associated with the predefined business at index |idx|, or null.
    // This is used by Collect* which already knows the business id.
    CPickup* FindRevenuePickupForIndex(int idx) {
        if (idx < 0 || idx >= static_cast<int>(m_predefined.size())) return nullptr;
        const PredefinedBusiness& pb = m_predefined[idx];

        CPickup* best = nullptr;
        float bestDist = std::numeric_limits<float>::max();

        for (int i = 0; i < 620; i++) {
            CPickup& p = CPickups::aPickUps[i];
            if (p.m_nPickupType != 16) continue; // Only PICKUP_ASSET_REVENUE
            CVector pos = p.GetPosn();
            float dx = pos.x - pb.x;
            float dy = pos.y - pb.y;
            float d = dx*dx + dy*dy;
            if (d < bestDist) {
                bestDist = d;
                best = &p;
            }
        }
        return best;
    }

    int FindPredefinedIndexById(const std::string& id) const {
        for (int i = 0; i < static_cast<int>(m_predefined.size()); i++) {
            if (m_predefined[i].id == id) return i;
        }
        return -1;
    }

public:
    GtaBusinessProvider() = default;
    ~GtaBusinessProvider() override = default;

    std::vector<BusinessInfo> GetBusinesses() override {
        // Build result map pre-populated with defaults (not owned)
        std::vector<BusinessInfo> result(m_predefined.size());
        for (int i = 0; i < static_cast<int>(m_predefined.size()); i++) {
            const auto& pb = m_predefined[i];
            result[i].id          = pb.id;
            result[i].name        = TR(pb.name.c_str());
            result[i].description = TR(pb.description.c_str());
            result[i].icon        = pb.icon;
            result[i].cost        = pb.cost;
            result[i].maxProfit   = pb.maxProfit;
            result[i].dailyYield  = pb.dailyYield;
            result[i].owned       = false;
            result[i].currentProfit = 0;
        }

        // Single pass over the entire global pickup pool.
        // For each relevant pickup, find the nearest predefined business and assign it.
        for (int i = 0; i < 620; i++) {
            CPickup& p = CPickups::aPickUps[i];

            // Only care about asset-revenue (16) and property-forsale (18) pickups.
            if (p.m_nPickupType != 16 && p.m_nPickupType != 18) continue;

            CVector pos = p.GetPosn();
            int idx = NearestBusinessIndex(pos.x, pos.y);
            if (idx < 0) continue;

            if (p.m_nPickupType == 16) { // PICKUP_ASSET_REVENUE
                result[idx].owned        = true;
                result[idx].currentProfit = static_cast<int>(p.m_fRevenueValue);
                if (p.m_nMoneyPerDay > 0) {
                    result[idx].dailyYield = p.m_nMoneyPerDay;
                }
            }
            // type 18 = for sale: owned stays false — already set by default
        }

        // Grove Street fallback: always owned. But if no revenue pickup was found, set unlocked = false.
        {
            int groveIdx = FindPredefinedIndexById("grove");
            if (groveIdx >= 0) {
                if (!result[groveIdx].owned) {
                    result[groveIdx].owned = true;
                    result[groveIdx].currentProfit = 0;
                    result[groveIdx].unlocked = false; // Not unlocked (insufficient reputation)
                } else {
                    result[groveIdx].unlocked = true;
                }
            }
        }

        return result;
    }

    void CollectProfit(const std::string& id) override {
        int idx = FindPredefinedIndexById(id);
        CPickup* p = FindRevenuePickupForIndex(idx);
        if (p && p->m_fRevenueValue > 0.0f) {
            CWorld::Players[0].m_nMoney += static_cast<int>(p->m_fRevenueValue);
            p->m_fRevenueValue = 0.0f;
        }
    }

    void CollectAllProfits() override {
        for (int i = 0; i < static_cast<int>(m_predefined.size()); i++) {
            CPickup* p = FindRevenuePickupForIndex(i);
            if (p && p->m_fRevenueValue > 0.0f) {
                CWorld::Players[0].m_nMoney += static_cast<int>(p->m_fRevenueValue);
                p->m_fRevenueValue = 0.0f;
            }
        }
    }
};
