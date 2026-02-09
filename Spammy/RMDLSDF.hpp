//
//  RMDLSDF.hpp
//  Spammy
//
//  Created by Rémy on 09/02/2026.
//

#ifndef RMDLSDF_hpp
#define RMDLSDF_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace TextRendering {

struct GlyphInfo
{   // Position dans l'atlas (normalisée 0-1)
    float atlasX;
    float atlasY;
    float atlasWidth;
    float atlasHeight;
    
    // Métriques du glyphe (en pixels à la taille de base)
    float width;
    float height;
    float bearingX;    // Offset horizontal depuis le curseur
    float bearingY;    // Offset vertical depuis la baseline
    float advance;     // Avancement horizontal pour le prochain caractère
};

struct TextVertex
{
    simd::float2 position;
    simd::float2 texCoord;
    simd::float4 color;
    float thickness; // Épaisseur (0.5 = normal)
    float outlineWidth; // 0 = pas de contour
    simd::float4 outlineColor;
};


struct TextUniforms
{
    simd::float4x4 projectionMatrix;
    float smoothing; // anti-aliasing (typiquement 0.1-0.25)
    float padding;
};

struct TextRenderOptions
{
    simd::float4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    float scale = 1.0f;
    float thickness = 0.5f;           // 0.3 = fin, 0.5 = normal, 0.7 = gras
    float outlineWidth = 0.0f;        // 0 = pas de contour
    simd::float4 outlineColor = {0.6f, 0.6f, 0.0f, 1.0f};
    
    enum class Alignment { Left, Center, Right };
    Alignment alignment = Alignment::Left;
};

class SDFTextSystem
{
public:
    SDFTextSystem(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary, const std::string& resourcesPath);
    ~SDFTextSystem();
    
    void generateTextMesh(const std::string& text, float x, float y, const TextRenderOptions& options = {});
    
    void render(MTL::RenderCommandEncoder* encoder, float screenWidth, float screenHeight);
    
    void clearBatch();

    simd::float2 measureText(const std::string& text, float scale = 1.0f) const;

    MTL::Texture* getAtlasTexture() const { return m_atlasTexture; }
    
private:
    void createPipelineState(MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    void createBuffers();
    void loadAtlasTexture(const std::string& resourcesPath);
    void loadFontMetrics(const std::string& resourcesPath);
    void updateBuffers();
    
    std::vector<uint32_t> utf8ToCodepoints(const std::string& text) const;
    
private:
    MTL::Device* m_device;
    MTL::RenderPipelineState* m_pipelineState = nullptr;
    MTL::Buffer* m_vertexBuffer = nullptr;
    MTL::Buffer* m_indexBuffer = nullptr;
    MTL::Buffer* m_uniformBuffer = nullptr;
    MTL::Texture* m_atlasTexture = nullptr;
    MTL::SamplerState* m_sampler = nullptr;
    
    std::unordered_map<uint32_t, GlyphInfo> m_glyphs;
    float m_baseSize = 96.0f;
    float m_atlasWidth = 1024.0f;
    float m_atlasHeight = 1024.0f;
    
    std::vector<TextVertex> m_vertices;
    std::vector<uint16_t> m_indices;
    
    TextRenderOptions m_currentOptions;
    
    static constexpr size_t MAX_CHARACTERS = 4096;
    static constexpr size_t MAX_VERTICES = MAX_CHARACTERS * 4;
    static constexpr size_t MAX_INDICES = MAX_CHARACTERS * 6;
};

}

#endif /* RMDLSDF_hpp */
