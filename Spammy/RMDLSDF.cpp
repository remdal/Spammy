//
//  RMDLSDF.cpp
//  Spammy
//
//  Created by Rémy on 09/02/2026.
//

#include "RMDLSDF.hpp"

#include <fstream>
#include <sstream>
#include <cmath>

#include "stb_image.h"

namespace TextRendering {

static const char* shaderSource = R"(

#include <metal_stdlib>
using namespace metal;

struct TextVertex {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
};

struct TextUniforms {
    float4x4 projectionMatrix;
    float4 textColor;
    float smoothing;
    float thickness;
    float outlineWidth;
    float padding;
    float4 outlineColor;
};

vertex VertexOut textVertexShader(TextVertex in [[stage_in]],
                                   constant TextUniforms& uniforms [[buffer(1)]]) {
    VertexOut out;
    out.position = uniforms.projectionMatrix * float4(in.position, 0.0, 1.0);
    out.texCoord = in.texCoord;
    return out;
}

fragment float4 textFragmentShader(VertexOut in [[stage_in]],
                                    constant TextUniforms& uniforms [[buffer(0)]],
                                    texture2d<float> atlas [[texture(0)]],
                                    sampler samp [[sampler(0)]]) {
    // Échantillonnage de la distance signée
    float distance = atlas.sample(samp, in.texCoord).r;
    
    // Calcul du smoothing adaptatif basé sur les dérivées d'écran
    float2 dTexCoord = fwidth(in.texCoord);
    float smoothingBase = max(dTexCoord.x, dTexCoord.y) * 16.0;
    float smoothing = max(uniforms.smoothing, smoothingBase);
    
    // Seuil pour le texte principal
    float threshold = uniforms.thickness;
    
    // Anti-aliasing avec smoothstep
    float alpha = smoothstep(threshold - smoothing, threshold + smoothing, distance);
    
    // Couleur finale du texte
    float4 color = uniforms.textColor;
    color.a *= alpha;
    
    // Contour optionnel
    if (uniforms.outlineWidth > 0.0) {
        float outlineThreshold = threshold - uniforms.outlineWidth;
        float outlineAlpha = smoothstep(outlineThreshold - smoothing, 
                                         outlineThreshold + smoothing, distance);
        
        // Blend entre contour et texte
        float4 outlineColor = uniforms.outlineColor;
        outlineColor.a *= outlineAlpha;
        
        // Le texte est au-dessus du contour
        color = mix(outlineColor, color, alpha);
        color.a = max(color.a, outlineColor.a);
    }
    
    // Discard les pixels complètement transparents
    if (color.a < 0.01) {
        discard_fragment();
    }
    
    return color;
}
)";

SDFTextSystem::SDFTextSystem(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary, const std::string& resourcesPath)
: m_device(device)
{
    m_vertices.reserve(MAX_VERTICES);
    m_indices.reserve(MAX_INDICES);
    
    loadAtlasTexture(resourcesPath + "/atlas.png");
    loadFontMetrics(resourcesPath + "/metrics.csv");
    
    createPipelineState(pixelFormat, depthPixelFormat, shaderLibrary);
    createBuffers();
}

SDFTextSystem::~SDFTextSystem()
{
    if (m_pipelineState) m_pipelineState->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    if (m_uniformBuffer) m_uniformBuffer->release();
    if (m_atlasTexture) m_atlasTexture->release();
    if (m_sampler) m_sampler->release();
}

