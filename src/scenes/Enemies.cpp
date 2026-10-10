#include "GameScene.hpp"
#include "MathUtil.hpp"
#include "I18n.hpp"
#include "audio/Synth.hpp"

/** Enemy roster (Enemy.ts) and the wave director (WaveManager.ts). */
namespace Game {

using Graphics::SCREEN_WIDTH;
using Graphics::SCREEN_HEIGHT;

Enemy* GameScene::spawnEnemy(EType type, float x, float y) {
    Enemy* slot = nullptr;
    for (auto& e : enemies) {
        if (!e.active) { slot = &e; break; }
    }
    if (!slot) return nullptr;
    Enemy& e = *slot;
    e = Enemy{};
    e.active = true;
    e.type = type;
    e.x = x;
    e.y = y;
    const Assets& a = assets();
    float sm = speedMult;

    switch (type) {
        case EType::DRONE: // white drone flying straight down
            e.tex = a.enDrone;
            e.vy = 150 * sm;
            break;
        case EType::WEAVER: // white drone zig-zagging down
            e.tex = a.enDrone;
            e.points = 15;
            e.vy = 170 * sm;
            break;
        case EType::SWOOPER: // enters from a top corner, curving down (V formations)
            e.tex = a.enDrone;
            e.points = 15;
            e.speed = 220 * sm;
            break;
        case EType::BLINKER: // PURPLE · teleport
            e.tex = a.enDronePurple;
            e.hp = 2;
            e.points = 30;
            e.vy = 90 * sm;
            e.timer = 1200;
            break;
        case EType::KAMIKAZE: // YELLOW · locks on and accelerates into the player
            e.tex = a.enDroneYellow;
            e.sx = e.sy = 0.7f;
            e.points = 15;
            e.explosionSize = 0.7f;
            e.speed = 120;
            break;
        case EType::POD: // BLUE · bubble shield soaks 3 hits
            e.tex = a.enPodBlue;
            e.hp = 2;
            e.points = 35;
            e.heavy = true;
            e.shield = 3;
            e.cooldown = 1500;
            e.vy = 60 * sm;
            break;
        case EType::GUNSHIP: // RED · aimed shot every 2 s
            e.tex = a.enGunshipRed;
            e.hp = 2;
            e.points = 25;
            e.heavy = true;
            e.cooldown = 800;
            e.fireEvery = 2000;
            e.vy = 80 * sm;
            break;
        case EType::SPREADER: // ORANGE · fan of 3-5 shots
            e.tex = a.enGunshipOrange;
            e.hp = 3;
            e.points = 35;
            e.heavy = true;
            e.cooldown = 800;
            e.fireEvery = 2300;
            e.vy = 70 * sm;
            break;
        case EType::LASER: // CYAN · parks and fires a telegraphed vertical beam
            e.tex = a.enGunshipCyan;
            e.hp = 3;
            e.points = 40;
            e.heavy = true;
            e.cooldown = 1800;
            e.fireEvery = 3800;
            break;
        case EType::LANCER: // YELLOW · stops, telegraphs, rams
            e.tex = a.enLancerYellow;
            e.hp = 2;
            e.points = 30;
            e.stopY = SCREEN_HEIGHT * frand(0.15f, 0.3f);
            e.vy = 200 * sm;
            break;
        case EType::FIGHTER: // yellow lancers crossing diagonally (pincer)
            e.tex = a.enLancerYellow;
            e.points = 20;
            break;
        case EType::SPLITTER: // GREEN · twin diagonal shots, splits on death
            e.tex = a.enSpiderGreen;
            e.hp = 4;
            e.points = 40;
            e.heavy = true;
            e.explosionSize = 1.2f;
            e.cooldown = 1200;
            e.vy = 60 * sm;
            break;
        case EType::ASTEROID:
            e.tex = a.asteroid;
            e.big = true;
            e.hp = 3;
            e.points = 30;
            e.heavy = true;
            e.explosionSize = 1.4f;
            e.angVel = (float)irand(-90, 90);
            break;
        case EType::MINE: // drifts toward the player and bursts after 3 s
            e.tex = lt->mine;
            e.hp = 2;
            e.points = 20;
            e.timer = 3000;
            e.angVel = 120;
            break;
        case EType::PART:
            e.hp = 8;
            e.points = 150;
            e.heavy = e.solid = e.bossPart = true;
            e.explosionSize = 1.2f;
            e.fireEvery = 1800;
            e.cooldown = 1200;
            break;
    }
    return slot;
}

Enemy* GameScene::spawnKind(EnemyKind kind, float x, float y) {
    switch (kind) {
        case EnemyKind::DRONE: return spawnEnemy(EType::DRONE, x, y);
        case EnemyKind::WEAVER: return spawnEnemy(EType::WEAVER, x, y);
        case EnemyKind::GUNSHIP: return spawnEnemy(EType::GUNSHIP, x, y);
        case EnemyKind::LANCER: return spawnEnemy(EType::LANCER, x, y);
        case EnemyKind::POD: return spawnEnemy(EType::POD, x, y);
        case EnemyKind::SPREADER: return spawnEnemy(EType::SPREADER, x, y);
        case EnemyKind::SPLITTER: return spawnEnemy(EType::SPLITTER, x, y);
        case EnemyKind::BLINKER: return spawnEnemy(EType::BLINKER, x, y);
        case EnemyKind::KAMIKAZE: return spawnEnemy(EType::KAMIKAZE, x, y);
        case EnemyKind::LASER: return spawnEnemy(EType::LASER, x, y);
        default: return spawnEnemy(EType::DRONE, x, y);
    }
}

float GameScene::aimFrom(float x, float y) const {
    return std::atan2(py - y, px - x);
}

/** Fires one bullet in the enemy's ability color. */
void GameScene::enemyShoot(Enemy& e, float angle, Ability color, float speed, bool sound) {
    float h = Graphics::Renderer::texHeight(e.tex) * e.sy;
    fireEnemyBullet(e.x, e.y + h * 0.4f, angle, speed, sound, false, assets().ebAbility[(int)color]);
}

int GameScene::countWaveEnemies() const {
    int n = 0;
    for (const auto& e : enemies) n += (e.active && !e.bossPart) ? 1 : 0;
    return n;
}

void GameScene::onKilled(Enemy& e) {
    if (e.type == EType::SPLITTER) {
        for (int side : {-1, 1}) {
            Enemy* baby = spawnEnemy(EType::WEAVER, e.x + side * 16, e.y);
            if (!baby) continue;
            baby->tex = assets().enDroneGreen;
            baby->sx = baby->sy = 0.6f;
            baby->hp = 1;
        }
    } else if (e.type == EType::ASTEROID && e.big) {
        int pieces = irand(2, 3);
        for (int i = 0; i < pieces; ++i) {
            Enemy* s = spawnEnemy(EType::ASTEROID, e.x, e.y);
            if (!s) continue;
            s->tex = assets().asteroidSmall;
            s->big = false;
            s->hp = 1;
            s->points = 10;
            s->heavy = false;
            s->explosionSize = 0.6f;
            s->vx = irand(-120, 120) * speedMult;
            s->vy = irand(60, 160) * speedMult;
        }
    }
}

void GameScene::updateEnemies(float dms) {
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (enemies[i].active) updateEnemy(enemies[i], dms);
    }
}

