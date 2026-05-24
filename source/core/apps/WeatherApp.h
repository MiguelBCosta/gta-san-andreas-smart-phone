#pragma once
#include "../PhoneApp.h"
#include "../providers/IWeatherProvider.h"
#include <imgui.h>
#include <IconsFontAwesome5.h>
#include <string>

class WeatherApp : public PhoneApp {
private:
    IWeatherProvider* weatherProvider = nullptr;

    // Converte o tema em cores para o gradiente do card
    void GetThemeColors(WeatherTheme theme, ImColor& topColor, ImColor& bottomColor) {
        switch (theme) {
            case WeatherTheme::SUNNY:
                topColor = ImColor(255, 140, 0, 220);     // Dark Orange
                bottomColor = ImColor(255, 215, 0, 220);  // Gold
                break;
            case WeatherTheme::CLOUDY:
                topColor = ImColor(74, 86, 104, 220);     // Slate Gray-Blue
                bottomColor = ImColor(142, 158, 171, 220); // Light Gray
                break;
            case WeatherTheme::RAINY:
                topColor = ImColor(20, 30, 48, 230);       // Dark Blue-Gray
                bottomColor = ImColor(36, 59, 85, 230);    // Muted Ocean Blue
                break;
            case WeatherTheme::FOGGY:
                topColor = ImColor(58, 68, 80, 220);       // Deep Fog
                bottomColor = ImColor(120, 130, 140, 220); // Mist
                break;
            case WeatherTheme::SANDSTORM:
                topColor = ImColor(139, 90, 43, 230);     // Sandy Brown
                bottomColor = ImColor(210, 180, 140, 230); // Tan
                break;
            case WeatherTheme::SPECIAL:
            default:
                topColor = ImColor(26, 188, 156, 220);    // Teal
                bottomColor = ImColor(142, 68, 173, 220); // Purple
                break;
        }
    }

    // Retorna uma temperatura aproximada baseada no tema e chuva para dar mais realismo
    int GetSimulatedTemp(WeatherTheme theme, float rain) {
        switch (theme) {
            case WeatherTheme::SUNNY: return 32;
            case WeatherTheme::CLOUDY: return 22;
            case WeatherTheme::RAINY: return 17 - (int)(rain * 5.0f);
            case WeatherTheme::FOGGY: return 14;
            case WeatherTheme::SANDSTORM: return 37;
            case WeatherTheme::SPECIAL: return 20;
            default: return 25;
        }
    }

public:
    WeatherApp() {
        id = "weather";
        icon = ICON_FA_CLOUD_SUN;
        name = "Clima";
        color = ImVec4(0.25f, 0.60f, 0.90f, 1.0f);
    }

    void SetWeatherProvider(IWeatherProvider* provider) {
        weatherProvider = provider;
    }

    void onDraw() override {
        if (!weatherProvider) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Nenhum provedor de clima!");
            return;
        }

        WeatherInfo current = weatherProvider->GetCurrentWeather();
        WeatherInfo next = weatherProvider->GetNextWeather();
        float rain = weatherProvider->GetRainIntensity();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        float cardHeight = 190.0f;

        // Reservar o espaço do Card principal para o ImGui expandir os limites da janela corretamente
        ImGui::Dummy(ImVec2(width, cardHeight));

        ImVec2 cardMin = startPos;
        ImVec2 cardMax = ImVec2(startPos.x + width, startPos.y + cardHeight);

        // 1. Desenhar fundo do Card com Gradiente correspondente ao clima
        ImColor colTop, colBottom;
        GetThemeColors(current.theme, colTop, colBottom);

