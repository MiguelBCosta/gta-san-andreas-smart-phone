#pragma once
#include <imgui.h>
#include <string>

class IAvatarProvider {
public:
    virtual ~IAvatarProvider() = default;
    virtual ImTextureID GetAvatarTexture(const std::string& contactId) = 0;
};
