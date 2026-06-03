#include "MessagesApp.h"
#include "../Phone.h"
#include "../providers/IMessageProvider.h"
#include "../providers/IAvatarProvider.h"
#include "../LocalizationManager.h"
#include <IconsFontAwesome5.h>
#include <cstring>
#include <algorithm>

extern Phone phone;

struct StorySmsTrigger {
    std::string missionId;
    std::string contactId;
    std::string text;
};

// Database of story messages based on completed missions.
// Mission IDs are the lowercased SCM thread names (m_szName) from main.scm DEFINE MISSIONS table.
//
// SCM Reference (ID → Label → Mission Name):
//  12 INTRO2   Ryder                  |  46 TRUTH1   Body Harvest
//  14 SWEET1B  Cleaning The Hood      |  59 SYN2     Jizzy
//  19 SWEET6   Cesar Vialpando        |  63 SYN7     Yay Ka-Boom-Boom
//  21 CRASH4   Doberman               |  69 STEAL4   Customs Fast Track
//  23 DRUGS3   Gray Imports           |  70 STEAL5   Puncture Wounds
//  26 RYDER2   Robbing Uncle Sam      |  83 DESERT5  Learning to Fly
//  30 DRUGS1   Just Business          |  89 CASINO5  Intensive Care
//  32 MUSIC2   Madd Dogg's Rhymes     |  92 CASIN10  Saint Mark's Bistro
//  38 LA1FIN2  The Green Sabre        | 101 HEIST9   Breaking the Bank
//  39 BCRASH1  Badlands               | 112 FINALEC  End Of The Line

static const std::vector<StorySmsTrigger> g_storySmsTriggers = {
    // === LOS SANTOS ===
    { "intro2",   "ryder",     "msg.intro2.ryder"   },
    { "sweet1b",  "sweet",     "msg.sweet1b.sweet"  },
    { "sweet6",   "kendl",     "msg.sweet6.kendl"   },
    { "sweet6",   "cesar",     "msg.sweet6.cesar"   },
    { "ryder2",   "ryder",     "msg.ryder2.ryder"   },
    { "music2",   "ogloc",     "msg.music2.ogloc"   },
    { "drugs1",   "smoke",     "msg.drugs1.smoke"   },
    { "drugs3",   "crash",     "msg.drugs3.crash"   },
    { "crash4",   "sweet",     "msg.crash4.sweet"   },
    { "la1fin2",  "cesar",     "msg.la1fin2.cesar"  },
    // === COUNTRYSIDE ===
    { "bcrash1",  "cesar",     "msg.bcrash1.cesar"  },
    { "bcrash1",  "crash",     "msg.bcrash1.crash"  },
    { "truth1",   "truth",     "msg.truth1.truth"   },
    // === SAN FIERRO ===
    { "syn2",     "woozie",    "msg.syn2.woozie"    },
    { "steal4",   "cesar",     "msg.steal4.cesar"   },
    { "steal5",   "cesar",     "msg.steal5.cesar"   },
    { "syn7",     "toreno",    "msg.syn7.toreno"    },
    // === DESERT ===
    { "desert5",  "toreno",    "msg.desert5.toreno" },
    // === LAS VENTURAS ===
    { "casino5",  "rosenberg", "msg.casino5.rosenberg" },
    { "casin10",  "salvatore", "msg.casin10.salvatore" },
    { "heist9",   "woozie",    "msg.heist9.woozie"  },
    // === RETORNO A LOS SANTOS ===
    { "finalec",  "sweet",     "msg.finalec.sweet"  },
};

