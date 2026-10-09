#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "Renderer.hpp"

namespace Game {

enum class BulletType {
    PLAYER_LASER,
    PLAYER_MISSILE,
    ENEMY_ORB,
    ENEMY_LASER
};

struct Bullet {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float angleDeg = 0.0f;
    int damage = 1;
    bool homing = false;
    bool isEnemy = false;
    bool active = false;
    BulletType type = BulletType::PLAYER_LASER;
};

template<size_t MAX_BULLETS = 512>
class BulletPool {
public:
    BulletPool() {
        bullets.resize(MAX_BULLETS);
    }

    void spawn(float x, float y, float angleDeg, float speed, int damage, BulletType type, bool isEnemy = false, bool homing = false) {
        for (auto& b : bullets) {
            if (!b.active) {
                b.active = true;
                b.x = x;
                b.y = y;
                b.angleDeg = angleDeg;
                float rad = angleDeg * (3.1415926535f / 180.0f);
                b.vx = std::cos(rad) * speed;
                b.vy = std::sin(rad) * speed;
                b.damage = damage;
                b.type = type;
                b.isEnemy = isEnemy;
                b.homing = homing;
                return;
            }
        }
    }

    void update(float dt) {
        for (auto& b : bullets) {
            if (!b.active) continue;

            b.x += b.vx * dt;
            b.y += b.vy * dt;

            // Desactivar si sale de los límites de la pantalla (con margen)
            if (b.x < -40.0f || b.x > Graphics::SCREEN_WIDTH + 40.0f ||
                b.y < -40.0f || b.y > Graphics::SCREEN_HEIGHT + 40.0f) {
                b.active = false;
            }
        }
    }

    void render(Graphics::Renderer& renderer, SDL_Texture* texLaser, SDL_Texture* texMissile, SDL_Texture* texEnemyOrb) {
        for (const auto& b : bullets) {
            if (!b.active) continue;

            SDL_Texture* tex = texLaser;
            float scale = 1.0f;
            if (b.type == BulletType::PLAYER_MISSILE) {
                tex = texMissile;
                scale = 1.2f;
            } else if (b.type == BulletType::ENEMY_ORB) {
                tex = texEnemyOrb;
            }

            renderer.drawTexture(tex, static_cast<int>(b.x), static_cast<int>(b.y), b.angleDeg, scale, true);
        }
    }

    void clear() {
        for (auto& b : bullets) {
            b.active = false;
        }
    }

    std::vector<Bullet>& getBullets() { return bullets; }
    const std::vector<Bullet>& getBullets() const { return bullets; }

private:
    std::vector<Bullet> bullets;
};

} // namespace Game
