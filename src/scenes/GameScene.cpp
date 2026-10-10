#include "GameScene.hpp"
#include "MathUtil.hpp"
#include "Text.hpp"
#include "I18n.hpp"
#include "TrophyManager.hpp"
#include "audio/Synth.hpp"

namespace Game {

using Graphics::Color;
using Graphics::rgb;
using Graphics::SCREEN_WIDTH;
using Graphics::SCREEN_HEIGHT;
using I18n::L;

const float POWER_DURATIONS[POWER_COUNT] = {5000, 6000, 10000, 10000, 12000, 10000};

static constexpr float DROP_CHANCE = 0.07f;
static constexpr float CRIT_CHANCE = 0.1f;
static constexpr float BULLET_SPEED = 600.0f;
static constexpr float PLAYER_RADIUS = 14.0f;
static constexpr float FOREVER = 1e12f;
static const char* DROP_LABELS[DROP_COUNT] = {"SHIELD", "RAPID FIRE", "SPREAD SHOT", "MISSILES", "HOMING DRONE", "MAGNET", "+1 HYPER BOMB"};

static Audio::Synth& synth() { return Audio::Synth::instance(); }

float Enemy::radius() const {
    return std::min(Graphics::Renderer::texWidth(tex) * std::fabs(sx), Graphics::Renderer::texHeight(tex) * std::fabs(sy)) * 0.42f;
}

// --- Setup ---

void GameScene::start(const CampaignState& c) {
    campaign = c;
    level = &getLevel(c.level);
    ship = &getShipDef(c.ship);
    lt = &assets().levels[level->id - 1];
    const DifficultyDef& diff = difficultyDef(c.difficulty);
    // Each sector ramps speed and spawn rate on top of the chosen difficulty.
    speedMult = diff.speedMultiplier * (1 + (level->id - 1) * 0.05f);
    spawnDelay = diff.spawnDelay * (1 - (level->id - 1) * 0.04f);
    score = ScoreManager(diff.scoreMulti, c.score);
    lastScore = score.score;
    saveCampaign(c); // auto-save at the start of every sector

    pending = Outcome::NONE;
    ended = false;
    kills = hits = 0;
    time = 0;
    bullets.assign(200, Bullet{});
    enemyBullets.assign(400, Bullet{});
    enemies.assign(160, Enemy{});
    powerups.assign(32, Powerup{});
    texts.assign(48, FloatText{});
    decor.assign(16, DecorProp{});
    announcements.clear();
    scheduled.clear();
    ripples.clear();
    particles.clear();
    boss = Boss{};

    px = SCREEN_WIDTH / 2.0f;
    py = SCREEN_HEIGHT - 100.0f;
    pvx = pvy = 0;
    aim = -90;
    lives = c.lives;
    bombs = ship->bombs;
    for (float& p : powers) p = 0;
    invuln = fireCooldown = nextMissile = nextDroneShot = droneOrbit = hitTint = 0;
    shotsFired = 0;
    playerAlive = true;
    playerSpeedMult = ship->speed * (1 + 0.15f * c.perks[(int)PerkKind::ENGINE]);
    fireDelay = 200.0f * std::pow(0.9f, (float)c.perks[(int)PerkKind::PLASMA]) / ship->fireRate;
    if (ship->permanentDrone) activatePower(Power::DRONE, FOREVER);
    if (ship->sectorShield) activatePower(Power::SHIELD, (float)ship->sectorShield);

    wave = 0;
    waveSerial++;
    pendingSpawns = 0;
    waveElapsed = 0;
    waveActive = false;
    wavesDone = false;
    lastWaveKind = -1;

    trauma = hitStopT = flashT = 0;
    bgScroll = 0;
    decorTimer = 600;
    lightningTimer = 5000;
    bossWarningT = -1;
    bossBar = false;
    scorePop = 0;
    bombWaveT = -1;

    after(2200, [this] { nextWave(); });
    announce(std::string("SECTOR ") + std::to_string(level->id) + "\n" + level->name, 0xffdd33);
    after(1700, [this] { announce(L(level->threatEs, level->threatEn), level->unlock != EnemyKind::NONE ? 0xff9d4d : 0xc8d0e8, 2); });

    if (level->id >= 5) TrophyManager::instance().unlock(TrophyId::SECTOR_MASTER);
    synth().playCombatMusic(level->bpm, level->id);
}

void GameScene::after(float ms, std::function<void()> fn) {
    scheduled.push_back({ms, std::move(fn)});
}

// --- Loop ---

Outcome GameScene::update(float dt, const Input::InputState& in) {
    if (in.pause && !ended) return Outcome::PAUSE;
    float dms = dt * 1000.0f;

    updateFx(dms);
    updateBackground(dms);
    if (hitStopT > 0) {
        // Brief freeze (~2 frames) to sell heavy impacts.
        hitStopT -= dms;
        return Outcome::NONE;
    }
    time += dms;

    // Timers (callbacks may schedule more)
    std::vector<std::function<void()>> due;
    for (auto& s : scheduled) {
        s.t -= dms;
        if (s.t <= 0) due.push_back(std::move(s.fn));
    }
    scheduled.erase(std::remove_if(scheduled.begin(), scheduled.end(), [](const Scheduled& s) { return s.t <= 0; }), scheduled.end());
    for (auto& fn : due) fn();

    updatePlayer(dms, in);

    // Waves: next one when the current is cleared (or times out)
    if (!wavesDone && waveActive) {
        waveElapsed += dms;
        bool cleared = pendingSpawns == 0 && countWaveEnemies() == 0;
        if (cleared || waveElapsed > 20000) {
            waveActive = false;
            after(1500, [this] { nextWave(); });
        }
    }

    updateBoss(dms);
    updateEnemies(dms);
    updateBullets(dms);
    updatePowerups(dms);
    collisions();
    score.update(dms);
    if (score.score != lastScore) {
        lastScore = score.score;
        scorePop = 120;
        if (score.score >= 10000) TrophyManager::instance().unlock(TrophyId::SCORE_10K);
    }
    return pending;
}

// --- Player ---

void GameScene::activatePower(Power p, float durationMs) {
    // Never shorten a longer/permanent effect (e.g. Nova's permanent drone).
    float& left = powers[(int)p];
    left = std::max(left, durationMs);
    if (p == Power::SHIELD) shieldWarned = false;
}

void GameScene::updatePlayer(float dms, const Input::InputState& in) {
    if (!playerAlive) return;
    float dt = dms / 1000.0f;

    // Movement: same arcade feel as the first Vita build, scaled by ship speed and the engine perk
    float accel = 1500.0f * playerSpeedMult;
    pvx += in.moveX * accel * dt;
    pvy += in.moveY * accel * dt;
    float drag = std::pow(0.88f, dt * 60.0f);
    pvx *= drag;
    pvy *= drag;
    float spd = std::hypot(pvx, pvy), maxSpd = 360.0f * playerSpeedMult;
    if (spd > maxSpd) {
        pvx = pvx / spd * maxSpd;
        pvy = pvy / spd * maxSpd;
    }
    px = std::clamp(px + pvx * dt, 25.0f, SCREEN_WIDTH - 25.0f);
    py = std::clamp(py + pvy * dt, 25.0f, SCREEN_HEIGHT - 25.0f);

    // Aim: right stick, smoothly rotating instead of snapping
    if (std::hypot(in.aimX, in.aimY) > 0.25f) {
        float target = std::atan2(in.aimY, in.aimX) / DEG;
        float diff = wrapAngle((target - aim) * DEG) / DEG;
        aim += std::clamp(diff, -0.6f * dms, 0.6f * dms);
    }

    fireCooldown -= dms;
    if (in.fire && fireCooldown <= 0) firePlayer();
    if (in.bomb) useBomb();

    for (int i = 0; i < POWER_COUNT; ++i) {
        if (powers[i] > 0) powers[i] = std::max(0.0f, powers[i] - dms);
    }
    if (hasPower(Power::SHIELD) && powers[(int)Power::SHIELD] < 1500 && !shieldWarned) {
        shieldWarned = true;
        synth().playShieldWarning();
    }

    if (hasPower(Power::DRONE)) {
        droneOrbit += dms * 0.004f;
        float dx = px + std::cos(droneOrbit) * 45, dy = py + std::sin(droneOrbit) * 45;
        nextDroneShot -= dms;
        if (nextDroneShot <= 0) {
            float tx, ty;
            float angle = nearestTarget(dx, dy, tx, ty) ? std::atan2(ty - dy, tx - dx) / DEG : aim;
            spawnPlayerBullet(dx, dy, angle, assets().orb, 320, 1, true);
            nextDroneShot = 450;
        }
    }

    if (hasPower(Power::MAGNET)) {
        for (auto& p : powerups) {
            if (!p.active) continue;
            float d = std::sqrt(dist2(p.x, p.y, px, py));
            if (d < 350 && d > 1) {
                p.vx = (px - p.x) / d * 350;
                p.vy = (py - p.y) / d * 350;
            }
        }
    }

    if (invuln > 0) invuln -= dms;
    if (hitTint > 0) hitTint -= dms;

    // Thruster trail behind the ship; denser when moving
    float back = aim * DEG;
    particles.spawnThruster(px - std::cos(back) * 16, py - std::sin(back) * 16, aim, std::hypot(pvx, pvy));
}

void GameScene::spawnPlayerBullet(float x, float y, float angleDeg, Tex* tex, float speed, int damage, bool homing) {
    for (auto& b : bullets) {
        if (b.active) continue;
        b = Bullet{};
        b.active = true;
        b.x = x;
        b.y = y;
        b.vx = std::cos(angleDeg * DEG) * speed;
        b.vy = std::sin(angleDeg * DEG) * speed;
        b.damage = damage;
        b.homing = homing;
        b.tex = tex;
        shotsFired++;
        return;
    }
}

void GameScene::firePlayer() {
    float rad = aim * DEG;
    float bx = px + std::cos(rad) * 20, by = py + std::sin(rad) * 20;
    int extra = campaign.perks[(int)PerkKind::MAGAZINE] + ship->extraShots;
    if (hasPower(Power::SPREAD)) {
        // 3 to 5 projectiles in an arc.
        int n = std::min(5, 3 + extra);
        for (int i = 0; i < n; ++i) spawnPlayerBullet(bx, by, aim + (i - (n - 1) / 2.0f) * 15, assets().bullet, BULLET_SPEED, 1, false);
    } else {
        // Standard: single, or parallel double/triple with the magazine perk.
        int n = std::min(3, 1 + extra);
        for (int i = 0; i < n; ++i) {
            float off = (i - (n - 1) / 2.0f) * 10;
            spawnPlayerBullet(bx - std::sin(rad) * off, by + std::cos(rad) * off, aim, assets().bullet, BULLET_SPEED, 1, false);
        }
    }
    fireCooldown = hasPower(Power::RAPID) ? fireDelay / 2 : fireDelay;
    synth().playLaser();

    if (hasPower(Power::MISSILE) && time > nextMissile) {
        // Launch sideways from both wings; homing steers them onto targets.
        spawnPlayerBullet(px, py, aim - 70, assets().missile, 300, 2, true);
        spawnPlayerBullet(px, py, aim + 70, assets().missile, 300, 2, true);
        nextMissile = time + 700;
        synth().playMissile();
    }
}

void GameScene::useBomb() {
    if (bombs <= 0 || !playerAlive) return;
    bombs--;
    hyperBomb();
}

/** Secondary fire: expanding shockwave that wipes enemy bullets and hurts everything. */
void GameScene::hyperBomb() {
    synth().playBomb();
    flashScreen(300, 0xb4dcff);
    shake(0.6f);
    explode(px, py, 2);
    bombWaveT = 0;
    bombX = px;
    bombY = py;
    for (auto& b : enemyBullets) {
        if (!b.active) continue;
        sparkBurst(b.x, b.y, 2);
        b.active = false;
    }
    for (size_t i = 0; i < enemies.size(); ++i) {
        Enemy& e = enemies[i];
        if (e.active && damageEnemy(e, 5)) killEnemy(e);
    }
    damageBoss(5);
    TrophyManager::instance().unlock(TrophyId::HYPER_BOMB);
}

void GameScene::damagePlayer() {
    if (!playerAlive || ended) return;
    if (hasPower(Power::SHIELD)) {
        shieldRipple(px, py, 30);
        synth().playShieldHit();
        TrophyManager::instance().unlock(TrophyId::UNTOUCHABLE);
        return;
    }
    if (invuln > 0) return;
    lives--;
    invuln = (float)ship->invulnMs;
    hitTint = 200;
    if (lives == 1) synth().playCriticalWarning();
    score.resetCombo();
    explode(px, py, 0.6f);
    shake(0.55f);
    synth().playExplosion();
    if (lives <= 0) gameOver();
}

void GameScene::gameOver() {
    playerAlive = false;
    if (ended) return;
    ended = true;
    finalScore = score.score;
    newRecord = ScoreManager::submit(finalScore, (int)campaign.difficulty);
    synth().setMusicIntensity(false);
    explode(px, py, 2);
    // Retry restarts the current sector from its auto-save (score/perks at sector start).
    after(900, [this] { pending = Outcome::GAME_OVER; });
}

/** Boss destroyed: bonus, then perk selection (or the final victory). */
void GameScene::levelComplete() {
    if (ended) return;
    ended = true;
    float accuracy = shotsFired ? std::min(1.0f, (float)hits / shotsFired) : 0.0f;
    int bonus = (int)std::lround((accuracy * 1000 + kills * 10) * level->id);
    score.score += bonus;
    stats = SectorStats{kills, accuracy, bonus};
    resultCampaign = campaign;
    resultCampaign.score = score.score;
    resultCampaign.lives = lives;
    finalScore = score.score;
    synth().setMusicIntensity(false);
    TrophyManager::instance().unlock(TrophyId::BOSS_SLAYER);

    if (level->id >= LEVEL_COUNT) {
        newRecord = ScoreManager::submit(finalScore, (int)campaign.difficulty);
        clearCampaign();
        pending = Outcome::VICTORY;
        return;
    }
    pending = Outcome::LEVEL_CLEAR;
}

// --- Bullets / power-ups ---

Bullet* GameScene::fireEnemyBullet(float x, float y, float angle, float speed, bool sound, bool fake, Tex* tex) {
    for (auto& b : enemyBullets) {
        if (b.active) continue;
        b = Bullet{};
        b.active = true;
        b.x = x;
        b.y = y;
        float s = speed * (0.85f + speedMult * 0.1f);
        b.vx = std::cos(angle) * s;
        b.vy = std::sin(angle) * s;
        b.fake = fake;
        b.tex = tex ? tex : lt->eb; // default: the sector's boss-colored bullet
        if (sound) synth().playEnemyShoot();
        return &b;
    }
    return nullptr;
}

void GameScene::fireEnemyBulletAt(float x, float y, float speed, bool fake) {
    fireEnemyBullet(x, y, std::atan2(py - y, px - x), speed, true, fake);
}

bool GameScene::nearestTarget(float x, float y, float& tx, float& ty) const {
    float best = 1e18f;
    bool found = false;
    for (const auto& e : enemies) {
        if (!e.active || e.invulnerable || e.y < 0) continue;
        float d = dist2(x, y, e.x, e.y);
        if (d < best) { best = d; tx = e.x; ty = e.y; found = true; }
    }
    if (boss.active && boss.visible && !bossInvulnerable() && boss.y >= 0) {
        float d = dist2(x, y, boss.x, boss.y);
        if (d < best) { tx = boss.x; ty = boss.y; found = true; }
    }
    return found;
}

void GameScene::updateBullets(float dms) {
    float dt = dms / 1000.0f;
    for (auto& b : bullets) {
        if (!b.active) continue;
        if (b.homing) {
            // Steers toward the nearest target and accelerates.
            float heading = std::atan2(b.vy, b.vx), tx, ty;
            if (nearestTarget(b.x, b.y, tx, ty)) heading = rotateTo(heading, std::atan2(ty - b.y, tx - b.x), 0.006f * dms);
            float s = std::min(std::hypot(b.vx, b.vy) + dms * 0.6f, 550.0f);
            b.vx = std::cos(heading) * s;
            b.vy = std::sin(heading) * s;
        }
        b.x += b.vx * dt;
        b.y += b.vy * dt;
        if (b.x < -60 || b.x > SCREEN_WIDTH + 60 || b.y < -60 || b.y > SCREEN_HEIGHT + 60) b.active = false;
    }
    for (auto& b : enemyBullets) {
        if (!b.active) continue;
        b.x += b.vx * dt;
        b.y += b.vy * dt;
        if (b.x < -60 || b.x > SCREEN_WIDTH + 60 || b.y < -60 || b.y > SCREEN_HEIGHT + 60) b.active = false;
    }
}

void GameScene::spawnPowerup(float x, float y) {
    for (auto& p : powerups) {
        if (p.active) continue;
        p = Powerup{};
        p.active = true;
        p.x = x;
        p.y = y;
        p.vy = 100;
        p.kind = static_cast<Power>(std::rand() % DROP_COUNT);
        return;
    }
}

void GameScene::spawnPowerupRandom() {
    spawnPowerup((float)irand(30, SCREEN_WIDTH - 30), -30);
}

void GameScene::updatePowerups(float dms) {
    float dt = dms / 1000.0f;
    for (auto& p : powerups) {
        if (!p.active) continue;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        if (p.y > SCREEN_HEIGHT + 100 || p.x < -100 || p.x > SCREEN_WIDTH + 100) p.active = false;
    }
}

void GameScene::collectPowerup(Powerup& p) {
    p.active = false;
    synth().playPowerUp();
    floatingText(px, py - 30, DROP_LABELS[(int)p.kind], 0x00ffcc, 2);
    if (p.kind == Power::BOMB) bombs = std::min(MAX_BOMBS, bombs + 1);
    else activatePower(p.kind, POWER_DURATIONS[(int)p.kind]);
}

// --- Combat ---

bool GameScene::damageEnemy(Enemy& e, int amount) {
    if (e.invulnerable) return false;
    if (e.type == EType::POD && e.shield > 0) {
        e.shield--;
        e.bubbleFlash = 60;
        if (e.shield == 0) shieldRipple(e.x, e.y, 32);
        return false;
    }
    e.hp -= amount;
    return e.hp <= 0;
}

void GameScene::killEnemy(Enemy& e) {
    float x = e.x, y = e.y, size = e.explosionSize;
    bool heavy = e.heavy;
    bool crit = frand(0, 1) < CRIT_CHANCE;
    bool tierUp = false;
    int points = score.addKill(e.points, crit, tierUp);
    kills++;
    onKilled(e); // may spawn children (uses e's position before it is freed)
    e.active = false;

    explode(x, y, size);
    shake(0.12f * size);
    floatingText(x, y, crit ? "CRITICAL! +" + std::to_string(points) : "+" + std::to_string(points), crit ? 0xff5555 : 0xffffff, crit ? 3 : 2);
    if (tierUp) {
        float mult = score.multiplier();
        char buf[24];
        std::snprintf(buf, sizeof(buf), "COMBO x%.1f", mult);
        floatingText(x, y - 30, buf, 0xffdd33, 3);
        synth().playCombo(mult >= 3 ? 2 : mult >= 2 ? 1 : 0);
    }
    synth().playExplosion(heavy);
    TrophyManager::instance().unlock(TrophyId::FIRST_BLOOD);
    if (frand(0, 1) < DROP_CHANCE) spawnPowerup(x, y);
}

void GameScene::collisions() {
    // Player shots vs enemies and boss
    for (auto& b : bullets) {
        if (!b.active) continue;
        for (auto& e : enemies) {
            if (!e.active) continue;
            float r = e.radius() + 4;
            if (dist2(b.x, b.y, e.x, e.y) > r * r) continue;
            b.active = false;
            hits++;
            if (e.invulnerable) {
                sparkBurst(b.x, b.y, 3);
                break;
            }
            if (damageEnemy(e, b.damage)) {
                killEnemy(e);
                break;
            }
            sparkBurst(e.x, e.y);
            if (e.heavy) {
                e.flash = 60;
                hitStop();
                shake(0.08f);
            }
            break;
        }
        if (!b.active || !boss.active || !boss.visible || bossInvulnerable()) continue;
        float nx = (b.x - boss.x) / (bossDisplayW() * 0.45f), ny = (b.y - boss.y) / (bossDisplayH() * 0.4f);
        if (nx * nx + ny * ny <= 1) {
            hits++;
            sparkBurst(b.x, b.y);
            b.active = false;
            damageBoss(b.damage);
        }
    }

    if (!playerAlive || ended) return;

    for (auto& b : enemyBullets) {
        if (!b.active) continue;
        float r = PLAYER_RADIUS + 4 * b.scale;
        if (dist2(b.x, b.y, px, py) > r * r) continue;
        b.active = false;
        if (!b.fake) damagePlayer();
    }

    for (auto& e : enemies) {
        if (!e.active) continue;
        float r = e.radius() * 0.8f + PLAYER_RADIUS;
        if (dist2(e.x, e.y, px, py) > r * r) continue;
        if (e.solid) {
            damagePlayer();
        } else if (hasPower(Power::SHIELD)) {
            shieldRipple(px, py, 30);
            killEnemy(e);
        } else {
            explode(e.x, e.y, e.explosionSize);
            e.active = false;
            damagePlayer();
        }
        if (!playerAlive) return;
    }

    if (boss.active && boss.visible && !bossInvulnerable()) {
        float nx = (px - boss.x) / (bossDisplayW() * 0.45f), ny = (py - boss.y) / (bossDisplayH() * 0.4f);
        if (nx * nx + ny * ny <= 1) {
            damagePlayer();
            pvy = 300;
        }
    }

    for (auto& p : powerups) {
        if (p.active && dist2(p.x, p.y, px, py) < 30 * 30) collectPowerup(p);
    }
}

// --- FX ---

void GameScene::explode(float x, float y, float size) {
    particles.explode(x, y, size);
}

void GameScene::sparkBurst(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        float a = frand(0, 2 * PI), s = frand(120, 380);
        particles.spawn(Graphics::ParticleType::SPARK, x, y, std::cos(a) * s, std::sin(a) * s, frand(0.2f, 0.5f), 3.0f, 255, (uint8_t)irand(160, 255), 80);
    }
}

