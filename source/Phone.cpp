#include "Phone.h"
#include <cmath>
#include <cstdio>
#include <time.h>

// ============================================================
// PUBLIC API
// ============================================================

void Phone::registerApp(PhoneApp* app) {
    apps.push_back(app);
}

void Phone::openApp(PhoneApp* app) {
    currentApp = app;
    app->onOpen();
}

void Phone::closeApp() {
    if (currentApp) {
        currentApp->onClose();
        currentApp = nullptr;
    }
}

void Phone::closePhone() {
    closeApp();
    visible = false;
}

// ============================================================
// ICON DRAWING HELPER
// ============================================================

bool Phone::drawIcon(PhoneApp* app, ImDrawList* draw, ImVec2 winPos,
                     float curX, float curY, float sz, float rounding, const char* btnId) {
    float ax = winPos.x + curX;
    float ay = winPos.y + curY;
    ImVec2 p1(ax, ay);
    ImVec2 p2(ax + sz, ay + sz);

    // Colored background
    draw->AddRectFilled(p1, p2, ImGui::GetColorU32(app->color), rounding);

    // TODO: texture icon overlay (AddImageRounded) when texture loading is implemented

    // Invisible button for click detection
    ImGui::SetCursorPos(ImVec2(curX, curY));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1, 1, 1, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(1, 1, 1, 0.25f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    bool clicked = ImGui::Button(btnId, ImVec2(sz, sz));
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    return clicked;
}

// ============================================================
// HOME SCREEN (grid + dock)
// ============================================================

void Phone::drawHome(ImDrawList* draw, ImVec2 winPos) {
    int col = 0;
    float curX = GRID_START_X;
    float curY = GRID_START_Y;
    float maxY = DOCK_Y - GRID_GAP_Y;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.95f));

    for (auto* app : apps) {
        if (curY + ICON_SZ > maxY) break;

        // Build unique button ID
        char btnId[64];
        snprintf(btnId, sizeof(btnId), "##g_%s", app->id.c_str());

        if (drawIcon(app, draw, winPos, curX, curY, ICON_SZ, ICON_R, btnId)) {
            openApp(app);
        }

        // Label below icon (centered)
        ImVec2 textSize = ImGui::CalcTextSize(app->name.c_str());
        float off = std::max(0.0f, std::floor((ICON_SZ - textSize.x) / 2.0f));
        ImGui::SetCursorPos(ImVec2(curX + off, curY + ICON_SZ + 3.0f));
        ImGui::Text("%s", app->name.c_str());

        col++;
        if (col >= GRID_COLS) {
            col = 0;
            curX = GRID_START_X;
            curY += ICON_SZ + LABEL_H + GRID_GAP_Y;
        } else {
            curX += ICON_SZ + GRID_GAP_X;
        }
    }

    ImGui::PopStyleColor();

    // ---- Dock ----
    std::vector<PhoneApp*> dockApps;
    for (auto* app : apps) {
        if (app->dock) dockApps.push_back(app);
    }

    // Sort by dockOrder
    std::sort(dockApps.begin(), dockApps.end(), [](PhoneApp* a, PhoneApp* b) {
        return a->dockOrder < b->dockOrder;
    });

    // Max 4 in dock
    if (dockApps.size() > 4) dockApps.resize(4);

    if (!dockApps.empty()) {
        float dkX = winPos.x + BEZEL;
        float dkY = winPos.y + DOCK_Y;

        // Semi-transparent dock background
        draw->AddRectFilled(
            ImVec2(dkX + SCR_R, dkY),
            ImVec2(dkX + SCREEN_W - SCR_R, dkY + DOCK_H - 16.0f),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.13f)),
            20.0f
        );

        int n = (int)dockApps.size();
        float totW = n * DOCK_ICON_SZ + (n - 1) * DOCK_GAP;
        float sx = BEZEL + std::floor((SCREEN_W - totW) / 2.0f);
        float iy = DOCK_Y + std::floor((DOCK_H - 16.0f - DOCK_ICON_SZ) / 2.0f);

        for (int i = 0; i < n; i++) {
            float ix = sx + i * (DOCK_ICON_SZ + DOCK_GAP);
            char dockBtnId[64];
            snprintf(dockBtnId, sizeof(dockBtnId), "##d_%s", dockApps[i]->id.c_str());

            if (drawIcon(dockApps[i], draw, winPos, ix, iy, DOCK_ICON_SZ, DOCK_ICON_R, dockBtnId)) {
                openApp(dockApps[i]);
            }
        }
    }
}

// ============================================================
// CURRENT APP VIEW (header + content)
// ============================================================

