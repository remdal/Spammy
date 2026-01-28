//
//  RMDLSkybox.cpp
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//

#include "RMDLSkybox.hpp"

namespace sky {

RMDLSkybox::RMDLSkybox(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
: m_device(device->retain())
{
    _pUniformBuffer = m_device->newBuffer(sizeof(RMDLSkyboxUniforms), MTL::ResourceStorageModeShared);
    createPipeline(shaderLibrary, pixelFormat, depthPixelFormat);
    float vertices[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f };
    _pVertexCount = 4;
    _pVertexBuffer = m_device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
}

RMDLSkybox::~RMDLSkybox()
{
    if (_pVertexBuffer) _pVertexBuffer->release();
    if (_pUniformBuffer) _pUniformBuffer->release();
    if (_pDepthState) _pDepthState->release();
    if (_pPipelineStateBlender) _pPipelineStateBlender->release();
    if (m_device) m_device->release();
}

void RMDLSkybox::createPipeline(MTL::Library *pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat)
{
    NS::SharedPtr<MTL::VertexDescriptor> pVertexDesc = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
    pVertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(0)->setOffset(0);
    pVertexDesc->attributes()->object(0)->setBufferIndex(0);
    pVertexDesc->layouts()->object(0)->setStride(sizeof(float) * 3);

    NS::SharedPtr<MTL::Function> pVertexFunction = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("skybox_vertex", NS::UTF8StringEncoding)));
    NS::SharedPtr<MTL::Function> pFragmentFunction = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("skybox_fragment", NS::UTF8StringEncoding)));

    NS::SharedPtr<MTL::RenderPipelineDescriptor> pRenderDescriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pRenderDescriptor->setVertexFunction(pVertexFunction.get());
    pRenderDescriptor->setFragmentFunction(pFragmentFunction.get());
    pRenderDescriptor->setVertexDescriptor(pVertexDesc.get());
    pRenderDescriptor->setDepthAttachmentPixelFormat(pDepthPixelFormat);
    pRenderDescriptor->colorAttachments()->object(0)->setPixelFormat(pPixelFormat);

    NS::Error* pError = nullptr;
    _pPipelineStateBlender = m_device->newRenderPipelineState(pRenderDescriptor.get(), &pError);

    NS::SharedPtr<MTL::DepthStencilDescriptor> pDepthStencilDesc = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    pDepthStencilDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLessEqual);
    pDepthStencilDesc->setDepthWriteEnabled(false);
    _pDepthState = m_device->newDepthStencilState(pDepthStencilDesc.get());
}

void RMDLSkybox::updateUniforms(const simd::float4x4& view, const simd::float4x4& proj, const simd::float3& camPos)
{
    RMDLSkyboxUniforms* pUniforms = reinterpret_cast<RMDLSkyboxUniforms*>(_pUniformBuffer->contents());

    simd::float4x4 viewProj = simd_mul(proj, view);
    pUniforms->invViewProjection = simd_inverse(viewProj);
    pUniforms->cameraPos = camPos;

    pUniforms->rmdlSun.sunDirection = simd_normalize(_pAtmosphereParams.sunDirection);
    pUniforms->sunIntensity = _pAtmosphereParams.sunIntensity;
    pUniforms->rayleighCoeff = _pAtmosphereParams.rayleighScattering;
    pUniforms->rayleighHeight = _pAtmosphereParams.rayleighScaleHeight;
    pUniforms->mieCoeff = _pAtmosphereParams.mieScattering;
    pUniforms->mieHeight = _pAtmosphereParams.mieScaleHeight;
    pUniforms->mieG = _pAtmosphereParams.mieG;
    pUniforms->planetRadius = _pAtmosphereParams.planetRadius;
    pUniforms->atmosphereRadius = _pAtmosphereParams.atmosphereRadius;
    pUniforms->exposure = _pAtmosphereParams.exposure;
    pUniforms->timeOfDay = _pAtmosphereParams.timeOfDay;
}

void RMDLSkybox::render(MTL::RenderCommandEncoder* pEncoder,
                       const simd::float4x4& viewMatrix,
                       const simd::float4x4& projMatrix,
                       const simd::float3& cameraPos)
{
    updateUniforms(viewMatrix, projMatrix, cameraPos);
    
    pEncoder->setRenderPipelineState(_pPipelineStateBlender);
    pEncoder->setDepthStencilState(_pDepthState);
    pEncoder->setCullMode(MTL::CullModeNone);
    
    pEncoder->setVertexBuffer(_pVertexBuffer, 0, 0);
    pEncoder->setVertexBuffer(_pUniformBuffer, 0, 1);
    pEncoder->setFragmentBuffer(_pUniformBuffer, 0, 1);
    
    pEncoder->drawPrimitives(MTL::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(_pVertexCount));
}

void RMDLSkybox::setAtmosphereParams(const AtmosphereParams& params)
{
    _pAtmosphereParams = params;
}

