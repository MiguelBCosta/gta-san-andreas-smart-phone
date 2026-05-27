#pragma once
#include "../../core/providers/IStatsProvider.h"

class SandboxStatsProvider : public IStatsProvider {
public:
  PlayerStats playerStats;
  SkillStats skillStats;
  GangStats gangStats;
  CrimeStats crimeStats;
  ProgressStats progressStats;

  SandboxStatsProvider() {
    // Realistic default values for simulator
    playerStats.money = 14250;
    playerStats.health = 100.0f;
    playerStats.maxHealth = 150.0f;
    playerStats.armor = 50.0f;
    playerStats.fat = 12.5f;
    playerStats.muscle = 85.0f;
    playerStats.totalPlayTimeSeconds = 52330; // ~14h 32m 10s
    playerStats.mealsEaten = 23;
    playerStats.lungCapacity = 65.0f;

    skillStats.drivingSkill = 75.0f;
    skillStats.bikeSkill = 62.0f;
    skillStats.cyclingSkill = 85.0f;
    skillStats.flyingSkill = 40.0f;
    skillStats.pistolSkill = 80.0f;
    skillStats.shotgunSkill = 50.0f;
    skillStats.smgSkill = 90.0f;
    skillStats.assaultRifleSkill = 70.0f;
    skillStats.sniperSkill = 30.0f;

    gangStats.respect = 72.0f;
    gangStats.territoryControlledPercentage = 42.0f;
    gangStats.strongestGangName = "Grove Street Families";
    gangStats.territoriesHeld = 24;
    gangStats.recruitedMembersCount = 3;
    gangStats.maxRecruitsCount = 5;

    crimeStats.peopleKilled = 245;
    crimeStats.carsStolen = 105;
    crimeStats.vehiclesDestroyed = 42;
    crimeStats.bustedCount = 8;
    crimeStats.wastedCount = 12;
    crimeStats.starsEvaded = 15;

    progressStats.completionPercentage = 68.4f;
    progressStats.storyMissionsCompleted = 48;
    progressStats.storyMissionsTotal = 98;
    progressStats.missionsFailed = 15;
    progressStats.cheatsUsedCount = 0;
    progressStats.photosTaken = 18;
    progressStats.oystersCollected = 12;
    progressStats.oystersTotal = 50;
    progressStats.horseshoesCollected = 25;
    progressStats.horseshoesTotal = 50;
    progressStats.tagsSprayed = 65;
    progressStats.tagsTotal = 100;
    progressStats.snapshotsTaken = 30;
    progressStats.snapshotsTotal = 50;
  }

  ~SandboxStatsProvider() override = default;

  PlayerStats GetPlayerStats() override { return playerStats; }
  SkillStats GetSkillStats() override { return skillStats; }
  GangStats GetGangStats() override { return gangStats; }
  CrimeStats GetCrimeStats() override { return crimeStats; }
  ProgressStats GetProgressStats() override { return progressStats; }
};
