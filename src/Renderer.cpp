#include "Renderer.hpp"
#include <iostream>

#ifdef __vita__
#include <psp2/power.h>
#endif

namespace Graphics {

Renderer& Renderer::instance() {
    static Renderer s_instance;
    return s_instance;
}

bool Renderer::init(const std::string& title) {
#ifdef __vita__
    // Configurar reloj máximo en PS Vita (444 MHz CPU, 222 MHz GPU) para fluidez absoluta
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    // En PS Vita inicializar solo Video primero para garantizar contexto GXM/EGL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    sdlWindow = SDL_CreateWindow(title.c_str(), 
                                 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                 SCREEN_WIDTH, SCREEN_HEIGHT, 
                                 0);
#else
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    sdlWindow = SDL_CreateWindow(title.c_str(), 
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                 SCREEN_WIDTH, SCREEN_HEIGHT, 
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
#endif

    if (!sdlWindow) {
        return false;
    }

    // En PS Vita SDL2 usa el driver GXM/vitaGL acelerado directamente con flag 0 o ACCELERATED
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
    }

    if (!sdlRenderer) {
        return false;
    }

    SDL_RenderSetLogicalSize(sdlRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return true;
}

void Renderer::shutdown() {
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = nullptr;
    }
    SDL_Quit();
}

void Renderer::clear(uint8_t r, uint8_t g, uint8_t b) {
    if (!sdlRenderer) return;
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderClear(sdlRenderer);
}

void Renderer::present() {
    if (!sdlRenderer) return;
    SDL_RenderPresent(sdlRenderer);
}

SDL_Texture* Renderer::createTextureFromRaw(const PixelArt::RawTexture& raw) {
    if (!sdlRenderer || raw.width <= 0 || raw.height <= 0) return nullptr;

    SDL_Texture* tex = SDL_CreateTexture(
        sdlRenderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        raw.width,
        raw.height
    );

    if (!tex) {
        return nullptr;
    }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(tex, nullptr, raw.pixels.data(), raw.width * sizeof(uint32_t));
    return tex;
}

void Renderer::drawTexture(SDL_Texture* tex, int x, int y, double angleDeg, float scale, bool centered) {
    if (!sdlRenderer || !tex) return;

    int w = 0, h = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    w = static_cast<int>(w * scale);
    h = static_cast<int>(h * scale);

    SDL_Rect dstRect;
    if (centered) {
        dstRect.x = x - w / 2;
        dstRect.y = y - h / 2;
    } else {
        dstRect.x = x;
        dstRect.y = y;
    }
    dstRect.w = w;
    dstRect.h = h;

    if (angleDeg == 0.0) {
        SDL_RenderCopy(sdlRenderer, tex, nullptr, &dstRect);
    } else {
        SDL_RenderCopyEx(sdlRenderer, tex, nullptr, &dstRect, angleDeg, nullptr, SDL_FLIP_NONE);
    }
}

void Renderer::drawBackgroundTiled(SDL_Texture* tex, int scrollX, int scrollY) {
    if (!sdlRenderer || !tex) return;

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
}

} // namespace Graphics
