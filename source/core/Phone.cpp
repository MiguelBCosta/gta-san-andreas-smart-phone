#include "Phone.h"
#include <cmath>
#include <IconsFontAwesome5.h>

// ============================================================
// PUBLIC API
// ============================================================

void Phone::setClockProvider(IClockProvider* provider) {
    clockProvider = provider;
}

void Phone::setScreenProvider(IScreenProvider* provider) {
    screenProvider = provider;
}

void Phone::toggle(PhoneAnimMode mode) {
    if (m_isOpen) close(mode);
    else open(mode);
}

void Phone::open(PhoneAnimMode mode) {
    m_isOpen = true;
    if (mode == PhoneAnimMode::FORCED) m_animProgress = 1.0f;
}

void Phone::close(PhoneAnimMode mode) {
    m_isOpen = false;
    if (mode == PhoneAnimMode::FORCED) m_animProgress = 0.0f;
}

void Phone::update(float dt) {
    if (screenProvider) {
        screenProvider->Update(dt);
    }
    float speed = 5.0f;
    if (m_isOpen) {
        m_animProgress += dt * speed;
        if (m_animProgress > 1.0f) m_animProgress = 1.0f;
    } else {
        m_animProgress -= dt * speed;
        if (m_animProgress < 0.0f) m_animProgress = 0.0f;
    }
}

bool Phone::isVisible() const {
    return m_animProgress > 0.0f;
}

void Phone::registerApp(PhoneApp* app) {
    if (app->dock) {
        dockApps.push_back(app);
        std::sort(dockApps.begin(), dockApps.end(), [](PhoneApp* a, PhoneApp* b) {
            return a->dockOrder < b->dockOrder;
        });
        // Max 4 in dock
        if (dockApps.size() > 4) dockApps.resize(4);
    } else {
        apps.push_back(app);
    }
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

    // Draw the FontAwesome icon centered
    if (!app->icon.empty()) {
        ImFont* font = ImGui::GetFont();
        float fontSize = ImGui::GetFontSize() * 2.0f; // Make icon 2x bigger
        ImVec2 tsz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, app->icon.c_str());
        
        // FontAwesome icons often sit on the baseline, leaving the descent space empty.
        // We push it down slightly to achieve visual centering.
        float tx = ax + (sz - tsz.x) / 2.0f;
        float ty = ay + (sz - tsz.y) / 2.0f + 2.5f;
        draw->AddText(font, fontSize, ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), app->icon.c_str(), NULL, 0.0f, nullptr);
    }

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
    drawHomeGrid(draw, winPos);
    drawHomeDock(draw, winPos);
}

void Phone::drawHomeGrid(ImDrawList* draw, ImVec2 winPos) {
    int col = 0;
    float curX = GRID_START_X;
    float curY = GRID_START_Y;
    float maxY = DOCK_Y - GRID_GAP_Y;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.95f));

    for (auto* app : apps) {
        if (curY + ICON_SZ > maxY) break;

        ImGui::PushID(app->id.c_str());
        if (drawIcon(app, draw, winPos, curX, curY, ICON_SZ, ICON_R, "##icon")) {
            openApp(app);
        }
        ImGui::PopID();

        // Label below icon (centered)
        ImVec2 textSize = ImGui::CalcTextSize(app->name.c_str());
        float off = std::floor((ICON_SZ - textSize.x) / 2.0f);
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
}

void Phone::drawHomeDock(ImDrawList* draw, ImVec2 winPos) {
    if (!dockApps.empty()) {
        float dkX = winPos.x + BEZEL;
        float dkY = winPos.y + DOCK_Y;

        // Semi-transparent dock background
        float dockMargin = 12.0f;
        draw->AddRectFilled(
            ImVec2(dkX + dockMargin, dkY),
            ImVec2(dkX + SCREEN_W - dockMargin, dkY + DOCK_H - 16.0f),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.13f)),
            24.0f
        );

        int n = (int)dockApps.size();
        float totW = n * DOCK_ICON_SZ + (n - 1) * DOCK_GAP;
        float sx = BEZEL + std::floor((SCREEN_W - totW) / 2.0f);
        float iy = DOCK_Y + std::floor((DOCK_H - 16.0f - DOCK_ICON_SZ) / 2.0f);

        for (int i = 0; i < n; i++) {
            float ix = sx + i * (DOCK_ICON_SZ + DOCK_GAP);
            
            ImGui::PushID(dockApps[i]->id.c_str());
            if (drawIcon(dockApps[i], draw, winPos, ix, iy, DOCK_ICON_SZ, DOCK_ICON_R, "##icon")) {
                openApp(dockApps[i]);
            }
            ImGui::PopID();
        }
    }
}

// ============================================================
// CURRENT APP VIEW (header + content)
// ============================================================

