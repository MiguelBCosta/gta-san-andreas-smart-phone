#pragma once
#include "../../core/providers/IGarageProvider.h"
#include <game_sa/CTheScripts.h>
#include <game_sa/CModelInfo.h>
#include <game_sa/CStreaming.h>
#include <game_sa/CMessages.h>
#include <game_sa/CText.h>
#include <game_sa/CWorld.h>
#include <game_sa/CVehicle.h>
#include <game_sa/CPed.h>
#include <game_sa/CPools.h>
#include <game_sa/common.h>
#include <game_sa/CGeneral.h>
#include <game_sa/enums/eScriptCommands.h>
#include <extensions/ScriptCommands.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

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

    bool IsVehicleValid(CVehicle* vehicle) {
        if (!vehicle) return false;
        if (!CPools::ms_pVehiclePool) return false;
        return CPools::ms_pVehiclePool->IsObjectValid(vehicle);
    }

    bool IsPedValid(CPed* ped) {
        if (!ped) return false;
        if (!CPools::ms_pPedPool) return false;
        return CPools::ms_pPedPool->IsObjectValid(ped);
    }

    bool IsModelIdValid(int modelId) {
        if (modelId < 0) return false;
        return CModelInfo::GetModelInfo(modelId) != nullptr;
    }

    bool FindSafeSpawnLocation(float minDist, float maxDist, float& outX, float& outY, float& outZ, float& outHeading) {
        CPlayerPed* player = FindPlayerPed(-1);
        if (!player) return false;
        CVector playerPos = player->GetPosition();

        for (int i = 0; i < 50; i++) {
            float randAngle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
            float randDist = minDist + ((float)rand() / RAND_MAX) * (maxDist - minDist);
            float targetX = playerPos.x + cos(randAngle) * randDist;
            float targetY = playerPos.y + sin(randAngle) * randDist;

            float roadX = 0.0f, roadY = 0.0f, roadZ = 0.0f, roadHeading = 0.0f;
            bool hasNode = plugin::Command<COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING>(targetX, targetY, playerPos.z, &roadX, &roadY, &roadZ, &roadHeading);

            if (hasNode) {
                float distToRoad = (playerPos - CVector(roadX, roadY, roadZ)).Magnitude();
                if (distToRoad >= minDist && distToRoad <= (maxDist + 15.0f)) {
                    if (!plugin::Command<COMMAND_IS_POINT_ON_SCREEN>(roadX, roadY, roadZ, 5.0f)) {
                        float angleToTarget = CGeneral::GetATanOfXY(playerPos.x - roadX, playerPos.y - roadY);
                        
                        float diff = fmodf(angleToTarget - roadHeading, 360.0f);
                        if (diff < 0.0f) diff += 360.0f;

                        float alignedHeading = roadHeading;
                        if (diff > 90.0f && diff < 270.0f) {
                            alignedHeading = fmod(roadHeading + 180.0f, 360.0f);
                        }

                        outX = roadX;
                        outY = roadY;
                        outZ = roadZ;
                        outHeading = alignedHeading;
                        return true;
                    }
                }
            }
        }
        return false;
    }

