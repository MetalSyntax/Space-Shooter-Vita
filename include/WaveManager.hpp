#pragma once

#include <vector>
#include "LevelManager.hpp"
#include "entities/Enemy.hpp"
#include "entities/Bullet.hpp"
#include "generators/BossGenerator.hpp"

namespace Game {

class WaveManager {
public:
    WaveManager();

    void init(int levelId);
    void update(float dt, float playerX, float playerY, EnemyPool<128>& enemyPool, BulletPool<512>& bulletPool);

    bool isBossPhase() const { return bossActive; }
    bool isLevelClear() const { return levelFinished; }
    float getSectorTime() const { return sectorTime; }
    float getDuration() const { return currentLevel.duration; }
    const LevelDef& getCurrentLevel() const { return currentLevel; }

    // Estado del Jefe de sector
    float getBossX() const { return bossX; }
    float getBossY() const { return bossY; }
    int getBossHp() const { return bossHp; }
    int getBossMaxHp() const { return bossMaxHp; }
    void damageBoss(int dmg) {
        bossHp -= dmg;
        if (bossHp <= 0) {
            bossActive = false;
            levelFinished = true;
        }
    }

private:
    LevelDef currentLevel;
    float sectorTime = 0.0f;
    float spawnCooldown = 0.0f;
    int waveCount = 0;

    bool bossActive = false;
    bool levelFinished = false;
    float bossX = 480.0f;
    float bossY = 120.0f;
    float bossVx = 100.0f;
    int bossHp = 50;
    int bossMaxHp = 50;
    float bossShootTimer = 0.0f;
};

} // namespace Game
