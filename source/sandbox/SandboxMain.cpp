#include <windows.h>
#include <d3d9.h>
#include <tchar.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <IconsFontAwesome5.h>

#include "../core/Phone.h"
#include "providers/SandboxClockProvider.h"
#include "providers/SandboxWeatherProvider.h"
#include "../core/apps/NotesApp.h"
#include "../core/apps/WeatherApp.h"
#include "../core/apps/CalculatorApp.h"
#include "../core/apps/PlaceholderApps.h"
#include "../core/apps/ClockApp.h"
#include "../core/apps/PhoneCallApp.h"
#include "providers/SandboxScreenProvider.h"
#include "providers/SandboxStorageProvider.h"
#include "providers/SandboxGarageProvider.h"
#include "providers/SandboxPhoneCallProvider.h"
#include "providers/SandboxAvatarProvider.h"
#include "../core/apps/GarageApp.h"
#include "../core/resources/resource.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// D3D9 globals
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync

    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_d3dpp.BackBufferWidth = LOWORD(lParam);
        g_d3dpp.BackBufferHeight = HIWORD(lParam);
        if (g_pd3dDevice != nullptr) {
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

// ---- App Instances ----
Phone phone;
SandboxClockProvider sandboxClock;
SandboxScreenProvider sandboxScreen;
static SandboxStorageProvider sandboxStorage;
static SandboxWeatherProvider sandboxWeather;
static SandboxGarageProvider sandboxGarage;
static SandboxPhoneCallProvider sandboxCallProvider;
static CalculatorApp calcApp;
static CameraApp     cameraApp;
ClockApp      clockApp;
static GarageApp     garageApp;
static MapsApp       mapsApp;
static MessagesApp   messagesApp;
static MusicApp      musicApp;
static NotesApp      notesApp;
static PhoneCallApp  phoneCallApp;
static SettingsApp   settingsApp;
static WeatherApp    weatherApp;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("MoonPhone Sandbox"), nullptr };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("MoonPhone UI Sandbox"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Load primary font (Roboto) from memory (Win32 embedded resource)
    HMODULE hModule = GetModuleHandle(NULL);

    HRSRC hResRoboto = FindResourceA(hModule, MAKEINTRESOURCEA(IDR_FONT_ROBOTO), (LPCSTR)RT_RCDATA);
    HGLOBAL hDataRoboto = LoadResource(hModule, hResRoboto);
    void* pRobotoData = LockResource(hDataRoboto);
    DWORD cbRobotoData = SizeofResource(hModule, hResRoboto);

    ImFontConfig robotoConfig;
    robotoConfig.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(pRobotoData, cbRobotoData, 15.0f, &robotoConfig);

    // Merge FontAwesome from memory (Win32 embedded resource)
    HRSRC hResFA = FindResourceA(hModule, MAKEINTRESOURCEA(IDR_FONT_AWESOME), (LPCSTR)RT_RCDATA);
    HGLOBAL hDataFA = LoadResource(hModule, hResFA);
    void* pFAData = LockResource(hDataFA);
    DWORD cbFAData = SizeofResource(hModule, hResFA);

    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.FontDataOwnedByAtlas = false;
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    io.Fonts->AddFontFromMemoryTTF(pFAData, cbFAData, 15.0f, &config, icon_ranges);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Setup Phone
    phone.setClockProvider(&sandboxClock);
    phone.setScreenProvider(&sandboxScreen);
    phone.getStorage().setStorageProvider(&sandboxStorage);
    phone.setCallProvider(&sandboxCallProvider);

    static SandboxAvatarProvider sandboxAvatarProvider(g_pd3dDevice);
    phone.setAvatarProvider(&sandboxAvatarProvider);

    weatherApp.SetWeatherProvider(&sandboxWeather);
    garageApp.SetGarageProvider(&sandboxGarage);
    phone.registerApp(&calcApp);
    phone.registerApp(&cameraApp);
    phone.registerApp(&clockApp);
    phone.registerApp(&garageApp);
    phone.registerApp(&mapsApp);
    phone.registerApp(&messagesApp);
    phone.registerApp(&musicApp);
    phone.registerApp(&notesApp);
    phone.registerApp(&phoneCallApp);
    phone.registerApp(&settingsApp);
    phone.registerApp(&weatherApp);
    phone.open(PhoneAnimMode::FORCED); // Always visible in sandbox

    // Main loop
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        // Handle lost D3D9 device
        HRESULT result = g_pd3dDevice->TestCooperativeLevel();
        if (result == D3DERR_DEVICELOST) {
            ::Sleep(10);
            continue;
        }
        if (result == D3DERR_DEVICENOTRESET)
            ResetDevice();

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Background color
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(40, 40, 40, 255);
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

        // Handle F1 toggle and animation
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            phone.toggle(PhoneAnimMode::SMOOTH);
        }
        float dt = ImGui::GetIO().DeltaTime;
        phone.update(dt);
        phone.process(dt);

        // Draw phone if visible
        if (phone.isVisible()) {
            phone.draw();
        }

        // Draw simulated fade overlay in Sandbox
        if (phone.getScreenProvider() && phone.getScreenProvider()->IsFading()) {
            float fadeAlpha = phone.getScreenProvider()->GetFadeAlpha();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::Begin("##SandboxFade", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoScrollbar | 
                ImGuiWindowFlags_NoSavedSettings | 
                ImGuiWindowFlags_NoInputs | 
                ImGuiWindowFlags_NoBackground);
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(0, 0), ImGui::GetIO().DisplaySize, 
                IM_COL32(0, 0, 0, (int)(fadeAlpha * 255)));
            ImGui::End();
        }

        // Control panel for weather simulation in sandbox
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 220), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controles do Clima (Sandbox)", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Simulador de Clima para o Celular");
        ImGui::Separator();
        ImGui::Spacing();

        // 1. Clima atual combo
        int currentId = sandboxWeather.GetCurrentWeatherId();
        const char* climates[] = {
            "0: Muito ensolarado (LS)",
            "1: Ensolarado (LS)",
            "2: Ensolarado c/ nevoa (LS)",
            "3: Ensolarado c/ fumaca (LS)",
            "4: Nublado (LS)",
            "5: Ensolarado (SF)",
            "6: Muito ensolarado (SF)",
            "7: Nublado (SF)",
            "8: Chuvoso (SF)",
            "9: Neblina (SF)",
            "10: Ensolarado (LV)",
            "11: Muito ensolarado (LV)",
            "12: Nublado (LV)",
            "13: Muito ensolarado (Interior)",
            "14: Ensolarado (Interior)",
            "15: Nublado (Interior)",
            "16: Chuvoso (Interior)",
            "17: Muito ensolarado (Desert)",
            "18: Ensolarado (Desert)",
            "19: Tempestade de areia (Desert)",
            "20: Subaquatico (Especial)",
            "21: Cores Extra 1 (Especial)",
            "22: Cores Extra 2 (Especial)"
        };

        if (ImGui::Combo("Clima Atual", &currentId, climates, IM_ARRAYSIZE(climates))) {
            sandboxWeather.SetCurrentWeather(currentId);
        }

        // 2. Proximo clima combo
        int nextId = sandboxWeather.GetNextWeatherId();
        if (ImGui::Combo("Proximo Clima", &nextId, climates, IM_ARRAYSIZE(climates))) {
            sandboxWeather.SetNextWeather(nextId);
        }

        // 3. Intensidade da chuva slider
        float rain = sandboxWeather.GetRainIntensity();
        if (ImGui::SliderFloat("Intensidade Chuva", &rain, 0.0f, 1.0f, "%.2f")) {
            sandboxWeather.SetRainIntensity(rain);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Tecla F1: Abre/Fecha o celular");
        ImGui::End();

        // Control panel for phone storage simulation in sandbox
        ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 160), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controles de Persistencia (Sandbox)", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Simulador de Save/Load/Wipe do Celular");
        ImGui::Separator();
        ImGui::Spacing();

        static int selectedSlot = 1;
        ImGui::SliderInt("Slot do Save", &selectedSlot, 1, 8);

        if (ImGui::Button("Salvar Slot", ImVec2(100, 30))) {
            phone.getStorage().onGameSave(selectedSlot);
        }
        ImGui::SameLine();
        if (ImGui::Button("Carregar Slot", ImVec2(100, 30))) {
            phone.getStorage().onGameLoad(selectedSlot);
        }
        ImGui::SameLine();
        if (ImGui::Button("Wipe", ImVec2(100, 30))) {
            phone.getStorage().onNewGame();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Controle do Relogio");
        bool canSkip = sandboxClock.CanSkipTime();
        if (ImGui::Checkbox("Permitir Descansar", &canSkip)) {
            sandboxClock.SetCanSkipTime(canSkip);
        }

        ImGui::End();

        // Control panel for phone call simulation
        ImGui::SetNextWindowPos(ImVec2(800, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 160), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controles de Ligacoes (Sandbox)", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Simular Recebimento de Chamadas");
        ImGui::Separator();
        ImGui::Spacing();

        static int selectedCallerIdx = 0;
        const char* callers[] = { 
            "sweet", "ryder", "cesar", "catalina", "woozie", "truth", "toreno", "crash", "zero", 
            "denise", "michelle", "helena", "katie", "barbara", "millie", 
            "kendl", "smoke", "ogloc", "jethro", "kentpaul", "rosenberg", "salvatore",
            "girlfriend", "unknown" 
        };
        ImGui::Combo("Remetente", &selectedCallerIdx, callers, IM_ARRAYSIZE(callers));

        if (ImGui::Button("Simular Chamada", ImVec2(160, 30))) {
            sandboxCallProvider.triggerCall(callers[selectedCallerIdx]);
        }
        ImGui::SameLine();
        if (ImGui::Button("Encerrar Chamada", ImVec2(160, 30))) {
            sandboxCallProvider.stopCall();
        }

        ImGui::End();

        // Control panel for garage simulation in sandbox
        ImGui::SetNextWindowPos(ImVec2(430, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 250), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controles da Garagem (Sandbox)", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Simulador da Garagem");
        ImGui::Separator();
        ImGui::Spacing();

        // 1. Inside check
        bool inside = sandboxGarage.GetInside();
        if (ImGui::Checkbox("Jogador em Interior", &inside)) {
            sandboxGarage.SetInside(inside);
        }

        // 2. Mission check
        bool mission = sandboxGarage.GetOnMission();
        if (ImGui::Checkbox("Jogador em Missao", &mission)) {
            sandboxGarage.SetOnMission(mission);
        }

        // 3. In Car check
        bool inCar = sandboxGarage.GetPlayerInAnyCar();
        if (ImGui::Checkbox("Jogador em Veiculo", &inCar)) {
            sandboxGarage.SetPlayerInAnyCar(inCar);
        }

        if (inCar) {
            ImGui::Indent(15.0f);
            auto& mockV = sandboxGarage.GetMockCurrentVehicle();
            
            // Edit vehicle name
            char nameBuf[128];
            strncpy_s(nameBuf, sizeof(nameBuf), mockV.name.c_str(), _TRUNCATE);
            if (ImGui::InputText("Nome do Carro", nameBuf, sizeof(nameBuf))) {
                mockV.name = nameBuf;
            }

            // Edit vehicle model
            ImGui::InputInt("ID Modelo", &mockV.model);
            
            // Checkboxes
            ImGui::Checkbox("Hidraulica", &mockV.hydraulics);
            ImGui::Unindent(15.0f);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Entregas Ativas:");
        
        auto& deliveries = sandboxGarage.GetDeliveries();
        if (deliveries.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Nenhuma entrega ativa.");
        } else {
            for (size_t i = 0; i < deliveries.size(); i++) {
                auto& del = deliveries[i];
                ImGui::Text("ID: %d | Distancia: %.1fm %s", del.id, del.distance, del.delivered ? "(Entregue)" : "(A caminho)");
                if (del.delivered) {
                    ImGui::SameLine();
                    std::string btnId = "Entrar##del_enter_" + std::to_string(del.id);
                    if (ImGui::Button(btnId.c_str())) {
                        // Simulate entering the delivered car
                        sandboxGarage.SetPlayerInAnyCar(true);
                        sandboxGarage.GetMockCurrentVehicle().model = 411; // Infernus
                        // Clean up delivery
                        deliveries.erase(deliveries.begin() + i);
                        break;
                    }
                }
            }
        }

        const std::string& lastMsg = sandboxGarage.GetLastMessage();
        if (!lastMsg.empty()) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", lastMsg.c_str());
            if (ImGui::Button("Limpar Mensagem")) {
                sandboxGarage.ClearLastMessage();
            }
        }

        ImGui::End();

        ImGui::EndFrame();

        // Render
        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT hr = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (hr == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
