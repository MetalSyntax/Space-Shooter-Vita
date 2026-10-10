#include "entities/Ships.hpp"
#include <algorithm>

namespace Game {

static const ShipDef SHIPS[SHIP_COUNT] = {
    {"VANGUARD", 0.0f, 1.0f, 0x3f6fe0, "EQUILIBRADA", "BALANCED", "LA NAVE ORIGINAL. 3 HYPER BOMBS AL INICIO", "ORIGINAL BALANCED SHIP. 3 HYPER BOMBS AT START", 5, 1.0f, 1.0f, 0, 3, false, 0, 1000},
    {"STRIKER", 150.0f, 1.1f, 0xff3b3b, "DOBLE CANON", "TWIN CANNON", "SIEMPRE DISPARA DOBLE Y +15% CADENCIA", "ALWAYS FIRES TWIN SHOTS & +15% FIRE RATE", 4, 0.95f, 1.15f, 1, 2, false, 0, 1000},
    {"PHANTOM", 60.0f, 1.0f, 0xb04dff, "EVASION", "EVASION", "+30% VELOCIDAD E INVULNERABILIDAD MAS LARGA", "+30% SPEED & LONGER HIT INVULNERABILITY", 4, 1.30f, 1.0f, 0, 2, false, 0, 1800},
    {"BULWARK", -100.0f, 1.0f, 0x3dff6e, "BLINDAJE", "ARMORED", "7 VIDAS Y ESCUDO AL EMPEZAR CADA SECTOR", "7 LIVES & SHIELD AT THE START OF EACH SECTOR", 7, 0.85f, 0.95f, 0, 2, false, 8000, 1000},
    {"NOVA", -185.0f, 1.2f, 0xffcf40, "DRON PERMANENTE", "PERMANENT DRONE", "UN DRON TELEDIRIGIDO TE ACOMPANA SIEMPRE", "A HOMING ESCORT DRONE FLIES WITH YOU ALWAYS", 4, 1.0f, 0.90f, 0, 2, true, 0, 1000},
};

const ShipDef& getShipDef(ShipId id) {
    return SHIPS[std::clamp(static_cast<int>(id), 0, SHIP_COUNT - 1)];
}

const std::array<ShipId, SHIP_COUNT>& shipOrder() {
    static const std::array<ShipId, SHIP_COUNT> order = {ShipId::VANGUARD, ShipId::STRIKER, ShipId::PHANTOM, ShipId::BULWARK, ShipId::NOVA};
    return order;
}

} // namespace Game
