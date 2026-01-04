//
//  RMDLColors.metal
//  Spammy
//
//  Created by Rémy on 04/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
};

vertex VertexOut vertexShader(uint vid [[vertex_id]])
{
    float2 positions[6] =
    {
        float2(-1, -1), float2(1, -1), float2(-1, 1),
        float2(-1, 1), float2(1, -1), float2(1, 1)
    };
    float2 texCoords[6] =
    {
        float2(0, 1), float2(1, 1), float2(0, 0),
        float2(0, 0), float2(1, 1), float2(1, 0)
    };

    VertexOut out;
    out.position = float4(positions[vid], 0, 1);
    out.texCoord = texCoords[vid];
    return out;
}

// Fragment shader pour post-processing avec couleurs vives
struct ColorGradingUniforms
{
    float saturation;
    float brightness;
    float contrast;
    float bloomIntensity;
};

// Conversion RGB vers HSV pour ajuster la saturation
float3 rgb2hsv(float3 c)
{
    float4 K = float4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    float4 p = mix(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = mix(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float3 hsv2rgb(float3 c)
{
    float4 K = float4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// Tone mapping ACES (cinématique)
float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

fragment float4 postProcessFragment(VertexOut in [[stage_in]],
                                    texture2d<float> hdrTexture [[texture(0)]],
                                    texture2d<float> bloomTexture [[texture(1)]],
                                    constant ColorGradingUniforms& uniforms [[buffer(0)]])
{
    constexpr sampler texSampler(mag_filter::linear, min_filter::linear);
    
    // Échantillonner la texture HDR
    float3 hdrColor = hdrTexture.sample(texSampler, in.texCoord).rgb;
    
    // Ajouter le bloom
    float3 bloom = bloomTexture.sample(texSampler, in.texCoord).rgb;
    hdrColor += bloom * uniforms.bloomIntensity;
    
    // Ajuster la luminosité
    hdrColor *= uniforms.brightness;
    
    // Ajuster le contraste
    hdrColor = (hdrColor - 0.5) * uniforms.contrast + 0.5;
    
    // Ajuster la saturation en HSV
    float3 hsv = rgb2hsv(hdrColor);
    hsv.y *= uniforms.saturation;  // Augmenter la saturation
    hdrColor = hsv2rgb(hsv);
    
    // Tone mapping ACES pour ramener HDR vers [0,1]
    float3 finalColor = ACESFilm(hdrColor);
    
    // Correction gamma (optionnel mais recommandé)
    finalColor = pow(finalColor, float3(1.0/2.2));
    
    return float4(finalColor, 1.0);
}

// Shader simple pour extraire les zones lumineuses (bloom)
fragment float4 bloomExtractFragment(VertexOut in [[stage_in]],
                                     texture2d<float> sourceTexture [[texture(0)]])
{
    constexpr sampler texSampler(mag_filter::linear, min_filter::linear);
    float3 color = sourceTexture.sample(texSampler, in.texCoord).rgb;
    
    // Extraire seulement les couleurs très lumineuses
    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    float threshold = 1.0;
    if (brightness > threshold)
        return float4(color * (brightness - threshold), 1.0);
    return float4(0.0);
}
