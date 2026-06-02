#pragma once
#include <string>

struct PlayerStats {
  int money = 0;
  float health = 0.0f;
  float maxHealth = 100.0f;
  float armor = 0.0f;
  float fat = 0.0f;    // 0% to 100%
  float muscle = 0.0f; // 0% to 100%
  int totalPlayTimeSeconds = 0;
  int mealsEaten = 0;
  float lungCapacity = 0.0f;
  float stamina = 0.0f;
  float sexAppeal = 0.0f;
};

struct SkillStats {
  // Vehicles (0% to 100%)
  float drivingSkill = 0.0f;
  float bikeSkill = 0.0f;
  float cyclingSkill = 0.0f;
  float flyingSkill = 0.0f;

  // Weapons (0% to 100%)
  float pistolSkill = 0.0f;
  float silencedPistolSkill = 0.0f;
  float desertEagleSkill = 0.0f;
  float shotgunSkill = 0.0f;
  float sawnoffShotgunSkill = 0.0f;
  float combatShotgunSkill = 0.0f;
  float machinePistolSkill = 0.0f;
  float smgSkill = 0.0f;
  float ak47Skill = 0.0f;
  float m4Skill = 0.0f;
  float sniperSkill = 0.0f;
  float assaultRifleSkill = 0.0f; // Keep for compatibility
};

struct GangStats {
  float respect = 0.0f;                       // 0% to 100%
  float territoryControlledPercentage = 0.0f; // 0% to 100%
  std::string strongestGangName = "Grove Street Families";
  std::string secondStrongestGangName = "Ballas";
  std::string thirdStrongestGangName = "Los Santos Vagos";
  int territoriesHeld = 0;
  int recruitedMembersCount = 0;
  int maxRecruitsCount = 0;
};

struct CrimeStats {
  int peopleKilled = 0;
  int carsStolen = 0;
  int vehiclesDestroyed = 0;
  int bustedCount = 0;
  int wastedCount = 0;
  int starsEvaded = 0;
};

struct ProgressStats {
  float completionPercentage = 0.0f;
  int storyMissionsCompleted = 0;
  int storyMissionsTotal = 0;
  int missionsFailed = 0;
  int cheatsUsedCount = 0;
  int photosTaken = 0;

  // Collectibles
  int oystersCollected = 0;
  int oystersTotal = 50;
  int horseshoesCollected = 0;
  int horseshoesTotal = 50;
  int tagsSprayed = 0;
  int tagsTotal = 100;
  int snapshotsTaken = 0;
  int snapshotsTotal = 50;
};

class IStatsProvider {
public:
  virtual ~IStatsProvider() = default;

  virtual PlayerStats GetPlayerStats() = 0;
  virtual SkillStats GetSkillStats() = 0;
  virtual GangStats GetGangStats() = 0;
  virtual CrimeStats GetCrimeStats() = 0;
  virtual ProgressStats GetProgressStats() = 0;
  virtual void AddPlayerMoney(int amount) = 0;
};

