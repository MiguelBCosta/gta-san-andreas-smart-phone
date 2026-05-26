#pragma once
#include "../../core/providers/IPhoneCallProvider.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/CRunningScript.h>
#include <windows.h>
#include <algorithm>
#include <string>

// $CELLPHONE_CALL_ID → Caller mapping from GTA SA main.scm
// Source: CLEO phone script community reference
// Global vars base: 0x00A49960, each var = 4 bytes
// $PHONE_RINGING_FLAG confirmed at 0x00A4999C (index 15)

class GtaPhoneCallProvider : public IPhoneCallProvider {
private:
    // $CELLPHONE_CALL_ID address (global variable $1412)
    static constexpr uintptr_t RINGING_FLAG_ADDR  = 0x00A4999C;
    static constexpr uintptr_t CALL_ID_ADDR       = 0x00A4AF70; 

    static std::string CallerFromCallId(int id) {
        // Catalina
        if (id >= 0  && id <= 5)  return "catalina";
        // OG Loc
        if (id == 6)              return "ogloc";
        // Cesar
        if (id == 7  || id == 8  || id == 10) return "cesar";
        // Kendl
        if (id == 11)             return "kendl";
        // C.R.A.S.H. (Tenpenny, Hernandez, Pulaski)
        if (id == 9  || id == 12 || id == 13) return "crash";
        // Smoke
        if (id == 14)             return "smoke";
        // Truth
        if (id == 15 || id == 16 || id == 17) return "truth";
        // Sweet
        if (id >= 18 && id <= 25) return "sweet";
        // Zero
        if (id == 29 || id == 30) return "zero";
        // Jethro
        if (id >= 31 && id <= 33) return "jethro";
        // Woozie
        if (id == 34 || (id >= 35 && id <= 40)) return "woozie";
        // Toreno
        if (id >= 41 && id <= 46) return "toreno";
        // Kent Paul
        if (id == 47)             return "kentpaul";
        // Rosenberg
        if (id >= 48 && id <= 50) return "rosenberg";
        // Salvatore
        if (id == 51 || id == 52) return "salvatore";

        // Girlfriends
        if (id == 70  || id == 130) return "denise";
        if (id == 80  || id == 131) return "michelle";
        if (id == 90  || id == 132) return "helena";
        if (id == 100 || id == 133) return "barbara";
        if (id == 110 || id == 134) return "katie";
        if (id == 53  || id == 120 || id == 135) return "millie";

        return "";
    }

public:
    bool IsIncomingCallActive() override {
        unsigned char ringing = *(unsigned char*)RINGING_FLAG_ADDR;
        return (ringing == 1);
    }

    std::string GetCallerId() override {
        // 1. Try $CELLPHONE_CALL_ID first (most reliable, mapped to all character and girlfriend calls)
        int callId = *(int*)CALL_ID_ADDR;
        std::string fromId = CallerFromCallId(callId);
        if (!fromId.empty()) return fromId;

        // 2. Fallback: only scan for girlfriend thread signature to identify generic girlfriend call
        CRunningScript* script = CTheScripts::pActiveScripts;
        while (script) {
            char name[9] = { 0 };
            memcpy(name, script->m_szName, 8);
            std::string n(name);
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);

            if (n.find("gf") != std::string::npos) return "girlfriend";

            script = script->m_pNext;
        }

        return "unknown";
    }

    void AnswerCall() override {}
    void HangUpCall() override {}
};
