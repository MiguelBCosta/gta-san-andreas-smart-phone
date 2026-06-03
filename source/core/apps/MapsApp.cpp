#include "MapsApp.h"
#include "../LocalizationManager.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static void GetCategoryMeta(int sprite, std::string& name, const char*& icon, ImVec4& color) {
    name = TR("maps.place.default");
    icon = ICON_FA_MAP_MARKER_ALT;
    color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    
    switch (sprite) {
        case 5:  name = TR("maps.place.airport"); icon = ICON_FA_PLANE; color = ImVec4(0.1f, 0.6f, 1.0f, 1.0f); break;
        case 6:  name = TR("maps.place.ammu"); icon = ICON_FA_CROSSHAIRS; color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); break;
        case 7:  name = TR("maps.place.barber"); icon = ICON_FA_CUT; color = ImVec4(1.0f, 0.6f, 0.1f, 1.0f); break;
        case 9:  name = TR("maps.place.boat_dock"); icon = ICON_FA_SHIP; color = ImVec4(0.1f, 0.6f, 1.0f, 1.0f); break;
        case 10: name = TR("maps.place.burger_shot"); icon = ICON_FA_UTENSILS; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 14: name = TR("maps.place.cluckin_bell"); icon = ICON_FA_UTENSILS; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 17: name = TR("maps.place.restaurant"); icon = ICON_FA_UTENSILS; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 21: name = TR("maps.place.girlfriend"); icon = ICON_FA_HEART; color = ImVec4(1.0f, 0.4f, 0.7f, 1.0f); break;
        case 22: name = TR("maps.place.hospital"); icon = ICON_FA_HOSPITAL; color = ImVec4(1.0f, 0.18f, 0.33f, 1.0f); break;
        case 25: name = TR("maps.place.caligulas"); icon = ICON_FA_DICE; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 27: name = TR("maps.place.mod_garage"); icon = ICON_FA_WRENCH; color = ImVec4(0.1f, 0.6f, 1.0f, 1.0f); break;
        case 29: name = TR("maps.place.pizza"); icon = ICON_FA_UTENSILS; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 30: name = TR("maps.place.police"); icon = ICON_FA_SHIELD_ALT; color = ImVec4(0.1f, 0.6f, 1.0f, 1.0f); break;
        case 31: name = TR("maps.place.property_owned"); icon = ICON_FA_HOME; color = ImVec4(0.2f, 0.8f, 0.3f, 1.0f); break;
        case 32: name = TR("maps.place.property_sale"); icon = ICON_FA_HOME; color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); break;
        case 33: name = TR("maps.place.race"); icon = ICON_FA_TROPHY; color = ImVec4(1.0f, 0.84f, 0.0f, 1.0f); break;
        case 35: name = TR("maps.place.safehouse"); icon = ICON_FA_HOME; color = ImVec4(0.2f, 0.8f, 0.3f, 1.0f); break;
        case 36: name = TR("maps.place.flight_school"); icon = ICON_FA_GRADUATION_CAP; color = ImVec4(0.7f, 0.3f, 0.9f, 1.0f); break;
        case 39: name = TR("maps.place.tattoo"); icon = ICON_FA_PAINT_BRUSH; color = ImVec4(1.0f, 0.6f, 0.1f, 1.0f); break;
        case 44: name = TR("maps.place.four_dragons"); icon = ICON_FA_DICE; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 45: name = TR("maps.place.clothes"); icon = ICON_FA_TSHIRT; color = ImVec4(0.4f, 0.3f, 0.8f, 1.0f); break;
        case 47: name = TR("maps.place.zero"); icon = ICON_FA_GAMEPAD; color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); break;
        case 48: name = TR("maps.place.club"); icon = ICON_FA_MUSIC; color = ImVec4(0.64f, 0.0f, 1.0f, 1.0f); break;
        case 49: name = TR("maps.place.bar"); icon = ICON_FA_GLASS_MARTINI_ALT; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 50: name = TR("maps.place.date_restaurant"); icon = ICON_FA_UTENSILS; color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
        case 54: name = TR("maps.place.gym"); icon = ICON_FA_DUMBBELL; color = ImVec4(0.7f, 0.3f, 0.9f, 1.0f); break;
        case 55: name = TR("maps.place.impound"); icon = ICON_FA_CAR; color = ImVec4(0.1f, 0.6f, 1.0f, 1.0f); break;
        case 63: name = TR("maps.place.pay_n_spray"); icon = ICON_FA_WRENCH; color = ImVec4(0.1f, 0.6f, 1.0f, 1.0f); break;
    }
}