MessagesApp::MessagesApp() {
    id = "messages";
    icon = ICON_FA_COMMENT;
    name = TR("messages.title");
    color = ImVec4(0.20f, 0.78f, 0.35f, 1.0f); // iOS SMS Green
    dock = true;
    dockOrder = 2;

    // Set up contacts
    m_contacts.push_back({ "sweet", "Sweet Johnson", "SJ", ImVec4(0.18f, 0.72f, 0.30f, 1.0f) });
    m_contacts.push_back({ "ryder", "Ryder", "R", ImVec4(0.18f, 0.72f, 0.30f, 1.0f) });
    m_contacts.push_back({ "smoke", "Big Smoke", "BS", ImVec4(0.18f, 0.72f, 0.30f, 1.0f) });
    m_contacts.push_back({ "cesar", "Cesar Vialpando", "CV", ImVec4(0.90f, 0.60f, 0.10f, 1.0f) });
    m_contacts.push_back({ "kendl", "Kendl Johnson", "KJ", ImVec4(0.18f, 0.72f, 0.30f, 1.0f) });
    m_contacts.push_back({ "ogloc", "OG Loc", "OL", ImVec4(0.18f, 0.72f, 0.30f, 1.0f) });
    m_contacts.push_back({ "crash", "C.R.A.S.H.", "CR", ImVec4(0.80f, 0.20f, 0.20f, 1.0f) });
    m_contacts.push_back({ "truth", "The Truth", "T", ImVec4(0.50f, 0.50f, 0.55f, 1.0f) });
    m_contacts.push_back({ "woozie", "Wu Zi Mu", "WZ", ImVec4(0.12f, 0.56f, 1.00f, 1.0f) });
    m_contacts.push_back({ "toreno", "Mike Toreno", "MT", ImVec4(0.30f, 0.30f, 0.35f, 1.0f) });
    m_contacts.push_back({ "rosenberg", "Ken Rosenberg", "KR", ImVec4(0.20f, 0.50f, 0.80f, 1.0f) });
    m_contacts.push_back({ "salvatore", "Salvatore Leone", "SL", ImVec4(0.40f, 0.40f, 0.45f, 1.0f) });
}

const MessagesApp::Contact* MessagesApp::findContact(const std::string& contactId) const {
    for (const auto& contact : m_contacts) {
        if (contact.id == contactId) {
            return &contact;
        }
    }
    return nullptr;
}

void MessagesApp::onOpen() {
    // If opening an active thread, mark its messages as read immediately
    if (!m_activeThreadContactId.empty()) {
        for (auto& thread : m_threads) {
            if (thread.contactId == m_activeThreadContactId) {
                for (auto& msg : thread.messages) {
                    msg.isRead = true;
                }
                break;
            }
        }
        updateBadgeCount();
    }
}

void MessagesApp::onClose() {
    m_activeThreadContactId = "";
    std::memset(m_inputBuf, 0, sizeof(m_inputBuf));
}

bool MessagesApp::onBack() {
    if (!m_activeThreadContactId.empty()) {
        m_activeThreadContactId = "";
        std::memset(m_inputBuf, 0, sizeof(m_inputBuf));
        return true; // Handled internally
    }
    return false; // Let the phone go home
}

void MessagesApp::onDraw() {
    if (m_activeThreadContactId.empty()) {
        drawThreadsList();
    } else {
        const Contact* contact = findContact(m_activeThreadContactId);
        ChatThread* thread = nullptr;
        for (auto& t : m_threads) {
            if (t.contactId == m_activeThreadContactId) {
                thread = &t;
                break;
            }
        }
        drawChatView(contact, thread);
    }
}

void MessagesApp::update(float dt) {
    if (!m_provider) return;

    std::string completedMission = m_provider->GetCompletedMissionThisFrame();
    if (!completedMission.empty()) {
        // Find if this mission triggers any SMS
        for (const auto& trigger : g_storySmsTriggers) {
            if (trigger.missionId == completedMission) {
                // trigger.text is now a translation key
                addIncomingMessage(trigger.contactId, TR(trigger.text.c_str()));
            }
        }
        m_provider->ResetCompletedMission();
    }
}

void MessagesApp::addIncomingMessage(const std::string& contactId, const std::string& text) {
    // Get current game time
    int hours = 12, minutes = 0;
    if (phone.getClockProvider()) {
        PhoneTime pt = phone.getClockProvider()->GetTime();
        hours = pt.hours;
        minutes = pt.minutes;
    }

    // Find or create thread
    ChatThread* targetThread = nullptr;
    for (auto& thread : m_threads) {
        if (thread.contactId == contactId) {
            targetThread = &thread;
            break;
        }
    }

    if (!targetThread) {
        m_threads.push_back({ contactId, {} });
        targetThread = &m_threads.back();
    }

    // Is current active thread this contact? If so, mark as read immediately
    bool isRead = (m_activeThreadContactId == contactId && phone.isOpen());

    targetThread->messages.push_back({ contactId, text, hours, minutes, isRead });

    // Move this thread to the front of the list (most recent first)
    if (m_threads.size() > 1 && targetThread != &m_threads.front()) {
        ChatThread threadCopy = *targetThread;
        m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(), [&](const ChatThread& t) {
            return t.contactId == contactId;
        }), m_threads.end());
        m_threads.insert(m_threads.begin(), threadCopy);
    }

    updateBadgeCount();
}

void MessagesApp::updateBadgeCount() {
    int unread = 0;
    for (const auto& thread : m_threads) {
        for (const auto& msg : thread.messages) {
            if (!msg.isRead) {
                unread++;
            }
        }
    }
    m_badgeCount = unread;
}

