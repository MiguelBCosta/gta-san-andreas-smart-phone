#include "SandboxGarageProvider.h"
#include <iostream>

SandboxGarageProvider::SandboxGarageProvider() {
    // Setup a default mock vehicle
    m_mockCurrentVehicle.name = "Infernus (Simulado)";
    m_mockCurrentVehicle.model = 411;
    m_mockCurrentVehicle.color1 = 1;
    m_mockCurrentVehicle.color2 = 3;
    m_mockCurrentVehicle.paintjob = -1;
    m_mockCurrentVehicle.hydraulics = true;
    m_mockCurrentVehicle.mods = { 1087, 1010 }; // Hydraulics, Nitro
}

void SandboxGarageProvider::SetInside(bool val) {
    m_isInside = val;
}

bool SandboxGarageProvider::GetInside() const {
    return m_isInside;
}

void SandboxGarageProvider::SetOnMission(bool val) {
    m_isOnMission = val;
}

bool SandboxGarageProvider::GetOnMission() const {
    return m_isOnMission;
}

void SandboxGarageProvider::SetPlayerInAnyCar(bool val) {
    m_isPlayerInAnyCar = val;
}

bool SandboxGarageProvider::GetPlayerInAnyCar() const {
    return m_isPlayerInAnyCar;
}

VehicleInfo& SandboxGarageProvider::GetMockCurrentVehicle() {
    return m_mockCurrentVehicle;
}

const std::string& SandboxGarageProvider::GetLastMessage() const {
    return m_lastMessage;
}

void SandboxGarageProvider::ClearLastMessage() {
    m_lastMessage.clear();
}

std::vector<SandboxGarageProvider::SimulatedDelivery>& SandboxGarageProvider::GetDeliveries() {
    return m_deliveries;
}

bool SandboxGarageProvider::IsInside() {
    return m_isInside;
}

bool SandboxGarageProvider::IsOnMission() {
    return m_isOnMission;
}

bool SandboxGarageProvider::IsPlayerInAnyCar() {
    return m_isPlayerInAnyCar;
}

bool SandboxGarageProvider::GetCurrentVehicleInfo(VehicleInfo& outInfo) {
    if (!m_isPlayerInAnyCar) return false;
    outInfo = m_mockCurrentVehicle;
    return true;
}

int SandboxGarageProvider::TriggerDelivery(const VehicleInfo& info) {
    m_deliveryIdCounter++;
    SimulatedDelivery del;
    del.id = m_deliveryIdCounter;
    del.distance = 100.0f; // starts 100m away
    del.delivered = false;
    m_deliveries.push_back(del);

    ShowMessage("[Sim] O veiculo " + info.name + " esta a caminho!", 3000);
    return del.id;
}

unsigned int SandboxGarageProvider::GetSpawnedVehicleHandle(int deliveryId) {
    for (const auto& del : m_deliveries) {
        if (del.id == deliveryId) {
            // Return a dummy handle matching the delivery ID
            return (unsigned int)(1000 + del.id);
        }
    }
    return 0;
}

bool SandboxGarageProvider::IsVehicleNearPlayer(unsigned int handle) {
    int delId = (int)handle - 1000;
    for (const auto& del : m_deliveries) {
        if (del.id == delId) {
            return del.distance < 10.0f; // Considered near if within 10 meters in simulation
        }
    }
    return false;
}

bool SandboxGarageProvider::IsPlayerInVehicleModel(int modelId) {
    return m_isPlayerInAnyCar && (m_mockCurrentVehicle.model == modelId);
}

void SandboxGarageProvider::Update(float dt) {
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
        it++;
    }
}

void SandboxGarageProvider::ShowMessage(const std::string& message, int durationMs) {
    m_lastMessage = message;
    std::cout << "[Garage App Message] " << message << std::endl;
}
