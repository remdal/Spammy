//
//  RMDLCards.hpp
//  Spammy
//
//  Created by Rémy on 14/03/2026.
//

#ifndef RMDLCards_hpp
#define RMDLCards_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <string>

struct HoloCardVertexData
{
    simd::float3 position;
    simd::float2 uv;
    simd::float3 normal;
    simd::float3 tangent;
};

struct HoloCardUniforms
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjMatrix;
    simd::float3   cameraPos;
    float          time;
    simd::float2   tilt;
    float          refractionStrength;
    float          chromaticAberration;
    float          fresnelPower;
    float          glassOpacity;
    float          holoIntensity;
    float          holoScale;
    float          holoShift;
    float          sparkleIntensity;
    simd::float3   lightDir;
    float          glareIntensity;
    float          glareShininess;
    simd::float2   screenSize;
    simd::float2   _pad;
};

struct HoloCardProfile
{
    float holoIntensity      = 0.5f;
    float glassOpacity       = 0.15f;
    float refractionStrength = 0.015f;
    float sparkleIntensity   = 0.35f;
    float glareIntensity     = 0.5f;

    static HoloCardProfile Common()  { return {0.15f, 0.07f, 0.007f, 0.10f, 0.25f}; }
    static HoloCardProfile Rare()    { return {0.50f, 0.15f, 0.015f, 0.35f, 0.50f}; }
    static HoloCardProfile Secret()  { return {0.90f, 0.25f, 0.025f, 0.70f, 0.80f}; }
};

class HoloCardRenderer
{
public:
    HoloCardRenderer(MTL::Device* device, MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* shaderLibrary, const std::string& resourcePath);
    ~HoloCardRenderer();

    // Charge l'artwork de la carte (utilise ton loadTexture habituel en interne)
    void loadArtwork(const std::string& path);

    void update(float deltaTime);

    // Appelé depuis ta draw() principale, APRÈS avoir rendu la scène
    void draw(MTL::RenderCommandEncoder* encoder,
              MTL::Texture*              sceneTexture,
              const simd::float4x4&     viewProj,
              const simd::float4x4&     model,
              NS::UInteger width, NS::UInteger heigth);

    // Tilt depuis gyro (iOS) ou souris (macOS), normalisé [-1..1]
    void setTilt(simd::float2 tilt);

    // Applique un profil de rareté d'un coup
    void applyProfile(const HoloCardProfile& profile);

    // Accès direct aux paramètres (comme tes Blender.transform etc.)
    simd::float3   cameraPosition    = {0, 0, 3};
    simd::float3   lightDirection    = {-0.5f, 1.0f, 0.8f};
    float          holoIntensity     = 0.5f;
    float          glassOpacity      = 0.15f;
    float          refractionStrength= 0.015f;
    float          sparkleIntensity  = 0.35f;
    float          glareIntensity    = 0.5f;

private:
    MTL::Device*              m_device        = nullptr;
    MTL::RenderPipelineState* m_pipeline      = nullptr;
    MTL::DepthStencilState*   m_depthState    = nullptr;
    MTL::SamplerState*        m_linearSampler = nullptr;
    MTL::SamplerState*        m_clampSampler  = nullptr;
    MTL::Buffer*              m_vertexBuffer  = nullptr;
    MTL::Buffer*              m_indexBuffer   = nullptr;
    MTL::Buffer*              m_uniformBuffer = nullptr;  // ring buffer x3
    MTL::Texture*             m_artwork       = nullptr;

    float          m_time       = 0.f;
    simd::float2   m_tilt       = {0.f, 0.f};
    NS::UInteger   m_frameIndex = 0;

    void createPipeline(MTL::Library* lib, MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt);
    void createBuffers();
    void createSamplers();
};

#endif /* RMDLCards_hpp */
