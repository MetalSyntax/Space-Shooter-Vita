#include "audio/Synth.hpp"
#include <algorithm>
#include <cstdlib>

namespace Audio {

constexpr float SAMPLE_RATE = 44100.0f;
constexpr float TWO_PI = 6.2831853f;
constexpr size_t MAX_TONES = 96;
constexpr float MASTER = 1.4f;

static void sdlCallbackBridge(void* userdata, Uint8* stream, int len) {
    static_cast<Synth*>(userdata)->audioCallback(stream, len);
}

Synth& Synth::instance() {
    static Synth s_instance;
    return s_instance;
}

bool Synth::init() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        return false;
    }

    SDL_AudioSpec wanted, obtained;
    SDL_zero(wanted);
    wanted.freq = 44100;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 1;
    wanted.samples = 1024;
    wanted.callback = sdlCallbackBridge;
    wanted.userdata = this;

    deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted, &obtained, 0);
    if (deviceId == 0) {
        // En Vita si el dispositivo de audio falla no debe colgar el hilo gráfico
        return false;
    }
    activeTones.reserve(MAX_TONES);

    SDL_PauseAudioDevice(deviceId, 0);
    return true;
}

void Synth::shutdown() {
    if (deviceId != 0) {
        SDL_CloseAudioDevice(deviceId);
        deviceId = 0;
    }
}

void Synth::setMusicMuted(bool m) {
    std::lock_guard<std::mutex> lock(audioMutex);
    musicOff = m;
}

void Synth::setSfxMuted(bool m) {
    std::lock_guard<std::mutex> lock(audioMutex);
    sfxOff = m;
}

void Synth::push(const ActiveTone& t) {
    if (activeTones.size() >= MAX_TONES) return;
    activeTones.push_back(t);
}

void Synth::add(const ActiveTone& t) {
    if (deviceId == 0 || sfxOff) return;
    std::lock_guard<std::mutex> lock(audioMutex);
    push(t);
}

static ActiveTone tone(WaveType type, float from, float to, float dur, float vol, float delay = 0.0f) {
    ActiveTone t;
    t.type = type;
    t.fromFreq = from;
    t.toFreq = to;
    t.duration = dur;
    t.volume = vol;
    t.delay = delay;
    return t;
}

static ActiveTone noise(float dur, float vol, Filter f, float cutoff, float delay = 0.0f) {
    ActiveTone t = tone(WaveType::NOISE, 0, 0, dur, vol, delay);
    t.filter = f;
    t.cutoffFrom = t.cutoffTo = cutoff;
    return t;
}

static ActiveTone note(WaveType type, float freq, float dur, float vol, float delay, float attack = 0.05f) {
    ActiveTone t = tone(type, freq, freq, dur, vol, delay);
    t.attack = attack;
    t.music = true;
    return t;
}

void Synth::playLaser() { add(tone(WaveType::SAWTOOTH, 880.0f, 110.0f, 0.2f, 0.08f)); }
void Synth::playMissile() { add(tone(WaveType::TRIANGLE, 300.0f, 900.0f, 0.15f, 0.06f)); }
void Synth::playEnemyShoot() { add(tone(WaveType::SQUARE, 200.0f, 100.0f, 0.1f, 0.04f)); }
void Synth::playShieldHit() { add(tone(WaveType::SINE, 1200.0f, 400.0f, 0.15f, 0.08f)); }
void Synth::playBomb() { add(tone(WaveType::SAWTOOTH, 100.0f, 1.0f, 0.5f, 0.3f)); }

void Synth::playExplosion(bool big) {
    float duration = big ? 0.9f : 0.5f;
    ActiveTone n = noise(duration, big ? 0.35f : 0.2f, Filter::LOWPASS, big ? 1600.0f : 1000.0f);
    n.cutoffTo = 100.0f;
    add(n);
    if (big) add(tone(WaveType::SINE, 120.0f, 30.0f, 0.6f, 0.3f));
}

void Synth::playPowerUp() {
    ActiveTone a = tone(WaveType::SINE, 440.0f, 880.0f, 0.1f, 0.1f);
    a.attack = 0.001f;
    ActiveTone b = tone(WaveType::SINE, 880.0f, 1760.0f, 0.2f, 0.1f, 0.1f);
    add(a);
    add(b);
}

void Synth::playCombo(int level) {
    const float notes[3] = {523.25f, 659.25f, 783.99f};
    for (int i = 0; i <= std::min(level, 2); ++i) add(tone(WaveType::SQUARE, notes[i], notes[i], 0.08f, 0.05f, i * 0.06f));
}

