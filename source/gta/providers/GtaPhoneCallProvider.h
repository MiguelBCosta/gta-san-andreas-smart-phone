#pragma once
#include "../../core/providers/IPhoneCallProvider.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/CRunningScript.h>
#include <game_sa/CMessages.h>
#include <game_sa/CPad.h>
#include <windows.h>
#include <algorithm>
#include <string>

class GtaPhoneCallProvider : public IPhoneCallProvider {
private:
    int m_answerFrames = 0;
    int m_hangupFrames = 0;

public:
    void Update() {
        if (m_answerFrames > 0) {
            m_answerFrames--;
            CPad* pad = CPad::GetPad(0);
            if (pad) {
                pad->NewState.DPadLeft = 255;
            }
        }
        if (m_hangupFrames > 0) {
            m_hangupFrames--;
            CPad* pad = CPad::GetPad(0);
            if (pad) {
                pad->NewState.DPadLeft = 255;
            }
        }
    }

    bool IsIncomingCallActive() override {
        // Address 0x00A4999C is 1 if phone is ringing/active, 0 otherwise
        unsigned char ringing = *(unsigned char*)0x00A4999C;
        return (ringing == 1);
    }

    std::string GetCallerId() override {
        // 1. First, check active brief messages (subtitles) for dialogue GXT keys
        for (int i = 0; i < 8; i++) {
            tMessage& msg = CMessages::BriefMessages[i];
            if (msg.m_pText) {
                std::string key(msg.m_pText);
                std::transform(key.begin(), key.end(), key.begin(), ::toupper);
                
                if (key.rfind("DEN", 0) == 0) return "denise";
                if (key.rfind("MIC", 0) == 0) return "michelle";
                if (key.rfind("HEL", 0) == 0) return "helena";
                if (key.rfind("KAT", 0) == 0) return "katie";
                if (key.rfind("BAR", 0) == 0) return "barbara";
                if (key.rfind("MIL", 0) == 0) return "millie";
                
                if (key.rfind("KND", 0) == 0 || key.rfind("KEND", 0) == 0) return "kendl";
                if (key.rfind("SMO", 0) == 0) return "smoke";
                if (key.rfind("LOC", 0) == 0 || key.rfind("OGL", 0) == 0) return "ogloc";
                if (key.rfind("ROS", 0) == 0 || key.rfind("ROSE", 0) == 0) return "rosenberg";
                if (key.rfind("PAU", 0) == 0) return "kentpaul";
                if (key.rfind("LEO", 0) == 0) return "salvatore";
                if (key.rfind("JET", 0) == 0) return "jethro";
                
                if (key.rfind("TEN", 0) == 0 || key.rfind("CRA", 0) == 0) return "tenpenny";
                if (key.rfind("PUL", 0) == 0) return "pulaski";
                if (key.rfind("HER", 0) == 0) return "hernandez";
            }
        }

        // 2. Loop through active scripts to identify who is calling CJ
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
                if (n.find("ten") != std::string::npos) return "tenpenny";
                if (n.find("pul") != std::string::npos) return "pulaski";
                if (n.find("zer") != std::string::npos) return "zero";
                
                // Story/CRASH/Girlfriend threads mappings
                if (n.find("smo") != std::string::npos) return "smoke";
                if (n.find("loc") != std::string::npos || n.find("ogl") != std::string::npos) return "ogloc";
                if (n.find("ros") != std::string::npos || n.find("rose") != std::string::npos) return "rosenberg";
                if (n.find("pau") != std::string::npos) return "kentpaul";
                if (n.find("leo") != std::string::npos) return "salvatore";
                if (n.find("jet") != std::string::npos) return "jethro";
                
                // Girlfriend thread name
                if (n.find("gf") != std::string::npos) return "girlfriend";
                
                // Kendl thread name (avoid conflicts with rosenberg/kentpaul)
                if (n.find("knd") != std::string::npos || n.find("ken") != std::string::npos) {
                    if (n.find("ros") == std::string::npos && n.find("pau") == std::string::npos) {
                        return "kendl";
                    }
                }
            }
            script = script->m_pNext;
        }
        return "unknown";
    }

    void AnswerCall() override {
        m_answerFrames = 15;
    }

    void HangUpCall() override {
        m_hangupFrames = 15;
    }
};
