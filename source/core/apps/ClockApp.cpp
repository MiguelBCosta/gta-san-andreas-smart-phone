#include "ClockApp.h"
#include "../Phone.h"
#include "../providers/IClockProvider.h"
#include "../LocalizationManager.h"
#include <IconsFontAwesome5.h>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

extern Phone phone;

ClockApp::ClockApp() {
    id = "clock";
    icon = ICON_FA_CLOCK;
    name = TR("clock.title");
    color = ImVec4(0.85f, 0.48f, 0.08f, 1.0f);
    installable = true;
    installed = false;
}

const char* ClockApp::getPeriodo(int h) {
    if (h >= 5 && h < 12) return TR("clock.morning");
    else if (h >= 12 && h < 18) return TR("clock.afternoon");
    else if (h >= 18 && h < 21) return TR("clock.evening");
    else return TR("clock.dawn");
}

ImVec4 ClockApp::getPeriodoColor(int h) {
    if (h >= 5 && h < 12) return ImVec4(1.0f, 0.85f, 0.3f, 1.0f);     // Gold/Yellow
    else if (h >= 12 && h < 18) return ImVec4(1.0f, 0.6f, 0.2f, 1.0f);    // Orange
    else if (h >= 18 && h < 21) return ImVec4(0.4f, 0.5f, 0.9f, 1.0f);    // Sky Blue
    else return ImVec4(0.3f, 0.3f, 0.6f, 1.0f);                           // Muted Blue
}

const char* ClockApp::getGameDay(int d) {
    switch (d) {
        case 1: return TR("clock.day.sun");
        case 2: return TR("clock.day.mon");
        case 3: return TR("clock.day.tue");
        case 4: return TR("clock.day.wed");
        case 5: return TR("clock.day.thu");
        case 6: return TR("clock.day.fri");
        case 7: return TR("clock.day.sat");
        default: return "---";
    }
}

void ClockApp::onOpen() {
    // Will initialize target hour to current hour in onDraw once we query the time.
}

void ClockApp::update(float dt) {
    IClockProvider* provider = phone.getClockProvider();
    if (m_state == SkipState::IDLE) {
        m_fadeAlpha = 0.0f;
        return;
    }

    m_stateTimer += dt;

    switch (m_state) {
        case SkipState::INITIATE: {
            phone.close(PhoneAnimMode::SMOOTH);

            if (phone.getScreenProvider()) {
                phone.getScreenProvider()->Fade(true, 2.0f);
            }

            m_fadeAlpha = 0.0f;
            m_state = SkipState::FADING_OUT;
            m_stateTimer = 0.0f;
            break;
        }
        case SkipState::FADING_OUT: {
            m_fadeAlpha = m_stateTimer / 2.0f;
            if (m_fadeAlpha > 1.0f) m_fadeAlpha = 1.0f;

            if (m_stateTimer >= 2.0f) {
                m_state = SkipState::WAITING_DARK;
                m_stateTimer = 0.0f;
            }
            break;
        }
        case SkipState::WAITING_DARK: {
            m_fadeAlpha = 1.0f;
            if (m_stateTimer >= 0.5f) {
                m_state = SkipState::SET_TIME;
                m_stateTimer = 0.0f;
            }
            break;
        }
        case SkipState::SET_TIME: {
            m_fadeAlpha = 1.0f;

            // Adjust game clock
            if (provider) {
                provider->SkipTime(m_hoursToPass);
            }

            if (phone.getScreenProvider()) {
                phone.getScreenProvider()->Fade(false, 2.0f);
            }

            m_state = SkipState::FADING_IN;
            m_stateTimer = 0.0f;
            break;
        }
        case SkipState::FADING_IN: {
            m_fadeAlpha = 1.0f - (m_stateTimer / 2.0f);
            if (m_fadeAlpha < 0.0f) m_fadeAlpha = 0.0f;

            if (m_stateTimer >= 2.0f) {
                m_state = SkipState::FINISHED;
                m_stateTimer = 0.0f;
            }
            break;
        }
        case SkipState::FINISHED: {
            m_fadeAlpha = 0.0f;
            m_isSkipping = false;
            m_state = SkipState::IDLE;
            break;
        }
        default:
            break;
    }
}

