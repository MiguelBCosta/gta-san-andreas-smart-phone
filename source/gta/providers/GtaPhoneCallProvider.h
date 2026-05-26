#pragma once
#include "../../core/providers/IPhoneCallProvider.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/CRunningScript.h>
#include <windows.h>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

// $CELLPHONE_CALL_ID → Caller mapping from GTA SA main.scm
// Source: CLEO phone script community reference
// Global vars base: 0x00A49960, each var = 4 bytes
// $PHONE_RINGING_FLAG confirmed at 0x00A4999C (index 15)
// $CELLPHONE_CALL_ID address determined by debug log below

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

class GtaPhoneCallProvider : public IPhoneCallProvider {
private:
    bool m_debugLogged = false;

    // $CELLPHONE_CALL_ID address (global variable $1412)
    static constexpr uintptr_t RINGING_FLAG_ADDR  = 0x00A4999C;
    static constexpr uintptr_t CALL_ID_ADDR       = 0x00A4AF70; 

    void LogDebugInfo() {
        std::ofstream log("phone_call_debug.log", std::ios::app);
        log << "\n=== PHONE CALL ACTIVE ===\n";

        // Dump 30 global ints around the ringing flag (index -5 to +25)
        log << "Memory around 0xA4999C (ringing flag = index 0):\n";
        for (int i = -5; i <= 25; i++) {
            uintptr_t addr = RINGING_FLAG_ADDR + (uintptr_t)(i * 4);
            int val = *(int*)addr;
            log << "  [" << (i >= 0 ? "+" : "") << i << "] 0x"
                << std::hex << addr << std::dec << " = " << val << "\n";
        }

        // Dump candidate Call ID variables specifically
        log << "Special Candidate Addresses:\n";
        log << "  0x00A49A04 (index +26) = " << *(int*)0x00A49A04 << "\n";
        log << "  0x00A4AFD0 (index 1436) = " << *(int*)0x00A4AFD0 << "\n";

        // Dump all non-zero global variables from index 0 to 2500
        log << "Non-zero global variables ($0 to $2500):\n";
        uintptr_t base_addr = 0x00A49960;
        for (int i = 0; i <= 2500; i++) {
            uintptr_t addr = base_addr + (uintptr_t)(i * 4);
            int val = *(int*)addr;
            if (val != 0) {
                log << "  $" << i << " (0x" << std::hex << addr << std::dec << ") = " << val << "\n";
            }
        }

        // Dump all active thread names
        log << "Active script threads:\n";
        CRunningScript* script = CTheScripts::pActiveScripts;
        while (script) {
            char name[9] = { 0 };
            memcpy(name, script->m_szName, 8);
            log << "  [" << name << "]\n";
            script = script->m_pNext;
        }
        log << "=========================\n";
        log.close();
    }

public:
    bool IsIncomingCallActive() override {
        unsigned char ringing = *(unsigned char*)RINGING_FLAG_ADDR;
        bool active = (ringing == 1);

        if (active && !m_debugLogged) {
            LogDebugInfo();
            m_debugLogged = true;
        }
        if (!active) m_debugLogged = false;

        return active;
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
