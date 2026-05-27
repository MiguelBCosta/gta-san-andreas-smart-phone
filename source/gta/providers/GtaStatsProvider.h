#pragma once
#include "../../core/providers/IStatsProvider.h"
#include <game_sa/CWorld.h>
#include <game_sa/common.h>
#include <game_sa/CStats.h>
#include <game_sa/enums/eStats.h>
#include <game_sa/CPedIntelligence.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/CPlayerData.h>
#include <game_sa/CPedGroups.h>
#include <algorithm>

class GtaStatsProvider : public IStatsProvider {
public:
    GtaStatsProvider() = default;
    ~GtaStatsProvider() override = default;

    PlayerStats GetPlayerStats() override {
        PlayerStats stats;
        CPlayerPed* player = FindPlayerPed();
        if (player) {
            stats.money = CWorld::Players[0].m_nMoney;
            stats.health = player->m_fHealth;
            stats.maxHealth = player->m_fMaxHealth;
            stats.armor = player->m_fArmour;
        }
        
        // CStats fat and muscle are stored 0-1000. Convert to 0-100%
        stats.fat = CStats::GetStatValue(STAT_FAT) / 10.0f;
        stats.muscle = CStats::GetStatValue(STAT_MUSCLE) / 10.0f;
        
        // Playtime in milliseconds converted to seconds
        stats.totalPlayTimeSeconds = static_cast<int>(CStats::GetStatValue(STAT_PLAYING_TIME) / 1000.0f);
        stats.mealsEaten = static_cast<int>(CStats::GetStatValue(STAT_NUMBER_OF_MEALS_EATEN));
        
        return stats;
    }

    SkillStats GetSkillStats() override {
        SkillStats stats;
        
        // Vehicle skills are 0-1000. Convert to 0-100%
        stats.drivingSkill = CStats::GetStatValue(STAT_DRIVING_SKILL) / 10.0f;
        stats.bikeSkill = CStats::GetStatValue(STAT_BIKE_SKILL) / 10.0f;
        stats.cyclingSkill = CStats::GetStatValue(STAT_CYCLING_SKILL) / 10.0f;
        stats.flyingSkill = CStats::GetStatValue(STAT_FLYING_SKILL) / 10.0f;

        // Weapon skills are 0-1000. Convert to 0-100%
        stats.pistolSkill = CStats::GetStatValue(STAT_PISTOL_SKILL) / 10.0f;
        stats.shotgunSkill = CStats::GetStatValue(STAT_SHOTGUN_SKILL) / 10.0f;
        stats.smgSkill = CStats::GetStatValue(STAT_SMG_SKILL) / 10.0f;
        
        // Assault rifle skill: average of AK47 and M4
        float ak = CStats::GetStatValue(STAT_AK_47_SKILL);
        float m4 = CStats::GetStatValue(STAT_M4_SKILL);
        stats.assaultRifleSkill = ((ak + m4) / 2.0f) / 10.0f;
        
        stats.sniperSkill = CStats::GetStatValue(STAT_RIFLE_SKILL) / 10.0f;
        stats.lungCapacity = CStats::GetStatValue(STAT_LUNG_CAPACITY) / 10.0f;

        // Clamp values to 0-100%
        stats.drivingSkill = std::clamp(stats.drivingSkill, 0.0f, 100.0f);
        stats.bikeSkill = std::clamp(stats.bikeSkill, 0.0f, 100.0f);
        stats.cyclingSkill = std::clamp(stats.cyclingSkill, 0.0f, 100.0f);
        stats.flyingSkill = std::clamp(stats.flyingSkill, 0.0f, 100.0f);
        stats.pistolSkill = std::clamp(stats.pistolSkill, 0.0f, 100.0f);
        stats.shotgunSkill = std::clamp(stats.shotgunSkill, 0.0f, 100.0f);
        stats.smgSkill = std::clamp(stats.smgSkill, 0.0f, 100.0f);
        stats.assaultRifleSkill = std::clamp(stats.assaultRifleSkill, 0.0f, 100.0f);
        stats.sniperSkill = std::clamp(stats.sniperSkill, 0.0f, 100.0f);
        stats.lungCapacity = std::clamp(stats.lungCapacity, 0.0f, 100.0f);

        return stats;
    }

