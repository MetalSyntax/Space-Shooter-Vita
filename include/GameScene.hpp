#pragma once

#include <array>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "Assets.hpp"
#include "InputManager.hpp"
#include "LevelManager.hpp"
#include "ParticlePool.hpp"
#include "ScoreManager.hpp"

/**
 * One sector of the campaign: player, enemy roster, waves, boss, power-ups, FX and HUD.
 * Port of MainGame.ts + Player.ts + Enemy.ts + Boss.ts + WaveManager.ts + HUD.ts.
 * Time inside the scene is in milliseconds, like the Phaser original.
 */
namespace Game {

extern const float POWER_DURATIONS[POWER_COUNT];
constexpr int MAX_BOMBS = 5;

struct SectorStats {
    int kills = 0;
    float accuracy = 0.0f;
    int bonus = 0;
};

enum class Outcome { NONE, PAUSE, LEVEL_CLEAR, GAME_OVER, VICTORY };

struct Bullet {
    bool active = false;
    float x = 0, y = 0, vx = 0, vy = 0;
    int damage = 1;
    bool homing = false;
    bool fake = false; // decoy shots never hurt
    float scale = 1.0f;
    Tex* tex = nullptr;
};

enum class EType { DRONE, WEAVER, SWOOPER, BLINKER, KAMIKAZE, POD, GUNSHIP, SPREADER, LASER, LANCER, FIGHTER, SPLITTER, ASTEROID, MINE, PART };
enum class Follow { NONE, ORBIT, WING, FIXED, BARRIER };

struct Enemy {
    bool active = false;
    EType type = EType::DRONE;
    Tex* tex = nullptr;
    float x = 0, y = 0, vx = 0, vy = 0;
    float rot = 0;      // radians, 0 = sprite as drawn (nose down)
    float angVel = 0;   // deg/s
    float sx = 1, sy = 1;
    float alpha = 1;
    uint32_t tint = 0xffffff;
    float flash = 0;    // ms of white hit flash
    int hp = 1;
    int points = 10;
    bool heavy = false;
    bool solid = false;
    bool invulnerable = false;
    bool bossPart = false;
    float explosionSize = 1;

    float cooldown = 0;
    float fireEvery = 0;
    float timer = 0;
    float beam = 0;     // laser ship telegraph/beam progress (ms)
    int phase = 0;
    int shield = 0;     // pod bubble hits
    float bubbleFlash = 0;
    float stopY = 0;
    float heading = 0, turn = 0, speed = 0;
    bool big = false;
    float blinkT = -1;  // blinker teleport animation (ms), -1 = idle

    // Boss parts
    Follow follow = Follow::NONE;
    float fRadius = 0, fSpeed = 0, fOffset = 0, fx = 0, fy = 0;
    int fSide = 0;
    bool armed = false;
    bool fake = false;

    float radius() const;
};

struct Powerup {
    bool active = false;
    float x = 0, y = 0, vx = 0, vy = 0;
    Power kind = Power::SHIELD;
};

struct FloatText {
    bool active = false;
    float x = 0, y = 0;
    std::string text;
    uint32_t color = 0xffffff;
    float scale = 2;
    float t = 0;
};

struct Announcement {
    std::string text;
    uint32_t color = 0x00ffff;
    float scale = 3;
    float t = 0;
};

struct DecorProp {
    bool active = false;
    float x = 0, y = 0, vy = 0, alpha = 1, scale = 1, angle = 0, angVel = 0;
};

enum class BossState { ENTERING, IDLE, ATTACK_SPREAD, ATTACK_LASER, DASH, SPECIAL, DYING };

struct Beam {
    bool active = false;
    float from = 0, to = 0; // radians
    float telegraph = 0, fire = 0, width = 0, t = 0;
    float angle = 0, curWidth = 0;
};

struct Boss {
    bool active = false;
    BossState state = BossState::ENTERING;
    float stateTime = 0, stateDuration = 0;
    int hp = 1, maxHp = 1;
    int phase = 1;
    float x = 0, y = -160, homeY = 140;
    float dashX = 0, dashY = 0;
    float alpha = 1;
    float flash = 0;
    uint32_t tint = 0xffffff;
    float speedBoost = 1;
    float enterT = 0;
    float dieT = 0;
    int dieCount = 0;
    bool visible = true;
    float fadeT = -1; // phantom teleport (ms), -1 = idle
    Beam beam;
    std::map<int, float> timers;
    int counter = 0;   // dashes / sweep side
    float spin = 0;    // spiral angle
    std::vector<int> parts;
};

class GameScene {
public:
    void start(const CampaignState& c);
    Outcome update(float dt, const Input::InputState& in);
    void render(Graphics::Renderer& r);

    const CampaignState& campaignAtStart() const { return campaign; }
    // Filled when the sector ends
    CampaignState resultCampaign;
    SectorStats stats;
    int finalScore = 0;
    bool newRecord = false;
    Difficulty difficulty() const { return campaign.difficulty; }

private:
    struct Scheduled {
        float t;
        std::function<void()> fn;
    };

    // --- setup / loop ---
    void after(float ms, std::function<void()> fn);
    void updatePlayer(float dms, const Input::InputState& in);
    void updateBullets(float dms);
    void updateEnemies(float dms);
    void updateEnemy(Enemy& e, float dms);
    void updatePowerups(float dms);
    void updateFx(float dms);
    void updateBackground(float dms);
    void collisions();

