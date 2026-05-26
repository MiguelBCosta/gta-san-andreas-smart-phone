#pragma once
#include "../../core/providers/IPhoneCallProvider.h"

class SandboxPhoneCallProvider : public IPhoneCallProvider {
private:
    bool m_active = false;
    std::string m_callerId = "unknown";

public:
    bool IsIncomingCallActive() override {
        return m_active;
    }

    std::string GetCallerId() override {
        return m_callerId;
    }

    void AnswerCall() override {
        // No-op, just stays active until hung up
    }

    void HangUpCall() override {
        m_active = false;
    }

    // Sandbox Trigger APIs
    void triggerCall(const std::string& callerId) {
        m_active = true;
        m_callerId = callerId;
    }

    void stopCall() {
        m_active = false;
    }
};