void Phone::drawCurrentApp(ImDrawList* draw, ImVec2 winPos) {
    PhoneApp* app = currentApp;

    // Header: back button + title
    ImGui::SetCursorPos(ImVec2(BEZEL + 4.0f, 50.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.18f, 0.25f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.28f, 0.28f, 0.38f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(1, 1, 1, 1));
    if (ImGui::Button("< Voltar", ImVec2(75.0f, 28.0f))) {
        closeApp();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        return;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    // Centered title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    float tw = ImGui::CalcTextSize(app->name.c_str()).x;
    ImGui::SetCursorPos(ImVec2(std::floor((PH_W - tw) / 2.0f), 55.0f));
    ImGui::Text("%s", app->name.c_str());
    ImGui::PopStyleColor();

    // Separator line
    draw->AddLine(
        ImVec2(winPos.x + BEZEL + 10.0f, winPos.y + 88.0f),
        ImVec2(winPos.x + PH_W - BEZEL - 10.0f, winPos.y + 88.0f),
        ImGui::GetColorU32(ImVec4(1, 1, 1, 0.12f))
    );

    // Content area (child window)
    float contentX = BEZEL + 4.0f;
    float contentY = 94.0f;
    float contentW = PH_W - 2.0f * BEZEL - 8.0f;
    float contentH = PH_H - BEZEL - 30.0f - contentY;

    ImGui::SetCursorPos(ImVec2(contentX, contentY));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));

    ImGui::BeginChild("##app_content", ImVec2(contentW, contentH), false,
                      ImGuiWindowFlags_NoScrollbar);

    app->onDraw();

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// ============================================================
// MAIN DRAW (called from EndScene hook)
// ============================================================

void Phone::draw() {
    if (!visible) return;

    // Get screen resolution from the ImGui display size
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float wx = displaySize.x - PH_W - PH_PADDING_X;
    float wy = displaySize.y - PH_H - PH_PADDING_Y;

    // Window style: phone chassis
    ImGui::GetStyle().WindowRounding = WIN_R;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::SetNextWindowPos(ImVec2(wx, wy), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(PH_W, PH_H), ImGuiCond_Always);

    bool phoneVisible = visible;
    ImGui::Begin("##moonphone", &phoneVisible,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();

    // Screen area (inside bezel)
    ImVec2 pMin(winPos.x + BEZEL, winPos.y + BEZEL);
    ImVec2 pMax(winPos.x + PH_W - BEZEL, winPos.y + PH_H - BEZEL);

    // 1. Wallpaper — fallback gradient (purple/blue like original Lua)
    draw->AddRectFilled(pMin, pMax, IM_COL32(0x4B, 0x1A, 0x7A, 0xFF), SCR_R);  // base

    // 2. If app is open, draw dark overlay
    if (currentApp) {
        draw->AddRectFilled(pMin, pMax,
            ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.07f, 0.97f)), SCR_R);
    }

    // 3. Main content
    if (currentApp) {
        drawCurrentApp(draw, winPos);
    } else {
        drawHome(draw, winPos);
    }

    // 4. Home indicator bar
    {
        float barW = 100.0f;
        float barH = 5.0f;
        float bx = winPos.x + std::floor((PH_W - barW) / 2.0f);
        float by = winPos.y + PH_H - BEZEL - 10.0f;
        draw->AddRectFilled(
            ImVec2(bx, by), ImVec2(bx + barW, by + barH),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.65f)),
            barH / 2.0f
        );
    }

    // 5. Notch pill (drawn ON TOP of everything)
    {
        float niX = winPos.x + std::floor((PH_W - NI_W) / 2.0f);
        float niY = winPos.y + BEZEL - 2.0f;
        draw->AddRectFilled(
            ImVec2(niX, niY), ImVec2(niX + NI_W, niY + NI_H),
            ImGui::GetColorU32(ImVec4(0, 0, 0, 1)),
            NI_R
        );
    }

    // 6. Status bar (time left, battery right)
    {
        char timeStr[16];

#ifndef SANDBOX
        // Get in-game time. We read from the known GTA SA memory addresses.
        // CClock::ms_nGameClockHours  = 0xB70153 (unsigned char)
        // CClock::ms_nGameClockMinutes = 0xB70152 (unsigned char)
        unsigned char gameHour   = *(unsigned char*)0xB70153;
        unsigned char gameMinute = *(unsigned char*)0xB70152;
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", gameHour, gameMinute);
#else
        // In standalone mode, use the real PC time
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
#endif

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

        ImGui::SetCursorPos(ImVec2(BEZEL + 8.0f, BEZEL + 5.0f));
        ImGui::Text("%s", timeStr);

        const char* statusStr = "lll 100%%";
        float stW = ImGui::CalcTextSize(statusStr).x;
        ImGui::SetCursorPos(ImVec2(PH_W - BEZEL - stW - 6.0f, BEZEL + 5.0f));
        ImGui::Text("%s", statusStr);

        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
