//
//  RMDLSkybox.cpp
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//

#include "RMDLSkybox.hpp"

namespace sky {

RMDLSkybox::RMDLSkybox(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary) :
_pDevice(pDevice->retain()), _pPixelFormat(pPixelFormat), _pDepthPixelFormat(pDepthPixelFormat)
{
    _pUniformBuffer = _pDevice->newBuffer(sizeof(RMDLSkyboxUniforms), MTL::ResourceStorageModeShared);
    createPipeline(pShaderLibrary, pPixelFormat, pDepthPixelFormat);
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };
    _pVertexCount = 4;
    _pVertexBuffer = _pDevice->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
}

RMDLSkybox::~RMDLSkybox()
{
    
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
    pRenderDescriptor->setDepthAttachmentPixelFormat(_pDepthPixelFormat);
    pRenderDescriptor->colorAttachments()->object(0)->setPixelFormat(pPixelFormat);//??

    NS::Error* pError = nullptr;
    _pPipelineStateBlender = _pDevice->newRenderPipelineState(pRenderDescriptor.get(), &pError);

    NS::SharedPtr<MTL::DepthStencilDescriptor> pDepthStencilDesc = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    pDepthStencilDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLessEqual);
    pDepthStencilDesc->setDepthWriteEnabled(false);
    _pDepthState = _pDevice->newDepthStencilState(pDepthStencilDesc.get());
}

void RMDLSkybox::updateUniforms(const simd::float4x4& view, const simd::float4x4& proj, const simd::float3& camPos)
{
    RMDLSkyboxUniforms* pUniforms = reinterpret_cast<RMDLSkyboxUniforms*>(_pUniformBuffer->contents());

    simd::float4x4 viewProj = simd_mul(proj, view);
    pUniforms->invViewProjection = simd_inverse(viewProj);
    pUniforms->cameraPos = camPos;
    
    // Atmosphere params
    pUniforms->sunDir = simd_normalize(_pAtmosphereParams.sunDirection);
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

void RMDLSkybox::setSunDirection(const simd::float3& dir)
{
    _pAtmosphereParams.sunDirection = simd_normalize(dir);
}

}
