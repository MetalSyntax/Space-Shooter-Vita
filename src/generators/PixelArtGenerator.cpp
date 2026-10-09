#include "generators/PixelArtGenerator.hpp"
#include <algorithm>
#include <stack>
#include <cmath>

namespace PixelArt {

const std::unordered_map<std::string, Palette> PALETTES = {
    {"player",   {0, 0x05070f, 0x1d3a8a, 0x3f6fe0, 0xdfe8ff, 0x5cf2ff, 0xff9a2e, 0xff3b3b}},
    {"military", {0, 0x070b14, 0x1f3a5f, 0x3d6a99, 0x9fb4c8, 0x40e0ff, 0xff9933, 0x6b7a8f}},
    {"nebula",   {0, 0x0d0614, 0x3d1a66, 0x7a3ac9, 0xd6a8ff, 0xff5cf0, 0xffb347, 0x40e0ff}},
    {"copper",   {0, 0x120a05, 0x5a3218, 0xa8642e, 0xf0b27a, 0x5cf2ff, 0xff7a2e, 0x7d7d7d}},
    {"volcanic", {0, 0x1a0805, 0x6b1e0e, 0xb5401c, 0xf08a3c, 0xffe14d, 0xff5522, 0x7a3a22}},
    {"toxic",    {0, 0x050d07, 0x2e1a47, 0x2f8f4e, 0x7cff9b, 0xc6ff3d, 0xb04dff, 0x5a2d82}},
    {"rift",     {0, 0x03121a, 0x0f4a63, 0x1c8fb3, 0x9ff6ff, 0xffffff, 0x3dfcff, 0x7a35ff}},
    {"gold",     {0, 0x0f0c03, 0x6b5310, 0x2f8f4e, 0xffe08a, 0x9dff6e, 0xffb000, 0x1f6b52}},
    {"steel",    {0, 0x08090c, 0x3a3f4a, 0x6e7686, 0xc9d1de, 0xff3344, 0xff9933, 0xffd23d}},
    {"mono",     {0, 0x000000, 0x3a3a3a, 0x8a8a8a, 0xf2f2f2, 0xffffff, 0xbbbbbb, 0x5a5a5a}},
    {"cyber",    {0, 0x10061a, 0x4a1670, 0xc0209f, 0xff7ae0, 0x3dfcff, 0xffffff, 0x7a35ff}},
    {"rock",     {0, 0x0b0907, 0x3e3329, 0x6e5e4c, 0xa89880, 0x6e5e4c, 0x3e3329, 0x8a7a66}}
};

PRNG::PRNG(uint32_t seed) : state(seed) {}

float PRNG::nextFloat() {
    state = (state + 0x6d2b79f5u);
    uint32_t t = state;
    t = (t ^ (t >> 15)) * (t | 1u);
    t ^= t + (t ^ (t >> 7)) * (t | 61u);
    uint32_t res = t ^ (t >> 14);
    return static_cast<float>(res) / 4294967296.0f;
}

uint32_t hashString(const std::string& s) {
    uint32_t h = 2166136261u;
    for (char c : s) {
        h = (h ^ static_cast<uint8_t>(c)) * 16777619u;
    }
    return h;
}

Grid makeGrid(int w, int h, uint8_t fill) {
    return Grid(h, std::vector<uint8_t>(w, fill));
}

Grid mirror(const Grid& g) {
    if (g.empty()) return g;
    int h = static_cast<int>(g.size());
    int w = static_cast<int>(g[0].size());
    Grid out = g;
    int half = (w + 1) / 2;
    for (int y = 0; y < h; ++y) {
        for (int x = half; x < w; ++x) {
            out[y][x] = out[y][w - 1 - x];
        }
    }
    return out;
}

Grid flipV(const Grid& g) {
    Grid out = g;
    std::reverse(out.begin(), out.end());
    return out;
}

Grid rotateCW(const Grid& g) {
    if (g.empty()) return g;
    int h = static_cast<int>(g.size());
    int w = static_cast<int>(g[0].size());
    Grid out = makeGrid(h, w, EMPTY);
    for (int y = 0; y < w; ++y) {
        for (int x = 0; x < h; ++x) {
            out[y][x] = g[h - 1 - x][y];
        }
    }
    return out;
}

Grid keepConnected(const Grid& g, int sx, int sy) {
    if (g.empty()) return g;
    int h = static_cast<int>(g.size());
    int w = static_cast<int>(g[0].size());
    Grid out = makeGrid(w, h, EMPTY);
    if (sy < 0 || sy >= h || sx < 0 || sx >= w || g[sy][sx] == EMPTY) {
        return g;
    }
    std::stack<std::pair<int, int>> st;
    st.push({sx, sy});
    while (!st.empty()) {
        auto [cx, cy] = st.top();
        st.pop();
        if (cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
        if (g[cy][cx] == EMPTY || out[cy][cx] != EMPTY) continue;
        out[cy][cx] = g[cy][cx];
        st.push({cx + 1, cy});
        st.push({cx - 1, cy});
        st.push({cx, cy + 1});
        st.push({cx, cy - 1});
    }
    return out;
}

Grid shade(const Grid& g) {
    if (g.empty()) return g;
    int h = static_cast<int>(g.size());
    int w = static_cast<int>(g[0].size());
    Grid out = g;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (out[y][x] != BODY) continue;
            bool noTop = (y == 0 || g[y - 1][x] == EMPTY);
            bool noBottom = (y == h - 1 || g[y + 1][x] == EMPTY);
            if (noTop) out[y][x] = LIGHT;
            else if (noBottom) out[y][x] = SHADOW;
        }
    }
    return out;
}

