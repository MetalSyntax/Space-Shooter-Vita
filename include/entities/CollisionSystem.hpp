#pragma once

#include <cmath>
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "entities/Bullet.hpp"
#include "WaveManager.hpp"
#include "ParticlePool.hpp"

namespace Game {

struct CollisionSystem {
    // Procesa colisiones entre balas del jugador y esbirros / Boss
    static int checkBulletVsEnemies(BulletPool<512>& bulletPool, EnemyPool<128>& enemyPool, WaveManager& waveMgr, Graphics::ParticlePool<1024>& particles) {
        int scoreGained = 0;
        auto& bullets = bulletPool.getBullets();

        // 1. Balas del jugador contra el Boss
        if (waveMgr.isBossPhase()) {
            float bx = waveMgr.getBossX();
            float by = waveMgr.getBossY();
            constexpr float bossRadius = 45.0f;

            for (auto& b : bullets) {
                if (!b.active || b.isEnemy) continue;

                float distSq = (b.x - bx) * (b.x - bx) + (b.y - by) * (b.y - by);
                if (distSq <= bossRadius * bossRadius) {
                    b.active = false;
                    waveMgr.damageBoss(b.damage);
                    scoreGained += 5;

                    for (int s = 0; s < 5; ++s) {
                        float angle = (std::rand() % 360) * (3.1415926535f / 180.0f);
                        float spd = 80.0f + (std::rand() % 120);
                        particles.spawn(Graphics::ParticleType::SPARK, b.x, b.y, std::cos(angle) * spd, std::sin(angle) * spd, 0.2f, 2.5f, 255, 200, 100);
                    }

                    if (waveMgr.isLevelClear()) {
                        particles.explode(bx, by, 3.5f);
                        scoreGained += 1000;
                    }
                }
            }
        }

        // 2. Balas del jugador contra esbirros
        auto& enemies = enemyPool.getEnemies();
        for (auto& b : bullets) {
            if (!b.active || b.isEnemy) continue;

            for (auto& e : enemies) {
                if (!e.active) continue;

                float distSq = (b.x - e.x) * (b.x - e.x) + (b.y - e.y) * (b.y - e.y);
                float radiusCombined = e.radius + 8.0f;

                if (distSq <= radiusCombined * radiusCombined) {
                    b.active = false;
                    e.hp -= b.damage;

                    for (int s = 0; s < 4; ++s) {
                        float angle = (std::rand() % 360) * (3.1415926535f / 180.0f);
                        float spd = 60.0f + (std::rand() % 100);
                        particles.spawn(Graphics::ParticleType::SPARK, b.x, b.y, std::cos(angle) * spd, std::sin(angle) * spd, 0.15f, 2.0f, 255, 240, 160);
                    }

                    if (e.hp <= 0) {
                        e.active = false;
                        scoreGained += e.points;
                        particles.explode(e.x, e.y, (e.radius > 18.0f) ? 1.5f : 1.0f);
                    }
                    break;
                }
            }
        }
        return scoreGained;
    }

    // Procesa colisiones contra el jugador (balas enemigas o naves)
    static void checkEnemiesVsPlayer(BulletPool<512>& bulletPool, EnemyPool<128>& enemyPool, Player& player, Graphics::ParticlePool<1024>& particles) {
        if (player.isInvulnerable()) return;

        float px = player.getX();
        float py = player.getY();
        constexpr float playerRadius = 16.0f;

        // 1. Balas enemigas contra el jugador
        auto& bullets = bulletPool.getBullets();
        for (auto& b : bullets) {
            if (!b.active || !b.isEnemy) continue;

            float distSq = (b.x - px) * (b.x - px) + (b.y - py) * (b.y - py);
            float radiusCombined = playerRadius + 6.0f;

            if (distSq <= radiusCombined * radiusCombined) {
                b.active = false;
                player.takeHit();
                particles.spawn(Graphics::ParticleType::RING, px, py, 0, 0, 0.25f, 25.0f, 100, 255, 255);
                if (player.isInvulnerable()) return;
            }
        }

        // 2. Contacto directo con esbirros
        auto& enemies = enemyPool.getEnemies();
        for (auto& e : enemies) {
            if (!e.active) continue;

            float distSq = (e.x - px) * (e.x - px) + (e.y - py) * (e.y - py);
            float radiusCombined = playerRadius + e.radius;

            if (distSq <= radiusCombined * radiusCombined) {
                e.hp -= 2;
                if (e.hp <= 0) {
                    e.active = false;
                    particles.explode(e.x, e.y, 1.2f);
                }
                player.takeHit();
                particles.spawn(Graphics::ParticleType::RING, px, py, 0, 0, 0.25f, 30.0f, 255, 100, 100);
                if (player.isInvulnerable()) return;
            }
        }
    }
};

} // namespace Game
