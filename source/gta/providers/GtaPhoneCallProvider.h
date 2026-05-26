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
        // Scan active script threads to identify the caller by thread name
        CRunningScript* script = CTheScripts::pActiveScripts;
        while (script) {
            char name[9] = { 0 };
            memcpy(name, script->m_szName, 8);
            std::string n(name);
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            
            // Only look at phone/mobile/cell/mob_ related threads
            bool isMobThread = (n.rfind("mob", 0) == 0 || n.rfind("cell", 0) == 0 || n.rfind("phone", 0) == 0);
            if (isMobThread) {
                // --- Grove Street / Families ---
                if (n.find("swe") != std::string::npos) return "sweet";
                if (n.find("ces") != std::string::npos || n.find("via") != std::string::npos) return "cesar";
                if (n.find("knd") != std::string::npos || n.find("ken") != std::string::npos) return "kendl";
                if (n.find("smo") != std::string::npos) return "smoke";
                if (n.find("loc") != std::string::npos || n.find("ogl") != std::string::npos) return "ogloc";
                
                // --- C.R.A.S.H. ---
                // "mob_her" or "mob_cra" or any thread with "her" and no "woo"
                if (n.find("her") != std::string::npos && n.find("woo") == std::string::npos) return "hernandez";
                if (n.find("pul") != std::string::npos) return "pulaski";
                // Tenpenny must come after hernandez/pulaski checks
                if (n.find("ten") != std::string::npos) return "tenpenny";
                
                // --- Other story characters ---
                if (n.find("cat") != std::string::npos) return "catalina";
                if (n.find("woo") != std::string::npos) return "woozie";
                if (n.find("tru") != std::string::npos) return "truth";
                if (n.find("tor") != std::string::npos) return "toreno";
                if (n.find("ros") != std::string::npos) return "rosenberg";
                if (n.find("pau") != std::string::npos) return "kentpaul";
                if (n.find("leo") != std::string::npos) return "salvatore";
                if (n.find("jet") != std::string::npos) return "jethro";
                if (n.find("zer") != std::string::npos) return "zero";
                
                // --- Girlfriends ---
                if (n.find("gf") != std::string::npos) return "girlfriend";
                if (n.find("den") != std::string::npos) return "denise";
                if (n.find("mic") != std::string::npos) return "michelle";
                if (n.find("hel") != std::string::npos) return "helena";
                if (n.find("kat") != std::string::npos) return "katie";
                if (n.find("bar") != std::string::npos) return "barbara";
                if (n.find("mil") != std::string::npos) return "millie";
            }
            script = script->m_pNext;
        }
        return "unknown";
    }

    void AnswerCall() override {
        // No-op: buttons are UI-only placeholders; game handles answering natively via TAB
    }

    void HangUpCall() override {
        // No-op: buttons are UI-only placeholders; game handles call end natively via TAB
    }
};
