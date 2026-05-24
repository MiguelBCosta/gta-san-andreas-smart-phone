#pragma once
#include "../../core/providers/IScreenProvider.h"

class SandboxScreenProvider : public IScreenProvider {
private:
    bool m_isFading = false;
    bool m_fadeOut = false;
    float m_fadeDuration = 0.0f;
    float m_fadeTimer = 0.0f;

public:
    void Fade(bool fadeOut, float duration) override {
        m_isFading = true;
        m_fadeOut = fadeOut;
        m_fadeDuration = duration;
        m_fadeTimer = 0.0f;
    }

    float GetFadeAlpha() const override {
        if (m_fadeDuration <= 0.0f) return 0.0f;
        if (m_fadeOut) {
            // Fade out: 0.0 -> 1.0
            float val = m_fadeTimer / m_fadeDuration;
            return val > 1.0f ? 1.0f : val;
        } else {
            // Fade in: 1.0 -> 0.0
            float val = 1.0f - (m_fadeTimer / m_fadeDuration);
            return val < 0.0f ? 0.0f : val;
        }
    }

    bool IsFading() const override {
        return m_isFading || GetFadeAlpha() > 0.0f;
    }

    void Update(float dt) override {
        if (m_isFading) {
            m_fadeTimer += dt;
            if (m_fadeTimer >= m_fadeDuration) {
                m_fadeTimer = m_fadeDuration;
                m_isFading = false;
            }
        }
    }
};
