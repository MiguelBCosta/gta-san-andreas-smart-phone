#pragma once
#include "../PhoneApp.h"
#include <imgui.h>

class IClockProvider;

enum class SkipState {
    IDLE,
    INITIATE,
    FADING_OUT,
    WAITING_DARK,
    SET_TIME,
    FADING_IN,
    FINISHED
};

class ClockApp : public PhoneApp {
private:
    SkipState m_state = SkipState::IDLE;
    float m_stateTimer = 0.0f;
    float m_fadeAlpha = 0.0f;
    bool m_isSkipping = false;
    int m_skipTargetHour = 12;
    int m_hoursToPass = 0;

    const char* getPeriodo(int h);
    ImVec4 getPeriodoColor(int h);
    const char* getGameDay(int d);

public:
    ClockApp();

    bool isSkipping() const { return m_isSkipping; }
    float getFadeAlpha() const { return m_fadeAlpha; }

    void onOpen() override;
    void update(float dt) override;
    void onDraw() override;
    void onLanguageChange() override { name = TR("clock.title"); }
};
