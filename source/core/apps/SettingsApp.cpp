#include "SettingsApp.h"
#include <IconsFontAwesome5.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>

const ImVec4 SettingsApp::PRESET_SOLIDS[PRESET_COUNT] = {
    ImVec4(0.039f, 0.518f, 1.0f, 1.0f), // iOS Blue
    ImVec4(0.20f, 0.20f, 0.22f, 1.0f), // Space Gray
    ImVec4(0.13f, 0.56f, 0.36f, 1.0f), // Forest Green
    ImVec4(0.80f, 0.15f, 0.18f, 1.0f), // Crimson Red
    ImVec4(0.40f, 0.20f, 0.60f, 1.0f), // Royal Purple
    ImVec4(0.95f, 0.60f, 0.10f, 1.0f)  // Amber Gold
};

const std::pair<ImVec4, ImVec4> SettingsApp::PRESET_GRADIENTS[PRESET_COUNT] = {
    { ImVec4(0.29f, 0.10f, 0.48f, 1.0f), ImVec4(0.10f, 0.10f, 0.23f, 1.0f) }, // Classic Moon (Purple -> Dark Blue)
    { ImVec4(0.95f, 0.30f, 0.30f, 1.0f), ImVec4(0.95f, 0.60f, 0.10f, 1.0f) }, // Sunset (Warm Orange/Red)
    { ImVec4(0.05f, 0.15f, 0.30f, 1.0f), ImVec4(0.01f, 0.03f, 0.08f, 1.0f) }, // Midnight (Deep Indigo/Black)
    { ImVec4(0.10f, 0.60f, 0.60f, 1.0f), ImVec4(0.05f, 0.20f, 0.40f, 1.0f) }, // Ocean (Teal -> Royal Blue)
    { ImVec4(0.10f, 0.50f, 0.15f, 1.0f), ImVec4(0.03f, 0.20f, 0.08f, 1.0f) }, // Emerald Forest (Bright -> Dark Green)
    { ImVec4(0.90f, 0.50f, 0.60f, 1.0f), ImVec4(0.40f, 0.20f, 0.40f, 1.0f) }  // Rose Gold (Pink -> Deep Violet)
};

const char* SettingsApp::PRESET_GRADIENT_NAMES[PRESET_COUNT] = {
    "Classic Moon",
    "Sunset",
    "Midnight",
    "Ocean Blue",
    "Emerald Forest",
    "Rose Gold"
};

SettingsApp::SettingsApp() {
    id = "settings";
    icon = ICON_FA_COG;
    name = TR("settings.title");
    color = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
    dock = false;
    dockOrder = 99;
}

void SettingsApp::onOpen() {
    // Scan wallpapers when settings open
    Inject<IWallpaperProvider> wp;
    if (wp.isValid()) {
        wp->Refresh();
        m_availableWallpapers = wp->GetAvailableWallpapers();
    }
    m_wallpapersLoaded = true;
    m_menuState = 0; // Default to Settings Home list
}

void SettingsApp::onClose() {
    m_wallpapersLoaded = false;
}

bool SettingsApp::onBack() {
    if (m_menuState > 0) {
        m_menuState = 0; // Return to Settings Home
        return true; // We handled the back button press
    }
    return false; // Let the phone close the app
}

static void DrawInlineColorWheel(const char* label, ImVec4& color) {
    ImGui::PushID(label);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", label);
    ImGui::Spacing();
    
    // Set item width to fit perfectly inside the phone screen
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    
    // Render the color picker with Hue Wheel flag
    ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | 
                                ImGuiColorEditFlags_NoAlpha | 
                                ImGuiColorEditFlags_NoInputs | 
                                ImGuiColorEditFlags_NoSidePreview |
                                ImGuiColorEditFlags_NoLabel;
                                
    ImGui::ColorPicker4("##wheel", &color.x, flags);
    
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::Spacing();
}

