#include "entities/Ships.hpp"

namespace Game {

static const std::unordered_map<ShipId, ShipDef> SHIPS = {
    {ShipId::VANGUARD, {"VANGUARD", 0.0f, 1.0f, 0x3f6fe0, "EQUILIBRADA", "LA NAVE ORIGINAL. 3 HYPER BOMBS AL INICIO", 5, 1.0f, 1.0f, 0, 3, false, 0, 1000}},
    {ShipId::STRIKER,  {"STRIKER", 150.0f, 1.1f, 0xff3b3b, "DOBLE CANON", "SIEMPRE DISPARA DOBLE Y +15% CADENCIA", 4, 0.95f, 1.15f, 1, 2, false, 0, 1000}},
    {ShipId::PHANTOM,  {"PHANTOM", 60.0f, 1.0f, 0xb04dff, "EVASION", "+30% VELOCIDAD E INVULNERABILIDAD MAS LARGA", 4, 1.30f, 1.0f, 0, 2, false, 0, 1800}},
    {ShipId::BULWARK,  {"BULWARK", -100.0f, 1.0f, 0x3dff6e, "BLINDAJE", "7 VIDAS Y ESCUDO AL EMPEZAR CADA SECTOR", 7, 0.85f, 0.95f, 0, 2, false, 8000, 1000}},
    {ShipId::NOVA,     {"NOVA", -185.0f, 1.2f, 0xffcf40, "DRON PERMANENTE", "UN DRON TELEDIRIGIDO TE ACOMPANA SIEMPRE", 4, 1.0f, 0.90f, 0, 2, true, 0, 1000}}
};

const std::unordered_map<ShipId, ShipDef>& getShips() {
    return SHIPS;
}

const ShipDef& getShipDef(ShipId id) {
    return SHIPS.at(id);
}

} // namespace Game
