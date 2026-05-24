#pragma once
#include "../../core/providers/IClockProvider.h"
#include <game_sa/CClock.h>

class GtaClockProvider : public IClockProvider {
public:
    PhoneTime GetTime() override {
        return { 
            (int)CClock::ms_nGameClockHours, 
            (int)CClock::ms_nGameClockMinutes,
            (int)CClock::ms_nGameClockSeconds,
            (int)CClock::CurrentDay
        };
    }

    void SkipTime(int hoursToPass) override {
        int targetHours = (int)CClock::ms_nGameClockHours + hoursToPass;
        int daysToPass = targetHours / 24;
        int finalHours = targetHours % 24;

        // Natively offset the clock by the number of days passed
        for (int i = 0; i < daysToPass; ++i) {
            CClock::OffsetClockByADay(1); // 1 = one day forwards
        }

        // Set the final hour and minute, keeping the updated day of the week
        CClock::SetGameClock(finalHours, CClock::ms_nGameClockMinutes, CClock::CurrentDay);
    }
};
