//
//  RMDLSkybox.hpp
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//

#ifndef RMDLSkybox_hpp
#define RMDLSkybox_hpp

#include <stdio.h>
#import <Metal/Metal.hpp>
#import <simd/simd.h>
#include <memory>
#include <string>
#include "RMDLMainRenderer_shared.h"

namespace sky {

struct AtmosphereParams {
    simd::float3 sunDirection{0.0f, 0.5f, 0.5f};
    float sunIntensity = 122.0f;
    // Rayleigh scattering (bleu du ciel)
    simd::float3 rayleighScattering{5.8e-6f, 13.5e-6f, 33.1e-6f};
    float rayleighScaleHeight = 8000.0f;
    // Mie scattering (brume/pollution)
    float mieScattering = 21e-6f;
    float mieScaleHeight = 1200.0f;
    float mieG = 0.76f; // Anisotropie
    // Paramètres planète
    float planetRadius = 6371e3f;      // Terre
    float atmosphereRadius = 6471e3f;  // +100km
    // Artistique
    simd::float3 groundAlbedo{0.3f, 0.3f, 0.3f};
    float exposure = 1.0f;
    float timeOfDay = 0.5f; // 0=minuit, 0.5=midi, 1=minuit
};

class RMDLSkybox
{
public:
    RMDLSkybox(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary);
    ~RMDLSkybox();
    
    void render(MTL::RenderCommandEncoder* pEncoder, const simd::float4x4& viewMatrix, const simd::float4x4& projMatrix, const simd::float3& cameraPos);
    void setAtmosphereParams(const AtmosphereParams& params);
    void setTimeOfDay(float time);
    void setSunDirection(const simd::float3& dir);
    void updateUniforms(const simd::float4x4& view,
                       const simd::float4x4& proj,
                       const simd::float3& camPos);

    AtmosphereParams& getParams() { return _pAtmosphereParams; }
    
private:
    void createPipeline(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat);
    
    MTL::Device*                    _pDevice;
    MTL::Buffer*                    _pVertexBuffer;
    MTL::Buffer*                    _pUniformBuffer;
    MTL::PixelFormat                _pPixelFormat;
    MTL::PixelFormat                _pDepthPixelFormat;
    MTL::DepthStencilState*         _pDepthState;
    MTL::RenderPipelineState*       _pPipelineStateBlender;
   
    
    AtmosphereParams                _pAtmosphereParams;
    uint32_t                        _pVertexCount;
};

inline simd::float3 calculateSunDirection(float timeOfDay)
{   // 0.25=aube, 0.75=crépuscule
    float angle = (timeOfDay - 0.25f) * 2.0f * M_PI;
    return simd::normalize(simd::float3{ std::cos(angle), std::sin(angle), 0.0f });
}

}

#endif /* RMDLSkybox_hpp */
