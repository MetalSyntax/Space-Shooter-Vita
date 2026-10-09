#include "Renderer.hpp"
#include <iostream>
#include <cmath>

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
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    sdlWindow = SDL_CreateWindow(title.c_str(), 
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                 SCREEN_WIDTH, SCREEN_HEIGHT, 
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!sdlWindow) return false;

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) return false;

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

void Renderer::drawTexture(TextureType* tex, int x, int y, double angleDeg, float scale, bool centered) {
    if (!tex) return;

#ifdef __vita__
    float w = (float)tex->width * scale;
    float h = (float)tex->height * scale;
    float x0 = centered ? -w * 0.5f : 0.0f;
    float y0 = centered ? -h * 0.5f : 0.0f;
    float x1 = x0 + w;
    float y1 = y0 + h;

    float vertices[8] = {
        x0, y0,
        x1, y0,
        x0, y1,
        x1, y1
    };

    float texCoords[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef((float)x, (float)y, 0.0f);
    if (angleDeg != 0.0) {
        glRotatef((float)angleDeg, 0.0f, 0.0f, 1.0f);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
#else
    if (!sdlRenderer) return;
    int w = 0, h = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    w = static_cast<int>(w * scale);
    h = static_cast<int>(h * scale);

    SDL_Rect dstRect = {centered ? (x - w / 2) : x, centered ? (y - h / 2) : y, w, h};
    if (angleDeg == 0.0) {
        SDL_RenderCopy(sdlRenderer, tex, nullptr, &dstRect);
    } else {
        SDL_RenderCopyEx(sdlRenderer, tex, nullptr, &dstRect, angleDeg, nullptr, SDL_FLIP_NONE);
    }
#endif
}

void Renderer::drawBackgroundTiled(TextureType* tex, int scrollX, int scrollY) {
    if (!tex) return;

#ifdef __vita__
    int tw = tex->width;
    int th = tex->height;
    if (tw <= 0 || th <= 0) return;

    int startX = (scrollX % tw) - tw;
    int startY = (scrollY % th) - th;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    float texCoords[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
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
    int tw = 0, th = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
    if (tw <= 0 || th <= 0) return;

    int startX = (scrollX % tw) - tw;
    int startY = (scrollY % th) - th;

    for (int y = startY; y < SCREEN_HEIGHT + th; y += th) {
        for (int x = startX; x < SCREEN_WIDTH + tw; x += tw) {
            SDL_Rect dstRect = {x, y, tw, th};
            SDL_RenderCopy(sdlRenderer, tex, nullptr, &dstRect);
        }
    }
#endif
}

void Renderer::drawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef __vita__
    glDisable(GL_TEXTURE_2D);
    glColor4f((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);

    float vertices[8] = {
        (float)x, (float)y,
        (float)(x + w), (float)y,
        (float)x, (float)(y + h),
        (float)(x + w), (float)(y + h)
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
#else
    if (!sdlRenderer) return;
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(sdlRenderer, &rect);
#endif
}

void Renderer::drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
#ifdef __vita__
    drawRect(x, y, 1, 1, r, g, b, a);
#else
    if (!sdlRenderer) return;
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
    SDL_RenderDrawPoint(sdlRenderer, x, y);
#endif
}

} // namespace Graphics
