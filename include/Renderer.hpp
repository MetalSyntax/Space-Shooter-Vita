#pragma once

#include <string>
#include <cstdint>
#include "generators/PixelArtGenerator.hpp"

#ifdef __vita__
#include <vitaGL.h>
namespace Graphics {
struct Texture2D {
    GLuint id = 0;
    int width = 0;
    int height = 0;
};
typedef Texture2D TextureType;
}
#else
#include <SDL2/SDL.h>
namespace Graphics {
typedef SDL_Texture TextureType;
}
#endif

namespace Graphics {

constexpr int SCREEN_WIDTH  = 960;
constexpr int SCREEN_HEIGHT = 544;

struct Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;
};

/** 0xRRGGBB + alpha → Color. */
inline Color rgb(uint32_t hex, uint8_t a = 255) {
    return Color{static_cast<uint8_t>((hex >> 16) & 255), static_cast<uint8_t>((hex >> 8) & 255), static_cast<uint8_t>(hex & 255), a};
}

class Renderer {
public:
    static Renderer& instance();

    bool init(const std::string& title = "Shooter Space (PS Vita)");
    void shutdown();

    void clear(uint8_t r = 15, uint8_t g = 23, uint8_t b = 42); // #0f172a
    void present();

    TextureType* createTextureFromRaw(const PixelArt::RawTexture& raw);
    void freeTexture(TextureType* tex);
    static int texWidth(TextureType* tex);
    static int texHeight(TextureType* tex);

    // Dibuja una textura centrada o en coordenadas (x, y) con rotación y escala
    void drawTexture(TextureType* tex, int x, int y, double angleDeg = 0.0, float scale = 1.0f, bool centered = true);

    /** Centered sprite with independent X/Y scale, color modulation and optional additive blending. */
    void drawSprite(TextureType* tex, float x, float y, float angleDeg = 0.0f, float sx = 1.0f, float sy = 1.0f,
                    Color tint = Color{}, bool additive = false);

    /** Textured triangles (xy/uv pairs, uv in 0..1), one draw call; used by the bitmap font. */
    void drawTexturedTriangles(TextureType* tex, const float* xy, const float* uv, int vertCount, Color tint);

    void drawBackgroundTiled(TextureType* tex, int scrollX = 0, int scrollY = 0, uint8_t alpha = 255);

    // Primitivas de renderizado directo
    void drawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void drawRect(float x, float y, float w, float h, Color c);
    void drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void fillTriangles(const float* xy, int vertCount, Color c, bool additive = false);
    void fillCircle(float cx, float cy, float radius, Color c, bool additive = false);
    /** Ring arc from startRad to endRad (clockwise on screen). */
    void drawArc(float cx, float cy, float radius, float thickness, float startRad, float endRad, Color c);
    void drawCircle(float cx, float cy, float radius, float thickness, Color c);
    void drawLine(float x0, float y0, float x1, float y1, float width, Color c, bool additive = false);

    /** Global draw offset (screen shake). */
    void setOffset(float ox, float oy) { offX = ox; offY = oy; }
    void setClip(int x, int y, int w, int h);
    void clearClip();

private:
    Renderer() = default;
    ~Renderer() = default;

    float offX = 0.0f;
    float offY = 0.0f;

#ifndef __vita__
    SDL_Window* sdlWindow = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
#endif
};

} // namespace Graphics
