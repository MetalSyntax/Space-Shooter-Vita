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
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    // Ventana fija de 960x544 en PS Vita
    sdlWindow = SDL_CreateWindow(title.c_str(), 
                                 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                 SCREEN_WIDTH, SCREEN_HEIGHT, 
                                 SDL_WINDOW_SHOWN);
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

#ifdef __vita__
    // En PS Vita, SDL_CreateRenderer con flag 0 selecciona el backend oficial de GXM
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
#else
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#endif

    if (!sdlRenderer) {
        return false;
    }

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

    // Crear Surface intermedia con los píxeles RGBA en memoria RAM
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        const_cast<uint32_t*>(raw.pixels.data()),
        raw.width,
        raw.height,
        32,
        raw.width * sizeof(uint32_t),
        SDL_PIXELFORMAT_ABGR8888
    );

    if (!surf) {
        return nullptr;
    }

    // En Vita GXM, SDL_CreateTextureFromSurface asigna la memoria VRAM de GXM y
    // realiza la subida de datos y alineamiento de pitch de forma garantizada y nativa
    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
    SDL_FreeSurface(surf);

    if (!tex) {
        return nullptr;
    }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
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