void SettingsApp::onDraw() {
    // 0: Settings Menu, 1: Wallpaper sub-page, 2: Language sub-page
    if (m_menuState == 0) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), TR("settings.general"));
        
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
        ImGui::BeginChild("##settings_rows", ImVec2(ImGui::GetContentRegionAvail().x, 88.0f), true, ImGuiWindowFlags_NoScrollbar);
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Custom interactive row for iOS-like styling with subtle interactive highlights
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.03f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 1.0f, 1.0f, 0.07f));
        
        // --- Row: Wallpaper ---
        ImVec2 itemMin1 = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##wp_row_action", false, ImGuiSelectableFlags_None, ImVec2(0, 38.0f))) {
            m_menuState = 1; // Go to Wallpaper Sub-page
        }
        
        // Render custom elements on top of the selectable
        {
            float sz = 24.0f;
            ImVec2 rectMin = ImVec2(itemMin1.x + 8.0f, itemMin1.y + (38.0f - sz) * 0.5f);
            ImVec2 rectMax = ImVec2(rectMin.x + sz, rectMin.y + sz);
            
            // Draw iOS Wallpaper blue background
            drawList->AddRectFilled(rectMin, rectMax, IM_COL32(0, 122, 255, 255), 6.0f);
            
            // Draw Icon
            ImVec2 textSz = ImGui::CalcTextSize(ICON_FA_IMAGE);
            ImVec2 iconPos = ImVec2(rectMin.x + (sz - textSz.x) * 0.5f + 1.0f, rectMin.y + (sz - textSz.y) * 0.5f + 1.0f);
            drawList->AddText(iconPos, IM_COL32(255, 255, 255, 255), ICON_FA_IMAGE);
            
            // Draw Label
            ImVec2 textPos = ImVec2(rectMax.x + 12.0f, itemMin1.y + (38.0f - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), TR("settings.wallpaper"));
            
            // Draw Chevron
            ImVec2 chevronPos = ImVec2(itemMin1.x + ImGui::GetWindowWidth() - 30.0f, itemMin1.y + (38.0f - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(chevronPos, IM_COL32(255, 255, 255, 100), ICON_FA_CHEVRON_RIGHT);
            
            // Draw thin divider line aligned with text
            ImVec2 divStart = ImVec2(itemMin1.x + 44.0f, itemMin1.y + 38.0f);
            ImVec2 divEnd = ImVec2(itemMin1.x + ImGui::GetContentRegionAvail().x, itemMin1.y + 38.0f);
            drawList->AddLine(divStart, divEnd, IM_COL32(255, 255, 255, 20));
        }
        
        // --- Row: Language ---
        ImVec2 itemMin2 = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##lang_row_action", false, ImGuiSelectableFlags_None, ImVec2(0, 38.0f))) {
            m_menuState = 2; // Go to Language Sub-page
        }
        
        // Render custom elements on top of the selectable
        {
            float sz = 24.0f;
            ImVec2 rectMin = ImVec2(itemMin2.x + 8.0f, itemMin2.y + (38.0f - sz) * 0.5f);
            ImVec2 rectMax = ImVec2(rectMin.x + sz, rectMin.y + sz);
            
            // Draw iOS Settings green background
            drawList->AddRectFilled(rectMin, rectMax, IM_COL32(38, 140, 64, 255), 6.0f);
            
            // Draw Icon
            ImVec2 textSz = ImGui::CalcTextSize(ICON_FA_GLOBE);
            ImVec2 iconPos = ImVec2(rectMin.x + (sz - textSz.x) * 0.5f + 1.0f, rectMin.y + (sz - textSz.y) * 0.5f + 1.0f);
            drawList->AddText(iconPos, IM_COL32(255, 255, 255, 255), ICON_FA_GLOBE);
            
            // Draw Label
            ImVec2 textPos = ImVec2(rectMax.x + 12.0f, itemMin2.y + (38.0f - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), TR("settings.language"));
            
            // Draw Chevron
            ImVec2 chevronPos = ImVec2(itemMin2.x + ImGui::GetWindowWidth() - 30.0f, itemMin2.y + (38.0f - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(chevronPos, IM_COL32(255, 255, 255, 100), ICON_FA_CHEVRON_RIGHT);
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    else if (m_menuState == 1) {
        // Wallpaper Selection Page
        // Wrap everything in a scrollable child window so it stays perfectly inside the phone screen
        ImGui::BeginChild("##wp_scrollable_container", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoBackground);
        
        // 1. Wallpaper Real-Time Preview
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 100.0f) / 2.0f);
        ImGui::BeginChild("##wp_preview", ImVec2(100.0f, 180.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
        
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pMin = ImGui::GetWindowPos();
        ImVec2 pMax = ImVec2(pMin.x + 100.0f, pMin.y + 180.0f);
        DrawWallpaper(draw, pMin, pMax, 10.0f);
        
        ImGui::EndChild();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // 2. Wallpaper Type Selector (Segmented control style with rounded corners)
        float btnW = std::floor(ImGui::GetContentRegionAvail().x / 3.0f) - 3.0f;
        
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        
        auto renderTab = [&](const char* label, WallpaperType type) {
            bool active = (m_config.type == type);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.10f, 0.12f, 1.0f)); // Neutral dark gray
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.05f, 0.06f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 0.4f)); // Neutral slate gray
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.35f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.22f, 0.4f));
            }
            
            bool clicked = ImGui::Button(label, ImVec2(btnW, 30.0f));
            ImGui::PopStyleColor(3);
            return clicked;
        };

        if (renderTab(TR("settings.wp.solid"), WallpaperType::SOLID)) {
            m_config.type = WallpaperType::SOLID;
        }
        ImGui::SameLine();
        if (renderTab(TR("settings.wp.gradient"), WallpaperType::GRADIENT)) {
            m_config.type = WallpaperType::GRADIENT;
        }
        ImGui::SameLine();
        if (renderTab(TR("settings.wp.image"), WallpaperType::IMAGE)) {
            m_config.type = WallpaperType::IMAGE;
            
            // Refresh list dynamically
            Inject<IWallpaperProvider> wp;
            if (wp.isValid()) {
                wp->Refresh();
                m_availableWallpapers = wp->GetAvailableWallpapers();
                if (!m_availableWallpapers.empty() && m_config.imageName.empty()) {
                    m_config.imageName = m_availableWallpapers[0];
                }
            }
        }
        
        ImGui::PopStyleVar(); // FrameRounding
        
        ImGui::Spacing();
        
        // 3. Configurations details based on active Tab
        if (m_config.type == WallpaperType::SOLID) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), TR("settings.wp.color_presets"));
            ImGui::Spacing();
            
            for (int i = 0; i < PRESET_COUNT; i++) {
                ImGui::PushID(i);
                ImGui::PushStyleColor(ImGuiCol_Button, PRESET_SOLIDS[i]);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PRESET_SOLIDS[i]);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, PRESET_SOLIDS[i]);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f); // Circular
                
                if (ImGui::Button("##preset_solid", ImVec2(30.0f, 30.0f))) {
                    m_config.solidColor = PRESET_SOLIDS[i];
                }
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);
                ImGui::PopID();
                
                if (i < PRESET_COUNT - 1) ImGui::SameLine(0, 12.0f);
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Custom inline color wheel (Zero popups!)
            DrawInlineColorWheel(TR("settings.wp.custom_color"), m_config.solidColor);
        }
        else if (m_config.type == WallpaperType::GRADIENT) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), TR("settings.wp.gradient_presets"));
            ImGui::Spacing();
            
            for (int i = 0; i < PRESET_COUNT; i++) {
                ImGui::PushID(i + 100);
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImVec2 size(35.0f, 25.0f);
                
                if (ImGui::Button("##preset_grad", size)) {
                    m_config.gradientColor1 = PRESET_GRADIENTS[i].first;
                    m_config.gradientColor2 = PRESET_GRADIENTS[i].second;
                }
                
                // Draw custom gradient color over button
                ImDrawList* dList = ImGui::GetWindowDrawList();
                ImU32 c1 = ImGui::ColorConvertFloat4ToU32(PRESET_GRADIENTS[i].first);
                ImU32 c2 = ImGui::ColorConvertFloat4ToU32(PRESET_GRADIENTS[i].second);
                dList->AddRectFilledMultiColor(pos, ImVec2(pos.x + size.x, pos.y + size.y), c1, c1, c2, c2);
                dList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 255, 255, 50));
                
                ImGui::PopID();
                if (i < PRESET_COUNT - 1) ImGui::SameLine(0, 10.0f);
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Custom inline color wheels for gradient ends (Zero popups!)
            DrawInlineColorWheel(TR("settings.wp.custom_top"), m_config.gradientColor1);
            ImGui::Separator();
            ImGui::Spacing();
            DrawInlineColorWheel(TR("settings.wp.custom_bottom"), m_config.gradientColor2);
        }
        else if (m_config.type == WallpaperType::IMAGE) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), TR("settings.wp.available_images"));
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25.0f);
            
            // Sync/Rescan button (circular button styled nicely)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.20f, 0.6f));
            if (ImGui::Button(ICON_FA_SYNC "##refresh_wp", ImVec2(20.0f, 20.0f))) {
                Inject<IWallpaperProvider> wp;
                if (wp.isValid()) {
                    wp->Refresh();
                    m_availableWallpapers = wp->GetAvailableWallpapers();
                }
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            
            ImGui::Spacing();
            
            if (m_availableWallpapers.empty()) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::BeginChild("##wp_empty_help", ImVec2(ImGui::GetContentRegionAvail().x, 80.0f), true);
                
                ImGui::TextWrapped("%s", TR("settings.wp.no_images"));
                
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            } else {
                // Style listbox and selectable items with matching rounding and neutral colors
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.14f, 0.16f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.28f, 0.8f)); // Neutral dark gray active
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.30f, 0.35f, 0.6f)); // Hover item
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.18f, 0.18f, 0.20f, 0.9f)); // Click item
                
                if (ImGui::BeginListBox("##wps_list", ImVec2(ImGui::GetContentRegionAvail().x, 150.0f))) {
                    for (const auto& name : m_availableWallpapers) {
                        bool selected = (m_config.imageName == name);
                        if (ImGui::Selectable(name.c_str(), selected)) {
                            m_config.imageName = name;
                        }
                        
                        if (selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::PopStyleColor(4);
                ImGui::PopStyleVar();
            }
        }
        
        ImGui::EndChild(); // ##wp_scrollable_container
    }
    else if (m_menuState == 2) {
        // Language Selection Page
        ImGui::BeginChild("##lang_scrollable", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoBackground);
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), TR("settings.lang.select"));
        ImGui::Spacing();
        
        const auto& languages = LocalizationManager::Get().GetAvailableLanguages();
        const std::string& currentLang = LocalizationManager::Get().GetCurrentLanguage();
        
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
        
        float rowH = 38.0f; // Uniform 38px row height matching Settings main menu
        float listH = languages.size() * rowH;
        ImGui::BeginChild("##lang_list", ImVec2(ImGui::GetContentRegionAvail().x, listH + 12.0f), true, ImGuiWindowFlags_NoScrollbar);
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.03f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 1.0f, 1.0f, 0.07f));
        
        for (size_t i = 0; i < languages.size(); i++) {
            const auto& lang = languages[i];
            bool isSelected = (lang.code == currentLang);
            
            ImGui::PushID((int)i);
            
            ImVec2 itemMin = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##lang_sel", false, ImGuiSelectableFlags_None, ImVec2(0, rowH))) {
                LocalizationManager::Get().SetLanguage(lang.code);
                name = TR("settings.title"); // Update title immediately
            }
            
            // Draw language display name over the selectable
            ImVec2 textPos = ImVec2(itemMin.x + 12.0f, itemMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), lang.name.c_str());
            
            // Draw iOS classic checkmark in blue for active language
            if (isSelected) {
                ImVec2 checkPos = ImVec2(itemMin.x + ImGui::GetWindowWidth() - 36.0f, itemMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f);
                drawList->AddText(checkPos, IM_COL32(10, 132, 255, 255), ICON_FA_CHECK);
            }
            
            // Draw thin divider line aligned with item content
            if (i < languages.size() - 1) {
                ImVec2 divStart = ImVec2(itemMin.x + 12.0f, itemMin.y + rowH);
                ImVec2 divEnd = ImVec2(itemMin.x + ImGui::GetContentRegionAvail().x, itemMin.y + rowH);
                drawList->AddLine(divStart, divEnd, IM_COL32(255, 255, 255, 20));
            }
            
            ImGui::PopID();
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        
        ImGui::EndChild(); // ##lang_scrollable
    }
}

