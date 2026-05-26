#pragma once
#include "../../core/providers/ICameraProvider.h"
#include <iostream>

class SandboxCameraProvider : public ICameraProvider {
private:
    bool m_active = false;
    bool m_selfie = false;
    float m_captureTimer = 0.0f;
    float m_zoom = 1.6f;

public:
    bool IsActive() const override { return m_active; }
    void SetActive(bool active) override {
        m_active = active;
        std::cout << "[SandboxCamera] Camera mode: " << (active ? "ACTIVE" : "INACTIVE") << std::endl;
    }
    
    void SetSelfieMode(bool selfie) override {
        m_selfie = selfie;
        std::cout << "[SandboxCamera] Selfie mode: " << (selfie ? "ON" : "OFF") << std::endl;
    }
    bool IsSelfieMode() const override { return m_selfie; }
    
    bool TakePhoto() override {
        std::cout << "[SandboxCamera] PHOTO TAKEN! (" << (m_selfie ? "Selfie" : "Normal") << ")" << std::endl;
        m_captureTimer = 0.15f; // Hide HUD for 150ms
        return true;
    }
    
    bool IsCapturing() const override { return m_captureTimer > 0.0f; }
    
    void AdjustZoom(float delta) override {
        m_zoom -= delta * 0.2f;
        if (m_zoom < 1.0f) m_zoom = 1.0f;
        if (m_zoom > 3.0f) m_zoom = 3.0f;
        std::cout << "[SandboxCamera] Zoom adjusted: " << m_zoom << std::endl;
    }
    
    bool RequiresMouse() const override { return true; } // Sandbox needs mouse interaction
    bool IsSandbox() const override { return true; }
    
    void Update(float dt) override {
        if (m_captureTimer > 0.0f) {
            m_captureTimer -= dt;
        }
    }
};