bool MapsApp::bMapAppOpen = false;

ImVec2 MapsApp::worldToMap(float worldX, float worldY, float mapSize) {
    float x = ((worldX + 3000.0f) / 6000.0f) * mapSize;
    float y = ((3000.0f - worldY) / 6000.0f) * mapSize;
    return ImVec2(x, y);
}

ImVec2 MapsApp::mapToWorld(float mapX, float mapY, float mapSize) {
    float x = (mapX / mapSize) * 6000.0f - 3000.0f;
    float y = 3000.0f - (mapY / mapSize) * 6000.0f;
    return ImVec2(x, y);
}

void MapsApp::onOpen() {
    bMapAppOpen = true;
    m_showSidebar = false;
    m_zoom = 1.0f;
    
    if (m_mapProvider) {
        m_mapProvider->LoadMapTiles();
        
        // Center on player
        float px = 0.0f, py = 0.0f, ph = 0.0f;
        if (m_mapProvider->GetPlayerPosition(px, py, ph)) {
            m_scroll = worldToMap(px, py, 3072.0f);
        } else {
            m_scroll = ImVec2(1536.0f, 1536.0f);
        }
    }
}

void MapsApp::onClose() {
    bMapAppOpen = false;
}

bool MapsApp::onBack() {
    if (m_showSidebar) {
        m_showSidebar = false;
        return true;
    }
    return false; // Let the phone close the app
}

