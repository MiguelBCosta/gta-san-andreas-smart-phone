#pragma once
#include "../../core/providers/IWallpaperProvider.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include "../../../vendor/stb_image.h"

class SandboxWallpaperProvider : public IWallpaperProvider {
private:
    struct AsyncWallpaperTask {
        std::string name;
        std::string path;
        std::atomic<bool> done{false};
        std::atomic<bool> failed{false};
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
    };

    IDirect3DDevice9* m_device = nullptr;
    std::unordered_map<std::string, ImTextureID> m_textures;
    std::unordered_map<std::string, std::shared_ptr<AsyncWallpaperTask>> m_loadingTasks;
    std::mutex m_taskMutex;
    std::vector<std::string> m_wallpaperNames;
    bool m_scanned = false;

public:
    SandboxWallpaperProvider(IDirect3DDevice9* device) : m_device(device) {}
    
    ~SandboxWallpaperProvider() {
        ClearCache();
    }
    
    void ClearCache() {
        // Wait for all active background loading tasks to complete before freeing memory/resources
        std::vector<std::shared_ptr<AsyncWallpaperTask>> tasksToWaitFor;
        {
            std::lock_guard<std::mutex> lock(m_taskMutex);
            for (auto& pair : m_loadingTasks) {
                tasksToWaitFor.push_back(pair.second);
            }
        }
        for (auto& task : tasksToWaitFor) {
            while (!task->done) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            if (task->pixels) {
                stbi_image_free(task->pixels);
                task->pixels = nullptr;
            }
        }
        m_loadingTasks.clear();

        for (auto& pair : m_textures) {
            if (pair.second) {
                IDirect3DTexture9* tex = (IDirect3DTexture9*)pair.second;
                tex->Release();
            }
        }
        m_textures.clear();
    }

    std::vector<std::string> GetAvailableWallpapers() override {
        if (!m_scanned) {
            Refresh();
        }
        return m_wallpaperNames;
    }
    
    ImTextureID GetWallpaperTexture(const std::string& name) override {
        if (!m_device || name.empty()) return 0;

        auto it = m_textures.find(name);
        if (it != m_textures.end()) return it->second;

        // Check if there is an active loading task
        std::shared_ptr<AsyncWallpaperTask> task = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_taskMutex);
            auto taskIt = m_loadingTasks.find(name);
            if (taskIt != m_loadingTasks.end()) {
                task = taskIt->second;
            }
        }

        if (task) {
            if (task->done) {
                if (task->failed) {
                    m_textures[name] = 0;
                } else if (task->pixels) {
                    // Create D3D9 texture on the main thread
                    IDirect3DTexture9* texture = nullptr;
                    HRESULT hr = m_device->CreateTexture(
                        task->width, task->height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, nullptr
                    );
                    if (SUCCEEDED(hr)) {
                        D3DLOCKED_RECT rect;
                        if (SUCCEEDED(texture->LockRect(0, &rect, nullptr, 0))) {
                            unsigned char* dest = (unsigned char*)rect.pBits;
                            unsigned char* src = task->pixels;
                            
                            // stb_image loads RGBA, D3DFMT_A8R8G8B8 expects BGRA
                            for (int y = 0; y < task->height; ++y) {
                                unsigned char* rowDest = dest + y * rect.Pitch;
                                unsigned char* rowSrc = src + y * task->width * 4;
                                for (int x = 0; x < task->width; ++x) {
                                    rowDest[x * 4 + 0] = rowSrc[x * 4 + 2]; // B
                                    rowDest[x * 4 + 1] = rowSrc[x * 4 + 1]; // G
                                    rowDest[x * 4 + 2] = rowSrc[x * 4 + 0]; // R
                                    rowDest[x * 4 + 3] = rowSrc[x * 4 + 3]; // A
                                }
                            }
                            texture->UnlockRect(0);
                            m_textures[name] = (ImTextureID)texture;
                        } else {
                            texture->Release();
                            m_textures[name] = 0;
                        }
                    } else {
                        m_textures[name] = 0;
                    }
                    stbi_image_free(task->pixels);
                    task->pixels = nullptr;
                }
                
                std::lock_guard<std::mutex> lock(m_taskMutex);
                m_loadingTasks.erase(name);
                return m_textures[name];
            } else {
                // Thread is still running, return 0 (fallback wallpaper shows in the meantime)
                return 0;
            }
        }

        // Determine path and launch background thread
        std::string path = "wallpaper/" + name;
        if (!std::filesystem::exists(path)) {
            path = "source/core/resources/wallpaper/" + name;
        }

        auto newTask = std::make_shared<AsyncWallpaperTask>();
        newTask->name = name;
        newTask->path = path;

        {
            std::lock_guard<std::mutex> lock(m_taskMutex);
            m_loadingTasks[name] = newTask;
        }

        std::thread([newTask]() {
            int w = 0, h = 0, comp = 0;
            // Force 4 channels (RGBA)
            newTask->pixels = stbi_load(newTask->path.c_str(), &w, &h, &comp, 4);
            if (newTask->pixels) {
                newTask->width = w;
                newTask->height = h;
                newTask->done = true;
            } else {
                newTask->failed = true;
                newTask->done = true;
            }
        }).detach();

        return 0;
    }
    
    void Refresh() override {
        m_wallpaperNames.clear();
        
        namespace fs = std::filesystem;
        std::string folderPath = "wallpaper/";
        try {
            if (!fs::exists(folderPath)) {
                fs::create_directories(folderPath);
            }
            for (const auto& entry : fs::directory_iterator(folderPath)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                        m_wallpaperNames.push_back(entry.path().filename().string());
                    }
                }
            }
        } catch (...) {}
        
        m_scanned = true;
    }
};