void GameScene::shieldRipple(float x, float y, float radius) {
    ripples.push_back({x, y, radius, 0});
}

void GameScene::floatingText(float x, float y, const std::string& text, uint32_t color, float scale) {
    for (auto& t : texts) {
        if (t.active) continue;
        t.active = true;
        t.x = x;
        t.y = y;
        t.text = text;
        t.color = color;
        t.scale = scale;
        t.t = 0;
        return;
    }
}

void GameScene::announce(const std::string& text, uint32_t color, float scale) {
    announcements.push_back({text, color, scale, 0});
}

void GameScene::flashScreen(float ms, uint32_t color) {
    flashT = flashDur = ms;
    flashColor = color;
}

void GameScene::updateFx(float dms) {
    particles.update(dms / 1000.0f);
    for (auto& t : texts) {
        if (!t.active) continue;
        t.t += dms;
        if (t.t > 950) t.active = false;
    }
    for (auto& a : announcements) a.t += dms;
    announcements.erase(std::remove_if(announcements.begin(), announcements.end(), [](const Announcement& a) { return a.t > 1500; }), announcements.end());
    for (auto& r : ripples) r.t += dms;
    ripples.erase(std::remove_if(ripples.begin(), ripples.end(), [](const Ripple& r) { return r.t > 300; }), ripples.end());
    trauma = trauma <= 0.001f ? 0 : trauma * std::exp(-dms / 180.0f);
    if (flashT > 0) flashT -= dms;
    if (bombWaveT >= 0) {
        bombWaveT += dms;
        if (bombWaveT > 600) bombWaveT = -1;
    }
    if (scorePop > 0) scorePop -= dms;
    if (bossWarningT >= 0) {
        bossWarningT += dms;
        if (bossWarningT >= 3000) {
            bossWarningT = -1;
            bossBar = true;
        }
    }
    for (auto& e : enemies) {
        if (e.active && e.flash > 0) e.flash -= dms;
        if (e.active && e.bubbleFlash > 0) e.bubbleFlash -= dms;
    }
    if (boss.flash > 0) boss.flash -= dms;
}

