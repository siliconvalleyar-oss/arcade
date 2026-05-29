#pragma once
#include <cstdint>
#include <thread>
#include <atomic>
#include <vector>

namespace ECS {

// Procedural audio system - generates PCM samples and pipes to aplay
struct AudioSystem_t {
    explicit AudioSystem_t();
    ~AudioSystem_t();

    void playJump();
    void playDeath();
    void playScore();
    void playGameOver();
    void playBossHit();
    void playBossDefeat();
    void startMusic();
    void stopMusic();

private:
    // Internal PCM generation
    static void generateSine(std::vector<int16_t>& buffer, float freq,
                             float durationSec, float volume, float sampleRate = 44100.0f);
    static void generateNoise(std::vector<int16_t>& buffer,
                              float durationSec, float volume, float sampleRate = 44100.0f);
    static void generateSweep(std::vector<int16_t>& buffer, float freqStart,
                              float freqEnd, float durationSec, float volume,
                              float sampleRate = 44100.0f);

    // Play PCM data (blocking)
    void playPCM(const std::vector<int16_t>& samples);

    // Background music thread
    static void musicThreadFunc(AudioSystem_t* self);

    // Raw PCM playback via aplay
    FILE* m_aplayPipe { nullptr };
    std::thread m_musicThread;
    std::atomic<bool> m_musicRunning { false };
    std::atomic<bool> m_initialized { false };
};

}