static void DrawInverseCorner(ImDrawList* draw, ImVec2 center, float radius, int corner_idx, ImU32 color) {
    // corner_idx: 0 = Top-Left, 1 = Top-Right, 2 = Bottom-Right, 3 = Bottom-Left
    const int num_segments = 8;
    ImVec2 pts[num_segments + 3];
    
    ImVec2 corner_pos;
    float start_angle = 0.0f;
    const float pi = 3.1415926535f;
    
    if (corner_idx == 0) { // Top-Left
        corner_pos = ImVec2(center.x - radius, center.y - radius);
        start_angle = pi; // 180 deg to 270 deg
    } else if (corner_idx == 1) { // Top-Right
        corner_pos = ImVec2(center.x + radius, center.y - radius);
        start_angle = 1.5f * pi; // 270 deg to 360 deg
    } else if (corner_idx == 2) { // Bottom-Right
        corner_pos = ImVec2(center.x + radius, center.y + radius);
        start_angle = 0.0f; // 0 deg to 90 deg
    } else { // Bottom-Left
        corner_pos = ImVec2(center.x - radius, center.y + radius);
        start_angle = 0.5f * pi; // 90 deg to 180 deg
    }
    
    pts[0] = corner_pos;
    for (int i = 0; i <= num_segments; ++i) {
        float angle = start_angle + (float)i / (float)num_segments * (0.5f * pi);
        pts[i + 1] = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
    }
    
    draw->AddConvexPolyFilled(pts, num_segments + 2, color);
}

