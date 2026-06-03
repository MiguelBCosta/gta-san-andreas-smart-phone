#include "LocalizationManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <algorithm>

// ---------------------------------------------------------------------------
// Singleton instance
// ---------------------------------------------------------------------------
LocalizationManager& LocalizationManager::Get() {
    static LocalizationManager instance;
    return instance;
}

// ---------------------------------------------------------------------------
// Init — define o path base e escaneia os idiomas disponíveis
// ---------------------------------------------------------------------------
void LocalizationManager::Init(const std::string& basePath) {
    m_basePath = basePath;
    ScanLanguages();

    // Idioma padrão: inglês. LoadLanguage faz fallback gracioso se não existir.
    LoadLanguage(m_currentLang);
}

// ---------------------------------------------------------------------------
// ScanLanguages — lê todos os .json da pasta lang/ e extrai metadados
// ---------------------------------------------------------------------------
void LocalizationManager::ScanLanguages() {
    m_availableLanguages.clear();
    std::string langDir = m_basePath + "lang/";

    std::error_code ec;
    if (!std::filesystem::exists(langDir, ec)) return;

    for (const auto& entry : std::filesystem::directory_iterator(langDir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;

        auto path = entry.path();
        if (path.extension() != ".json") continue;

        std::string code = path.stem().string();

        // Tenta ler o campo "name" do JSON para obter o nome amigável
        std::ifstream file(path);
        if (!file.is_open()) continue;

        std::string displayName = code; // fallback: usar o código
        try {
            nlohmann::json j = nlohmann::json::parse(file, nullptr, false);
            if (!j.is_discarded() && j.contains("name") && j["name"].is_string()) {
                displayName = j["name"].get<std::string>();
            }
        } catch (...) {}

        m_availableLanguages.push_back({ code, displayName });
    }

    // Ordenar por código para exibição consistente
    std::sort(m_availableLanguages.begin(), m_availableLanguages.end(),
        [](const LanguageInfo& a, const LanguageInfo& b) {
            return a.code < b.code;
        });
}

// ---------------------------------------------------------------------------
// SetLanguage — troca idioma e recarrega strings imediatamente
// ---------------------------------------------------------------------------
void LocalizationManager::SetLanguage(const std::string& langCode) {
    m_currentLang = langCode;
    LoadLanguage(langCode);
}

const std::string& LocalizationManager::GetCurrentLanguage() const {
    return m_currentLang;
}

const std::vector<LanguageInfo>& LocalizationManager::GetAvailableLanguages() const {
    return m_availableLanguages;
}

// ---------------------------------------------------------------------------
// LoadLanguage — carrega o JSON de um idioma e preenche m_strings
// ---------------------------------------------------------------------------
void LocalizationManager::LoadLanguage(const std::string& langCode) {
    m_strings.clear();

    std::string filePath = m_basePath + "lang/" + langCode + ".json";
    std::ifstream file(filePath);
    if (!file.is_open()) return;

    try {
        nlohmann::json j = nlohmann::json::parse(file, nullptr, false);
        if (j.is_discarded()) return;

        // Percorre todas as chaves e armazena os valores como strings
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_string()) {
                m_strings[it.key()] = it.value().get<std::string>();
            }
        }
    } catch (...) {}
}

// ---------------------------------------------------------------------------
// Translate — retorna a string traduzida ou a própria chave como fallback
// ---------------------------------------------------------------------------
const char* LocalizationManager::Translate(const char* key) const {
    auto it = m_strings.find(key);
    if (it != m_strings.end()) {
        return it->second.c_str();
    }
    return key; // fallback: exibir a chave bruta
}
