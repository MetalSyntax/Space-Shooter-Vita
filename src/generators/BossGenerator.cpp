#include "generators/BossGenerator.hpp"
#include "generators/OriginalSprites.hpp"
#include <cmath>

namespace BossGenerator {

BossKind parseBossKind(const std::string& name) {
    if (name == "goliath") return BossKind::GOLIATH;
    if (name == "viper") return BossKind::VIPER;
    if (name == "aegis") return BossKind::AEGIS;
    if (name == "hydra") return BossKind::HYDRA;
    if (name == "miner") return BossKind::MINER;
    if (name == "phantom") return BossKind::PHANTOM;
    if (name == "carrier") return BossKind::CARRIER;
    if (name == "behemoth") return BossKind::BEHEMOTH;
    if (name == "titan") return BossKind::TITAN;
    return BossKind::LEVIATHAN;
}

PixelArt::Grid generateTurret(uint32_t seed) {
    return PixelArt::generateShip(seed, 8, 8, 0.9f);
}

PixelArt::Grid generateShieldOrb() {
    return PixelArt::generateOrb(12);
}

PixelArt::Grid generateCore() {
    return PixelArt::generateOrb(8);
}

static void blit(PixelArt::Grid& dest, const PixelArt::Grid& src, int ox, int oy) {
    int dh = static_cast<int>(dest.size());
    if (dh == 0) return;
    int dw = static_cast<int>(dest[0].size());

    int sh = static_cast<int>(src.size());
    for (int y = 0; y < sh; ++y) {
        int sw = static_cast<int>(src[y].size());
        for (int x = 0; x < sw; ++x) {
            uint8_t val = src[y][x];
            if (val != 0) {
                int dy = oy + y;
                int dx = ox + x;
                if (dy >= 0 && dy < dh && dx >= 0 && dx < dw) {
                    dest[dy][dx] = val;
                }
            }
        }
    }
}

PixelArt::Grid generateBossHull(BossKind kind) {
    PixelArt::Palette p1, p2, p3;
    PixelArt::Grid origBoss = OriginalSprites::decodeSprite(OriginalSprites::boss, p1);
    PixelArt::Grid origShip = OriginalSprites::decodeSprite(OriginalSprites::enemyShip, p2);
    PixelArt::Grid origWeaver = OriginalSprites::decodeSprite(OriginalSprites::enemyWeaver, p3);

    switch (kind) {
        case BossKind::GOLIATH: {
            int w = static_cast<int>(origBoss[0].size()) + 20;
            int h = static_cast<int>(origBoss.size());
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, origBoss, 10, 0);
            PixelArt::Grid wingL = PixelArt::crop(origShip, 0, 10, 15, 35);
            PixelArt::Grid wingR = PixelArt::crop(origShip, 40, 10, 15, 35);
            blit(out, wingL, 0, 20);
            blit(out, wingR, w - 15, 20);
            return out;
        }

        case BossKind::VIPER: {
            PixelArt::Grid core = PixelArt::crop(origBoss, 15, 0, 40, 77);
            int w = 64, h = 80;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, core, 12, 0);
            PixelArt::Grid bladeL = PixelArt::crop(origShip, 10, 0, 12, 45);
            PixelArt::Grid bladeR = PixelArt::crop(origShip, 33, 0, 12, 45);
            blit(out, bladeL, 0, 25);
            blit(out, bladeR, w - 12, 25);
            return out;
        }

        case BossKind::AEGIS: {
            PixelArt::Grid center = PixelArt::crop(origBoss, 10, 10, 50, 60);
            PixelArt::Grid plate = PixelArt::crop(origWeaver, 0, 0, 42, 20);
            int w = 70, h = 85;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, center, 10, 15);
            blit(out, plate, 14, 0);
            return out;
        }

        case BossKind::HYDRA: {
            PixelArt::Grid halfL = PixelArt::crop(origBoss, 0, 0, 32, 70);
            PixelArt::Grid halfR = PixelArt::crop(origBoss, 38, 0, 32, 70);
            PixelArt::Grid bridge = PixelArt::crop(origShip, 18, 15, 19, 18);
            int w = 84, h = 75;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, halfL, 0, 5);
            blit(out, halfR, 52, 5);
            blit(out, bridge, 32, 20);
            return out;
        }

        case BossKind::MINER: {
            PixelArt::Grid hull = PixelArt::crop(origBoss, 5, 20, 60, 50);
            PixelArt::Grid prongs = PixelArt::crop(origWeaver, 5, 15, 32, 20);
            int w = 76, h = 75;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, hull, 8, 0);
            blit(out, prongs, 22, 50);
            return out;
        }

        case BossKind::PHANTOM: {
            PixelArt::Grid mid = PixelArt::crop(origBoss, 12, 0, 46, 75);
            PixelArt::Grid out = mid;
            for (int y = 30; y < 50; ++y) {
                for (int x = 17; x < 29; ++x) {
                    if (y < static_cast<int>(out.size()) && x < static_cast<int>(out[0].size())) {
                        out[y][x] = 0;
                    }
                }
            }
            return out;
        }

        case BossKind::CARRIER: {
            int w = 90, h = 80;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, origBoss, 10, 0);
            PixelArt::Grid podL = PixelArt::crop(origWeaver, 0, 0, 18, 28);
            PixelArt::Grid podR = PixelArt::crop(origWeaver, 24, 0, 18, 28);
            blit(out, podL, 0, 10);
            blit(out, podR, w - 18, 10);
            return out;
        }

        case BossKind::BEHEMOTH: {
            PixelArt::Grid cannon = PixelArt::crop(origShip, 22, 0, 11, 48);
            int w = 78, h = 90;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, origBoss, 4, 12);
            blit(out, cannon, 33, 0);
            return out;
        }

        case BossKind::TITAN: {
            int w = 74, h = 74;
            PixelArt::Grid out = PixelArt::makeGrid(w, h);
            blit(out, origBoss, 2, 0);
            float cx = 37.0f, cy = 37.0f;
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    float d = std::hypot(x - cx, y - cy);
                    if (d < 14.0f) {
                        out[y][x] = 0;
                    } else if (d >= 14.0f && d < 18.0f && out[y][x] != 0) {
                        out[y][x] = 1;
                    }
                }
            }
            return out;
        }

        case BossKind::LEVIATHAN:
        default:
            return origBoss;
    }
}

} // namespace BossGenerator
