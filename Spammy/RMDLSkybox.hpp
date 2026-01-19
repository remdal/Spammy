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
#include <cmath>
#include "RMDLMainRenderer_shared.h"

namespace sky {

struct AtmosphereParams
{
    simd::float3 sunDirection{0.0f, 0.5f, 0.5f};
    float sunIntensity = 12200.0f;
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

namespace skybox {

class BlackHole
{
public:
    BlackHole(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~BlackHole();
    
    void render(MTL::RenderCommandEncoder* renderCommandEncoder, const simd::float4x4& viewProjectionMatrix,
                const simd::float4x4& invViewProjectionMatrix, const simd::float3& cameraPosition);
    
    void update(float dt);
    
    void setPosition(simd::float3 pos) { m_position = pos; }
    simd::float3 position() const { return m_position; }
    
    void setRadius(float r) { m_radius = r; }
    float radius() const { return m_radius; }
    
    void setAccretionDiskRadii(float inner, float outer) { m_diskInner = inner; m_diskOuter = outer; }
    void setGravitationalStrength(float s) { m_gravStrength = s; }
    void setRotationSpeed(float s) { m_rotationSpeed = s; }
    void setVisible(bool v) { m_visible = v; }
    bool isVisible() const { return m_visible; }
    
private:
    void buildPipeline(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    void buildBuffers(MTL::Device* device);
    
    MTL::RenderPipelineState* m_renderPipelineState;
    MTL::DepthStencilState*   m_depthStencilState;
    MTL::Buffer*              m_vertexBuffer;
    MTL::Buffer*              m_indexBuffer;
    MTL::Buffer*              m_uniformBuffer;
    
    simd::float3 m_position      = {0.f, 50.f, -100.f};
    float        m_radius        = 8.f;
    float        m_diskInner     = 12.f;
    float        m_diskOuter     = 40.f;
    float        m_gravStrength  = 2.5f;
    float        m_rotationSpeed = 0.3f;
    float        m_time          = 0.f;
    bool         m_visible       = true;
    uint32_t     m_indexCount    = 0;
};
}

#endif /* RMDLSkybox_hpp */
