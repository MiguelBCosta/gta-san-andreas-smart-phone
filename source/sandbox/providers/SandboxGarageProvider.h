#pragma once
#include "../../core/providers/IGarageProvider.h"
#include <vector>
#include <string>

class SandboxGarageProvider : public IGarageProvider {
private:
    struct SimulatedDelivery {
        int id;
        float distance;
        bool delivered;
    };

    bool m_isInside = false;
    bool m_isOnMission = false;
    bool m_isPlayerInAnyCar = false;
    VehicleInfo m_mockCurrentVehicle;

    int m_deliveryIdCounter = 0;
    std::vector<SimulatedDelivery> m_deliveries;
    std::string m_lastMessage = "";

public:
    SandboxGarageProvider();
    ~SandboxGarageProvider() override = default;

    // Setters/Getters for Simulator Control Panel
    void SetInside(bool val);
    bool GetInside() const;
    void SetOnMission(bool val);
    bool GetOnMission() const;
    void SetPlayerInAnyCar(bool val);
    bool GetPlayerInAnyCar() const;
    VehicleInfo& GetMockCurrentVehicle();
    const std::string& GetLastMessage() const;
    void ClearLastMessage();
    std::vector<SimulatedDelivery>& GetDeliveries();

    // IGarageProvider Implementation
    bool IsInside() override;
    bool IsOnMission() override;
    bool IsPlayerInAnyCar() override;
    bool GetCurrentVehicleInfo(VehicleInfo& outInfo) override;
    int TriggerDelivery(const VehicleInfo& info) override;
    unsigned int GetSpawnedVehicleHandle(int deliveryId) override;
    bool IsVehicleNearPlayer(unsigned int handle) override;
    bool IsPlayerInVehicleModel(int modelId) override;
    void Update(float dt) override;
    void ShowMessage(const std::string& message, int durationMs) override;
};
