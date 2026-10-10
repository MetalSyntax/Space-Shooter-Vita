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
    bool fire   = false; // Disparo normal (R / Cruz / Espacio), held
    bool bomb   = false; // Bomba (Cuadrado / Circulo / X), one-shot
    bool pause  = false; // Start / Escape, one-shot

    // Menu navigation (one-shot, directions auto-repeat while held)
    bool up = false, down = false, left = false, right = false;
    bool confirm = false; // Cruz / Enter
    bool back = false;    // Circulo / Escape / Backspace
    bool tabLeft = false, tabRight = false; // L / R
};

class InputManager {
public:
    static InputManager& instance();

    void init();
    InputState poll(float dt);

private:
    InputManager() = default;
    ~InputManager() = default;

    enum Btn { B_UP, B_DOWN, B_LEFT, B_RIGHT, B_CONFIRM, B_BACK, B_BOMB, B_PAUSE, B_TABL, B_TABR, B_COUNT };

    bool edge(int b, bool down);
    bool repeat(int b, bool down, float dt);

    SDL_GameController* controller = nullptr;
    bool prev[B_COUNT] = {};
    float held[B_COUNT] = {};
};

} // namespace Input
