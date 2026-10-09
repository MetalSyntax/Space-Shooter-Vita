#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "generators/BossGenerator.hpp"

namespace Game {

enum class PerkKind {
    ENGINE,    // +15% Velocidad
    PLASMA,    // +10% Cadencia
    NANOTECH,  // +1 Vida
    MAGAZINE   // +1 Proyectil
};

struct PerkInfo {
    std::string title;
    std::string desc;
};

const std::unordered_map<PerkKind, PerkInfo>& getPerksInfo();

struct LevelDef {
    int id;
    std::string name;
    std::string palette;
    float hue;
    float sat;
    uint32_t spaceColor;
    uint32_t nebula1;
    uint32_t nebula2;
    float duration; // Segundos antes del Boss
    BossGenerator::BossKind boss;
    std::string bossName;
    std::string threat;
};

const std::vector<LevelDef>& getLevels();
const LevelDef& getLevel(int id);

// Estado de la campaña (1..10)
struct CampaignState {
    int level = 1;
    int score = 0;
    int lives = 5;
    std::unordered_map<PerkKind, int> perks;
};

} // namespace Game
