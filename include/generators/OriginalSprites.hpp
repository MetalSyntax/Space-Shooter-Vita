#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "generators/PixelArtGenerator.hpp"

namespace OriginalSprites {

struct EncodedSprite {
    std::vector<uint32_t> palette;
    std::vector<std::string> rows;
};

extern const EncodedSprite ship;
extern const EncodedSprite enemy;
extern const EncodedSprite enemyShip;
extern const EncodedSprite enemyWeaver;
extern const EncodedSprite boss;
extern const EncodedSprite bullet;
extern const EncodedSprite enemyBullet;
extern const EncodedSprite powerupBlue;
extern const EncodedSprite powerupRed;
extern const EncodedSprite powerupYellow;

PixelArt::Grid decodeSprite(const EncodedSprite& s, PixelArt::Palette& outPalette);

}
