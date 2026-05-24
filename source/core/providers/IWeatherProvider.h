#pragma once
#include <string>

enum class WeatherTheme {
    SUNNY,
    CLOUDY,
    RAINY,
    FOGGY,
    SANDSTORM,
    SPECIAL
};

struct WeatherInfo {
    std::string name;        // Ex: "Muito ensolarado"
    std::string region;      // Ex: "Los Santos"
    std::string icon;        // Ex: ICON_FA_SUN
    WeatherTheme theme;
};

class IWeatherProvider {
public:
    virtual ~IWeatherProvider() = default;
    virtual WeatherInfo GetCurrentWeather() = 0;
    virtual WeatherInfo GetNextWeather() = 0;
    virtual float GetRainIntensity() = 0; // 0.0f a 1.0f
};
