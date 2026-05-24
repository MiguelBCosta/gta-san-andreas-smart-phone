#pragma once
#include "../PhoneApp.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>

// Placeholder "Notas" app — equivalent to notes.lua
class NotesApp : public PhoneApp {
public:
    char buffer[1024] = "";

    NotesApp() {
        id        = "notes";
        icon      = ICON_FA_STICKY_NOTE;
        name      = "Notas";
        color     = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        dock      = false;
        dockOrder = 99;
    }

    void onDraw() override {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.13f, 0.9f));
        ImGui::InputTextMultiline(
            "##notes_buf",
            buffer, sizeof(buffer),
            ImVec2(ImGui::GetContentRegionAvail().x, 380.0f)
        );
        ImGui::PopStyleColor();
    }
};
