#include "MessagesApp.h"
#include "../Phone.h"
#include "../providers/IMessageProvider.h"
#include "../providers/IAvatarProvider.h"
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
//  39 BCRASH1  Badlands               | 112 FINALEC  End Of The Line (3)
static const std::vector<StorySmsTrigger> g_storySmsTriggers = {
    // === LOS SANTOS ===
    // INTRO2 (ID 12) — Ryder: CJ visits barber with Ryder, pizza shop robbery
    { "intro2", "ryder", "CJ, seu cabelo tá decente agora, mas seu estilo tá fraco. Passa no Binco e compra umas roupas verdes da Grove. Não passa vergonha!" },
    // SWEET1B (ID 14) — Cleaning The Hood: clear crack dealers from the neighborhood
    { "sweet1b", "sweet", "Carl, bom trabalho limpando aqueles viciados de Banton. Mas fica esperto, os Ballas não vão deixar barato. Fica de olho na quebrada." },
    // SWEET6 (ID 19) — Cesar Vialpando: lowrider competition, CJ meets Cesar
    { "sweet6", "kendl", "Carl, obrigada por não brigar com o Cesar. Ele é um bom homem, você vai ver. E vê se não bate o carro de corrida dele!" },
    { "sweet6", "cesar", "Hey CJ, você manda bem no volante, mano. Se quiser correr valendo grana na pista clandestina, me dá um salve. Respeito." },
    // RYDER2 (ID 26) — Robbing Uncle Sam: rob National Guard depot
    { "ryder2", "ryder", "CJ, nós roubamos o exército, cara! Eu sou um gênio militar! Guarda bem aquelas caixas, vamos precisar delas pra mostrar quem manda." },
    // MUSIC2 (ID 32) — Madd Dogg's Rhymes: steal rhyme book for OG Loc
    { "music2", "ogloc", "Carl, esse livro de rimas é ouro! Minhas letras tão de outro mundo agora. O OG Loc tá na área! Respeito!" },
    // DRUGS1 (ID 30) — Just Business: CJ and Smoke vs Russian mobsters
    { "drugs1", "smoke", "Carl, aquele trabalho com os russos foi tenso, mas você mandou bem, irmão. A Grove Street é forte quando trabalhamos juntos!" },
    // DRUGS3 (ID 23) — Gray Imports: investigate arms deal at docks for C.R.A.S.H.
    { "drugs3", "crash", "Carl, você fez um belo serviço com os russos nas docas. É bom ver você sendo útil. Continue assim e talvez a gente não te incomode tanto..." },
    // CRASH4 (ID 21) — Doberman: take over Glen Park from the Ballas
    { "crash4", "sweet", "Carl, Glen Park é nosso! É assim que se faz. Mas o território é quente, os Ballas vão tentar retomar. Fique sempre armado." },
    // LA1FIN2 (ID 38) — The Green Sabre: Sweet arrested, CJ exiled to countryside
    { "la1fin2", "cesar", "CJ, deu tudo errado, os Ballas armaram uma emboscada e o Sweet tá preso. Fica escondido aí no interior, não volta pra LS!" },

    // === COUNTRYSIDE ===
    // BCRASH1 (ID 39) — Badlands: kill informant for Tenpenny in the countryside
    { "bcrash1", "cesar", "CJ, a Kendl tá segura aqui comigo no interior. Fiquei sabendo que a fumaça de Los Santos tá se espalhando. Fica de olho, mano." },
    { "bcrash1", "crash", "Carl, gostou do ar do campo? Espero que o informante esteja bem silenciado. Fique por perto, tenho mais tarefas para você." },
    // TRUTH1 (ID 46) — Body Harvest: steal harvester for The Truth
    { "truth1", "truth", "Carl, a colheitadeira já está coletando a energia cósmica. O governo nos vigia através das vacas, irmão. Fique longe do asfalto!" },

    // === SAN FIERRO ===
    // SYN2 (ID 59) — Jizzy: work for Jizzy B to infiltrate the Loco Syndicate
    { "syn2", "woozie", "Carl, soube que você se infiltrou no Loco Syndicate. Tenha cuidado com o Jizzy, ele é traiçoeiro. Se precisar, meus homens estão prontos." },
    // STEAL4 (ID 69) — Customs Fast Track: steal car from ship at docks (Wang Cars)
    { "steal4", "cesar", "CJ, aquele carro que pegamos nas docas é uma máquina, cara! Já limpei a numeração e deixei na Wang Cars. Passa lá!" },
    // STEAL5 (ID 70) — Puncture Wounds: steal car using stingers (Wang Cars)
    { "steal5", "cesar", "Mano, os furos de pneus funcionaram perfeito! A Wang Cars tá rendendo uma grana preta agora. Valeu pela ajuda, CJ." },
    // SYN7 (ID 63) — Yay Ka-Boom-Boom: blow up Syndicate crack factory
    { "syn7", "toreno", "Carl, você achou que eu estava morto? Engenhoso explodir aquela fábrica. Venha até a minha pista no deserto se quiser ver o Sweet livre." },

    // === DESERT ===
    // DESERT5 (ID 83) — Learning to Fly: flying school at Verdant Meadows
    { "desert5", "toreno", "Carl, parabéns pelas suas asas. Agora você é oficialmente um piloto descartável do governo. Venha receber suas ordens de voo." },

    // === LAS VENTURAS ===
    // CASINO5 (ID 89) — Intensive Care: rescue Johnny Sindacco from ambulance
    { "casino5", "rosenberg", "Carl, obrigado por tirar o Johnny da ambulância. Se a máfia dos Sindacco descobre que ele morreu sob meus cuidados, eu seria história!" },
    // CASIN10 (ID 92) — Saint Mark's Bistro: kill Forelli family in Liberty City
    { "casin10", "salvatore", "Carl, você fez um trabalho limpo em Liberty City. Os Forelli receberam a lição deles. Você tem o meu respeito, garoto." },
    // HEIST9 (ID 101) — Breaking the Bank at Caligula's: the casino heist
    { "heist9", "woozie", "Carl, o roubo ao Caligula's foi lendário! Os Triades mandam lembranças. A sua parte do cofre foi limpa e depositada. Nós fizemos história!" },

    // === RETORNO A LOS SANTOS ===
    // FINALEC (ID 112) — End Of The Line (parte 3): Tenpenny dies, Grove Street wins
    { "finalec", "sweet", "Carl... Nós conseguimos. A Grove Street está no topo, os traidores se foram e o Tenpenny tá no inferno. Orgulho de você, irmão. Nós vencemos." },
};

