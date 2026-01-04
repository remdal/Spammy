//
//  RMDLColors.hpp
//  Spammy
//
//  Created by Rémy on 04/01/2026.
//

#ifndef RMDLColors_hpp
#define RMDLColors_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>

struct ColorGradingUniforms
{
    float saturation;      // 1.0 = normal, 1.3 = plus saturé
    float brightness;      // 1.0 = normal
    float contrast;        // 1.0 = normal
    float bloomIntensity;  // Force du bloom
};

class VibrantColorRenderer
{
public:
    VibrantColorRenderer(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~VibrantColorRenderer();
    
    void setupRenderTargets(MTL::Device* device, MTL::PixelFormat pixelFormat);
    void setupPipelines();
    void render(MTL::RenderCommandEncoder* encoder);
    // Post-processing pour tone mapping et effets
    void renderPostProcess(MTL::RenderCommandEncoder* encoder, MTL::Texture* finalTarget);

    MTL::RenderPipelineState*   m_renderPipelineState;
    MTL::RenderPipelineState*   m_postProcessrenderPipelineState;
    MTL::Texture*               m_hdrRenderTarget;
    MTL::Texture*               m_bloomTexture;
};
// ============================================
// PALETTE DE COULEURS VIVES RECOMMANDÉES
// ============================================
namespace VibrantPalette {
    // Couleurs en format HDR (peuvent dépasser 1.0)
    const simd::float4 ELECTRIC_BLUE    = {0.1f, 0.5f, 3.0f, 1.0f};
    const simd::float4 HOT_PINK         = {3.0f, 0.2f, 1.5f, 1.0f};
    const simd::float4 NEON_GREEN       = {0.5f, 3.0f, 0.3f, 1.0f};
    const simd::float4 BRIGHT_ORANGE    = {3.0f, 1.2f, 0.1f, 1.0f};
    const simd::float4 VIVID_PURPLE     = {2.0f, 0.2f, 2.5f, 1.0f};
    const simd::float4 CYBER_YELLOW     = {3.0f, 2.8f, 0.2f, 1.0f};
    const simd::float4 LASER_RED        = {2.5f, 0.1f, 0.1f, 1.0f};
    const simd::float4 MINT_CYAN        = {0.3f, 2.5f, 2.2f, 1.0f};
    // Pour les éléments moins importants (valeurs normales)
    const simd::float4 DEEP_PURPLE      = {0.3f, 0.1f, 0.5f, 1.0f};
    const simd::float4 DARK_BLUE        = {0.1f, 0.2f, 0.5f, 1.0f};
}

#endif /* RMDLColors_hpp */