void GameScene::updateEnemy(Enemy& e, float dms) {
    float dt = dms / 1000.0f;
    float sm = speedMult;

    switch (e.type) {
        case EType::WEAVER:
            e.vx = std::sin(time / 200) * 180;
            break;

        case EType::SWOOPER: {
            const float target = 100;
            if (std::fabs(e.heading - target) > 1) {
                e.heading += e.turn * dt;
                if ((e.turn > 0 && e.heading > target) || (e.turn < 0 && e.heading < target)) e.heading = target;
            }
            float rad = e.heading * DEG;
            e.vx = std::cos(rad) * e.speed;
            e.vy = std::sin(rad) * e.speed;
            e.rot = rad - PI / 2;
            break;
        }

        case EType::BLINKER:
            if (e.blinkT >= 0) {
                // Shrink/fade out, jump sideways, then reappear and shoot.
                float before = e.blinkT;
                e.blinkT += dms;
                if (before < 160 && e.blinkT >= 160) {
                    sparkBurst(e.x, e.y, 6);
                    e.x = std::clamp(e.x + irand(-140, 140), 30.0f, SCREEN_WIDTH - 30.0f);
                }
                float k = e.blinkT < 160 ? e.blinkT / 160 : std::max(0.0f, 1 - (e.blinkT - 160) / 160);
                e.alpha = 1 - k;
                e.sx = 1 - 0.8f * k;
                if (e.blinkT >= 320) {
                    e.blinkT = -1;
                    e.alpha = e.sx = 1;
                    e.invulnerable = false;
                    if (e.y > 0) enemyShoot(e, aimFrom(e.x, e.y), Ability::PURPLE, 260);
                }
            } else {
                e.timer -= dms;
                if (e.timer <= 0) {
                    e.timer = 1700;
                    e.invulnerable = true;
                    e.blinkT = 0;
                }
            }
            break;

        case EType::KAMIKAZE: {
            e.speed = std::min(e.speed + dms * 0.25f, 340 * sm);
            float heading = rotateTo(e.rot + PI / 2, aimFrom(e.x, e.y), 0.003f * dms);
            e.rot = heading - PI / 2;
            e.vx = std::cos(heading) * e.speed;
            e.vy = std::sin(heading) * e.speed;
            break;
        }

        case EType::POD:
            e.vx = std::sin(time / 600) * 60;
            e.cooldown -= dms;
            if (e.cooldown <= 0 && e.y > 0) {
                enemyShoot(e, aimFrom(e.x, e.y), Ability::BLUE, 200);
                e.cooldown = 2500;
            }
            break;

        case EType::GUNSHIP:
        case EType::SPREADER:
        case EType::LASER: {
            if (e.type == EType::LASER) {
                // Hover around 22% of the screen while charging/firing, then drift down.
                float parkY = SCREEN_HEIGHT * 0.22f;
                e.vy = e.beam > 0 ? 0 : e.y < parkY ? 120 : 25 * sm;
            }
            e.cooldown -= dms;
            if (e.cooldown <= 0 && e.y > 0) {
                if (e.type == EType::GUNSHIP) {
                    enemyShoot(e, aimFrom(e.x, e.y), Ability::RED, 250);
                } else if (e.type == EType::SPREADER) {
                    float base = aimFrom(e.x, e.y);
                    int n = level->id >= 8 ? 5 : 3;
                    for (int i = 0; i < n; ++i) enemyShoot(e, base + (i - (n - 1) / 2.0f) * 0.26f, Ability::ORANGE, 230, i == 0);
                } else {
                    e.beam = 1;
                }
                e.cooldown = e.fireEvery;
            }
            if (e.type == EType::LASER && e.beam > 0) {
                e.beam += dms;
                float top = e.y + Graphics::Renderer::texHeight(e.tex) * 0.45f;
                if (e.beam >= 800 && e.beam < 1300) {
                    if (std::fabs(px - e.x) < 18 && py > top) damagePlayer();
                } else if (e.beam >= 1300) {
                    e.beam = 0;
                }
            }
            break;
        }

        case EType::LANCER:
            if (e.phase == 0 && e.y >= e.stopY) {
                e.phase = 1;
                e.timer = 650;
                e.vx = e.vy = 0;
            } else if (e.phase == 1) {
                e.timer -= dms;
                float a = aimFrom(e.x, e.y);
                e.rot = a - PI / 2;
                e.tint = ((int)(e.timer / 80) % 2) ? 0xffffff : abilityDef(Ability::YELLOW).color;
                if (e.timer <= 0) {
                    e.phase = 2;
                    e.tint = 0xffffff;
                    float s = 520 * sm;
                    e.vx = std::cos(a) * s;
                    e.vy = std::sin(a) * s;
                }
            }
            break;

        case EType::SPLITTER:
            e.cooldown -= dms;
            if (e.cooldown <= 0 && e.y > 0) {
                enemyShoot(e, 65 * DEG, Ability::GREEN, 220);
                enemyShoot(e, 115 * DEG, Ability::GREEN, 220, false);
                e.cooldown = 2000;
            }
            break;

        case EType::MINE: {
            e.timer -= dms;
            // accelerateToObject(player, 40, maxSpeed 90)
            float a = aimFrom(e.x, e.y);
            e.vx = std::clamp(e.vx + std::cos(a) * 40 * dt, -90.0f, 90.0f);
            e.vy = std::clamp(e.vy + std::sin(a) * 40 * dt, -90.0f, 90.0f);
            if (e.timer < 800) e.tint = ((int)(e.timer / 100) % 2) ? 0xffffff : 0xff3333;
            if (e.timer <= 0) {
                for (int i = 0; i < 8; ++i) fireEnemyBullet(e.x, e.y, 2 * PI * i / 8, 170, i == 0);
                explode(e.x, e.y, 0.6f);
                e.active = false;
                return;
            }
            break;
        }

        case EType::PART: {
            const float W = SCREEN_WIDTH;
            switch (e.follow) {
                case Follow::ORBIT: {
                    float a = time / 1000 * e.fSpeed + e.fOffset;
                    e.x = boss.x + std::cos(a) * e.fRadius;
                    e.y = boss.y + std::sin(a) * e.fRadius * 0.8f;
                    break;
                }
                case Follow::WING:
                    e.x = boss.x + e.fSide * bossDisplayW() * 0.36f;
                    e.y = boss.y + 8;
                    break;
                case Follow::BARRIER:
                    e.x = boss.x + std::sin(time / 700 + e.fOffset * 2.1f) * W * 0.3f;
                    e.y = boss.y + bossDisplayH() * 0.75f;
                    break;
                case Follow::FIXED:
                    e.x = e.fx;
                    e.y = e.fy;
                    break;
                case Follow::NONE:
                    break;
            }
            e.rot += e.angVel * DEG * dt;
            if (!e.armed) return;
            e.rot = std::atan2(py - e.y, px - e.x) - PI / 2;
            e.cooldown -= dms;
            if (e.cooldown <= 0) {
                fireEnemyBulletAt(e.x, e.y, 250, e.fake);
                e.cooldown = e.fireEvery;
            }
            return; // parts never leave the screen on their own
        }

        default:
            break;
    }

    e.x += e.vx * dt;
    e.y += e.vy * dt;
    e.rot += e.angVel * DEG * dt;
    if (e.y > SCREEN_HEIGHT + 120 || e.x < -200 || e.x > SCREEN_WIDTH + 200) e.active = false;
}

