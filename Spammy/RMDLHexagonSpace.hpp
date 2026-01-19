//
//  RMDLHexagonSpace.hpp
//  Spammy
//
//  Created by Rémy on 19/01/2026.
//

#ifndef RMDLHexagonSpace_hpp
#define RMDLHexagonSpace_hpp

#include <simd/simd.h>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <random>

struct BlockSoundProfile
{
    float baseFrequency = 80.0f;    // Hz
    float harmonicRichness = 0.5f;  // 0-1 : nombre d'harmoniques
    float filterCutoff = 0.6f;      // 0-1 : brillance
    float filterResonance = 0.3f;   // 0-1 : résonance
    float lfoRate = 0.5f;           // Hz : vitesse de modulation
    float lfoDepth = 0.2f;          // 0-1 : intensité modulation
    float attack = 0.3f;            // secondes
    float release = 0.5f;           // secondes
    float detuneAmount = 0.02f;     // désaccord entre oscillateurs
    float noiseAmount = 0.05f;      // bruit de moteur
    float reverbMix = 0.3f;         // 0-1
    float delayMix = 0.2f;          // 0-1
    float delayTime = 0.3f;         // secondes
};

// Presets par type de bloc
namespace BlockPresets {
    inline BlockSoundProfile Engine() {
        return { 60.0f, 0.7f, 0.4f, 0.4f, 2.0f, 0.3f, 0.1f, 0.3f, 0.03f, 0.15f, 0.2f, 0.1f, 0.1f };
    }
    inline BlockSoundProfile Thruster() {
        return { 120.0f, 0.9f, 0.7f, 0.2f, 8.0f, 0.4f, 0.05f, 0.2f, 0.05f, 0.25f, 0.4f, 0.3f, 0.2f };
    }
    inline BlockSoundProfile Generator() {
        return { 100.0f, 0.5f, 0.5f, 0.5f, 0.3f, 0.1f, 0.5f, 1.0f, 0.01f, 0.02f, 0.3f, 0.2f, 0.4f };
    }
    inline BlockSoundProfile Shield() {
        return { 200.0f, 0.3f, 0.8f, 0.6f, 1.0f, 0.5f, 0.2f, 0.8f, 0.04f, 0.0f, 0.6f, 0.4f, 0.5f };
    }
    inline BlockSoundProfile Weapon() {
        return { 150.0f, 0.8f, 0.9f, 0.3f, 12.0f, 0.2f, 0.01f, 0.1f, 0.02f, 0.1f, 0.2f, 0.5f, 0.15f };
    }
    inline BlockSoundProfile Cockpit() {
        return { 180.0f, 0.2f, 0.6f, 0.4f, 0.2f, 0.15f, 0.4f, 1.5f, 0.005f, 0.01f, 0.5f, 0.3f, 0.6f };
    }
}

class SpaceVoice
{
public:
    void setProfile(const BlockSoundProfile& profile) { m_profile = profile; }
    void noteOn(float velocity = 1.0f);
    void noteOff();
    void setThrottle(float t) { m_throttle = std::clamp(t, 0.0f, 1.0f); }
    float process(float sampleRate);
    bool isActive() const { return m_active || m_amplitude > 0.001f; }

private:
    BlockSoundProfile m_profile;
    
    // Oscillateurs
    float m_phase[4] = {0};           // 4 oscillateurs désaccordés
    float m_lfoPhase = 0.0f;
    
    // Filtre
    float m_filterState[2] = {0};     // Filtre 2-pole
    
    // Enveloppe
    float m_amplitude = 0.0f;
    float m_targetAmplitude = 0.0f;
    bool m_active = false;
    
    // État
    float m_throttle = 0.0f;
    float m_time = 0.0f;
};

class ReverbEffect {
public:
    void init(float sampleRate, float roomSize = 0.8f);
    float process(float input);
    void setMix(float mix) { m_mix = mix; }

private:
    static constexpr int kNumCombs = 8;
    static constexpr int kNumAllpass = 4;
    
    std::vector<float> m_combBuffers[kNumCombs];
    std::vector<float> m_allpassBuffers[kNumAllpass];
    int m_combIndex[kNumCombs] = {0};
    int m_allpassIndex[kNumAllpass] = {0};
    float m_combFeedback[kNumCombs] = {0};
    float m_mix = 0.3f;
};

class DelayEffect
{
public:
    void init(float sampleRate, float maxDelay = 1.0f);
    float process(float input);
    void setTime(float seconds);
    void setFeedback(float fb) { m_feedback = std::clamp(fb, 0.0f, 0.9f); }
    void setMix(float mix) { m_mix = mix; }

private:
    std::vector<float> m_buffer;
    int m_writeIndex = 0;
    int m_delaySamples = 0;
    float m_feedback = 0.4f;
    float m_mix = 0.2f;
    float m_sampleRate = 44100.0f;
};

class SpaceshipSynthesizer
{
public:
    void init(float sampleRate = 44100.0f);
    
    // Gestion des blocs/voix
    int addVoice(const BlockSoundProfile& profile);
    void removeVoice(int voiceId);
    void setVoiceThrottle(int voiceId, float throttle);
    void setVoiceProfile(int voiceId, const BlockSoundProfile& profile);
    void triggerVoice(int voiceId, float velocity = 1.0f);
    void releaseVoice(int voiceId);
    
    // Master
    void setMasterVolume(float vol) { m_masterVolume = vol; }
    void setMasterThrottle(float t);
    
    // Rendu audio - appeler depuis le callback audio
    void renderBuffer(float* output, int numFrames, int numChannels = 2);
    
    // Pour obtenir les samples (si pas de callback direct)
    float processSample();

private:
    float m_sampleRate = 44100.0f;
    float m_masterVolume = 0.7f;
    
    std::vector<std::unique_ptr<SpaceVoice>> m_voices;
    ReverbEffect m_reverb;
    DelayEffect m_delay;
};

#endif /* RMDLHexagonSpace_hpp */