Grid outline(const Grid& g) {
    if (g.empty()) return g;
    int h = static_cast<int>(g.size());
    int w = static_cast<int>(g[0].size());
    int nh = h + 2;
    int nw = w + 2;
    Grid p = makeGrid(nw, nh, EMPTY);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            p[y + 1][x + 1] = g[y][x];
        }
    }
    Grid out = p;
    for (int y = 0; y < nh; ++y) {
        for (int x = 0; x < nw; ++x) {
            if (p[y][x] != EMPTY) continue;
            bool near = (y > 0 && p[y - 1][x] != EMPTY) ||
                        (y + 1 < nh && p[y + 1][x] != EMPTY) ||
                        (x > 0 && p[y][x - 1] != EMPTY) ||
                        (x + 1 < nw && p[y][x + 1] != EMPTY);
            if (near) out[y][x] = OUTLINE;
        }
    }
    return out;
}

Grid fromMask(const std::vector<std::string>& lines) {
    int h = static_cast<int>(lines.size());
    int w = h > 0 ? static_cast<int>(lines[0].size()) : 0;
    Grid out = makeGrid(w, h, EMPTY);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < static_cast<int>(lines[y].size()); ++x) {
            char c = lines[y][x];
            switch (c) {
                case '#': out[y][x] = BODY; break;
                case 'o': out[y][x] = CORE; break;
                case 'e': out[y][x] = ENGINE; break;
                case 'a': out[y][x] = ACCENT; break;
                case 's': out[y][x] = SHADOW; break;
                case 'l': out[y][x] = LIGHT; break;
                default:  out[y][x] = EMPTY; break;
            }
        }
    }
    return out;
}

Grid mirrorMask(const std::vector<std::string>& lines, bool odd) {
    Grid half = fromMask(lines);
    if (half.empty()) return half;
    int h = static_cast<int>(half.size());
    int hw = static_cast<int>(half[0].size());
    int nw = odd ? (hw * 2 - 1) : (hw * 2);
    Grid out = makeGrid(nw, h, EMPTY);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < hw; ++x) {
            out[y][x] = half[y][x];
        }
        for (int x = 0; x < hw; ++x) {
            int targetX = nw - 1 - x;
            out[y][targetX] = half[y][x];
        }
    }
    return out;
}

