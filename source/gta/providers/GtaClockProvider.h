#pragma once
#include "../../core/providers/IClockProvider.h"
#include <game_sa/CClock.h>
#include <game_sa/CGameLogic.h>
#include <game_sa/CTheScripts.h>

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
        CGameLogic::PassTime(hoursToPass * 60);
    }

    bool CanSkipTime() override {
        return !CTheScripts::IsPlayerOnAMission();
    }
};
