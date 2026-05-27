#pragma once
#include <string>
#include <vector>

struct VehicleInfo {
    std::string name;
    int model = 0;
    int color1 = 0;
    int color2 = 0;
    int paintjob = -1;
    std::vector<int> mods;
    bool hydraulics = false;
    std::string plate = "";
};

class IGarageProvider {
public:
    virtual ~IGarageProvider() = default;

    virtual bool IsInside() = 0;
    virtual bool IsOnMission() = 0;
    virtual bool IsPlayerInAnyCar() = 0;
    virtual bool GetCurrentVehicleInfo(VehicleInfo& outInfo) = 0;
    virtual int TriggerDelivery(const VehicleInfo& info) = 0;
    virtual unsigned int GetSpawnedVehicleHandle(int deliveryId) = 0;
    virtual bool IsVehicleNearPlayer(unsigned int handle) = 0;
    virtual bool IsPlayerInVehicleModel(int modelId) = 0;
    virtual void Update(float dt) = 0;
    virtual void ShowMessage(const std::string& message, int durationMs) {}
};