Grid crop(const Grid& g, int x, int y, int w, int h) {
    Grid out = makeGrid(w, h, EMPTY);
    int gh = static_cast<int>(g.size());
    for (int dy = 0; dy < h; ++dy) {
        int sy = y + dy;
        if (sy < 0 || sy >= gh) continue;
        int gw = static_cast<int>(g[sy].size());
        for (int dx = 0; dx < w; ++dx) {
            int sx = x + dx;
            if (sx >= 0 && sx < gw) {
                out[dy][dx] = g[sy][sx];
            }
        }
    }
    return out;
}

Grid downsample(const Grid& g, int f) {
    if (g.empty() || f <= 1) return g;
    int h = static_cast<int>(g.size());
    int w = static_cast<int>(g[0].size());
    Grid out;
    for (int y = 0; y < h; y += f) {
        std::vector<uint8_t> row;
        for (int x = 0; x < w; x += f) {
            row.push_back(g[y][x]);
        }
        out.push_back(row);
    }
    return out;
}

Grid generateShip(uint32_t seed, int w, int h, float density) {
    PRNG rnd(seed);
    int half = (w + 1) / 2;
    Grid g = makeGrid(w, h, EMPTY);
    int wingRow = static_cast<int>(h * (0.3f + rnd.nextFloat() * 0.4f));
    int wingSpan = 1 + static_cast<int>(rnd.nextFloat() * 3.0f);
    float taper = 0.5f + rnd.nextFloat() * 0.6f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < half; ++x) {
            float fromCenter = static_cast<float>(half - 1 - x) / half;
            float ny = static_cast<float>(y) / (h - 1);
            if (x == half - 1 && y > 0) {
                g[y][x] = BODY;
                continue;
            }
            float nose = ny < 0.25f ? fromCenter * 2.2f : 0.0f;
            float wing = std::abs(y - wingRow) < wingSpan ? 0.6f : 0.0f;
            float p = density - fromCenter * taper * 1.4f - nose + wing - std::abs(ny - 0.55f) * 0.6f;
            if (rnd.nextFloat() < p) {
                g[y][x] = BODY;
            }
        }
    }

    Grid s = keepConnected(mirror(g), half - 1, h / 2);
    s = shade(s);

    int cy = static_cast<int>(h * 0.5f);
    int spineCols[2] = {half - 1, w - half};
    for (int x : spineCols) {
        if (s[cy][x] != EMPTY) s[cy][x] = CORE;
        int tail = h - 1;
        while (tail > 0 && s[tail][x] == EMPTY) tail--;
        if (tail > 0) s[tail][x] = ENGINE;
    }

    if (rnd.nextFloat() < 0.6f && wingRow < h && s[wingRow][1] != EMPTY) {
        s[wingRow][1] = ACCENT;
        s[wingRow][w - 2] = ACCENT;
    }

    return outline(s);
}

Grid generateAsteroid(uint32_t seed, int size) {
    PRNG rnd(seed);
    float r = size / 2.0f;
    float bumps[8];
    for (int i = 0; i < 8; ++i) bumps[i] = 0.78f + rnd.nextFloat() * 0.22f;
    Grid g = makeGrid(size, size, EMPTY);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = x + 0.5f - r, dy = y + 0.5f - r;
            float a = (std::atan2(dy, dx) + M_PI) / (M_PI * 2.0f) * 8.0f;
            int i = static_cast<int>(std::floor(a)) % 8;
            float frac = a - std::floor(a);
            float edge = r * (bumps[i] + (bumps[(i + 1) % 8] - bumps[i]) * frac);
            if (std::hypot(dx, dy) < edge) {
                g[y][x] = (dx + dy < -r * 0.4f) ? LIGHT : (dx + dy > r * 0.5f ? SHADOW : BODY);
            }
        }
    }

    int craters = std::max(1, size / 8);
    for (int c = 0; c < craters; ++c) {
        int cx = static_cast<int>(r * 0.5f + rnd.nextFloat() * r);
        int cy = static_cast<int>(r * 0.5f + rnd.nextFloat() * r);
        if (cy < size && cx < size && g[cy][cx] != EMPTY) g[cy][cx] = SHADOW;
        if (cy < size && cx + 1 < size && g[cy][cx + 1] != EMPTY) g[cy][cx + 1] = SHADOW;
    }

    return outline(g);
}

