#include "GarageApp.h"
#include "../Phone.h"
#include "../LocalizationManager.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>

extern Phone phone;

GarageApp::GarageApp() {
    id = "garage";
    icon = ICON_FA_CAR;
    name = TR("garage.title");
    // Accent blue/cyan color
    color = ImVec4(0.196f, 0.541f, 0.988f, 1.0f);
    dock = true;
    dockOrder = 4;
}
void GarageApp::onOpen() {
    m_selectedIdx = -1;
    m_renamingIdx = -1;
    m_editingPlateIdx = -1;
    m_renameBuf[0] = '\0';
    m_plateBuf[0] = '\0';
}

void GarageApp::onClose() {
    m_selectedIdx = -1;
    m_renamingIdx = -1;
    m_editingPlateIdx = -1;
}

void GarageApp::update(float dt) {
    if (m_provider) {
        m_provider->Update(dt);
    }
}

bool GarageApp::onBack() {
    if (m_selectedIdx != -1) {
        m_selectedIdx = -1;
        m_renamingIdx = -1;
        m_editingPlateIdx = -1;
        return true; // Handled internally
    }
    return false; // Let the phone go home
}

void GarageApp::onSave(nlohmann::json& out) {
    nlohmann::json listJson = nlohmann::json::array();
    for (const auto& v : m_vehicles) {
        nlohmann::json vJson = nlohmann::json::object();
        vJson["name"] = v.info.name;
        vJson["model"] = v.info.model;
        vJson["color1"] = v.info.color1;
        vJson["color2"] = v.info.color2;
        vJson["paintjob"] = v.info.paintjob;
        vJson["mods"] = v.info.mods;
        vJson["hydraulics"] = v.info.hydraulics;
        vJson["plate"] = v.info.plate;
        listJson.push_back(vJson);
    }
    out["vehicles"] = listJson;
}

void GarageApp::onLoad(const nlohmann::json& in) {
    m_vehicles.clear();
    m_selectedIdx = -1;
    m_renamingIdx = -1;
    m_editingPlateIdx = -1;
    if (in.contains("vehicles") && in["vehicles"].is_array()) {
        for (const auto& vJson : in["vehicles"]) {
            SavedVehicle v;
            if (vJson.contains("name")) v.info.name = vJson["name"].get<std::string>();
            if (vJson.contains("model")) v.info.model = vJson["model"].get<int>();
            if (vJson.contains("color1")) v.info.color1 = vJson["color1"].get<int>();
            if (vJson.contains("color2")) v.info.color2 = vJson["color2"].get<int>();
            if (vJson.contains("paintjob")) v.info.paintjob = vJson["paintjob"].get<int>();
            if (vJson.contains("mods")) v.info.mods = vJson["mods"].get<std::vector<int>>();
            if (vJson.contains("hydraulics")) v.info.hydraulics = vJson["hydraulics"].get<bool>();
            if (vJson.contains("plate")) v.info.plate = vJson["plate"].get<std::string>();
            m_vehicles.push_back(v);
        }
    }
}

void GarageApp::onWipe() {
    m_vehicles.clear();
    m_selectedIdx = -1;
    m_renamingIdx = -1;
    m_editingPlateIdx = -1;
}

