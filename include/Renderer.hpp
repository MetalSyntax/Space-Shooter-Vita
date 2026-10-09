#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "generators/PixelArtGenerator.hpp"

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

    SDL_Renderer* getSDLRenderer() const { return sdlRenderer; }

    // Crea una textura SDL a partir de RawTexture (pixel-perfect RGBA)
    SDL_Texture* createTextureFromRaw(const PixelArt::RawTexture& raw);

    // Dibuja una textura centrada o en coordenadas (x, y) con rotación y escala
    void drawTexture(SDL_Texture* tex, int x, int y, double angleDeg = 0.0, float scale = 1.0f, bool centered = true);

    // Dibuja un fondo en mosaico (tiling)
    void drawBackgroundTiled(SDL_Texture* tex, int scrollX = 0, int scrollY = 0);

private:
    Renderer() = default;
    ~Renderer() = default;

    SDL_Window* sdlWindow = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
};

} // namespace Graphics
