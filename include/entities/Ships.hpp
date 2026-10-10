#pragma once

#include <string>
#include <array>
#include <cstdint>

namespace Game {

enum class ShipId {
    VANGUARD,
    STRIKER,
    PHANTOM,
    BULWARK,
    NOVA
};

constexpr int SHIP_COUNT = 5;

struct ShipDef {
    std::string name;
    float hue;   // Hue rotation applied to the original (blue) ship art
    float sat;
    uint32_t color;
    const char* abilityEs;
    const char* abilityEn;
    const char* descEs;
    const char* descEn;
    int lives;
    float speed;
    float fireRate;
    int extraShots;
    int bombs;
    bool permanentDrone;
    int sectorShield; // ms
    int invulnMs;     // ms
};

const ShipDef& getShipDef(ShipId id);
const std::array<ShipId, SHIP_COUNT>& shipOrder();

} // namespace Game