namespace {
    static const ImVec4 g_gtaColors[] = {
        ImVec4(0/255.0f, 0/255.0f, 0/255.0f, 1.0f), // 0
        ImVec4(245/255.0f, 245/255.0f, 245/255.0f, 1.0f), // 1
        ImVec4(42/255.0f, 119/255.0f, 161/255.0f, 1.0f), // 2
        ImVec4(132/255.0f, 4/255.0f, 16/255.0f, 1.0f), // 3
        ImVec4(38/255.0f, 55/255.0f, 57/255.0f, 1.0f), // 4
        ImVec4(134/255.0f, 68/255.0f, 110/255.0f, 1.0f), // 5
        ImVec4(215/255.0f, 142/255.0f, 16/255.0f, 1.0f), // 6
        ImVec4(76/255.0f, 117/255.0f, 183/255.0f, 1.0f), // 7
        ImVec4(189/255.0f, 190/255.0f, 198/255.0f, 1.0f), // 8
        ImVec4(94/255.0f, 112/255.0f, 114/255.0f, 1.0f), // 9
        ImVec4(70/255.0f, 89/255.0f, 122/255.0f, 1.0f), // 10
        ImVec4(101/255.0f, 106/255.0f, 121/255.0f, 1.0f), // 11
        ImVec4(93/255.0f, 126/255.0f, 141/255.0f, 1.0f), // 12
        ImVec4(88/255.0f, 89/255.0f, 90/255.0f, 1.0f), // 13
        ImVec4(214/255.0f, 218/255.0f, 214/255.0f, 1.0f), // 14
        ImVec4(156/255.0f, 161/255.0f, 163/255.0f, 1.0f), // 15
        ImVec4(51/255.0f, 95/255.0f, 63/255.0f, 1.0f), // 16
        ImVec4(115/255.0f, 14/255.0f, 26/255.0f, 1.0f), // 17
        ImVec4(123/255.0f, 10/255.0f, 42/255.0f, 1.0f), // 18
        ImVec4(159/255.0f, 157/255.0f, 148/255.0f, 1.0f), // 19
        ImVec4(59/255.0f, 78/255.0f, 120/255.0f, 1.0f), // 20
        ImVec4(115/255.0f, 46/255.0f, 62/255.0f, 1.0f), // 21
        ImVec4(105/255.0f, 30/255.0f, 59/255.0f, 1.0f), // 22
        ImVec4(150/255.0f, 145/255.0f, 140/255.0f, 1.0f), // 23
        ImVec4(81/255.0f, 84/255.0f, 89/255.0f, 1.0f), // 24
        ImVec4(63/255.0f, 62/255.0f, 69/255.0f, 1.0f), // 25
        ImVec4(165/255.0f, 169/255.0f, 167/255.0f, 1.0f), // 26
        ImVec4(99/255.0f, 92/255.0f, 90/255.0f, 1.0f), // 27
        ImVec4(61/255.0f, 74/255.0f, 104/255.0f, 1.0f), // 28
        ImVec4(151/255.0f, 149/255.0f, 146/255.0f, 1.0f), // 29
        ImVec4(66/255.0f, 31/255.0f, 33/255.0f, 1.0f), // 30
        ImVec4(95/255.0f, 39/255.0f, 43/255.0f, 1.0f), // 31
        ImVec4(132/255.0f, 148/255.0f, 171/255.0f, 1.0f), // 32
        ImVec4(118/255.0f, 123/255.0f, 124/255.0f, 1.0f), // 33
        ImVec4(100/255.0f, 100/255.0f, 100/255.0f, 1.0f), // 34
        ImVec4(90/255.0f, 87/255.0f, 82/255.0f, 1.0f), // 35
        ImVec4(37/255.0f, 37/255.0f, 39/255.0f, 1.0f), // 36
        ImVec4(45/255.0f, 58/255.0f, 53/255.0f, 1.0f), // 37
        ImVec4(147/255.0f, 163/255.0f, 150/255.0f, 1.0f), // 38
        ImVec4(109/255.0f, 122/255.0f, 136/255.0f, 1.0f), // 39
        ImVec4(34/255.0f, 25/255.0f, 24/255.0f, 1.0f), // 40
        ImVec4(111/255.0f, 103/255.0f, 95/255.0f, 1.0f), // 41
        ImVec4(124/255.0f, 28/255.0f, 42/255.0f, 1.0f), // 42
        ImVec4(95/255.0f, 10/255.0f, 21/255.0f, 1.0f), // 43
        ImVec4(25/255.0f, 56/255.0f, 38/255.0f, 1.0f), // 44
        ImVec4(93/255.0f, 27/255.0f, 32/255.0f, 1.0f), // 45
        ImVec4(157/255.0f, 152/255.0f, 114/255.0f, 1.0f), // 46
        ImVec4(122/255.0f, 117/255.0f, 96/255.0f, 1.0f), // 47
        ImVec4(152/255.0f, 149/255.0f, 134/255.0f, 1.0f), // 48
        ImVec4(173/255.0f, 176/255.0f, 176/255.0f, 1.0f), // 49
        ImVec4(132/255.0f, 137/255.0f, 136/255.0f, 1.0f), // 50
        ImVec4(48/255.0f, 79/255.0f, 69/255.0f, 1.0f), // 51
        ImVec4(77/255.0f, 98/255.0f, 104/255.0f, 1.0f), // 52
        ImVec4(22/255.0f, 34/255.0f, 72/255.0f, 1.0f), // 53
        ImVec4(39/255.0f, 47/255.0f, 75/255.0f, 1.0f), // 54
        ImVec4(125/255.0f, 98/255.0f, 86/255.0f, 1.0f), // 55
        ImVec4(158/255.0f, 164/255.0f, 171/255.0f, 1.0f), // 56
        ImVec4(156/255.0f, 141/255.0f, 113/255.0f, 1.0f), // 57
        ImVec4(109/255.0f, 24/255.0f, 34/255.0f, 1.0f), // 58
        ImVec4(78/255.0f, 104/255.0f, 129/255.0f, 1.0f), // 59
        ImVec4(156/255.0f, 156/255.0f, 152/255.0f, 1.0f), // 60
        ImVec4(145/255.0f, 115/255.0f, 71/255.0f, 1.0f), // 61
        ImVec4(102/255.0f, 28/255.0f, 38/255.0f, 1.0f), // 62
        ImVec4(148/255.0f, 157/255.0f, 159/255.0f, 1.0f), // 63
        ImVec4(164/255.0f, 167/255.0f, 165/255.0f, 1.0f), // 64
        ImVec4(142/255.0f, 140/255.0f, 70/255.0f, 1.0f), // 65
        ImVec4(52/255.0f, 26/255.0f, 30/255.0f, 1.0f), // 66
        ImVec4(106/255.0f, 122/255.0f, 140/255.0f, 1.0f), // 67
        ImVec4(170/255.0f, 173/255.0f, 142/255.0f, 1.0f), // 68
        ImVec4(171/255.0f, 152/255.0f, 143/255.0f, 1.0f), // 69
        ImVec4(133/255.0f, 31/255.0f, 46/255.0f, 1.0f), // 70
        ImVec4(111/255.0f, 130/255.0f, 151/255.0f, 1.0f), // 71
        ImVec4(88/255.0f, 88/255.0f, 83/255.0f, 1.0f), // 72
        ImVec4(154/255.0f, 167/255.0f, 144/255.0f, 1.0f), // 73
        ImVec4(96/255.0f, 26/255.0f, 35/255.0f, 1.0f), // 74
        ImVec4(32/255.0f, 32/255.0f, 44/255.0f, 1.0f), // 75
        ImVec4(164/255.0f, 160/255.0f, 150/255.0f, 1.0f), // 76
        ImVec4(170/255.0f, 157/255.0f, 132/255.0f, 1.0f), // 77
        ImVec4(120/255.0f, 34/255.0f, 43/255.0f, 1.0f), // 78
        ImVec4(14/255.0f, 49/255.0f, 109/255.0f, 1.0f), // 79
        ImVec4(114/255.0f, 42/255.0f, 63/255.0f, 1.0f), // 80
        ImVec4(123/255.0f, 113/255.0f, 94/255.0f, 1.0f), // 81
        ImVec4(116/255.0f, 29/255.0f, 40/255.0f, 1.0f), // 82
        ImVec4(30/255.0f, 46/255.0f, 50/255.0f, 1.0f), // 83
        ImVec4(77/255.0f, 50/255.0f, 47/255.0f, 1.0f), // 84
        ImVec4(124/255.0f, 27/255.0f, 68/255.0f, 1.0f), // 85
        ImVec4(46/255.0f, 91/255.0f, 32/255.0f, 1.0f), // 86
        ImVec4(57/255.0f, 90/255.0f, 131/255.0f, 1.0f), // 87
        ImVec4(109/255.0f, 40/255.0f, 55/255.0f, 1.0f), // 88
        ImVec4(167/255.0f, 162/255.0f, 143/255.0f, 1.0f), // 89
        ImVec4(175/255.0f, 177/255.0f, 177/255.0f, 1.0f), // 90
        ImVec4(54/255.0f, 65/255.0f, 85/255.0f, 1.0f), // 91
        ImVec4(109/255.0f, 108/255.0f, 110/255.0f, 1.0f), // 92
        ImVec4(15/255.0f, 106/255.0f, 137/255.0f, 1.0f), // 93
        ImVec4(32/255.0f, 75/255.0f, 107/255.0f, 1.0f), // 94
        ImVec4(43/255.0f, 62/255.0f, 87/255.0f, 1.0f), // 95
        ImVec4(155/255.0f, 159/255.0f, 157/255.0f, 1.0f), // 96
        ImVec4(108/255.0f, 132/255.0f, 149/255.0f, 1.0f), // 97
        ImVec4(77/255.0f, 93/255.0f, 96/255.0f, 1.0f), // 98
        ImVec4(174/255.0f, 155/255.0f, 127/255.0f, 1.0f), // 99
        ImVec4(64/255.0f, 108/255.0f, 143/255.0f, 1.0f), // 100
        ImVec4(31/255.0f, 37/255.0f, 59/255.0f, 1.0f), // 101
        ImVec4(171/255.0f, 146/255.0f, 118/255.0f, 1.0f), // 102
        ImVec4(19/255.0f, 69/255.0f, 115/255.0f, 1.0f), // 103
        ImVec4(150/255.0f, 129/255.0f, 108/255.0f, 1.0f), // 104
        ImVec4(100/255.0f, 104/255.0f, 106/255.0f, 1.0f), // 105
        ImVec4(16/255.0f, 80/255.0f, 130/255.0f, 1.0f), // 106
        ImVec4(161/255.0f, 153/255.0f, 131/255.0f, 1.0f), // 107
        ImVec4(56/255.0f, 86/255.0f, 148/255.0f, 1.0f), // 108
        ImVec4(82/255.0f, 86/255.0f, 97/255.0f, 1.0f), // 109
        ImVec4(127/255.0f, 105/255.0f, 86/255.0f, 1.0f), // 110
        ImVec4(140/255.0f, 146/255.0f, 154/255.0f, 1.0f), // 111
        ImVec4(89/255.0f, 110/255.0f, 135/255.0f, 1.0f), // 112
        ImVec4(71/255.0f, 53/255.0f, 50/255.0f, 1.0f), // 113
        ImVec4(68/255.0f, 98/255.0f, 79/255.0f, 1.0f), // 114
        ImVec4(115/255.0f, 10/255.0f, 39/255.0f, 1.0f), // 115
        ImVec4(34/255.0f, 52/255.0f, 87/255.0f, 1.0f), // 116
        ImVec4(100/255.0f, 13/255.0f, 27/255.0f, 1.0f), // 117
        ImVec4(163/255.0f, 173/255.0f, 198/255.0f, 1.0f), // 118
        ImVec4(105/255.0f, 88/255.0f, 83/255.0f, 1.0f), // 119
        ImVec4(155/255.0f, 139/255.0f, 128/255.0f, 1.0f), // 120
        ImVec4(98/255.0f, 11/255.0f, 28/255.0f, 1.0f), // 121
        ImVec4(91/255.0f, 93/255.0f, 94/255.0f, 1.0f), // 122
        ImVec4(98/255.0f, 68/255.0f, 40/255.0f, 1.0f), // 123
        ImVec4(115/255.0f, 24/255.0f, 39/255.0f, 1.0f), // 124
        ImVec4(27/255.0f, 55/255.0f, 109/255.0f, 1.0f), // 125
        ImVec4(236/255.0f, 106/255.0f, 174/255.0f, 1.0f), // 126
        ImVec4(0.0f, 0.0f, 0.0f, 1.0f) // 127
    };

