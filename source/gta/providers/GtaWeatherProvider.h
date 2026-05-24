#pragma once
#include "../../core/providers/IWeatherProvider.h"
#include <game_sa/CWeather.h>
#include <IconsFontAwesome5.h>

class GtaWeatherProvider : public IWeatherProvider {
private:
    WeatherInfo MapWeather(int id) {
        WeatherInfo info;
        switch (id) {
            case 0:  info = { "Muito ensolarado", "Los Santos", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 1:  info = { "Ensolarado", "Los Santos", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 2:  info = { "Ensolarado c/ névoa", "Los Santos", ICON_FA_SMOG, WeatherTheme::FOGGY }; break;
            case 3:  info = { "Ensolarado c/ fumaça", "Los Santos", ICON_FA_SMOG, WeatherTheme::FOGGY }; break;
            case 4:  info = { "Nublado", "Los Santos", ICON_FA_CLOUD, WeatherTheme::CLOUDY }; break;
            case 5:  info = { "Ensolarado", "San Fierro", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 6:  info = { "Muito ensolarado", "San Fierro", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 7:  info = { "Nublado", "San Fierro", ICON_FA_CLOUD, WeatherTheme::CLOUDY }; break;
            case 8:  info = { "Chuvoso", "San Fierro", ICON_FA_CLOUD_SHOWERS_HEAVY, WeatherTheme::RAINY }; break;
            case 9:  info = { "Neblina", "San Fierro", ICON_FA_SMOG, WeatherTheme::FOGGY }; break;
            case 10: info = { "Ensolarado", "Las Venturas", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 11: info = { "Muito ensolarado", "Las Venturas", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 12: info = { "Nublado", "Las Venturas", ICON_FA_CLOUD, WeatherTheme::CLOUDY }; break;
            case 13: info = { "Muito ensolarado", "Interior", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 14: info = { "Ensolarado", "Interior", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 15: info = { "Nublado", "Interior", ICON_FA_CLOUD, WeatherTheme::CLOUDY }; break;
            case 16: info = { "Chuvoso", "Interior", ICON_FA_CLOUD_SHOWERS_HEAVY, WeatherTheme::RAINY }; break;
            case 17: info = { "Muito ensolarado", "Bone County", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 18: info = { "Ensolarado", "Bone County", ICON_FA_SUN, WeatherTheme::SUNNY }; break;
            case 19: info = { "Tempestade de areia", "Bone County", ICON_FA_WIND, WeatherTheme::SANDSTORM }; break;
            case 20: info = { "Subaquático", "San Andreas", ICON_FA_WATER, WeatherTheme::SPECIAL }; break;
            case 21: info = { "Cores Extra 1", "San Andreas", ICON_FA_PALETTE, WeatherTheme::SPECIAL }; break;
            case 22: info = { "Cores Extra 2", "San Andreas", ICON_FA_PALETTE, WeatherTheme::SPECIAL }; break;
            default:
                info = { "Clima " + std::to_string(id), "San Andreas", ICON_FA_CLOUD, WeatherTheme::SPECIAL };
                break;
        }
        return info;
    }

public:
    WeatherInfo GetCurrentWeather() override {
        return MapWeather(CWeather::OldWeatherType);
    }

    WeatherInfo GetNextWeather() override {
        return MapWeather(CWeather::NewWeatherType);
    }

    float GetRainIntensity() override {
        return CWeather::Rain;
    }
};
