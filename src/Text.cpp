#include "Text.hpp"
#include "FontData.hpp"
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace UI {

using Graphics::Color;
using Graphics::Renderer;

static constexpr int ATLAS_COLS = 16;
static Graphics::TextureType* s_atlas = nullptr;
static int s_atlasW = 0, s_atlasH = 0;
static std::unordered_map<uint32_t, int> s_index;

void initFont(Renderer& r) {
    int rows = (FONT_GLYPH_COUNT + ATLAS_COLS - 1) / ATLAS_COLS;
    PixelArt::RawTexture raw;
    raw.width = s_atlasW = ATLAS_COLS * GLYPH;
    raw.height = s_atlasH = rows * GLYPH;
    raw.pixels.assign(raw.width * raw.height, 0);
    for (int i = 0; i < FONT_GLYPH_COUNT; ++i) {
        s_index[FONT_GLYPHS[i].cp] = i;
        int ox = (i % ATLAS_COLS) * GLYPH, oy = (i / ATLAS_COLS) * GLYPH;
        for (int y = 0; y < GLYPH; ++y) {
            for (int x = 0; x < GLYPH; ++x) {
                if (FONT_GLYPHS[i].rows[y] & (0x80 >> x)) raw.pixels[(oy + y) * raw.width + ox + x] = 0xFFFFFFFFu;
            }
        }
    }
    s_atlas = r.createTextureFromRaw(raw);
}

void freeFont(Renderer& r) {
    r.freeTexture(s_atlas);
    s_atlas = nullptr;
}

static std::vector<uint32_t> decode(const std::string& s) {
    std::vector<uint32_t> out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size();) {
        unsigned char c = s[i];
        uint32_t cp = c;
        int len = 1;
        if (c >= 0xF0) { cp = c & 0x07; len = 4; }
        else if (c >= 0xE0) { cp = c & 0x0F; len = 3; }
        else if (c >= 0xC0) { cp = c & 0x1F; len = 2; }
        for (int k = 1; k < len && i + k < s.size(); ++k) cp = (cp << 6) | (s[i + k] & 0x3F);
        out.push_back(cp);
        i += len;
    }
    return out;
}

float lineHeight(float scale) {
    return GLYPH * scale;
}

static std::vector<std::vector<uint32_t>> splitLines(const std::string& s) {
    std::vector<std::vector<uint32_t>> lines(1);
    for (uint32_t cp : decode(s)) {
        if (cp == '\n') lines.emplace_back();
        else lines.back().push_back(cp);
    }
    return lines;
}

float textWidth(const std::string& s, float scale) {
    size_t widest = 0;
    for (const auto& l : splitLines(s)) widest = std::max(widest, l.size());
    return widest * GLYPH * scale;
}

static void emit(Renderer& r, const std::vector<std::vector<uint32_t>>& lines, float x, float y, float scale, Color c, Align align, float lineGap) {
    static std::vector<float> xy, uv;
    xy.clear();
    uv.clear();
    float g = GLYPH * scale;
    for (size_t li = 0; li < lines.size(); ++li) {
        const auto& line = lines[li];
        float w = line.size() * g;
        float lx = align == Align::CENTER ? x - w / 2 : align == Align::RIGHT ? x - w : x;
        lx = std::round(lx);
        float ly = std::round(y + li * (g + lineGap));
        for (size_t i = 0; i < line.size(); ++i) {
            auto it = s_index.find(line[i]);
            if (it == s_index.end() || line[i] == ' ') continue;
            int idx = it->second;
            float u0 = (float)((idx % ATLAS_COLS) * GLYPH) / s_atlasW, v0 = (float)((idx / ATLAS_COLS) * GLYPH) / s_atlasH;
            float u1 = u0 + (float)GLYPH / s_atlasW, v1 = v0 + (float)GLYPH / s_atlasH;
            float x0 = lx + i * g, y0 = ly, x1 = x0 + g, y1 = y0 + g;
            float q[12] = {x0, y0, x1, y0, x0, y1, x1, y0, x1, y1, x0, y1};
            float t[12] = {u0, v0, u1, v0, u0, v1, u1, v0, u1, v1, u0, v1};
            xy.insert(xy.end(), q, q + 12);
            uv.insert(uv.end(), t, t + 12);
        }
    }
    r.drawTexturedTriangles(s_atlas, xy.data(), uv.data(), (int)xy.size() / 2, c);
}

void drawText(Renderer& r, const std::string& s, float x, float y, float scale, Color c, Align align, bool shadow) {
    if (!s_atlas || s.empty() || c.a == 0) return;
    auto lines = splitLines(s);
    float gap = 4.0f * scale / 2.0f;
    if (shadow) {
        float o = std::max(1.0f, std::round(scale));
        emit(r, lines, x + o, y + o, scale, Color{0, 0, 0, (uint8_t)(c.a * 0.85f)}, align, gap);
    }
    emit(r, lines, x, y, scale, c, align, gap);
}

