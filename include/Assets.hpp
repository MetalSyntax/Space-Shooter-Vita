#pragma once

#include <map>
#include "Renderer.hpp"
#include "LevelManager.hpp"

/** Every sprite baked at boot from the pixel-art generators (port of Preloader.ts). */
namespace Game {

using Tex = Graphics::TextureType;

/** Power-up drops; the first six are timed powers, BOMB adds a Hyper Bomb charge. */
enum class Power { SHIELD, RAPID, SPREAD, MISSILE, DRONE, MAGNET, BOMB };
constexpr int POWER_COUNT = 6;
constexpr int DROP_COUNT = 7;

struct LevelTextures {
    Tex* eb = nullptr;     // boss/sector bullet
    Tex* boss = nullptr;
    Tex* mine = nullptr;
    Tex* turret = nullptr;
    Tex* shield = nullptr;
    Tex* core = nullptr;
};

struct Assets {
    Tex* ships[SHIP_COUNT] = {};
    Tex* bullet = nullptr;
    Tex* missile = nullptr;
    Tex* orb = nullptr;
    Tex* drone = nullptr;
    Tex* star = nullptr;

    Tex* enDrone = nullptr;
    Tex* enDronePurple = nullptr;
    Tex* enDroneYellow = nullptr;
    Tex* enDroneGreen = nullptr;
    Tex* enPodBlue = nullptr;
    Tex* enGunshipRed = nullptr;
    Tex* enGunshipOrange = nullptr;
    Tex* enGunshipCyan = nullptr;
    Tex* enLancerYellow = nullptr;
    Tex* enSpiderGreen = nullptr;
    Tex* asteroid = nullptr;
    Tex* asteroidSmall = nullptr;

    Tex* ebAbility[static_cast<int>(Ability::COUNT)] = {};
    Tex* powerups[DROP_COUNT] = {};
    LevelTextures levels[LEVEL_COUNT];
    Tex* decor[7] = {}; // indexed by Decor

    void load(Graphics::Renderer& r);
    void unload(Graphics::Renderer& r);

    /** Sector backdrop, generated on first use and cached. */
    Tex* background(int level);

private:
    std::map<int, Tex*> bgCache;
};

Assets& assets();

} // namespace Game
