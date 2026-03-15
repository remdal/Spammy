//
//  RMDLCards.cpp
//  Spammy
//
//  Created by Rémy on 14/03/2026.
//

#include "RMDLCards.hpp"

#include "stb_image.h"
#include <cstring>
#include <cassert>
#include <cmath>

static const HoloCardVertexData kCardQuad[4] = {
    // position                uv            normal       tangent
    {{ -0.65f, -0.9f, 0.f }, { 0.f, 1.f }, { 0, 0, 1 }, { 1, 0, 0 }},
    {{  0.65f, -0.9f, 0.f }, { 1.f, 1.f }, { 0, 0, 1 }, { 1, 0, 0 }},
    {{ -0.65f,  0.9f, 0.f }, { 0.f, 0.f }, { 0, 0, 1 }, { 1, 0, 0 }},
    {{  0.65f,  0.9f, 0.f }, { 1.f, 0.f }, { 0, 0, 1 }, { 1, 0, 0 }},
};

static const uint16_t kCardIndices[6] = { 0, 1, 2, 1, 3, 2 };

HoloCardRenderer::HoloCardRenderer(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthFormat, MTL::Library* shaderLibrary, const std::string& resourcePath)
: m_device(device)
{
    createPipeline(shaderLibrary, pixelFormat, depthFormat);
    createBuffers();
    createSamplers();
    loadArtwork(resourcePath);
}

HoloCardRenderer::~HoloCardRenderer()
{
    if (m_pipeline)      m_pipeline->release();
    if (m_depthState)    m_depthState->release();
    if (m_linearSampler) m_linearSampler->release();
    if (m_clampSampler)  m_clampSampler->release();
    if (m_vertexBuffer)  m_vertexBuffer->release();
    if (m_indexBuffer)   m_indexBuffer->release();
    if (m_uniformBuffer) m_uniformBuffer->release();
    if (m_artwork)       m_artwork->release();
}

void HoloCardRenderer::createPipeline(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthFormat)
{
    auto* vFn = shaderLibrary->newFunction(NS::String::string("holoCard_vertex", NS::StringEncoding::UTF8StringEncoding));
    auto* fFn = shaderLibrary->newFunction(NS::String::string("holoCard_fragment", NS::StringEncoding::UTF8StringEncoding));

    assert(vFn && "holoCard_vertex introuvable dans la library");
    assert(fFn && "holoCard_fragment introuvable dans la library");

    auto* vd  = MTL::VertexDescriptor::alloc()->init();

    // attribute(0) : position float3
    vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vd->attributes()->object(0)->setOffset(offsetof(HoloCardVertexData, position));
    vd->attributes()->object(0)->setBufferIndex(0);

    // attribute(1) : uv float2
    vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vd->attributes()->object(1)->setOffset(offsetof(HoloCardVertexData, uv));
    vd->attributes()->object(1)->setBufferIndex(0);

    // attribute(2) : normal float3
    vd->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
    vd->attributes()->object(2)->setOffset(offsetof(HoloCardVertexData, normal));
    vd->attributes()->object(2)->setBufferIndex(0);

    // attribute(3) : tangent float3
    vd->attributes()->object(3)->setFormat(MTL::VertexFormatFloat3);
    vd->attributes()->object(3)->setOffset(offsetof(HoloCardVertexData, tangent));
    vd->attributes()->object(3)->setBufferIndex(0);

    vd->layouts()->object(0)->setStride(sizeof(HoloCardVertexData));
    vd->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    // ── Pipeline descriptor ──
    auto* pd = MTL::RenderPipelineDescriptor::alloc()->init();
    pd->setLabel(NS::String::string("HoloCard", NS::StringEncoding::UTF8StringEncoding));
    pd->setVertexFunction(vFn);
    pd->setFragmentFunction(fFn);
    pd->setVertexDescriptor(vd);
    pd->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pd->setDepthAttachmentPixelFormat(depthFormat);

    // Alpha blending pour les coins arrondis de l'artwork (si PNG avec alpha)
    auto* ca = pd->colorAttachments()->object(0);
    ca->setBlendingEnabled(true);
    ca->setRgbBlendOperation(MTL::BlendOperationAdd);
    ca->setAlphaBlendOperation(MTL::BlendOperationAdd);
    ca->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    ca->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    ca->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    ca->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);

    NS::Error* err = nullptr;
    m_pipeline = m_device->newRenderPipelineState(pd, &err);
    assert(m_pipeline && "HoloCard pipeline creation failed");

    // ── Depth state — test mais pas d'écriture (la carte est transparente sur la scène) ──
    auto* dd = MTL::DepthStencilDescriptor::alloc()->init();
    dd->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    dd->setDepthWriteEnabled(false);
    m_depthState = m_device->newDepthStencilState(dd);

    pd->release();
    dd->release();
    vd->release();
    vFn->release();
    fFn->release();
}