    // --- player ---
    bool hasPower(Power p) const { return powers[(int)p] > 0; }
    void activatePower(Power p, float durationMs);
    void firePlayer();
    void spawnPlayerBullet(float x, float y, float angleDeg, Tex* tex, float speed, int damage, bool homing);
    void useBomb();
    void hyperBomb();
    void damagePlayer();

    // --- enemies ---
    Enemy* spawnEnemy(EType type, float x, float y);
    Enemy* spawnKind(EnemyKind kind, float x, float y);
    void enemyShoot(Enemy& e, float angle, Ability color, float speed, bool sound = true);
    float aimFrom(float x, float y) const;
    bool damageEnemy(Enemy& e, int amount);
    void killEnemy(Enemy& e);
    void onKilled(Enemy& e);
    int countWaveEnemies() const;
    bool nearestTarget(float x, float y, float& tx, float& ty) const;

    // --- bullets ---
    Bullet* fireEnemyBullet(float x, float y, float angle, float speed = 250, bool sound = true, bool fake = false, Tex* tex = nullptr);
    void fireEnemyBulletAt(float x, float y, float speed, bool fake);

    // --- power-ups ---
    void spawnPowerup(float x, float y);
    void spawnPowerupRandom();
    void collectPowerup(Powerup& p);

    // --- waves ---
    void nextWave();
    void buildWave(WaveKind kind);
    void queueSpawn(float delay, std::function<void()> fn);
    void classic(int count, float offset);
    void stream(int count, std::function<void()> make, float offset);
    void vFormation(bool fromLeft, float offset);
    void pincer(int pairs);
    EnemyKind pickKind(const std::function<bool(EnemyKind)>& filter);
    float randX() const;

    // --- boss ---
    void startBoss();
    void updateBoss(float dms);
    void bossGo(BossState s, float duration = 0);
    bool bossEvery(int key, float ms, float dms);
    void bossNext();
    void bossTick(float dms);
    void bossOnPhase(int phase);
    void bossSetup();
    bool bossInvulnerable() const;
    bool bossFrenzy() const { return boss.phase > 1; }
    void damageBoss(int amount);
    void bossDie();
    float bossDisplayW() const;
    float bossDisplayH() const;
    void bossAimed(int n, float spreadDeg, float speed = 240, float ox = 0, float oy = 0);
    void bossFan(std::initializer_list<float> anglesDeg, float speed = 240, float ox = 0, float oy = 0);
    void bossRing(int n, float speed = 180, float offsetDeg = 0);
    void bossLaser(float fromDeg, float toDeg, float telegraphMs, float fireMs, float width);
    Enemy* bossAddPart(Tex* tex, int hp, int points, float fireEvery, bool invulnerable);
    int aliveParts();
    void levelComplete();
    void gameOver();

    // --- fx / hud ---
    void explode(float x, float y, float size);
    void sparkBurst(float x, float y, int count = 5);
    void shieldRipple(float x, float y, float radius);
    void floatingText(float x, float y, const std::string& text, uint32_t color, float scale);
    void announce(const std::string& text, uint32_t color = 0x00ffff, float scale = 3);
    void shake(float amount) { trauma = std::min(1.0f, trauma + amount); }
    void flashScreen(float ms, uint32_t color);
    void hitStop(float ms = 35) { if (hitStopT <= 0) hitStopT = ms; }
    void renderHud(Graphics::Renderer& r);
    void renderEnemy(Graphics::Renderer& r, const Enemy& e);

    CampaignState campaign;
    const LevelDef* level = nullptr;
    const ShipDef* ship = nullptr;
    LevelTextures* lt = nullptr;
    float speedMult = 1;
    float spawnDelay = 700;
    ScoreManager score;
    Outcome pending = Outcome::NONE;
    bool ended = false;
    int kills = 0;
    int hits = 0;
    float time = 0; // ms since the sector started

    // Player
    float px = 480, py = 450, pvx = 0, pvy = 0;
    float aim = -90; // degrees
    int lives = 5;
    int bombs = 3;
    float powers[POWER_COUNT] = {};
    float invuln = 0;
    float fireCooldown = 0;
    float nextMissile = 0;
    float nextDroneShot = 0;
    float droneOrbit = 0;
    float hitTint = 0;
    bool shieldWarned = false;
    bool playerAlive = true;
    int shotsFired = 0;
    float playerSpeedMult = 1;
    float fireDelay = 200;

    std::vector<Bullet> bullets;
    std::vector<Bullet> enemyBullets;
    std::vector<Enemy> enemies;
    std::vector<Powerup> powerups;
    std::vector<FloatText> texts;
    std::vector<Announcement> announcements;
    std::vector<DecorProp> decor;
    std::vector<Scheduled> scheduled;
    Graphics::ParticlePool<1024> particles;
    Boss boss;

    // Waves
    int wave = 0;
    int waveSerial = 0; // invalidates queued spawns of a finished wave phase
    int pendingSpawns = 0;
    float waveElapsed = 0;
    bool waveActive = false;
    bool wavesDone = false;
    int lastWaveKind = -1;

    // FX state
    float trauma = 0;
    float hitStopT = 0;
    float flashT = 0, flashDur = 1;
    uint32_t flashColor = 0xffffff;
    float bgScroll = 0;
    float decorTimer = 2000;
    float lightningTimer = 5000;
    float bossWarningT = -1; // ms into the boss warning, -1 = none
    bool bossBar = false;
    float scorePop = 0;
    int lastScore = 0;
    float bombWaveT = -1;
    float bombX = 0, bombY = 0;
    struct Ripple {
        float x, y, r, t;
    };
    std::vector<Ripple> ripples;
};

} // namespace Game