Grid generateOrb(int size, bool ring) {
    float r = size / 2.0f;
    Grid g = makeGrid(size, size, EMPTY);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float d = std::hypot(x + 0.5f - r, y + 0.5f - r);
            if (d < r) {
                g[y][x] = (ring && d < r - 2.0f) ? EMPTY :
                          (d < r * 0.45f ? CORE : (d < r * 0.75f ? LIGHT : BODY));
            }
        }
    }
    return outline(g);
}

Grid generateIcon(const std::string& kind) {
    static const std::unordered_map<std::string, std::vector<std::string>> GLYPHS = {
        {"shield",  {".###.", "#...#", "#.#.#", "#...#", ".###."}},
        {"rapid",   {"..#..", ".##..", "#####", "..##.", "..#.."}},
        {"bomb",    {"..#..", ".###.", "#####", ".###.", "..#.."}},
        {"spread",  {"#.#.#", "#.#.#", ".###.", "..#..", "..#.."}},
        {"missile", {"..#..", ".###.", ".###.", "#####", "#.#.#"}},
        {"drone",   {".#.#.", "#####", ".#o#.", "#####", ".#.#."}},
        {"magnet",  {"#...#", "#...#", "#...#", "#...#", ".###."}},
    };

    Grid g = makeGrid(11, 11, EMPTY);
    for (int y = 0; y < 11; ++y) {
        for (int x = 0; x < 11; ++x) {
            float d = std::hypot(x - 5.0f, y - 5.0f);
            if (d <= 5.4f) {
                g[y][x] = d > 4.4f ? LIGHT : BODY;
            }
        }
    }

    auto it = GLYPHS.find(kind);
    const auto& glyph = (it != GLYPHS.end()) ? it->second : GLYPHS.at("bomb");
    for (size_t y = 0; y < glyph.size(); ++y) {
        for (size_t x = 0; x < glyph[y].size(); ++x) {
            char c = glyph[y][x];
            if (c != '.') {
                g[y + 3][x + 3] = (c == 'o') ? ENGINE : CORE;
            }
        }
    }

    return outline(g);
}

Grid generateMissile() {
    Grid m = {
        {ENGINE, SHADOW, BODY, BODY, LIGHT, EMPTY},
        {ENGINE, BODY,   BODY, BODY, LIGHT, CORE},
        {ENGINE, SHADOW, BODY, BODY, LIGHT, EMPTY}
    };
    return outline(m);
}

Palette shiftPalette(const Palette& p, float hueDeg, float sat) {
    if (hueDeg == 0.0f && sat == 1.0f) return p;
    Palette out = p;
    for (size_t i = 1; i < p.size(); ++i) {
        uint32_t c = p[i];
        float r = ((c >> 16) & 255) / 255.0f;
        float g = ((c >> 8) & 255) / 255.0f;
        float b = (c & 255) / 255.0f;

        float max = std::max({r, g, b});
        float min = std::min({r, g, b});
        float l = (max + min) / 2.0f;
        float h = 0.0f, s = 0.0f;

        if (max != min) {
            float d = max - min;
            s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);
            if (max == r) h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            else if (max == g) h = (b - r) / d + 2.0f;
            else h = (r - g) / d + 4.0f;
            h /= 6.0f;
        }

        h = std::fmod((h + hueDeg / 360.0f), 1.0f);
        if (h < 0.0f) h += 1.0f;
        s = std::min(1.0f, s * sat);

        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float pp = 2.0f * l - q;

        auto fn = [pp, q](float t) {
            t = std::fmod(t, 1.0f);
            if (t < 0.0f) t += 1.0f;
            if (t < 1.0f / 6.0f) return pp + (q - pp) * 6.0f * t;
            if (t < 1.0f / 2.0f) return q;
            if (t < 2.0f / 3.0f) return pp + (q - pp) * (2.0f / 3.0f - t) * 6.0f;
            return pp;
        };

        if (s == 0.0f) {
            r = g = b = l;
        } else {
            r = fn(h + 1.0f / 3.0f);
            g = fn(h);
            b = fn(h - 1.0f / 3.0f);
        }

        uint32_t ir = static_cast<uint32_t>(std::round(r * 255.0f));
        uint32_t ig = static_cast<uint32_t>(std::round(g * 255.0f));
        uint32_t ib = static_cast<uint32_t>(std::round(b * 255.0f));
        out[i] = (ir << 16) | (ig << 8) | ib;
    }
    return out;
}

