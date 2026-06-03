#include "EmpresasApp.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>
#include "../LocalizationManager.h"

EmpresasApp::EmpresasApp() {
    id = "empresas";
    icon = ICON_FA_BUILDING;
    name = TR("empresas.title");
    color = ImVec4(0.196f, 0.843f, 0.294f, 1.0f); // Apple green
    dock = false;
    dockOrder = 99;
}

void EmpresasApp::onOpen() {
    m_feedbackMsg = "";
    m_feedbackTimer = 0.0f;
    RefreshData();
}

void EmpresasApp::RefreshData() {
    if (m_businessProvider.isValid()) {
        std::vector<BusinessInfo> list = m_businessProvider->GetBusinesses();
        m_cachedBusinesses.clear();
        
        // 1. Find and add "grove" first (always first, even if not fully unlocked/active)
        auto groveIt = std::find_if(list.begin(), list.end(), [](const BusinessInfo& b) {
            return b.id == "grove";
        });
        if (groveIt != list.end()) {
            m_cachedBusinesses.push_back(*groveIt);
        }
        
        // 2. Add other businesses only if they are owned and not "grove"
        for (const auto& b : list) {
            if (b.id != "grove" && b.owned) {
                m_cachedBusinesses.push_back(b);
            }
        }
    }
}

void EmpresasApp::onClose() {}

void EmpresasApp::update(float dt) {
    if (m_businessProvider.isValid()) {
        m_businessProvider->Update(dt);
    }
    if (m_feedbackTimer > 0.0f) {
        m_feedbackTimer -= dt;
        if (m_feedbackTimer <= 0.0f) {
            m_feedbackMsg = "";
        }
    }
}

void EmpresasApp::SetFeedback(const std::string& msg, float duration) {
    m_feedbackMsg = msg;
    m_feedbackTimer = duration;
}