void Synth::playShieldWarning() {
    for (int i = 0; i < 4; ++i) add(tone(WaveType::SQUARE, 1500.0f, 1500.0f, 0.08f, 0.05f, i * 0.14f));
}

void Synth::playCriticalWarning() {
    for (int i = 0; i < 3; ++i) add(tone(WaveType::SAWTOOTH, 220.0f, 180.0f, 0.2f, 0.08f, i * 0.3f));
}

void Synth::playBossSiren() {
    for (int i = 0; i < 6; ++i) {
        for (float f : {440.0f, 554.0f}) {
            ActiveTone t = tone(WaveType::SQUARE, i % 2 ? f * 0.75f : f, i % 2 ? f * 0.75f : f, 0.42f, 0.05f, i * 0.5f);
            t.attack = 0.02f;
            add(t);
        }
    }
}

void Synth::playLaserCharge(float duration) {
    add(tone(WaveType::SINE, 100.0f, 1600.0f, duration, 0.06f));
    add(tone(WaveType::SQUARE, 50.0f, 800.0f, duration, 0.02f));
    add(noise(0.35f, 0.25f, Filter::LOWPASS, 2500.0f, duration));
}

void Synth::playLaserBeam(float duration) {
    ActiveTone t = tone(WaveType::SAWTOOTH, 70.0f, 60.0f, duration, 0.15f);
    t.attack = 0.02f;
    add(t);
    ActiveTone n = noise(duration, 0.1f, Filter::LOWPASS, 1200.0f);
    n.attack = 0.02f;
    add(n);
}

// --- Music ---

void Synth::stopMusic() {
    std::lock_guard<std::mutex> lock(audioMutex);
    mode = Mode::NONE;
    activeTones.erase(std::remove_if(activeTones.begin(), activeTones.end(), [](const ActiveTone& t) { return t.music; }), activeTones.end());
}

void Synth::playMenuMusic() {
    stopMusic();
    std::lock_guard<std::mutex> lock(audioMutex);
    mode = Mode::MENU;
    step = 0;
    stepTimer = 0.0f;
}

void Synth::playCombatMusic(int newBpm, int sector) {
    stopMusic();
    static const int SEMITONES[10] = {0, 2, -2, 3, -3, 1, 5, -1, 4, 7};
    std::lock_guard<std::mutex> lock(audioMutex);
    baseBpm = bpm = static_cast<float>(newBpm);
    transpose = std::pow(2.0f, SEMITONES[(sector - 1 + 10) % 10] / 12.0f);
    mode = Mode::COMBAT;
    step = 0;
    stepTimer = 0.05f;
}

void Synth::setMusicIntensity(bool high) {
    std::lock_guard<std::mutex> lock(audioMutex);
    bpm = high ? std::max(baseBpm + 10.0f, 145.0f) : baseBpm;
}

void Synth::playVictoryMusic() {
    stopMusic();
    std::lock_guard<std::mutex> lock(audioMutex);
    push(note(WaveType::SQUARE, 523.25f, 0.2f, 0.1f, 0.0f));
    push(note(WaveType::SQUARE, 659.25f, 0.2f, 0.1f, 0.2f));
    push(note(WaveType::SQUARE, 783.99f, 0.2f, 0.1f, 0.4f));
    push(note(WaveType::SQUARE, 1046.50f, 0.8f, 0.1f, 0.6f));
    push(note(WaveType::SAWTOOTH, 261.63f, 0.6f, 0.05f, 0.0f));
    push(note(WaveType::SAWTOOTH, 329.63f, 0.8f, 0.05f, 0.6f));
}

void Synth::playGameOverMusic() {
    stopMusic();
    std::lock_guard<std::mutex> lock(audioMutex);
    push(note(WaveType::SAWTOOTH, 392.00f, 0.4f, 0.1f, 0.0f));
    push(note(WaveType::SAWTOOTH, 369.99f, 0.4f, 0.1f, 0.4f));
    push(note(WaveType::SAWTOOTH, 349.23f, 0.4f, 0.1f, 0.8f));
    push(note(WaveType::SAWTOOTH, 311.13f, 1.2f, 0.1f, 1.2f));
}

