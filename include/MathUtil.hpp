#pragma once

#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Game {

constexpr float PI = 3.14159265f;
constexpr float DEG = PI / 180.0f;

inline float frand(float a, float b) {
    return a + (b - a) * (std::rand() / (float)RAND_MAX);
}

/** Integer in [a, b] inclusive, like Phaser.Math.Between. */
inline int irand(int a, int b) {
    return a + std::rand() % (b - a + 1);
}

inline float wrapAngle(float a) {
    while (a > PI) a -= 2 * PI;
    while (a < -PI) a += 2 * PI;
    return a;
}

/** Phaser.Math.Angle.RotateTo: turns `current` toward `target` by at most `step` radians. */
inline float rotateTo(float current, float target, float step) {
    float diff = wrapAngle(target - current);
    if (std::fabs(diff) <= step) return target;
    return current + (diff > 0 ? step : -step);
}

inline float dist2(float x0, float y0, float x1, float y1) {
    return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

} // namespace Game
