//
//  RMDLHexagonSpace.cpp
//  Spammy
//
//  Created by Rémy on 19/01/2026.
//

#include "RMDLHexagonSpace.hpp"

static float randomFloat()
{
    static std::mt19937 gen(42);
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(gen);
}

void SpaceVoice::noteOn(float velocity)
{
    m_active = true;
    m_targetAmplitude = velocity;
}

void SpaceVoice::noteOff()
{
    m_active = false;
    m_targetAmplitude = 0.0f;
}

float SpaceVoice::process(float sampleRate)
{
    // Enveloppe ADSR simplifiée
    float envSpeed = m_active ? (1.0f / (m_profile.attack * sampleRate)) : (1.0f / (m_profile.release * sampleRate));
    m_amplitude += (m_targetAmplitude - m_amplitude) * envSpeed * 100.0f;
    m_amplitude = std::clamp(m_amplitude, 0.0f, 1.0f);
    
    if (m_amplitude < 0.0001f && !m_active) return 0.0f;
    
    // LFO
    m_lfoPhase += m_profile.lfoRate / sampleRate;
    if (m_lfoPhase > 1.0f) m_lfoPhase -= 1.0f;
    float lfo = sinf(m_lfoPhase * 2.0f * M_PI) * m_profile.lfoDepth;
    
    // Fréquence modulée par throttle et LFO
    float freq = m_profile.baseFrequency * (1.0f + m_throttle * 0.5f) * (1.0f + lfo * 0.1f);
    
    // 4 oscillateurs désaccordés (supersaw style)
    float detune[4] =
    {
        1.0f - m_profile.detuneAmount,
        1.0f - m_profile.detuneAmount * 0.5f,
        1.0f + m_profile.detuneAmount * 0.5f,
        1.0f + m_profile.detuneAmount
    };
    
    float oscMix = 0.0f;
    int numHarmonics = 1 + (int)(m_profile.harmonicRichness * 6);
    
    for (int o = 0; o < 4; o++)
    {
        float oscFreq = freq * detune[o];
        m_phase[o] += oscFreq / sampleRate;
        if (m_phase[o] > 1.0f) m_phase[o] -= 1.0f;
        
        // Onde saw avec harmoniques
        float saw = 0.0f;
        for (int h = 1; h <= numHarmonics; h++) {
            float harmPhase = fmodf(m_phase[o] * h, 1.0f);
            saw += sinf(harmPhase * 2.0f * M_PI) / h;
        }
        saw *= 0.5f;
        
        // Mix square pour plus de corps
        float square = (m_phase[o] < 0.5f) ? 0.5f : -0.5f;
        
        oscMix += saw * 0.7f + square * 0.3f;
    }
    oscMix *= 0.25f;  // Normaliser 4 oscillateurs
    
    // Bruit de moteur
    float noise = randomFloat() * m_profile.noiseAmount * (0.5f + m_throttle * 0.5f);
    oscMix += noise;
    
    // Filtre passe-bas résonant (2-pole)
    float cutoff = m_profile.filterCutoff * (0.5f + m_throttle * 0.5f) * (1.0f + lfo * 0.2f);
    cutoff = std::clamp(cutoff, 0.01f, 0.99f);
    float resonance = m_profile.filterResonance;
    
    float fc = cutoff * cutoff * 0.5f;  // Courbe exponentielle
    float q = 1.0f - resonance * 0.9f;
    
    m_filterState[0] += fc * (oscMix - m_filterState[0] + resonance * (m_filterState[0] - m_filterState[1]));
    m_filterState[1] += fc * (m_filterState[0] - m_filterState[1]);
    
    float filtered = m_filterState[1];
    
    // Amplitude finale
    float output = filtered * m_amplitude * (0.3f + m_throttle * 0.7f);
    
    m_time += 1.0f / sampleRate;
    
    return output;
}

void ReverbEffect::init(float sampleRate, float roomSize)
{
    int combSizes[kNumCombs] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
    int allpassSizes[kNumAllpass] = { 556, 441, 341, 225 };
    
    for (int i = 0; i < kNumCombs; i++)
    {
        int size = (int)(combSizes[i] * sampleRate / 44100.0f);
        m_combBuffers[i].resize(size, 0.0f);
        m_combIndex[i] = 0;
        m_combFeedback[i] = powf(roomSize, (float)combSizes[i] / 1000.0f);
    }
    
    for (int i = 0; i < kNumAllpass; i++)
    {
        int size = (int)(allpassSizes[i] * sampleRate / 44100.0f);
        m_allpassBuffers[i].resize(size, 0.0f);
        m_allpassIndex[i] = 0;
    }
}

