#pragma once
#include "../../core/providers/IStorageProvider.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

class GtaStorageProvider : public IStorageProvider {
public:
    GtaStorageProvider() {
        HMODULE hModule = NULL;
        // Get the module handle from this function's address to correctly locate the ASI folder
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                               (LPCSTR)&GtaStorageProvider::getAnchorAddress, &hModule)) {
            char modulePath[MAX_PATH];
            GetModuleFileNameA(hModule, modulePath, MAX_PATH);
            std::filesystem::path fullPath(modulePath);
            m_basePath = (fullPath.parent_path() / "saves").string() + "\\";
        } else {
            m_basePath = "modloader\\sasmartphone\\saves\\";
        }
    }

    std::string ReadSlotData(int slot) override {
        std::string path = GetSlotPath(slot);
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool WriteSlotData(int slot, const std::string& jsonContent) override {
        std::string path = GetSlotPath(slot);
        try {
            std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        } catch (...) {
            // Fail silently or handle fallback
        }

        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << jsonContent;
        return true;
    }

    bool DeleteSlotData(int slot) override {
        std::string path = GetSlotPath(slot);
        std::error_code ec;
        return std::filesystem::remove(path, ec);
    }

private:
    std::string m_basePath;

    // Anchor to retrieve module handle
    static void getAnchorAddress() {}

    std::string GetSlotPath(int slot) const {
        return m_basePath + "slot_" + std::to_string(slot) + ".json";
    }
};
