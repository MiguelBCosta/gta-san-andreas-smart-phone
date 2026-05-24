#pragma once

class IScreenProvider {
public:
    virtual ~IScreenProvider() = default;
    virtual void Fade(bool fadeOut, float duration) = 0;
    virtual float GetFadeAlpha() const = 0;
    virtual bool IsFading() const = 0;
    virtual void Update(float dt) = 0;
};
