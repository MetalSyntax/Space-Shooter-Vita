#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cmath>

namespace PixelArt {

using Grid = std::vector<std::vector<uint8_t>>;
using Palette = std::vector<uint32_t>; // formato 0xRRGGBB

// Constantes de roles para las paletas
enum Role : uint8_t {
    EMPTY   = 0,
    OUTLINE = 1,
    SHADOW  = 2,
    BODY    = 3,
    LIGHT   = 4,
    CORE    = 5,
    ENGINE  = 6,
    ACCENT  = 7
};

// Paletas predefinidas del juego
extern const std::unordered_map<std::string, Palette> PALETTES;

// Generador de números pseudo-aleatorios (Mulberry32) reproducible
class PRNG {
public:
    explicit PRNG(uint32_t seed);
    float nextFloat(); // Devuelve float en [0.0, 1.0)
private:
    uint32_t state;
};

uint32_t hashString(const std::string& s);

// Manipulación de Grids
Grid makeGrid(int w, int h, uint8_t fill = EMPTY);
Grid mirror(const Grid& g);
Grid flipV(const Grid& g);
Grid rotateCW(const Grid& g);
Grid keepConnected(const Grid& g, int sx, int sy);
Grid shade(const Grid& g);
Grid outline(const Grid& g);
Grid fromMask(const std::vector<std::string>& lines);
Grid mirrorMask(const std::vector<std::string>& lines, bool odd = true);
Grid crop(const Grid& g, int x, int y, int w, int h);
Grid downsample(const Grid& g, int f);

// Generadores Procedurales
Grid generateShip(uint32_t seed, int w = 12, int h = 12, float density = 0.85f);
Grid generateAsteroid(uint32_t seed, int size);
Grid generateOrb(int size, bool ring = false);
Grid generateIcon(const std::string& kind);
Grid generateMissile();

// Manipulación de Paleta
Palette shiftPalette(const Palette& p, float hueDeg, float sat = 1.0f);
Palette tintPalette(const Palette& p, uint32_t color, float amount);

// Representación de textura en memoria lista para subir a SDL2 o GXM
struct RawTexture {
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels; // 0xAABBGGRR o 0xAARRGGBB (RGBA32)
};

RawTexture bakeTexture(const Grid& grid, const Palette& palette, int cell = 3);
RawTexture generateBackground(uint32_t seed, uint32_t spaceColor, uint32_t nebula1, uint32_t nebula2, int size = 256);

} // namespace PixelArt
