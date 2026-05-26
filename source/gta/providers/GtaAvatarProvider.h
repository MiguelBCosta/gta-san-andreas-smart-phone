#pragma once
#include "../../core/providers/IAvatarProvider.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <unordered_map>
#include <string>

class GtaAvatarProvider : public IAvatarProvider {
private:
    IDirect3DDevice9* m_device = nullptr;
    std::unordered_map<std::string, ImTextureID> m_textures;

public:
    GtaAvatarProvider(IDirect3DDevice9* device) : m_device(device) {}
    
    ~GtaAvatarProvider() {
        for (auto& pair : m_textures) {
            if (pair.second) {
                IDirect3DTexture9* tex = (IDirect3DTexture9*)pair.second;
                tex->Release();
            }
        }
        m_textures.clear();
    }

    ImTextureID GetAvatarTexture(const std::string& contactId) override {
        if (!m_device) return 0;

        auto it = m_textures.find(contactId);
        if (it != m_textures.end()) return it->second;

        IDirect3DTexture9* texture = nullptr;
        // Search path: modloader/sasmartphone/avatars/<id>.png
        std::string path = "modloader/sasmartphone/avatars/" + contactId + ".png";
        HRESULT hr = D3DXCreateTextureFromFileA(m_device, path.c_str(), &texture);
        if (SUCCEEDED(hr)) {
            m_textures[contactId] = (ImTextureID)texture;
            return (ImTextureID)texture;
        }

        // Try fallback path relative to execution
        path = "avatars/" + contactId + ".png";
        hr = D3DXCreateTextureFromFileA(m_device, path.c_str(), &texture);
        if (SUCCEEDED(hr)) {
            m_textures[contactId] = (ImTextureID)texture;
            return (ImTextureID)texture;
        }

        m_textures[contactId] = 0;
        return 0;
    }
};