void MapsApp::onDraw() {
    if (!m_mapProvider) {
        ImGui::Text(TR("maps.no_provider"));
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    
    // Header Bar
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f));
    
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
    ImGui::Text(TR("maps.title"));
    ImGui::SameLine(ImGui::GetWindowWidth() - 40.0f);
    
    if (ImGui::Button(m_showSidebar ? ICON_FA_MAP : ICON_FA_LIST)) {
        m_showSidebar = !m_showSidebar;
    }
    
    ImGui::PopStyleColor(3);
    ImGui::Separator();
    
    // Viewport Calculations
    ImVec2 viewportMin = ImGui::GetCursorScreenPos();
    ImVec2 viewportSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 10.0f); // leave a small margin at bottom
    ImVec2 viewportMax = ImVec2(viewportMin.x + viewportSize.x, viewportMin.y + viewportSize.y);
    
    // Begin map child window
    ImGui::BeginChild("##MapViewport", viewportSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 localViewportMin = ImGui::GetCursorScreenPos();
    
    // 1. Zoom and Pan Controls
    if (ImGui::IsWindowHovered()) {
        // Zoom
        if (io.MouseWheel != 0.0f) {
            float oldZoom = m_zoom;
            m_zoom = std::max(0.08f, std::min(10.0f, m_zoom + io.MouseWheel * 0.1f * m_zoom));
            
            // Adjust scroll so zoom stays centered on mouse
            ImVec2 mousePos = io.MousePos;
            ImVec2 mouseRelToMap = ImVec2((mousePos.x - localViewportMin.x) / oldZoom + m_scroll.x - (viewportSize.x * 0.5f / oldZoom),
                                          (mousePos.y - localViewportMin.y) / oldZoom + m_scroll.y - (viewportSize.y * 0.5f / oldZoom));
            
            m_scroll.x = mouseRelToMap.x - (mousePos.x - localViewportMin.x) / m_zoom + (viewportSize.x * 0.5f / m_zoom);
            m_scroll.y = mouseRelToMap.y - (mousePos.y - localViewportMin.y) / m_zoom + (viewportSize.y * 0.5f / m_zoom);
        }
        
        // Pan
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            m_scroll.x -= dragDelta.x / m_zoom;
            m_scroll.y -= dragDelta.y / m_zoom;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
        
        // Right-click to place/remove Waypoint
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImVec2 mousePos = io.MousePos;
            
            // Check if there is an active waypoint and if the click was close to it (within 15 pixels)
            float wx = 0.0f, wy = 0.0f;
            bool hasWp = m_mapProvider->GetWaypointPosition(wx, wy);
            bool clickedOnWp = false;
            
            if (hasWp) {
                ImVec2 mapWp = worldToMap(wx, wy, 3072.0f);
                ImVec2 wpScreen = ImVec2(localViewportMin.x + (mapWp.x - m_scroll.x) * m_zoom + viewportSize.x * 0.5f,
                                         localViewportMin.y + (mapWp.y - m_scroll.y) * m_zoom + viewportSize.y * 0.5f);
                
                float dx = mousePos.x - wpScreen.x;
                float dy = mousePos.y - wpScreen.y;
                float distPx = sqrtf(dx * dx + dy * dy);
                if (distPx < 15.0f) {
                    clickedOnWp = true;
                }
            }
            
            if (clickedOnWp) {
                m_mapProvider->ClearWaypoint();
            } else {
                float mapX = (mousePos.x - localViewportMin.x - viewportSize.x * 0.5f) / m_zoom + m_scroll.x;
                float mapY = (mousePos.y - localViewportMin.y - viewportSize.y * 0.5f) / m_zoom + m_scroll.y;
                
                ImVec2 worldPos = mapToWorld(mapX, mapY, 3072.0f);
                
                // Clamp to SA world boundaries
                worldPos.x = std::max(-3000.0f, std::min(3000.0f, worldPos.x));
                worldPos.y = std::max(-3000.0f, std::min(3000.0f, worldPos.y));
                
                m_mapProvider->SetWaypointPosition(worldPos.x, worldPos.y);
            }
        }
    }
    
    // Clamp scroll
    m_scroll.x = std::max(0.0f, std::min(3072.0f, m_scroll.x));
    m_scroll.y = std::max(0.0f, std::min(3072.0f, m_scroll.y));
    
    // 2. Render Tiles
    for (int ty = 0; ty < 12; ++ty) {
        for (int tx = 0; tx < 12; ++tx) {
            float tileLeft = tx * 256.0f;
            float tileTop = ty * 256.0f;
            
            // Screen coords for tile bounds
            ImVec2 tileMin = ImVec2(localViewportMin.x + (tileLeft - m_scroll.x) * m_zoom + viewportSize.x * 0.5f,
                                    localViewportMin.y + (tileTop - m_scroll.y) * m_zoom + viewportSize.y * 0.5f);
            ImVec2 tileMax = ImVec2(tileMin.x + 256.0f * m_zoom, tileMin.y + 256.0f * m_zoom);
            
            // Viewport clipping check
            if (tileMax.x < localViewportMin.x || tileMin.x > localViewportMin.x + viewportSize.x ||
                tileMax.y < localViewportMin.y || tileMin.y > localViewportMin.y + viewportSize.y) {
                continue;
            }
            
            ImTextureID texId = m_mapProvider->GetTileTexture(ty * 12 + tx);
            if (texId) {
                drawList->AddImage(texId, tileMin, tileMax);
            } else {
                // Vector grid fallback
                drawList->AddRectFilled(tileMin, tileMax, IM_COL32(24, 38, 28, 255)); // dark forest green
                drawList->AddRect(tileMin, tileMax, IM_COL32(255, 255, 255, 15));   // grid lines
                
                // Draw tile coordinates in sandbox
                char label[16];
                sprintf(label, "%02d,%02d", tx, ty);
                ImVec2 labelSize = ImGui::CalcTextSize(label);
                drawList->AddText(ImVec2(tileMin.x + (256.0f * m_zoom - labelSize.x) * 0.5f,
                                         tileMin.y + (256.0f * m_zoom - labelSize.y) * 0.5f),
                                  IM_COL32(255, 255, 255, 40), label);
            }
        }
    }
    
    // 3. Render Waypoint
    float wx = 0.0f, wy = 0.0f;
    if (m_mapProvider->GetWaypointPosition(wx, wy)) {
        ImVec2 mapWp = worldToMap(wx, wy, 3072.0f);
        ImVec2 wpScreen = ImVec2(localViewportMin.x + (mapWp.x - m_scroll.x) * m_zoom + viewportSize.x * 0.5f,
                                 localViewportMin.y + (mapWp.y - m_scroll.y) * m_zoom + viewportSize.y * 0.5f);
        
        ImTextureID wpTex = m_mapProvider->GetBlipTexture(41); // RADAR_SPRITE_WAYPOINT = 41
        if (wpTex) {
            ImVec2 wpMin = ImVec2(wpScreen.x - 12.0f, wpScreen.y - 12.0f);
            ImVec2 wpMax = ImVec2(wpScreen.x + 12.0f, wpScreen.y + 12.0f);
            drawList->AddImage(wpTex, wpMin, wpMax);
        } else {
            // Draw fallback Red Flag / Crosshair
            drawList->AddCircleFilled(wpScreen, 7.0f, IM_COL32(255, 59, 48, 255)); // iOS Red
            drawList->AddCircle(wpScreen, 9.0f, IM_COL32(255, 255, 255, 220), 12, 1.5f);
            
            drawList->AddLine(ImVec2(wpScreen.x - 12.0f, wpScreen.y), ImVec2(wpScreen.x + 12.0f, wpScreen.y), IM_COL32(255, 59, 48, 255), 2.0f);
            drawList->AddLine(ImVec2(wpScreen.x, wpScreen.y - 12.0f), ImVec2(wpScreen.x, wpScreen.y + 12.0f), IM_COL32(255, 59, 48, 255), 2.0f);
        }
    }
    
    // 4. Render Active Blips (Discovered Locations)
    std::vector<MapBlip> blips = m_mapProvider->GetActiveBlips();
    int hoveredBlipIdx = -1;
    for (size_t i = 0; i < blips.size(); ++i) {
        const auto& blip = blips[i];
        ImVec2 mapBl = worldToMap(blip.x, blip.y, 3072.0f);
        ImVec2 blScreen = ImVec2(localViewportMin.x + (mapBl.x - m_scroll.x) * m_zoom + viewportSize.x * 0.5f,
                                 localViewportMin.y + (mapBl.y - m_scroll.y) * m_zoom + viewportSize.y * 0.5f);
        
        // Assign color and icon based on sprite type using helper function
        std::string catName;
        const char* iconStr = nullptr;
        ImVec4 itemColor;
        GetCategoryMeta(blip.sprite, catName, iconStr, itemColor);
        ImU32 color = ImGui::ColorConvertFloat4ToU32(itemColor);
        
        // Render blip icon
        ImTextureID blipTex = m_mapProvider->GetBlipTexture(blip.sprite);
        if (blipTex) {
            ImVec2 blMin = ImVec2(blScreen.x - 9.0f, blScreen.y - 9.0f);
            ImVec2 blMax = ImVec2(blScreen.x + 9.0f, blScreen.y + 9.0f);
            drawList->AddImage(blipTex, blMin, blMax);
        } else {
            // Render blip icon circle (fallback/Sandbox)
            drawList->AddCircleFilled(blScreen, 9.0f, color);
            drawList->AddCircle(blScreen, 9.0f, IM_COL32(255, 255, 255, 255), 12, 1.0f);
            
            // Render icon char in center of blip circle
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // FontAwesome/Roboto
            float iconWidth = ImGui::CalcTextSize(iconStr).x;
            float iconHeight = ImGui::CalcTextSize(iconStr).y;
            drawList->AddText(ImVec2(blScreen.x - iconWidth * 0.5f, blScreen.y - iconHeight * 0.5f), IM_COL32(255, 255, 255, 255), iconStr);
            ImGui::PopFont();
        }
        
        // Hover detection
        float dx = io.MousePos.x - blScreen.x;
        float dy = io.MousePos.y - blScreen.y;
        if (dx * dx + dy * dy < 100.0f) {
            hoveredBlipIdx = static_cast<int>(i);
        }
    }
    
    // Draw hovered tooltip label
    if (hoveredBlipIdx != -1) {
        const auto& blip = blips[hoveredBlipIdx];
        ImVec2 mapBl = worldToMap(blip.x, blip.y, 3072.0f);
        ImVec2 blScreen = ImVec2(localViewportMin.x + (mapBl.x - m_scroll.x) * m_zoom + viewportSize.x * 0.5f,
                                 localViewportMin.y + (mapBl.y - m_scroll.y) * m_zoom + viewportSize.y * 0.5f);
        
        // Draw nice tooltip frame above blip
        std::string labelText = blip.name;
        ImVec2 labelSize = ImGui::CalcTextSize(labelText.c_str());
        ImVec2 rectMin = ImVec2(blScreen.x - labelSize.x * 0.5f - 6.0f, blScreen.y - 30.0f - labelSize.y * 0.5f);
        ImVec2 rectMax = ImVec2(blScreen.x + labelSize.x * 0.5f + 6.0f, blScreen.y - 30.0f + labelSize.y * 0.5f);
        
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, 220), 4.0f);
        drawList->AddRect(rectMin, rectMax, IM_COL32(255, 255, 255, 100), 4.0f);
        drawList->AddText(ImVec2(rectMin.x + 6.0f, rectMin.y + 1.0f), IM_COL32(255, 255, 255, 255), labelText.c_str());
    }
    
    // 5. Render Player Position
    float px = 0.0f, py = 0.0f, ph = 0.0f;
    if (m_mapProvider->GetPlayerPosition(px, py, ph)) {
        ImVec2 mapPl = worldToMap(px, py, 3072.0f);
        ImVec2 plScreen = ImVec2(localViewportMin.x + (mapPl.x - m_scroll.x) * m_zoom + viewportSize.x * 0.5f,
                                 localViewportMin.y + (mapPl.y - m_scroll.y) * m_zoom + viewportSize.y * 0.5f);
        
        // Trig calculations for Arrow direction
        // heading is in degrees, convert to radians. 0 is North/up.
        float rad = ph * (M_PI / 180.0f);
        
        ImVec2 nose = ImVec2(plScreen.x + sinf(rad) * 11.0f, plScreen.y - cosf(rad) * 11.0f);
        ImVec2 tailL = ImVec2(plScreen.x + sinf(rad - 2.5f) * 8.0f, plScreen.y - cosf(rad - 2.5f) * 8.0f);
        ImVec2 tailR = ImVec2(plScreen.x + sinf(rad + 2.5f) * 8.0f, plScreen.y - cosf(rad + 2.5f) * 8.0f);
        
        // Draw blue player arrow with white border
        drawList->AddTriangleFilled(nose, tailL, tailR, IM_COL32(0, 122, 255, 255)); // iOS Blue
        drawList->AddTriangle(nose, tailL, tailR, IM_COL32(255, 255, 255, 255), 1.5f);
    }
    
    ImGui::EndChild();
    
    // 6. Draw Sidebar List Overlay
    if (m_showSidebar) {
        ImGui::SetCursorScreenPos(viewportMin);
        ImGui::BeginChild("##MapSidebar", viewportSize, false, ImGuiWindowFlags_NoScrollbar);
        
        // Black iOS-style background
        ImGui::GetWindowDrawList()->AddRectFilled(viewportMin, viewportMax, IM_COL32(18, 18, 20, 240));
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
        ImGui::Indent(15.0f);
        ImGui::Text(TR("maps.available_places"));
        ImGui::Unindent(15.0f);
        ImGui::Spacing();
        ImGui::Separator();
        
        // Quick options
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
        bool hasWp = m_mapProvider->GetWaypointPosition(wx, wy);
        
        if (hasWp) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 0.8f));
            if (ImGui::Button((std::string(ICON_FA_TRASH_ALT " ") + TR("maps.remove_route")).c_str(), ImVec2(viewportSize.x - 30.0f, 35.0f))) {
                m_mapProvider->ClearWaypoint();
                m_showSidebar = false;
            }
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }
        
        // Scrollable list content
        ImGui::BeginChild("##SidebarScroll", ImVec2(viewportSize.x - 10.0f, viewportSize.y - (hasWp ? 100.0f : 60.0f)), false);
        
        if (blips.empty()) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), TR("maps.no_places"));
        } else {
            // Get unique sprite categories from discovered blips
            std::vector<int> uniqueSprites;
            for (const auto& blip : blips) {
                if (std::find(uniqueSprites.begin(), uniqueSprites.end(), blip.sprite) == uniqueSprites.end()) {
                    uniqueSprites.push_back(blip.sprite);
                }
            }
            
            // Sort categories by name
            std::sort(uniqueSprites.begin(), uniqueSprites.end(), [&](int a, int b) {
                std::string nameA, nameB;
                const char* iconA = nullptr; const char* iconB = nullptr;
                ImVec4 colA, colB;
                GetCategoryMeta(a, nameA, iconA, colA);
                GetCategoryMeta(b, nameB, iconB, colB);
                return nameA < nameB;
            });
            
            // Group unique categories
            std::vector<int> safehouses, shops, services, others;
            for (int sprite : uniqueSprites) {
                if (sprite == 35 || sprite == 31 || sprite == 32) {
                    safehouses.push_back(sprite);
                } else if (sprite == 6 || sprite == 45 || sprite == 10 || sprite == 14 || 
                           sprite == 29 || sprite == 17 || sprite == 25 || sprite == 44 || 
                           sprite == 47 || sprite == 49 || sprite == 50) {
                    shops.push_back(sprite);
                } else if (sprite == 63 || sprite == 27 || sprite == 7 || sprite == 54 || 
                           sprite == 5 || sprite == 9 || sprite == 22 || sprite == 30 || 
                           sprite == 36 || sprite == 39 || sprite == 55) {
                    services.push_back(sprite);
                } else {
                    others.push_back(sprite);
                }
            }
            
            auto drawCategoryList = [&](const char* title, const std::vector<int>& list) {
                if (list.empty()) return;
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), title);
                ImGui::Spacing();
                
                // iOS Rounded Group List
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 0.9f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                
                float height = list.size() * 45.0f;
                ImGui::BeginChild(title, ImVec2(viewportSize.x - 20.0f, height), false, ImGuiWindowFlags_NoScrollbar);
                
                for (size_t idx = 0; idx < list.size(); ++idx) {
                    int sprite = list[idx];
                    
                    ImGui::SetCursorPosY(idx * 45.0f + 10.0f);
                    ImGui::Indent(10.0f);
                    
                    std::string catName;
                    const char* iconStr = nullptr;
                    ImVec4 itemColor;
                    GetCategoryMeta(sprite, catName, iconStr, itemColor);
                    
                    // Icon
                    ImTextureID blipTex = m_mapProvider->GetBlipTexture(sprite);
                    if (blipTex) {
                        ImGui::Image(blipTex, ImVec2(16.0f, 16.0f));
                    } else {
                        ImGui::TextColored(itemColor, iconStr);
                    }
                    ImGui::SameLine();
                    
                    // Name
                    ImGui::Text("%s", catName.c_str());
                    
                    // Go button (right aligned)
                    ImGui::SameLine(viewportSize.x - 70.0f);
                    ImGui::SetCursorPosY(idx * 45.0f + 5.0f);
                    
                    char btnId[32];
                    sprintf(btnId, "%s##%d", TR("maps.go"), sprite);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.52f, 1.0f, 0.9f));
                    if (ImGui::Button(btnId, ImVec2(40.0f, 26.0f))) {
                        // Find closest blip of this sprite type to player
                        float px = 0.0f, py = 0.0f, ph = 0.0f;
                        m_mapProvider->GetPlayerPosition(px, py, ph);
                        
                        float minDst = 1e9f;
                        MapBlip closestBlip;
                        bool found = false;
                        
                        for (const auto& b : blips) {
                            if (b.sprite == sprite) {
                                float dx = b.x - px;
                                float dy = b.y - py;
                                float dst = dx * dx + dy * dy;
                                if (dst < minDst) {
                                    minDst = dst;
                                    closestBlip = b;
                                    found = true;
                                }
                            }
                        }
                        
                        if (found) {
                            m_mapProvider->SetWaypointPosition(closestBlip.x, closestBlip.y);
                            m_scroll = worldToMap(closestBlip.x, closestBlip.y, 3072.0f);
                        }
                        m_showSidebar = false;
                    }
                    ImGui::PopStyleColor();
                    
                    ImGui::Unindent(10.0f);
                    
                    // Dividers
                    if (idx < list.size() - 1) {
                        ImGui::GetWindowDrawList()->AddLine(
                            ImVec2(ImGui::GetWindowPos().x + 10.0f, ImGui::GetWindowPos().y + (idx + 1) * 45.0f),
                            ImVec2(ImGui::GetWindowPos().x + viewportSize.x - 30.0f, ImGui::GetWindowPos().y + (idx + 1) * 45.0f),
                            IM_COL32(255, 255, 255, 20)
                        );
                    }
                }
                
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            };
            
            drawCategoryList(TR("maps.safehouses"), safehouses);
            drawCategoryList(TR("maps.shops"), shops);
            drawCategoryList(TR("maps.services"), services);
            drawCategoryList(TR("maps.others"), others);
        }
        
        ImGui::EndChild();
        ImGui::EndChild();
    }
}
