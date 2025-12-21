//
//  RMDLSnow.hpp
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//

#ifndef RMDLSnow_hpp
#define RMDLSnow_hpp

#include <stdio.h>
#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include "RMDLMainRenderer_shared.h"

namespace snow {

struct SnowConfig
{
    uint32_t particleCount = 1500;
    float spawnRadius = 100.0f;
    float spawnHeight = 50.0f;
    float fallSpeed = 2.0f;
    float windStrength = 1.0f;
    simd::float3 windDirection{1.0f, 0.0f, 0.5f};
    float turbulence = 0.5f;
    float particleSize = 0.05f;
    float respawnThreshold = -5.0f;
};

class RMDLSnow
{
public:
    RMDLSnow(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary, uint32_t maxParticles = 10000);
    ~RMDLSnow();

    void update(MTL::ComputeCommandEncoder* pEncoder,
                float deltaTime,
                const simd::float3& cameraPos);

    void render(MTL::RenderCommandEncoder* pEncoder,
                const simd::float4x4& viewProj,
                const simd::float3& cameraPos);

    void setConfig(const SnowConfig& config);
    SnowConfig& getConfig() { return _config; }
    void reset();
    void setIntensity(float intensity); // 0.0 = pas de neige, 1.0 = max
    
private:
    void createComputePipeline(MTL::Library* pShaderLibrary);
    void createRenderPipeline(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat);
    void createBuffers();
    void initializeParticles();
    
    MTL::Device*                _pDevice;
    MTL::ComputePipelineState*  _pComputePipeline;
    MTL::RenderPipelineState*   _pRenderPipeline;
    MTL::DepthStencilState*     _pDepthState;

    MTL::Buffer*                _pParticleBuffer;
    MTL::Buffer*                _pParticleDataBuffer;
    MTL::Buffer*                _pUniformBuffer;
    MTL::Texture*               _pSnowflakeTexture;
    
    SnowConfig                  _config;
    uint32_t                    _maxParticles;
    float                       _time;
};

}

#endif /* RMDLSnow_hpp */
