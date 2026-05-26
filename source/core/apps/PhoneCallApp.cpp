#include "PhoneCallApp.h"
#include "../Phone.h"
#include "../providers/IPhoneCallProvider.h"
#include "../providers/IAvatarProvider.h"
#include <IconsFontAwesome5.h>
#include <algorithm>
#include <cmath>

extern Phone phone;

PhoneCallApp::PhoneCallApp() {
    id = "phone";
    icon = ICON_FA_PHONE;
    name = "Ligar";
    color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
    dock = true;
    dockOrder = 1;

    // Built-in game contacts database (initially unknown)
    m_contacts.push_back({"sweet", "Sweet Johnson", "555-0100", "SJ", ImVec4(0.18f, 0.72f, 0.30f, 1.0f), false});
    m_contacts.push_back({"cesar", "Cesar Vialpando", "555-0101", "CV", ImVec4(0.90f, 0.60f, 0.10f, 1.0f), false});
    m_contacts.push_back({"catalina", "Catalina", "555-0102", "C", ImVec4(0.70f, 0.10f, 0.15f, 1.0f), false});
    m_contacts.push_back({"woozie", "Wu Zi Mu", "555-0103", "WZ", ImVec4(0.12f, 0.56f, 1.00f, 1.0f), false});
    m_contacts.push_back({"truth", "The Truth", "555-0104", "T", ImVec4(0.50f, 0.50f, 0.55f, 1.0f), false});
    m_contacts.push_back({"toreno", "Mike Toreno", "555-0105", "MT", ImVec4(0.30f, 0.30f, 0.35f, 1.0f), false});
    m_contacts.push_back({"tenpenny", "Frank Tenpenny", "555-0106", "FT", ImVec4(0.80f, 0.20f, 0.20f, 1.0f), false});
    m_contacts.push_back({"zero", "Zero", "555-0110", "Z", ImVec4(0.60f, 0.20f, 0.80f, 1.0f), false});
}

void PhoneCallApp::onOpen() {
    // Only reset state if we aren't currently talking or ringing
    if (m_callState == PhoneCallState::IDLE) {
        m_activeCallerId = "";
        m_isOutgoingCall = false;
        m_callTimer = 0.0f;
    }
}

void PhoneCallApp::onClose() {
    // No-op to preserve active conversation if user goes home
}

void PhoneCallApp::onDraw() {
    switch (m_callState) {
    case PhoneCallState::RINGING:
        drawIncomingCall();
        break;
    case PhoneCallState::TALKING:
        drawActiveCall();
        break;
    case PhoneCallState::IDLE:
    default:
        drawContactsList();
        break;
    }
}

void PhoneCallApp::update(float dt) {
    auto* prov = phone.getCallProvider();
    
    if (m_callState == PhoneCallState::TALKING) {
        m_callTimer += dt;
    }

    if (!m_isOutgoingCall && prov) {
        bool active = prov->IsIncomingCallActive();
        if (active) {
            if (m_callState == PhoneCallState::IDLE) {
                m_callState = PhoneCallState::RINGING;
                m_activeCallerId = prov->GetCallerId();
                m_callTimer = 0.0f;
            }
            
            // Check Action Key (TAB) press to answer call
            if (m_callState == PhoneCallState::RINGING && ImGui::IsKeyPressed(ImGuiKey_Tab)) {
                prov->AnswerCall();
                m_callState = PhoneCallState::TALKING;
                m_callTimer = 0.0f;
                // Mark contact as known!
                for (auto& c : m_contacts) {
                    if (c.id == m_activeCallerId) {
                        c.known = true;
                        break;
                    }
                }
            }
        } else {
            // Call ended from game/provider side
            if (m_callState != PhoneCallState::IDLE) {
                m_callState = PhoneCallState::IDLE;
                m_activeCallerId = "";
                phone.closeApp();
            }
        }
    }
}

void PhoneCallApp::onSave(nlohmann::json& out) {
    nlohmann::json knownList = nlohmann::json::array();
    for (const auto& contact : m_contacts) {
        if (contact.known) {
            knownList.push_back(contact.id);
        }
    }
    out["known_contacts"] = knownList;
}