    static ImVec4 GetGtaColor(int colorId) {
        if (colorId >= 0 && colorId < 128) {
            return g_gtaColors[colorId];
        }
        return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default grey
    }
}

void GarageApp::DrawDetailsScreen() {
    auto& v = m_vehicles[m_selectedIdx];

    ImGui::Spacing();

    // Details content container (allows scrolling if needed)
    ImGui::BeginChild("##details_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

    // Vehicle Name
    ImGui::Text(ICON_FA_CAR " %s", v.info.name.c_str());
    
    std::string modelName = TR("garage.model_id") + std::to_string(v.info.model);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", modelName.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Specifications Card
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.14f, 0.14f, 0.16f, 0.6f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    
    ImGui::BeginChild("##specs_card", ImVec2(-1, 185.0f), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::Spacing();
    
    auto DrawPropertyRow = [](const char* label, const char* value, const ImVec4& valColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", label);
        ImGui::SameLine();
        float valueWidth = ImGui::CalcTextSize(value).x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - valueWidth - 15.0f);
        ImGui::TextColored(valColor, "%s", value);
        ImGui::Separator();
    };

    auto DrawColorRow = [](const char* label, int colorId) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", label);
        ImGui::SameLine();
        
        std::string valueStr = std::to_string(colorId);
        float valueWidth = ImGui::CalcTextSize(valueStr.c_str()).x;
        
        float squareSize = 14.0f;
        float spacing = 6.0f;
        
        float totalWidth = valueWidth + squareSize + spacing;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalWidth - 15.0f);
        
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float yOffset = (ImGui::GetTextLineHeight() - squareSize) * 0.5f;
        pos.y += yOffset;
        
        ImVec4 colVec = GetGtaColor(colorId);
        ImU32 colU32 = ImGui::GetColorU32(colVec);
        
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, 
            ImVec2(pos.x + squareSize, pos.y + squareSize), 
            colU32, 
            3.0f
        );
        ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.8f));
        ImGui::GetWindowDrawList()->AddRect(
            pos, 
            ImVec2(pos.x + squareSize, pos.y + squareSize), 
            borderCol, 
            3.0f, 
            0, 
            1.0f
        );
        
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - valueWidth - 15.0f);
        ImGui::Text("%s", valueStr.c_str());
        ImGui::Separator();
    };

    DrawColorRow(TR("garage.primary_color"), v.info.color1);
    DrawColorRow(TR("garage.secondary_color"), v.info.color2);
    
    std::string pjText = v.info.paintjob != -1 ? std::to_string(v.info.paintjob) : TR("garage.none");
    DrawPropertyRow(TR("garage.paintjob"), pjText.c_str());
    
    DrawPropertyRow(TR("garage.suspension"), v.info.hydraulics ? TR("garage.suspension_hydraulic") : TR("garage.suspension_standard"), v.info.hydraulics ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    std::string nitroStr = TR("garage.none");
    for (int mod : v.info.mods) {
        if (mod == 1008) nitroStr = "2x";
        else if (mod == 1009) nitroStr = "5x";
        else if (mod == 1010) nitroStr = "10x";
    }
    DrawPropertyRow(TR("garage.nitro"), nitroStr.c_str(), nitroStr != TR("garage.none") ? ImVec4(0.2f, 0.6f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    int totalMods = (int)v.info.mods.size();
    char modsBuf[32];
    sprintf_s(modsBuf, TR("garage.mods_items"), totalMods);
    DrawPropertyRow(TR("garage.mods"), modsBuf);

    DrawPropertyRow(TR("garage.plate"), v.info.plate.empty() ? TR("garage.plate_default") : v.info.plate.c_str());
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Actions
    unsigned int handle = m_provider->GetSpawnedVehicleHandle(v.deliveryId);
    bool isSameCar = m_provider->IsPlayerInVehicleModel(v.info.model);
    bool isNearSpawned = (handle != 0) && m_provider->IsVehicleNearPlayer(handle);
    bool isInside = m_provider->IsInside();
    bool onMission = m_provider->IsOnMission();
    bool isDeliveryBlocked = isInside || onMission;

    // 1. Chamar Button
    if (isDeliveryBlocked) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        
        std::string btnText = std::string(ICON_FA_PHONE " ") + TR("garage.call");
        if (isInside) btnText += TR("garage.blocked_interior");
        else if (onMission) btnText += TR("garage.blocked_mission");
        
        ImGui::Button(btnText.c_str(), ImVec2(-1, 36));
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    } else if (isSameCar || isNearSpawned) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        
        std::string btnText = TR("garage.call");
        if (isSameCar) btnText += TR("garage.already_inside");
        else if (isNearSpawned) btnText += TR("garage.already_near");
        
        if (ImGui::Button(btnText.c_str(), ImVec2(-1, 36))) {
            // No message, disabled action
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.039f, 0.518f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.58f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.02f, 0.44f, 0.88f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        
        if (ImGui::Button((std::string(ICON_FA_PHONE " ") + TR("garage.call")).c_str(), ImVec2(-1, 36))) {
            v.deliveryId = m_provider->TriggerDelivery(v.info);
            phone.closeApp();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }

    // 2. Atualizar Tuning Button
    if (isSameCar) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.35f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.75f, 0.40f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        
        if (ImGui::Button((std::string(ICON_FA_SYNC_ALT " ") + TR("garage.update_tuning")).c_str(), ImVec2(-1, 34))) {
            VehicleInfo currentCar;
            if (m_provider->GetCurrentVehicleInfo(currentCar)) {
                v.info.color1 = currentCar.color1;
                v.info.color2 = currentCar.color2;
                v.info.paintjob = currentCar.paintjob;
                v.info.mods = currentCar.mods;
                v.info.hydraulics = currentCar.hydraulics;
            }
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }

    // 3. Renomear Section
    if (m_renamingIdx == m_selectedIdx) {
        ImGui::Spacing();
        ImGui::Text(TR("garage.rename_title"));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::InputText("##rename_val", m_renameBuf, sizeof(m_renameBuf));
        ImGui::PopStyleVar();
        ImGui::Spacing();
        
        float btnW = (ImGui::GetContentRegionAvail().x - 10.0f) / 2.0f;
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.35f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        if (ImGui::Button(TR("garage.ok"), ImVec2(btnW, 30))) {
            if (m_renameBuf[0] != '\0') {
                v.info.name = m_renameBuf;
            }
            m_renamingIdx = -1;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        ImGui::SameLine(0, 10.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.40f, 0.42f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        if (ImGui::Button(TR("garage.cancel"), ImVec2(btnW, 30))) {
            m_renamingIdx = -1;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    } else if (m_editingPlateIdx == m_selectedIdx) {
        ImGui::Spacing();
        ImGui::Text(TR("garage.plate_title"));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::InputText("##plate_val", m_plateBuf, sizeof(m_plateBuf));
        ImGui::PopStyleVar();
        ImGui::Spacing();
        
        float btnW = (ImGui::GetContentRegionAvail().x - 10.0f) / 2.0f;
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.35f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        if (ImGui::Button((std::string(TR("garage.ok")) + "##plate").c_str(), ImVec2(btnW, 30))) {
            v.info.plate = m_plateBuf;
            m_editingPlateIdx = -1;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        ImGui::SameLine(0, 10.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.40f, 0.42f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        if (ImGui::Button((std::string(TR("garage.cancel")) + "##plate").c_str(), ImVec2(btnW, 30))) {
            m_editingPlateIdx = -1;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    } else {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.28f, 0.30f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.38f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        
        if (ImGui::Button((std::string(ICON_FA_EDIT " ") + TR("garage.rename_btn")).c_str(), ImVec2(-1, 34))) {
            m_renamingIdx = m_selectedIdx;
            m_editingPlateIdx = -1;
            strncpy_s(m_renameBuf, sizeof(m_renameBuf), v.info.name.c_str(), _TRUNCATE);
        }
        
        ImGui::Spacing();
        if (ImGui::Button((std::string(ICON_FA_EDIT " ") + TR("garage.plate_btn")).c_str(), ImVec2(-1, 34))) {
            m_editingPlateIdx = m_selectedIdx;
            m_renamingIdx = -1;
            strncpy_s(m_plateBuf, sizeof(m_plateBuf), v.info.plate.c_str(), _TRUNCATE);
        }
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }

    // 4. Excluir Button
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.22f, 0.22f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.28f, 0.28f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.16f, 0.16f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    
    if (ImGui::Button((std::string(ICON_FA_TRASH_ALT " ") + TR("garage.delete")).c_str(), ImVec2(-1, 34))) {
        m_vehicles.erase(m_vehicles.begin() + m_selectedIdx);
        m_selectedIdx = -1;
        m_renamingIdx = -1;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::EndChild();
}

void GarageApp::DrawListScreen() {
    // Save Current Vehicle Section
    if (m_provider->IsPlayerInAnyCar()) {
        VehicleInfo currentCar;
        if (m_provider->GetCurrentVehicleInfo(currentCar)) {
            bool isInside = m_provider->IsInside();
            bool onMission = m_provider->IsOnMission();
            bool isSaveBlocked = isInside || onMission;

            if (isSaveBlocked) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

                std::string btnText = std::string(ICON_FA_PLUS_CIRCLE " ") + TR("garage.save_current");
                if (isInside) btnText += TR("garage.blocked_interior");
                else if (onMission) btnText += TR("garage.blocked_mission");

                ImGui::Button(btnText.c_str(), ImVec2(-1, 35));

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.35f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.75f, 0.40f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

                if (ImGui::Button((std::string(ICON_FA_PLUS_CIRCLE " ") + TR("garage.save_current")).c_str(), ImVec2(-1, 35))) {
                    if (m_vehicles.size() >= 12) {
                        // Limit reached
                    } else {
                        SavedVehicle sv;
                        sv.info = currentCar;
                        m_vehicles.push_back(sv);
                    }
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
    }

    // Vehicle List Header
    ImGui::Text(TR("garage.my_vehicles"), (int)m_vehicles.size());
    ImGui::Spacing();

    if (m_vehicles.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::TextWrapped(TR("garage.empty"));
        ImGui::PopStyleColor();
        return;
    }

    // List Container
    ImGui::BeginChild("##garage_list", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
    
    for (size_t i = 0; i < m_vehicles.size(); i++) {
        auto& v = m_vehicles[i];

        ImGui::PushID((int)i);

        // iOS-style list items: rounded dark gray cards
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.16f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.22f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.10f, 0.12f, 0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

        // Button width and height
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 42.0f);
        
        if (ImGui::Button(std::string("##btn_" + std::to_string(i)).c_str(), size)) {
            m_selectedIdx = (int)i;
            m_renamingIdx = -1;
        }

        // Draw car icon and name on the left, chevron on the right
        ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 12.0f, pos.y + 12.0f), ImGui::GetColorU32(ImGuiCol_Text), (std::string(ICON_FA_CAR "  ") + v.info.name).c_str());
        
        float chevronWidth = ImGui::CalcTextSize(ICON_FA_CHEVRON_RIGHT).x;
        ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + size.x - 12.0f - chevronWidth, pos.y + 12.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), ICON_FA_CHEVRON_RIGHT);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void GarageApp::onDraw() {
    if (!m_provider) {
        ImGui::Text(TR("garage.no_provider"));
        return;
    }

    if (m_selectedIdx != -1 && m_selectedIdx < (int)m_vehicles.size()) {
        DrawDetailsScreen();
    } else {
        DrawListScreen();
    }
}
