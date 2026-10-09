#pragma once

#include <vector>
#include <cmath>
#include <mutex>
#include <SDL2/SDL.h>

namespace Audio {

enum class WaveType {
    SINE,
    SQUARE,
    SAWTOOTH,
    TRIANGLE,
    NOISE
};

struct ActiveTone {
    WaveType type = WaveType::SINE;
    float fromFreq = 440.0f;
    float toFreq = 440.0f;
    float duration = 0.2f;
    float volume = 0.1f;
    float elapsed = 0.0f;
    float phase = 0.0f;
};

class Synth {
public:
    static Synth& instance();

    bool init();
    void shutdown();

    void playLaser();
    void playMissile();
    void playEnemyShoot();
    void playExplosion(bool big = false);
    void playShieldHit();
    void playBomb();

    void audioCallback(Uint8* stream, int len);

private:
    Synth() = default;
    ~Synth() = default;

    SDL_AudioDeviceID deviceId = 0;
    std::mutex audioMutex;
    std::vector<ActiveTone> activeTones;
};

} // namespace Audio