/** Themed background behavior per sector (props drifting by, lightning). */
void GameScene::updateBackground(float dms) {
    float dt = dms / 1000.0f;
    bgScroll += (0.6f + level->id * 0.05f) * 60.0f * dt;

    if (level->decor != Decor::NONE) {
        decorTimer -= dms;
        if (decorTimer <= 0) {
            decorTimer += 2600;
            for (auto& d : decor) {
                if (d.active) continue;
                bool flat = level->decor == Decor::RINGS || level->decor == Decor::STRUCTURES;
                d.active = true;
                d.x = (float)irand(0, SCREEN_WIDTH);
                d.y = -80;
                d.alpha = frand(0.25f, 0.5f);
                d.scale = frand(0.6f, 1.3f);
                d.angle = flat ? 0 : (float)irand(0, 360);
                float duration = (float)irand(12000, 20000) / 1000.0f;
                d.vy = (SCREEN_HEIGHT + 200) / duration;
                d.angVel = flat ? 0 : irand(-40, 40) / duration;
                break;
            }
        }
        for (auto& d : decor) {
            if (!d.active) continue;
            d.y += d.vy * dt;
            d.angle += d.angVel * dt;
            if (d.y > SCREEN_HEIGHT + 120) d.active = false;
        }
    }
    if (level->fx == BgFx::LIGHTNING) {
        lightningTimer -= dms;
        if (lightningTimer <= 0) {
            lightningTimer += 5000;
            if (frand(0, 1) < 0.7f) flashScreen(120, 0xff8c5a);
        }
    }
}

