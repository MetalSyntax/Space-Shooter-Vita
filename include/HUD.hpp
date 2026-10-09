#pragma once

#include <string>
#include <SDL2/SDL.h>
#include "Renderer.hpp"

namespace UI {

class BitmapFont {
public:
    static void drawText(Graphics::Renderer& renderer, const std::string& text, int x, int y, int scale = 2, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);
};

struct HUD {
    static void render(Graphics::Renderer& renderer, int score, int lives, int bombs, int sector, float bossHpPct = -1.0f);
};

} // namespace UI
