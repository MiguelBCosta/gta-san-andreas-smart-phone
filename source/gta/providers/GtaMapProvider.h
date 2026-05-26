#pragma once
#include "../../core/providers/IMapProvider.h"
#include <d3d9.h>
#include <RenderWare.h>

class GtaMapProvider : public IMapProvider {
private:
    IDirect3DDevice9* m_device = nullptr;

    IDirect3DTexture9* GetD3DTexture(RwRaster* raster);

public:
    GtaMapProvider(IDirect3DDevice9* device) : m_device(device) {}

    bool GetPlayerPosition(float& x, float& y, float& heading) override;
    bool GetWaypointPosition(float& x, float& y) override;
    void SetWaypointPosition(float x, float y) override;
    void ClearWaypoint() override;
    void LoadMapTiles() override;
    ImTextureID GetTileTexture(int index) override;
    ImTextureID GetBlipTexture(int spriteId) override;
    std::vector<MapBlip> GetActiveBlips() override;
};