void ClockApp::onDraw() {
    IClockProvider* provider = phone.getClockProvider();
    if (!provider) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), TR("clock.no_provider"));
        return;
    }

    PhoneTime timeVal = provider->GetTime();
    int h = timeVal.hours;
    int m = timeVal.minutes;
    int s = timeVal.seconds;
    int dayOfWeek = timeVal.dayOfWeek;

    // Initialize target hour to current hour once on open
    static int lastApp = -1;
    if (lastApp != h && !m_isSkipping) {
        m_skipTargetHour = h;
        lastApp = h;
    }

    float availW = ImGui::GetContentRegionAvail().x;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // ---- 1. Analog Clock Face ----
    ImVec2 cursorSp = ImGui::GetCursorScreenPos();
    float clockRadius = 70.0f;
    ImVec2 clockCenter = ImVec2(cursorSp.x + availW / 2.0f, cursorSp.y + clockRadius + 5.0f);
    
    // Reserve space for the analog clock
    ImGui::Dummy(ImVec2(availW, clockRadius * 2.0f + 10.0f));

    // Draw Clock Background
    drawList->AddCircleFilled(clockCenter, clockRadius, IM_COL32(24, 24, 30, 240), 64);
    drawList->AddCircle(clockCenter, clockRadius, IM_COL32(255, 255, 255, 30), 64, 2.0f);
    drawList->AddCircle(clockCenter, clockRadius + 4.0f, IM_COL32(217, 123, 20, 40), 64, 1.5f); // Orange glow outline

    // Draw Hour Ticks (12 hours)
    for (int i = 0; i < 12; ++i) {
        float angle = i * (2.0f * M_PI / 12.0f) - (M_PI / 2.0f);
        ImVec2 d(cosf(angle), sinf(angle));
        if (i % 3 == 0) {
            ImVec2 pInner(clockCenter.x + d.x * (clockRadius - 8.0f), clockCenter.y + d.y * (clockRadius - 8.0f));
            ImVec2 pOuter(clockCenter.x + d.x * (clockRadius - 2.0f), clockCenter.y + d.y * (clockRadius - 2.0f));
            drawList->AddLine(pInner, pOuter, IM_COL32(255, 255, 255, 220), 2.5f);
        } else {
            ImVec2 pInner(clockCenter.x + d.x * (clockRadius - 5.0f), clockCenter.y + d.y * (clockRadius - 5.0f));
            ImVec2 pOuter(clockCenter.x + d.x * (clockRadius - 2.0f), clockCenter.y + d.y * (clockRadius - 2.0f));
            drawList->AddLine(pInner, pOuter, IM_COL32(255, 255, 255, 120), 1.5f);
        }
    }

    // Calculate hand angles
    float hourAngle = ((h % 12) + m / 60.0f) * (2.0f * M_PI / 12.0f) - (M_PI / 2.0f);
    float minAngle = m * (2.0f * M_PI / 60.0f) - (M_PI / 2.0f);

    // Draw Hour Hand
    ImVec2 hourDir(cosf(hourAngle), sinf(hourAngle));
    ImVec2 hourEnd(clockCenter.x + hourDir.x * (clockRadius * 0.45f), clockCenter.y + hourDir.y * (clockRadius * 0.45f));
    drawList->AddLine(clockCenter, hourEnd, IM_COL32(255, 255, 255, 255), 4.0f);

    // Draw Minute Hand
    ImVec2 minDir(cosf(minAngle), sinf(minAngle));
    ImVec2 minEnd(clockCenter.x + minDir.x * (clockRadius * 0.70f), clockCenter.y + minDir.y * (clockRadius * 0.70f));
    drawList->AddLine(clockCenter, minEnd, IM_COL32(255, 255, 255, 200), 2.5f);

    // Draw Center Cap (Silver pin)
    drawList->AddCircleFilled(clockCenter, 5.0f, IM_COL32(255, 255, 255, 255), 16);
    drawList->AddCircleFilled(clockCenter, 2.0f, IM_COL32(24, 24, 30, 255), 16);

    // ---- 2. Time Info Text ----
    ImGui::Spacing();

    // Day of Week
    const char* dayName = getGameDay(dayOfWeek);
    float dw = ImGui::CalcTextSize(dayName).x;
    ImGui::SetCursorPosX((availW - dw) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", dayName);

    // Digital Clock Time
    char timeStr[32];
    sprintf_s(timeStr, sizeof(timeStr), "%02d:%02d", h, m);
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.8f);
    float tw = ImGui::CalcTextSize(timeStr).x;
    ImGui::SetCursorPosX((availW - tw) / 2.0f);
    ImGui::Text("%s", timeStr);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    // Day Period
    const char* period = getPeriodo(h);
    ImVec4 periodColor = getPeriodoColor(h);
    float pw = ImGui::CalcTextSize(period).x;
    ImGui::SetCursorPosX((availW - pw) / 2.0f);
    ImGui::TextColored(periodColor, "%s", period);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- 3. Rest UI ----
    bool allowedToSkip = provider->CanSkipTime();

    if (!allowedToSkip) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), TR("clock.cant_rest"));
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), TR("clock.rest_until"));
        ImGui::Spacing();

        // Hour Slider
        ImGui::PushItemWidth(availW - 100.0f);
        ImGui::SliderInt("##hora", &m_skipTargetHour, 0, 23);
        ImGui::PopItemWidth();

        ImGui::SameLine();

        // Target period preview
        const char* targetPeriod = getPeriodo(m_skipTargetHour);
        ImVec4 targetPeriodCol = getPeriodoColor(m_skipTargetHour);
        ImGui::TextColored(targetPeriodCol, " %s", targetPeriod);

        ImGui::Spacing();

        bool canSkip = (m_skipTargetHour != h) && !m_isSkipping;

        // Rest Button
        if (!canSkip) {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.85f, 0.48f, 0.08f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.58f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.75f, 0.38f, 0.00f, 1.0f));
        }

        if (ImGui::Button(TR("clock.rest"), ImVec2(-1.0f, 35.0f))) {
            if (canSkip) {
                m_isSkipping = true;
                m_state = SkipState::INITIATE;
                m_stateTimer = 0.0f;
                
                // Calculate hours to pass
                if (m_skipTargetHour > h) {
                    m_hoursToPass = m_skipTargetHour - h;
                } else {
                    m_hoursToPass = (24 - h) + m_skipTargetHour;
                }
            }
        }
        ImGui::PopStyleColor(3);
    }
}