// --- Waves ---

float GameScene::randX() const {
    return (float)irand(30, SCREEN_WIDTH - 30);
}

EnemyKind GameScene::pickKind(const std::function<bool(EnemyKind)>& filter) {
    // Plain drones stay the backbone of every stream; specials are rarer.
    std::vector<EnemyKind> bag;
    for (EnemyKind k : roster(level->id)) {
        if (!filter(k)) continue;
        int weight = k == EnemyKind::DRONE ? 4 : k == EnemyKind::WEAVER ? 3 : 1;
        for (int i = 0; i < weight; ++i) bag.push_back(k);
    }
    return bag.empty() ? EnemyKind::DRONE : bag[std::rand() % bag.size()];
}

void GameScene::queueSpawn(float delay, std::function<void()> fn) {
    pendingSpawns++;
    int serial = waveSerial;
    after(800 + delay, [this, serial, fn] {
        if (serial != waveSerial) return; // sector restarted
        if (!wavesDone) fn();
        pendingSpawns--;
    });
}

/** Mixed stream: mostly drones, every 5th enemy is a shooter. */
void GameScene::classic(int count, float offset) {
    for (int i = 0; i < count; ++i) {
        queueSpawn(offset + i * spawnDelay, [this, i] {
            EnemyKind k = pickKind([i](EnemyKind k) { return i % 5 == 1 ? isShooter(k) : !isShooter(k) && k != EnemyKind::KAMIKAZE; });
            spawnKind(k, randX(), -40);
        });
    }
}