void Phone::drawCurrentApp(ImDrawList* draw, ImVec2 winPos) {
    PhoneApp* app = currentApp;

    // Header: back button + title
    ImGui::SetCursorPos(ImVec2(BEZEL + 4.0f, HEADER_Y_POS));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    
    // Borderless button styling: transparent background, iOS premium blue text color
    ImGui::PushStyleColor(ImGuiCol_Button,         ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // no hover background
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // no active background
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.039f, 0.518f, 1.0f, 1.0f)); // iOS Blue
    
    if (ImGui::Button(ICON_FA_CHEVRON_LEFT " Voltar", ImVec2(HEADER_BTN_W, HEADER_BTN_H))) {
        closeApp();
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
        return;
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();

    // Centered title: dynamically aligned vertically with the center of the back button
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImVec2 titleSize = ImGui::CalcTextSize(app->name.c_str());
    float titleY = HEADER_Y_POS + (HEADER_BTN_H - titleSize.y) / 2.0f;
    ImGui::SetCursorPos(ImVec2(std::floor((PH_W - titleSize.x) / 2.0f), titleY));
    ImGui::Text("%s", app->name.c_str());
    ImGui::PopStyleColor();

    // Separator line (drawn full-width inside bezel)
    draw->AddLine(
        ImVec2(winPos.x + BEZEL, winPos.y + HEADER_SEP_Y),
        ImVec2(winPos.x + PH_W - BEZEL, winPos.y + HEADER_SEP_Y),
        ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.12f))
    );

    // Content area (child window)
    float contentX = BEZEL + 4.0f;
    float contentY = CONTENT_Y_OFFSET;
    float contentW = PH_W - 2.0f * BEZEL - 8.0f;
    float contentH = PH_H - BEZEL - 30.0f - contentY;

    ImGui::SetCursorPos(ImVec2(contentX, contentY));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
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
    // Get screen resolution from the ImGui display size
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float margin = 40.0f;
    ImVec2 targetPos(displaySize.x - PH_W - margin, displaySize.y - PH_H - margin);
    ImVec2 offScreenPos(targetPos.x, displaySize.y + 10.0f);
    
    // Smoothstep interpolation for slide animation
    float t = m_animProgress;
    t = t * t * (3.0f - 2.0f * t); 
    
    ImVec2 currentPos;
    currentPos.x = targetPos.x;
    currentPos.y = offScreenPos.y + (targetPos.y - offScreenPos.y) * t;

    // Window style: phone chassis
    ImGui::GetStyle().WindowRounding = WIN_R;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::SetNextWindowPos(currentPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(PH_W, PH_H), ImGuiCond_Always);

    bool phoneVisible = true;
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

    // 4. Home indicator bar (only visible and interactive if an app is open)
    if (currentApp) {
        float barW = 100.0f;
        float barH = 5.0f;
        float bx = winPos.x + std::floor((PH_W - barW) / 2.0f);
        float by = winPos.y + PH_H - BEZEL - 10.0f;
        draw->AddRectFilled(
            ImVec2(bx, by), ImVec2(bx + barW, by + barH),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.65f)),
            barH / 2.0f
        );

        // Detect click to go home (close app)
        ImVec2 pMinBar(bx - 10.0f, by - 7.0f);
        ImVec2 pMaxBar(bx + barW + 10.0f, by + barH + 7.0f);
        if (ImGui::IsMouseHoveringRect(pMinBar, pMaxBar) && ImGui::IsMouseClicked(0)) {
            closeApp();
        }
    }

    // 5. Notch pill (drawn ON TOP of everything)
    {
        float niX = winPos.x + std::floor((PH_W - NI_W) / 2.0f);
        float niY = winPos.y + BEZEL + 5;
        draw->AddRectFilled(
            ImVec2(niX, niY), ImVec2(niX + NI_W, niY + NI_H),
            ImGui::GetColorU32(ImVec4(0, 0, 0, 1)),
            NI_R
        );
    }

    // 6. Status bar
    drawStatusBar();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ============================================================
// STATUS BAR
// ============================================================

void Phone::drawStatusBar() {
    int h = 0, m = 0;
    if (clockProvider) {
        PhoneTime time = clockProvider->GetTime();
        h = time.hours;
        m = time.minutes;
    }
    
    // Convert to string safely using std::string or ImGui format
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

    // Draw time larger and perfectly centered with notch
    ImGui::SetWindowFontScale(1.15f);
    ImGui::SetCursorPos(ImVec2(BEZEL + 20.0f, STATUS_BAR_Y));
    ImGui::Text("%02d:%02d", h, m);
    ImGui::SetWindowFontScale(1.0f);

    // FontAwesome icons for signal (normal) and battery (scaled)
    ImGui::SetWindowFontScale(1.0f);
    float sigW = ImGui::CalcTextSize(ICON_FA_SIGNAL " ").x;
    ImGui::SetWindowFontScale(1.25f);
    float batW = ImGui::CalcTextSize(ICON_FA_BATTERY_FULL).x;
    ImGui::SetWindowFontScale(1.0f);
    
    float totalW = sigW + batW;
    float startX = PH_W - BEZEL - totalW - 12.0f;

    // Draw Signal
    ImGui::SetCursorPos(ImVec2(startX, STATUS_BAR_Y));
    ImGui::Text(ICON_FA_SIGNAL " ");
    
    // Draw Battery
    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosY(STATUS_BAR_Y - 1.0f); // slight Y offset for larger font
    ImGui::SetWindowFontScale(1.25f);
    ImGui::Text(ICON_FA_BATTERY_FULL);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::PopStyleColor();
}