void PhoneCallApp::onLoad(const nlohmann::json& in) {
    // Reset all to false first
    for (auto& contact : m_contacts) {
        contact.known = false;
    }
    if (in.contains("known_contacts") && in["known_contacts"].is_array()) {
        for (const auto& idJson : in["known_contacts"]) {
            std::string id = idJson.get<std::string>();
            for (auto& contact : m_contacts) {
                if (contact.id == id) {
                    contact.known = true;
                    break;
                }
            }
        }
    }
}

void PhoneCallApp::onWipe() {
    for (auto& contact : m_contacts) {
        contact.known = false;
    }
    m_callState = PhoneCallState::IDLE;
    m_callTimer = 0.0f;
    m_activeCallerId = "";
    m_isOutgoingCall = false;
}

void PhoneCallApp::drawContactsList() {
    // Collect discovered/known contacts
    std::vector<Contact*> knownContacts;
    for (auto& c : m_contacts) {
        if (c.known) {
            knownContacts.push_back(&c);
        }
    }

    if (knownContacts.empty()) {
        float contentW = ImGui::GetWindowWidth();
        float contentH = ImGui::GetWindowHeight();

        // Position empty state around 25% down the available window height
        ImGui::SetCursorPosY(contentH * 0.25f);

        // 1. Icon
        ImGui::SetWindowFontScale(3.5f);
        std::string iconStr = ICON_FA_USER_SLASH;
        float iconW = ImGui::CalcTextSize(iconStr.c_str()).x;
        ImGui::SetCursorPosX((contentW - iconW) / 2.0f);
        ImGui::TextColored(ImVec4(0.24f, 0.24f, 0.26f, 1.0f), "%s", iconStr.c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing();
        ImGui::Spacing();

        // 2. Title: "Sem Contatos"
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::SetWindowFontScale(1.25f);
        std::string titleStr = "Sem Contatos";
        float titleW = ImGui::CalcTextSize(titleStr.c_str()).x;
        ImGui::SetCursorPosX((contentW - titleW) / 2.0f);
        ImGui::Text("%s", titleStr.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        return;
    }

    // Scrollable list
    ImGui::BeginChild("##contacts_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
    
    for (size_t i = 0; i < knownContacts.size(); i++) {
        Contact* c = knownContacts[i];
        ImGui::PushID(c->id.c_str());

        ImVec2 pos = ImGui::GetCursorScreenPos();
        float itemH = 46.0f;
        ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, itemH);

        // iOS-style list card
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.16f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.22f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.10f, 0.12f, 0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

        if (ImGui::Button("##item", size)) {
            m_callState = PhoneCallState::TALKING;
            m_callTimer = 0.0f;
            m_activeCallerId = c->id;
            m_isOutgoingCall = true;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        // Draw circular avatar on the left
        float avatarRadius = 16.0f;
        drawAvatar(c->id, c->name, c->color, avatarRadius, ImVec2(pos.x + 8.0f, pos.y + (itemH - avatarRadius * 2.0f) / 2.0f));

        // Draw Name & Number
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + 8.0f + avatarRadius * 2.0f + 10.0f, pos.y + 6.0f),
            IM_COL32(255, 255, 255, 255),
            c->name.c_str()
        );
        
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + 8.0f + avatarRadius * 2.0f + 10.0f, pos.y + 24.0f),
            IM_COL32(150, 150, 150, 255),
            c->number.c_str()
        );

        // Phone call icon on the right
        float iconW = ImGui::CalcTextSize(ICON_FA_PHONE).x;
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + size.x - 16.0f - iconW, pos.y + (itemH - ImGui::GetFontSize()) / 2.0f),
            IM_COL32(76, 209, 55, 255),
            ICON_FA_PHONE
        );

        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void PhoneCallApp::drawIncomingCall() {
    float contentW = ImGui::GetWindowWidth();
    float contentH = ImGui::GetWindowHeight();

    // Resolve display name
    std::string displayName = "Desconhecido";
    Contact* activeContact = nullptr;
    for (auto& c : m_contacts) {
        if (c.id == m_activeCallerId) {
            activeContact = &c;
            if (c.known) {
                displayName = c.name;
            }
            break;
        }
    }

    // Top texts: Subtitle and Name
    ImGui::SetCursorPosY(20.0f);

    // Subtitle: "CHAMADA DE VOZ"
    std::string subtitle = "CHAMADA DE VOZ";
    ImVec2 subSize = ImGui::CalcTextSize(subtitle.c_str());
    ImGui::SetCursorPosX((contentW - subSize.x) / 2.0f);
    ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.57f, 1.0f), "%s", subtitle.c_str());

    ImGui::Spacing();

    // Big display name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.65f);
    ImVec2 nameSize = ImGui::CalcTextSize(displayName.c_str());
    ImGui::SetCursorPosX((contentW - nameSize.x) / 2.0f);
    ImGui::Text("%s", displayName.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    // Center the avatar in the available vertical space
    float topLimit = 110.0f;
    float bottomLimit = contentH - 120.0f;
    float avatarRadius = 55.0f;
    float avatarY = topLimit + (bottomLimit - topLimit - avatarRadius * 2.0f) / 2.0f;

    ImGui::SetCursorPosY(avatarY);
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    float avatarX = (contentW - avatarRadius * 2.0f) / 2.0f;
    drawAvatar(m_activeCallerId, displayName, activeContact ? activeContact->color : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), avatarRadius, ImVec2(screenPos.x + avatarX, screenPos.y));

    // Place decline and accept buttons at the bottom of the window
    float btnSize = 64.0f;
    float gap = 52.0f;
    float totalW = btnSize * 2.0f + gap;
    float startX = (contentW - totalW) / 2.0f;
    float buttonsY = contentH - 110.0f;

    // 1. Decline (Desligar)
    ImGui::SetCursorPosY(buttonsY);
    ImGui::SetCursorPosX(startX);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.92f, 0.30f, 0.26f, 1.0f)); // iOS red
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1.0f, 0.35f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.80f, 0.25f, 0.20f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, btnSize / 2.0f);

    ImGui::SetWindowFontScale(1.8f); // Make icon bigger inside the button
    if (ImGui::Button(ICON_FA_PHONE_SLASH "##decline", ImVec2(btnSize, btnSize))) {
        if (auto* prov = phone.getCallProvider()) {
            prov->HangUpCall();
        }
        m_callState = PhoneCallState::IDLE;
        m_activeCallerId = "";
        phone.closeApp();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // 2. Accept (Atender)
    ImGui::SetCursorPosY(buttonsY);
    ImGui::SetCursorPosX(startX + btnSize + gap);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.30f, 0.82f, 0.22f, 1.0f)); // iOS green
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.35f, 0.90f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.25f, 0.75f, 0.18f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, btnSize / 2.0f);

    ImGui::SetWindowFontScale(1.8f); // Make icon bigger inside the button
    if (ImGui::Button(ICON_FA_PHONE "##accept", ImVec2(btnSize, btnSize))) {
        if (auto* prov = phone.getCallProvider()) {
            prov->AnswerCall();
        }
        m_callState = PhoneCallState::TALKING;
        m_callTimer = 0.0f;
        // Mark caller as known!
        for (auto& c : m_contacts) {
            if (c.id == m_activeCallerId) {
                c.known = true;
                break;
            }
        }
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // Add labels under the buttons
    float labelY = contentH - 38.0f;
    
    float declSize = ImGui::CalcTextSize("Recusar").x;
    ImGui::SetCursorPosY(labelY);
    ImGui::SetCursorPosX(startX + (btnSize - declSize) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Recusar");

    float accSize = ImGui::CalcTextSize("Aceitar").x;
    ImGui::SetCursorPosY(labelY);
    ImGui::SetCursorPosX(startX + btnSize + gap + (btnSize - accSize) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Aceitar");
}

void PhoneCallApp::drawActiveCall() {
    float contentW = ImGui::GetWindowWidth();
    float contentH = ImGui::GetWindowHeight();

    // Resolve display name
    std::string displayName = "Desconhecido";
    Contact* activeContact = nullptr;
    for (auto& c : m_contacts) {
        if (c.id == m_activeCallerId) {
            activeContact = &c;
            displayName = c.name;
            break;
        }
    }

    // Top texts: Timer and Name
    ImGui::SetCursorPosY(20.0f);

    // Subtitle showing timer MM:SS
    int mins = (int)m_callTimer / 60;
    int secs = (int)m_callTimer % 60;
    char timerStr[32];
    sprintf_s(timerStr, "%02d:%02d", mins, secs);

    ImVec2 subSize = ImGui::CalcTextSize(timerStr);
    ImGui::SetCursorPosX((contentW - subSize.x) / 2.0f);
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "%s", timerStr);

    ImGui::Spacing();

    // Big display name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.65f);
    ImVec2 nameSize = ImGui::CalcTextSize(displayName.c_str());
    ImGui::SetCursorPosX((contentW - nameSize.x) / 2.0f);
    ImGui::Text("%s", displayName.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    // Center the avatar in the available vertical space
    float topLimit = 110.0f;
    float bottomLimit = contentH - 120.0f;
    float avatarRadius = 55.0f;
    float avatarY = topLimit + (bottomLimit - topLimit - avatarRadius * 2.0f) / 2.0f;

    ImGui::SetCursorPosY(avatarY);
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    float avatarX = (contentW - avatarRadius * 2.0f) / 2.0f;
    drawAvatar(m_activeCallerId, displayName, activeContact ? activeContact->color : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), avatarRadius, ImVec2(screenPos.x + avatarX, screenPos.y));

    // Single red "Desligar" button at the bottom of the window
    float btnSize = 64.0f;
    float buttonsY = contentH - 110.0f;

    ImGui::SetCursorPosY(buttonsY);
    ImGui::SetCursorPosX((contentW - btnSize) / 2.0f);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.92f, 0.30f, 0.26f, 1.0f)); // iOS red
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1.0f, 0.35f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.80f, 0.25f, 0.20f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, btnSize / 2.0f);

    ImGui::SetWindowFontScale(1.8f); // Make icon bigger inside the button
    if (ImGui::Button(ICON_FA_PHONE_SLASH "##hangup", ImVec2(btnSize, btnSize))) {
        if (!m_isOutgoingCall) {
            if (auto* prov = phone.getCallProvider()) {
                prov->HangUpCall();
            }
        }
        m_callState = PhoneCallState::IDLE;
        m_activeCallerId = "";
        m_isOutgoingCall = false;
        phone.closeApp();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // Add label under the button
    float labelY = contentH - 38.0f;
    float hangSize = ImGui::CalcTextSize("Desligar").x;
    ImGui::SetCursorPosY(labelY);
    ImGui::SetCursorPosX((contentW - hangSize) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Desligar");
}

void PhoneCallApp::drawAvatar(const std::string& contactId, const std::string& name, const ImVec4& color, float radius, ImVec2 pos) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Check if we should render the actual avatar, or fallback/unknown
    bool showRealAvatar = true;
    if (contactId != "unknown") {
        for (const auto& c : m_contacts) {
            if (c.id == contactId && !c.known) {
                showRealAvatar = false;
                break;
            }
        }
    } else {
        showRealAvatar = false;
    }

    ImTextureID tex = 0;
    auto* avatarProv = phone.getAvatarProvider();
    if (showRealAvatar && avatarProv) {
        tex = avatarProv->GetAvatarTexture(contactId);
    }

    if (tex != 0) {
        // Draw the custom PNG texture in a circle
        ImVec2 pMin = pos;
        ImVec2 pMax = ImVec2(pos.x + radius * 2.0f, pos.y + radius * 2.0f);
        drawList->AddImageRounded(tex, pMin, pMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255), radius);
    } else {
        // Fallback: draw initials or "?" in a filled circle
        ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
        ImU32 colorU32 = IM_COL32(100, 100, 100, 255); // unknown default gray
        if (showRealAvatar) {
            colorU32 = ImGui::GetColorU32(color);
        }
        drawList->AddCircleFilled(center, radius, colorU32);

        std::string initials = "?";
        if (showRealAvatar) {
            for (const auto& c : m_contacts) {
                if (c.id == contactId) {
                    initials = c.initials;
                    break;
                }
            }
        }

        ImFont* font = ImGui::GetFont();
        float fontSize = radius * 0.9f;
        ImVec2 tsz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, initials.c_str());
        ImVec2 textPos = ImVec2(center.x - tsz.x / 2.0f, center.y - tsz.y / 2.0f);
        drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, 255), initials.c_str());
    }
}
