#include "StoreApp.h"
#include "../Phone.h"
#include "../LocalizationManager.h"
#include "../providers/IAvatarProvider.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

extern Phone phone;

StoreApp::StoreApp() {
    id = "appstore";
    icon = ICON_FA_STORE;
    name = TR("appstore.title");
    color = ImVec4(0.039f, 0.518f, 1.0f, 1.0f); // Apple Accent Blue
    installable = false; // System app, not uninstallable
    dock = false;
    dockOrder = 99;
}

void StoreApp::onOpen() {
    m_installStates.clear();
}

void StoreApp::onClose() {
    m_installStates.clear();
}

void StoreApp::update(float dt) {
}

void StoreApp::onDraw() {
    float dt = ImGui::GetIO().DeltaTime;
    
    // 1. Update installation timers
    const auto& apps = phone.getApps();

    for (auto* app : apps) {
        if (app->installable) {
            auto& state = m_installStates[app->id];
            if (state.isInstalling) {
                state.progress += dt / 1.5f; // Takes 1.5s
                if (state.progress >= 1.0f) {
                    state.progress = 0.0f;
                    state.isInstalling = false;
                    phone.installApp(app);
                }
            }
        }
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float width = ImGui::GetContentRegionAvail().x;

    // ---- Large Title & Profile Section ----
    ImGui::Spacing();
    ImVec2 startTitlePos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(width, 44.0f));

    // Large Title: App Store
    ImGui::SetCursorScreenPos(ImVec2(startTitlePos.x + 4.0f, startTitlePos.y + 6.0f));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.8f);
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", name.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    // Profile Icon on the right (perfectly rounded and centered, matching the Profile App)
    Inject<IAvatarProvider> ap;
    ImTextureID avatarTex = ap.isValid() ? ap->GetAvatarTexture("profile") : 0;
    float avatarSz = 30.0f;
    ImVec2 avatarPos(startTitlePos.x + width - avatarSz - 4.0f, startTitlePos.y + 6.0f);
    if (avatarTex) {
        drawList->AddImageRounded(avatarTex, avatarPos, ImVec2(avatarPos.x + avatarSz, avatarPos.y + avatarSz), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255), avatarSz / 2.0f);
    } else {
        // Fallback profile circle with ICON_FA_USER
        drawList->AddCircleFilled(ImVec2(avatarPos.x + avatarSz/2.0f, avatarPos.y + avatarSz/2.0f), avatarSz/2.0f, IM_COL32(80, 80, 85, 255));
        ImGui::SetCursorScreenPos(ImVec2(avatarPos.x + (avatarSz - ImGui::CalcTextSize(ICON_FA_USER).x)/2.0f, avatarPos.y + (avatarSz - ImGui::GetTextLineHeight())/2.0f));
        ImGui::Text(ICON_FA_USER);
    }

    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    // ---- App List (Scroll Child) ----
    ImGui::BeginChild("##store_scroll", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_None);
    ImDrawList* childDrawList = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    float cardW = ImGui::GetContentRegionAvail().x - 6.0f; // Leave space for scrollbar

    for (auto* app : apps) {
        if (!app->installable)
            continue;

        ImGui::PushID(app->id.c_str());
        
        float itemH = 76.0f;
        ImVec2 itemStart = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(cardW, itemH));

        ImVec2 itemMin = itemStart;
        ImVec2 itemMax = ImVec2(itemStart.x + cardW, itemStart.y + itemH);

        // Container Box Background (frosted container style)
        childDrawList->AddRectFilled(itemMin, itemMax, ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.20f, 0.8f)), 12.0f);
        childDrawList->AddRect(itemMin, itemMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.04f)), 12.0f);

        // 1. Render App Icon
        float iconSz = 44.0f;
        ImVec2 iconMin(itemMin.x + 12.0f, itemMin.y + (itemH - iconSz) / 2.0f);
        ImVec2 iconMax(iconMin.x + iconSz, iconMin.y + iconSz);

        childDrawList->AddRectFilled(iconMin, iconMax, ImGui::GetColorU32(app->color), 10.0f);

        if (!app->icon.empty()) {
            ImFont* font = ImGui::GetFont();
            float fontSize = ImGui::GetFontSize() * 1.5f;
            ImVec2 tsz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, app->icon.c_str());
            ImVec2 tpos(iconMin.x + (iconSz - tsz.x)/2.0f, iconMin.y + (iconSz - tsz.y)/2.0f + 1.0f);
            childDrawList->AddText(font, fontSize, tpos, IM_COL32(255, 255, 255, 255), app->icon.c_str());
        }

        // 2. Render App Info (Name and Description)
        float textStartX = iconMax.x + 12.0f;
        float btnW = 68.0f;
        float btnH = 26.0f;
        float textMaxW = cardW - 12.0f - iconSz - 12.0f - btnW - 16.0f;

        // Name
        ImGui::SetCursorScreenPos(ImVec2(textStartX, itemMin.y + 12.0f));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetWindowFontScale(1.05f);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", app->name.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        // Description
        ImGui::SetCursorScreenPos(ImVec2(textStartX, itemMin.y + 30.0f));
        ImGui::SetWindowFontScale(0.82f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.72f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textMaxW);
        ImGui::TextUnformatted(app->getAppDescription().c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);

        // 3. Render Pill Action Button or Progress Spinner
        auto& state = m_installStates[app->id];
        ImVec2 btnMin(itemMax.x - btnW - 12.0f, itemMin.y + (itemH - btnH) / 2.0f);
        ImVec2 btnMax(btnMin.x + btnW, btnMin.y + btnH);

        if (state.isInstalling) {
            // Draw iOS-style circular progress spinner
            ImVec2 spinnerCenter(btnMin.x + btnW / 2.0f, btnMin.y + btnH / 2.0f);
            float radius = 10.0f;

            // Draw background track (light grey circle outline)
            childDrawList->AddCircle(spinnerCenter, radius, IM_COL32(255, 255, 255, 30), 32, 2.0f);

            // Draw progress arc (Accent Blue)
            float startAngle = -M_PI / 2.0f;
            float endAngle = startAngle + state.progress * 2.0f * M_PI;
            childDrawList->PathArcTo(spinnerCenter, radius, startAngle, endAngle, 32);
            childDrawList->PathStroke(IM_COL32(3, 132, 255, 255), false, 2.0f);

            // Draw center stop square
            float sqSz = 5.0f;
            childDrawList->AddRectFilled(
                ImVec2(spinnerCenter.x - sqSz/2.0f, spinnerCenter.y - sqSz/2.0f),
                ImVec2(spinnerCenter.x + sqSz/2.0f, spinnerCenter.y + sqSz/2.0f),
                IM_COL32(3, 132, 255, 255),
                1.0f
            );
        } else {
            // Render GET or OPEN button
            ImGui::SetCursorScreenPos(btnMin);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, btnH * 0.5f);
            
            // Styled like AppStore buttons (Pills with gray backgrounds and blue texts)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.15f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.05f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.1f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.039f, 0.518f, 1.0f, 1.0f)); // iOS Blue

            std::string btnLabel = app->installed ? TR("appstore.open") : TR("appstore.get");
            if (ImGui::Button(btnLabel.c_str(), ImVec2(btnW, btnH))) {
                if (app->installed) {
                    // Open the app directly
                    phone.closeApp();
                    phone.openApp(app);
                } else {
                    // Trigger download animation
                    state.isInstalling = true;
                    state.progress = 0.0f;
                }
            }

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();
        }

        ImGui::PopID();
        ImGui::SetCursorScreenPos(ImVec2(itemStart.x, itemStart.y + itemH));
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();
}