void GameScene::stream(int count, std::function<void()> make, float offset) {
    for (int i = 0; i < count; ++i) queueSpawn(offset + i * spawnDelay, make);
}

/** 5 swoopers in a "V" entering in an arc from a top corner. */
void GameScene::vFormation(bool fromLeft, float offset) {
    float x = fromLeft ? -30.0f : SCREEN_WIDTH + 30.0f;
    const int slots[5][2] = {{0, 0}, {1, -35}, {1, 35}, {2, -70}, {2, 70}};
    for (const auto& s : slots) {
        float y = 60.0f + s[1];
        queueSpawn(offset + s[0] * 260, [this, x, y, fromLeft] {
            Enemy* e = spawnEnemy(EType::SWOOPER, x, y);
            if (!e) return;
            e->heading = fromLeft ? 10 : 170;
            e->turn = fromLeft ? 55 : -55;
        });
    }
}

/** Fighters crossing diagonally from both top corners at once. */
void GameScene::pincer(int pairs) {
    const float W = SCREEN_WIDTH, H = SCREEN_HEIGHT, speed = 260;
    float vx = speed * (W / std::hypot(W, H)), vy = speed * (H / std::hypot(W, H));
    for (int i = 0; i < pairs; ++i) {
        queueSpawn(i * 500.0f, [this, vx, vy, W] {
            for (int side : {1, -1}) {
                Enemy* e = spawnEnemy(EType::FIGHTER, side > 0 ? -20 : W + 20, -20);
                if (!e) continue;
                e->vx = side * vx * speedMult;
                e->vy = vy * speedMult;
                e->rot = std::atan2(e->vy, e->vx) - PI / 2;
            }
        });
    }
}

