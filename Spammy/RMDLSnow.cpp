//
//  RMDLSnow.cpp
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//

#include "RMDLSnow.hpp"
#include <random>

namespace snow {

RMDLSnow::RMDLSnow(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary, uint32_t maxParticles)
    : _pDevice(pDevice->retain())
    , _pComputePipeline(nullptr)
    , _pRenderPipeline(nullptr)
    , _pDepthState(nullptr)
    , _pParticleBuffer(nullptr)
    , _pUniformBuffer(nullptr)
    , _maxParticles(maxParticles)
    , _time(0.0f)
{
    createBuffers();
    createComputePipeline(pShaderLibrary);
    createRenderPipeline(pShaderLibrary, pPixelFormat, pDepthPixelFormat);
    initializeParticles();
}

RMDLSnow::~RMDLSnow()
{
    _pComputePipeline->release();
    _pRenderPipeline->release();
    _pDepthState->release();
    _pParticleBuffer->release();
    _pUniformBuffer->release();
    _pDevice->release();
}

void RMDLSnow::createBuffers()
{
    size_t particleSize = sizeof(float) * 8; // 3 pos + 1 lifetime + 3 vel + 1 size
    _pParticleBuffer = _pDevice->newBuffer(particleSize * _maxParticles, MTL::ResourceStorageModeShared);
    _pUniformBuffer = _pDevice->newBuffer(sizeof(RMDLSnowUniforms), MTL::ResourceStorageModeShared);
}

void RMDLSnow::createComputePipeline(MTL::Library* pShaderLibrary)
{
    NS::SharedPtr<MTL::Function> pComputeFunc = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("snow_update_particles", NS::UTF8StringEncoding)));

    if (!pComputeFunc) {
        printf("ERROR: Cannot find snow_update_particles function!\n");
        return;
    }
    NS::Error* pError = nullptr;

    _pComputePipeline = _pDevice->newComputePipelineState(pComputeFunc.get(), &pError);
    
    if (!_pComputePipeline)
        printf("Compute pipeline error: %s\n", pError->localizedDescription()->utf8String());
}

void RMDLSnow::createRenderPipeline(MTL::Library* pShaderLibrary,
                                    MTL::PixelFormat pPixelFormat,
                                    MTL::PixelFormat pDepthPixelFormat)
{
    NS::SharedPtr<MTL::Function> pVertexFunc = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("snow_vertex", NS::UTF8StringEncoding)));
    NS::SharedPtr<MTL::Function> pFragmentFunc = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("snow_fragment", NS::UTF8StringEncoding)));
    
    if (!pVertexFunc || !pFragmentFunc) {
        printf("ERROR: Cannot find snow shader functions!\n");
        return;
    }
    
    NS::SharedPtr<MTL::RenderPipelineDescriptor> pDesc = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pDesc->setVertexFunction(pVertexFunc.get());
    pDesc->setFragmentFunction(pFragmentFunc.get());
    pDesc->colorAttachments()->object(0)->setPixelFormat(pPixelFormat);
    pDesc->setDepthAttachmentPixelFormat(pDepthPixelFormat);
    
    auto* colorAttachment = pDesc->colorAttachments()->object(0);
    colorAttachment->setBlendingEnabled(true);
    colorAttachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    colorAttachment->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    colorAttachment->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    colorAttachment->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    NS::Error* pError = nullptr;
    _pRenderPipeline = _pDevice->newRenderPipelineState(pDesc.get(), &pError);
    
    if (!_pRenderPipeline) {
        printf("Render pipeline error: %s\n", pError->localizedDescription()->utf8String());
    }

    NS::SharedPtr<MTL::DepthStencilDescriptor> pDSDesc = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    pDSDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    pDSDesc->setDepthWriteEnabled(false);
    _pDepthState = _pDevice->newDepthStencilState(pDSDesc.get());
}

void RMDLSnow::initializeParticles()
{
    struct Particle {
        simd::float3 position;
        float lifetime;
        simd::float3 velocity;
        float size;
    };
    
    Particle* particles = (Particle*)_pParticleBuffer->contents();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (uint32_t i = 0; i < 100; ++i) // _maxParticles
    {
        float angle = dis(gen) * 2.0f * M_PI;
        float radius = dis(gen) * _config.spawnRadius;
        
        particles[i].position = simd::float3{
            cos(angle) * radius,
            dis(gen) * _config.spawnHeight,
            sin(angle) * radius
        };
        
        particles[i].velocity = simd::float3{0.0f, 0.0f, 0.0f};
        particles[i].lifetime = dis(gen) * 10.0f;
        particles[i].size = 0.03f + dis(gen) * 0.05f;
    }
}

void RMDLSnow::update(MTL::ComputeCommandEncoder* pEncoder,
                           float deltaTime,
                           const simd::float3& cameraPos)
{
    _time += deltaTime;
    
    RMDLSnowUniforms* uniforms = (RMDLSnowUniforms*)_pUniformBuffer->contents();
    uniforms->deltaTime = deltaTime;
    uniforms->time = _time;
    uniforms->cameraPos = cameraPos;
    uniforms->spawnRadius = _config.spawnRadius;
    uniforms->spawnHeight = _config.spawnHeight;
    uniforms->fallSpeed = _config.fallSpeed;
    uniforms->windDirection = simd_normalize(_config.windDirection);
    uniforms->windStrength = _config.windStrength;
    uniforms->turbulence = _config.turbulence;
    uniforms->respawnThreshold = _config.respawnThreshold;
    uniforms->intensity = 1.0f;
    
    pEncoder->setComputePipelineState(_pComputePipeline);
    pEncoder->setBuffer(_pParticleBuffer, 0, 0);
    pEncoder->setBuffer(_pUniformBuffer, 0, 1);
    
    MTL::Size threadgroupSize = MTL::Size::Make(256, 1, 1);
    MTL::Size gridSize = MTL::Size::Make(_maxParticles, 1, 1);
    
    pEncoder->dispatchThreads(gridSize, threadgroupSize);
}

void RMDLSnow::render(MTL::RenderCommandEncoder* pEncoder,
                      const simd::float4x4& viewProj,
                      const simd::float3& cameraPos)
{
    pEncoder->setRenderPipelineState(_pRenderPipeline);
    pEncoder->setDepthStencilState(_pDepthState);
    pEncoder->setCullMode(MTL::CullModeNone);
    
    pEncoder->setVertexBuffer(_pParticleBuffer, 0, 0);
    pEncoder->setVertexBytes(&viewProj, sizeof(simd::float4x4), 1);
    pEncoder->setVertexBytes(&cameraPos, sizeof(simd::float3), 2);
    
    pEncoder->drawPrimitives(MTL::PrimitiveTypePoint, NS::UInteger(0), NS::UInteger(_maxParticles));
}

void RMDLSnow::setConfig(const SnowConfig& config)
{
    _config = config;
}

void RMDLSnow::reset()
{
    initializeParticles();
    _time = 0.0f;
}

}
