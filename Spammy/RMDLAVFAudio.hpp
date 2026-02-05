//
//  RMDLAVFAudio.hpp
//  Spammy
//
//  Created by Rémy on 19/01/2026.
//

#ifndef RMDLAVFAudio_h
#define RMDLAVFAudio_h

#include "RMDLHexagonSpace.hpp"

#include <memory>

class SpaceshipAudioEngine
{
public:
    SpaceshipAudioEngine();
    ~SpaceshipAudioEngine();
    
    void start();
    void stop();
    
    SpaceshipSynthesizer& synth() { return m_synth; }
    
    void setEngineThrottle(float throttle);
    void addBlockSound(int blockType);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    SpaceshipSynthesizer m_synth;
};

#endif /* RMDLAVFAudio_hpp */
