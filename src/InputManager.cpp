#include "InputManager.hpp"
#include <cmath>
#include <algorithm>

namespace Input {

constexpr float REPEAT_DELAY = 0.35f;
constexpr float REPEAT_RATE = 0.11f;

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

bool InputManager::edge(int b, bool down) {
    bool fired = down && !prev[b];
    prev[b] = down;
    return fired;
}

bool InputManager::repeat(int b, bool down, float dt) {
    // held[] counts down to the next auto-repeat.
    if (!down) {
        prev[b] = false;
        return false;
    }
    if (!prev[b]) {
        prev[b] = true;
        held[b] = REPEAT_DELAY;
        return true;
    }
    held[b] -= dt;
    if (held[b] > 0.0f) return false;
    held[b] += REPEAT_RATE;
    return true;
}

InputState InputManager::poll(float dt) {
    InputState state;
    bool up = false, down = false, left = false, right = false;
    bool confirm = false, back = false, bomb = false, pause = false, tabL = false, tabR = false;

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

    // 4. Disparo: Gatillo R / Botón Cruz (X)
    state.fire = (pad.buttons & SCE_CTRL_RTRIGGER) || (pad.buttons & SCE_CTRL_CROSS);
    bomb = (pad.buttons & SCE_CTRL_SQUARE) || (pad.buttons & SCE_CTRL_CIRCLE);
    pause = pad.buttons & SCE_CTRL_START;
    confirm = pad.buttons & SCE_CTRL_CROSS;
    back = pad.buttons & SCE_CTRL_CIRCLE;
    tabL = pad.buttons & SCE_CTRL_LTRIGGER;
    tabR = pad.buttons & SCE_CTRL_RTRIGGER;
#else
    // Desktop: Teclado + SDL Gamepad
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) state.moveX -= 1.0f;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) state.moveX += 1.0f;
    if (keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) state.moveY -= 1.0f;
    if (keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) state.moveY += 1.0f;
    // IJKL aims like the Vita right stick.
    if (keys[SDL_SCANCODE_J]) state.aimX -= 1.0f;
    if (keys[SDL_SCANCODE_L]) state.aimX += 1.0f;
    if (keys[SDL_SCANCODE_I]) state.aimY -= 1.0f;
    if (keys[SDL_SCANCODE_K]) state.aimY += 1.0f;

    state.fire = keys[SDL_SCANCODE_SPACE];
    bomb = keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_B];
    pause = keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_P];
    confirm = keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_SPACE];
    back = keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_BACKSPACE];
    tabL = keys[SDL_SCANCODE_Q];
    tabR = keys[SDL_SCANCODE_E];

    if (controller) {
        int16_t stickLX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        int16_t stickLY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        if (std::abs(stickLX) > 8000) state.moveX = stickLX / 32767.0f;
        if (std::abs(stickLY) > 8000) state.moveY = stickLY / 32767.0f;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) state.moveX = -1.0f;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) state.moveX = 1.0f;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) state.moveY = -1.0f;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) state.moveY = 1.0f;

        int16_t stickRX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int16_t stickRY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        if (std::hypot(stickRX, stickRY) > 9000) {
            state.aimX = stickRX / 32767.0f;
            state.aimY = stickRY / 32767.0f;
        }

        bool a = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        bool rb = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        bool b = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
        state.fire = state.fire || a || rb;
        bomb = bomb || b || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
        pause = pause || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
        confirm = confirm || a;
        back = back || b;
        tabL = tabL || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        tabR = tabR || rb;
    }
#endif

    // Menu directions from D-pad or left stick
    up = state.moveY < -0.5f;
    down = state.moveY > 0.5f;
    left = state.moveX < -0.5f;
    right = state.moveX > 0.5f;

    state.up = repeat(B_UP, up, dt);
    state.down = repeat(B_DOWN, down, dt);
    state.left = repeat(B_LEFT, left, dt);
    state.right = repeat(B_RIGHT, right, dt);
    state.confirm = edge(B_CONFIRM, confirm);
    state.back = edge(B_BACK, back);
    state.bomb = edge(B_BOMB, bomb);
    state.pause = edge(B_PAUSE, pause);
    state.tabLeft = edge(B_TABL, tabL);
    state.tabRight = edge(B_TABR, tabR);

    // Normalizar vector de movimiento
    float len = std::hypot(state.moveX, state.moveY);
    if (len > 1.0f) {
        state.moveX /= len;
        state.moveY /= len;
    }

    return state;
}

} // namespace Input