void RMDLSkybox::setTimeOfDay(float time)
{
    _pAtmosphereParams.timeOfDay = std::clamp(time, 0.0f, 1.0f);
    _pAtmosphereParams.sunDirection = calculateSunDirection(time);
}

void RMDLSkybox::setSunDirection(const simd::float3& direction)
{
    _pAtmosphereParams.sunDirection = simd_normalize(direction);
}
}

namespace skybox {

BlackHole::BlackHole(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
{
    buildPipeline(device, pixelFormat, depthPixelFormat, shaderLibrary);
    buildBuffers(device);
}

BlackHole::~BlackHole()
{
    m_renderPipelineState->release();
    m_depthStencilState->release();
    m_vertexBuffer->release();
    m_indexBuffer->release();
    m_uniformBuffer->release();
}

void BlackHole::buildPipeline(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
{
    NS::Error* error = nullptr;
    
    NS::SharedPtr<MTL::Function> vertexFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("blackHoleVertex")));
    NS::SharedPtr<MTL::Function> fragmentFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("blackHoleFragment")));
    
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipelineDescriptor->setVertexFunction(vertexFunction.get());
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction.get());
    
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
//    NS::SharedPtr<MTL::VertexDescriptor> vertexDescriptor = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
//    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//    vertexDescriptor->attributes()->object(0)->setOffset(0);
//    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
//    vertexDescriptor->layouts()->object(0)->setStride(sizeof(12));
//    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
//    
//    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor.get());
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(depthPixelFormat);
    m_renderPipelineState = device->newRenderPipelineState(renderPipelineDescriptor, &error);
    
    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(false);
    m_depthStencilState = device->newDepthStencilState(depthStencilDescriptor.get());
}

void BlackHole::buildBuffers(MTL::Device* device)
{
    // Sphère billboard - quad face caméra
    constexpr uint32_t segments = 128;
    constexpr uint32_t rings = 64;
    
    std::vector<simd::float3> vertices;
    std::vector<uint32_t> indices;
    
    // Génération sphère pour le volume de distorsion
    for (uint32_t y = 0; y <= rings; ++y)
    {
        for (uint32_t x = 0; x <= segments; ++x)
        {
            float xSeg = (float)x / segments;
            float ySeg = (float)y / rings;
            float xPos = std::cos(xSeg * 2.f * M_PI) * std::sin(ySeg * M_PI);
            float yPos = std::cos(ySeg * M_PI);
            float zPos = std::sin(xSeg * 2.f * M_PI) * std::sin(ySeg * M_PI);
            vertices.push_back(simd::make_float3(xPos, yPos, zPos));
        }
    }
    
    for (uint32_t y = 0; y < rings; ++y)
    {
        for (uint32_t x = 0; x < segments; ++x)
        {
            uint32_t i0 = y * (segments + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + (segments + 1);
            uint32_t i3 = i2 + 1;
            indices.insert(indices.end(), {i0, i2, i1, i1, i2, i3});
        }
    }
    
    m_indexCount = (uint32_t)indices.size();
    m_vertexBuffer = device->newBuffer(vertices.data(), vertices.size() * sizeof(simd::float3), MTL::ResourceStorageModeShared);
    m_indexBuffer = device->newBuffer(indices.data(), indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    m_uniformBuffer = device->newBuffer(sizeof(BlackHoleUniforms), MTL::ResourceStorageModeShared);
}

void BlackHole::update(float dt)
{
    m_time += dt;
}

void BlackHole::render(MTL::RenderCommandEncoder* renderCommandEncoder, const simd::float4x4& viewProjectionMatrix,
                       const simd::float4x4& invViewProjectionMatrix, const simd::float3& cameraPosition)
{
    if (!m_visible) return;
    
    BlackHoleUniforms uniforms;
    uniforms.viewProjectionMatrix = viewProjectionMatrix;
    uniforms.invViewProjectionMatrix = invViewProjectionMatrix;
    uniforms.cameraPosition = cameraPosition;
    uniforms.time = m_time;
    uniforms.blackHolePosition = m_position;
    uniforms.blackHoleRadius = m_radius;
    uniforms.accretionDiskInnerRadius = m_diskInner;
    uniforms.accretionDiskOuterRadius = m_diskOuter;
    uniforms.gravitationalStrength = m_gravStrength;
    uniforms.rotationSpeed = m_rotationSpeed;
    
    memcpy(m_uniformBuffer->contents(), &uniforms, sizeof(uniforms));
    
    renderCommandEncoder->setRenderPipelineState(m_renderPipelineState);
    renderCommandEncoder->setDepthStencilState(m_depthStencilState);
    renderCommandEncoder->setCullMode(MTL::CullModeNone);
    renderCommandEncoder->setVertexBuffer(m_vertexBuffer, 0, 0);
    renderCommandEncoder->setVertexBuffer(m_uniformBuffer, 0, 1);
    renderCommandEncoder->setFragmentBuffer(m_uniformBuffer, 0, 0);
    renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, m_indexCount, MTL::IndexTypeUInt32, m_indexBuffer, 0);
}

}
