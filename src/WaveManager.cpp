#include "WaveManager.hpp"
#include <cmath>
#include <cstdlib>

namespace Game {

WaveManager::WaveManager() {
    init(1);
}

void WaveManager::init(int levelId) {
    currentLevel = getLevel(levelId);
    sectorTime = 0.0f;
    spawnCooldown = 1.0f;
    waveCount = 0;
    bossActive = false;
    levelFinished = false;

    bossX = Graphics::SCREEN_WIDTH / 2.0f;
    bossY = 110.0f;
    bossVx = 90.0f;
    bossMaxHp = bossHp = 40 + levelId * 15;
    bossShootTimer = 2.0f;
}

void WaveManager::update(float dt, float playerX, float playerY, EnemyPool<128>& enemyPool, BulletPool<512>& bulletPool) {
    if (levelFinished) return;

    sectorTime += dt;

    // Fase 1: Oleadas de esbirros antes del Jefe
    if (!bossActive && sectorTime < currentLevel.duration) {
        spawnCooldown -= dt;
        if (spawnCooldown <= 0.0f) {
            spawnCooldown = 1.6f - std::min(0.6f, currentLevel.id * 0.05f);
            waveCount++;

            // Generación de patrones coreografiados
            int pattern = waveCount % 5;
            float cx = 150.0f + (std::rand() % (Graphics::SCREEN_WIDTH - 300));

            if (pattern == 0) {
                // Formación en V de Swoopers
                enemyPool.spawn(EnemyType::SWOOPER, 60.0f, -20.0f);
                enemyPool.spawn(EnemyType::SWOOPER, Graphics::SCREEN_WIDTH - 60.0f, -20.0f);
            } else if (pattern == 1) {
                // Escuadrón Gunship + Weavers
                enemyPool.spawn(EnemyType::GUNSHIP, cx, -30.0f);
                enemyPool.spawn(EnemyType::WEAVER, cx - 70.0f, -50.0f);
                enemyPool.spawn(EnemyType::WEAVER, cx + 70.0f, -50.0f);
            } else if (pattern == 2) {
                // Spreader de fuego en abanico
                enemyPool.spawn(EnemyType::SPREADER, cx, -30.0f);
            } else if (pattern == 3) {
                // Enjambre rápido de Kamikazes
                enemyPool.spawn(EnemyType::KAMIKAZE, cx - 80.0f, -20.0f);
                enemyPool.spawn(EnemyType::KAMIKAZE, cx + 80.0f, -20.0f);
            } else {
                // Flota clásica de 4 drones
                for (int i = 0; i < 4; ++i) {
                    enemyPool.spawn(EnemyType::DRONE, 160.0f + i * 200.0f, -30.0f - i * 30.0f);
                }
            }
        }
    } 
    // Fase 2: Aparición del Boss del Sector
    else if (!bossActive && sectorTime >= currentLevel.duration) {
        bossActive = true;
        enemyPool.clear(); // Limpiar esbirros para el duelo contra el Jefe
    }

    // Comportamiento del Boss activo
    if (bossActive) {
        bossX += bossVx * dt;
        if (bossX < 140.0f) {
            bossX = 140.0f;
            bossVx = -bossVx;
        } else if (bossX > Graphics::SCREEN_WIDTH - 140.0f) {
            bossX = Graphics::SCREEN_WIDTH - 140.0f;
            bossVx = -bossVx;
        }

        bossShootTimer -= dt;
        if (bossShootTimer <= 0.0f) {
            bossShootTimer = 1.3f;

            // Ráfaga radial de proyectiles del Boss
            float baseAngle = std::atan2(playerY - bossY, playerX - bossX) * (180.0f / 3.1415926535f);
            for (int i = -2; i <= 2; ++i) {
                bulletPool.spawn(bossX, bossY + 30.0f, baseAngle + i * 16.0f, 220.0f, 1, BulletType::ENEMY_ORB, true);
            }
        }
    }
}

} // namespace Game