Palette tintPalette(const Palette& p, uint32_t color, float amount) {
    uint32_t cr = (color >> 16) & 255;
    uint32_t cg = (color >> 8) & 255;
    uint32_t cb = color & 255;
    Palette out = p;
    for (size_t i = 1; i < p.size(); ++i) {
        uint32_t c = p[i];
        float r = static_cast<float>((c >> 16) & 255);
        float g = static_cast<float>((c >> 8) & 255);
        float b = static_cast<float>(c & 255);
        float luma = (r * 0.3f + g * 0.59f + b * 0.11f) / 255.0f;

        auto mixVal = [amount, luma](float v, uint32_t target) {
            float t = std::min(255.0f, static_cast<float>(target) * (0.3f + luma * 1.4f));
            return static_cast<uint32_t>(std::round(v + (t - v) * amount));
        };

        uint32_t nr = mixVal(r, cr);
        uint32_t ng = mixVal(g, cg);
        uint32_t nb = mixVal(b, cb);
        out[i] = (nr << 16) | (ng << 8) | nb;
    }
    return out;
}

RawTexture bakeTexture(const Grid& grid, const Palette& palette, int cell) {
    RawTexture tex;
    if (grid.empty()) return tex;
    int gh = static_cast<int>(grid.size());
    int gw = static_cast<int>(grid[0].size());
    tex.width = gw * cell;
    tex.height = gh * cell;
    tex.pixels.assign(tex.width * tex.height, 0x00000000); // transparente por defecto

    for (int gy = 0; gy < gh; ++gy) {
        for (int gx = 0; gx < gw; ++gx) {
            uint8_t role = grid[gy][gx];
            if (role == EMPTY || role >= palette.size()) continue;
            uint32_t rgb = palette[role];
            // Convertimos 0xRRGGBB a 0xAABBGGRR (formato SDL_PIXELFORMAT_ABGR8888 / RGBA nativo)
            uint8_t r = (rgb >> 16) & 255;
            uint8_t g = (rgb >> 8) & 255;
            uint8_t b = rgb & 255;
            uint32_t colorRGBA = (0xFFu << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);

            int startY = gy * cell;
            int startX = gx * cell;
            for (int dy = 0; dy < cell; ++dy) {
                int rowIdx = (startY + dy) * tex.width;
                for (int dx = 0; dx < cell; ++dx) {
                    tex.pixels[rowIdx + (startX + dx)] = colorRGBA;
                }
            }
        }
    }
    return tex;
}

