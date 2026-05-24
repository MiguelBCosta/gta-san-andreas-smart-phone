#include <plugin.h>
#include <game_sa/CPad.h>
#include <game_sa/CMenuManager.h>
#include <game_sa/CCutsceneMgr.h>
#include <game_sa/CTimer.h>
#include <game_sa/common.h>
#include <game_sa/CPlayerPed.h>
#include <d3d9.h>
#include <MinHook.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>
#include <IconsFontAwesome5.h>

#include "../core/Phone.h"
#include "providers/GtaClockProvider.h"
#include "providers/GtaWeatherProvider.h"
#include "../core/apps/NotesApp.h"
#include "../core/apps/CalculatorApp.h"
#include "../core/apps/WeatherApp.h"
#include "../core/apps/PlaceholderApps.h"
#include "../core/apps/ClockApp.h"
#include "providers/GtaScreenProvider.h"

using namespace plugin;

// ---- D3D9 Hook Types ----
typedef HRESULT(__stdcall* EndScene_t)(IDirect3DDevice9*);
typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
static EndScene_t oEndScene = nullptr;
static Reset_t    oReset    = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static WNDPROC oWndProc = nullptr;
static HWND    gameWindow = nullptr;
static bool    imguiInitialized = false;

// ---- Phone Instance ----
Phone phone;
static GtaClockProvider gtaClock;
static GtaWeatherProvider gtaWeather;
static GtaScreenProvider gtaScreen;

// ---- App Instances (static lifetime) ----
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

bool CanOpenPhone() {
    // 1. Menu de pausa ou mapa
    if (FrontEndMenuManager.m_bMenuActive) {
        return false;
    }

    // 2. Cutscene ativa
    if (CCutsceneMgr::ms_running) {
        return false;
    }

    // 3. Jogador controlando o personagem (deve existir e estar vivo)
    CPlayerPed* player = FindPlayerPed(-1);
    if (!player || !player->IsAlive()) {
        return false;
    }

    // 4. Controles desabilitados por script/jogo
    CPad* pad = CPad::GetPad(0);
    if (pad && pad->DisablePlayerControls) {
        return false;
    }

    return true;
}

// ---- WndProc Hook ----
LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // F1 toggles phone visibility (ignore auto-repeat)
    if (msg == WM_KEYDOWN && wParam == VK_F1 && !(lParam & 0x40000000)) {
        if (phone.isOpen()) {
            phone.close(PhoneAnimMode::SMOOTH);
            phone.closeApp();
        } else if (CanOpenPhone()) {
            phone.open(PhoneAnimMode::SMOOTH);
            
            // Centraliza o cursor do mouse
            if (gameWindow) {
                RECT rect;
                GetClientRect(gameWindow, &rect);
                POINT center = { rect.right / 2, rect.bottom / 2 };
                ClientToScreen(gameWindow, &center);
                SetCursorPos(center.x, center.y);
            }
        }
        return 0;
    }

    // When phone is open, let ImGui consume input
    if (phone.isVisible()) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return 0;
    }

    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

// ---- Reset Hook ----
HRESULT __stdcall hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams) {
    if (imguiInitialized) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }
    return oReset(pDevice, pParams);
}

