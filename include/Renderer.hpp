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

class Renderer {
public:
    static Renderer& instance();

    bool init(const std::string& title = "Shooter Space (PS Vita)");
    void shutdown();

    void clear(uint8_t r = 15, uint8_t g = 23, uint8_t b = 42); // #0f172a
    void present();

    // Crea una textura nativa (vita2d_texture en PS Vita, SDL_Texture en PC)
    TextureType* createTextureFromRaw(const PixelArt::RawTexture& raw);
    void freeTexture(TextureType* tex);

    // Dibuja una textura centrada o en coordenadas (x, y) con rotación y escala
    void drawTexture(TextureType* tex, int x, int y, double angleDeg = 0.0, float scale = 1.0f, bool centered = true);

    // Dibuja un fondo en mosaico (tiling)
    void drawBackgroundTiled(TextureType* tex, int scrollX = 0, int scrollY = 0);

    // Primitivas de renderizado directo
    void drawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

private:
    Renderer() = default;
    ~Renderer() = default;

#ifndef __vita__
    SDL_Window* sdlWindow = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
#endif
};

} // namespace Graphics