void HoloCardRenderer::createBuffers()
{
    m_vertexBuffer = m_device->newBuffer(kCardQuad, sizeof(kCardQuad), MTL::ResourceStorageModeShared);
    m_vertexBuffer->setLabel(NS::String::string("HoloCardVB", NS::StringEncoding::UTF8StringEncoding));

    m_indexBuffer  = m_device->newBuffer(kCardIndices, sizeof(kCardIndices), MTL::ResourceStorageModeShared);
    m_indexBuffer->setLabel(NS::String::string("HoloCardIB", NS::StringEncoding::UTF8StringEncoding));

    // Ring buffer uniforms ×3 — évite les stalls CPU/GPU comme dans ton RMDLBlender
    m_uniformBuffer = m_device->newBuffer(sizeof(HoloCardUniforms) * 3, MTL::ResourceStorageModeShared);
    m_uniformBuffer->setLabel(NS::String::string("HoloCardUB", NS::StringEncoding::UTF8StringEncoding));
}

void HoloCardRenderer::createSamplers()
{
    {
        auto* d = MTL::SamplerDescriptor::alloc()->init();
        d->setMinFilter(MTL::SamplerMinMagFilterLinear);
        d->setMagFilter(MTL::SamplerMinMagFilterLinear);
        d->setMipFilter(MTL::SamplerMipFilterLinear);
        d->setSAddressMode(MTL::SamplerAddressModeRepeat);
        d->setTAddressMode(MTL::SamplerAddressModeRepeat);
        m_linearSampler = m_device->newSamplerState(d);
        d->release();
    }
    {
        auto* d = MTL::SamplerDescriptor::alloc()->init();
        d->setMinFilter(MTL::SamplerMinMagFilterLinear);
        d->setMagFilter(MTL::SamplerMinMagFilterLinear);
        d->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
        d->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
        m_clampSampler = m_device->newSamplerState(d);
        d->release();
    }
}

// ─────────────────────────────────────────────
// Artwork — même logique que ton loadTexture
// ─────────────────────────────────────────────

void HoloCardRenderer::loadArtwork(const std::string& resourcesPath)
{
    int w, h, ch;
    std::string path = resourcesPath + "/diffusecube.png";
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(path.c_str() , &w, &h, &ch, 4);
    assert(data && "HoloCardRenderer::loadArtwork — fichier introuvable");

    auto* td = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA8Unorm, (NS::UInteger)w, (NS::UInteger)h, true);
    td->setUsage(MTL::TextureUsageShaderRead);
    td->setStorageMode(MTL::StorageModeShared);

    if (m_artwork) { m_artwork->release(); m_artwork = nullptr; }
    m_artwork = m_device->newTexture(td);
    td->release();

    m_artwork->replaceRegion(MTL::Region::Make2D(0, 0, (NS::UInteger)w, (NS::UInteger)h),
                              0, data, (NS::UInteger)(w * 4));
    stbi_image_free(data);
}

void HoloCardRenderer::setTilt(simd::float2 tilt)
{
    // Smoothing identique à ce que tu ferais pour un interpolation d'animation
    const float alpha = 0.15f;
    m_tilt = m_tilt * (1.f - alpha) + tilt * alpha;
}

void HoloCardRenderer::applyProfile(const HoloCardProfile& p)
{
    holoIntensity      = p.holoIntensity;
    glassOpacity       = p.glassOpacity;
    refractionStrength = p.refractionStrength;
    sparkleIntensity   = p.sparkleIntensity;
    glareIntensity     = p.glareIntensity;
}

void HoloCardRenderer::update(float deltaTime)
{
    m_time += deltaTime;
}

void HoloCardRenderer::draw(MTL::RenderCommandEncoder* encoder, MTL::Texture* sceneTexture, const simd::float4x4& viewProj, const simd::float4x4& model, NS::UInteger width, NS::UInteger height)
{
    assert(m_artwork     && "HoloCardRenderer::draw — artwork non chargé");
    assert(sceneTexture  && "HoloCardRenderer::draw — sceneTexture nullptr");

    m_frameIndex = (m_frameIndex + 1) % 3;
    const NS::UInteger offset = sizeof(HoloCardUniforms) * m_frameIndex;

    HoloCardUniforms u{};
    u.modelMatrix         = model;
    u.viewProjMatrix      = viewProj;
    u.cameraPos           = cameraPosition;
    u.time                = m_time;
    u.tilt                = m_tilt;
    u.refractionStrength  = refractionStrength;
    u.chromaticAberration = 0.004f;
    u.fresnelPower        = 3.0f;
    u.glassOpacity        = glassOpacity;
    u.holoIntensity       = holoIntensity;
    u.holoScale           = 6.0f;
    u.holoShift           = 0.3f;
    u.sparkleIntensity    = sparkleIntensity;
    u.lightDir            = simd::normalize(lightDirection);
    u.glareIntensity      = glareIntensity;
    u.glareShininess      = 64.0f;
    u.screenSize          = { (float)width, (float)height };

    std::memcpy(static_cast<uint8_t*>(m_uniformBuffer->contents()) + offset, &u, sizeof(HoloCardUniforms));

    encoder->setRenderPipelineState(m_pipeline);
    encoder->setDepthStencilState(m_depthState);
    encoder->setVertexBuffer(m_vertexBuffer, 0, 0);
    encoder->setVertexBuffer(m_uniformBuffer, offset, 1);
    encoder->setFragmentBuffer(m_uniformBuffer, offset, 1);
    encoder->setFragmentTexture(m_artwork, 0);
    encoder->setFragmentTexture(sceneTexture, 1);
    encoder->setFragmentSamplerState(m_linearSampler, 0);
    encoder->setFragmentSamplerState(m_clampSampler, 1);
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(6), MTL::IndexTypeUInt16, m_indexBuffer, NS::UInteger(0));
}
