#include "audio.hpp"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <sys/wait.h>

namespace ECS {

// ============================================================
// PCM Sample Generators
// ============================================================
void AudioSystem_t::generateSine(std::vector<int16_t>& buffer, float freq,
                                  float durationSec, float volume, float sampleRate)
{
    size_t numSamples = static_cast<size_t>(sampleRate * durationSec);
    buffer.resize(numSamples);
    for(size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float sample = std::sin(2.0f * 3.14159f * freq * t);
        buffer[i] = static_cast<int16_t>(sample * volume * 32767.0f);
    }
}

void AudioSystem_t::generateNoise(std::vector<int16_t>& buffer,
                                   float durationSec, float volume, float sampleRate)
{
    size_t numSamples = static_cast<size_t>(sampleRate * durationSec);
    buffer.resize(numSamples);
    for(size_t i = 0; i < numSamples; ++i) {
        float sample = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
        buffer[i] = static_cast<int16_t>(sample * volume * 32767.0f);
    }
}

void AudioSystem_t::generateSweep(std::vector<int16_t>& buffer, float freqStart,
                                   float freqEnd, float durationSec, float volume,
                                   float sampleRate)
{
    size_t numSamples = static_cast<size_t>(sampleRate * durationSec);
    buffer.resize(numSamples);
    for(size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float frac = t / durationSec;
        float freq = freqStart + (freqEnd - freqStart) * frac;
        float sample = std::sin(2.0f * 3.14159f * freq * t);
        // Fade out
        float envelope = 1.0f - frac;
        buffer[i] = static_cast<int16_t>(sample * volume * envelope * 32767.0f);
    }
}

// ============================================================
// AudioSystem lifecycle
// ============================================================
AudioSystem_t::AudioSystem_t()
{
    // Open aplay subprocess for raw PCM playback
    // aplay -f S16_LE -r 44100 -c 1 -t raw
    m_aplayPipe = popen("aplay -f S16_LE -r 44100 -c 1 -t raw 2>/dev/null", "w");
    if(m_aplayPipe) {
        m_initialized = true;
    }
}

AudioSystem_t::~AudioSystem_t()
{
    stopMusic();
    if(m_aplayPipe) {
        pclose(m_aplayPipe);
        m_aplayPipe = nullptr;
    }
}

void AudioSystem_t::playPCM(const std::vector<int16_t>& samples)
{
    if(!m_initialized || !m_aplayPipe || samples.empty()) return;
    fwrite(samples.data(), sizeof(int16_t), samples.size(), m_aplayPipe);
    fflush(m_aplayPipe);
}

// ============================================================
// Sound effects
// ============================================================
void AudioSystem_t::playJump()
{
    std::vector<int16_t> buf;
    generateSweep(buf, 400.0f, 800.0f, 0.12f, 0.3f);
    playPCM(buf);
}

void AudioSystem_t::playDeath()
{
    std::vector<int16_t> buf;
    // Descending sweep + noise burst
    generateSweep(buf, 600.0f, 80.0f, 0.3f, 0.5f);
    std::vector<int16_t> noise;
    generateNoise(noise, 0.1f, 0.3f);
    buf.insert(buf.end(), noise.begin(), noise.end());
    playPCM(buf);
}

void AudioSystem_t::playScore()
{
    std::vector<int16_t> buf;
    generateSine(buf, 880.0f, 0.08f, 0.2f);
    std::vector<int16_t> buf2;
    generateSine(buf2, 1100.0f, 0.08f, 0.2f);
    buf.insert(buf.end(), buf2.begin(), buf2.end());
    playPCM(buf);
}

void AudioSystem_t::playBossHit()
{
    std::vector<int16_t> buf;
    // Impact sound: short low-frequency burst + high ping
    generateNoise(buf, 0.05f, 0.4f);
    std::vector<int16_t> buf2;
    generateSine(buf2, 220.0f, 0.1f, 0.5f);
    buf.insert(buf.end(), buf2.begin(), buf2.end());
    std::vector<int16_t> buf3;
    generateSine(buf3, 660.0f, 0.15f, 0.3f);
    buf.insert(buf.end(), buf3.begin(), buf3.end());
    playPCM(buf);
}

void AudioSystem_t::playBossDefeat()
{
    std::vector<int16_t> buf;
    // Long ascending sweep + explosion
    generateSweep(buf, 200.0f, 1200.0f, 0.8f, 0.6f);
    std::vector<int16_t> noise;
    generateNoise(noise, 0.3f, 0.5f);
    buf.insert(buf.end(), noise.begin(), noise.end());
    // Final triumphant chord
    std::vector<int16_t> chord;
    generateSine(chord, 440.0f, 0.5f, 0.3f);
    buf.insert(buf.end(), chord.begin(), chord.end());
    playPCM(buf);
}

void AudioSystem_t::playGameOver()
{
    std::vector<int16_t> buf;
    // Sad descending tones
    generateSine(buf, 440.0f, 0.3f, 0.4f);
    std::vector<int16_t> buf2;
    generateSine(buf2, 330.0f, 0.3f, 0.4f);
    buf.insert(buf.end(), buf2.begin(), buf2.end());
    std::vector<int16_t> buf3;
    generateSine(buf3, 220.0f, 0.4f, 0.4f);
    buf.insert(buf.end(), buf3.begin(), buf3.end());
    playPCM(buf);
}

// ============================================================
// Background music (simple looping beat)
// ============================================================
void AudioSystem_t::startMusic()
{
    if(m_musicRunning) return;
    m_musicRunning = true;
    m_musicThread = std::thread(musicThreadFunc, this);
}

void AudioSystem_t::stopMusic()
{
    m_musicRunning = false;
    if(m_musicThread.joinable()) {
        m_musicThread.join();
    }
}

void AudioSystem_t::musicThreadFunc(AudioSystem_t* self)
{
    if(!self || !self->m_initialized || !self->m_aplayPipe) return;

    // Generate a simple looping beat pattern
    constexpr float kBPM = 140.0f;
    constexpr float kBeatSec = 60.0f / kBPM;
    constexpr float kSampleRate = 44100.0f;
    constexpr size_t kBeatSamples = static_cast<size_t>(kSampleRate * kBeatSec);

    // Bass drum: low sine burst
    std::vector<int16_t> kick(kBeatSamples, 0);
    for(size_t i = 0; i < kBeatSamples / 4; ++i) {
        float t = static_cast<float>(i) / kSampleRate;
        float env = 1.0f - t / (kBeatSec * 0.25f);
        if(env < 0) env = 0;
        kick[i] = static_cast<int16_t>(std::sin(2.0f * 3.14159f * 80.0f * t) * env * 0.3f * 32767.0f);
    }

    // Hi-hat: noise burst
    std::vector<int16_t> hat(kBeatSamples, 0);
    for(size_t i = 0; i < kBeatSamples / 8; ++i) {
        float t = static_cast<float>(i) / kSampleRate;
        float env = 1.0f - t / (kBeatSec * 0.125f);
        if(env < 0) env = 0;
        float sample = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
        hat[i] = static_cast<int16_t>(sample * env * 0.15f * 32767.0f);
    }

    // Bass line (simple repeating)
    constexpr float kBassNotes[] = {110.0f, 130.81f, 146.83f, 130.81f};
    std::vector<int16_t> bass(kBeatSamples * 4, 0);
    for(int n = 0; n < 4; ++n) {
        size_t offset = n * kBeatSamples;
        for(size_t i = 0; i < kBeatSamples; ++i) {
            float t = static_cast<float>(i) / kSampleRate;
            float sample = std::sin(2.0f * 3.14159f * kBassNotes[n] * t);
            bass[offset + i] = static_cast<int16_t>(sample * 0.12f * 32767.0f);
        }
    }

    // Loop the pattern
    while(self->m_musicRunning) {
        // 4-beat pattern: KICK - HAT - KICK - HAT
        for(int beat = 0; beat < 4; ++beat) {
            if(!self->m_musicRunning) return;
            std::vector<int16_t> bar(kBeatSamples, 0);

            // Mix kick (beats 0 and 2)
            if(beat == 0 || beat == 2) {
                for(size_t i = 0; i < kBeatSamples; ++i) {
                    bar[i] += kick[i];
                }
            }

            // Mix hat (beats 1 and 3, plus offbeat)
            if(beat == 1 || beat == 3) {
                for(size_t i = 0; i < kBeatSamples; ++i) {
                    bar[i] += hat[i];
                }
            }

            // Mix bass note
            size_t bassOffset = (beat % 4) * kBeatSamples;
            for(size_t i = 0; i < kBeatSamples; ++i) {
                bar[i] += bass[bassOffset + i];
            }

            fwrite(bar.data(), sizeof(int16_t), bar.size(), self->m_aplayPipe);
            fflush(self->m_aplayPipe);
        }
    }
}

}
