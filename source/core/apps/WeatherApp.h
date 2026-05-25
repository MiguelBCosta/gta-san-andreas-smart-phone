#pragma once
#include "../PhoneApp.h"

class IWeatherProvider;
enum class WeatherTheme;

class WeatherApp : public PhoneApp {
private:
    IWeatherProvider* m_weatherProvider = nullptr;

    void getThemeColors(WeatherTheme theme, ImColor& topColor, ImColor& bottomColor);
    int getSimulatedTemp(WeatherTheme theme, float rain);

public:
    WeatherApp();

    void SetWeatherProvider(IWeatherProvider* provider);
    void onDraw() override;
};
