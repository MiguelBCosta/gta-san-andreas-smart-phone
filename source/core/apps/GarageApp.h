#pragma once
#include "../PhoneApp.h"
#include "../providers/IGarageProvider.h"
#include <IconsFontAwesome5.h>
#include <vector>
#include <string>
#include <cstring>

// Forward declare the global phone instance
class Phone;
extern Phone phone;

class GarageApp : public PhoneApp {
private:
    struct SavedVehicle {
        VehicleInfo info;
        int deliveryId = -1; // runtime only
    };

    IGarageProvider* m_provider = nullptr;
    std::vector<SavedVehicle> m_vehicles;
    int m_selectedIdx = -1;
    int m_renamingIdx = -1;
    char m_renameBuf[256] = "";

public:
    GarageApp() {
        id = "garage";
        icon = ICON_FA_CAR;
        name = "Garagem";
        // Accent blue/cyan color
        color = ImVec4(0.196f, 0.541f, 0.988f, 1.0f);
        dock = true;
        dockOrder = 3;
    }

    void SetGarageProvider(IGarageProvider* provider) {
        m_provider = provider;
    }

    void onOpen() override {
        m_selectedIdx = -1;
        m_renamingIdx = -1;
        m_renameBuf[0] = '\0';
    }

    void onClose() override {
        m_selectedIdx = -1;
        m_renamingIdx = -1;
    }

    void update(float dt) override {
        if (m_provider) {
            m_provider->Update(dt);
        }
    }

    void onSave(nlohmann::json& out) override {
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
            listJson.push_back(vJson);
        }
        out["vehicles"] = listJson;
    }

    void onLoad(const nlohmann::json& in) override {
        m_vehicles.clear();
        m_selectedIdx = -1;
        m_renamingIdx = -1;
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
                m_vehicles.push_back(v);
            }
        }
    }

    void onWipe() override {
        m_vehicles.clear();
        m_selectedIdx = -1;
        m_renamingIdx = -1;
    }

    void DrawDetailsScreen() {
        auto& v = m_vehicles[m_selectedIdx];

        // Back button (chevron left)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
        if (ImGui::Button(ICON_FA_CHEVRON_LEFT " Voltar", ImVec2(80, 30))) {
            m_selectedIdx = -1;
            m_renamingIdx = -1;
            ImGui::PopStyleColor(3);
            return;
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();

        // Details content container (allows scrolling if needed)
        ImGui::BeginChild("##details_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

        // Vehicle Name
        ImGui::Text(ICON_FA_CAR " %s", v.info.name.c_str());
        
        std::string modelName = "Modelo ID: " + std::to_string(v.info.model);
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

        DrawPropertyRow("Cor Primaria", std::to_string(v.info.color1).c_str());
        DrawPropertyRow("Cor Secundaria", std::to_string(v.info.color2).c_str());
        
        std::string pjText = v.info.paintjob != -1 ? std::to_string(v.info.paintjob) : "Nenhuma";
        DrawPropertyRow("Pintura Especial", pjText.c_str());
        
        DrawPropertyRow("Suspensao", v.info.hydraulics ? "Hidraulica" : "Padrao", v.info.hydraulics ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        std::string nitroStr = "Nenhum";
        for (int mod : v.info.mods) {
            if (mod == 1008) nitroStr = "2x";
            else if (mod == 1009) nitroStr = "5x";
            else if (mod == 1010) nitroStr = "10x";
        }
        DrawPropertyRow("Nitro", nitroStr.c_str(), nitroStr != "Nenhum" ? ImVec4(0.2f, 0.6f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        int totalMods = (int)v.info.mods.size();
        DrawPropertyRow("Modificacoes", (std::to_string(totalMods) + " itens").c_str());
        
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

        // 1. Chamar Button
        if (isSameCar || isNearSpawned) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            
            std::string btnText = "Chamar Veiculo";
            if (isSameCar) btnText += " (Ja no veiculo)";
            else if (isNearSpawned) btnText += " (Ja perto)";
            
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
            
            if (ImGui::Button(ICON_FA_PHONE " Chamar Veiculo", ImVec2(-1, 36))) {
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
            
            if (ImGui::Button(ICON_FA_SYNC_ALT " Atualizar Tuning", ImVec2(-1, 34))) {
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
            ImGui::Text("Renomear Veiculo:");
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::InputText("##rename_val", m_renameBuf, sizeof(m_renameBuf));
            ImGui::PopStyleVar();
            ImGui::Spacing();
            
            float btnW = (ImGui::GetContentRegionAvail().x - 10.0f) / 2.0f;
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.35f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            if (ImGui::Button("Ok", ImVec2(btnW, 30))) {
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
            if (ImGui::Button("Cancelar", ImVec2(btnW, 30))) {
                m_renamingIdx = -1;
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        } else {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.28f, 0.30f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.38f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            
            if (ImGui::Button(ICON_FA_EDIT " Renomear Veiculo", ImVec2(-1, 34))) {
                m_renamingIdx = m_selectedIdx;
                strncpy_s(m_renameBuf, sizeof(m_renameBuf), v.info.name.c_str(), _TRUNCATE);
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
        
        if (ImGui::Button(ICON_FA_TRASH_ALT " Excluir Veiculo", ImVec2(-1, 34))) {
            m_vehicles.erase(m_vehicles.begin() + m_selectedIdx);
            m_selectedIdx = -1;
            m_renamingIdx = -1;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::EndChild();
    }

    void DrawListScreen() {
        // Save Current Vehicle Section
        if (m_provider->IsPlayerInAnyCar()) {
            VehicleInfo currentCar;
            if (m_provider->GetCurrentVehicleInfo(currentCar)) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.65f, 0.35f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.75f, 0.40f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

                if (ImGui::Button(ICON_FA_PLUS_CIRCLE " Salvar Veiculo Atual", ImVec2(-1, 35))) {
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
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }
        }

        // Vehicle List Header
        ImGui::Text("Seus Veiculos (%d/12):", (int)m_vehicles.size());
        ImGui::Spacing();

        if (m_vehicles.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::TextWrapped("Nenhum veiculo salvo neste slot.");
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

    void onDraw() override {
        if (!m_provider) {
            ImGui::Text("Servico indisponivel.");
            return;
        }

        // Restrictions: Interior or Active Mission block deliveries
        bool isInside = m_provider->IsInside();
        bool onMission = m_provider->IsOnMission();
        if (isInside || onMission) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::TextWrapped("Servico indisponivel no momento. O Garagem nao faz entregas durante missoes ou dentro de interiores.");
            ImGui::PopStyleColor();
            return;
        }

        if (m_selectedIdx != -1 && m_selectedIdx < (int)m_vehicles.size()) {
            DrawDetailsScreen();
        } else {
            DrawListScreen();
        }
    }

};