public:
    GtaGarageProvider() = default;
    ~GtaGarageProvider() override = default;

    bool IsInside() override {
        int interior = 0;
        plugin::Command<COMMAND_GET_AREA_VISIBLE>(&interior);
        return interior > 0;
    }

    bool IsOnMission() override {
        return CTheScripts::IsPlayerOnAMission();
    }

    bool IsPlayerInAnyCar() override {
        CPlayerPed* player = FindPlayerPed(-1);
        if (!player) return false;
        return player->bInVehicle && player->m_pVehicle != nullptr;
    }

    bool GetCurrentVehicleInfo(VehicleInfo& outInfo) override {
        CPlayerPed* player = FindPlayerPed(-1);
        if (!player || !player->bInVehicle || !player->m_pVehicle) return false;
        
        CVehicle* car = player->m_pVehicle;
        int modelId = car->m_nModelIndex;
        
        // Exclude planes, helis, boats, trains, trailers
        if (CModelInfo::IsPlaneModel(modelId) || 
            CModelInfo::IsHeliModel(modelId) || 
            CModelInfo::IsBoatModel(modelId) || 
            CModelInfo::IsTrainModel(modelId) ||
            CModelInfo::IsTrailerModel(modelId)) {
            return false;
        }
        
        outInfo.model = modelId;
        outInfo.color1 = car->m_nPrimaryColor;
        outInfo.color2 = car->m_nSecondaryColor;
        
        // Read paintjob using CVehicle member function
        int pj = car->GetRemapIndex();
        if (pj >= 0 && pj <= 2) {
            outInfo.paintjob = pj;
        } else {
            outInfo.paintjob = -1;
        }
        
        // Read mods/upgrades from CVehicle array
        outInfo.mods.clear();
        for (int i = 0; i < 15; i++) {
            int mod = car->m_anUpgrades[i];
            if (mod >= 1000 && mod <= 1193) {
                outInfo.mods.push_back(mod);
            }
        }
        
        // Hydraulics check
        outInfo.hydraulics = car->m_nHandlingFlags.bHydraulicInst || (car->GetUpgrade(1087) != -1);
        
        // Retrieve vehicle game name from GXT
        std::string name = "Veiculo " + std::to_string(modelId);
        CBaseModelInfo* modelInfo = CModelInfo::GetModelInfo(modelId);
        if (modelInfo) {
            CVehicleModelInfo* vehicleModelInfo = (CVehicleModelInfo*)modelInfo;
            const char* gxtKey = vehicleModelInfo->m_szGameName;
            const char* gxtName = TheText.Get(gxtKey);
            if (gxtName && gxtName[0] != '\0') {
                name = gxtName;
            } else if (gxtKey && gxtKey[0] != '\0') {
                name = gxtKey;
            }
        }
        outInfo.name = name;
        return true;
    }

    int TriggerDelivery(const VehicleInfo& info) override {
        float spawnX = 0.0f, spawnY = 0.0f, spawnZ = 0.0f, spawnHeading = 0.0f;
        if (!FindSafeSpawnLocation(40.0f, 80.0f, spawnX, spawnY, spawnZ, spawnHeading)) {
            return -1;
        }

        m_deliveryIdCounter++;

        PendingSpawn spawn;
        spawn.deliveryId = m_deliveryIdCounter;
        spawn.info = info;
        spawn.spawnX = spawnX;
        spawn.spawnY = spawnY;
        spawn.spawnZ = spawnZ;
        spawn.spawnHeading = spawnHeading;

        // Request models
        CStreaming::RequestModel(info.model, 2);
        CStreaming::RequestModel(105, 2); // Grove driver model

        // Request vehicle upgrade/tuning mod models
        for (int mod : info.mods) {
            if (IsModelIdValid(mod)) {
                CStreaming::RequestModel(mod, 2);
            }
        }
        if (info.hydraulics && IsModelIdValid(1087)) {
            CStreaming::RequestModel(1087, 2);
        }

        m_pendingSpawns.push_back(spawn);
        return spawn.deliveryId;
    }

    unsigned int GetSpawnedVehicleHandle(int deliveryId) override {
        for (auto it = m_spawnedCars.begin(); it != m_spawnedCars.end(); ) {
            if (!IsVehicleValid(it->car)) {
                it = m_spawnedCars.erase(it);
            } else {
                if (it->deliveryId == deliveryId) {
                    return (unsigned int)it->car;
                }
                it++;
            }
        }
        return 0;
    }

    bool IsVehicleNearPlayer(unsigned int handle) override {
        CVehicle* car = (CVehicle*)handle;
        if (!IsVehicleValid(car) || car->m_fHealth <= 0.0f) {
            return false;
        }
        CPlayerPed* player = FindPlayerPed(-1);
        if (!player) return false;
        
        CVector playerPos = player->GetPosition();
        CVector carPos = car->GetPosition();
        float dist = (playerPos - carPos).Magnitude();
        return dist < 100.0f;
    }

    bool IsPlayerInVehicleModel(int modelId) override {
        CPlayerPed* player = FindPlayerPed(-1);
        if (!player || !player->bInVehicle || !player->m_pVehicle) return false;
        return player->m_pVehicle->m_nModelIndex == modelId;
    }

    void Update(float dt) override {
        // 1. Process Pending Spawns
        for (auto it = m_pendingSpawns.begin(); it != m_pendingSpawns.end(); ) {
            // Check if vehicle, driver, and all vehicle mods are fully loaded in memory
            bool allLoaded = CStreaming::HasModelLoaded(it->info.model) && CStreaming::HasModelLoaded(105);
            if (allLoaded) {
                for (int mod : it->info.mods) {
                    if (IsModelIdValid(mod) && !CStreaming::HasModelLoaded(mod)) {
                        allLoaded = false;
                        break;
                    }
                }
            }
            if (allLoaded && it->info.hydraulics && IsModelIdValid(1087)) {
                if (!CStreaming::HasModelLoaded(1087)) {
                    allLoaded = false;
                }
            }

            if (allLoaded) {
                CVehicle* car = nullptr;
                plugin::Command<COMMAND_CREATE_CAR>(it->info.model, it->spawnX, it->spawnY, it->spawnZ, &car);
                
                if (car) {
                    plugin::Command<COMMAND_SET_CAR_HEADING>(car, it->spawnHeading);
                    
                    // Clear random upgrades added by the game safely using native/script methods
                    for (int i = 0; i < 15; i++) {
                        int mod = car->m_anUpgrades[i];
                        if (mod >= 1000 && mod <= 1193 && IsModelIdValid(mod)) {
                            plugin::Command<COMMAND_REMOVE_VEHICLE_MOD>(car, mod);
                        }
                    }
                    car->m_nHandlingFlags.bHydraulicInst = false;
                    car->m_nHandlingFlags.bNosInst = false;
                    car->RemoveAllUpgrades();

                    
                    // Apply saved colors
                    car->m_nPrimaryColor = it->info.color1;
                    car->m_nSecondaryColor = it->info.color2;
                    
                    // Apply paintjob
                    if (it->info.paintjob >= 0 && it->info.paintjob <= 2) {
                        car->SetRemap(it->info.paintjob);
                    }
                    
                    // Apply saved upgrades (guaranteed loaded)
                    for (int mod : it->info.mods) {
                        if (IsModelIdValid(mod)) {
                            car->AddVehicleUpgrade(mod);
                            CStreaming::SetModelIsDeletable(mod);
                        }
                    }
                    
                    // Apply hydraulics (guaranteed loaded)
                    if (it->info.hydraulics && IsModelIdValid(1087)) {
                        car->AddVehicleUpgrade(1087);
                        CStreaming::SetModelIsDeletable(1087);
                    }
                    
                    // Spawn driver inside car
                    CPed* driver = nullptr;
                    plugin::Command<COMMAND_CREATE_CHAR_INSIDE_CAR>(car, 8, 105, &driver);
                    
                    // Set models as no longer needed
                    CStreaming::SetModelIsDeletable(it->info.model);
                    CStreaming::SetModelIsDeletable(105);
                    
                    // Add green blip
                    int blip = 0;
                    plugin::Command<COMMAND_ADD_BLIP_FOR_CAR>(car, &blip);
                    plugin::Command<COMMAND_CHANGE_BLIP_COLOUR>(blip, 1);
                    
                    // Get closest node near player to drive to
                    CPlayerPed* player = FindPlayerPed(-1);
                    float targetX = player->GetPosition().x;
                    float targetY = player->GetPosition().y;
                    float targetZ = player->GetPosition().z;
                    float targetHeading = 0.0f;
                    plugin::Command<COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING>(targetX, targetY, targetZ, &targetX, &targetY, &targetZ, &targetHeading);
                    
                    // Drive task
                    plugin::Command<COMMAND_TASK_CAR_DRIVE_TO_COORD>(driver, car, targetX, targetY, targetZ, 15.0f, 0, it->info.model, 1);
                    
                    // Save to active deliveries
                    ActiveDelivery ad;
                    ad.deliveryId = it->deliveryId;
                    ad.car = car;
                    ad.driver = driver;
                    ad.blip = blip;
                    ad.targetX = targetX;
                    ad.targetY = targetY;
                    ad.targetZ = targetZ;
                    ad.state = "delivering";
                    
                    m_activeDeliveries.push_back(ad);

                    // Track spawned car
                    SpawnedCar sc;
                    sc.deliveryId = it->deliveryId;
                    sc.car = car;
                    m_spawnedCars.push_back(sc);
                }
                it = m_pendingSpawns.erase(it);
            } else {
                // Keep requesting models
                CStreaming::RequestModel(it->info.model, 2);
                CStreaming::RequestModel(105, 2);
                for (int mod : it->info.mods) {
                    if (IsModelIdValid(mod)) {
                        CStreaming::RequestModel(mod, 2);
                    }
                }
                if (it->info.hydraulics && IsModelIdValid(1087)) {
                    CStreaming::RequestModel(1087, 2);
                }
                it++;
            }
        }
        
        // 2. Process Active Deliveries
        CPlayerPed* player = FindPlayerPed(-1);
        if (!player) return;
        CVector playerPos = player->GetPosition();

        for (auto it = m_activeDeliveries.begin(); it != m_activeDeliveries.end(); ) {
            bool isCarDestroyed = !IsVehicleValid(it->car) || it->car->m_fHealth <= 0.0f;
            bool isDriverKilled = !IsPedValid(it->driver) || it->driver->m_fHealth <= 0.0f;
            
            bool playerTookOver = false;
            if (!isCarDestroyed) {
                playerTookOver = (it->car->m_pDriver == player);
            }
            
            bool isDeliveryFinished = (it->state == "delivered");
            bool shouldCleanUp = isCarDestroyed || isDriverKilled || playerTookOver || isDeliveryFinished;
            
            if (shouldCleanUp) {
                if (plugin::Command<COMMAND_DOES_BLIP_EXIST>(it->blip)) {
                    plugin::Command<COMMAND_REMOVE_BLIP>(it->blip);
                }
                if (IsVehicleValid(it->car)) {
                    CVehicle* carPtr = it->car;
                    plugin::Command<COMMAND_MARK_CAR_AS_NO_LONGER_NEEDED>(&carPtr);
                }
                if (IsPedValid(it->driver)) {
                    CPed* driverPtr = it->driver;
                    plugin::Command<COMMAND_MARK_CHAR_AS_NO_LONGER_NEEDED>(&driverPtr);
                }
                it = m_activeDeliveries.erase(it);
            } else {
                CVector carPos = it->car->GetPosition();
                if (it->state == "delivering") {
                    float distToTarget = sqrtf(powf(carPos.x - it->targetX, 2.0f) + powf(carPos.y - it->targetY, 2.0f));
                    float distToPlayer = (carPos - playerPos).Magnitude();
                    
                    bool reachedTarget = distToTarget < 5.0f;
                    bool intercepted = distToPlayer < 5.0f;
                    
                    if (reachedTarget || intercepted) {
                        plugin::Command<COMMAND_SET_CAR_FORWARD_SPEED>(it->car, 0.0f);
                        plugin::Command<COMMAND_CLEAR_CHAR_TASKS>(it->driver);
                        plugin::Command<COMMAND_TASK_LEAVE_CAR>(it->driver, it->car);
                        
                        if (plugin::Command<COMMAND_DOES_BLIP_EXIST>(it->blip)) {
                            plugin::Command<COMMAND_REMOVE_BLIP>(it->blip);
                        }
                        it->state = "leaving";
                    }
                } else if (it->state == "leaving") {
                    bool isStillInCar = plugin::Command<COMMAND_IS_CHAR_IN_CAR>(it->driver, it->car);
                    if (!isStillInCar) {
                        plugin::Command<COMMAND_TASK_WANDER_STANDARD>(it->driver);
                        it->state = "delivered";
                    }
                }
                it++;
            }
        }
    }

    void ShowMessage(const std::string& message, int durationMs) override {
        // Disabled
    }
};
