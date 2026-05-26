#pragma once
#include "../../core/providers/IMessageProvider.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/CRunningScript.h>
#include <game_sa/CStats.h>
#include <game_sa/enums/eStats.h>
#include <algorithm>
#include <string>
#include <cstring>

class GtaMessageProvider : public IMessageProvider {
private:
    std::string m_activeMission = "";
    float m_lastMissionsPassed = -1.0f;
    std::string m_completedMission = "";
    bool m_initialized = false;

    // Find the active mission script thread by checking m_bIsMission flag.
    // Returns the lowercased script name, or empty string if no mission is running.
    std::string FindActiveMissionScript() {
        CRunningScript* script = CTheScripts::pActiveScripts;
        while (script) {
            if (script->m_bIsMission) {
                char nameBuf[9] = { 0 };
                std::memcpy(nameBuf, script->m_szName, 8);
                std::string name(nameBuf);
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                
                // Skip the "main" thread — it's not a real mission
                if (!name.empty() && name != "main") {
                    return name;
                }
            }
            script = script->m_pNext;
        }
        return "";
    }

public:
    GtaMessageProvider() = default;
    ~GtaMessageProvider() override = default;

    std::string GetCompletedMissionThisFrame() override {
        // First call: capture baseline stat so we don't false-trigger on game load
        if (!m_initialized) {
            m_lastMissionsPassed = CStats::GetStatValue(STAT_MISSIONS_PASSED);
            m_initialized = true;
        }

        // If we already detected a completion and the consumer hasn't reset it yet,
        // keep returning it (the consumer must call ResetCompletedMission).
        if (!m_completedMission.empty()) {
            return m_completedMission;
        }

        bool onMission = CTheScripts::IsPlayerOnAMission();

        if (onMission) {
            // Track the currently running mission script
            std::string missionName = FindActiveMissionScript();
            if (!missionName.empty()) {
                m_activeMission = missionName;
                // Continuously refresh the stat baseline while the mission is active
                m_lastMissionsPassed = CStats::GetStatValue(STAT_MISSIONS_PASSED);
            }
        } else {
            // Player is no longer on a mission — check if it was completed
            if (!m_activeMission.empty()) {
                float currentPassed = CStats::GetStatValue(STAT_MISSIONS_PASSED);
                if (currentPassed > m_lastMissionsPassed) {
                    // Stat incremented → mission was passed (not failed/aborted)
                    m_completedMission = m_activeMission;
                    m_lastMissionsPassed = currentPassed;
                }
                m_activeMission = "";
            }
        }

        return m_completedMission;
    }

    void ResetCompletedMission() override {
        m_completedMission = "";
    }
};
