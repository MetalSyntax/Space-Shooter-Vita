#include "InputManager.hpp"
#include <cmath>
#include <algorithm>

namespace Input {

InputManager& InputManager::instance() {
    static InputManager s_instance;
    return s_instance;
}

void InputManager::init() {
#ifdef __vita__
    // Habilitar modo analógico extendido para capturar los dos sticks de PS Vita
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
#else
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) break;
        }
    }
#endif
}

InputState InputManager::poll() {
    InputState state;

#ifdef __vita__
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(0, &pad, 1);

    // 1. D-Pad digital de PS Vita
    if (pad.buttons & SCE_CTRL_LEFT)  state.moveX -= 1.0f;
    if (pad.buttons & SCE_CTRL_RIGHT) state.moveX += 1.0f;
    if (pad.buttons & SCE_CTRL_UP)    state.moveY -= 1.0f;
    if (pad.buttons & SCE_CTRL_DOWN)  state.moveY += 1.0f;

    // 2. Stick analógico izquierdo de PS Vita (0..255, centro ~128)
    float lx = (pad.lx - 128) / 128.0f;
    float ly = (pad.ly - 128) / 128.0f;
    if (std::hypot(lx, ly) > 0.20f) {
        state.moveX = lx;
        state.moveY = ly;
    }

    // 3. Stick analógico derecho de PS Vita (0..255, centro ~128)
    float rx = (pad.rx - 128) / 128.0f;
    float ry = (pad.ry - 128) / 128.0f;
    if (std::hypot(rx, ry) > 0.25f) {
        state.aimX = rx;
        state.aimY = ry;
    }

    // 4. Botones de acción físicos de PS Vita
    // Disparo: Gatillo R / Botón Cruz (X)
    if ((pad.buttons & SCE_CTRL_RTRIGGER) || (pad.buttons & SCE_CTRL_CROSS)) {
        state.fire = true;
    }

    // Bomba: Botón Cuadrado / Círculo
    bool currentBombPressed = (pad.buttons & SCE_CTRL_SQUARE) || (pad.buttons & SCE_CTRL_CIRCLE);
    if (currentBombPressed && !prevBombState) {
        state.bomb = true; // One-shot trigger
    }
    prevBombState = currentBombPressed;

    // Pausa: Start
    if (pad.buttons & SCE_CTRL_START) {
        state.pause = true;
    }

#else
    // Desktop: Teclado + SDL Gamepad
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) state.moveX -= 1.0f;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) state.moveX += 1.0f;
    if (keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) state.moveY -= 1.0f;
    if (keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) state.moveY += 1.0f;

    if (keys[SDL_SCANCODE_SPACE]) state.fire = true;
    if (keys[SDL_SCANCODE_X])     state.bomb = true;
    if (keys[SDL_SCANCODE_ESCAPE]) state.pause = true;

    if (controller) {
        int16_t stickLX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        int16_t stickLY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        if (std::abs(stickLX) > 8000) state.moveX = stickLX / 32767.0f;
        if (std::abs(stickLY) > 8000) state.moveY = stickLY / 32767.0f;

        int16_t stickRX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int16_t stickRY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        if (std::hypot(stickRX, stickRY) > 9000) {
            state.aimX = stickRX / 32767.0f;
            state.aimY = stickRY / 32767.0f;
        }

        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) ||
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
            state.fire = true;
        }

        bool currentPadBomb = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) ||
                              SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
        if (currentPadBomb && !prevBombState) {
            state.bomb = true;
        }
        prevBombState = currentPadBomb;

        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
            state.pause = true;
        }
    }
#endif

    // Normalizar vector de movimiento
    float len = std::hypot(state.moveX, state.moveY);
    if (len > 1.0f) {
        state.moveX /= len;
        state.moveY /= len;
    }

    return state;
}

} // namespace Input