static void DrawRoundedScreenMask(ImDrawList* draw, ImVec2 pMin, ImVec2 pMax, float radius, ImU32 color) {
    // 0: Top-Left
    DrawInverseCorner(draw, ImVec2(pMin.x + radius, pMin.y + radius), radius, 0, color);
    // 1: Top-Right
    DrawInverseCorner(draw, ImVec2(pMax.x - radius, pMin.y + radius), radius, 1, color);
    // 2: Bottom-Right
    DrawInverseCorner(draw, ImVec2(pMax.x - radius, pMax.y - radius), radius, 2, color);
    // 3: Bottom-Left
    DrawInverseCorner(draw, ImVec2(pMin.x + radius, pMax.y - radius), radius, 3, color);
}

void SettingsApp::DrawWallpaper(ImDrawList* draw, ImVec2 pMin, ImVec2 pMax, float rounding) {
    ImU32 bezelColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.08f, 0.08f, 0.09f, 1.0f));

    if (m_config.type == WallpaperType::SOLID) {
        ImU32 col = ImGui::ColorConvertFloat4ToU32(m_config.solidColor);
        draw->AddRectFilled(pMin, pMax, col, rounding);
    } 
    else if (m_config.type == WallpaperType::GRADIENT) {
        ImU32 col1 = ImGui::ColorConvertFloat4ToU32(m_config.gradientColor1);
        ImU32 col2 = ImGui::ColorConvertFloat4ToU32(m_config.gradientColor2);
        
        // Draw sharp gradient block first
        draw->AddRectFilledMultiColor(pMin, pMax, col1, col1, col2, col2);
        
        // Apply corner rounding mask using mathematically perfect inverse corners
        DrawRoundedScreenMask(draw, pMin, pMax, rounding, bezelColor);
    } 
    else if (m_config.type == WallpaperType::IMAGE) {
        ImTextureID texId = 0;
        Inject<IWallpaperProvider> wallpaperProvider;
        if (wallpaperProvider.isValid()) {
            texId = wallpaperProvider->GetWallpaperTexture(m_config.imageName);
        }
        
        if (texId) {
            // Draw image with standard AddImage (which is fast and universally supported)
            draw->AddImage(texId, pMin, pMax);
            
            // Apply corner rounding mask using mathematically perfect inverse corners
            DrawRoundedScreenMask(draw, pMin, pMax, rounding, bezelColor);
        } else {
            // Fallback default gradient (Grove Street Green)
            ImU32 col1 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.08f, 0.35f, 0.18f, 1.0f));
            ImU32 col2 = ImGui::ColorConvertFloat4ToU32(ImVec4(0.02f, 0.10f, 0.05f, 1.0f));
            draw->AddRectFilledMultiColor(pMin, pMax, col1, col1, col2, col2);
            
            DrawRoundedScreenMask(draw, pMin, pMax, rounding, bezelColor);
        }
    }
}

