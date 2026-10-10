#include "GameScene.hpp"
#include "MathUtil.hpp"
#include "I18n.hpp"
#include "audio/Synth.hpp"

/**
 * Finite state machine shared by the 10 sector bosses (Boss.ts):
 * ENTERING → IDLE ⇄ (ATTACK_SPREAD | ATTACK_LASER | DASH | SPECIAL) → DYING.
 */
namespace Game {

using BossGenerator::BossKind;
using Graphics::SCREEN_WIDTH;
using Graphics::SCREEN_HEIGHT;

static constexpr float WARNING_MS = 3000;
static Audio::Synth& synth() { return Audio::Synth::instance(); }

struct BehaviorDef {
    float idleMs;
    float moveSpeed;
};

static BehaviorDef behaviorDef(BossKind k) {
    switch (k) {
        case BossKind::GOLIATH: return {1200, 70};
        case BossKind::VIPER: return {700, 120};
        case BossKind::AEGIS: return {1100, 60};
        case BossKind::HYDRA: return {1000, 80};
        case BossKind::MINER: return {1300, 60};
        case BossKind::PHANTOM: return {1200, 90};
        case BossKind::CARRIER: return {1500, 50};
        case BossKind::BEHEMOTH: return {1000, 70};
        case BossKind::TITAN: return {900, 40};
        case BossKind::LEVIATHAN: return {900, 60};
    }
    return {1000, 60};
}

float GameScene::bossDisplayW() const { return Graphics::Renderer::texWidth(lt->boss) * 1.5f; }
float GameScene::bossDisplayH() const { return Graphics::Renderer::texHeight(lt->boss) * 1.5f; }

bool GameScene::bossInvulnerable() const {
    return boss.state == BossState::ENTERING || boss.state == BossState::DYING || boss.alpha < 0.5f;
}

void GameScene::startBoss() {
    const DifficultyDef& diff = difficultyDef(campaign.difficulty);
    boss = Boss{};
    boss.active = true;
    boss.maxHp = boss.hp = (int)std::lround((60 + level->id * 15) * diff.bossHp);
    boss.x = SCREEN_WIDTH / 2.0f;
    boss.y = -160;
    boss.homeY = std::min(160.0f, SCREEN_HEIGHT * 0.22f);
    bossSetup();
    synth().playBossSiren();
    synth().setMusicIntensity(true);
    bossWarningT = 0;
    bossBar = false;
}

void GameScene::bossGo(BossState s, float duration) {
    boss.state = s;
    boss.stateTime = 0;
    boss.stateDuration = duration;
}

/** Cooldown helper: true every `ms` while called each frame. */
bool GameScene::bossEvery(int key, float ms, float dms) {
    float& t = boss.timers[key];
    t -= dms;
    if (t > 0) return false;
    t = ms;
    return true;
}

// --- Shared attacks ---

void GameScene::bossAimed(int n, float spreadDeg, float speed, float ox, float oy) {
    float mx = boss.x + ox, my = boss.y + bossDisplayH() * 0.35f + oy;
    float base = std::atan2(py - my, px - mx);
    for (int i = 0; i < n; ++i) fireEnemyBullet(mx, my, base + (i - (n - 1) / 2.0f) * spreadDeg * DEG, speed, i == 0);
}

void GameScene::bossFan(std::initializer_list<float> anglesDeg, float speed, float ox, float oy) {
    float mx = boss.x + ox, my = boss.y + bossDisplayH() * 0.35f + oy;
    int i = 0;
    for (float a : anglesDeg) fireEnemyBullet(mx, my, a * DEG, speed, i++ == 0);
}

void GameScene::bossRing(int n, float speed, float offsetDeg) {
    for (int i = 0; i < n; ++i) fireEnemyBullet(boss.x, boss.y, (360.0f / n * i + offsetDeg) * DEG, speed, i == 0);
}

void GameScene::bossLaser(float fromDeg, float toDeg, float telegraphMs, float fireMs, float width) {
    boss.beam = Beam{};
    boss.beam.active = true;
    boss.beam.from = boss.beam.angle = fromDeg * DEG;
    boss.beam.to = toDeg * DEG;
    boss.beam.telegraph = telegraphMs;
    boss.beam.fire = fireMs;
    boss.beam.width = boss.beam.curWidth = width;
    synth().playLaserCharge(telegraphMs / 1000);
    bossGo(BossState::ATTACK_LASER);
}

Enemy* GameScene::bossAddPart(Tex* tex, int hp, int points, float fireEvery, bool invulnerable) {
    Enemy* p = spawnEnemy(EType::PART, -200, -200);
    if (!p) return nullptr;
    p->tex = tex;
    p->hp = hp;
    p->points = points;
    p->fireEvery = fireEvery;
    p->invulnerable = invulnerable;
    p->armed = boss.state != BossState::ENTERING && fireEvery > 0;
    boss.parts.push_back((int)(p - enemies.data()));
    return p;
}

int GameScene::aliveParts() {
    auto& v = boss.parts;
    v.erase(std::remove_if(v.begin(), v.end(), [this](int i) { return !enemies[i].active || !enemies[i].bossPart; }), v.end());
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    return (int)v.size();
}

static void destroyParts(std::vector<Enemy>& enemies, std::vector<int>& parts) {
    for (int i : parts) {
        if (enemies[i].bossPart) enemies[i].active = false;
    }
    parts.clear();
}

// --- The 10 archetypes ---

void GameScene::bossSetup() {
    switch (level->boss) {
        case BossKind::GOLIATH:
            for (int i = 0; i < 4; ++i) {
                Enemy* p = bossAddPart(assets().asteroidSmall, 3, 20, 0, false);
                if (p) { p->follow = Follow::ORBIT; p->fRadius = 105; p->fSpeed = 1.4f; p->fOffset = PI / 2 * i; }
            }
            break;
        case BossKind::AEGIS:
            bossOnPhase(1);
            break;
        case BossKind::HYDRA:
            for (int side : {-1, 1}) {
                Enemy* p = bossAddPart(lt->turret, 15, 200, 1300, false);
                if (p) { p->follow = Follow::WING; p->fSide = side; }
            }
            break;
        case BossKind::LEVIATHAN:
            for (int i = 0; i < 3; ++i) {
                Enemy* p = bossAddPart(lt->shield, 8, 150, 0, true);
                if (p) { p->follow = Follow::BARRIER; p->fOffset = (float)i; p->sx = 1.6f; p->sy = 0.7f; }
            }
            break;
        default:
            break;
    }
}

void GameScene::bossOnPhase(int phase) {
    switch (level->boss) {
        case BossKind::AEGIS: {
            // Rotating shield orbs block frontal shots; more and faster in frenzy.
            aliveParts();
            destroyParts(enemies, boss.parts);
            int n = phase > 1 ? 3 : 2;
            float speed = phase > 1 ? 1.9f : 1.3f;
            for (int i = 0; i < n; ++i) {
                Enemy* p = bossAddPart(lt->shield, 8, 150, 0, true);
                if (p) { p->follow = Follow::ORBIT; p->fRadius = 88; p->fSpeed = speed; p->fOffset = 2 * PI * i / n; }
            }
            break;
        }
        case BossKind::LEVIATHAN:
            if (phase == 2) { aliveParts(); destroyParts(enemies, boss.parts); }
            if (phase == 3) boss.speedBoost = 2;
            break;
        default:
            break;
    }
}

void GameScene::bossNext() {
    const float W = SCREEN_WIDTH, H = SCREEN_HEIGHT;
    switch (level->boss) {
        case BossKind::GOLIATH:
            if (aliveParts() < 2 && frand(0, 1) < 0.5f) bossGo(BossState::SPECIAL, 600);
            else bossGo(BossState::ATTACK_SPREAD, 2400);
            break;
        case BossKind::VIPER:
            if (boss.counter++ < 3) {
                boss.dashX = boss.x < W / 2 ? W - 80 : 80;
                boss.dashY = (float)irand(80, (int)std::min(260.0f, H * 0.4f));
                bossGo(BossState::DASH);
            } else {
                boss.counter = 0;
                bossGo(BossState::ATTACK_SPREAD, 1600);
            }
            break;
        case BossKind::AEGIS: {
            int r = std::rand() % 3;
            bossGo(r < 2 ? BossState::ATTACK_SPREAD : BossState::SPECIAL, 1800);
            break;
        }
        case BossKind::HYDRA:
            if (aliveParts() < 2) {
                if (frand(0, 1) < 0.6f) {
                    float a = std::atan2(py - boss.y, px - boss.x) / DEG;
                    bossLaser(a, a, 900, 1200, 26);
                } else bossGo(BossState::ATTACK_SPREAD, 1200);
            } else bossGo(BossState::ATTACK_SPREAD, 1800);
            break;
        case BossKind::MINER:
            bossGo(frand(0, 1) < 0.6f ? BossState::SPECIAL : BossState::ATTACK_SPREAD, 1500);
            break;
        case BossKind::PHANTOM:
            bossGo(frand(0, 1) < 0.5f ? BossState::SPECIAL : BossState::ATTACK_SPREAD, 1600);
            break;
        case BossKind::CARRIER:
            bossGo(frand(0, 1) < 0.75f ? BossState::SPECIAL : BossState::ATTACK_SPREAD, 1800);
            break;
        case BossKind::BEHEMOTH:
            if (frand(0, 1) < 0.55f) {
                float span = bossFrenzy() ? 60.0f : 35.0f;
                bool left = boss.counter == 0;
                bossLaser(left ? 90 - span : 90 + span, left ? 90 + span : 90 - span, 1500, 1800, 48);
                boss.counter ^= 1;
            } else bossGo(BossState::ATTACK_SPREAD, 1600);
            break;
        case BossKind::TITAN:
            bossGo(frand(0, 1) < 0.6f ? BossState::SPECIAL : BossState::ATTACK_SPREAD, bossFrenzy() ? 4500.0f : 3500.0f);
            break;
        case BossKind::LEVIATHAN:
            bossGo(boss.phase == 1 ? BossState::ATTACK_SPREAD : BossState::SPECIAL, boss.phase == 3 ? 5000.0f : 2500.0f);
            break;
    }
}

void GameScene::bossTick(float dms) {
    const BossState s = boss.state;
    const bool first = boss.stateTime <= dms + 0.01f;
    const bool fr = bossFrenzy();

    switch (level->boss) {
        case BossKind::GOLIATH:
            // Heavy central cannon + shrapnel V; keeps mini-asteroids orbiting as shields.
            if (s == BossState::SPECIAL && first) {
                for (int i = aliveParts(); i < 4; ++i) {
                    Enemy* p = bossAddPart(assets().asteroidSmall, 3, 20, 0, false);
                    if (p) { p->follow = Follow::ORBIT; p->fRadius = 105; p->fSpeed = 1.4f; p->fOffset = PI / 2 * i; }
                }
            }
            if (s != BossState::ATTACK_SPREAD) break;
            if (bossEvery(0, fr ? 700 : 1000, dms)) {
                Bullet* b = fireEnemyBullet(boss.x, boss.y + bossDisplayH() * 0.35f, 90 * DEG, 160);
                if (b) b->scale = 2.2f;
            }
            if (bossEvery(1, fr ? 450 : 650, dms)) {
                bossFan({70, 110}, 230, -40);
                bossFan({55, 125}, 230, 40);
            }
            break;

        case BossKind::VIPER:
            // Zig-zag dashes, plasma spikes on the diagonals.
            if (s == BossState::DASH && bossEvery(0, fr ? 110 : 160, dms)) bossFan({45, 135}, 260);
            if (s == BossState::ATTACK_SPREAD && bossEvery(1, 400, dms)) {
                if (fr) bossFan({30, 50, 70, 90, 110, 130, 150}, 230);
                else bossFan({40, 65, 90, 115, 140}, 230);
            }
            break;

        case BossKind::AEGIS:
            if (s == BossState::ATTACK_SPREAD && bossEvery(0, 500, dms)) bossAimed(5, 12, 240);
            if (s == BossState::SPECIAL && bossEvery(1, 600, dms)) bossRing(fr ? 18 : 12, 170, boss.stateTime / 20);
            break;

        case BossKind::HYDRA:
            // Two wing turrets; losing one switches the core to concentrated fire.
            if (s != BossState::ATTACK_SPREAD) break;
            if (aliveParts() < 2) {
                if (bossEvery(0, 90, dms)) bossAimed(1, 0, 330);
            } else if (bossEvery(1, 800, dms)) bossAimed(3, 15, 230);
            break;

        case BossKind::MINER:
            // Lays proximity mines that burst into rings.
            if (s == BossState::SPECIAL && bossEvery(0, 350, dms) && boss.stateTime < (fr ? 1500 : 1000)) {
                Enemy* m = spawnEnemy(EType::MINE, boss.x + irand(-40, 40), boss.y + 50);
                if (m) { m->vx = (float)irand(-60, 60); m->vy = 80; }
            }
            if (s == BossState::ATTACK_SPREAD && bossEvery(1, 500, dms)) bossAimed(3, 20, 220);
            break;

        case BossKind::PHANTOM:
            // Teleports and leaves holographic decoys that fire harmless bursts.
            if (s == BossState::ATTACK_SPREAD && bossEvery(0, 420, dms)) bossAimed(3, 10, 260);
            if (s == BossState::SPECIAL && first && boss.fadeT < 0) boss.fadeT = 0;
            break;

        case BossKind::CARRIER:
            // Mothership launching kamikaze swarms; rarely fires itself.
            if (s == BossState::SPECIAL && bossEvery(0, fr ? 220 : 320, dms)) {
                int side = frand(0, 1) < 0.5f ? -1 : 1;
                spawnEnemy(EType::KAMIKAZE, boss.x + side * bossDisplayW() * 0.4f, boss.y);
            }
            if (s == BossState::ATTACK_SPREAD && bossEvery(1, 900, dms)) bossRing(10, 140);
            break;

        case BossKind::BEHEMOTH:
            if (s == BossState::ATTACK_SPREAD && bossEvery(0, 300, dms)) {
                int side = ((int)(boss.stateTime / 300) % 2) ? 1 : -1;
                bossAimed(1, 0, 300, side * 36.0f);
            }
            break;

        case BossKind::TITAN: {
            // Gravity vortex dragging the player in while firing dense spirals.
            if (s == BossState::ATTACK_SPREAD && bossEvery(0, 700, dms)) bossRing(16, 160, boss.spin += 11);
            if (s != BossState::SPECIAL) break;
            float a = std::atan2(boss.y - py, boss.x - px);
            // Phaser pulls at 170/260 px/s² against weak damping; scaled to this build's stronger drag.
            float pull = (fr ? 260.0f : 170.0f) * 3.75f * dms / 1000.0f;
            if (playerAlive) {
                pvx += std::cos(a) * pull;
                pvy += std::sin(a) * pull;
            }
            if (bossEvery(1, 90, dms)) {
                boss.spin += 13;
                int arms = fr ? 4 : 3;
                for (int i = 0; i < arms; ++i) fireEnemyBullet(boss.x, boss.y, (boss.spin + 360.0f / arms * i) * DEG, 170, i == 0);
            }
            break;
        }

        case BossKind::LEVIATHAN:
            // Fortress (barriers + triple cannons) → Spiral (50%) → Self-destruct frenzy (20%).
            if (boss.phase == 1 && s == BossState::ATTACK_SPREAD && bossEvery(0, 700, dms)) {
                for (float ox : {-50.0f, 0.0f, 50.0f}) bossAimed(1, 0, 250, ox);
            }
            if (boss.phase == 2 && s == BossState::SPECIAL) {
                if (bossEvery(1, 110, dms)) {
                    boss.spin += 9;
                    for (int i = 0; i < 4; ++i) fireEnemyBullet(boss.x, boss.y, (boss.spin + 90.0f * i) * DEG, 170, i == 0);
                }
                if (bossEvery(2, 1400, dms)) bossRing(20, 130, boss.spin);
            }
            if (boss.phase == 3) {
                if (bossEvery(3, 380, dms)) bossRing(20, 210, boss.spin += 9);
                if (bossEvery(4, 600, dms)) bossAimed(3, 12, 300);
            }
            break;
    }
}

// --- Loop ---

void GameScene::updateBoss(float dms) {
    if (!boss.active) return;
    const float dt = dms / 1000.0f;
    const float W = SCREEN_WIDTH, H = SCREEN_HEIGHT;
    BehaviorDef def = behaviorDef(level->boss);

    if (boss.state == BossState::ENTERING) {
        boss.enterT += dms;
        if (boss.enterT > WARNING_MS) {
            float k = std::min(1.0f, (boss.enterT - WARNING_MS) / 2500.0f);
            float ease = 1 - (1 - k) * (1 - k) * (1 - k);
            boss.y = -160 + (boss.homeY + 160) * ease;
            if (k >= 1) {
                aliveParts();
                for (int i : boss.parts) enemies[i].armed = enemies[i].fireEvery > 0;
                bossGo(BossState::IDLE, def.idleMs);
            }
        }
        return;
    }

    if (boss.state == BossState::DYING) {
        boss.dieT += dms;
        while (boss.dieCount < 15 && boss.dieT >= 140.0f * (boss.dieCount + 1)) {
            boss.dieCount++;
            float ox = (float)irand((int)(-bossDisplayW() / 2), (int)(bossDisplayW() / 2));
            float oy = (float)irand((int)(-bossDisplayH() / 2), (int)(bossDisplayH() / 2));
            explode(boss.x + ox, boss.y + oy, 1.2f);
            shake(0.25f);
            synth().playExplosion(boss.dieCount % 3 == 0);
            if (boss.dieCount == 15) {
                explode(boss.x, boss.y, 4);
                flashScreen(600, 0xffffff);
                synth().playExplosion(true);
                boss.visible = false;
            }
        }
        if (boss.dieCount >= 15 && boss.dieT >= 140.0f * 15 + 1200) {
            boss.active = false;
            levelComplete();
        }
        return;
    }

    // Phantom teleport: fade out, jump + spawn decoys, fade back in.
    if (boss.fadeT >= 0) {
        float before = boss.fadeT;
        boss.fadeT += dms;
        if (before < 300 && boss.fadeT >= 300) {
            boss.x = (float)irand(90, (int)W - 90);
            boss.y = (float)irand(90, (int)std::min(220.0f, H * 0.35f));
            aliveParts();
            destroyParts(enemies, boss.parts);
            for (int i = 0; i < (bossFrenzy() ? 3 : 2); ++i) {
                Enemy* d = bossAddPart(lt->boss, 6, 50, 700, false);
                if (!d) continue;
                d->follow = Follow::FIXED;
                d->fx = (float)irand(80, (int)W - 80);
                d->fy = (float)irand(80, (int)std::min(240.0f, H * 0.4f));
                d->x = d->fx;
                d->y = d->fy;
                d->fake = true;
                d->alpha = 0.55f;
                d->sx = d->sy = 0.8f * 1.5f;
            }
        }
        boss.alpha = boss.fadeT < 300 ? 1 - boss.fadeT / 300 : std::min(1.0f, (boss.fadeT - 300) / 300);
        if (boss.fadeT >= 600) {
            boss.fadeT = -1;
            boss.alpha = 1;
        }
    }

    boss.stateTime += dms;
    float boost = boss.speedBoost * (bossFrenzy() ? 1.5f : 1.0f);

    if (boss.state == BossState::IDLE) {
        float tx = std::clamp(px, 90.0f, W - 90);
        float s = def.moveSpeed * boost * dt;
        boss.x += std::clamp(tx - boss.x, -s, s);
        boss.y += (boss.homeY + std::sin(time / 600) * 8 - boss.y) * std::min(1.0f, dt * 3);
        if (boss.stateTime >= boss.stateDuration / boost) bossNext();
    } else if (boss.state == BossState::DASH) {
        float s = 650 * boost * dt;
        float dx = boss.dashX - boss.x, dy = boss.dashY - boss.y;
        float d = std::hypot(dx, dy);
        if (d <= s) {
            boss.x = boss.dashX;
            boss.y = boss.dashY;
            bossGo(BossState::IDLE, def.idleMs * 0.4f);
        } else {
            boss.x += dx / d * s;
            boss.y += dy / d * s;
        }
    }

    bossTick(dms);

    if (boss.state == BossState::ATTACK_LASER && boss.beam.active) {
        Beam& bm = boss.beam;
        bool charging = bm.t < bm.telegraph;
        float k = charging ? 0 : std::min(1.0f, (bm.t - bm.telegraph) / bm.fire);
        bm.angle = bm.from + (bm.to - bm.from) * k;
        if (!charging) {
            if (bm.t - dms < bm.telegraph) {
                synth().playLaserBeam(bm.fire / 1000);
                shake(0.4f);
            }
            bm.curWidth = bm.width + frand(0, 8);
            float ox = boss.x, oy = boss.y + bossDisplayH() * 0.35f;
            // Distance from the player to the beam ray.
            float t = std::max(0.0f, (px - ox) * std::cos(bm.angle) + (py - oy) * std::sin(bm.angle));
            float d = std::hypot(px - (ox + std::cos(bm.angle) * t), py - (oy + std::sin(bm.angle) * t));
            if (d < bm.curWidth / 2 + 10) damagePlayer();
        }
        bm.t += dms;
        if (bm.t >= bm.telegraph + bm.fire) {
            bm.active = false;
            bossGo(BossState::IDLE, def.idleMs);
        }
    }
    if ((boss.state == BossState::ATTACK_SPREAD || boss.state == BossState::SPECIAL) && boss.stateDuration > 0 && boss.stateTime >= boss.stateDuration) {
        bossGo(BossState::IDLE, def.idleMs);
    }
}

// --- Damage / phases ---

void GameScene::damageBoss(int amount) {
    if (!boss.active || bossInvulnerable()) return;
    boss.hp = std::max(0, boss.hp - amount);
    boss.flash = 60;
    hitStop();
    shake(0.1f);

    float frac = (float)boss.hp / boss.maxHp;
    int phase = boss.hp <= 0 ? boss.phase : (frac <= 0.2f && level->boss == BossKind::LEVIATHAN) ? 3 : frac <= 0.5f ? 2 : 1;
    if (phase > boss.phase) {
        boss.phase = phase;
        boss.tint = phase == 3 ? 0xff4444 : 0xff9999;
        shake(0.5f);
        floatingText(boss.x, boss.y + 60, phase == 3 ? I18n::L("¡AUTODESTRUCCION!", "SELF-DESTRUCT!") : I18n::L("¡MODO FRENESI!", "FRENZY MODE!"), 0xff4444, 3);
        flashScreen(300, 0xff0000);
        bossOnPhase(phase);
    }
    if (boss.hp <= 0) {
        bossDie();
        int points = score.addPoints(1000 * level->id);
        floatingText(boss.x, boss.y, level->bossName + " DESTROYED +" + std::to_string(points), 0xffdd33, 3);
    }
}

void GameScene::bossDie() {
    bossGo(BossState::DYING);
    boss.beam.active = false;
    boss.fadeT = -1;
    boss.alpha = 1;
    aliveParts();
    destroyParts(enemies, boss.parts);
    boss.dieT = 0;
    boss.dieCount = 0;
}

} // namespace Game