MessagesApp::MessagesApp() {
    id = "messages";
    icon = ICON_FA_COMMENT;
    name = "SMS";
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
                addIncomingMessage(trigger.contactId, trigger.text);
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
        std::string titleStr = "Sem Mensagens";
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
            replyText = "Tô resolvendo umas paradas com a Grove, Carl. Fica esperto nas ruas.";
        } else if (cid == "ryder") {
            replyText = "Não enche, CJ! Eu sou um gênio muito ocupado para ficar mandando SMS.";
        } else if (cid == "smoke") {
            replyText = "Deus abençoe, CJ. Agora estou fazendo um lanchinho. Nos falamos depois.";
        } else if (cid == "cesar") {
            replyText = "Hey mano, tô na garagem mexendo nos motores. Se cuida.";
        } else if (cid == "kendl") {
            replyText = "Estou ocupada arrumando nossa nova casa. Não cause problemas, Carl.";
        } else if (cid == "ogloc") {
            replyText = "Yo! Estou escrevendo minhas rimas, CJ! O som não para!";
        } else if (cid == "crash") {
            replyText = "Não me mande mensagens a menos que eu mande primeiro, Johnson. Eu sou a lei aqui.";
        } else if (cid == "truth") {
            replyText = "As ondas eletromagnéticas do seu celular estão abrindo portais, cara! Desliga isso!";
        } else if (cid == "woozie") {
            replyText = "Carl, estou no cassino resolvendo negócios dos Triades. Até mais.";
        } else if (cid == "toreno") {
            replyText = "Este número não é seguro. Não envie mensagens para mim.";
        } else if (cid == "rosenberg") {
            replyText = "Carl! Estou tendo um ataque de pânico aqui! Não posso digitar agora!";
        } else if (cid == "salvatore") {
            replyText = "Você acha que eu tenho tempo para SMS, garoto? Me ligue quando tiver o meu dinheiro!";
        } else {
            replyText = "Estou ocupado agora, CJ. Nos falamos depois.";
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
