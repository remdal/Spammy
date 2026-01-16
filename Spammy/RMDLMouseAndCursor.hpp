//
//  RMDLMouseAndCursor.hpp
//  Spammy
//
//  Created by Rémy on 16/01/2026.
//

#ifndef RMDLMouseAndCursor_hpp
#define RMDLMouseAndCursor_hpp

#include <Metal/Metal.hpp>

#include <simd/simd.h>

#include "RMDLMainRenderer_shared.h"

struct MousePickResult
{
    simd::float3 worldPosition;
    float depth;
    bool valid;
};

struct PickerUniforms
{
    simd::float2 mousePos;
    simd::float2 screenSize;
    simd::float4x4 inverseViewProj;
};

class MouseDepthPicker
{
public:
    MouseDepthPicker(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~MouseDepthPicker();
    
    void setMousePosition(float x, float y);
    void setScreenSize(float width, float height);
    void setInverseViewProjection(simd::float4x4 invViewProj);
    
    void pick(MTL::CommandBuffer* commandBuffer, MTL::Texture* depthTexture);
    
    MousePickResult getResult();
    
private:
    void buildPipeline(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device);
    void buildBuffers(MTL::Device* device);

    MTL::ComputePipelineState*  m_mousePositionComputePipelineState;
    MTL::Buffer*                m_uniformBuffer;
    MTL::Buffer*                m_resultBuffer;
    
    PickerUniforms              m_uniforms;
};

#endif /* RMDLMouseAndCursor_hpp */
