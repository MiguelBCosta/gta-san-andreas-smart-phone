#pragma once
#include "../PhoneApp.h"
#include "../providers/ICameraProvider.h"

class CameraApp : public PhoneApp {
private:
    ICameraProvider* m_provider = nullptr;
    float m_flashAlpha = 0.0f; // Screenshot flash animation timer
    bool m_hudVisible = true; // Toggle grid/HUD visualization (GTA V style)

public:
    CameraApp();
    void SetCameraProvider(ICameraProvider* provider) { m_provider = provider; }

    bool requiresMouse() const override;
    bool hasSolidBackground() const override { return false; }
    bool hidePhoneChassis() const override;

    void onOpen() override;
    void onClose() override;
    void onDraw() override;
    void update(float dt) override;
};
