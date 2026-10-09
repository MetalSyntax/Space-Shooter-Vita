#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#ifdef __vita__
#include <psp2/ctrl.h>
#endif

namespace Input {

struct InputState {
    float moveX = 0.0f; // -1.0 a 1.0
    float moveY = 0.0f; // -1.0 a 1.0
    float aimX  = 0.0f; // -1.0 a 1.0 (Stick derecho)
    float aimY  = 0.0f; // -1.0 a 1.0 (Stick derecho)
    bool fire   = false; // Disparo normal (R / Cruz / Espacio)
    bool bomb   = false; // Bomba (Cuadrado / Circulo / X)
    bool pause  = false; // Start / Escape
};

class InputManager {
public:
    static InputManager& instance();

    void init();
    InputState poll();

private:
    InputManager() = default;
    ~InputManager() = default;

    SDL_GameController* controller = nullptr;
    bool prevBombState = false;
};

} // namespace Input
