#pragma once
#include "../../core/providers/IGarageProvider.h"
#include <vector>
#include <string>
#include <iostream>

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
    SandboxGarageProvider() {
        // Setup a default mock vehicle
        m_mockCurrentVehicle.name = "Infernus (Simulado)";
        m_mockCurrentVehicle.model = 411;
        m_mockCurrentVehicle.color1 = 1;
        m_mockCurrentVehicle.color2 = 3;
        m_mockCurrentVehicle.paintjob = -1;
        m_mockCurrentVehicle.hydraulics = true;
        m_mockCurrentVehicle.mods = { 1087, 1010 }; // Hydraulics, Nitro
    }

    // Setters for Simulator Control Panel
    void SetInside(bool val) { m_isInside = val; }
    bool GetInside() const { return m_isInside; }

    void SetOnMission(bool val) { m_isOnMission = val; }
    bool GetOnMission() const { return m_isOnMission; }

    void SetPlayerInAnyCar(bool val) { m_isPlayerInAnyCar = val; }
    bool GetPlayerInAnyCar() const { return m_isPlayerInAnyCar; }

    VehicleInfo& GetMockCurrentVehicle() { return m_mockCurrentVehicle; }
    const std::string& GetLastMessage() const { return m_lastMessage; }
    void ClearLastMessage() { m_lastMessage.clear(); }

    std::vector<SimulatedDelivery>& GetDeliveries() { return m_deliveries; }

    // IGarageProvider Implementation
    bool IsInside() override { return m_isInside; }
    bool IsOnMission() override { return m_isOnMission; }
    bool IsPlayerInAnyCar() override { return m_isPlayerInAnyCar; }

    bool GetCurrentVehicleInfo(VehicleInfo& outInfo) override {
        if (!m_isPlayerInAnyCar) return false;
        outInfo = m_mockCurrentVehicle;
        return true;
    }

    int TriggerDelivery(const VehicleInfo& info) override {
        m_deliveryIdCounter++;
        SimulatedDelivery del;
        del.id = m_deliveryIdCounter;
        del.distance = 100.0f; // starts 100m away
        del.delivered = false;
        m_deliveries.push_back(del);

        ShowMessage("[Sim] O veiculo " + info.name + " esta a caminho!", 3000);
        return del.id;
    }

    unsigned int GetSpawnedVehicleHandle(int deliveryId) override {
        for (const auto& del : m_deliveries) {
            if (del.id == deliveryId) {
                // Return a dummy handle matching the delivery ID
                return (unsigned int)(1000 + del.id);
            }
        }
        return 0;
    }

    bool IsVehicleNearPlayer(unsigned int handle) override {
        int delId = (int)handle - 1000;
        for (const auto& del : m_deliveries) {
            if (del.id == delId) {
                return del.distance < 10.0f; // Considered near if within 10 meters in simulation
            }
        }
        return false;
    }

    bool IsPlayerInVehicleModel(int modelId) override {
        return m_isPlayerInAnyCar && (m_mockCurrentVehicle.model == modelId);
    }

    void Update(float dt) override {
        // Tick deliveries
        for (auto it = m_deliveries.begin(); it != m_deliveries.end(); ) {
            if (!it->delivered) {
                it->distance -= 15.0f * dt; // drives at 15m/s
                if (it->distance <= 0.0f) {
                    it->distance = 0.0f;
                    it->delivered = true;
                    ShowMessage("[Sim] Motorista entregou o veiculo e foi embora!", 3000);
                }
            }
            
            // In simulation, if player "gets in", the delivery is cleaned up
            // We can let the user click a mock "Get in delivered car" in the control panel
            it++;
        }
    }

    void ShowMessage(const std::string& message, int durationMs) override {
        m_lastMessage = message;
        std::cout << "[Garage App Message] " << message << std::endl;
    }
};
