#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include "generators/BossGenerator.hpp"
#include "entities/Ships.hpp"

/** The 10-sector campaign, difficulty table, perks and enemy abilities (port of LevelManager.ts / MainMenu.ts). */
namespace Game {

// --- Difficulty ---

enum class Difficulty { EASY = 0, MEDIUM = 1, HARD = 2 };

struct DifficultyDef {
    float speedMultiplier;
    float spawnDelay; // ms
    int scoreMulti;
    float bossHp;
    uint32_t color;
    const char* nameEs;
    const char* nameEn;
    const char* descEs;
    const char* descEn;
};

const DifficultyDef& difficultyDef(Difficulty d);
const char* difficultyName(Difficulty d);

// --- Abilities: enemy color = ability, the same in every sector ---

enum class Ability { RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE, COUNT };

struct AbilityDef {
    uint32_t color;
    float hue;
    const char* nameEs;
    const char* nameEn;
};

const AbilityDef& abilityDef(Ability a);

// --- Sectors ---

enum class EnemyKind { NONE, DRONE, WEAVER, GUNSHIP, LANCER, POD, SPREADER, SPLITTER, BLINKER, KAMIKAZE, LASER };
enum class WaveKind { STREAM, VFORMATION, PINCER, ASTEROIDS, SHOOTERS, MINES, SWARM };
enum class Decor { NONE, ASTEROIDS, PLASMA, SCRAP, BUOYS, RINGS, STRUCTURES };
enum class BgFx { NONE, LIGHTNING, DISTORTION, GRAVITY, PULSE };

struct LevelDef {
    int id;
    std::string name;
    std::string palette;
    float hue;
    float sat;
    uint32_t spaceColor;
    uint32_t nebula1;
    uint32_t nebula2;
    Decor decor;
    BgFx fx;
    int bpm;
    float duration; // Segundos antes del Boss
    BossGenerator::BossKind boss;
    std::string bossName;
    std::vector<WaveKind> waves;
    EnemyKind unlock;
    const char* threatEs;
    const char* threatEn;
};

const std::vector<LevelDef>& getLevels();
const LevelDef& getLevel(int id);
constexpr int LEVEL_COUNT = 10;

/** Enemies available in a sector: the starters plus every unlock so far. */
std::vector<EnemyKind> roster(int level);
bool isShooter(EnemyKind k);

// --- Perks ---

enum class PerkKind { ENGINE = 0, PLASMA = 1, NANOTECH = 2, MAGAZINE = 3 };

struct PerkInfo {
    const char* titleEs;
    const char* descEs;
    const char* titleEn;
    const char* descEn;
};

const PerkInfo& perkInfo(PerkKind p);
std::array<PerkKind, 3> rollPerks();

// --- Campaign ---

constexpr int MAX_LIVES = 9;

struct CampaignState {
    Difficulty difficulty = Difficulty::MEDIUM;
    ShipId ship = ShipId::VANGUARD;
    int level = 1;      // 1..10
    int score = 0;      // score at the start of `level`
    int lives = 5;
    std::array<int, 4> perks = {0, 0, 0, 0};
};

CampaignState newCampaign(Difficulty d, ShipId ship);
CampaignState applyPerk(const CampaignState& s, PerkKind p);
void saveCampaign(const CampaignState& s);
bool loadCampaign(CampaignState& out);
void clearCampaign();

} // namespace Game