// --- Rendering ---

void GameScene::renderEnemy(Graphics::Renderer& r, const Enemy& e) {
    Color c = rgb(e.tint, (uint8_t)(255 * std::clamp(e.alpha, 0.0f, 1.0f)));
    r.drawSprite(e.tex, e.x, e.y, e.rot / DEG, e.sx, e.sy, c);
    if (e.flash > 0) r.drawSprite(e.tex, e.x, e.y, e.rot / DEG, e.sx, e.sy, rgb(0xffffff, 200), true);

    if (e.type == EType::POD && e.shield > 0) {
        uint32_t col = abilityDef(Ability::BLUE).color;
        r.fillCircle(e.x, e.y, 32, e.bubbleFlash > 0 ? rgb(0xffffff, 128) : rgb(col, 46));
        r.drawCircle(e.x, e.y, 32, 2, rgb(col, 230));
    }
    if (e.type == EType::LASER && e.beam > 0) {
        uint32_t col = abilityDef(Ability::CYAN).color;
        float top = e.y + Graphics::Renderer::texHeight(e.tex) * 0.45f;
        if (e.beam < 800) {
            r.drawLine(e.x, top, e.x, SCREEN_HEIGHT, 2, rgb(col, ((int)(e.beam / 100) % 2) ? 230 : 77));
        } else if (e.beam < 1300) {
            r.drawRect(e.x - 9, top, 18, SCREEN_HEIGHT - top, rgb(col, 140));
            r.drawRect(e.x - 3, top, 6, SCREEN_HEIGHT - top, rgb(0xffffff, 230));
        }
    }
}

