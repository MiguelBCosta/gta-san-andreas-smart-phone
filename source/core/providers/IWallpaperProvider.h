#pragma once
#include <imgui.h>
#include <string>
#include <vector>

class IWallpaperProvider {
public:
    virtual ~IWallpaperProvider() = default;

    // Scans the wallpaper directory and returns a list of files (e.g. "cyberpunk.png", "synthwave.png")
    virtual std::vector<std::string> GetAvailableWallpapers() = 0;

    // Loads the texture from file if not loaded, and returns its ImTextureID. Returns nullptr if fails.
    virtual ImTextureID GetWallpaperTexture(const std::string& name) = 0;

    // Clears loaded textures cache and rescans directory
    virtual void Refresh() = 0;
};
