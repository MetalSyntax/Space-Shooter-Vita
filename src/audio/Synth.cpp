#include "audio/Synth.hpp"
#include <algorithm>
#include <cstdlib>

namespace Audio {

static void sdlCallbackBridge(void* userdata, Uint8* stream, int len) {
    static_cast<Synth*>(userdata)->audioCallback(stream, len);
}

Synth& Synth::instance() {
    static Synth s_instance;
    return s_instance;
}

bool Synth::init() {
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
        return false;
    }

    SDL_PauseAudioDevice(deviceId, 0); // Comenzar reproducción
    return true;
}

void Synth::shutdown() {
    if (deviceId != 0) {
        SDL_CloseAudioDevice(deviceId);
        deviceId = 0;
    }
}

void Synth::playLaser() {
    std::lock_guard<std::mutex> lock(audioMutex);
    activeTones.push_back({WaveType::SAWTOOTH, 880.0f, 110.0f, 0.18f, 0.12f, 0.0f, 0.0f});
}

void Synth::playMissile() {
    std::lock_guard<std::mutex> lock(audioMutex);
    activeTones.push_back({WaveType::TRIANGLE, 300.0f, 900.0f, 0.20f, 0.10f, 0.0f, 0.0f});
}

void Synth::playEnemyShoot() {
    std::lock_guard<std::mutex> lock(audioMutex);
    activeTones.push_back({WaveType::SQUARE, 200.0f, 100.0f, 0.12f, 0.08f, 0.0f, 0.0f});
}

void Synth::playExplosion(bool big) {
    std::lock_guard<std::mutex> lock(audioMutex);
    activeTones.push_back({WaveType::NOISE, 150.0f, 40.0f, big ? 0.60f : 0.35f, big ? 0.28f : 0.18f, 0.0f, 0.0f});
}

void Synth::playShieldHit() {
    std::lock_guard<std::mutex> lock(audioMutex);
    activeTones.push_back({WaveType::SINE, 1200.0f, 400.0f, 0.15f, 0.12f, 0.0f, 0.0f});
}

void Synth::playBomb() {
    std::lock_guard<std::mutex> lock(audioMutex);
    activeTones.push_back({WaveType::SAWTOOTH, 120.0f, 20.0f, 0.70f, 0.35f, 0.0f, 0.0f});
}

void Synth::audioCallback(Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int numSamples = len / sizeof(int16_t);
    std::fill(buffer, buffer + numSamples, 0);

    std::lock_guard<std::mutex> lock(audioMutex);
    if (activeTones.empty()) return;

    constexpr float dt = 1.0f / 44100.0f;

    for (int i = 0; i < numSamples; ++i) {
        float mix = 0.0f;

        for (auto& tone : activeTones) {
            float progress = tone.elapsed / tone.duration;
            float freq = tone.fromFreq + (tone.toFreq - tone.fromFreq) * progress;
            float env = (1.0f - progress) * tone.volume;

            float sample = 0.0f;
            switch (tone.type) {
                case WaveType::SINE:
                    sample = std::sin(tone.phase);
                    break;
                case WaveType::SQUARE:
                    sample = (std::sin(tone.phase) >= 0.0f) ? 1.0f : -1.0f;
                    break;
                case WaveType::SAWTOOTH:
                    sample = (std::fmod(tone.phase, 6.2831853f) / 3.14159265f) - 1.0f;
                    break;
                case WaveType::TRIANGLE:
                    sample = (2.0f / 3.14159265f) * std::asin(std::sin(tone.phase));
                    break;
                case WaveType::NOISE:
                    sample = (std::rand() / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
                    break;
            }

            mix += sample * env;
            tone.phase += 2.0f * 3.1415926535f * freq * dt;
            tone.elapsed += dt;
        }

        // Limitador suave para evitar clipping de audio
        mix = std::clamp(mix, -1.0f, 1.0f);
        buffer[i] = static_cast<int16_t>(mix * 32767.0f);
    }

    // Eliminar tonos finalizados
    activeTones.erase(
        std::remove_if(activeTones.begin(), activeTones.end(), [](const ActiveTone& t) {
            return t.elapsed >= t.duration;
        }),
        activeTones.end()
    );
}

} // namespace Audio
