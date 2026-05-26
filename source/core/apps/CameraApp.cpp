#include "CameraApp.h"
#include "../Phone.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>
#include <cmath>

CameraApp::CameraApp() {
    id        = "camera";
    icon      = ICON_FA_CAMERA;
    name      = "Camera";
    color     = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
    dock      = true;
    dockOrder = 3;
}

bool CameraApp::requiresMouse() const {
    if (m_provider) {
        return m_provider->RequiresMouse();
    }
    return false; // In GTA, we let the mouse rotate the game camera
}

bool CameraApp::hidePhoneChassis() const {
    if (m_provider) {
        return !m_provider->IsSandbox(); // Hide phone body in GTA mode
    }
    return false;
}

void CameraApp::onOpen() {
    if (m_provider) {
        m_provider->SetActive(true);
        m_provider->SetSelfieMode(false);
    }
    m_flashAlpha = 0.0f;
    m_hudVisible = true;
}

void CameraApp::onClose() {
    if (m_provider) {
        m_provider->SetActive(false);
    }
}

extern Phone phone;

void CameraApp::update(float dt) {
    if (!m_provider) return;

    m_provider->Update(dt);

    // Fade out screen flash overlay
    if (m_flashAlpha > 0.0f) {
        m_flashAlpha -= dt * 4.0f; // fade out in 0.25s
        if (m_flashAlpha < 0.0f) m_flashAlpha = 0.0f;
    }

    // Guard ImGui calls: context check and active check
    if (ImGui::GetCurrentContext() == nullptr) return;
    if (!phone.isVisible() || phone.getCurrentApp() != this) return;

    // Input Handling
    // 1. Exit camera: Backspace or Escape
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        phone.closeApp();
        return;
    }

    // 2. Toggle Selfie: Space or Tab
    if (ImGui::IsKeyPressed(ImGuiKey_Space) || ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        m_provider->SetSelfieMode(!m_provider->IsSelfieMode());
    }

    // 3. Take Photo: Enter key or Left Click
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) || ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_provider->TakePhoto();
        m_flashAlpha = 1.0f;
    }

    // 4. Toggle Grid: G key
    if (ImGui::IsKeyPressed(ImGuiKey_G)) {
        m_hudVisible = !m_hudVisible;
    }

    // 5. Adjust Zoom (Mouse Wheel or Up/Down Arrows)
    float wheel = ImGui::GetIO().MouseWheel;
    if (std::abs(wheel) > 0.01f) {
        m_provider->AdjustZoom(wheel);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_provider->AdjustZoom(1.0f);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_provider->AdjustZoom(-1.0f);
    }
}
void CameraApp::onDraw() {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    if (m_provider && m_provider->IsSandbox()) {
        // ---- SANDBOX SIMULATOR LAYOUT ----
        // Set layout inside the phone chassis screen bounds
        ImGui::SetCursorPosY(5.0f);
        ImGui::Text("Camera Simulator");

        // Mock Viewfinder box
        ImVec2 contentAvail = ImGui::GetContentRegionAvail();
        float viewH = 260.0f;
        ImVec2 viewPosMin = ImGui::GetCursorScreenPos();
        ImVec2 viewPosMax = ImVec2(viewPosMin.x + contentAvail.x, viewPosMin.y + viewH);

        // Draw gray viewfinder background
        draw->AddRectFilled(viewPosMin, viewPosMax, IM_COL32(30, 30, 35, 255), 8.0f);
        
        bool isCapturing = m_provider->IsCapturing();

        // Draw grid inside simulator viewfinder
        if (m_hudVisible && !isCapturing) {
            float stepX = contentAvail.x / 3.0f;
            float stepY = viewH / 3.0f;
            ImU32 gridCol = IM_COL32(255, 255, 255, 40);
            
            // Verticals
            draw->AddLine(ImVec2(viewPosMin.x + stepX, viewPosMin.y), ImVec2(viewPosMin.x + stepX, viewPosMax.y), gridCol);
            draw->AddLine(ImVec2(viewPosMin.x + stepX * 2.0f, viewPosMin.y), ImVec2(viewPosMin.x + stepX * 2.0f, viewPosMax.y), gridCol);
            // Horizontals
            draw->AddLine(ImVec2(viewPosMin.x, viewPosMin.y + stepY), ImVec2(viewPosMax.x, viewPosMin.y + stepY), gridCol);
            draw->AddLine(ImVec2(viewPosMin.x, viewPosMin.y + stepY * 2.0f), ImVec2(viewPosMax.x, viewPosMin.y + stepY * 2.0f), gridCol);

            // Centered bracket
            float cx = viewPosMin.x + contentAvail.x / 2.0f;
            float cy = viewPosMin.y + viewH / 2.0f;
            float sz = 15.0f;
            draw->AddRect(ImVec2(cx - sz, cy - sz), ImVec2(cx + sz, cy + sz), IM_COL32(255, 255, 255, 80), 0.0f, 15, 1.5f);
        }

        if (!isCapturing) {
            // Preview status text
            const char* modeText = m_provider->IsSelfieMode() ? "MODO: SELFIE (CJ)" : "MODO: PRIMEIRA PESSOA";
            ImVec2 textSz = ImGui::CalcTextSize(modeText);
            ImGui::SetCursorScreenPos(ImVec2(viewPosMin.x + (contentAvail.x - textSz.x) / 2.0f, viewPosMin.y + viewH / 2.0f - 5.0f));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "%s", modeText);

            ImGui::SetCursorScreenPos(ImVec2(viewPosMin.x, viewPosMax.y + 15.0f));

            // Control Buttons
            ImGui::Columns(2, "##cam_buttons", false);
            ImGui::SetColumnWidth(0, 140.0f);
            ImGui::SetColumnWidth(1, 140.0f);

            // Big Capture button
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 30.0f);
            if (ImGui::Button(ICON_FA_CAMERA " Capturar", ImVec2(120.0f, 50.0f))) {
                m_provider->TakePhoto();
                m_flashAlpha = 1.0f;
            }
            ImGui::PopStyleVar();

            ImGui::NextColumn();

            // Mode switch button
            if (ImGui::Button(ICON_FA_SYNC " Alternar Câmera", ImVec2(120.0f, 50.0f))) {
                m_provider->SetSelfieMode(!m_provider->IsSelfieMode());
            }

            ImGui::Columns(1);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Instructions in Sandbox
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Teclas de Atalho:");
            ImGui::BulletText("LMB ou Enter: Capturar");
            ImGui::BulletText("Espaço ou Tab: Selfie");
            ImGui::BulletText("G: Alternar Grid");
        }

        // Flash Overlay for Simulator
        if (m_flashAlpha > 0.0f) {
            draw->AddRectFilled(viewPosMin, viewPosMax, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, m_flashAlpha)), 8.0f);
        }
    }
    else {
        // ---- GTA SA FULL SCREEN HUD LAYOUT ----
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;

        bool isCapturing = m_provider && m_provider->IsCapturing();

        if (!isCapturing) {
            // Draw 3x3 grid lines if enabled
            if (m_hudVisible) {
                float stepX = displaySize.x / 3.0f;
                float stepY = displaySize.y / 3.0f;
                ImU32 gridCol = IM_COL32(255, 255, 255, 30);
                
                // Vertical Grid lines
                draw->AddLine(ImVec2(stepX, 0.0f), ImVec2(stepX, displaySize.y), gridCol, 1.0f);
                draw->AddLine(ImVec2(stepX * 2.0f, 0.0f), ImVec2(stepX * 2.0f, displaySize.y), gridCol, 1.0f);
                
                // Horizontal Grid lines
                draw->AddLine(ImVec2(0.0f, stepY), ImVec2(displaySize.x, stepY), gridCol, 1.0f);
                draw->AddLine(ImVec2(0.0f, stepY * 2.0f), ImVec2(displaySize.x, stepY * 2.0f), gridCol, 1.0f);

                // Centered Target Focus Bracket (GTA V Snapmatic design)
                float cx = displaySize.x / 2.0f;
                float cy = displaySize.y / 2.0f;
                float bSz = 40.0f; // bracket size
                float bThick = 2.0f; // thickness
                ImU32 bracketCol = IM_COL32(255, 255, 255, 120);

                // Top-left corner
                draw->AddLine(ImVec2(cx - bSz, cy - bSz), ImVec2(cx - bSz + 15.0f, cy - bSz), bracketCol, bThick);
                draw->AddLine(ImVec2(cx - bSz, cy - bSz), ImVec2(cx - bSz, cy - bSz + 15.0f), bracketCol, bThick);

                // Top-right corner
                draw->AddLine(ImVec2(cx + bSz, cy - bSz), ImVec2(cx + bSz - 15.0f, cy - bSz), bracketCol, bThick);
                draw->AddLine(ImVec2(cx + bSz, cy - bSz), ImVec2(cx + bSz, cy - bSz + 15.0f), bracketCol, bThick);

                // Bottom-left corner
                draw->AddLine(ImVec2(cx - bSz, cy + bSz), ImVec2(cx - bSz + 15.0f, cy + bSz), bracketCol, bThick);
                draw->AddLine(ImVec2(cx - bSz, cy + bSz), ImVec2(cx - bSz, cy + bSz - 15.0f), bracketCol, bThick);

                // Bottom-right corner
                draw->AddLine(ImVec2(cx + bSz, cy + bSz), ImVec2(cx + bSz - 15.0f, cy + bSz), bracketCol, bThick);
                draw->AddLine(ImVec2(cx + bSz, cy + bSz), ImVec2(cx + bSz, cy + bSz - 15.0f), bracketCol, bThick);
            }

            // Top bar indicators
            float barH = 50.0f;
            draw->AddRectFilled(ImVec2(0, 0), ImVec2(displaySize.x, barH), IM_COL32(0, 0, 0, 150));

            // Snapmatic Title
            ImGui::SetCursorPos(ImVec2(20.0f, 12.0f));
            ImGui::SetWindowFontScale(1.3f);
            ImGui::TextColored(ImVec4(0.97f, 0.78f, 0.05f, 1.0f), ICON_FA_CAMERA " SNAPMATIC");
            ImGui::SetWindowFontScale(1.0f);

            // Right side indicators (Battery and Camera Mode)
            std::string modeLabel = m_provider->IsSelfieMode() ? "MODO: SELFIE" : "MODO: PADRAO";
            ImVec2 modeTextSz = ImGui::CalcTextSize(modeLabel.c_str());
            
            ImGui::SetCursorPos(ImVec2(displaySize.x - modeTextSz.x - 30.0f, 15.0f));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "%s", modeLabel.c_str());

            // Bottom help legend bar (GTA V style)
            float bottomBarH = 45.0f;
            ImVec2 bBarMin(0.0f, displaySize.y - bottomBarH);
            ImVec2 bBarMax(displaySize.x, displaySize.y);
            draw->AddRectFilled(bBarMin, bBarMax, IM_COL32(0, 0, 0, 180));

            // Legend content
            ImGui::SetCursorPos(ImVec2(20.0f, displaySize.y - bottomBarH + 12.0f));
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 0.95f), 
                "[LMB / Enter] Capturar   |   [Espaco / Tab] Selfie/Normal   |   [Mouse Wheel] Zoom   |   [G] Grid   |   [Backspace] Sair"
            );
        }

        // Draw Full-screen white flash overlay
        if (m_flashAlpha > 0.0f) {
            draw->AddRectFilled(ImVec2(0, 0), displaySize, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, m_flashAlpha)));
        }
    }
}
