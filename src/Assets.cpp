#include "Assets.hpp"
#include "generators/PixelArtGenerator.hpp"
#include "generators/OriginalSprites.hpp"
#include "generators/BossGenerator.hpp"

namespace Game {

using namespace PixelArt;
using Graphics::Renderer;

Assets& assets() {
    static Assets s_assets;
    return s_assets;
}

static void blitGrid(Grid& dest, const Grid& src, int ox, int oy) {
    for (int y = 0; y < (int)src.size(); ++y) {
        for (int x = 0; x < (int)src[y].size(); ++x) {
            if (src[y][x] == EMPTY) continue;
            int dy = oy + y, dx = ox + x;
            if (dy >= 0 && dy < (int)dest.size() && dx >= 0 && dx < (int)dest[0].size()) dest[dy][dx] = src[y][x];
        }
    }
}

struct Decoded {
    Grid grid;
    Palette palette;
};

static Decoded decode(const OriginalSprites::EncodedSprite& s) {
    Decoded d;
    d.grid = OriginalSprites::decodeSprite(s, d.palette);
    return d;
}

static Tex* bake(Renderer& r, const Grid& g, const Palette& p, int cell) {
    return r.createTextureFromRaw(bakeTexture(g, p, cell));
}

/** Original sprite 1:1, optionally recolored. */
static Tex* original(Renderer& r, const OriginalSprites::EncodedSprite& s, float hue = 0.0f, float sat = 1.0f) {
    Decoded d = decode(s);
    return bake(r, d.grid, shiftPalette(d.palette, hue, sat), 1);
}

/** Spiky proximity mine. */
static Grid mineGrid() {
    Grid orb = generateOrb(7);
    Grid m = makeGrid(11, 11);
    for (int y = 1; y + 1 < (int)orb.size(); ++y) {
        for (int x = 1; x + 1 < (int)orb[y].size(); ++x) m[y + 1][x + 1] = orb[y][x];
    }
    const int spikes[6][2] = {{5, 0}, {5, 1}, {0, 5}, {1, 5}, {1, 1}, {2, 2}};
    for (const auto& s : spikes) {
        int x = s[0], y = s[1];
        m[y][x] = ACCENT;
        m[10 - y][x] = ACCENT;
        m[y][10 - x] = ACCENT;
        m[10 - y][10 - x] = ACCENT;
    }
    return outline(m);
}

void Assets::load(Renderer& r) {
    const Palette& player = PALETTES.at("player");

    // Player and weapons: original v1 art, one recolor per ship
    for (int i = 0; i < SHIP_COUNT; ++i) {
        const ShipDef& s = getShipDef(static_cast<ShipId>(i));
        ships[i] = original(r, OriginalSprites::ship, s.hue, s.sat);
    }
    bullet = original(r, OriginalSprites::bullet);
    missile = bake(r, generateMissile(), player, 3);
    orb = bake(r, generateOrb(4), player, 2);
    drone = bake(r, generateShip(hashString("drone"), 6, 6, 1.0f), player, 3);

    RawTexture starRaw;
    starRaw.width = starRaw.height = 2;
    starRaw.pixels.assign(4, 0xFFFFFFFFu);
    star = r.createTextureFromRaw(starRaw);

    // Enemy fleet derived from the three original designs, each with its own silhouette
    Decoded weaver = decode(OriginalSprites::enemyWeaver);
    Decoded ship = decode(OriginalSprites::enemyShip);
    Decoded boss = decode(OriginalSprites::boss);
    Decoded rock = decode(OriginalSprites::enemy);
    auto tint = [](const Palette& p, Ability c, float amount) {
        return tintPalette(shiftPalette(p, abilityDef(c).hue), abilityDef(c).color, amount);
    };
    auto hue = [](const Palette& p, Ability c) { return shiftPalette(p, abilityDef(c).hue, 1.1f); };

    enDrone = bake(r, weaver.grid, weaver.palette, 1);
    enDronePurple = bake(r, crop(weaver.grid, 6, 0, 30, 30), tint(weaver.palette, Ability::PURPLE, 0.65f), 1);
    enDroneYellow = bake(r, crop(ship.grid, 18, 0, 19, 36), hue(ship.palette, Ability::YELLOW), 1);
    enDroneGreen = bake(r, weaver.grid, tint(weaver.palette, Ability::GREEN, 0.65f), 1);
    enPodBlue = bake(r, crop(weaver.grid, 0, 0, 42, 25), tint(weaver.palette, Ability::BLUE, 0.6f), 1);
    enGunshipRed = bake(r, ship.grid, hue(ship.palette, Ability::RED), 1);

    Grid spreader = makeGrid(65, 40);
    blitGrid(spreader, crop(ship.grid, 0, 10, 55, 40), 5, 0);
    enGunshipOrange = bake(r, spreader, hue(ship.palette, Ability::ORANGE), 1);

    Grid laser = makeGrid(35, 55);
    blitGrid(laser, crop(ship.grid, 16, 0, 23, 50), 6, 5);
    blitGrid(laser, crop(weaver.grid, 11, 0, 20, 15), 7, 0);
    enGunshipCyan = bake(r, laser, hue(ship.palette, Ability::CYAN), 1);

    enLancerYellow = bake(r, crop(ship.grid, 14, 0, 27, 50), hue(ship.palette, Ability::YELLOW), 1);
    enSpiderGreen = bake(r, downsample(boss.grid, 2), hue(boss.palette, Ability::GREEN), 1);
    asteroid = bake(r, rock.grid, rock.palette, 1);
    asteroidSmall = bake(r, downsample(rock.grid, 2), rock.palette, 1);
    for (int c = 0; c < static_cast<int>(Ability::COUNT); ++c) {
        ebAbility[c] = original(r, OriginalSprites::enemyBullet, abilityDef(static_cast<Ability>(c)).hue, 1.1f);
    }

    // Power-ups: the three original orbs, plus hue-shifted copies of the blue one for the newer kinds
    powerups[(int)Power::SHIELD] = original(r, OriginalSprites::powerupBlue);
    powerups[(int)Power::RAPID] = original(r, OriginalSprites::powerupYellow);
    powerups[(int)Power::BOMB] = original(r, OriginalSprites::powerupRed);
    powerups[(int)Power::SPREAD] = original(r, OriginalSprites::powerupBlue, -80.0f, 1.0f);
    powerups[(int)Power::MISSILE] = original(r, OriginalSprites::powerupBlue, 170.0f, 1.1f);
    powerups[(int)Power::DRONE] = original(r, OriginalSprites::powerupBlue, 0.0f, 0.1f);
    powerups[(int)Power::MAGNET] = original(r, OriginalSprites::powerupBlue, 80.0f, 1.0f);

    // Per-sector faction sprites and unique boss hulls
    for (const LevelDef& level : getLevels()) {
        const Palette& pal = PALETTES.at(level.palette);
        LevelTextures& t = levels[level.id - 1];
        t.eb = original(r, OriginalSprites::enemyBullet, level.hue, level.sat);
        t.boss = bake(r, BossGenerator::generateBossHull(level.boss), shiftPalette(boss.palette, level.hue, level.sat), 1);
        t.mine = bake(r, mineGrid(), pal, 3);
        t.turret = bake(r, flipV(BossGenerator::generateTurret(hashString("turret:" + std::to_string(level.id)))), pal, 3);
        t.shield = bake(r, BossGenerator::generateShieldOrb(), pal, 3);
        t.core = bake(r, BossGenerator::generateCore(), pal, 3);
    }

    // Background parallax props for each sector theme
    decor[(int)Decor::ASTEROIDS] = bake(r, generateAsteroid(77, 26), PALETTES.at("rock"), 3);
    decor[(int)Decor::PLASMA] = bake(r, generateOrb(14, true), PALETTES.at("nebula"), 3);
    decor[(int)Decor::BUOYS] = bake(r, generateOrb(5), PALETTES.at("steel"), 3);

    Grid scrap = generateShip(hashString("scrap"), 16, 14, 1.0f);
    for (int y = 0; y < (int)scrap.size(); ++y) {
        for (int x = 0; x < (int)scrap[y].size(); ++x) {
            if ((x * 7 + y * 3) % 5 == 0) scrap[y][x] = EMPTY;
        }
    }
    decor[(int)Decor::SCRAP] = bake(r, scrap, PALETTES.at("copper"), 3);

    Grid girder = makeGrid(20, 12);
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 10; ++x) {
            if (y == 0 || y == 11 || x == 0 || (x + y) % 6 == 0 || (x - y + 12) % 6 == 0) girder[y][x] = (y % 11 == 0) ? LIGHT : BODY;
        }
    }
    decor[(int)Decor::STRUCTURES] = bake(r, outline(mirror(girder)), PALETTES.at("steel"), 3);

    Grid ring = makeGrid(60, 14);
    for (int y = 0; y < 14; ++y) {
        for (int x = 0; x < 60; ++x) {
            float d = std::hypot((x - 29.5f) / 30.0f, (y - 6.5f) / 7.0f);
            if (d < 1.0f && d > 0.7f) ring[y][x] = y < 7 ? LIGHT : d > 0.85f ? SHADOW : BODY;
        }
    }
    decor[(int)Decor::RINGS] = bake(r, ring, PALETTES.at("gold"), 3);
}

