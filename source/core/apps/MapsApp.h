#pragma once
#include "../PhoneApp.h"
#include "../LocalizationManager.h"
#include "../providers/IMapProvider.h"
#include "../ServiceContainer.h"
#include <IconsFontAwesome5.h>
#include <vector>

class MapsApp : public PhoneApp {
private:
    Inject<IMapProvider> m_mapProvider;
    float m_zoom = 1.0f;
    ImVec2 m_scroll; // Will be initialized on open
    bool m_showSidebar = false;

    // Helper conversion methods
    ImVec2 worldToMap(float worldX, float worldY, float mapSize);
    ImVec2 mapToWorld(float mapX, float mapY, float mapSize);

public:
    static bool bMapAppOpen;

    MapsApp() {
        id = "maps";
        icon = ICON_FA_MAP_MARKED_ALT;
        name = TR("maps.title");
        color = ImVec4(0.18f, 0.72f, 0.30f, 1.0f);
        m_scroll = ImVec2(1536.0f, 1536.0f);
    }

    void onOpen() override;
    void onClose() override;
    void onDraw() override;
    bool onBack() override;
    void onLanguageChange() override { name = TR("maps.title"); }
};
