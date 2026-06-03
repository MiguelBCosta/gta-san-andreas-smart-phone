#pragma once
#include "../PhoneApp.h"
#include "../ServiceContainer.h"

class IWeatherProvider;
enum class WeatherTheme;

class WeatherApp : public PhoneApp {
private:
    Inject<IWeatherProvider> m_weatherProvider;

    void getThemeColors(WeatherTheme theme, ImColor& topColor, ImColor& bottomColor);
    int getSimulatedTemp(WeatherTheme theme, float rain);

public:
    WeatherApp();

    void onDraw() override;
    void onLanguageChange() override { name = TR("weather.title"); }
    std::string getAppDescription() const override { return TR("appstore.desc.weather"); }
};
