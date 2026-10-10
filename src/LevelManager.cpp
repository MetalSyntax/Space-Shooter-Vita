#include "LevelManager.hpp"
#include "SaveSystem.hpp"
#include "I18n.hpp"
#include <algorithm>
#include <cstdlib>

namespace Game {

using BossGenerator::BossKind;

static const DifficultyDef DIFFICULTIES[3] = {
    {1.0f, 1000.0f, 1, 0.8f, 0x33dd66, "FACIL", "EASY", "PUNTOS x1 · RITMO TRANQUILO", "SCORE x1 · RELAXED TEMPO"},
    {1.5f, 700.0f, 2, 1.0f, 0xffcc33, "NORMAL", "NORMAL", "PUNTOS x2 · RITMO RAPIDO", "SCORE x2 · FAST TEMPO"},
    {2.0f, 400.0f, 3, 1.25f, 0xff4455, "DIFICIL", "HARD", "PUNTOS x3 · SOLO PARA ASES", "SCORE x3 · FOR ACES ONLY"},
};

const DifficultyDef& difficultyDef(Difficulty d) {
    int i = std::clamp(static_cast<int>(d), 0, 2);
    return DIFFICULTIES[i];
}

const char* difficultyName(Difficulty d) {
    const auto& def = difficultyDef(d);
    return I18n::L(def.nameEs, def.nameEn);
}

static const AbilityDef ABILITIES[] = {
    {0xff3333, 0.0f, "DISPARO DIRIGIDO", "AIMED FIRE"},
    {0xff8a1f, 30.0f, "ABANICO", "SPREAD FAN"},
    {0xffd21f, 55.0f, "EMBESTIDA", "RAM DASH"},
    {0x3dff6e, 120.0f, "SE DIVIDE", "SPLITS"},
    {0x2ff3ff, 180.0f, "LASER", "LASER BEAM"},
    {0x3d7bff, 220.0f, "ESCUDO", "BUBBLE SHIELD"},
    {0xb04dff, 275.0f, "TELETRANSPORTE", "TELEPORT"},
};

const AbilityDef& abilityDef(Ability a) {
    return ABILITIES[static_cast<int>(a)];
}

using W = WaveKind;

static const std::vector<LevelDef> LEVELS = {
    {1, "ASTEROID BELT", "military", 0.0f, 1.00f, 0x060a18, 0x1a3a7a, 0x4a7ad0, Decor::ASTEROIDS, BgFx::NONE, 110, 90.0f, BossKind::GOLIATH, "GOLIATH-CORE",
     {W::STREAM, W::ASTEROIDS, W::VFORMATION}, EnemyKind::NONE, "DRONES BLANCOS · GUNSHIPS ROJOS DISPARAN", "WHITE DRONES · RED GUNSHIPS FIRE"},
    {2, "CAELUM NEBULA", "nebula", -80.0f, 1.00f, 0x0c0618, 0x4a1a7a, 0xc04ad0, Decor::PLASMA, BgFx::NONE, 110, 95.0f, BossKind::VIPER, "VIPER-STING",
     {W::STREAM, W::VFORMATION, W::PINCER}, EnemyKind::LANCER, "NUEVO: LANCER AMARILLO · EMBESTIDA", "NEW: YELLOW LANCER · RAM DASH"},
    {3, "ORBITAL SCRAPYARD", "copper", 30.0f, 0.80f, 0x0e0905, 0x5a3218, 0xa8642e, Decor::SCRAP, BgFx::NONE, 110, 100.0f, BossKind::AEGIS, "AEGIS-FORTRESS",
     {W::STREAM, W::ASTEROIDS, W::SHOOTERS, W::PINCER}, EnemyKind::POD, "NUEVO: POD AZUL · ESCUDO", "NEW: BLUE POD · BUBBLE SHIELD"},
    {4, "PLASMA STORM", "volcanic", 15.0f, 1.20f, 0x140505, 0x7a1a0e, 0xff5522, Decor::PLASMA, BgFx::LIGHTNING, 128, 100.0f, BossKind::HYDRA, "HYDRA-TWIN",
     {W::VFORMATION, W::PINCER, W::SHOOTERS, W::STREAM}, EnemyKind::SPREADER, "NUEVO: GUNSHIP NARANJA · ABANICO", "NEW: ORANGE GUNSHIP · SPREAD FAN"},
    {5, "THE MINEFIELD", "steel", 0.0f, 0.35f, 0x040406, 0x1a1a24, 0x33334a, Decor::BUOYS, BgFx::NONE, 128, 105.0f, BossKind::MINER, "DREAD-MINER",
     {W::MINES, W::STREAM, W::SHOOTERS, W::VFORMATION}, EnemyKind::SPLITTER, "NUEVO: ARANA VERDE · SE DIVIDE", "NEW: GREEN SPIDER · SPLITS ON DEATH"},
    {6, "QUANTUM RIFT", "rift", 180.0f, 1.00f, 0x03101a, 0x0f4a63, 0x3dfcff, Decor::NONE, BgFx::DISTORTION, 128, 105.0f, BossKind::PHANTOM, "PHANTOM-CRUISER",
     {W::PINCER, W::VFORMATION, W::SHOOTERS, W::SWARM}, EnemyKind::BLINKER, "NUEVO: DRON PURPURA · TELETRANSPORTE", "NEW: PURPLE DRONE · TELEPORTS"},
    {7, "GAS GIANT RINGS", "gold", 110.0f, 1.00f, 0x061208, 0x1f6b52, 0xffcf40, Decor::RINGS, BgFx::NONE, 128, 110.0f, BossKind::CARRIER, "SOLARIS-CARRIER",
     {W::SWARM, W::STREAM, W::ASTEROIDS, W::VFORMATION}, EnemyKind::KAMIKAZE, "NUEVO: ENJAMBRE AMARILLO · KAMIKAZE", "NEW: YELLOW SWARM · KAMIKAZE RAM"},
    {8, "HYPERION OUTPOST", "steel", 0.0f, 0.15f, 0x07080c, 0x2a3040, 0x6e7686, Decor::STRUCTURES, BgFx::NONE, 140, 110.0f, BossKind::BEHEMOTH, "BEHEMOTH-LASER",
     {W::SHOOTERS, W::PINCER, W::VFORMATION, W::MINES}, EnemyKind::LASER, "NUEVO: GUNSHIP CIAN · LASER", "NEW: CYAN GUNSHIP · BEAM LASER"},
    {9, "EVENT HORIZON", "mono", 0.0f, 0.00f, 0x000000, 0x2a2a2a, 0x9a9a9a, Decor::NONE, BgFx::GRAVITY, 140, 115.0f, BossKind::TITAN, "GRAVITY-TITAN",
     {W::SWARM, W::PINCER, W::MINES, W::SHOOTERS, W::VFORMATION}, EnemyKind::NONE, "TODAS LAS AMENAZAS", "ALL THREATS ACTIVE"},
    {10, "THE MOTHER CORE", "cyber", -60.0f, 1.20f, 0x0a0414, 0x4a1670, 0x3dfcff, Decor::STRUCTURES, BgFx::PULSE, 145, 120.0f, BossKind::LEVIATHAN, "OMNI-LEVIATHAN",
     {W::SWARM, W::PINCER, W::SHOOTERS, W::MINES, W::VFORMATION, W::ASTEROIDS}, EnemyKind::NONE, "ULTIMA BATALLA", "FINAL SHOWDOWN"},
};

const std::vector<LevelDef>& getLevels() {
    return LEVELS;
}

const LevelDef& getLevel(int id) {
    if (id < 1) return LEVELS.front();
    if (id > static_cast<int>(LEVELS.size())) return LEVELS.back();
    return LEVELS[id - 1];
}

std::vector<EnemyKind> roster(int level) {
    std::vector<EnemyKind> out = {EnemyKind::DRONE, EnemyKind::WEAVER, EnemyKind::GUNSHIP};
    for (int i = 0; i < level && i < static_cast<int>(LEVELS.size()); ++i) {
        if (LEVELS[i].unlock != EnemyKind::NONE) out.push_back(LEVELS[i].unlock);
    }
    return out;
}

bool isShooter(EnemyKind k) {
    return k == EnemyKind::GUNSHIP || k == EnemyKind::SPREADER || k == EnemyKind::LASER;
}

static const PerkInfo PERKS[4] = {
    {"OVERCLOCK MOTORES", "+15% VELOCIDAD", "ENGINE OVERCLOCK", "+15% SPEED"},
    {"CONDENSADOR PLASMA", "+10% CADENCIA", "PLASMA CAPACITOR", "+10% FIRE RATE"},
    {"PLACAS NANOTEC", "+1 VIDA", "NANOTECH PLATING", "+1 LIFE"},
    {"CARGADOR EXTENDIDO", "+1 PROYECTIL", "EXTENDED MAGAZINE", "+1 PROJECTILE"},
};

const PerkInfo& perkInfo(PerkKind p) {
    return PERKS[static_cast<int>(p)];
}

std::array<PerkKind, 3> rollPerks() {
    std::array<PerkKind, 4> all = {PerkKind::ENGINE, PerkKind::PLASMA, PerkKind::NANOTECH, PerkKind::MAGAZINE};
    for (int i = 3; i > 0; --i) std::swap(all[i], all[std::rand() % (i + 1)]);
    return {all[0], all[1], all[2]};
}

CampaignState newCampaign(Difficulty d, ShipId ship) {
    CampaignState s;
    s.difficulty = d;
    s.ship = ship;
    s.lives = getShipDef(ship).lives;
    return s;
}

CampaignState applyPerk(const CampaignState& s, PerkKind p) {
    CampaignState next = s;
    next.perks[static_cast<int>(p)]++;
    if (p == PerkKind::NANOTECH) next.lives = std::min(MAX_LIVES, next.lives + 1);
    return next;
}

void saveCampaign(const CampaignState& s) {
    auto& c = Storage::SaveSystem::data().campaign;
    c.valid = 1;
    c.difficulty = static_cast<int32_t>(s.difficulty);
    c.ship = static_cast<int32_t>(s.ship);
    c.level = s.level;
    c.score = s.score;
    c.lives = s.lives;
    for (int i = 0; i < 4; ++i) c.perks[i] = s.perks[i];
    auto& d = Storage::SaveSystem::data();
    d.highestSector = std::max(d.highestSector, s.level);
    Storage::SaveSystem::flush();
}

bool loadCampaign(CampaignState& out) {
    const auto& c = Storage::SaveSystem::data().campaign;
    if (!c.valid || c.level < 1 || c.level > LEVEL_COUNT) return false;
    out.difficulty = static_cast<Difficulty>(std::clamp(c.difficulty, 0, 2));
    out.ship = static_cast<ShipId>(std::clamp(c.ship, 0, 4));
    out.level = c.level;
    out.score = std::max(0, c.score);
    out.lives = std::clamp(c.lives, 1, MAX_LIVES);
    for (int i = 0; i < 4; ++i) out.perks[i] = std::max(0, c.perks[i]);
    return true;
}

void clearCampaign() {
    Storage::SaveSystem::data().campaign.valid = 0;
    Storage::SaveSystem::flush();
}

} // namespace Game
