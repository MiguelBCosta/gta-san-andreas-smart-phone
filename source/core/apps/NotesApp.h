#pragma once
#include "../PhoneApp.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>

class NotesApp : public PhoneApp {
private:
    char m_buffer[1024] = "";

public:
    NotesApp();

    void onDraw() override;
    void onSave(nlohmann::json& out) override;
    void onLoad(const nlohmann::json& in) override;
    void onWipe() override;
    void onLanguageChange() override { name = TR("notes.title"); }
};
