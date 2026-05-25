#pragma once
#include "../../core/providers/IGarageProvider.h"
#include <vector>
#include <string>

// Forward declarations of GTA types to keep compilation fast
class CVehicle;
class CPed;

class GtaGarageProvider : public IGarageProvider {
private:
    struct ActiveDelivery {
        int deliveryId;
        CVehicle* car;
        CPed* driver;
        int blip;
        float targetX;
        float targetY;
        float targetZ;
        std::string state; // "delivering", "leaving", "delivered"
    };

    struct PendingSpawn {
        int deliveryId;
        VehicleInfo info;
        float spawnX;
        float spawnY;
        float spawnZ;
        float spawnHeading;
    };

    struct SpawnedCar {
        int deliveryId;
        CVehicle* car;
    };

    int m_deliveryIdCounter = 0;
    std::vector<ActiveDelivery> m_activeDeliveries;
    std::vector<PendingSpawn> m_pendingSpawns;
    std::vector<SpawnedCar> m_spawnedCars;

    bool IsVehicleValid(CVehicle* vehicle);
    bool IsPedValid(CPed* ped);
    bool IsModelIdValid(int modelId);
    bool FindSafeSpawnLocation(float minDist, float maxDist, float& outX, float& outY, float& outZ, float& outHeading);

public:
    GtaGarageProvider() = default;
    ~GtaGarageProvider() override = default;

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
