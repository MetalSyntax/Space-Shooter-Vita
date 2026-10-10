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

enum class Filter { NONE, LOWPASS, HIGHPASS };

struct ActiveTone {
    WaveType type = WaveType::SINE;
    float fromFreq = 440.0f;
    float toFreq = 440.0f;
    float duration = 0.2f;
    float volume = 0.1f;
    float delay = 0.0f;   // seconds before it starts sounding
    float attack = 0.0f;  // > 0: music envelope (attack, hold, release) instead of a decay
    bool music = false;
    Filter filter = Filter::NONE;
    float cutoffFrom = 1000.0f;
    float cutoffTo = 1000.0f;
    float elapsed = 0.0f;
    float phase = 0.0f;
    float lp = 0.0f;
};

/** Procedural audio: SFX + sequenced music (port of Synth.ts), mixed in the SDL audio callback. */
class Synth {
public:
    static Synth& instance();

    bool init();
    void shutdown();

    void setMusicMuted(bool m);
    void setSfxMuted(bool m);
    bool musicMuted() const { return musicOff; }
    bool sfxMuted() const { return sfxOff; }

    void playLaser();
    void playMissile();
    void playEnemyShoot();
    void playExplosion(bool big = false);
    void playShieldHit();
    void playPowerUp();
    void playBomb();
    void playCombo(int level);
    void playShieldWarning();
    void playCriticalWarning();
    void playBossSiren();
    void playLaserCharge(float duration);
    void playLaserBeam(float duration);

    void stopMusic();
    void playMenuMusic();
    void playCombatMusic(int bpm, int sector);
    void setMusicIntensity(bool high);
    void playVictoryMusic();
    void playGameOverMusic();

    void audioCallback(Uint8* stream, int len);

private:
    Synth() = default;
    ~Synth() = default;

    enum class Mode { NONE, MENU, COMBAT };

    void add(const ActiveTone& t);       // locks
    void push(const ActiveTone& t);      // caller holds the lock
    void scheduleStep();                 // caller holds the lock

    SDL_AudioDeviceID deviceId = 0;
    std::mutex audioMutex;
    std::vector<ActiveTone> activeTones;
    bool musicOff = false;
    bool sfxOff = false;

    Mode mode = Mode::NONE;
    float bpm = 120.0f;
    float baseBpm = 120.0f;
    float transpose = 1.0f;
    int step = 0;
    float stepTimer = 0.0f;
};

} // namespace Audio