void SDFTextSystem::createPipelineState(MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
{
    NS::Error* error = nullptr;
    
    NS::SharedPtr<MTL::CompileOptions> compileOptions = NS::TransferPtr(MTL::CompileOptions::alloc()->init());
    shaderLibrary = m_device->newLibrary(NS::String::string(shaderSource, NS::UTF8StringEncoding), compileOptions.get(), &error);
    
    MTL::Function* vertexFunc = shaderLibrary->newFunction(NS::String::string("textVertexShader", NS::UTF8StringEncoding));
    MTL::Function* fragmentFunc = shaderLibrary->newFunction(NS::String::string("textFragmentShader", NS::UTF8StringEncoding));
    
    MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();
    
    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(0)->setOffset(0);
    vertexDesc->attributes()->object(0)->setBufferIndex(0);
    
    // TexCoord
    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(1)->setOffset(sizeof(simd::float2));
    vertexDesc->attributes()->object(1)->setBufferIndex(0);
    
    vertexDesc->layouts()->object(0)->setStride(sizeof(TextVertex));
    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setVertexFunction(vertexFunc);
    pipelineDesc->setFragmentFunction(fragmentFunc);
    pipelineDesc->setVertexDescriptor(vertexDesc);

    pipelineDesc->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    
    pipelineDesc->setDepthAttachmentPixelFormat(depthPixelFormat);
    
    pipelineDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
    pipelineDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    pipelineDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    pipelineDesc->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    pipelineDesc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    m_pipelineState = m_device->newRenderPipelineState(pipelineDesc, &error);
    
    if (!m_pipelineState)
        printf("Erreur création pipeline texte: %s\n", error->localizedDescription()->utf8String());
    
    // Sampler pour l'atlas
    MTL::SamplerDescriptor* samplerDesc = MTL::SamplerDescriptor::alloc()->init();
    samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMipFilter(MTL::SamplerMipFilterNotMipmapped);
    samplerDesc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDesc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    m_sampler = m_device->newSamplerState(samplerDesc);
    samplerDesc->release();
    
    vertexDesc->release();
    pipelineDesc->release();
    vertexFunc->release();
    fragmentFunc->release();
}

void SDFTextSystem::createBuffers()
{
    m_vertexBuffer = m_device->newBuffer(MAX_VERTICES * sizeof(TextVertex), MTL::ResourceStorageModeShared);
    
    m_indexBuffer = m_device->newBuffer(MAX_INDICES * sizeof(uint16_t), MTL::ResourceStorageModeShared);
    
    m_uniformBuffer = m_device->newBuffer(sizeof(TextUniforms), MTL::ResourceStorageModeShared);
}

void SDFTextSystem::loadAtlasTexture(const std::string& resourcesPath)
{
    int width, height, channels;
    unsigned char* data = stbi_load(resourcesPath.c_str(), &width, &height, &channels, 1);
    
    if (!data)
        printf("Erreur chargement atlas: %s\n", resourcesPath.c_str());
    
    m_atlasWidth = static_cast<float>(width);
    m_atlasHeight = static_cast<float>(height);
    
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::alloc()->init();
    texDesc->setTextureType(MTL::TextureType2D);
    texDesc->setPixelFormat(MTL::PixelFormatR8Unorm);  // Un seul canal pour SDF
    texDesc->setWidth(width);
    texDesc->setHeight(height);
    texDesc->setUsage(MTL::TextureUsageShaderRead);
    
    m_atlasTexture = m_device->newTexture(texDesc);
    texDesc->release();
    
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
    m_atlasTexture->replaceRegion(region, 0, data, width);
    
    stbi_image_free(data);
}

void SDFTextSystem::loadFontMetrics(const std::string& resourcesPath)
{
    std::ifstream file(resourcesPath);
    if (!file.is_open())
        printf("Erreur ouverture métriques: %s\n", resourcesPath.c_str());
    
    std::string line;

    if (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '='))
            iss >> m_baseSize;
    }
    
    // Format: codepoint,atlasX,atlasY,atlasW,atlasH,width,height,bearingX,bearingY,advance
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string token;
        std::vector<float> values;
        
        while (std::getline(iss, token, ','))
            values.push_back(std::stof(token));
        
        if (values.size() >= 10)
        {
            uint32_t codepoint = static_cast<uint32_t>(values[0]);
            GlyphInfo glyph;
            glyph.atlasX = values[1] / m_atlasWidth;
            glyph.atlasY = values[2] / m_atlasHeight;
            glyph.atlasWidth = values[3] / m_atlasWidth;
            glyph.atlasHeight = values[4] / m_atlasHeight;
            glyph.width = values[5];
            glyph.height = values[6];
            glyph.bearingX = values[7];
            glyph.bearingY = values[8];
            glyph.advance = values[9];
            
            m_glyphs[codepoint] = glyph;
        }
    }
    printf("Chargé %zu glyphes\n", m_glyphs.size());
}