        drawList->AddRectFilledMultiColor(cardMin, cardMax, colTop, colTop, colBottom, colBottom);
        // Desenha uma borda sutil ao redor do card
        drawList->AddRect(cardMin, cardMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.15f)), 0.0f, 0, 1.5f);

        // 2. Textos e Informações dentro do Card (posicionados de forma absoluta dentro da área reservada)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        
        // Região (Los Santos, etc.)
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 15.0f, cardMin.y + 12.0f));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.70f), "%s", current.region.c_str());

        // Clima Atual
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 15.0f, cardMin.y + 28.0f));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Fonte padrão
        ImGui::SetWindowFontScale(1.4f);
        ImGui::Text("%s", current.name.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        // Ícone grande (FontAwesome) e Temperatura simulada
        float iconSizeMultiplier = 3.5f;
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetWindowFontScale(iconSizeMultiplier);
        ImVec2 iconSize = ImGui::CalcTextSize(current.icon.c_str());
        float iconY = cardMin.y + (cardHeight - iconSize.y) / 2.0f + 10.0f;
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 20.0f, iconY));
        ImGui::Text("%s", current.icon.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        // Temperatura
        int temp = GetSimulatedTemp(current.theme, rain);
        ImGui::SetWindowFontScale(2.6f);
        std::string tempStr = std::to_string(temp) + "°";
        ImVec2 tempSize = ImGui::CalcTextSize(tempStr.c_str());
        ImGui::SetCursorScreenPos(ImVec2(cardMax.x - tempSize.x - 20.0f, cardMin.y + 70.0f));
        ImGui::Text("%s", tempStr.c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::PopStyleColor();

        // Restaurar cursor para a base do card e adicionar espaçamento
        ImGui::SetCursorScreenPos(ImVec2(startPos.x, startPos.y + cardHeight));
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        // 3. Exibição de Chuva (se aplicável)
        if (rain > 0.0f || current.theme == WeatherTheme::RAINY) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text(ICON_FA_TINT "  Chuva:");
            ImGui::SameLine();
            
            // Barra de progresso azul estilizada para intensidade da chuva
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.20f, 0.50f, 1.0f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1, 1, 1, 0.10f));
            char rainPercent[32];
            sprintf_s(rainPercent, "%.0f%%", rain * 100.0f);
            ImGui::ProgressBar(rain, ImVec2(-1.0f, 18.0f), rainPercent);
            ImGui::PopStyleColor(2);
            
            ImGui::PopStyleVar();
            ImGui::Spacing();
        }

        // 4. Divisor sutil
        ImGui::Separator();
        ImGui::Spacing();

        // 5. Próximo Clima (Previsão)
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), "PREVISÃO DO TEMPO");
        ImGui::Spacing();

        // Reservar espaço do subcard para previsão
        ImVec2 subMin = ImGui::GetCursorScreenPos();
        float subHeight = 65.0f;
        ImGui::Dummy(ImVec2(width, subHeight));

        ImVec2 subMax = ImVec2(subMin.x + width, subMin.y + subHeight);

        // Fundo cinza translúcido estilo iOS (glassmorphism)
        drawList->AddRectFilled(subMin, subMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.07f)), 10.0f);
        drawList->AddRect(subMin, subMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.04f)), 10.0f, 0, 1.0f);

        // Ícone pequeno do próximo clima
        ImGui::SetCursorScreenPos(ImVec2(subMin.x + 15.0f, subMin.y + (subHeight - 20.0f) / 2.0f));
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("%s", next.icon.c_str());
        ImGui::SetWindowFontScale(1.0f);

        // Nome do próximo clima
        ImGui::SetCursorScreenPos(ImVec2(subMin.x + 50.0f, subMin.y + 12.0f));
        ImGui::Text("A seguir");
        ImGui::SetCursorScreenPos(ImVec2(subMin.x + 50.0f, subMin.y + 32.0f));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.7f), "%s", next.name.c_str());

        // Seta indicativa na direita
        ImGui::SetCursorScreenPos(ImVec2(subMax.x - 30.0f, subMin.y + (subHeight - 20.0f) / 2.0f));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.3f), ICON_FA_ARROW_RIGHT);

        // Restaurar cursor para a base do subcard
        ImGui::SetCursorScreenPos(ImVec2(subMin.x, subMin.y + subHeight));
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
    }
};
