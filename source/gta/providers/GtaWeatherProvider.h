#pragma once
#include "../../core/providers/IWeatherProvider.h"
#include "../../core/providers/WeatherCommon.h"
#include <game_sa/CWeather.h>

class GtaWeatherProvider : public IWeatherProvider {
public:
    WeatherInfo GetCurrentWeather() override {
        return mapWeather(CWeather::OldWeatherType);
    }

    WeatherInfo GetNextWeather() override {
        return mapWeather(CWeather::NewWeatherType);
    }

    float GetRainIntensity() override {
        return CWeather::Rain;
    }
};
