#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Macro de acesso rápido às strings traduzidas
// Uso: ImGui::Text(TR("clock.title"));
// ---------------------------------------------------------------------------
#define TR(key) LocalizationManager::Get().Translate(key)

// ---------------------------------------------------------------------------
// Estrutura de metadados de um idioma disponível
// ---------------------------------------------------------------------------
struct LanguageInfo {
    std::string code; // ex: "en", "pt"
    std::string name; // ex: "🇺🇸 English" (lido do JSON)
};

// ---------------------------------------------------------------------------
// LocalizationManager — Singleton
// Responsável por:
//   - Escanear a pasta lang/ e detectar idiomas disponíveis
//   - Carregar o arquivo JSON do idioma ativo
//   - Fornecer strings traduzidas com fallback para inglês hardcoded
// ---------------------------------------------------------------------------
class LocalizationManager {
public:
    static LocalizationManager& Get();

    // Inicializa o manager com o path base (relativo ao .asi ou exe)
    // Deve ser chamado uma vez na inicialização (Main.cpp / SandboxMain.cpp)
    void Init(const std::string& basePath);

    // Troca o idioma ativo e recarrega as strings. Efeito imediato.
    void SetLanguage(const std::string& langCode);

    // Retorna o código do idioma ativo (ex: "en")
    const std::string& GetCurrentLanguage() const;

    // Retorna a lista de idiomas disponíveis (escaneada da pasta lang/)
    const std::vector<LanguageInfo>& GetAvailableLanguages() const;

    // Retorna a string traduzida para a chave fornecida.
    // Fallback: retorna a própria chave se não encontrada.
    const char* Translate(const char* key) const;

private:
    LocalizationManager() = default;
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    void ScanLanguages();
    void LoadLanguage(const std::string& langCode);

    std::string m_basePath;
    std::string m_currentLang = "en";
    std::unordered_map<std::string, std::string> m_strings;
    std::vector<LanguageInfo> m_availableLanguages;
};
