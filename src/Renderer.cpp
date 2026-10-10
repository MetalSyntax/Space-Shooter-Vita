#include "Renderer.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#ifdef __vita__
#include <psp2/power.h>
#include <cstring>
#endif

namespace Graphics {

Renderer& Renderer::instance() {
    static Renderer s_instance;
    return s_instance;
}

bool Renderer::init(const std::string& title) {
#ifdef __vita__
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    // Inicializar vitaGL vendorizado con 8 MB de vram
    vglInitExtended(0, SCREEN_WIDTH, SCREEN_HEIGHT, 8 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(15.0f / 255.0f, 23.0f / 255.0f, 42.0f / 255.0f, 1.0f);
    return true;
#else
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return false;
    }

    sdlWindow = SDL_CreateWindow(title.c_str(),
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 SCREEN_WIDTH, SCREEN_HEIGHT,
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!sdlWindow) return false;

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) return false;
    SDL_RenderSetLogicalSize(sdlRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    return true;
#endif
}

void Renderer::shutdown() {
#ifdef __vita__
    // vitaGL se limpia al salir el proceso sceKernelExitProcess
#else
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = nullptr;
    }
    SDL_Quit();
#endif
}

void Renderer::clear(uint8_t r, uint8_t g, uint8_t b) {
#ifdef __vita__
    glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    if (!sdlRenderer) return;
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderClear(sdlRenderer);
#endif
}

void Renderer::present() {
#ifdef __vita__
    vglSwapBuffers(GL_TRUE);
#else
    if (!sdlRenderer) return;
    SDL_RenderPresent(sdlRenderer);
#endif
}

TextureType* Renderer::createTextureFromRaw(const PixelArt::RawTexture& raw) {
    if (raw.width <= 0 || raw.height <= 0) return nullptr;

#ifdef __vita__
    TextureType* tex = new TextureType();
    tex->width = raw.width;
    tex->height = raw.height;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Los píxeles raw son uint32_t RGBA (o ABGR). Subir a OpenGL como GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raw.width, raw.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw.pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
#else
    if (!sdlRenderer) return nullptr;
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        const_cast<uint32_t*>(raw.pixels.data()),
        raw.width, raw.height, 32,
        raw.width * sizeof(uint32_t),
        SDL_PIXELFORMAT_ABGR8888
    );
    if (!surf) return nullptr;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
    SDL_FreeSurface(surf);
    if (tex) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
#endif
}

void Renderer::freeTexture(TextureType* tex) {
    if (!tex) return;
#ifdef __vita__
    if (tex->id != 0) {
        glDeleteTextures(1, &tex->id);
    }
    delete tex;
#else
    SDL_DestroyTexture(tex);
#endif
}

int Renderer::texWidth(TextureType* tex) {
    if (!tex) return 0;
#ifdef __vita__
    return tex->width;
#else
    int w = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, nullptr);
    return w;
#endif
}

int Renderer::texHeight(TextureType* tex) {
    if (!tex) return 0;
#ifdef __vita__
    return tex->height;
#else
    int h = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, nullptr, &h);
    return h;
#endif
}

void Renderer::drawTexture(TextureType* tex, int x, int y, double angleDeg, float scale, bool centered) {
    if (!tex) return;
    float fx = (float)x, fy = (float)y;
    if (!centered) {
        fx += texWidth(tex) * scale * 0.5f;
        fy += texHeight(tex) * scale * 0.5f;
    }
    drawSprite(tex, fx, fy, (float)angleDeg, scale, scale);
}