void GameScene::render(Graphics::Renderer& r) {
    r.clear();
    if (trauma > 0.001f) {
        float s = trauma * trauma * 14;
        r.setOffset(frand(-s, s), frand(-s, s));
    }

    uint8_t bgAlpha = 255;
    if (level->fx == BgFx::PULSE) {
        float period = 60000.0f / level->bpm;
        bgAlpha = (uint8_t)(255 * (0.8f + 0.2f * std::cos(time / period * PI)));
    }
    int scrollX = level->fx == BgFx::DISTORTION ? (int)(std::sin(time / 700) * 12) : 0;
    if (level->fx == BgFx::GRAVITY) scrollX = (int)(std::sin(time / 3000 * PI) * 6);
    r.drawBackgroundTiled(assets().background(level->id), scrollX, (int)bgScroll, bgAlpha);

    Tex* decorTex = assets().decor[(int)level->decor];
    for (const auto& d : decor) {
        if (d.active) r.drawSprite(decorTex, d.x, d.y, d.angle, d.scale, d.scale, rgb(0xffffff, (uint8_t)(255 * d.alpha)));
    }

    for (const auto& e : enemies) {
        if (e.active) renderEnemy(r, e);
    }
    for (const auto& p : powerups) {
        if (p.active) r.drawSprite(assets().powerups[(int)p.kind], p.x, p.y);
    }
    for (const auto& b : bullets) {
        if (b.active) r.drawSprite(b.tex, b.x, b.y, std::atan2(b.vy, b.vx) / DEG, b.scale, b.scale);
    }

    // Boss: beam, gravity well, hull and pulsing core
    if (boss.active && boss.visible) {
        if (boss.beam.active && boss.state == BossState::ATTACK_LASER) {
            const Beam& bm = boss.beam;
            float ox = boss.x, oy = boss.y + bossDisplayH() * 0.35f;
            float ex = ox + std::cos(bm.angle) * 2000, ey = oy + std::sin(bm.angle) * 2000;
            if (bm.t < bm.telegraph) {
                r.drawLine(ox, oy, ex, ey, 2, rgb(0xff2222, ((int)(bm.t / 100) % 2 == 0) ? 230 : 90));
            } else {
                r.drawLine(ox, oy, ex, ey, bm.curWidth, rgb(0xff2244, 140));
                r.drawLine(ox, oy, ex, ey, bm.curWidth / 3, rgb(0xffffff, 242));
            }
        }
        if (level->boss == BossGenerator::BossKind::TITAN && boss.state == BossState::SPECIAL) {
            for (int i = 0; i < 4; ++i) {
                float rr = 40 + std::fmod(std::fmod(i * 60 - boss.stateTime * 0.12f, 240.0f) + 240.0f, 240.0f);
                r.drawCircle(boss.x, boss.y, rr, 2, rgb(0xffffff, (uint8_t)(255 * (0.15f + 0.2f * (1 - rr / 280)))));
            }
        }
        Color bc = rgb(boss.tint, (uint8_t)(255 * std::clamp(boss.alpha, 0.0f, 1.0f)));
        r.drawSprite(lt->boss, boss.x, boss.y, 0, 1.5f, 1.5f, bc);
        if (boss.flash > 0) r.drawSprite(lt->boss, boss.x, boss.y, 0, 1.5f, 1.5f, rgb(0xffffff, 180), true);
        float pulse = 0.5f + 0.5f * std::sin(time / 380 * PI);
        float cs = 1 + 0.4f * pulse;
        r.drawSprite(lt->core, boss.x, boss.y + bossDisplayH() * 0.05f, 0, cs, cs, rgb(0xffffff, (uint8_t)(255 * (0.5f - 0.35f * pulse) * boss.alpha)), true);
    }

    for (const auto& b : enemyBullets) {
        if (b.active) r.drawSprite(b.tex, b.x, b.y, std::atan2(b.vy, b.vx) / DEG, b.scale, b.scale, rgb(0xffffff, b.fake ? 115 : 255));
    }

    particles.render(r);

    if (playerAlive) {
        bool blink = invuln > 0 && ((int)(invuln / 100) % 2 == 0);
        Color pc = rgb(hitTint > 0 ? 0xff0000 : 0xffffff, blink ? 77 : 255);
        r.drawSprite(assets().ships[(int)campaign.ship], px, py, aim, 1, 1, pc);
        if (hasPower(Power::SHIELD)) {
            float left = powers[(int)Power::SHIELD];
            uint8_t a = (left < 1500 && ((int)(time / 120) % 2 == 0)) ? 77 : 255;
            r.fillCircle(px, py, 30, rgb(0x0066ff, (uint8_t)(64 * a / 255)));
            r.drawCircle(px, py, 30, 2, rgb(0x00ffff, a));
        }
        if (hasPower(Power::DRONE)) {
            r.drawSprite(assets().drone, px + std::cos(droneOrbit) * 45, py + std::sin(droneOrbit) * 45);
        }
    }

    for (const auto& rp : ripples) {
        float k = rp.t / 300.0f;
        r.drawCircle(rp.x, rp.y, rp.r * (1 + 0.8f * k), 4, rgb(0x66ffff, (uint8_t)(255 * (1 - k))));
    }
    if (bombWaveT >= 0) {
        float k = bombWaveT / 600.0f;
        float ease = 1 - (1 - k) * (1 - k) * (1 - k);
        r.drawCircle(bombX, bombY, 20 + ease * SCREEN_WIDTH, 10, rgb(0x66ccff, (uint8_t)(230 * (1 - k))));
    }

    for (const auto& t : texts) {
        if (!t.active) continue;
        float s = t.t < 120 ? t.scale * (0.5f + 0.5f * t.t / 120) : t.scale;
        float k = t.t < 350 ? 0 : (t.t - 350) / 600;
        UI::drawText(r, t.text, t.x, t.y - 50 * k - 4 * s, s, rgb(t.color, (uint8_t)(255 * (1 - k))), UI::Align::CENTER);
    }

    r.setOffset(0, 0);
    if (flashT > 0) r.drawRect(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, rgb(flashColor, (uint8_t)(200 * flashT / flashDur)));
    renderHud(r);
}

