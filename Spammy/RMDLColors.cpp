//
//  RMDLColors.cpp
//  Spammy
//
//  Created by Rémy on 04/01/2026.
//

#include "RMDLColors.hpp"

VibrantColorRenderer::VibrantColorRenderer(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
{
    setupPipelines(device, pixelFormat, depthPixelFormat, shaderLibrary);
    setupRenderTargets(device, pixelFormat);
    createQuadBuffer(device);
    createSamplerState(device);
}

VibrantColorRenderer::~VibrantColorRenderer()
{
    m_hdrRenderTarget->release();
    m_bloomTexture->release();
}

void VibrantColorRenderer::setupRenderTargets(MTL::Device* device, MTL::PixelFormat pixelFormat)
{
    NS::SharedPtr<MTL::TextureDescriptor> hdrDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    hdrDesc->setWidth(1920);
    hdrDesc->setHeight(1080);
    hdrDesc->setPixelFormat(pixelFormat);
    hdrDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    hdrDesc->setStorageMode(MTL::StorageModePrivate);
    m_hdrRenderTarget = device->newTexture(hdrDesc.get());

    NS::SharedPtr<MTL::TextureDescriptor> bloomDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    bloomDesc->setWidth(1920 / 4);
    bloomDesc->setHeight(1080 / 4);
    bloomDesc->setPixelFormat(pixelFormat);
    bloomDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    bloomDesc->setStorageMode(MTL::StorageModePrivate);
    m_bloomTexture = device->newTexture(bloomDesc.get());
}

void VibrantColorRenderer::renderPostProcess(MTL::RenderCommandEncoder* encoder)
{
    encoder->setRenderPipelineState(m_postProcessrenderPipelineState);

    ColorGradingUniforms uniforms;
    uniforms.saturation = 1.3f;      // +30% saturation
    uniforms.brightness = 1.05f;     // +5% luminosité
    uniforms.contrast = 1.15f;       // +15% contraste
    uniforms.bloomIntensity = 0.3f;  // Bloom modéré

    encoder->setFragmentBytes(&uniforms, sizeof(uniforms), 0);
    encoder->setFragmentTexture(m_hdrRenderTarget, 0);
    encoder->setFragmentTexture(m_bloomTexture, 1);

    render(encoder);
}

void VibrantColorRenderer::setupPipelines(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
{
    NS::SharedPtr<MTL::VertexDescriptor> vertexDescriptor = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());

    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd::float2));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(simd::float2) * 2);
    vertexDescriptor->layouts()->object(0)->setStepRate(1);
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    NS::SharedPtr<MTL::RenderPipelineDescriptor> renderPipelineDescriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    NS::SharedPtr<MTL::Function> vertexFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("postProcessVertex")));
    NS::SharedPtr<MTL::Function> fragmentFunction = NS::TransferPtr(shaderLibrary->newFunction(AAPLSTR("postProcessFragment")));

    renderPipelineDescriptor->setVertexFunction(vertexFunction.get());
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction.get());
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor.get());
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(depthPixelFormat);
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);

    NS::Error* error = nullptr;
    m_postProcessrenderPipelineState = device->newRenderPipelineState(renderPipelineDescriptor.get(), &error);

    NS::SharedPtr<MTL::Function> bloomFragFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("bloomExtractFragment")));
    renderPipelineDescriptor->setFragmentFunction(bloomFragFunction.get());
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);

    m_renderPipelineState = device->newRenderPipelineState(renderPipelineDescriptor.get(), &error);

    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(false);
    m_depthStencilState = device->newDepthStencilState(depthStencilDescriptor.get());
}

void VibrantColorRenderer::render(MTL::RenderCommandEncoder *encoder)
{
    encoder->setDepthStencilState(m_depthStencilState);
    encoder->setVertexBuffer(m_quadVertexBuffer, 0, 0);
    encoder->setFragmentSamplerState(m_samplerState, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
}

void VibrantColorRenderer::createQuadBuffer(MTL::Device* device)
{
    struct VertexQuadColors {
        simd::float2 position;
        simd::float2 textureCoordonnees;
    };

    VertexQuadColors quadVerticesColors[] = {
        {{-1.0f, -1.0f}, {0.0f, 1.0f}}, // Bottom-left
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}}, // Bottom-right
        {{-1.0f,  1.0f}, {0.0f, 0.0f}}, // Top-left
        
        {{-1.0f,  1.0f}, {0.0f, 0.0f}}, // Top-left
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}}, // Bottom-right
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}}  // Top-right
    };
    m_quadVertexBuffer = device->newBuffer(quadVerticesColors, sizeof(quadVerticesColors), MTL::ResourceStorageModeShared);
}

void VibrantColorRenderer::createSamplerState(MTL::Device* device)
{
    NS::SharedPtr<MTL::SamplerDescriptor> samplerDescriptor = NS::TransferPtr(MTL::SamplerDescriptor::alloc()->init());
    samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    m_samplerState = device->newSamplerState(samplerDescriptor.get());
}
