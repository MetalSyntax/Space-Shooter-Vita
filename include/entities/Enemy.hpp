#pragma once

#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include "Renderer.hpp"
#include "entities/Bullet.hpp"

namespace Game {

enum class EnemyType {
    DRONE,      // Vuela en línea recta hacia abajo
    WEAVER,     // Oscila en zig-zag (senoidal)
    SWOOPER,    // Entra en arco/curva
    GUNSHIP,    // Nave que apunta y dispara
    SPREADER,   // Dispara en abanico
    KAMIKAZE    // Acelera persiguiendo al jugador
};

struct Enemy {
    bool active = false;
    EnemyType type = EnemyType::DRONE;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float rotationDeg = 0.0f;
    int hp = 1;
    int maxHp = 1;
    int points = 10;
    float radius = 18.0f;

    // Temporizadores de comportamiento/IA
    float cooldown = 0.0f;
    float stateTimer = 0.0f;
    float param1 = 0.0f;
};

template<size_t MAX_ENEMIES = 128>
class EnemyPool {
public:
    EnemyPool() {
        enemies.resize(MAX_ENEMIES);
    }

    void spawn(EnemyType type, float x, float y, float speedMult = 1.0f) {
        for (auto& e : enemies) {
            if (!e.active) {
                e.active = true;
                e.type = type;
                e.x = x;
                e.y = y;
                e.rotationDeg = 180.0f; // Mirando hacia abajo
                e.stateTimer = 0.0f;

                switch (type) {
                    case EnemyType::DRONE:
                        e.hp = e.maxHp = 1;
                        e.points = 10;
                        e.radius = 16.0f;
                        e.vx = 0.0f;
                        e.vy = 150.0f * speedMult;
                        break;
                    case EnemyType::WEAVER:
                        e.hp = e.maxHp = 1;
                        e.points = 15;
                        e.radius = 16.0f;
                        e.vx = 0.0f;
                        e.vy = 170.0f * speedMult;
                        break;
                    case EnemyType::SWOOPER:
                        e.hp = e.maxHp = 1;
                        e.points = 15;
                        e.radius = 16.0f;
                        e.param1 = (x < Graphics::SCREEN_WIDTH / 2.0f) ? 10.0f : 170.0f; // ángulo inicial
                        e.vx = 0.0f;
                        e.vy = 220.0f * speedMult;
                        break;
                    case EnemyType::GUNSHIP:
                        e.hp = e.maxHp = 2;
                        e.points = 25;
                        e.radius = 22.0f;
                        e.vx = 0.0f;
                        e.vy = 80.0f * speedMult;
                        e.cooldown = 1.5f;
                        break;
                    case EnemyType::SPREADER:
                        e.hp = e.maxHp = 3;
                        e.points = 35;
                        e.radius = 24.0f;
                        e.vx = 0.0f;
                        e.vy = 70.0f * speedMult;
                        e.cooldown = 1.8f;
                        break;
                    case EnemyType::KAMIKAZE:
                        e.hp = e.maxHp = 1;
                        e.points = 20;
                        e.radius = 14.0f;
                        e.vx = 0.0f;
                        e.vy = 120.0f;
                        break;
                }
                return;
            }
        }
    }

    void update(float dt, float playerX, float playerY, BulletPool<512>& bulletPool) {
        for (auto& e : enemies) {
            if (!e.active) continue;

            e.stateTimer += dt;

            switch (e.type) {
                case EnemyType::DRONE:
                    e.y += e.vy * dt;
                    break;

                case EnemyType::WEAVER:
                    e.y += e.vy * dt;
                    e.vx = std::sin(e.stateTimer * 5.0f) * 180.0f;
                    e.x += e.vx * dt;
                    break;

                case EnemyType::SWOOPER: {
                    float target = 100.0f;
                    float turn = (e.param1 < target) ? 55.0f : -55.0f;
                    if (std::abs(e.param1 - target) > 2.0f) {
                        e.param1 += turn * dt;
                    }
                    float rad = e.param1 * (3.1415926535f / 180.0f);
                    e.x += std::cos(rad) * 220.0f * dt;
                    e.y += std::sin(rad) * 220.0f * dt;
                    e.rotationDeg = e.param1 - 90.0f;
                    break;
                }

                case EnemyType::GUNSHIP:
                    e.y += e.vy * dt;
                    e.cooldown -= dt;
                    if (e.cooldown <= 0.0f && e.y > 40.0f && e.y < Graphics::SCREEN_HEIGHT - 60.0f) {
                        float aimAngle = std::atan2(playerY - e.y, playerX - e.x) * (180.0f / 3.1415926535f);
                        bulletPool.spawn(e.x, e.y + 15.0f, aimAngle, 260.0f, 1, BulletType::ENEMY_ORB, true);
                        e.cooldown = 2.0f;
                    }
                    break;

                case EnemyType::SPREADER:
                    e.y += e.vy * dt;
                    e.cooldown -= dt;
                    if (e.cooldown <= 0.0f && e.y > 40.0f && e.y < Graphics::SCREEN_HEIGHT - 60.0f) {
                        float aimAngle = std::atan2(playerY - e.y, playerX - e.x) * (180.0f / 3.1415926535f);
                        bulletPool.spawn(e.x, e.y + 15.0f, aimAngle - 15.0f, 220.0f, 1, BulletType::ENEMY_ORB, true);
                        bulletPool.spawn(e.x, e.y + 15.0f, aimAngle,         220.0f, 1, BulletType::ENEMY_ORB, true);
                        bulletPool.spawn(e.x, e.y + 15.0f, aimAngle + 15.0f, 220.0f, 1, BulletType::ENEMY_ORB, true);
                        e.cooldown = 2.2f;
                    }
                    break;

                case EnemyType::KAMIKAZE: {
                    float aimAngle = std::atan2(playerY - e.y, playerX - e.x);
                    float spd = 260.0f;
                    e.vx = std::cos(aimAngle) * spd;
                    e.vy = std::sin(aimAngle) * spd;
                    e.x += e.vx * dt;
                    e.y += e.vy * dt;
                    e.rotationDeg = aimAngle * (180.0f / 3.1415926535f) + 90.0f;
                    break;
                }
            }

            // Desactivar si sale de la pantalla
            if (e.y > Graphics::SCREEN_HEIGHT + 60.0f || e.x < -60.0f || e.x > Graphics::SCREEN_WIDTH + 60.0f) {
                e.active = false;
            }
        }
    }

    void render(Graphics::Renderer& renderer, SDL_Texture* texDrone, SDL_Texture* texGunship) {
        for (const auto& e : enemies) {
            if (!e.active) continue;

            SDL_Texture* tex = texDrone;
            if (e.type == EnemyType::GUNSHIP || e.type == EnemyType::SPREADER) {
                tex = texGunship;
            }

            renderer.drawTexture(tex, static_cast<int>(e.x), static_cast<int>(e.y), e.rotationDeg, 1.0f, true);
        }
    }

    std::vector<Enemy>& getEnemies() { return enemies; }
    const std::vector<Enemy>& getEnemies() const { return enemies; }

    void clear() {
        for (auto& e : enemies) e.active = false;
    }

private:
    std::vector<Enemy> enemies;
};

} // namespace Game
