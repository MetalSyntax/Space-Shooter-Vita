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

    // Inicializar vita2d nativo con VBlank wait activado para 60 FPS
    vita2d_init();
    vita2d_set_vblank_wait(1);
    vita2d_set_clear_color(RGBA8(15, 23, 42, 255));
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
    vita2d_fini();
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
    vita2d_start_drawing();
    vita2d_clear_screen();
#else
    if (!sdlRenderer) return;
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderClear(sdlRenderer);
#endif
}

void Renderer::present() {
#ifdef __vita__
    vita2d_end_drawing();
    vita2d_swap_buffers();
#else
    if (!sdlRenderer) return;
    SDL_RenderPresent(sdlRenderer);
#endif
}

TextureType* Renderer::createTextureFromRaw(const PixelArt::RawTexture& raw) {
    if (raw.width <= 0 || raw.height <= 0) return nullptr;

#ifdef __vita__
    // Crear textura vacía en vita2d (formato nativo SCE_GXM_TEXTURE_FORMAT_A8B8G8R8)
    vita2d_texture* tex = vita2d_create_empty_texture_format(raw.width, raw.height, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
    if (!tex) return nullptr;

    uint32_t* dst = reinterpret_cast<uint32_t*>(vita2d_texture_get_datap(tex));
    unsigned int stride = vita2d_texture_get_stride(tex) / sizeof(uint32_t);

    for (int y = 0; y < raw.height; ++y) {
        for (int x = 0; x < raw.width; ++x) {
            dst[y * stride + x] = raw.pixels[y * raw.width + x];
        }
    }
    // Filtrado por vecino más cercano para pixel art nítido
    vita2d_texture_set_filters(tex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
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
    vita2d_free_texture(tex);
#else
    SDL_DestroyTexture(tex);
#endif
}

void Renderer::drawTexture(TextureType* tex, int x, int y, double angleDeg, float scale, bool centered) {
    if (!tex) return;

#ifdef __vita__
    float rad = static_cast<float>(angleDeg * (3.1415926535 / 180.0));
    float tw = static_cast<float>(vita2d_texture_get_width(tex));
    float th = static_cast<float>(vita2d_texture_get_height(tex));

    if (centered) {
        vita2d_draw_texture_scale_rotate_hotspot(tex, x, y, scale, scale, rad, tw / 2.0f, th / 2.0f);
    } else {
        vita2d_draw_texture_scale_rotate(tex, x, y, scale, scale, rad);
    }
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
    int tw = static_cast<int>(vita2d_texture_get_width(tex));
    int th = static_cast<int>(vita2d_texture_get_height(tex));
    if (tw <= 0 || th <= 0) return;

    int startX = (scrollX % tw) - tw;
    int startY = (scrollY % th) - th;

    for (int y = startY; y < SCREEN_HEIGHT + th; y += th) {
        for (int x = startX; x < SCREEN_WIDTH + tw; x += tw) {
            vita2d_draw_texture(tex, x, y);
        }
    }
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
    vita2d_draw_rectangle(x, y, w, h, RGBA8(r, g, b, a));
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
    vita2d_draw_pixel(x, y, RGBA8(r, g, b, a));
#else
    if (!sdlRenderer) return;
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
    SDL_RenderDrawPoint(sdlRenderer, x, y);
#endif
}

} // namespace Graphics