// ---- EndScene Hook ----
HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice) {
    if (!imguiInitialized) {
        D3DDEVICE_CREATION_PARAMETERS params;
        pDevice->GetCreationParameters(&params);
        gameWindow = params.hFocusWindow;

        oWndProc = (WNDPROC)SetWindowLongPtr(gameWindow, GWL_WNDPROC, (LONG_PTR)hkWndProc);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // Use a clean default font (slightly larger for phone readability)
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = 1.0f;
        
        // Dynamic path resolution for the font (finds the ASI location)
        HMODULE hModule = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&hkEndScene, &hModule);
        char modulePath[MAX_PATH];
        GetModuleFileNameA(hModule, modulePath, MAX_PATH);
        std::string fullPath = modulePath;
        std::string basePath = fullPath.substr(0, fullPath.find_last_of("\\/")) + "\\";

        // Load Roboto
        io.Fonts->AddFontFromFileTTF((basePath + "Roboto-Medium.ttf").c_str(), 15.0f);

        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        io.Fonts->AddFontFromFileTTF((basePath + "fa-solid-900.ttf").c_str(), 0.0f, &config, icon_ranges);

        ImGui_ImplWin32_Init(gameWindow);
        ImGui_ImplDX9_Init(pDevice);

        imguiInitialized = true;
    }

    // ---- Cursor management (draw only) ----
    if (imguiInitialized) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = phone.isVisible();
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Update and draw the phone
    phone.update(ImGui::GetIO().DeltaTime);
    if (phone.isVisible()) {
        phone.draw();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

// ---- Hook Installation ----
static bool hookInstalled = false;

void TryInstallHook() {
    if (hookInstalled) return;

    IDirect3DDevice9* device = *(IDirect3DDevice9**)0xC97C28;
    if (!device) return;

    void** vtable = *(void***)device;

    if (MH_Initialize() != MH_OK) return;
    if (MH_CreateHook(vtable[42], &hkEndScene, (void**)&oEndScene) != MH_OK) return;
    MH_CreateHook(vtable[16], &hkReset, (void**)&oReset);
    MH_EnableHook(MH_ALL_HOOKS);

    hookInstalled = true;
}

// ---- CPad::UpdateMouse Hook ----
// GTA SA 1.0 US: CPad::UpdateMouse is at 0x53F3C0
// This is where the game reads GetCursorPos for camera rotation.
// By skipping it when the phone is open, the camera stays locked
// but WASD still works because keyboard input goes through a different path.
typedef void(__cdecl* UpdateMouse_t)();
static UpdateMouse_t oUpdateMouse = nullptr;

void __cdecl hkUpdateMouse() {
    static bool wasVisible = false;

    if (phone.isVisible()) {
        wasVisible = true;
        
        // We skip the original mouse function so ImGui can use the cursor freely.
        // HOWEVER, the game's mouse struct still holds the "last known" delta from 
        // right before the phone was opened. If we don't zero it out every frame, 
        // the camera will read that stale delta forever and spin infinitely!
        CPad* pad = CPad::GetPad(0);
        pad->NewMouseControllerState.x = 0.0f;
        pad->NewMouseControllerState.y = 0.0f;
        pad->PCTempMouseControllerState.x = 0.0f;
        pad->PCTempMouseControllerState.y = 0.0f;
        *(float*)0xB73404 = 0.0f;
        *(float*)0xB73408 = 0.0f;

        return; 
    }

    // Run the original game function. 
    // This reads the physical mouse (DirectInput) and flushes its buffer.
    oUpdateMouse();

    if (wasVisible) {
        // The phone was just closed!
        // oUpdateMouse() just processed all the accumulated mouse movement
        // from the time the phone was open. We must ZERO the result this frame
        // before the camera reads it, swallowing the jump.
        CPad* pad = CPad::GetPad(0);
        pad->NewMouseControllerState.x = 0.0f;
        pad->NewMouseControllerState.y = 0.0f;
        pad->PCTempMouseControllerState.x = 0.0f;
        pad->PCTempMouseControllerState.y = 0.0f;
        *(float*)0xB73404 = 0.0f;
        *(float*)0xB73408 = 0.0f;

        wasVisible = false;
    }
}

static bool mouseHookInstalled = false;

void TryInstallMouseHook() {
    if (mouseHookInstalled) return;
    if (MH_CreateHook((void*)0x53F3C0, &hkUpdateMouse, (void**)&oUpdateMouse) == MH_OK) {
        MH_EnableHook((void*)0x53F3C0);
        mouseHookInstalled = true;
    }
}

// ---- Plugin Entry ----
class SaSmartPhone {
public:
    SaSmartPhone() {
        phone.setClockProvider(&gtaClock);
        phone.setScreenProvider(&gtaScreen);
        weatherApp.SetWeatherProvider(&gtaWeather);

        // Register all apps (order = order on home screen)
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

        Events::gameProcessEvent += []() {
            TryInstallHook();
            TryInstallMouseHook();

            // 1. Obter o delta time de forma segura usando o CTimer do jogo (ms_fTimeStep / 50.0f)
            float dt = CTimer::ms_fTimeStep / 50.0f;

            // 2. Fechar o celular de forma forçada se ele estiver visível mas as condições de abertura não forem mais válidas
            if (phone.isVisible() && !CanOpenPhone()) {
                phone.close(PhoneAnimMode::FORCED);
                phone.closeApp();
            }

            clockApp.update(dt, &gtaClock);
        };
    }
} gSmartPhone;
