#pragma once
#include "../../core/providers/ICameraProvider.h"
#include <game_sa/CPlayerPed.h>
#include <game_sa/CPad.h>

class GtaCameraProvider : public ICameraProvider {
private:
    bool m_active = false;
    bool m_selfie = false;
    int m_originalWeapon = -1;
    bool m_gaveCamera = false;
    int m_captureFrames = 0;
    float m_selfieZoom = 1.6f;
    float m_selfieAngleX = 0.0f;
    float m_selfieAngleY = 0.0f;

public:
    bool IsActive() const override { return m_active; }
    void SetActive(bool active) override;
    void SetSelfieMode(bool selfie) override;
    bool IsSelfieMode() const override { return m_selfie; }
    
    bool TakePhoto() override;
    bool IsCapturing() const override { return m_captureFrames > 0; }
    void AdjustZoom(float delta) override;
    
    bool RequiresMouse() const override { return false; }
    bool IsSandbox() const override { return false; }
    
    void Update(float dt) override;
};