Tex* Assets::background(int level) {
    auto it = bgCache.find(level);
    if (it != bgCache.end()) return it->second;
    const LevelDef& def = getLevel(level);
    Tex* t = Renderer::instance().createTextureFromRaw(generateBackground(hashString(def.name), def.spaceColor, def.nebula1, def.nebula2, 256));
    bgCache[level] = t;
    return t;
}

void Assets::unload(Renderer& r) {
    for (auto* t : ships) r.freeTexture(t);
    for (Tex* t : {bullet, missile, orb, drone, star, enDrone, enDronePurple, enDroneYellow, enDroneGreen, enPodBlue, enGunshipRed,
                   enGunshipOrange, enGunshipCyan, enLancerYellow, enSpiderGreen, asteroid, asteroidSmall}) {
        r.freeTexture(t);
    }
    for (auto* t : ebAbility) r.freeTexture(t);
    for (auto* t : powerups) r.freeTexture(t);
    for (auto& l : levels) {
        for (Tex* t : {l.eb, l.boss, l.mine, l.turret, l.shield, l.core}) r.freeTexture(t);
    }
    for (auto* t : decor) r.freeTexture(t);
    for (auto& kv : bgCache) r.freeTexture(kv.second);
    bgCache.clear();
}

} // namespace Game
