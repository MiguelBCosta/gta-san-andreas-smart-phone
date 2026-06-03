#pragma once
#include "../../core/providers/IMapProvider.h"
#include "../../core/LocalizationManager.h"
#include <vector>
#include <string>

class SandboxMapProvider : public IMapProvider {
public:
    float m_playerX = 2495.0f;
    float m_playerY = -1687.0f;
    float m_playerHeading = 90.0f;

    bool m_waypointActive = false;
    float m_waypointX = 0.0f;
    float m_waypointY = 0.0f;

    struct SandboxBlip {
        float x;
        float y;
        int sprite;
        int color;
        std::string name;
        int id;
        bool revealed;
    };

    std::vector<SandboxBlip> m_blips;

    SandboxMapProvider() {
        m_blips = {
            { 2495.0f, -1687.0f, 35, 8, "maps.sandbox.blip.grove", 1, true },
            { -2026.0f, 138.0f, 35, 8, "maps.sandbox.blip.doherty", 2, false },
            { 2020.0f, 1007.0f, 44, 8, "maps.sandbox.blip.four_dragons", 3, false },
            { 2639.0f, -2040.0f, 27, 2, "maps.sandbox.blip.loco_low", 4, false },
            { 1367.0f, -1280.0f, 6, 0, "maps.sandbox.blip.ammu_ls", 5, false },
            { 2065.0f, -1831.0f, 63, 2, "maps.sandbox.blip.spray_idle", 6, false },
            { 2069.0f, -1698.0f, 7, 4, "maps.sandbox.blip.barber_idle", 7, false },
            { 1685.0f, -2238.0f, 5, 4, "maps.sandbox.blip.airport_ls", 8, false },
            { -2696.0f, 386.0f, 22, 1, "maps.sandbox.blip.hospital_sf", 9, false },
            { 1544.0f, -1675.0f, 30, 2, "maps.sandbox.blip.police_ganton", 10, false },
            { 2110.0f, -1810.0f, 10, 4, "maps.sandbox.blip.burger_marina", 11, false },
            { 2244.0f, -1665.0f, 54, 4, "maps.sandbox.blip.gym_ganton", 12, false },
            { 2200.0f, -1780.0f, 45, 4, "maps.sandbox.blip.suburban_ganton", 13, false },
            { 2520.0f, -1670.0f, 21, 5, "maps.sandbox.blip.denise", 14, false },
            { -2244.0f, -120.0f, 36, 4, "maps.sandbox.blip.driving_school", 15, false },
            { 1800.0f, -1500.0f, 47, 4, "maps.sandbox.blip.zero_rc", 16, false }
        };
    }

    bool GetPlayerPosition(float& x, float& y, float& heading) override {
        x = m_playerX;
        y = m_playerY;
        heading = m_playerHeading;
        return true;
    }

    bool GetWaypointPosition(float& x, float& y) override {
        if (!m_waypointActive) return false;
        x = m_waypointX;
        y = m_waypointY;
        return true;
    }

    void SetWaypointPosition(float x, float y) override {
        m_waypointActive = true;
        m_waypointX = x;
        m_waypointY = y;
    }

    void ClearWaypoint() override {
        m_waypointActive = false;
    }

    void LoadMapTiles() override {
        // Nothing in Sandbox
    }

    ImTextureID GetTileTexture(int index) override {
        return 0; // Force grid fallback rendering
    }

    ImTextureID GetBlipTexture(int spriteId) override {
        return 0; // Fallback to FontAwesome in Sandbox
    }

    std::vector<MapBlip> GetActiveBlips() override {
        std::vector<MapBlip> active;
        for (const auto& sb : m_blips) {
            if (sb.revealed) {
                active.push_back({ sb.x, sb.y, sb.sprite, sb.color, TR(sb.name.c_str()), sb.id });
            }
        }
        return active;
    }
};