void Renderer::drawSprite(TextureType* tex, float x, float y, float angleDeg, float sx, float sy, Color tint, bool additive) {
    if (!tex || tint.a == 0) return;
    x += offX;
    y += offY;

#ifdef __vita__
    float w = (float)tex->width * sx;
    float h = (float)tex->height * sy;
    float x0 = -w * 0.5f, y0 = -h * 0.5f, x1 = w * 0.5f, y1 = h * 0.5f;
    float vertices[8] = {x0, y0, x1, y0, x0, y1, x1, y1};
    float texCoords[8] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

    if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glColor4f(tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f, tint.a / 255.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    if (angleDeg != 0.0f) glRotatef(angleDeg, 0.0f, 0.0f, 1.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
    if (!sdlRenderer) return;
    int tw = 0, th = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
    float w = tw * std::fabs(sx), h = th * std::fabs(sy);
    SDL_FRect dst = {x - w / 2.0f, y - h / 2.0f, w, h};
    int flip = SDL_FLIP_NONE;
    if (sx < 0) flip |= SDL_FLIP_HORIZONTAL;
    if (sy < 0) flip |= SDL_FLIP_VERTICAL;
    SDL_SetTextureColorMod(tex, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(tex, tint.a);
    if (additive) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
    SDL_RenderCopyExF(sdlRenderer, tex, nullptr, &dst, angleDeg, nullptr, (SDL_RendererFlip)flip);
    if (additive) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex, 255);
#endif
}

void Renderer::drawTexturedTriangles(TextureType* tex, const float* xy, const float* uv, int vertCount, Color tint) {
    if (!tex || vertCount <= 0) return;
#ifdef __vita__
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glColor4f(tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f, tint.a / 255.0f);
    // Only pay for the matrix push while the screen is shaking.
    bool shifted = offX != 0.0f || offY != 0.0f;
    if (shifted) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(offX, offY, 0.0f);
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, xy);
    glTexCoordPointer(2, GL_FLOAT, 0, uv);
    glDrawArrays(GL_TRIANGLES, 0, vertCount);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    if (shifted) glPopMatrix();
    glDisable(GL_TEXTURE_2D);
#else
    if (!sdlRenderer) return;
    std::vector<SDL_Vertex> verts(vertCount);
    for (int i = 0; i < vertCount; ++i) {
        verts[i].position = {xy[i * 2] + offX, xy[i * 2 + 1] + offY};
        verts[i].color = {tint.r, tint.g, tint.b, tint.a};
        verts[i].tex_coord = {uv[i * 2], uv[i * 2 + 1]};
    }
    SDL_RenderGeometry(sdlRenderer, tex, verts.data(), vertCount, nullptr, 0);
#endif
}

void Renderer::drawBackgroundTiled(TextureType* tex, int scrollX, int scrollY, uint8_t alpha) {
    if (!tex) return;
    int tw = texWidth(tex);
    int th = texHeight(tex);
    if (tw <= 0 || th <= 0) return;

    int startX = (scrollX % tw) - tw;
    int startY = (scrollY % th) - th;

#ifdef __vita__
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glColor4f(1.0f, 1.0f, 1.0f, alpha / 255.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    float texCoords[8] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    for (int y = startY; y < SCREEN_HEIGHT + th; y += th) {
        for (int x = startX; x < SCREEN_WIDTH + tw; x += tw) {
            float vertices[8] = {
                (float)x, (float)y,
                (float)(x + tw), (float)y,
                (float)x, (float)(y + th),
                (float)(x + tw), (float)(y + th)
            };
            glVertexPointer(2, GL_FLOAT, 0, vertices);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
#else
    if (!sdlRenderer) return;
    SDL_SetTextureAlphaMod(tex, alpha);
    for (int y = startY; y < SCREEN_HEIGHT + th; y += th) {
        for (int x = startX; x < SCREEN_WIDTH + tw; x += tw) {
            SDL_Rect dstRect = {x, y, tw, th};
            SDL_RenderCopy(sdlRenderer, tex, nullptr, &dstRect);
        }
    }
    SDL_SetTextureAlphaMod(tex, 255);
#endif
}

void Renderer::drawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    drawRect((float)x, (float)y, (float)w, (float)h, Color{r, g, b, a});
}

void Renderer::drawRect(float x, float y, float w, float h, Color c) {
    if (c.a == 0 || w <= 0 || h <= 0) return;
    float v[12] = {x, y, x + w, y, x, y + h, x + w, y, x + w, y + h, x, y + h};
    fillTriangles(v, 6, c);
}

void Renderer::drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    drawRect(x, y, 1, 1, r, g, b, a);
}

void Renderer::fillTriangles(const float* xy, int vertCount, Color c, bool additive) {
    if (vertCount <= 0 || c.a == 0) return;
#ifdef __vita__
    if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_TEXTURE_2D);
    glColor4f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
    // Only pay for the matrix push while the screen is shaking.
    bool shifted = offX != 0.0f || offY != 0.0f;
    if (shifted) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(offX, offY, 0.0f);
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, xy);
    glDrawArrays(GL_TRIANGLES, 0, vertCount);
    glDisableClientState(GL_VERTEX_ARRAY);
    if (shifted) glPopMatrix();
    if (additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
    if (!sdlRenderer) return;
    std::vector<SDL_Vertex> verts(vertCount);
    for (int i = 0; i < vertCount; ++i) {
        verts[i].position = {xy[i * 2] + offX, xy[i * 2 + 1] + offY};
        verts[i].color = {c.r, c.g, c.b, c.a};
        verts[i].tex_coord = {0, 0};
    }
    SDL_SetRenderDrawBlendMode(sdlRenderer, additive ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(sdlRenderer, nullptr, verts.data(), vertCount, nullptr, 0);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
#endif
}

void Renderer::fillCircle(float cx, float cy, float radius, Color c, bool additive) {
    if (radius <= 0) return;
    const int seg = radius < 12 ? 12 : 24;
    float v[24 * 6];
    for (int i = 0; i < seg; ++i) {
        float a0 = 6.2831853f * i / seg, a1 = 6.2831853f * (i + 1) / seg;
        v[i * 6 + 0] = cx; v[i * 6 + 1] = cy;
        v[i * 6 + 2] = cx + std::cos(a0) * radius; v[i * 6 + 3] = cy + std::sin(a0) * radius;
        v[i * 6 + 4] = cx + std::cos(a1) * radius; v[i * 6 + 5] = cy + std::sin(a1) * radius;
    }
    fillTriangles(v, seg * 3, c, additive);
}

void Renderer::drawArc(float cx, float cy, float radius, float thickness, float startRad, float endRad, Color c) {
    if (radius <= 0 || endRad <= startRad) return;
    const int seg = 32;
    float v[32 * 12];
    float ri = radius - thickness * 0.5f, ro = radius + thickness * 0.5f;
    for (int i = 0; i < seg; ++i) {
        float a0 = startRad + (endRad - startRad) * i / seg;
        float a1 = startRad + (endRad - startRad) * (i + 1) / seg;
        float c0 = std::cos(a0), s0 = std::sin(a0), c1 = std::cos(a1), s1 = std::sin(a1);
        float* q = v + i * 12;
        q[0] = cx + c0 * ri; q[1] = cy + s0 * ri;
        q[2] = cx + c0 * ro; q[3] = cy + s0 * ro;
        q[4] = cx + c1 * ro; q[5] = cy + s1 * ro;
        q[6] = cx + c0 * ri; q[7] = cy + s0 * ri;
        q[8] = cx + c1 * ro; q[9] = cy + s1 * ro;
        q[10] = cx + c1 * ri; q[11] = cy + s1 * ri;
    }
    fillTriangles(v, seg * 6, c);
}

void Renderer::drawCircle(float cx, float cy, float radius, float thickness, Color c) {
    drawArc(cx, cy, radius, thickness, 0.0f, 6.2831853f, c);
}

void Renderer::drawLine(float x0, float y0, float x1, float y1, float width, Color c, bool additive) {
    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.0f) return;
    float nx = -dy / len * width * 0.5f, ny = dx / len * width * 0.5f;
    float v[12] = {x0 + nx, y0 + ny, x1 + nx, y1 + ny, x1 - nx, y1 - ny,
                   x0 + nx, y0 + ny, x1 - nx, y1 - ny, x0 - nx, y0 - ny};
    fillTriangles(v, 6, c, additive);
}

void Renderer::setClip(int x, int y, int w, int h) {
#ifdef __vita__
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, SCREEN_HEIGHT - y - h, w, h);
#else
    if (!sdlRenderer) return;
    SDL_Rect r = {x, y, w, h};
    SDL_RenderSetClipRect(sdlRenderer, &r);
#endif
}

void Renderer::clearClip() {
#ifdef __vita__
    glDisable(GL_SCISSOR_TEST);
#else
    if (!sdlRenderer) return;
    SDL_RenderSetClipRect(sdlRenderer, nullptr);
#endif
}

} // namespace Graphics
