#include "LevelManager.hpp"

namespace Game {

static const std::unordered_map<PerkKind, PerkInfo> PERKS = {
    {PerkKind::ENGINE,   {"OVERCLOCK MOTORES", "+15% VELOCIDAD"}},
    {PerkKind::PLASMA,   {"CONDENSADOR PLASMA", "+10% CADENCIA"}},
    {PerkKind::NANOTECH, {"PLACAS NANOTEC", "+1 VIDA"}},
    {PerkKind::MAGAZINE, {"CARGADOR EXTENDIDO", "+1 PROYECTIL"}}
};

const std::unordered_map<PerkKind, PerkInfo>& getPerksInfo() {
    return PERKS;
}

static const std::vector<LevelDef> LEVELS = {
    {1,  "ASTEROID BELT",    "military",   0.0f, 1.00f, 0x060a18, 0x1a3a7a, 0x4a7ad0, 60.0f, BossGenerator::BossKind::GOLIATH,   "GOLIATH-CORE",     "DRONES BLANCOS · GUNSHIPS ROJOS DISPARAN"},
    {2,  "CAELUM NEBULA",    "nebula",   -80.0f, 1.00f, 0x0c0618, 0x4a1a7a, 0xc04ad0, 65.0f, BossGenerator::BossKind::VIPER,     "VIPER-STING",      "LANCER AMARILLO · EMBESTIDA"},
    {3,  "ORBITAL SCRAPYARD", "copper",    30.0f, 0.80f, 0x0e0905, 0x5a3218, 0xa8642e, 70.0f, BossGenerator::BossKind::AEGIS,     "AEGIS-FORTRESS",   "POD AZUL · ESCUDO DE ENERGIA"},
    {4,  "PLASMA STORM",     "volcanic",  15.0f, 1.20f, 0x140505, 0x7a1a0e, 0xff5522, 70.0f, BossGenerator::BossKind::HYDRA,     "HYDRA-TWIN",       "GUNSHIP NARANJA · ABANICO"},
    {5,  "THE MINEFIELD",     "steel",      0.0f, 0.35f, 0x040406, 0x1a1a24, 0x33334a, 75.0f, BossGenerator::BossKind::MINER,     "DREAD-MINER",      "ARANA VERDE · SE DIVIDE"},
    {6,  "QUANTUM RIFT",     "rift",     180.0f, 1.00f, 0x03101a, 0x0f4a63, 0x3dfcff, 75.0f, BossGenerator::BossKind::PHANTOM,   "PHANTOM-CRUISER",  "DRON PURPURA · TELETRANSPORTE"},
    {7,  "GAS GIANT RINGS",  "gold",     110.0f, 1.00f, 0x061208, 0x1f6b52, 0xffcf40, 80.0f, BossGenerator::BossKind::CARRIER,   "SOLARIS-CARRIER",  "ENJAMBRE AMARILLO · KAMIKAZE"},
    {8,  "HYPERION OUTPOST", "steel",      0.0f, 0.15f, 0x07080c, 0x2a3040, 0x6e7686, 80.0f, BossGenerator::BossKind::BEHEMOTH,  "BEHEMOTH-LASER",   "GUNSHIP CIAN · LASER"},
    {9,  "EVENT HORIZON",    "mono",       0.0f, 0.00f, 0x000000, 0x2a2a2a, 0x9a9a9a, 85.0f, BossGenerator::BossKind::TITAN,     "GRAVITY-TITAN",    "TODAS LAS AMENAZAS ACTIVAS"},
    {10, "THE MOTHER CORE",  "cyber",    -60.0f, 1.20f, 0x0a0414, 0x4a1670, 0x3dfcff, 90.0f, BossGenerator::BossKind::LEVIATHAN, "OMNI-LEVIATHAN",   "BATALLA FINAL"}
};

const std::vector<LevelDef>& getLevels() {
    return LEVELS;
}

const LevelDef& getLevel(int id) {
    if (id < 1) return LEVELS.front();
    if (id > static_cast<int>(LEVELS.size())) return LEVELS.back();
    return LEVELS[id - 1];
}

} // namespace Game