std::vector<uint32_t> SDFTextSystem::utf8ToCodepoints(const std::string& text) const
{
    std::vector<uint32_t> codepoints;
    codepoints.reserve(text.size());
    
    for (size_t i = 0; i < text.size();) {
        uint32_t cp = 0;
        unsigned char c = text[i];
        
        if ((c & 0x80) == 0) {
            // ASCII
            cp = c;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2 bytes
            cp = (c & 0x1F) << 6;
            cp |= (text[i + 1] & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3 bytes
            cp = (c & 0x0F) << 12;
            cp |= (text[i + 1] & 0x3F) << 6;
            cp |= (text[i + 2] & 0x3F);
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4 bytes
            cp = (c & 0x07) << 18;
            cp |= (text[i + 1] & 0x3F) << 12;
            cp |= (text[i + 2] & 0x3F) << 6;
            cp |= (text[i + 3] & 0x3F);
            i += 4;
        } else {
            i += 1; // Skip invalid
        }
        
        codepoints.push_back(cp);
    }
    
    return codepoints;
}

// ============================================================================
// Génération du mesh pour une chaîne de texte
// ============================================================================
void SDFTextSystem::generateTextMesh(const std::string& text,
                                      float x, float y,
                                      const TextRenderOptions& options) {
    m_currentOptions = options;
    
    std::vector<uint32_t> codepoints = utf8ToCodepoints(text);
    float scale = options.scale * (m_baseSize / 64.0f);  // Normalisation
    
    // Calcul de l'offset pour l'alignement
    float totalWidth = 0;
    if (options.alignment != TextRenderOptions::Alignment::Left) {
        simd::float2 size = measureText(text, options.scale);
        totalWidth = size.x;
    }
    
    float offsetX = 0;
    if (options.alignment == TextRenderOptions::Alignment::Center) {
        offsetX = -totalWidth / 2.0f;
    } else if (options.alignment == TextRenderOptions::Alignment::Right) {
        offsetX = -totalWidth;
    }
    
    float cursorX = x + offsetX;
    float cursorY = y;
    
    for (uint32_t cp : codepoints) {
        // Gestion des retours à la ligne
        if (cp == '\n') {
            cursorX = x + offsetX;
            cursorY -= m_baseSize * scale;  // Descendre d'une ligne
            continue;
        }
        
        auto it = m_glyphs.find(cp);
        if (it == m_glyphs.end()) {
            // Caractère inconnu - utiliser espace ou placeholder
            it = m_glyphs.find(' ');
            if (it == m_glyphs.end()) continue;
        }
        
        const GlyphInfo& g = it->second;
        
        // Position du quad
        float px = cursorX + g.bearingX * scale;
        float py = cursorY - (g.height - g.bearingY) * scale;
        float pw = g.width * scale;
        float ph = g.height * scale;
        
        // Index de base pour ce quad
        uint16_t baseIndex = static_cast<uint16_t>(m_vertices.size());
        
        // 4 vertices par caractère
        m_vertices.push_back({{px,      py},      {g.atlasX,                  g.atlasY + g.atlasHeight}});
        m_vertices.push_back({{px + pw, py},      {g.atlasX + g.atlasWidth,   g.atlasY + g.atlasHeight}});
        m_vertices.push_back({{px + pw, py + ph}, {g.atlasX + g.atlasWidth,   g.atlasY}});
        m_vertices.push_back({{px,      py + ph}, {g.atlasX,                  g.atlasY}});
        
        // 6 indices (2 triangles)
        m_indices.push_back(baseIndex + 0);
        m_indices.push_back(baseIndex + 1);
        m_indices.push_back(baseIndex + 2);
        m_indices.push_back(baseIndex + 0);
        m_indices.push_back(baseIndex + 2);
        m_indices.push_back(baseIndex + 3);
        
        cursorX += g.advance * scale;
    }
}

// ============================================================================
// Mesure des dimensions d'un texte
// ============================================================================
simd::float2 SDFTextSystem::measureText(const std::string& text, float scale) const {
    std::vector<uint32_t> codepoints = utf8ToCodepoints(text);
    float adjustedScale = scale * (m_baseSize / 64.0f);
    
    float width = 0;
    float maxHeight = 0;
    float lineWidth = 0;
    int lineCount = 1;
    
    for (uint32_t cp : codepoints) {
        if (cp == '\n') {
            width = std::max(width, lineWidth);
            lineWidth = 0;
            lineCount++;
            continue;
        }
        
        auto it = m_glyphs.find(cp);
        if (it == m_glyphs.end()) {
            it = m_glyphs.find(' ');
            if (it == m_glyphs.end()) continue;
        }
        
        lineWidth += it->second.advance * adjustedScale;
        maxHeight = std::max(maxHeight, it->second.height * adjustedScale);
    }
    
    width = std::max(width, lineWidth);
    float totalHeight = maxHeight + (lineCount - 1) * m_baseSize * adjustedScale;
    
    return {width, totalHeight};
}

void SDFTextSystem::render(MTL::RenderCommandEncoder* encoder, float screenWidth, float screenHeight)
{
    if (m_vertices.empty()) return;
    
    memcpy(m_vertexBuffer->contents(), m_vertices.data(), m_vertices.size() * sizeof(TextVertex));
    memcpy(m_indexBuffer->contents(), m_indices.data(), m_indices.size() * sizeof(uint16_t));
    
    // orthographique
    TextUniforms uniforms;
    uniforms.projectionMatrix = simd::float4x4{
        simd::float4{2.0f / screenWidth, 0, 0, 0},
        simd::float4{0, 2.0f / screenHeight, 0, 0},
        simd::float4{0, 0, 1, 0},
        simd::float4{-1, -1, 0, 1} };
    uniforms.textColor = m_currentOptions.color;
    uniforms.smoothing = 0.15f;
    uniforms.thickness = m_currentOptions.thickness;
    uniforms.outlineWidth = m_currentOptions.outlineWidth;
    uniforms.outlineColor = m_currentOptions.outlineColor;
    
    memcpy(m_uniformBuffer->contents(), &uniforms, sizeof(TextUniforms));
    
    encoder->setRenderPipelineState(m_pipelineState);
    encoder->setVertexBuffer(m_vertexBuffer, 0, 0);
    encoder->setVertexBuffer(m_uniformBuffer, 0, 1);
    encoder->setFragmentBuffer(m_uniformBuffer, 0, 0);
    encoder->setFragmentTexture(m_atlasTexture, 0);
    encoder->setFragmentSamplerState(m_sampler, 0);
    
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, static_cast<NS::UInteger>(m_indices.size()), MTL::IndexTypeUInt16, m_indexBuffer, 0);
}

void SDFTextSystem::clearBatch()
{
    m_vertices.clear();
    m_indices.clear();
}

}
