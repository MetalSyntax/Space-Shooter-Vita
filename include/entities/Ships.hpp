#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Game {

enum class ShipId {
    VANGUARD,
    STRIKER,
    PHANTOM,
    BULWARK,
    NOVA
};

struct ShipDef {
    std::string name;
    float hue;
    float sat;
    uint32_t color;
    std::string ability;
    std::string desc;
    int lives;
    float speed;
    float fireRate;
    int extraShots;
    int bombs;
    bool permanentDrone;
    int sectorShield; // ms
    int invulnMs;     // ms
};

const std::unordered_map<ShipId, ShipDef>& getShips();
const ShipDef& getShipDef(ShipId id);

} // namespace Game
