//
//  RMDLMouseAndCursor.cpp
//  Spammy
//
//  Created by Rémy on 16/01/2026.
//

#include "RMDLMouseAndCursor.hpp"

static const char* pickerShaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct PickerUniforms {
    float2 mousePos;
    float2 screenSize;
    float4x4 inverseViewProj;
};

struct PickResult {
    float3 worldPosition;
    float depth;
    int valid;
};

kernel void pickDepth(
    texture2d<float, access::read> depthTexture [[texture(0)]],
    constant PickerUniforms& uniforms [[buffer(0)]],
    device PickResult& result [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x != 0 || gid.y != 0) return;
    
    int2 pixelCoord = int2(uniforms.mousePos);
    
    if (pixelCoord.x < 0 || pixelCoord.x >= int(uniforms.screenSize.x) ||
        pixelCoord.y < 0 || pixelCoord.y >= int(uniforms.screenSize.y))
    {
        result.valid = 0;
        return;
    }
    
    float depth = depthTexture.read(uint2(pixelCoord)).r;
    
    if (depth >= 1.0) {
        result.valid = 0;
        result.depth = depth;
        return;
    }
    
    // NDC: x [-1, 1], y [-1, 1]
    float2 ndc;
    ndc.x = (uniforms.mousePos.x / uniforms.screenSize.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (uniforms.mousePos.y / uniforms.screenSize.y) * 2.0;
    
    // Clip space -> World space
    float4 clipPos = float4(ndc.x, ndc.y, depth, 1.0);
    float4 worldPos = uniforms.inverseViewProj * clipPos;
    worldPos /= worldPos.w;
    
    result.worldPosition = worldPos.xyz;
    result.depth = depth;
    result.valid = 1;
}
)";

MouseDepthPicker::MouseDepthPicker(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
    : m_mousePositionComputePipelineState(nullptr), m_uniformBuffer(nullptr), m_resultBuffer(nullptr)
{
    m_uniforms = {};
    buildPipeline(shaderLibrary, pixelFormat, depthPixelFormat, device);
    buildBuffers(device);
}

MouseDepthPicker::~MouseDepthPicker()
{
    if (m_mousePositionComputePipelineState) m_mousePositionComputePipelineState->release();
    if (m_uniformBuffer) m_uniformBuffer->release();
    if (m_resultBuffer) m_resultBuffer->release();
}

void MouseDepthPicker::buildPipeline(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device)
{
    NS::Error* error = nullptr;
    
    NS::SharedPtr<MTL::CompileOptions> compileOptions = NS::TransferPtr(MTL::CompileOptions::alloc()->init());
    shaderLibrary = device->newLibrary(NS::String::string(pickerShaderSource, NS::UTF8StringEncoding), compileOptions.get(), &error); // for const char*
    
    NS::SharedPtr<MTL::Function> function = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("pickDepth")));
    
    m_mousePositionComputePipelineState = device->newComputePipelineState(function.get(), &error);
    
    if (!m_mousePositionComputePipelineState)
        printf("Failed to create picker pipeline: %s\n", error->localizedDescription()->utf8String());
}

void MouseDepthPicker::buildBuffers(MTL::Device* device)
{
    m_uniformBuffer = device->newBuffer(sizeof(PickerUniforms), MTL::ResourceStorageModeShared);
    
    m_resultBuffer = device->newBuffer(sizeof(MousePickResult), MTL::ResourceStorageModeShared);
}

void MouseDepthPicker::setMousePosition(float x, float y)
{
    m_uniforms.mousePos = simd::make_float2(x, y);
}

void MouseDepthPicker::setScreenSize(float width, float height)
{
    m_uniforms.screenSize = simd::make_float2(width, height);
}

void MouseDepthPicker::setInverseViewProjection(simd::float4x4 invViewProj)
{
    m_uniforms.inverseViewProj = invViewProj;
}

void MouseDepthPicker::pick(MTL::CommandBuffer* commandBuffer, MTL::Texture* depthTexture)
{
    memcpy(m_uniformBuffer->contents(), &m_uniforms, sizeof(PickerUniforms));
//    m_uniformBuffer->didModifyRange(NS::Range::Make(0, sizeof(PickerUniforms)));
    
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    encoder->setComputePipelineState(m_mousePositionComputePipelineState);
    encoder->setTexture(depthTexture, 0);
    encoder->setBuffer(m_uniformBuffer, 0, 0);
    encoder->setBuffer(m_resultBuffer, 0, 1);
    
    encoder->dispatchThreads(MTL::Size(1, 1, 1), MTL::Size(1, 1, 1));
    encoder->endEncoding();
}

MousePickResult MouseDepthPicker::getResult()
{
    MousePickResult result;
    
    struct GPUResult {
        simd::float3 worldPosition;
        float depth;
        int valid;
    };
    
    GPUResult* gpuResult = static_cast<GPUResult*>(m_resultBuffer->contents());
    result.worldPosition = gpuResult->worldPosition;
    result.depth = gpuResult->depth;
    result.valid = gpuResult->valid != 0;
    
    return result;
}