RawTexture generateBackground(uint32_t seed, uint32_t spaceColor, uint32_t nebula1, uint32_t nebula2, int size) {
    PRNG rnd(seed);
    RawTexture tex;
    tex.width = size;
    tex.height = size;
    tex.pixels.assign(size * size, 0);

    uint8_t sr = (spaceColor >> 16) & 255;
    uint8_t sg = (spaceColor >> 8) & 255;
    uint8_t sb = spaceColor & 255;
    uint32_t spaceRGBA = (0xFFu << 24) | (static_cast<uint32_t>(sb) << 16) | (static_cast<uint32_t>(sg) << 8) | static_cast<uint32_t>(sr);
    std::fill(tex.pixels.begin(), tex.pixels.end(), spaceRGBA);

    const int n = 8;
    float lattice[n][n];
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            lattice[j][i] = rnd.nextFloat();
        }
    }

    const int cell = 4;
    auto smooth = [](float t) { return t * t * (3.0f - 2.0f * t); };

    for (int y = 0; y < size; y += cell) {
        for (int x = 0; x < size; x += cell) {
            float fx = (static_cast<float>(x) / size) * n;
            float fy = (static_cast<float>(y) / size) * n;
            int x0 = static_cast<int>(std::floor(fx));
            int y0 = static_cast<int>(std::floor(fy));
            float tx = fx - x0;
            float ty = fy - y0;

            auto v = [&](int i, int j) {
                return lattice[(j % n + n) % n][(i % n + n) % n];
            };

            float top = v(x0, y0) + (v(x0 + 1, y0) - v(x0, y0)) * smooth(tx);
            float bot = v(x0, y0 + 1) + (v(x0 + 1, y0 + 1) - v(x0, y0 + 1)) * smooth(tx);
            float val = top + (bot - top) * smooth(ty);

            if (val > 0.55f) {
                float alpha = std::round((val - 0.55f) * 6.0f) / 6.0f * 0.35f;
                uint32_t nebHex = (val > 0.72f) ? nebula2 : nebula1;
                uint8_t nr = (nebHex >> 16) & 255;
                uint8_t ng = (nebHex >> 8) & 255;
                uint8_t nb = nebHex & 255;

                for (int dy = 0; dy < cell && (y + dy) < size; ++dy) {
                    for (int dx = 0; dx < cell && (x + dx) < size; ++dx) {
                        int idx = (y + dy) * size + (x + dx);
                        uint32_t curr = tex.pixels[idx];
                        uint8_t cr = curr & 255;
                        uint8_t cg = (curr >> 8) & 255;
                        uint8_t cb = (curr >> 16) & 255;

                        uint8_t blendR = static_cast<uint8_t>(cr * (1.0f - alpha) + nr * alpha);
                        uint8_t blendG = static_cast<uint8_t>(cg * (1.0f - alpha) + ng * alpha);
                        uint8_t blendB = static_cast<uint8_t>(cb * (1.0f - alpha) + nb * alpha);

                        tex.pixels[idx] = (0xFFu << 24) | (static_cast<uint32_t>(blendB) << 16) | (static_cast<uint32_t>(blendG) << 8) | static_cast<uint32_t>(blendR);
                    }
                }
            }
        }
    }

    // Estrellas
    for (int i = 0; i < 90; ++i) {
        bool big = rnd.nextFloat() < 0.12f;
        uint32_t starHex = (rnd.nextFloat() < 0.2f) ? nebula2 : 0xFFFFFF;
        uint8_t str = (starHex >> 16) & 255;
        uint8_t stg = (starHex >> 8) & 255;
        uint8_t stb = starHex & 255;
        uint32_t starRGBA = (0xFFu << 24) | (static_cast<uint32_t>(stb) << 16) | (static_cast<uint32_t>(stg) << 8) | static_cast<uint32_t>(str);

        int sx = static_cast<int>(rnd.nextFloat() * size / 2.0f) * 2;
        int sy = static_cast<int>(rnd.nextFloat() * size / 2.0f) * 2;

        auto plot = [&](int px, int py) {
            if (px >= 0 && px < size && py >= 0 && py < size) {
                tex.pixels[py * size + px] = starRGBA;
            }
        };

        plot(sx, sy); plot(sx + 1, sy);
        plot(sx, sy + 1); plot(sx + 1, sy + 1);

        if (big) {
            for (int k = -2; k <= 3; ++k) {
                plot(sx + k, sy);
                plot(sx, sy + k);
            }
        }
    }

    return tex;
}

} // namespace PixelArt
