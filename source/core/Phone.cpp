#include "Phone.h"
#include "providers/IPhoneCallProvider.h"
#include <cmath>
#include <IconsFontAwesome5.h>

// ============================================================
// PUBLIC API
// ============================================================

Phone::Phone() {
    m_storage.setPhone(this);
}

void Phone::setClockProvider(IClockProvider* provider) {
    m_clockProvider = provider;
}

void Phone::setScreenProvider(IScreenProvider* provider) {
    m_screenProvider = provider;
}

void Phone::setCallProvider(IPhoneCallProvider* provider) {
    m_callProvider = provider;
}

void Phone::setAvatarProvider(IAvatarProvider* provider) {
    m_avatarProvider = provider;
}

void Phone::setCameraProvider(ICameraProvider* provider) {
    m_cameraProvider = provider;
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
    m_editMode = false;
    m_isDragging = false;
    m_draggedKey = "";
    m_pressedKey = "";
    m_pressTimer = 0.0f;
    m_dragStartedThisClick = false;
}

void Phone::update(float dt) {
    if (m_screenProvider) {
        m_screenProvider->Update(dt);
    }
    
    // Auto-open phone and the Phone Call app on active incoming calls
    if (m_callProvider && m_callProvider->IsIncomingCallActive()) {
        if (!m_isOpen) {
            open(PhoneAnimMode::SMOOTH);
        }
        if (m_currentApp == nullptr || m_currentApp->id != "phone") {
            for (auto* app : m_apps) {
                if (app->id == "phone") {
                    openApp(app);
                    break;
                }
            }
        }
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

void Phone::process(float dt) {
    for (auto* app : m_apps) {
        app->update(dt);
    }
}

bool Phone::isVisible() const {
    return m_animProgress > 0.0f;
}

bool Phone::shouldCaptureInput() const {
    if (!isVisible()) return false;
    if (m_currentApp) {
        return m_currentApp->requiresMouse();
    }
    return true; // Home screen requires input capture
}

void Phone::registerApp(PhoneApp* app) {
    // Check if app is already registered
    auto it = std::find(m_apps.begin(), m_apps.end(), app);
    if (it == m_apps.end()) {
        m_apps.push_back(app);
        m_defaultApps.push_back(app);
    }

    if (app->dock) {
        auto itDock = std::find(m_dockApps.begin(), m_dockApps.end(), app);
        if (itDock == m_dockApps.end()) {
            m_dockApps.push_back(app);
            std::sort(m_dockApps.begin(), m_dockApps.end(), [](PhoneApp* a, PhoneApp* b) {
                return a->dockOrder < b->dockOrder;
            });
            // Max 4 in dock
            if (m_dockApps.size() > 4) m_dockApps.resize(4);
        }
    }
    m_storage.addApp(app);
}

void Phone::openApp(PhoneApp* app) {
    m_editMode = false;
    m_currentApp = app;
    app->onOpen();
}

void Phone::closeApp() {
    if (m_currentApp) {
        m_currentApp->onClose();
        m_currentApp = nullptr;
    }
}

void Phone::resetDefaultLayout() {
    m_apps = m_defaultApps;
    m_dockApps.clear();
    for (auto* app : m_apps) {
        if (app->dock) {
            m_dockApps.push_back(app);
        }
    }
    std::sort(m_dockApps.begin(), m_dockApps.end(), [](PhoneApp* a, PhoneApp* b) {
        return a->dockOrder < b->dockOrder;
    });
    if (m_dockApps.size() > 4) m_dockApps.resize(4);

    m_editMode = false;
    m_isDragging = false;
    m_draggedKey = "";
    m_pressedKey = "";
    m_pressTimer = 0.0f;
    m_dragStartedThisClick = false;
    m_visualStates.clear();
}

// ============================================================
// ICON DRAWING HELPER
// ============================================================

bool Phone::drawIcon(PhoneApp* app, ImDrawList* draw, ImVec2 winPos,
                      float curX, float curY, float sz, float rounding, const char* btnId, const std::string& key) {
    AppVisualState& state = m_visualStates[key];
    
    // Compute wiggle offsets if edit mode is active and this app is not being dragged
    float wiggleX = 0.0f;
    float wiggleY = 0.0f;
    if (m_editMode && !state.isDragging) {
        size_t hash = std::hash<std::string>{}(key);
        float phase = (float)(hash % 100) * 0.1f;
        wiggleX = sin(m_time * 22.0f + phase) * 1.5f;
        wiggleY = cos(m_time * 22.0f + phase) * 1.5f;
    }

    float ax = winPos.x + curX + wiggleX;
    float ay = winPos.y + curY + wiggleY;
    ImVec2 p1(ax, ay);
    ImVec2 p2(ax + sz, ay + sz);

    // Draw background
    ImU32 bgColor = ImGui::GetColorU32(app->color);
    if (state.isDragging) {
        // Draw shadow first
        draw->AddRectFilled(ImVec2(p1.x + 4.0f, p1.y + 4.0f), ImVec2(p2.x + 4.0f, p2.y + 4.0f), IM_COL32(0, 0, 0, 80), rounding);
        
        ImVec4 col = app->color;
        col.w = 0.7f;
        bgColor = ImGui::GetColorU32(col);
    }
    
    draw->AddRectFilled(p1, p2, bgColor, rounding);

    // Draw the FontAwesome icon centered
    if (!app->icon.empty()) {
        ImFont* font = ImGui::GetFont();
        float fontSize = ImGui::GetFontSize() * (state.isDragging ? 2.2f : 2.0f);
        ImVec2 tsz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, app->icon.c_str());
        
        float tx = ax + (sz - tsz.x) / 2.0f;
        float ty = ay + (sz - tsz.y) / 2.0f + (state.isDragging ? 3.0f : 2.5f);
        
        ImU32 iconCol = IM_COL32(255, 255, 255, state.isDragging ? 180 : 255);
        draw->AddText(font, fontSize, ImVec2(tx, ty), iconCol, app->icon.c_str(), NULL, 0.0f, nullptr);
    }

    // Draw badge count if > 0
    if (app->badgeCount > 0 && !state.isDragging) {
        float badgeRadius = 8.5f;
        ImVec2 badgeCenter = ImVec2(p2.x - 2.0f, p1.y + 2.0f);
        
        // Draw red filled circle (iOS Red: #FF3B30)
        draw->AddCircleFilled(badgeCenter, badgeRadius, IM_COL32(255, 59, 48, 255));
        
        // Draw the badge number text centered
        std::string badgeText = std::to_string(app->badgeCount);
        if (app->badgeCount > 9) {
            badgeText = "9+";
        }
        
        ImFont* font = ImGui::GetFont();
        float badgeFontSize = badgeRadius * 1.3f;
        ImVec2 textSz = font->CalcTextSizeA(badgeFontSize, FLT_MAX, 0.0f, badgeText.c_str());
        ImVec2 textPos = ImVec2(badgeCenter.x - textSz.x / 2.0f, badgeCenter.y - textSz.y / 2.0f);
        
        draw->AddText(font, badgeFontSize, textPos, IM_COL32(255, 255, 255, 255), badgeText.c_str());
    }

    // Invisible button for click detection
    ImGui::SetCursorPos(ImVec2(curX + wiggleX, curY + wiggleY));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1, 1, 1, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(1, 1, 1, 0.25f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    bool clicked = ImGui::Button(btnId, ImVec2(sz, sz));
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    bool active = ImGui::IsItemActive();
    float dt = ImGui::GetIO().DeltaTime;

    if (active) {
        if (m_pressedKey.empty()) {
            m_pressedKey = key;
            m_pressTimer = 0.0f;
            m_dragStartedThisClick = false;
        }

        if (m_pressedKey == key) {
            if (m_editMode) {
                if (!m_isDragging) {
                    m_isDragging = true;
                    m_draggedKey = key;
                    state.isDragging = true;
                    m_dragOffset = ImVec2(ImGui::GetIO().MousePos.x - p1.x, ImGui::GetIO().MousePos.y - p1.y);
                    m_dragStartedThisClick = true;
                }
            } else {
                m_pressTimer += dt;
                if (m_pressTimer >= 1.5f) {
                    m_editMode = true;
                    m_isDragging = true;
                    m_draggedKey = key;
                    state.isDragging = true;
                    m_dragOffset = ImVec2(ImGui::GetIO().MousePos.x - p1.x, ImGui::GetIO().MousePos.y - p1.y);
                    m_dragStartedThisClick = true;
                    m_pressTimer = 0.0f;
                }
            }
        }
    } else {
        if (m_pressedKey == key) {
            m_pressedKey = "";
            m_pressTimer = 0.0f;
        }
    }

    return clicked;
}

// ============================================================
// HOME SCREEN (grid + dock)
// ============================================================

void Phone::drawHome(ImDrawList* draw, ImVec2 winPos) {
    m_time += ImGui::GetIO().DeltaTime;

    if (!ImGui::IsMouseDown(0)) {
        m_pressedKey = "";
        m_pressTimer = 0.0f;
    }

    // Handle drag movement and reordering
    if (m_isDragging && !m_draggedKey.empty()) {
        AppVisualState& state = m_visualStates[m_draggedKey];
        
        float mouseX = ImGui::GetIO().MousePos.x - winPos.x;
        float mouseY = ImGui::GetIO().MousePos.y - winPos.y;
        state.x = mouseX - m_dragOffset.x;
        state.y = mouseY - m_dragOffset.y;

        if (m_draggedKey.rfind("grid_", 0) == 0) {
            std::string appID = m_draggedKey.substr(5);
            
            float cx = state.x + ICON_SZ / 2.0f;
            float cy = state.y + ICON_SZ / 2.0f;

            int col = (int)std::round((cx - GRID_START_X - ICON_SZ / 2.0f) / (ICON_SZ + GRID_GAP_X));
            int row = (int)std::round((cy - GRID_START_Y - ICON_SZ / 2.0f) / (ICON_SZ + LABEL_H + GRID_GAP_Y));

            if (col < 0) col = 0;
            if (col >= GRID_COLS) col = GRID_COLS - 1;
            if (row < 0) row = 0;
            
            int maxRow = ((int)m_apps.size() - 1) / GRID_COLS;
            if (row > maxRow) row = maxRow;

            int slotIdx = row * GRID_COLS + col;
            if (slotIdx >= (int)m_apps.size()) slotIdx = (int)m_apps.size() - 1;

            int curIdx = -1;
            for (int i = 0; i < (int)m_apps.size(); i++) {
                if (m_apps[i]->id == appID) { curIdx = i; break; }
            }

            if (curIdx != -1 && curIdx != slotIdx) {
                PhoneApp* targetApp = m_apps[curIdx];
                m_apps.erase(m_apps.begin() + curIdx);
                m_apps.insert(m_apps.begin() + slotIdx, targetApp);
            }
        }
        else if (m_draggedKey.rfind("dock_", 0) == 0) {
            std::string appID = m_draggedKey.substr(5);
            float cx = state.x + DOCK_ICON_SZ / 2.0f;

            int n = (int)m_dockApps.size();
            if (n > 0) {
                float totW = n * DOCK_ICON_SZ + (n - 1) * DOCK_GAP;
                float sx = BEZEL + std::floor((SCREEN_W - totW) / 2.0f);

                int slotIdx = (int)std::round((cx - sx - DOCK_ICON_SZ / 2.0f) / (DOCK_ICON_SZ + DOCK_GAP));
                if (slotIdx < 0) slotIdx = 0;
                if (slotIdx >= n) slotIdx = n - 1;

                int curIdx = -1;
                for (int i = 0; i < n; i++) {
                    if (m_dockApps[i]->id == appID) { curIdx = i; break; }
                }

                if (curIdx != -1 && curIdx != slotIdx) {
                    PhoneApp* targetApp = m_dockApps[curIdx];
                    m_dockApps.erase(m_dockApps.begin() + curIdx);
                    m_dockApps.insert(m_dockApps.begin() + slotIdx, targetApp);
                }
            }
        }

        if (!ImGui::IsMouseDown(0)) {
            std::string appID = m_draggedKey.substr(5);
            AppVisualState& dragVS = m_visualStates[m_draggedKey];
            dragVS.isDragging = false;

            if (m_draggedKey.rfind("grid_", 0) == 0) {
                float mouseY = ImGui::GetIO().MousePos.y - winPos.y;
                float mouseX = ImGui::GetIO().MousePos.x - winPos.x;
                if (mouseY >= DOCK_Y && mouseY <= DOCK_Y + DOCK_H) {
                    PhoneApp* appToInsert = nullptr;
                    for (auto* a : m_apps) {
                        if (a->id == appID) { appToInsert = a; break; }
                    }

                    if (appToInsert) {
                        int n = (int)m_dockApps.size();
                        float totW = (n + 1) * DOCK_ICON_SZ + n * DOCK_GAP;
                        if (totW > SCREEN_W) totW = SCREEN_W;
                        float sx = BEZEL + std::floor((SCREEN_W - totW) / 2.0f);
                        int slotIdx = (int)std::round((mouseX - sx - DOCK_ICON_SZ / 2.0f) / (DOCK_ICON_SZ + DOCK_GAP));
                        if (slotIdx < 0) slotIdx = 0;
                        if (slotIdx > n) slotIdx = n;

                        auto it = std::find(m_dockApps.begin(), m_dockApps.end(), appToInsert);
                        if (it != m_dockApps.end()) {
                            m_dockApps.erase(it);
                        }

                        if (m_dockApps.size() < 4) {
                            if (slotIdx > (int)m_dockApps.size()) slotIdx = (int)m_dockApps.size();
                            m_dockApps.insert(m_dockApps.begin() + slotIdx, appToInsert);
                        } else {
                            if (slotIdx > 3) slotIdx = 3;
                            m_dockApps[slotIdx] = appToInsert;
                        }
                    }
                }
            }
            else if (m_draggedKey.rfind("dock_", 0) == 0) {
                float mouseY = ImGui::GetIO().MousePos.y - winPos.y;
                if (mouseY < DOCK_Y - 30.0f || mouseY > DOCK_Y + DOCK_H + 30.0f) {
                    auto it = std::find_if(m_dockApps.begin(), m_dockApps.end(), [&](PhoneApp* a) {
                        return a->id == appID;
                    });
                    if (it != m_dockApps.end()) {
                        m_dockApps.erase(it);
                    }
                }
            }

            m_isDragging = false;
            m_draggedKey = "";
        }
    }

    drawHomeGrid(draw, winPos);
    drawHomeDock(draw, winPos);

    if (m_editMode && ImGui::IsMouseClicked(0)) {
        if (m_pressedKey.empty() && !m_isDragging) {
            float mouseX = ImGui::GetIO().MousePos.x - winPos.x;
            float mouseY = ImGui::GetIO().MousePos.y - winPos.y;
            if (mouseX >= BEZEL && mouseX <= PH_W - BEZEL && mouseY >= BEZEL && mouseY <= PH_H - BEZEL) {
                m_editMode = false;
            }
        }
    }
}

void Phone::drawHomeGrid(ImDrawList* draw, ImVec2 winPos) {
    int col = 0;
    float curX = GRID_START_X;
    float curY = GRID_START_Y;
    float maxY = DOCK_Y - GRID_GAP_Y;
    float dt = ImGui::GetIO().DeltaTime;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.95f));

    for (int i = 0; i < (int)m_apps.size(); i++) {
        auto* app = m_apps[i];
        if (curY + ICON_SZ > maxY) break;

        std::string key = "grid_" + app->id;
        AppVisualState& state = m_visualStates[key];

        float targetX = curX;
        float targetY = curY;

        if (state.x < 0.0f) {
            state.x = targetX;
            state.y = targetY;
        }

        if (!state.isDragging) {
            float speed = 12.0f;
            state.x += (targetX - state.x) * speed * dt;
            state.y += (targetY - state.y) * speed * dt;
        }

        ImGui::PushID(key.c_str());
        
        bool clicked = drawIcon(app, draw, winPos, state.x, state.y, ICON_SZ, ICON_R, "##icon", key);
        if (clicked && !m_dragStartedThisClick && !m_editMode) {
            openApp(app);
        }
        
        ImGui::PopID();

        ImVec2 textSize = ImGui::CalcTextSize(app->name.c_str());
        float off = std::floor((ICON_SZ - textSize.x) / 2.0f);
        
        float labelWiggleX = 0.0f;
        float labelWiggleY = 0.0f;
        if (m_editMode && !state.isDragging) {
            size_t hash = std::hash<std::string>{}(key);
            float phase = (float)(hash % 100) * 0.1f;
            labelWiggleX = sin(m_time * 22.0f + phase) * 1.5f;
            labelWiggleY = cos(m_time * 22.0f + phase) * 1.5f;
        }

        ImGui::SetCursorPos(ImVec2(state.x + off + labelWiggleX, state.y + ICON_SZ + 3.0f + labelWiggleY));
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
    if (!m_dockApps.empty()) {
        float dkX = winPos.x + BEZEL;
        float dkY = winPos.y + DOCK_Y;
        float dt = ImGui::GetIO().DeltaTime;

        float dockMargin = 12.0f;
        draw->AddRectFilled(
            ImVec2(dkX + dockMargin, dkY),
            ImVec2(dkX + SCREEN_W - dockMargin, dkY + DOCK_H - 16.0f),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.13f)),
            24.0f
        );

        int n = (int)m_dockApps.size();
        float totW = n * DOCK_ICON_SZ + (n - 1) * DOCK_GAP;
        float sx = BEZEL + std::floor((SCREEN_W - totW) / 2.0f);
        float iy = DOCK_Y + std::floor((DOCK_H - 16.0f - DOCK_ICON_SZ) / 2.0f);

        for (int i = 0; i < n; i++) {
            float ix = sx + i * (DOCK_ICON_SZ + DOCK_GAP);
            
            auto* app = m_dockApps[i];
            std::string key = "dock_" + app->id;
            AppVisualState& state = m_visualStates[key];

            float targetX = ix;
            float targetY = iy;

            if (state.x < 0.0f) {
                state.x = targetX;
                state.y = targetY;
            }

            if (!state.isDragging) {
                float speed = 12.0f;
                state.x += (targetX - state.x) * speed * dt;
                state.y += (targetY - state.y) * speed * dt;
            }

            ImGui::PushID(key.c_str());
            
            bool clicked = drawIcon(app, draw, winPos, state.x, state.y, DOCK_ICON_SZ, DOCK_ICON_R, "##icon", key);
            if (clicked && !m_dragStartedThisClick && !m_editMode) {
                openApp(app);
            }
            
            ImGui::PopID();
        }
    }
}

