#pragma once
#include "PhoneApp.h"
#include <imgui.h>

// Placeholder "Notas" app — equivalent to notes.lua
class NotesApp : public PhoneApp {
public:
    char buffer[1024] = "";

    NotesApp() {
        id        = "notes";
        name      = "Notas";
        color     = ImVec4(0.92f, 0.82f, 0.18f, 1.0f);
        dock      = false;
        dockOrder = 99;
    }

    void onDraw() override {
        ImGui::Text("Bloco de Notas");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.13f, 0.9f));
        ImGui::InputTextMultiline(
            "##notes_buf",
            buffer, sizeof(buffer),
            ImVec2(ImGui::GetContentRegionAvail().x, 380.0f)
        );
        ImGui::PopStyleColor();
    }
};