void GameScene::renderHud(Graphics::Renderer& r) {
    using UI::Align;
    const float W = SCREEN_WIDTH;

    // Score, record line, combo
    float ss = 3 + (scorePop > 0 ? 0.4f * scorePop / 120 : 0);
    UI::drawText(r, "SCORE " + std::to_string(score.score), 16, 12, ss, rgb(0xffffff));
    UI::drawText(r, std::string("HI ") + std::to_string(std::max(ScoreManager::best(), 0)) + " · " + difficultyName(campaign.difficulty) + " · S" + std::to_string(level->id),
                 16, 42, 1.5f, rgb(0xaaaaaa));
    if (score.comboTimer > 0) {
        if (score.multiplier() > 1) {
            char buf[24];
            std::snprintf(buf, sizeof(buf), "COMBO x%.1f", score.multiplier());
            UI::drawText(r, buf, 16, 62, 2, rgb(0xffdd33));
        }
        r.drawRect(16.0f, 84.0f, 160.0f, 6.0f, rgb(0x000000, 128));
        r.drawRect(16.0f, 84.0f, 160 * score.comboTimer / COMBO_WINDOW, 6.0f, rgb(0xffdd33));
    }

    // Power-up timers
    int slot = 0;
    for (int i = 0; i < POWER_COUNT; ++i) {
        float left = powers[i];
        if (left <= 0) continue;
        float x = 34 + slot * 44.0f, y = 120;
        Tex* icon = assets().powerups[i];
        float s = 28.0f / std::max(1, Graphics::Renderer::texWidth(icon));
        bool dim = left < 1500 && ((int)(left / 150) % 2);
        r.drawSprite(icon, x, y, 0, s, s, rgb(0xffffff, dim ? 102 : 255));
        r.drawCircle(x, y, 19, 3, rgb(0x333333, 204));
        float frac = std::min(1.0f, left / POWER_DURATIONS[i]);
        r.drawArc(x, y, 19, 3, -PI / 2, -PI / 2 + 2 * PI * frac, rgb(0x00ffcc));
        slot++;
    }

    // Lives (ship icons) and bombs on the right
    int shown = std::min(lives, 5);
    float right = W - 16 - 14;
    std::string extraLives = lives > 5 ? "+" + std::to_string(lives - 5) : "";
    float shift = lives > 5 ? UI::textWidth(extraLives, 2) + 6 : 0;
    if (lives > 5) UI::drawText(r, extraLives, W - 16, 20, 2, rgb(0xffffff), Align::RIGHT);
    Tex* shipTex = assets().ships[(int)campaign.ship];
    float iconS = 26.0f / std::max(1, std::max(Graphics::Renderer::texWidth(shipTex), Graphics::Renderer::texHeight(shipTex)));
    for (int i = 0; i < shown; ++i) {
        r.drawSprite(shipTex, right - shift - i * 30, 26, -90, iconS, iconS, rgb(lives == 1 ? 0xff4444 : 0xffffff));
    }
    UI::drawText(r, "HYPER BOMB x" + std::to_string(bombs), W - 16, 50, 2, rgb(0x66ccff), Align::RIGHT);

    // Sector progress until the boss alarm
    if (!boss.active) {
        float w = 240, x = W / 2 - w / 2;
        float prog = std::min(1.0f, time / (level->duration * 1000));
        r.drawRect(x, 20.0f, w, 6.0f, rgb(0x000000, 128));
        r.drawRect(x, 20.0f, w * prog, 6.0f, rgb(0xff4455));
        r.drawRect(x + w - 2, 16.0f, 4.0f, 14.0f, rgb(0xff4455));
    }

    // Boss bar
    if (bossBar && boss.active) {
        float frac = std::max(0.0f, (float)boss.hp / boss.maxHp);
        float w = 340, x = W / 2 - w / 2, y = 64;
        r.drawRect(x, y, w, 14.0f, rgb(0x000000, 153));
        r.drawRect(x, y, w * frac, 14.0f, rgb(frac > 0.5f ? 0xff9900 : 0xff2222));
        r.drawRect(x - 2, y - 2, w + 4, 2.0f, rgb(0xffffff));
        r.drawRect(x - 2, y + 14, w + 4, 2.0f, rgb(0xffffff));
        r.drawRect(x - 2, y, 2.0f, 14.0f, rgb(0xffffff));
        r.drawRect(x + w, y, 2.0f, 14.0f, rgb(0xffffff));
        r.drawRect(x + w / 2 - 1, y - 3, 2.0f, 20.0f, rgb(0xffffff, 204)); // phase-2 threshold
        UI::drawText(r, level->bossName, W / 2, y - 20, 2, rgb(0xffaaaa), Align::CENTER);
    }

    // Boss warning: red pulsing overlay + flashing text
    if (bossWarningT >= 0) {
        float k = std::fmod(bossWarningT, 500.0f) / 250.0f;
        float tri = k < 1 ? k : 2 - k;
        r.drawRect(0.0f, 0.0f, W, (float)SCREEN_HEIGHT, rgb(0xff0000, (uint8_t)(64 * tri)));
        std::string msg = "WARNING\n" + level->bossName + "\nAPPROACHING";
        UI::drawText(r, msg, W / 2, SCREEN_HEIGHT * 0.4f - 40, 3, rgb(0xff3333, (uint8_t)(255 * (1 - 0.8f * tri))), Align::CENTER);
    }

    // Announcements: pop in, hold, pop out
    for (const auto& a : announcements) {
        float k;
        if (a.t < 300) k = a.t / 300;
        else if (a.t < 1200) k = 1;
        else k = 1 - (a.t - 1200) / 300;
        k = std::clamp(k, 0.0f, 1.0f);
        float back = k < 1 ? 1 + 0.15f * std::sin(k * PI) : 1; // Back.Out overshoot
        float s = a.scale * (0.6f + 0.4f * k) * back;
        auto lines = UI::wrap(a.text, s, W - 32);
        std::string joined;
        for (size_t i = 0; i < lines.size(); ++i) joined += (i ? "\n" : "") + lines[i];
        float h = lines.size() * UI::lineHeight(s);
        UI::drawText(r, joined, W / 2, SCREEN_HEIGHT * 0.4f - h / 2, s, rgb(a.color, (uint8_t)(255 * k)), Align::CENTER);
    }
}

} // namespace Game