// Persistence (automatically managed per game save slot)
void SettingsApp::onSave(nlohmann::json& out) {
    out["type"] = static_cast<int>(m_config.type);
    
    out["solidColor"] = { m_config.solidColor.x, m_config.solidColor.y, m_config.solidColor.z, m_config.solidColor.w };
    out["gradientColor1"] = { m_config.gradientColor1.x, m_config.gradientColor1.y, m_config.gradientColor1.z, m_config.gradientColor1.w };
    out["gradientColor2"] = { m_config.gradientColor2.x, m_config.gradientColor2.y, m_config.gradientColor2.z, m_config.gradientColor2.w };
    
    out["imageName"] = m_config.imageName;
    out["language"] = LocalizationManager::Get().GetCurrentLanguage();
}

void SettingsApp::onLoad(const nlohmann::json& in) {
    if (in.contains("type") && in["type"].is_number()) {
        m_config.type = static_cast<WallpaperType>(in["type"].get<int>());
    }
    
    if (in.contains("solidColor") && in["solidColor"].is_array() && in["solidColor"].size() == 4) {
        m_config.solidColor = ImVec4(in["solidColor"][0], in["solidColor"][1], in["solidColor"][2], in["solidColor"][3]);
    }
    if (in.contains("gradientColor1") && in["gradientColor1"].is_array() && in["gradientColor1"].size() == 4) {
        m_config.gradientColor1 = ImVec4(in["gradientColor1"][0], in["gradientColor1"][1], in["gradientColor1"][2], in["gradientColor1"][3]);
    }
    if (in.contains("gradientColor2") && in["gradientColor2"].is_array() && in["gradientColor2"].size() == 4) {
        m_config.gradientColor2 = ImVec4(in["gradientColor2"][0], in["gradientColor2"][1], in["gradientColor2"][2], in["gradientColor2"][3]);
    }
    
    if (in.contains("imageName") && in["imageName"].is_string()) {
        m_config.imageName = in["imageName"].get<std::string>();
    }
    
    if (in.contains("language") && in["language"].is_string()) {
        LocalizationManager::Get().SetLanguage(in["language"].get<std::string>());
        name = TR("settings.title");
    }
}

void SettingsApp::onWipe() {
    m_config = WallpaperConfig();
}