void Synth::scheduleStep() {
    if (mode == Mode::MENU) {
        static const float MELODY[16] = {220, 329.63f, 440, 329.63f, 261.63f, 392, 523.25f, 392,
                                         196, 293.66f, 392, 293.66f, 174.61f, 261.63f, 349.23f, 261.63f};
        push(note(WaveType::TRIANGLE, MELODY[step % 16], 0.5f, 0.05f, 0.0f));
        stepTimer += 0.5f;
        step++;
        return;
    }

    struct Chord { float bass; float arp[4]; };
    static const Chord CHORDS[4] = {
        {55.0f, {220, 261.63f, 329.63f, 440}},      // Am
        {43.65f, {174.61f, 220, 261.63f, 349.23f}}, // F
        {65.41f, {261.63f, 329.63f, 392, 523.25f}}, // C
        {49.0f, {196, 246.94f, 293.66f, 392}},      // G
    };
    const Chord& bar = CHORDS[(step / 16) % 4];
    int i = step % 16;
    float sixteenth = 60.0f / bpm / 4.0f;

    if (i % 4 == 0) {
        ActiveTone kick = tone(WaveType::SINE, 150.0f, 40.0f, 0.15f, 0.25f);
        kick.music = true;
        push(kick);
    }
    if (i == 4 || i == 12) {
        ActiveTone sn = noise(0.12f, 0.08f, Filter::HIGHPASS, 1500.0f);
        sn.music = true;
        push(sn);
    }
    if (i % 2 == 1) {
        ActiveTone hat = noise(0.03f, 0.025f, Filter::HIGHPASS, 7000.0f);
        hat.music = true;
        push(hat);
    }
    if (i % 2 == 0) push(note(WaveType::SAWTOOTH, bar.bass * transpose * (i % 4 == 2 ? 2.0f : 1.0f), sixteenth * 1.8f, 0.06f, 0.0f, 0.01f));
    push(note(WaveType::TRIANGLE, bar.arp[i % 4] * 2.0f * transpose, sixteenth * 0.9f, 0.05f, 0.0f, 0.005f));

    stepTimer += sixteenth;
    step++;
}

void Synth::audioCallback(Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int numSamples = len / sizeof(int16_t);
    std::fill(buffer, buffer + numSamples, 0);

    std::lock_guard<std::mutex> lock(audioMutex);
    constexpr float dt = 1.0f / SAMPLE_RATE;

    for (int i = 0; i < numSamples; ++i) {
        if (mode != Mode::NONE) {
            stepTimer -= dt;
            while (stepTimer <= 0.0f && mode != Mode::NONE) scheduleStep();
        }
        if (activeTones.empty()) continue;

        float mix = 0.0f;
        for (auto& t : activeTones) {
            if (t.delay > 0.0f) {
                t.delay -= dt;
                continue;
            }
            float p = std::min(1.0f, t.elapsed / t.duration);
            float freq = t.fromFreq + (t.toFreq - t.fromFreq) * p;

            float env;
            if (t.attack > 0.0f) {
                float rel = t.duration - t.attack;
                env = t.elapsed < t.attack ? t.elapsed / t.attack : t.elapsed > rel ? std::max(0.0f, (t.duration - t.elapsed) / t.attack) : 1.0f;
            } else {
                float k = 1.0f - p;
                env = k * k * k; // approximates the exponential ramp to 0.001
            }
            env *= t.volume;

            float sample = 0.0f;
            switch (t.type) {
                case WaveType::SINE: sample = std::sin(t.phase); break;
                case WaveType::SQUARE: sample = t.phase < 3.14159265f ? 1.0f : -1.0f; break;
                case WaveType::SAWTOOTH: sample = t.phase / 3.14159265f - 1.0f; break;
                case WaveType::TRIANGLE: {
                    float x = t.phase / TWO_PI;
                    sample = x < 0.5f ? 4.0f * x - 1.0f : 3.0f - 4.0f * x;
                    break;
                }
                case WaveType::NOISE: sample = (std::rand() / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f; break;
            }

            if (t.filter != Filter::NONE) {
                float fc = t.cutoffFrom + (t.cutoffTo - t.cutoffFrom) * p;
                float a = std::min(1.0f, TWO_PI * fc / SAMPLE_RATE);
                t.lp += a * (sample - t.lp);
                sample = t.filter == Filter::LOWPASS ? t.lp : sample - t.lp;
            }

            if (!(t.music ? musicOff : sfxOff)) mix += sample * env;
            t.phase += TWO_PI * freq * dt;
            if (t.phase >= TWO_PI) t.phase -= TWO_PI;
            t.elapsed += dt;
        }

        mix = std::clamp(mix * MASTER, -1.0f, 1.0f);
        buffer[i] = static_cast<int16_t>(mix * 32767.0f);
    }

    activeTones.erase(
        std::remove_if(activeTones.begin(), activeTones.end(), [](const ActiveTone& t) {
            return t.delay <= 0.0f && t.elapsed >= t.duration;
        }),
        activeTones.end()
    );
}

} // namespace Audio
