#include <plugin.h>
#include <game_sa/CPad.h>
#include <d3d9.h>
#include <MinHook.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

#include "Phone.h"
#include "apps/NotesApp.h"
#include "apps/PlaceholderApps.h"

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
static Phone phone;

// ---- App Instances (static lifetime) ----
static CalculatorApp calcApp;
static CameraApp     cameraApp;
static ClockApp      clockApp;
static GarageApp     garageApp;
static MapsApp       mapsApp;
static MessagesApp   messagesApp;
static MusicApp      musicApp;
static NotesApp      notesApp;
static PhoneCallApp  phoneCallApp;
static SettingsApp   settingsApp;
static WeatherApp    weatherApp;

// ---- WndProc Hook ----
LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // F1 toggles phone visibility (ignore auto-repeat)
    if (msg == WM_KEYDOWN && wParam == VK_F1 && !(lParam & 0x40000000)) {
        if (phone.visible) {
            phone.closePhone();
        } else {
            phone.visible = true;

            // Centraliza o cursor do mouse assim que o celular abre
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
    if (phone.visible) {
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

        ImGui_ImplWin32_Init(gameWindow);
        ImGui_ImplDX9_Init(pDevice);

        imguiInitialized = true;
    }

    // ---- Cursor management (draw only) ----
    if (imguiInitialized) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = phone.visible;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Draw the phone
    phone.draw();

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

    if (phone.visible) {
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
        };
    }
} gSmartPhone;
