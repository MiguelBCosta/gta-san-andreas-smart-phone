#pragma once
#include "../../core/providers/IClockProvider.h"
#include <ctime>

class SandboxClockProvider : public IClockProvider {
private:
    bool m_hasOverride = false;
    int m_overrideHour = 0;
    int m_overrideMin = 0;
    int m_overrideDay = 1;

public:
    PhoneTime GetTime() override {
        if (m_hasOverride) {
            return { m_overrideHour, m_overrideMin, 0, m_overrideDay };
        }
        time_t rawtime;
        time(&rawtime);
        struct tm* timeinfo = localtime(&rawtime);
        if (timeinfo) {
            return { timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_wday + 1 };
        }
        return { 0, 0, 0, 1 };
    }

    void SkipTime(int hoursToPass) override {
        PhoneTime current = GetTime();
        int targetHour = current.hours + hoursToPass;
        int daysPassed = targetHour / 24;

        m_overrideHour = targetHour % 24;
        m_overrideMin = current.minutes;

        int newDay = current.dayOfWeek + daysPassed;
        while (newDay > 7) newDay -= 7;
        m_overrideDay = newDay;

        m_hasOverride = true;
    }
};
