//
//  RMDLAVFAudio.cpp
//  Spammy
//
//  Created by Rémy on 19/01/2026.
//

#include "RMDLAVFAudio.hpp"

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>

struct SpaceshipAudioEngine::Impl
{
    AVAudioEngine* engine = nil;
    AVAudioSourceNode* sourceNode = nil;
    bool running = false;
};

SpaceshipAudioEngine::SpaceshipAudioEngine() : m_impl(std::make_unique<Impl>())
{
    m_impl->engine = [[AVAudioEngine alloc] init];
    
    AVAudioFormat* format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100.0 channels:2];
    m_synth.init(44100.0f);
    
    __block SpaceshipSynthesizer* synthPtr = &m_synth;
    
    m_impl->sourceNode = [[AVAudioSourceNode alloc] initWithFormat:format renderBlock:^OSStatus(BOOL* isSilence, const AudioTimeStamp* timestamp, AVAudioFrameCount frameCount, AudioBufferList* outputData) {
            
            float* leftChannel = (float*)outputData->mBuffers[0].mData;
            float* rightChannel = outputData->mNumberBuffers > 1 ? (float*)outputData->mBuffers[1].mData : leftChannel;
            
            for (AVAudioFrameCount i = 0; i < frameCount; i++)
            {
                float sample = synthPtr->processSample();
                leftChannel[i] = sample;
                rightChannel[i] = sample;
            }
            
            *isSilence = NO;
            return noErr;
        }];
    
    [m_impl->engine attachNode:m_impl->sourceNode];
    [m_impl->engine connect:m_impl->sourceNode to:m_impl->engine.mainMixerNode format:format];
}

SpaceshipAudioEngine::~SpaceshipAudioEngine()
{
    stop();
    m_impl->sourceNode = nil;
    m_impl->engine = nil;
}

void SpaceshipAudioEngine::start()
{
    if (!m_impl->running)
    {
        NSError* error = nil;
        [m_impl->engine startAndReturnError:&error];
        if (error)
            NSLog(@"Audio engine error: %@", error);
        m_impl->running = true;
    }
}

void SpaceshipAudioEngine::stop()
{
    if (m_impl->running)
    {
        [m_impl->engine stop];
        m_impl->running = false;
    }
}

void SpaceshipAudioEngine::setEngineThrottle(float throttle)
{
    m_synth.setMasterThrottle(throttle);
}

void SpaceshipAudioEngine::addBlockSound(int blockType)
{
    BlockSoundProfile profile;
    switch (blockType) {
        case 0: profile = BlockPresets::Engine(); break;
        case 1: profile = BlockPresets::Thruster(); break;
        case 2: profile = BlockPresets::Generator(); break;
        case 3: profile = BlockPresets::Shield(); break;
        case 4: profile = BlockPresets::Weapon(); break;
        case 5: profile = BlockPresets::Cockpit(); break;
        default: profile = BlockPresets::Engine(); break;
    }
    
    int voiceId = m_synth.addVoice(profile);
    m_synth.triggerVoice(voiceId, 1.0f);
}
