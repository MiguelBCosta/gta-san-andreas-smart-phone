#include "NotesApp.h"
#include "../LocalizationManager.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>
#include <cstring>

NotesApp::NotesApp() {
    id        = "notes";
    icon      = ICON_FA_STICKY_NOTE;
    name = TR("notes.title");
    color     = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
    dock      = false;
    dockOrder = 99;
}

void NotesApp::onDraw() {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.13f, 0.9f));
    ImGui::InputTextMultiline(
        "##notes_buf",
        m_buffer, sizeof(m_buffer),
        ImVec2(ImGui::GetContentRegionAvail().x, 380.0f)
    );
    ImGui::PopStyleColor();
}

void NotesApp::onSave(nlohmann::json& out) {
    out["buffer"] = std::string(m_buffer);
}

void NotesApp::onLoad(const nlohmann::json& in) {
    if (in.contains("buffer") && in["buffer"].is_string()) {
        std::string text = in["buffer"].get<std::string>();
        strncpy_s(m_buffer, sizeof(m_buffer), text.c_str(), _TRUNCATE);
    }
}

void NotesApp::onWipe() {
    std::memset(m_buffer, 0, sizeof(m_buffer));
}
