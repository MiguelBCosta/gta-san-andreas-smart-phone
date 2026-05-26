#pragma once
#include "../../core/providers/IPhoneCallProvider.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/CRunningScript.h>
#include <windows.h>
#include <algorithm>
#include <string>

class GtaPhoneCallProvider : public IPhoneCallProvider {
public:
    bool IsIncomingCallActive() override {
        // Address 0x00A4999C is 1 if phone is ringing/active, 0 otherwise
        unsigned char ringing = *(unsigned char*)0x00A4999C;
        return (ringing == 1);
    }

    std::string GetCallerId() override {
        // Loop through active scripts to identify who is calling CJ
        CRunningScript* script = CTheScripts::pActiveScripts;
        while (script) {
            char name[9] = { 0 };
            memcpy(name, script->m_szName, 8);
            std::string n(name);
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            
            // Script name signatures for phone calls
            if (n.rfind("mob", 0) == 0 || n.rfind("cell", 0) == 0 || n.rfind("phone", 0) == 0) {
                if (n.find("swe") != std::string::npos) return "sweet";
                if (n.find("ces") != std::string::npos || n.find("via") != std::string::npos) return "cesar";
                if (n.find("cat") != std::string::npos) return "catalina";
                if (n.find("woo") != std::string::npos) return "woozie";
                if (n.find("tru") != std::string::npos) return "truth";
                if (n.find("tor") != std::string::npos) return "toreno";
                if (n.find("ten") != std::string::npos || n.find("pul") != std::string::npos) return "tenpenny";
                if (n.find("zer") != std::string::npos) return "zero";
            }
            script = script->m_pNext;
        }
        return "unknown";
    }

    void AnswerCall() override {
        // Press TAB key in game (default Action key)
        keybd_event(VK_TAB, 0, 0, 0);
        keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
    }

    void HangUpCall() override {
        // Press TAB key to end/skip active conversation script
        keybd_event(VK_TAB, 0, 0, 0);
        keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
    }
};