/** Builds one wave; size grows with the sector and the wave number. */
void GameScene::buildWave(WaveKind kind) {
    int n = std::min(4 + level->id / 2 + wave / 2, 12);
    auto shooter = [this] { spawnKind(pickKind([](EnemyKind k) { return isShooter(k); }), randX(), -40); };
    switch (kind) {
        case WaveKind::STREAM:
            classic(n + n / 2, 0);
            break;
        case WaveKind::VFORMATION:
            vFormation(true, 0);
            if (n > 6) vFormation(false, 1800);
            stream(2, shooter, 1200);
            break;
        case WaveKind::PINCER:
            pincer((n + 1) / 2);
            break;
        case WaveKind::ASTEROIDS:
            for (int i = 0; i < (n + 1) / 2; ++i) {
                queueSpawn(i * 1100.0f, [this] {
                    Enemy* e = spawnEnemy(EType::ASTEROID, randX(), -60);
                    if (!e) return;
                    e->vx = irand(-30, 30) * speedMult;
                    e->vy = irand(70, 110) * speedMult;
                });
            }
            break;
        case WaveKind::SHOOTERS:
            stream((n + 1) / 2, [this] { spawnKind(pickKind([](EnemyKind k) { return isShooter(k) || k == EnemyKind::POD; }), randX(), -40); }, 0);
            classic(n / 2, 500);
            break;
        case WaveKind::MINES:
            stream((n + 1) / 2, [this] {
                Enemy* e = spawnEnemy(EType::MINE, randX(), -30);
                if (e) e->vy = 70;
            }, 0);
            classic(n / 3, 300);
            break;
        case WaveKind::SWARM:
            stream(n + 2, [this] { spawnEnemy(EType::KAMIKAZE, randX(), -30); }, 0);
            break;
    }
}

void GameScene::nextWave() {
    if (wavesDone) return;
    if (time >= level->duration * 1000) {
        wavesDone = true;
        startBoss();
        return;
    }
    const auto& pool = level->waves;
    int idx = wave % (int)pool.size();
    if (wave >= (int)pool.size()) {
        std::vector<int> options;
        for (int i = 0; i < (int)pool.size(); ++i) {
            if ((int)pool[i] != lastWaveKind) options.push_back(i);
        }
        if (!options.empty()) idx = options[std::rand() % options.size()];
    }
    WaveKind kind = pool[idx];
    lastWaveKind = (int)kind;
    pendingSpawns = 0;
    buildWave(kind);
    wave++;
    announce(std::string(I18n::L("OLEADA", "WAVE")) + " " + std::to_string(wave));
    if (wave % 2 == 1) spawnPowerupRandom(); // a guaranteed drop every other wave
    waveElapsed = 0;
    waveActive = true;
}

} // namespace Game
