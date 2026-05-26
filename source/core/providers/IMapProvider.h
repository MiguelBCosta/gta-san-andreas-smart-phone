#pragma once
#include <imgui.h>
#include <string>
#include <vector>

struct MapBlip {
    float x;
    float y;
    int sprite;
    int color;
    std::string name;
    int id;
};

class IMapProvider {
public:
    virtual ~IMapProvider() = default;
    virtual bool GetPlayerPosition(float& x, float& y, float& heading) = 0;
    virtual bool GetWaypointPosition(float& x, float& y) = 0;
    virtual void SetWaypointPosition(float x, float y) = 0;
    virtual void ClearWaypoint() = 0;
    virtual void LoadMapTiles() = 0;
    virtual ImTextureID GetTileTexture(int index) = 0;
    virtual ImTextureID GetBlipTexture(int spriteId) = 0;
    virtual std::vector<MapBlip> GetActiveBlips() = 0;
};
