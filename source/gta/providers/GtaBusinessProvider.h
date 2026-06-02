#pragma once
#include "../../core/providers/IBusinessProvider.h"
#include <game_sa/CPickups.h>
#include <game_sa/CWorld.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/common.h>
#include <IconsFontAwesome5.h>
#include <cmath>
#include <algorithm>

class GtaBusinessProvider : public IBusinessProvider {
private:
    struct PredefinedBusiness {
        std::string id;
        std::string name;
        std::string description;
        const char* icon; // Use FontAwesome macro
        int cost;
        int maxProfit;
        int dailyYield;
        float x, y, z;
    };

    std::vector<PredefinedBusiness> m_predefined = {
        {"grove", "Casa do CJ", "Rendimento do territorio da Grove Street.", ICON_FA_HOME, 0, 10000, 10000, 2495.2f, -1691.0f, 10.2f},
        {"robois", "Roboi's Food Mart", "Ponto de entregas em Los Santos.", ICON_FA_SHOPPING_CART, 10000, 2000, 2000, 1368.5f, -1279.8f, 13.5f},
        {"zero", "Zero's RC Shop", "Loja de aeromodelos em San Fierro.", ICON_FA_HELICOPTER, 30000, 5000, 5000, -2244.0f, 128.5f, 35.3f},
        {"wang", "Wang Cars", "Concessionaria em San Fierro.", ICON_FA_CAR, 50000, 8000, 8000, -1986.5f, 287.0f, 35.1f},
        {"hippy", "Hippy Shopper", "Loja de conveniencia em San Fierro.", ICON_FA_STORE, 12000, 2000, 2000, -2462.5f, 773.0f, 35.1f},
        {"vank_hoff", "Vank Hoff Hotel", "Servico de manobrista em San Fierro.", ICON_FA_HOTEL, 20000, 2000, 2000, -2375.0f, 1032.5f, 50.4f},
        {"rs_haul", "RS Haul", "Empresa de fretes em Flint County.", ICON_FA_TRUCK, 25000, 2000, 2000, -1010.5f, -1565.0f, 32.5f},
        {"quarry", "Hunter Quarry", "Pedreira industrial em Las Venturas.", ICON_FA_INDUSTRY, 40000, 10000, 10000, 593.5f, 865.0f, -42.5f},
        {"airstrip", "Verdant Meadows", "Aerodromo privado no deserto.", ICON_FA_PLANE, 80000, 10000, 10000, 402.5f, 2508.0f, 16.5f},
        {"burger", "Burger Shot LV", "Franquia de fast food em Las Venturas.", ICON_FA_HAMBURGER, 15000, 2000, 2000, 1872.5f, 2072.0f, 10.8f}
    };

    CPickup* FindPickupNear(float x, float y, float z, float radius = 30.0f) {
        for (int i = 0; i < 620; i++) {
            CPickup& p = CPickups::aPickUps[i];
            if (p.m_nPickupType == 0) continue; // PICKUP_NONE
            
            CVector pos = p.GetPosn();
            float dx = pos.x - x;
            float dy = pos.y - y;
            float dz = pos.z - z;
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < radius) {
                return &p;
            }
        }
        return nullptr;
    }

public:
    GtaBusinessProvider() = default;
    ~GtaBusinessProvider() override = default;

    std::vector<BusinessInfo> GetBusinesses() override {
        std::vector<BusinessInfo> result;
        for (const auto& pb : m_predefined) {
            BusinessInfo bi;
            bi.id = pb.id;
            bi.name = pb.name;
            bi.description = pb.description;
            bi.icon = pb.icon;
            bi.cost = pb.cost;
            bi.maxProfit = pb.maxProfit;
            bi.dailyYield = pb.dailyYield;
            
            CPickup* p = FindPickupNear(pb.x, pb.y, pb.z);
            if (p) {
                // Type 16 is PICKUP_ASSET_REVENUE
                if (p->m_nPickupType == 16) {
                    bi.owned = true;
                    bi.currentProfit = static_cast<int>(p->m_fRevenueValue);
                    if (p->m_nMoneyPerDay > 0) {
                        bi.dailyYield = p->m_nMoneyPerDay;
                    }
                } else if (p->m_nPickupType == 18) { // PICKUP_PROPERTY_FORSALE
                    bi.owned = false;
                    bi.currentProfit = 0;
                } else {
                    bi.owned = false;
                    bi.currentProfit = 0;
                }
            } else {
                // If it is CJ's House / Grove, we default it to owned if GGS is far enough or always owned
                if (pb.id == "grove") {
                    bi.owned = true;
                    bi.currentProfit = 0;
                } else {
                    bi.owned = false;
                    bi.currentProfit = 0;
                }
            }
            result.push_back(bi);
        }
        return result;
    }

    void CollectProfit(const std::string& id) override {
        for (const auto& pb : m_predefined) {
            if (pb.id == id) {
                CPickup* p = FindPickupNear(pb.x, pb.y, pb.z);
                if (p && p->m_nPickupType == 16 && p->m_fRevenueValue > 0.0f) {
                    CPlayerPed* player = FindPlayerPed();
                    if (player) {
                        CWorld::Players[0].m_nMoney += static_cast<int>(p->m_fRevenueValue);
                        p->m_fRevenueValue = 0.0f;
                    }
                }
                break;
            }
        }
    }

    void CollectAllProfits() override {
        for (const auto& pb : m_predefined) {
            CPickup* p = FindPickupNear(pb.x, pb.y, pb.z);
            if (p && p->m_nPickupType == 16 && p->m_fRevenueValue > 0.0f) {
                CPlayerPed* player = FindPlayerPed();
                if (player) {
                    CWorld::Players[0].m_nMoney += static_cast<int>(p->m_fRevenueValue);
                    p->m_fRevenueValue = 0.0f;
                }
            }
        }
    }
};
