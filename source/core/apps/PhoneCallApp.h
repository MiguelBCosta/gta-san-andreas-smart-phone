#pragma once
#include "../PhoneApp.h"
#include <vector>
#include <string>
#include <imgui.h>

class PhoneCallApp : public PhoneApp {
public:
    enum class PhoneCallState {
        IDLE,
        RINGING,
        TALKING
    };

    struct Contact {
        std::string id;
        std::string name;
        std::string number;
        std::string initials;
        ImVec4 color;
        bool known;
    };

    PhoneCallApp();
    virtual ~PhoneCallApp() = default;

    void onOpen() override;
    void onClose() override;
    void onDraw() override;
    void update(float dt) override;
    void onSave(nlohmann::json& out) override;
    void onLoad(const nlohmann::json& in) override;
    void onWipe() override;

private:
    PhoneCallState m_callState = PhoneCallState::IDLE;
    float m_callTimer = 0.0f;
    std::string m_activeCallerId = "";
    bool m_isOutgoingCall = false;
    
    std::vector<Contact> m_contacts;

    void drawContactsList();
    void drawIncomingCall();
    void drawActiveCall();
    void drawAvatar(const std::string& contactId, const std::string& name, const ImVec4& color, float radius, ImVec2 pos);
};