void MessagesApp::drawThreadsList() {
    float contentW = ImGui::GetWindowWidth();
    float contentH = ImGui::GetWindowHeight();

    if (m_threads.empty()) {
        // Draw centered empty state
        ImGui::SetCursorPosY(contentH * 0.25f);
        
        ImGui::SetWindowFontScale(3.5f);
        std::string iconStr = ICON_FA_COMMENTS;
        float iconW = ImGui::CalcTextSize(iconStr.c_str()).x;
        ImGui::SetCursorPosX((contentW - iconW) / 2.0f);
        ImGui::TextColored(ImVec4(0.24f, 0.24f, 0.26f, 1.0f), "%s", iconStr.c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::SetWindowFontScale(1.25f);
        std::string titleStr = TR("messages.empty");
        float titleW = ImGui::CalcTextSize(titleStr.c_str()).x;
        ImGui::SetCursorPosX((contentW - titleW) / 2.0f);
        ImGui::Text("%s", titleStr.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        return;
    }

    // List of active conversations
    ImGui::BeginChild("##threads_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

    for (size_t i = 0; i < m_threads.size(); i++) {
        auto& thread = m_threads[i];
        const Contact* contact = findContact(thread.contactId);
        if (!contact) continue;

        ImGui::PushID(thread.contactId.c_str());

        ImVec2 pos = ImGui::GetCursorScreenPos();
        float itemH = 58.0f;
        ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, itemH);

        // Click detection
        if (ImGui::Selectable("##thread_sel", false, ImGuiSelectableFlags_None, size)) {
            m_activeThreadContactId = thread.contactId;
            // Mark all messages as read
            for (auto& msg : thread.messages) {
                msg.isRead = true;
            }
            updateBadgeCount();
            ImGui::PopID();
            break;
        }

        // Draw card background
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(36, 36, 40, 150),
            12.0f
        );

        // Check if there are unread messages in this thread
        bool hasUnread = false;
        for (const auto& msg : thread.messages) {
            if (!msg.isRead) {
                hasUnread = true;
                break;
            }
        }

        // Draw unread blue/green dot
        float dotOffset = 0.0f;
        if (hasUnread) {
            dl->AddCircleFilled(
                ImVec2(pos.x + 10.0f, pos.y + itemH / 2.0f),
                4.0f,
                IM_COL32(52, 199, 89, 255) // Green dot for SMS
            );
            dotOffset = 12.0f;
        }

        // Draw avatar
        float avatarRadius = 18.0f;
        drawAvatar(
            thread.contactId, 
            contact->name, 
            contact->color, 
            avatarRadius, 
            ImVec2(pos.x + 8.0f + dotOffset, pos.y + (itemH - avatarRadius * 2.0f) / 2.0f)
        );

        // Draw Contact Name
        float textX = pos.x + 8.0f + dotOffset + avatarRadius * 2.0f + 10.0f;
        dl->AddText(
            ImVec2(textX, pos.y + 10.0f),
            IM_COL32(255, 255, 255, 255),
            contact->name.c_str()
        );

        // Draw last message preview (truncated)
        std::string preview = "";
        if (!thread.messages.empty()) {
            preview = thread.messages.back().text;
            if (preview.length() > 28) {
                preview = preview.substr(0, 25) + "...";
            }
        }
        dl->AddText(
            ImVec2(textX, pos.y + 30.0f),
            IM_COL32(150, 150, 150, 255),
            preview.c_str()
        );

        // Draw timestamp on the right
        if (!thread.messages.empty()) {
            char timeBuf[16];
            sprintf_s(timeBuf, "%02d:%02d", thread.messages.back().hours, thread.messages.back().minutes);
            float timeW = ImGui::CalcTextSize(timeBuf).x;
            dl->AddText(
                ImVec2(pos.x + size.x - 12.0f - timeW, pos.y + 10.0f),
                IM_COL32(120, 120, 120, 255),
                timeBuf
            );
        }

        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void MessagesApp::drawChatView(const Contact* contact, ChatThread* thread) {
    if (!contact) return;

    float contentW = ImGui::GetWindowWidth();
    float contentH = ImGui::GetWindowHeight();

    // 1. Custom app header inside child window
    float headerH = 34.0f;
    ImGui::BeginChild("##chat_header", ImVec2(0, headerH), false, ImGuiWindowFlags_NoScrollbar);

    // Centered name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImVec2 nameSz = ImGui::CalcTextSize(contact->name.c_str());
    ImGui::SetCursorPos(ImVec2((contentW - nameSz.x) / 2.0f, (headerH - nameSz.y) / 2.0f));
    ImGui::Text("%s", contact->name.c_str());
    ImGui::PopStyleColor();

    // Small avatar on the right
    float avatarR = 12.0f;
    drawAvatar(
        contact->id, 
        contact->name, 
        contact->color, 
        avatarR, 
        ImVec2(contentW - avatarR * 2.0f - 4.0f, (headerH - avatarR * 2.0f) / 2.0f)
    );

    ImGui::EndChild();

    // Divider line
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(ImGui::GetWindowPos().x, ImGui::GetCursorScreenPos().y),
        ImVec2(ImGui::GetWindowPos().x + contentW, ImGui::GetCursorScreenPos().y),
        IM_COL32(255, 255, 255, 20)
    );
    ImGui::Spacing();

    // 2. Chat history area
    float inputAreaH = 36.0f;
    float listH = contentH - headerH - inputAreaH - 18.0f;
    
    ImGui::BeginChild("##messages_list", ImVec2(0, listH), false, ImGuiWindowFlags_NoScrollbar);

    if (thread && !thread->messages.empty()) {
        float bubbleMaxW = contentW * 0.70f;
        float paddingX = 10.0f;
        float paddingY = 8.0f;
        float margin = 8.0f;
        
        for (const auto& msg : thread->messages) {
            bool isPlayer = (msg.senderId == "player");
            ImVec2 textSz = ImGui::CalcTextSize(msg.text.c_str(), nullptr, false, bubbleMaxW);
            
            float bubbleW = textSz.x + paddingX * 2.0f;
            float bubbleH = textSz.y + paddingY * 2.0f;

            ImVec2 bubblePos = ImGui::GetCursorScreenPos();
            if (isPlayer) {
                bubblePos.x = ImGui::GetWindowPos().x + contentW - bubbleW - margin;
            } else {
                bubblePos.x = ImGui::GetWindowPos().x + margin;
            }

            // Draw bubble rounded background
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 bubbleCol = isPlayer ? IM_COL32(52, 199, 89, 255) : IM_COL32(36, 36, 40, 255); // Green vs Dark Gray
            dl->AddRectFilled(
                bubblePos,
                ImVec2(bubblePos.x + bubbleW, bubblePos.y + bubbleH),
                bubbleCol,
                14.0f
            );

            // Draw message text
            ImGui::SetCursorScreenPos(ImVec2(bubblePos.x + paddingX, bubblePos.y + paddingY));
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textSz.x);
            ImGui::TextUnformatted(msg.text.c_str());
            ImGui::PopTextWrapPos();

            // Set cursor for the next message
            ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, bubblePos.y + bubbleH + 4.0f));
            
            // Subtle timestamp below bubble
            char timeBuf[16];
            sprintf_s(timeBuf, "%02d:%02d", msg.hours, msg.minutes);
            float timeW = ImGui::CalcTextSize(timeBuf).x;
            float timeX = isPlayer ? (contentW - timeW - margin - 4.0f) : (margin + 4.0f);
            
            ImGui::SetCursorPosX(timeX);
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.45f, 1.0f), "%s", timeBuf);
            ImGui::Spacing();
        }
        
        // Auto-scroll to bottom
        static float lastCount = 0.0f;
        if ((float)thread->messages.size() != lastCount) {
            ImGui::SetScrollHereY(1.0f);
            lastCount = (float)thread->messages.size();
        }
    }

    ImGui::EndChild();

    // 3. Input bar at the bottom
    ImGui::BeginChild("##input_area", ImVec2(0, inputAreaH), false, ImGuiWindowFlags_NoScrollbar);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.14f, 0.9f));
    ImGui::SetNextItemWidth(contentW - 46.0f);
    
    // Pressing Enter in input text sends it
    bool enterPressed = ImGui::InputText("##chat_in", m_inputBuf, sizeof(m_inputBuf), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::SameLine(0, 4.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(52/255.0f, 199/255.0f, 89/255.0f, 1.0f)); // iOS green text for send button
    
    bool sendClicked = ImGui::Button(ICON_FA_ARROW_UP, ImVec2(30.0f, 30.0f));
    ImGui::PopStyleColor(4);

    if ((enterPressed || sendClicked) && std::strlen(m_inputBuf) > 0) {
        // Send message
        std::string userMsg(m_inputBuf);
        
        int hours = 12, minutes = 0;
        if (phone.getClockProvider()) {
            PhoneTime pt = phone.getClockProvider()->GetTime();
            hours = pt.hours;
            minutes = pt.minutes;
        }

        // If thread doesn't exist, create it
        if (!thread) {
            m_threads.push_back({ m_activeThreadContactId, {} });
            thread = &m_threads.back();
        }

        thread->messages.push_back({ "player", userMsg, hours, minutes, true });
        std::memset(m_inputBuf, 0, sizeof(m_inputBuf));

        // Generate dynamic character responses (lore friendly responses!)
        std::string replyText = "";
        std::string cid = contact->id;
        
        if (cid == "sweet") {
            replyText = TR("msg.reply.sweet");
        } else if (cid == "ryder") {
            replyText = TR("msg.reply.ryder");
        } else if (cid == "smoke") {
            replyText = TR("msg.reply.smoke");
        } else if (cid == "cesar") {
            replyText = TR("msg.reply.cesar");
        } else if (cid == "kendl") {
            replyText = TR("msg.reply.kendl");
        } else if (cid == "ogloc") {
            replyText = TR("msg.reply.ogloc");
        } else if (cid == "crash") {
            replyText = TR("msg.reply.crash");
        } else if (cid == "truth") {
            replyText = TR("msg.reply.truth");
        } else if (cid == "woozie") {
            replyText = TR("msg.reply.woozie");
        } else if (cid == "toreno") {
            replyText = TR("msg.reply.toreno");
        } else if (cid == "rosenberg") {
            replyText = TR("msg.reply.rosenberg");
        } else if (cid == "salvatore") {
            replyText = TR("msg.reply.salvatore");
        } else {
            replyText = TR("msg.reply.default");
        }

        // Add reply after a tiny virtual delay
        addIncomingMessage(cid, replyText);
    }

    ImGui::EndChild();
}

