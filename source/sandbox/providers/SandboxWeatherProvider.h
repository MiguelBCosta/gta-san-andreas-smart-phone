#pragma once
#include "../../core/providers/IWeatherProvider.h"
#include "../../core/providers/WeatherCommon.h"

class SandboxWeatherProvider : public IWeatherProvider {
private:
    int m_currentWeatherId = 1; // Default Sunny
    int m_nextWeatherId = 4;    // Default Cloudy
    float m_rainIntensity = 0.0f;

public:
    WeatherInfo GetCurrentWeather() override {
        return mapWeather(m_currentWeatherId);
    }

    WeatherInfo GetNextWeather() override {
        return mapWeather(m_nextWeatherId);
    }

    float GetRainIntensity() override {
        return m_rainIntensity;
    }

    // Setters para simulação na Sandbox
    void SetCurrentWeather(int id) {
        m_currentWeatherId = id;
    }

    void SetNextWeather(int id) {
        m_nextWeatherId = id;
    }

    void SetRainIntensity(float intensity) {
        m_rainIntensity = intensity;
        if (m_rainIntensity < 0.0f) m_rainIntensity = 0.0f;
        if (m_rainIntensity > 1.0f) m_rainIntensity = 1.0f;
    }

    int GetCurrentWeatherId() const { return m_currentWeatherId; }
    int GetNextWeatherId() const { return m_nextWeatherId; }
};
