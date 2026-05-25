#pragma once
#include "../../core/providers/IStorageProvider.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

class SandboxStorageProvider : public IStorageProvider {
public:
    SandboxStorageProvider() {
        m_basePath = "sandbox_saves\\";
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

    std::string GetSlotPath(int slot) const {
        return m_basePath + "slot_" + std::to_string(slot) + ".json";
    }
};
