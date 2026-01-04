//
//  RMDLColors.cpp
//  Spammy
//
//  Created by Rémy on 04/01/2026.
//

#include "RMDLColors.hpp"

VibrantColorRenderer::VibrantColorRenderer(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
{
    setupPipelines();
    setupRenderTargets(device, pixelFormat);
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

void VibrantColorRenderer::renderPostProcess(MTL::RenderCommandEncoder* encoder, MTL::Texture* finalTarget)
{
    ColorGradingUniforms uniforms;
    uniforms.saturation = 1.3f;      // +30% saturation
    uniforms.brightness = 1.05f;     // +5% luminosité
    uniforms.contrast = 1.15f;       // +15% contraste
    uniforms.bloomIntensity = 0.3f;  // Bloom modéré

    encoder->setFragmentBytes(&uniforms, sizeof(uniforms), 0);
    encoder->setFragmentTexture(m_hdrRenderTarget, 0);
    encoder->setFragmentTexture(m_bloomTexture, 1);
}

void VibrantColorRenderer::setupPipelines()
{
    
}

void VibrantColorRenderer::render(MTL::RenderCommandEncoder *encoder)
{
    
}