// ============================================================
// CURRENT APP VIEW (header + content)
// ============================================================

void Phone::drawCurrentApp(ImDrawList* draw, ImVec2 winPos) {
    PhoneApp* app = m_currentApp;

    // Header: back button + title
    ImGui::SetCursorPos(ImVec2(BEZEL + 4.0f, HEADER_Y_POS));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    
    // Borderless button styling: transparent background, iOS premium blue text color
    ImGui::PushStyleColor(ImGuiCol_Button,         ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // no hover background
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // no active background
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.039f, 0.518f, 1.0f, 1.0f)); // iOS Blue
    
    if (ImGui::Button(ICON_FA_CHEVRON_LEFT " Voltar", ImVec2(HEADER_BTN_W, HEADER_BTN_H))) {
        if (!app->onBack()) {
            closeApp();
        }
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

    if (m_currentApp && m_currentApp->hidePhoneChassis()) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(displaySize, ImGuiCond_Always);
        
        bool visible = true;
        ImGui::Begin("##fullscreenapp", &visible,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBackground
        );
        
        m_currentApp->onDraw();
        
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        return;
    }

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
    if (m_currentApp) {
        if (m_currentApp->hasSolidBackground()) {
            draw->AddRectFilled(pMin, pMax,
                ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.07f, 0.97f)), SCR_R);
        } else {
            draw->AddRectFilled(pMin, pMax,
                ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.07f, 0.20f)), SCR_R);
        }
    }

    // 3. Main content
    if (m_currentApp) {
        drawCurrentApp(draw, winPos);
    } else {
        drawHome(draw, winPos);
    }

    // 4. Home indicator bar (only visible and interactive if an app is open)
    if (m_currentApp) {
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
    if (m_clockProvider) {
        PhoneTime time = m_clockProvider->GetTime();
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