void MessagesApp::drawAvatar(const std::string& contactId, const std::string& name, const ImVec4& color, float radius, ImVec2 pos) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImTextureID tex = 0;
    auto* avatarProv = phone.getAvatarProvider();
    if (avatarProv) {
        tex = avatarProv->GetAvatarTexture(contactId);
    }

    if (tex != 0) {
        ImVec2 pMin = pos;
        ImVec2 pMax = ImVec2(pos.x + radius * 2.0f, pos.y + radius * 2.0f);
        drawList->AddImageRounded(tex, pMin, pMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255), radius);
    } else {
        // Fallback: initials
        ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
        drawList->AddCircleFilled(center, radius, ImGui::GetColorU32(color));

        // Get initials
        std::string initials = "?";
        for (const auto& c : m_contacts) {
            if (c.id == contactId) {
                initials = c.initials;
                break;
            }
        }

        ImFont* font = ImGui::GetFont();
        float fontSize = radius * 0.9f;
        ImVec2 tsz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, initials.c_str());
        ImVec2 textPos = ImVec2(center.x - tsz.x / 2.0f, center.y - tsz.y / 2.0f);
        drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, 255), initials.c_str());
    }
}

void MessagesApp::onSave(nlohmann::json& out) {
    nlohmann::json threadsArr = nlohmann::json::array();
    
    for (const auto& thread : m_threads) {
        nlohmann::json tJson;
        tJson["contactId"] = thread.contactId;
        
        nlohmann::json msgsArr = nlohmann::json::array();
        for (const auto& msg : thread.messages) {
            nlohmann::json mJson;
            mJson["senderId"] = msg.senderId;
            mJson["text"] = msg.text;
            mJson["hours"] = msg.hours;
            mJson["minutes"] = msg.minutes;
            mJson["isRead"] = msg.isRead;
            msgsArr.push_back(mJson);
        }
        tJson["messages"] = msgsArr;
        threadsArr.push_back(tJson);
    }
    
    out["threads"] = threadsArr;
}

void MessagesApp::onLoad(const nlohmann::json& in) {
    m_threads.clear();
    m_activeThreadContactId = "";

    if (in.contains("threads") && in["threads"].is_array()) {
        for (const auto& tJson : in["threads"]) {
            if (!tJson.contains("contactId") || !tJson.contains("messages")) continue;
            
            ChatThread thread;
            thread.contactId = tJson["contactId"].get<std::string>();
            
            for (const auto& mJson : tJson["messages"]) {
                Message msg;
                msg.senderId = mJson.value("senderId", "");
                msg.text = mJson.value("text", "");
                msg.hours = mJson.value("hours", 0);
                msg.minutes = mJson.value("minutes", 0);
                msg.isRead = mJson.value("isRead", false);
                thread.messages.push_back(msg);
            }
            m_threads.push_back(thread);
        }
    }
    updateBadgeCount();
}

void MessagesApp::onWipe() {
    m_threads.clear();
    m_activeThreadContactId = "";
    m_badgeCount = 0;
}
