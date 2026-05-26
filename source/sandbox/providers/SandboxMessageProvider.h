#pragma once
#include "../../core/providers/IMessageProvider.h"
#include <string>

class SandboxMessageProvider : public IMessageProvider {
private:
    std::string m_completedMission = "";

public:
    SandboxMessageProvider() = default;
    ~SandboxMessageProvider() override = default;

    std::string GetCompletedMissionThisFrame() override {
        return m_completedMission;
    }

    void ResetCompletedMission() override {
        m_completedMission = "";
    }

    // Controls helper to trigger a mission completion in Sandbox
    void TriggerMissionComplete(const std::string& missionId) {
        m_completedMission = missionId;
    }
};