float ReverbEffect::process(float input)
{
    float combOut = 0.0f;
    
    // Parallel comb filters
    for (int i = 0; i < kNumCombs; i++) {
        auto& buf = m_combBuffers[i];
        int& idx = m_combIndex[i];
        
        float delayed = buf[idx];
        buf[idx] = input + delayed * m_combFeedback[i];
        idx = (idx + 1) % buf.size();
        combOut += delayed;
    }
    combOut /= kNumCombs;
    
    // Series allpass filters
    float allpassOut = combOut;
    for (int i = 0; i < kNumAllpass; i++) {
        auto& buf = m_allpassBuffers[i];
        int& idx = m_allpassIndex[i];
        
        float delayed = buf[idx];
        float newVal = allpassOut + delayed * 0.5f;
        buf[idx] = newVal;
        allpassOut = delayed - newVal * 0.5f;
        idx = (idx + 1) % buf.size();
    }
    
    return input * (1.0f - m_mix) + allpassOut * m_mix;
}


void DelayEffect::init(float sampleRate, float maxDelay)
{
    m_sampleRate = sampleRate;
    m_buffer.resize((int)(maxDelay * sampleRate), 0.0f);
    m_delaySamples = (int)(0.3f * sampleRate);
}

void DelayEffect::setTime(float seconds)
{
    m_delaySamples = std::clamp((int)(seconds * m_sampleRate), 1, (int)m_buffer.size() - 1);
}

float DelayEffect::process(float input)
{
    int readIndex = m_writeIndex - m_delaySamples;
    if (readIndex < 0) readIndex += m_buffer.size();
    
    float delayed = m_buffer[readIndex];
    m_buffer[m_writeIndex] = input + delayed * m_feedback;
    m_writeIndex = (m_writeIndex + 1) % m_buffer.size();
    
    return input * (1.0f - m_mix) + delayed * m_mix;
}

void SpaceshipSynthesizer::init(float sampleRate)
{
    m_sampleRate = sampleRate;
    m_reverb.init(sampleRate, 0.85f);
    m_delay.init(sampleRate, 1.0f);
    m_delay.setTime(0.3f);
    m_delay.setFeedback(0.4f);
}

int SpaceshipSynthesizer::addVoice(const BlockSoundProfile& profile)
{
    auto voice = std::make_unique<SpaceVoice>();
    voice->setProfile(profile);
    int id = (int)m_voices.size();
    m_voices.push_back(std::move(voice));
    return id;
}

void SpaceshipSynthesizer::removeVoice(int voiceId)
{
    if (voiceId >= 0 && voiceId < m_voices.size()) {
        m_voices[voiceId]->noteOff();
    }
}

void SpaceshipSynthesizer::setVoiceThrottle(int voiceId, float throttle)
{
    if (voiceId >= 0 && voiceId < m_voices.size()) {
        m_voices[voiceId]->setThrottle(throttle);
    }
}

void SpaceshipSynthesizer::setVoiceProfile(int voiceId, const BlockSoundProfile& profile)
{
    if (voiceId >= 0 && voiceId < m_voices.size()) {
        m_voices[voiceId]->setProfile(profile);
    }
}

void SpaceshipSynthesizer::triggerVoice(int voiceId, float velocity)
{
    if (voiceId >= 0 && voiceId < m_voices.size()) {
        m_voices[voiceId]->noteOn(velocity);
    }
}

void SpaceshipSynthesizer::releaseVoice(int voiceId)
{
    if (voiceId >= 0 && voiceId < m_voices.size()) {
        m_voices[voiceId]->noteOff();
    }
}

void SpaceshipSynthesizer::setMasterThrottle(float t)
{
    for (auto& voice : m_voices) {
        voice->setThrottle(t);
    }
}

float SpaceshipSynthesizer::processSample()
{
    float mix = 0.0f;
    
    for (auto& voice : m_voices)
    {
        if (voice->isActive()) {
            mix += voice->process(m_sampleRate);
        }
    }
    
    // Limiter
    mix = std::clamp(mix, -1.0f, 1.0f);
    
    // Effets master
    mix = m_delay.process(mix);
    mix = m_reverb.process(mix);
    
    return mix * m_masterVolume;
}

void SpaceshipSynthesizer::renderBuffer(float* output, int numFrames, int numChannels)
{
    for (int i = 0; i < numFrames; i++)
    {
        float sample = processSample();
        
        for (int ch = 0; ch < numChannels; ch++)
        {
            output[i * numChannels + ch] = sample;
        }
    }
}