    GangStats GetGangStats() override {
        GangStats stats;
        float respectVal = CStats::GetStatValue(STAT_TOTAL_RESPECT); // 0-1000
        stats.respect = respectVal / 10.0f;
        stats.respect = std::clamp(stats.respect, 0.0f, 100.0f);

        float territoriesHeld = CStats::GetStatValue(STAT_TERRITORIES_HELD);
        stats.territoriesHeld = static_cast<int>(territoriesHeld);
        
        // 57 territories total in vanilla GTA:SA
        stats.territoryControlledPercentage = (territoriesHeld / 57.0f) * 100.0f;
        stats.territoryControlledPercentage = std::clamp(stats.territoryControlledPercentage, 0.0f, 100.0f);

        float strongest = CStats::GetStatValue(STAT_STRONGEST_GANG);
        if (strongest == 0.0f) stats.strongestGangName = "Ballas";
        else if (strongest == 1.0f) stats.strongestGangName = "Grove Street Families";
        else if (strongest == 2.0f) stats.strongestGangName = "Los Santos Vagos";
        else if (strongest == 3.0f) stats.strongestGangName = "San Fierro Rifa";
        else if (strongest == 4.0f) stats.strongestGangName = "Da Nang Boys";
        else if (strongest == 5.0f) stats.strongestGangName = "Mafia / Triads";
        else stats.strongestGangName = "Grove Street Families";

        // Query player recruited gang members
        stats.recruitedMembersCount = 0;
        CPlayerPed* player = FindPlayerPed();
        if (player && player->m_pPlayerData) {
            unsigned int groupId = player->m_pPlayerData->m_nPlayerGroup;
            if (groupId < 8) {
                stats.recruitedMembersCount = CPedGroups::ms_groups[groupId].m_groupMembership.CountMembersExcludingLeader();
            }
        }

        // Calculate max recruits capacity based on respect levels in game:
        // Respect > 80% = 7 recruits
        // Respect > 60% = 6 recruits
        // Respect > 40% = 5 recruits
        // Respect > 20% = 4 recruits
        // Respect > 10% = 3 recruits
        // Respect > 1% = 2 recruits
        if (respectVal >= 800.0f) stats.maxRecruitsCount = 7;
        else if (respectVal >= 600.0f) stats.maxRecruitsCount = 6;
        else if (respectVal >= 400.0f) stats.maxRecruitsCount = 5;
        else if (respectVal >= 200.0f) stats.maxRecruitsCount = 4;
        else if (respectVal >= 100.0f) stats.maxRecruitsCount = 3;
        else if (respectVal >= 10.0f) stats.maxRecruitsCount = 2;
        else stats.maxRecruitsCount = 0;

        return stats;
    }

    CrimeStats GetCrimeStats() override {
        CrimeStats stats;
        stats.peopleKilled = static_cast<int>(CStats::GetStatValue(STAT_PEOPLE_YOUVE_WASTED));
        stats.carsStolen = static_cast<int>(CStats::GetStatValue(STAT_NUMBER_OF_CARS_STOLEN));
        
        float road = CStats::GetStatValue(STAT_ROAD_VEHICLES_DESTROYED);
        float boat = CStats::GetStatValue(STAT_BOATS_DESTROYED);
        float heli = CStats::GetStatValue(STAT_PLANES_HELICOPTERS_DESTROYED);
        stats.vehiclesDestroyed = static_cast<int>(road + boat + heli);
        
        stats.bustedCount = static_cast<int>(CStats::GetStatValue(STAT_TIMES_BUSTED));
        stats.wastedCount = static_cast<int>(CStats::GetStatValue(STAT_NUMBER_OF_HOSPITAL_VISITS));
        stats.starsEvaded = static_cast<int>(CStats::GetStatValue(STAT_TOTAL_NUMBER_OF_WANTED_STARS_EVADED));

        return stats;
    }

    ProgressStats GetProgressStats() override {
        ProgressStats stats;
        stats.completionPercentage = CStats::GetStatValue(STAT_PROGRESS_MADE);
        stats.completionPercentage = std::clamp(stats.completionPercentage, 0.0f, 100.0f);

        stats.storyMissionsCompleted = static_cast<int>(CStats::GetStatValue(STAT_MISSIONS_PASSED));
        stats.storyMissionsTotal = static_cast<int>(CStats::GetStatValue(STAT_TOTAL_NUMBER_OF_MISSIONS_IN_GAME));
        if (stats.storyMissionsTotal == 0) stats.storyMissionsTotal = 98; // Fallback

        int attempts = static_cast<int>(CStats::GetStatValue(STAT_MISSION_ATTEMPTS));
        stats.missionsFailed = attempts - stats.storyMissionsCompleted;
        if (stats.missionsFailed < 0) stats.missionsFailed = 0;

        stats.cheatsUsedCount = static_cast<int>(CStats::GetStatValue(STAT_TIMES_CHEATED));
        stats.photosTaken = static_cast<int>(CStats::GetStatValue(STAT_PHOTOGRAPHS_TAKEN));

        stats.oystersCollected = static_cast<int>(CStats::GetStatValue(STAT_OYSTERS_COLLECTED));
        stats.oystersTotal = static_cast<int>(CStats::GetStatValue(STAT_TOTAL_OYSTERS));
        if (stats.oystersTotal == 0) stats.oystersTotal = 50;

        stats.horseshoesCollected = static_cast<int>(CStats::GetStatValue(STAT_HORSESHOES_COLLECTED));
        stats.horseshoesTotal = static_cast<int>(CStats::GetStatValue(STAT_TOTAL_HORSESHOES));
        if (stats.horseshoesTotal == 0) stats.horseshoesTotal = 50;

        stats.tagsSprayed = static_cast<int>(CStats::GetStatValue(STAT_TAGS_SPRAYED));
        stats.tagsTotal = 100;

        stats.snapshotsTaken = static_cast<int>(CStats::GetStatValue(STAT_SNAPSHOTS_TAKEN));
        stats.snapshotsTotal = static_cast<int>(CStats::GetStatValue(STAT_TOTAL_SNAPSHOTS));
        if (stats.snapshotsTotal == 0) stats.snapshotsTotal = 50;

        return stats;
    }
};
