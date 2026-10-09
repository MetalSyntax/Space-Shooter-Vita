#pragma once

#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include "Renderer.hpp"
#include "entities/Ships.hpp"
#include "entities/Bullet.hpp"
#include "InputManager.hpp"

namespace Game {

enum class PowerKind {
    SHIELD,
    RAPID,
    SPREAD,
    MISSILE,
    DRONE,
    MAGNET
};

class Player {
public:
    Player();

    void init(ShipId shipId);
    void update(float dt, const Input::InputState& input, BulletPool<512>& bulletPool);
    void render(Graphics::Renderer& renderer, Graphics::TextureType* shipTex, Graphics::TextureType* shieldTex, Graphics::TextureType* droneTex);

    void takeHit();
    void useBomb();

    void activatePower(PowerKind kind, float durationMs);
    bool hasPower(PowerKind kind) const;

    float getX() const { return x; }
    float getY() const { return y; }
    int getLives() const { return lives; }
    int getBombs() const { return bombs; }
    float getAimAngle() const { return aimAngle; }
    bool isInvulnerable() const { return invulnTimer > 0.0f; }

private:
    void fire(BulletPool<512>& bulletPool);

    ShipDef def;
    float x = 480.0f;
    float y = 450.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float aimAngle = -90.0f;

    int lives = 5;
    int bombs = 3;
    float fireTimer = 0.0f;
    float missileTimer = 0.0f;
    float invulnTimer = 0.0f;
    float droneOrbitAngle = 0.0f;

    std::unordered_map<PowerKind, float> powers;
};

} // namespace Game
