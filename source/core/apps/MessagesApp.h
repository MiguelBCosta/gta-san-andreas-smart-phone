#pragma once
#include "../PhoneApp.h"
#include <vector>
#include <string>
#include <imgui.h>
#include <nlohmann/json.hpp>

class IMessageProvider;

class MessagesApp : public PhoneApp {
public:
    struct Message {
        std::string senderId; // "player" or contactId
        std::string text;
        int hours = 0;
        int minutes = 0;
        bool isRead = false;
    };

    struct ChatThread {
        std::string contactId;
        std::vector<Message> messages;
    };

    struct Contact {
        std::string id;
        std::string name;
        std::string initials;
        ImVec4 color;
    };

    MessagesApp();
    virtual ~MessagesApp() = default;

    void SetMessageProvider(IMessageProvider* provider) { m_provider = provider; }

    void onOpen() override;
    void onClose() override;
    void onDraw() override;
    void update(float dt) override;
    bool onBack() override;
    
    void onSave(nlohmann::json& out) override;
    void onLoad(const nlohmann::json& in) override;
    void onWipe() override;

private:
    IMessageProvider* m_provider = nullptr;
    std::vector<ChatThread> m_threads;
    std::vector<Contact> m_contacts;
    std::string m_activeThreadContactId = "";
    char m_inputBuf[256] = "";

    // Helper functions
    const Contact* findContact(const std::string& contactId) const;
    void drawThreadsList();
    void drawChatView(const Contact* contact, ChatThread* thread);
    void drawAvatar(const std::string& contactId, const std::string& name, const ImVec4& color, float radius, ImVec2 pos);
    void addIncomingMessage(const std::string& contactId, const std::string& text);
    void updateBadgeCount();
};