void EmpresasApp::onDraw() {
    if (!m_businessProvider.isValid()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), TR("empresas.no_provider"));
        return;
    }

    int totalYield = 0;
    int totalProfit = 0;
    for (const auto& b : m_cachedBusinesses) {
        if (b.unlocked) {
            totalYield += b.dailyYield;
            totalProfit += b.currentProfit;
        }
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float cardHeight = 100.0f;

    ImGui::Dummy(ImVec2(width, cardHeight));
    ImVec2 cardMin = startPos;
    ImVec2 cardMax = ImVec2(startPos.x + width, startPos.y + cardHeight);

    // Subtle dark background for summary
    ImColor cardBgCol = ImColor(25, 60, 35, 230);
    drawList->AddRectFilled(cardMin, cardMax, cardBgCol, 12.0f);
    drawList->AddRect(cardMin, cardMax, ImGui::GetColorU32(ImVec4(0.2f, 0.7f, 0.3f, 0.25f)), 12.0f, 0, 1.5f);

    // Text labels
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    // Total Yield
    ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 15.0f, cardMin.y + 12.0f));
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), TR("empresas.daily_yield"));
    ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 15.0f, cardMin.y + 26.0f));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.1f);
    char perDayBuf[32];
    sprintf_s(perDayBuf, sizeof(perDayBuf), TR("empresas.per_day"), totalYield);
    ImGui::Text("%s", perDayBuf);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    // Pending Profit
    ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 15.0f, cardMin.y + 50.0f));
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), TR("empresas.available_profit"));
    ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 15.0f, cardMin.y + 64.0f));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.3f);
    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "$%d", totalProfit);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    // Collect button on the right
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    ImVec2 btnSize = ImVec2(90.0f, 30.0f);
    ImGui::SetCursorScreenPos(ImVec2(cardMax.x - btnSize.x - 15.0f, cardMin.y + (cardHeight - btnSize.y) / 2.0f + 10.0f));
    
    bool canCollectAny = totalProfit > 0;
    if (!canCollectAny) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.4f));
        ImGui::Button(TR("empresas.collected"), btnSize);
        ImGui::PopStyleColor(3);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.196f, 0.843f, 0.294f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.23f, 0.9f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.75f, 0.25f, 1.0f));
        if (ImGui::Button(TR("empresas.collect"), btnSize)) {
            m_businessProvider->CollectAllProfits();
            SetFeedback(TR("empresas.profits_collected"));
            RefreshData();
        }
        ImGui::PopStyleColor(3);
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // Spacing
    ImGui::SetCursorScreenPos(ImVec2(startPos.x, startPos.y + cardHeight));
    ImGui::Dummy(ImVec2(0.0f, 8.0f));

    if (!m_feedbackMsg.empty()) {
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "%s", m_feedbackMsg.c_str());
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Title and Refresh Button
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), TR("empresas.my_businesses"));
    ImGui::SameLine(width - 32.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.85f, 0.4f, 1.0f));
    if (ImGui::Button(ICON_FA_SYNC_ALT "##refresh", ImVec2(24.0f, 20.0f))) {
        RefreshData();
    }
    ImGui::PopStyleColor(4);
    
    ImGui::Spacing();

    // Start List Scroll Child
    ImGui::BeginChild("##scroll_list", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_None);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    
    if (m_cachedBusinesses.empty()) {
        ImGui::SetCursorPosY(20.0f);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.4f), TR("empresas.empty"));
        ImGui::Spacing();
    }

    for (const auto& b : m_cachedBusinesses) {
        ImGui::PushID(b.id.c_str());
        
        float itemHeight = 90.0f;
        ImVec2 itemStart = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(width, itemHeight));
        
        ImVec2 itemMin = itemStart;
        ImVec2 itemMax = ImVec2(itemStart.x + width, itemStart.y + itemHeight);
        
        // Card BG
        drawList->AddRectFilled(itemMin, itemMax, ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.20f, 0.8f)), 12.0f);
        drawList->AddRect(itemMin, itemMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.04f)), 12.0f);
        
        // 1. Icon Bg
        float iconSize = 32.0f;
        ImVec2 iconMin = ImVec2(itemMin.x + 12.0f, itemMin.y + 12.0f);
        ImVec2 iconMax = ImVec2(iconMin.x + iconSize, iconMin.y + iconSize);
        ImColor iconCol = b.unlocked ? ImColor(50, 200, 100, 35) : ImColor(120, 120, 120, 35);
        drawList->AddRectFilled(iconMin, iconMax, iconCol, 8.0f);
        
        ImGui::SetCursorScreenPos(ImVec2(iconMin.x + (iconSize - ImGui::CalcTextSize(b.icon.c_str()).x)/2.0f, iconMin.y + (iconSize - ImGui::GetTextLineHeight())/2.0f));
        ImVec4 iconTextCol = b.unlocked ? ImVec4(0.2f, 0.85f, 0.4f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
        ImGui::TextColored(iconTextCol, "%s", b.icon.c_str());

        // 2. Texts
        ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 52.0f, itemMin.y + 10.0f));
        ImGui::Text("%s", b.name.c_str());
        
        if (!b.unlocked) {
            ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 52.0f, itemMin.y + 26.0f));
            ImGui::TextColored(ImVec4(0.85f, 0.35f, 0.35f, 1.0f), "%s", TR("empresas.yield_blocked"));

            // Disabled Right Action button
            ImVec2 actSize = ImVec2(80.0f, 24.0f);
            ImGui::SetCursorScreenPos(ImVec2(itemMax.x - actSize.x - 12.0f, itemMin.y + 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
            ImGui::Button(TR("empresas.blocked"), actSize);
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();

            // Lock message at the bottom
            ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 12.0f, itemMin.y + 54.0f));
            ImGui::SetWindowFontScale(0.9f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "%s", TR("empresas.no_reputation"));
            ImGui::SetWindowFontScale(1.0f);
        } else {
            ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 52.0f, itemMin.y + 26.0f));
            char perDayItem[32];
            sprintf_s(perDayItem, sizeof(perDayItem), TR("empresas.per_day"), b.dailyYield);
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "%s", perDayItem);

            // 3. Right Action button
            ImVec2 actSize = ImVec2(76.0f, 24.0f);
            ImGui::SetCursorScreenPos(ImVec2(itemMax.x - actSize.x - 12.0f, itemMin.y + 12.0f));
            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            if (b.currentProfit == 0) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.35f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.35f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.35f));
                ImGui::Button(TR("empresas.collected"), actSize);
                ImGui::PopStyleColor(3);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.196f, 0.843f, 0.294f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.23f, 0.9f, 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.75f, 0.25f, 1.0f));
                std::string btnLbl = "$" + std::to_string(b.currentProfit);
                if (ImGui::Button(btnLbl.c_str(), actSize)) {
                    m_businessProvider->CollectProfit(b.id);
                    SetFeedback(TR("empresas.profit_collected"));
                    RefreshData();
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::PopStyleVar();

            // 4. Progress bar
            float progress = static_cast<float>(b.currentProfit) / b.maxProfit;
            progress = std::clamp(progress, 0.0f, 1.0f);
            
            ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 12.0f, itemMin.y + 50.0f));
            
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.196f, 0.843f, 0.294f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.28f, 0.4f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            
            ImGui::ProgressBar(progress, ImVec2(width - 24.0f, 5.0f), "");
            
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
            
            ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 12.0f, itemMin.y + 60.0f));
            ImGui::SetWindowFontScale(0.85f);
            char accumBuf[64];
            sprintf_s(accumBuf, sizeof(accumBuf), TR("empresas.accumulated"), b.currentProfit, b.maxProfit);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.85f), "%s", accumBuf);
            ImGui::SetWindowFontScale(1.0f);
        }
        
        ImGui::PopID();
        ImGui::SetCursorScreenPos(ImVec2(itemStart.x, itemStart.y + itemHeight));
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();
}
