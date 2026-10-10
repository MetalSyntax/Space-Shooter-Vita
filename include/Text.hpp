#pragma once

#include <string>
#include <vector>
#include "Renderer.hpp"

/** Press Start 2P bitmap font (baked 8x8 atlas) and the pixel-art widgets built on it. */
namespace UI {

enum class Align { LEFT, CENTER, RIGHT };

constexpr int GLYPH = 8;

void initFont(Graphics::Renderer& r);
void freeFont(Graphics::Renderer& r);

/** Width in pixels of the widest line. */
float textWidth(const std::string& s, float scale);
float lineHeight(float scale);

/** Draws UTF-8 text (multi-line with '\n'); x is the left edge, center or right edge depending on align. */
void drawText(Graphics::Renderer& r, const std::string& s, float x, float y, float scale,
              Graphics::Color c = Graphics::Color{}, Align align = Align::LEFT, bool shadow = true);

/** Word-wraps to maxWidth pixels. */
std::vector<std::string> wrap(const std::string& s, float scale, float maxWidth);

/** Draws wrapped text, returns its height. */
float drawWrapped(Graphics::Renderer& r, const std::string& s, float x, float y, float scale, float maxWidth,
                  Graphics::Color c, Align align = Align::LEFT, float lineGap = 4.0f);

/** Big title with a 3-tone vertical gradient (like pixelTitle in the Phaser build). */
void drawTitle(Graphics::Renderer& r, const std::string& s, float cx, float y, float scale,
               uint32_t top, uint32_t mid, uint32_t shadow);

/** Chunky notched pixel button. */
void drawButton(Graphics::Renderer& r, float cx, float cy, float w, float h, const std::string& label,
                uint32_t accent, bool focused, bool selected, float fontScale = 2.0f, uint8_t alpha = 255);

uint32_t darken(uint32_t color, float amount);

} // namespace UI
