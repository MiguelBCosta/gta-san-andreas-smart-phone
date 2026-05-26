#pragma once

class ICameraProvider {
public:
    virtual ~ICameraProvider() = default;
    
    virtual bool IsActive() const = 0;
    virtual void SetActive(bool active) = 0;
    
    virtual void SetSelfieMode(bool selfie) = 0;
    virtual bool IsSelfieMode() const = 0;
    
    virtual bool TakePhoto() = 0;
    virtual bool IsCapturing() const = 0;
    virtual void AdjustZoom(float delta) = 0;
    
    virtual bool RequiresMouse() const = 0;
    virtual bool IsSandbox() const = 0;
    
    virtual void Update(float dt) = 0;
};
