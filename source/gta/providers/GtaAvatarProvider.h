#pragma once
#include "../../core/providers/IAvatarProvider.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <windows.h>

class GtaAvatarProvider : public IAvatarProvider {
private:
    IDirect3DDevice9* m_device = nullptr;
    std::unordered_map<std::string, ImTextureID> m_textures;

    static void getAnchorAddress() {}

    // Dynamically retrieve the absolute path of the directory containing the DLL
    std::string GetModDirectory() {
        char path[MAX_PATH];
        HMODULE hm = NULL;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)&getAnchorAddress, &hm)) {
            GetModuleFileNameA(hm, path, sizeof(path));
            std::string sPath(path);
            size_t pos = sPath.find_last_of("\\/");
            if (pos != std::string::npos) {
                return sPath.substr(0, pos + 1);
            }
        }
        return "";
    }

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
        
        // Retrieve dynamic path relative to the ASI DLL location
        std::string modDir = GetModDirectory();
        std::string path = modDir + "avatars/" + contactId + ".png";
        HRESULT hr = D3DXCreateTextureFromFileA(m_device, path.c_str(), &texture);
        if (SUCCEEDED(hr)) {
            m_textures[contactId] = (ImTextureID)texture;
            return (ImTextureID)texture;
        }

        // Try local fallback path
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
