#include "entities/Player.hpp"
#include <cmath>
#include <algorithm>

namespace Game {

constexpr float BASE_ACCEL       = 1500.0f;
constexpr float DRAG             = 0.88f;
constexpr float MAX_SPEED        = 360.0f;
constexpr float BASE_FIRE_DELAY  = 0.18f;
constexpr float BULLET_SPEED     = 700.0f;

Player::Player() {
    init(ShipId::VANGUARD);
}

void Player::init(ShipId shipId) {
    def = getShipDef(shipId);
    lives = def.lives;
    bombs = def.bombs;
    x = Graphics::SCREEN_WIDTH / 2.0f;
    y = Graphics::SCREEN_HEIGHT - 90.0f;
    vx = 0.0f;
    vy = 0.0f;
    aimAngle = -90.0f;
    fireTimer = 0.0f;
    missileTimer = 0.0f;
    invulnTimer = 0.0f;
    powers.clear();

    if (def.sectorShield > 0) {
        activatePower(PowerKind::SHIELD, static_cast<float>(def.sectorShield));
    }
    if (def.permanentDrone) {
        activatePower(PowerKind::DRONE, 9999999.0f);
    }
}

void Player::activatePower(PowerKind kind, float durationMs) {
    powers[kind] = std::max(powers[kind], durationMs);
}

bool Player::hasPower(PowerKind kind) const {
    auto it = powers.find(kind);
    return it != powers.end() && it->second > 0.0f;
}

void Player::useBomb() {
    if (bombs > 0) {
        bombs--;
    }
}

void Player::takeHit() {
    if (hasPower(PowerKind::SHIELD)) {
        powers[PowerKind::SHIELD] = 0.0f;
        return;
    }
    if (invulnTimer > 0.0f) return;

    lives--;
    invulnTimer = def.invulnMs / 1000.0f;
}

void Player::update(float dt, const Input::InputState& input, BulletPool<512>& bulletPool) {
    // 1. Movimiento desde controles físicos (D-Pad o Stick analógico izquierdo)
    float ax = input.moveX;
    float ay = input.moveY;

    float accel = BASE_ACCEL * def.speed;
    vx += ax * accel * dt;
    vy += ay * accel * dt;

    vx *= std::pow(DRAG, dt * 60.0f);
    vy *= std::pow(DRAG, dt * 60.0f);

    float speed = std::hypot(vx, vy);
    float maxSpd = MAX_SPEED * def.speed;
    if (speed > maxSpd) {
        vx = (vx / speed) * maxSpd;
        vy = (vy / speed) * maxSpd;
    }

    x += vx * dt;
    y += vy * dt;

    // Confinar a los límites nativos de PS Vita (960x544)
    constexpr float MARGIN = 25.0f;
    x = std::clamp(x, MARGIN, Graphics::SCREEN_WIDTH - MARGIN);
    y = std::clamp(y, MARGIN, Graphics::SCREEN_HEIGHT - MARGIN);

    // 2. Apuntado con el Stick derecho de PS Vita si se inclina
    if (std::hypot(input.aimX, input.aimY) > 0.25f) {
        aimAngle = std::atan2(input.aimY, input.aimX) * (180.0f / 3.1415926535f);
    }

    // 3. Cadencia y disparo (Gatillo R / Botón Cruz de PS Vita)
    fireTimer -= dt;
    if (input.fire && fireTimer <= 0.0f) {
        fire(bulletPool);
        float delay = BASE_FIRE_DELAY / def.fireRate;
        if (hasPower(PowerKind::RAPID)) delay *= 0.5f;
        fireTimer = delay;
    }

    // 4. Misiles automáticos si tiene el powerup activo
    if (hasPower(PowerKind::MISSILE)) {
        missileTimer -= dt;
        if (missileTimer <= 0.0f) {
            bulletPool.spawn(x - 16.0f, y, aimAngle - 15.0f, 400.0f, 2, BulletType::PLAYER_MISSILE, false, true);
            bulletPool.spawn(x + 16.0f, y, aimAngle + 15.0f, 400.0f, 2, BulletType::PLAYER_MISSILE, false, true);
            missileTimer = 0.65f;
        }
    }

    // 5. Temporizadores de powerups
    for (auto it = powers.begin(); it != powers.end(); ) {
        it->second -= dt * 1000.0f;
        if (it->second <= 0.0f && !def.permanentDrone) {
            it = powers.erase(it);
        } else {
            ++it;
        }
    }

    if (invulnTimer > 0.0f) {
        invulnTimer -= dt;
    }

    droneOrbitAngle += 180.0f * dt;
}

void Player::fire(BulletPool<512>& bulletPool) {
    float rad = aimAngle * (3.1415926535f / 180.0f);
    float muzzleX = x + std::cos(rad) * 22.0f;
    float muzzleY = y + std::sin(rad) * 22.0f;

    int extra = def.extraShots;
    if (hasPower(PowerKind::SPREAD)) {
        int numBullets = 3 + extra;
        for (int i = 0; i < numBullets; ++i) {
            float spreadAngle = aimAngle + (i - (numBullets - 1) / 2.0f) * 14.0f;
            bulletPool.spawn(muzzleX, muzzleY, spreadAngle, BULLET_SPEED, 1, BulletType::PLAYER_LASER, false);
        }
    } else {
        int numBullets = 1 + extra;
        for (int i = 0; i < numBullets; ++i) {
            float off = (i - (numBullets - 1) / 2.0f) * 10.0f;
            float bx = muzzleX - std::sin(rad) * off;
            float by = muzzleY + std::cos(rad) * off;
            bulletPool.spawn(bx, by, aimAngle, BULLET_SPEED, 1, BulletType::PLAYER_LASER, false);
        }
    }

    if (hasPower(PowerKind::DRONE)) {
        float droneRad = droneOrbitAngle * (3.1415926535f / 180.0f);
        float droneX = x + std::cos(droneRad) * 40.0f;
        float droneY = y + std::sin(droneRad) * 40.0f;
        bulletPool.spawn(droneX, droneY, aimAngle, BULLET_SPEED, 1, BulletType::PLAYER_LASER, false);
    }
}

void Player::render(Graphics::Renderer& renderer, SDL_Texture* shipTex, SDL_Texture* shieldTex, SDL_Texture* droneTex) {
    if (invulnTimer > 0.0f) {
        int flash = static_cast<int>(invulnTimer * 15.0f) % 2;
        if (flash == 0) return;
    }

    renderer.drawTexture(shipTex, static_cast<int>(x), static_cast<int>(y), aimAngle + 90.0, 1.2f, true);

    if (hasPower(PowerKind::SHIELD) && shieldTex) {
        renderer.drawTexture(shieldTex, static_cast<int>(x), static_cast<int>(y), 0.0, 1.5f, true);
    }

    if (hasPower(PowerKind::DRONE) && droneTex) {
        float droneRad = droneOrbitAngle * (3.1415926535f / 180.0f);
        int dx = static_cast<int>(x + std::cos(droneRad) * 40.0f);
        int dy = static_cast<int>(y + std::sin(droneRad) * 40.0f);
        renderer.drawTexture(droneTex, dx, dy, droneOrbitAngle, 0.8f, true);
    }
}

} // namespace Game
