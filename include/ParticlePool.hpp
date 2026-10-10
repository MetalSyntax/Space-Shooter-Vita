#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include "Renderer.hpp"

namespace Graphics {

enum class ParticleType {
    SPARK,
    DEBRIS,
    SMOKE,
    THRUSTER,
    RING
};

struct Particle {
    bool active = false;
    ParticleType type = ParticleType::SPARK;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 2.0f;
    float maxSize = 2.0f;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};

template<size_t MAX_PARTICLES = 1024>
class ParticlePool {
public:
    ParticlePool() {
        particles.resize(MAX_PARTICLES);
    }

    void spawn(ParticleType type, float x, float y, float vx, float vy, float life, float size, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        for (auto& p : particles) {
            if (!p.active) {
                p.active = true;
                p.type = type;
                p.x = x;
                p.y = y;
                p.vx = vx;
                p.vy = vy;
                p.life = p.maxLife = life;
                p.size = p.maxSize = size;
                p.r = r; p.g = g; p.b = b; p.a = a;
                return;
            }
        }
    }

    void explode(float x, float y, float scale = 1.0f) {
        int count = static_cast<int>(18 * scale);
        for (int i = 0; i < count; ++i) {
            float angle = (std::rand() % 360) * (3.1415926535f / 180.0f);
            float speed = 80.0f + (std::rand() % 220);
            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed;
            float life = 0.25f + ((std::rand() % 25) / 100.0f);

            if (i % 3 == 0) {
                spawn(ParticleType::SPARK, x, y, vx, vy, life, 3.0f * scale, 255, 255, 200, 255);
            } else if (i % 2 == 0) {
                spawn(ParticleType::DEBRIS, x, y, vx * 0.7f, vy * 0.7f, life * 1.3f, 4.0f * scale, 255, 120, 30, 255);
            } else {
                spawn(ParticleType::SMOKE, x, y, vx * 0.4f, vy * 0.4f, life * 2.0f, 6.0f * scale, 120, 120, 140, 200);
            }
        }
        spawn(ParticleType::RING, x, y, 0.0f, 0.0f, 0.25f, 10.0f * scale, 255, 255, 255, 255);
    }

    void spawnThruster(float x, float y, float angleDeg, float shipSpeed) {
        float backRad = (angleDeg + 180.0f + (std::rand() % 24 - 12)) * (3.1415926535f / 180.0f);
        float speed = 70.0f + shipSpeed * 0.5f;
        float vx = std::cos(backRad) * speed;
        float vy = std::sin(backRad) * speed;

        uint8_t r = 60 + std::rand() % 50;
        uint8_t g = 180 + std::rand() % 75;
        uint8_t b = 255;
        spawn(ParticleType::THRUSTER, x, y, vx, vy, 0.22f, 3.5f, r, g, b, 230);
    }

    void update(float dt) {
        for (auto& p : particles) {
            if (!p.active) continue;

            p.life -= dt;
            if (p.life <= 0.0f) {
                p.active = false;
                continue;
            }

            p.x += p.vx * dt;
            p.y += p.vy * dt;

            float progress = 1.0f - (p.life / p.maxLife);

            if (p.type == ParticleType::RING) {
                p.size = p.maxSize + progress * 60.0f;
                p.a = static_cast<uint8_t>(255 * (1.0f - progress));
            } else if (p.type == ParticleType::SMOKE) {
                p.size = p.maxSize * (1.0f + progress * 1.5f);
                p.a = static_cast<uint8_t>(180 * (1.0f - progress));
            } else {
                p.size = p.maxSize * (1.0f - progress * 0.7f);
                p.a = static_cast<uint8_t>(255 * (1.0f - progress));
            }
        }
    }

    void render(Graphics::Renderer& renderer) {
        for (const auto& p : particles) {
            if (!p.active) continue;

            if (p.type == ParticleType::RING) {
                renderer.drawCircle(p.x, p.y, p.size, 3.0f, Graphics::Color{p.r, p.g, p.b, p.a});
            } else {
                int sz = std::max(1, static_cast<int>(p.size));
                renderer.drawRect(static_cast<int>(p.x - sz / 2), static_cast<int>(p.y - sz / 2), sz, sz, p.r, p.g, p.b, p.a);
            }
        }
    }

    void clear() {
        for (auto& p : particles) p.active = false;
    }

private:
    std::vector<Particle> particles;
};

} // namespace Graphics
