#pragma once
#include "../../core/providers/IScreenProvider.h"
#include <game_sa/CCamera.h>

class GtaScreenProvider : public IScreenProvider {
public:
    void Fade(bool fadeOut, float duration) override {
        TheCamera.Fade(duration, fadeOut ? 0 : 1);
    }
    float GetFadeAlpha() const override { return 0.0f; }
    bool IsFading() const override { return false; }
    void Update(float dt) override {}
};
