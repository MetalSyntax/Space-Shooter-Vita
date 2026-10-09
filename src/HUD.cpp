#include "HUD.hpp"
#include <unordered_map>

namespace UI {

// Matriz de glifos pixel art retro 3x5 embebidos
static const std::unordered_map<char, uint16_t> FONT_3X5 = {
    {'0', 0x7B6F}, {'1', 0x2492}, {'2', 0x73E7}, {'3', 0x73CF}, {'4', 0x5BC9},
    {'5', 0x79CF}, {'6', 0x79EF}, {'7', 0x7249}, {'8', 0x7BEF}, {'9', 0x7BCF},
    {'A', 0x7BE9}, {'B', 0x77AF}, {'C', 0x7927}, {'D', 0x76AF}, {'E', 0x79E7},
    {'F', 0x79E1}, {'G', 0x796F}, {'H', 0x5BE9}, {'I', 0x7497}, {'J', 0x124E},
    {'K', 0x5BA9}, {'L', 0x4927}, {'M', 0x5FE9}, {'N', 0x5F69}, {'O', 0x7B6F},
    {'P', 0x7BE1}, {'Q', 0x7B77}, {'R', 0x7BE9}, {'S', 0x79CF}, {'T', 0x7492},
    {'U', 0x5B6F}, {'V', 0x5B64}, {'W', 0x5BEF}, {'X', 0x5AAD}, {'Y', 0x5BA4},
    {'Z', 0x73E7}, {' ', 0x0000}, {':', 0x0410}, {'-', 0x01C0}, {'/', 0x1248},
    {'!', 0x4904}, {'x', 0x0528}
};

void BitmapFont::drawText(Graphics::Renderer& renderer, const std::string& text, int x, int y, int scale, uint8_t r, uint8_t g, uint8_t b) {
    SDL_Renderer* sdl = renderer.getSDLRenderer();
    if (!sdl) return;

    SDL_SetRenderDrawColor(sdl, r, g, b, 255);
    int curX = x;

    for (char ch : text) {
        char upper = (ch >= 'a' && ch <= 'z') ? (ch - 32) : ch;
        auto it = FONT_3X5.find(upper);
        uint16_t glyph = (it != FONT_3X5.end()) ? it->second : 0x0000;

        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 3; ++col) {
                int bitIndex = 14 - (row * 3 + col);
                if ((glyph >> bitIndex) & 1) {
                    SDL_Rect rect = {curX + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(sdl, &rect);
                }
            }
        }
        curX += 4 * scale; // 3px + 1px espaciado
    }
}

void HUD::render(Graphics::Renderer& renderer, int score, int lives, int bombs, int sector, float bossHpPct) {
    // 1. Puntuación en la esquina superior izquierda
    BitmapFont::drawText(renderer, "SCORE:" + std::to_string(score), 24, 20, 2, 255, 255, 255);

    // 2. Sector actual
    BitmapFont::drawText(renderer, "SECTOR " + std::to_string(sector), Graphics::SCREEN_WIDTH / 2 - 40, 20, 2, 60, 220, 255);

    // 3. Vidas y Bombas en la esquina superior derecha
    std::string livesStr = "LIVES:" + std::to_string(lives);
    std::string bombsStr = "BOMBS:" + std::to_string(bombs);
    BitmapFont::drawText(renderer, livesStr, Graphics::SCREEN_WIDTH - 180, 20, 2, 255, 80, 80);
    BitmapFont::drawText(renderer, bombsStr, Graphics::SCREEN_WIDTH - 180, 36, 2, 255, 200, 40);

    // 4. Barra de Boss si está presente
    if (bossHpPct >= 0.0f) {
        BitmapFont::drawText(renderer, "BOSS WARNING", Graphics::SCREEN_WIDTH / 2 - 55, 42, 2, 255, 60, 60);
    }
}

} // namespace UI
