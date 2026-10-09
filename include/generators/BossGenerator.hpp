#pragma once

#include <string>
#include "generators/PixelArtGenerator.hpp"

namespace BossGenerator {

enum class BossKind {
    GOLIATH,
    VIPER,
    AEGIS,
    HYDRA,
    MINER,
    PHANTOM,
    CARRIER,
    BEHEMOTH,
    TITAN,
    LEVIATHAN
};

BossKind parseBossKind(const std::string& name);

PixelArt::Grid generateTurret(uint32_t seed);
PixelArt::Grid generateShieldOrb();
PixelArt::Grid generateCore();
PixelArt::Grid generateBossHull(BossKind kind);

} // namespace BossGenerator