std::vector<std::string> wrap(const std::string& s, float scale, float maxWidth) {
    std::vector<std::string> out;
    int maxChars = std::max(1, (int)(maxWidth / (GLYPH * scale)));
    size_t start = 0;
    while (start <= s.size()) {
        size_t nl = s.find('\n', start);
        std::string para = s.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
        std::string line;
        int lineLen = 0;
        size_t i = 0;
        while (i < para.size()) {
            size_t sp = para.find(' ', i);
            std::string word = para.substr(i, sp == std::string::npos ? std::string::npos : sp - i);
            int wl = (int)decode(word).size();
            if (lineLen > 0 && lineLen + 1 + wl > maxChars) {
                out.push_back(line);
                line.clear();
                lineLen = 0;
            }
            if (lineLen > 0) { line += ' '; lineLen++; }
            line += word;
            lineLen += wl;
            if (sp == std::string::npos) break;
            i = sp + 1;
        }
        out.push_back(line);
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return out;
}

float drawWrapped(Renderer& r, const std::string& s, float x, float y, float scale, float maxWidth, Color c, Align align, float lineGap) {
    auto lines = wrap(s, scale, maxWidth);
    float lh = GLYPH * scale + lineGap;
    for (size_t i = 0; i < lines.size(); ++i) drawText(r, lines[i], x, y + i * lh, scale, c, align);
    return lines.size() * lh - lineGap;
}

void drawTitle(Renderer& r, const std::string& s, float cx, float y, float scale, uint32_t top, uint32_t mid, uint32_t shadow) {
    if (!s_atlas) return;
    auto lines = splitLines(s);
    float o = std::max(2.0f, std::round(scale * 0.75f));
    emit(r, lines, cx + o, y + o, scale, Graphics::rgb(0x000000, 200), Align::CENTER, 0);
    emit(r, lines, cx + o * 0.5f, y + o * 0.5f, scale, Graphics::rgb(shadow), Align::CENTER, 0);
    emit(r, lines, cx, y, scale, Graphics::rgb(mid), Align::CENTER, 0);
    // Upper half in the light tone.
    r.setClip(0, (int)y, Graphics::SCREEN_WIDTH, (int)(GLYPH * scale * 0.45f));
    emit(r, lines, cx, y, scale, Graphics::rgb(top), Align::CENTER, 0);
    r.clearClip();
}

uint32_t darken(uint32_t color, float amount) {
    float k = 1.0f - amount;
    uint32_t rr = (uint32_t)(((color >> 16) & 255) * k), gg = (uint32_t)(((color >> 8) & 255) * k), bb = (uint32_t)((color & 255) * k);
    return (rr << 16) | (gg << 8) | bb;
}

static void notched(Renderer& r, float x, float y, float w, float h, float px, Color c) {
    r.drawRect(x + px, y, w - px * 2, h, c);
    r.drawRect(x, y + px, px, h - px * 2, c);
    r.drawRect(x + w - px, y + px, px, h - px * 2, c);
}

void drawButton(Renderer& r, float cx, float cy, float w, float h, const std::string& label, uint32_t accent, bool focused, bool selected, float fontScale, uint8_t alpha) {
    float x = std::round(cx - w / 2), y = std::round(cy - h / 2);
    float px = std::max(2.0f, std::round(h / 16));
    uint32_t border = (focused || selected) ? accent : 0x8892b0;
    uint32_t fill = selected ? darken(accent, 0.7f) : focused ? darken(accent, 0.85f) : 0x0b1022;
    notched(r, x + px, y + px, w, h, px, Graphics::rgb(0x000000, (uint8_t)(150 * alpha / 255)));
    notched(r, x, y, w, h, px, Graphics::rgb(border, alpha));
    notched(r, x + px, y + px, w - px * 2, h - px * 2, px, Graphics::rgb(fill, alpha));
    if (focused) {
        // Cursor marker on the left edge.
        float ms = std::max(4.0f, h * 0.18f);
        float tri[6] = {x - ms * 2.2f, cy - ms, x - ms * 0.6f, cy, x - ms * 2.2f, cy + ms};
        r.fillTriangles(tri, 3, Graphics::rgb(accent, alpha));
    }
    auto lines = splitLines(label);
    float th = lines.size() * GLYPH * fontScale + (lines.size() - 1) * 2.0f * fontScale;
    drawText(r, label, cx, cy - th / 2, fontScale, Graphics::rgb(0xffffff, alpha), Align::CENTER);
}

} // namespace UI
