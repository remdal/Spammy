//
//  RMDLSkybox.metal
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//

#include <metal_stdlib>
using namespace metal;

#include "RMDLMainRenderer_shared.h"

struct Vertex
{
    float3 position [[attribute(0)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 viewRay;
};

// Hash function pour gradient
float2 hash22(float2 p)
{
    float3 p3 = fract(float3(p.xyx) * float3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy) * 2.0 - 1.0;
}

float3 hash33(float3 p)
{
    p = float3(dot(p, float3(127.1, 311.7, 74.7)),
               dot(p, float3(269.5, 183.3, 246.1)),
               dot(p, float3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453123) * 2.0 - 1.0;
}

float quintic(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float3 quintic3(float3 t)
{
    return float3(quintic(t.x), quintic(t.y), quintic(t.z));
}

float perlinNoise2D(float2 p)
{
    float2 i = floor(p);
    float2 f = fract(p);
    
    // Gradients aux 4 coins
    float2 g00 = hash22(i + float2(0.0, 0.0));
    float2 g10 = hash22(i + float2(1.0, 0.0));
    float2 g01 = hash22(i + float2(0.0, 1.0));
    float2 g11 = hash22(i + float2(1.0, 1.0));
    
    // Distances aux coins
    float v00 = dot(g00, f - float2(0.0, 0.0));
    float v10 = dot(g10, f - float2(1.0, 0.0));
    float v01 = dot(g01, f - float2(0.0, 1.0));
    float v11 = dot(g11, f - float2(1.0, 1.0));
    
    // Interpolation smooth
    float2 u = float2(quintic(f.x), quintic(f.y));
    
    float v0 = mix(v00, v10, u.x);
    float v1 = mix(v01, v11, u.x);
    
    return mix(v0, v1, u.y);
}

float perlinNoise3DD(float3 p)
{
    float3 i = floor(p);
    float3 f = fract(p);
    
    // 8 coins du cube
    float3 g000 = hash33(i + float3(0,0,0));
    float3 g100 = hash33(i + float3(1,0,0));
    float3 g010 = hash33(i + float3(0,1,0));
    float3 g110 = hash33(i + float3(1,1,0));
    float3 g001 = hash33(i + float3(0,0,1));
    float3 g101 = hash33(i + float3(1,0,1));
    float3 g011 = hash33(i + float3(0,1,1));
    float3 g111 = hash33(i + float3(1,1,1));
    
    float v000 = dot(g000, f - float3(0,0,0));
    float v100 = dot(g100, f - float3(1,0,0));
    float v010 = dot(g010, f - float3(0,1,0));
    float v110 = dot(g110, f - float3(1,1,0));
    float v001 = dot(g001, f - float3(0,0,1));
    float v101 = dot(g101, f - float3(1,0,1));
    float v011 = dot(g011, f - float3(0,1,1));
    float v111 = dot(g111, f - float3(1,1,1));
    
    // Interpolation trilinéaire smooth
    float3 u = quintic3(f);
    
    float v00 = mix(v000, v100, u.x);
    float v10 = mix(v010, v110, u.x);
    float v01 = mix(v001, v101, u.x);
    float v11 = mix(v011, v111, u.x);
    
    float v0 = mix(v00, v10, u.y);
    float v1 = mix(v01, v11, u.y);
    
    return mix(v0, v1, u.z);
}

// Fractional Brownian Motion default; scale: -0.2, detail: 2, roughness: 0.5, distortion: 33.6, lacunarity: 2
float perlinFBM(float2 uv)
{
    const float SCALE = -0.2;
    const int DETAIL = 2;
    const float ROUGHNESS = 0.5;
    const float DISTORTION = 33.6;
    const float LACUNARITY = 2.0;
    
    float2 p = uv * SCALE;
    
    // Distorsion initiale
    p += float2(perlinNoise2D(p + float2(DISTORTION, 0.0)), perlinNoise2D(p + float2(0.0, DISTORTION))) * 0.1;
    
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < DETAIL; ++i)
    {
        value += perlinNoise2D(p * frequency) * amplitude;
        maxValue += amplitude;

        amplitude *= ROUGHNESS;
        frequency *= LACUNARITY;
    }
    return value / maxValue;
}

float2 raySphereIntersect(float3 ro, float3 rd, float r)
{
    float b = dot(ro, rd);
    float c = dot(ro, ro) - r * r;
    float d = b * b - c;
    if (d < 0.0) return float2(-1.0);
    d = sqrt(d);
    return float2(-b - d, -b + d);
}

float phaseRayleigh(float cosTheta)
{
    return (3.0 / (16.0 * M_PI_F)) * (1.0 + cosTheta * cosTheta);
}

float phaseMie(float cosTheta, float g)
{
    float g2 = g * g;
    float nom = (1.0 - g2);
    float denom = 4.0 * M_PI_F * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5);
    return nom / denom;
}

// PERLIN 3D COLOR - Génère RGB directement
float3 perlinNoiseColor3D(float3 p)
{
    float3 i = floor(p);
    float3 f = fract(p);
    
    // 8 coins avec hash RGB
    float3 c000 = hash33(i + float3(0,0,0));
    float3 c100 = hash33(i + float3(1,0,0));
    float3 c010 = hash33(i + float3(0,1,0));
    float3 c110 = hash33(i + float3(1,1,0));
    float3 c001 = hash33(i + float3(0,0,1));
    float3 c101 = hash33(i + float3(1,0,1));
    float3 c011 = hash33(i + float3(0,1,1));
    float3 c111 = hash33(i + float3(1,1,1));
    
    // Interpolation smooth RGB
    float3 u = quintic3(f);
    
    float3 c00 = mix(c000, c100, u.x);
    float3 c10 = mix(c010, c110, u.x);
    float3 c01 = mix(c001, c101, u.x);
    float3 c11 = mix(c011, c111, u.x);
    
    float3 c0 = mix(c00, c10, u.y);
    float3 c1 = mix(c01, c11, u.y);
    
    return mix(c0, c1, u.z);
}

// FBM 3D avec COLOR output
float3 perlinFBMColor3D(float3 pos)
{
    const float SCALE = -0.85;
    const int DETAIL = 2;
    const float ROUGHNESS = 0.5;
    float DISTORTION = 0.0;
    const float LACUNARITY = 2.0;
    
    float3 p = pos * SCALE;

    // Domain warping (distortion)
    float3 q = float3(perlinNoise3DD(p + float3(DISTORTION, 0.0, 0.0)),
                      perlinNoise3DD(p + float3(0.0, DISTORTION, 0.0)),
                      perlinNoise3DD(p + float3(0.0, 0.0, DISTORTION)));
    p += q / 0.05;

    // Accumulation RGB FBM
    float3 value = float3(0.0);
    float amplitude = 1.0;
    float frequency = 15.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < DETAIL; ++i)
    {
        // Chaque octave a ses propres couleurs RGB
        value += perlinNoiseColor3D(p * frequency) * amplitude;
        maxValue += amplitude;
        
        amplitude *= ROUGHNESS;
        frequency *= LACUNARITY;
    }
    
    return value / maxValue;
}

float3 atmosphericScattering(float3 ro, float3 rd, float3 sunDir, constant RMDLSkyboxUniforms& u)
{
    float2 atmoHit = raySphereIntersect(ro, rd, u.atmosphereRadius);
    if (atmoHit.x < 0.0) return float3(0.0);
    
    float tMax = atmoHit.y;
    int steps = 32;
    float dt = tMax / float(steps);
    
    float3 totalRayleigh = float3(0.0);
    float3 totalMie = float3(0.0);
    float opticalDepthR = 0.0;
    float opticalDepthM = 0.0;
    
    for (int i = 0; i < steps; ++i) {
        float t = dt * (float(i) + 0.5);
        float3 pos = ro + rd * t;
        float height = length(pos) - u.planetRadius;
        
        float hr = exp(-height / u.rayleighHeight) * dt;
        float hm = exp(-height / u.mieHeight) * dt;
        
        opticalDepthR += hr;
        opticalDepthM += hm;
        
        float2 sunHit = raySphereIntersect(pos, sunDir, u.atmosphereRadius);
        float tSun = sunHit.y;
        float odLightR = 0.0;
        float odLightM = 0.0;
        
        int lightSteps = 8;
        float dtLight = tSun / float(lightSteps);
        
        for (int j = 0; j < lightSteps; ++j) {
            float tl = dtLight * (float(j) + 0.5);
            float3 posL = pos + sunDir * tl;
            float hL = length(posL) - u.planetRadius;
            odLightR += exp(-hL / u.rayleighHeight) * dtLight;
            odLightM += exp(-hL / u.mieHeight) * dtLight;
        }
        
        float3 tau = u.rayleighCoeff * (opticalDepthR + odLightR) +
                    u.mieCoeff * (opticalDepthM + odLightM);
        float3 attenuation = exp(-tau);
        
        totalRayleigh += hr * attenuation;
        totalMie += hm * attenuation;
    }
    
    float cosTheta = dot(rd, sunDir);
    float phaseR = phaseRayleigh(cosTheta);
    float phaseM = phaseMie(cosTheta, u.mieG);
    
    float3 scatter = u.sunIntensity * (
        phaseR * u.rayleighCoeff * totalRayleigh +
        phaseM * u.mieCoeff * totalMie
    );
    
    return scatter;
}

vertex VertexOut skybox_vertex(Vertex in [[stage_in]],
                               constant RMDLSkyboxUniforms& uniforms [[buffer(1)]])
{
    VertexOut out;
    out.position = float4(in.position.xy, 0.999, 1.0);
    float4 farPoint = uniforms.invViewProjection * float4(in.position.xy, 1.0, 1.0);
    out.viewRay = farPoint.xyz / farPoint.w - uniforms.cameraPos;
    return out;
}

fragment float4 skybox_fragment(VertexOut in [[stage_in]],
                                constant RMDLSkyboxUniforms& uniforms [[buffer(1)]])
{
    float3 rd = normalize(in.viewRay);
    float3 color = perlinFBMColor3D(rd);
    // effet pastel
    color = color * 0.5 + 0.7;
    // saturation
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    color = mix(float3(gray), color, 1.3);
    color = saturate(color);
    // Gamma
    color = pow(color, float3(1.0 / 2.5));
    float sunDot = dot(rd, uniforms.sunDir);
    if (sunDot > 0.9995) {
        color += float3(1.0) * uniforms.sunIntensity * 0.1;
    }
    float sunHalo = pow(max(0.0, sunDot), 32.0);
    color += float3(1.0, 0.9, 0.7) * sunHalo * 0.3;
    

    return float4(color, 1.0);
}

//    // Base atmospheric scattering
//    float3 color = atmosphericScattering(ro, rd, uniforms.sunDir, uniforms);
//    
//    // Perlin noise overlay (pour texture atmosphérique)
//    // Projette le ray sur une sphère pour UV stables
//    float2 sphereUV = float2(
//        atan2(rd.z, rd.x) / (2.0 * M_PI_F) + 0.5,
//        asin(rd.y) / M_PI_F + 0.5
//    );
//    
//    // Génère noise avec tes params
//    float noise = perlinFBM(sphereUV * 10.0 + uniforms.timeOfDay * 0.5);
//    
//    // Applique le noise comme texture atmosphérique (subtle)
//    // Plus visible près de l'horizon
//    float horizonMask = 1.0 - abs(rd.y);
//    color += float3(noise) * 0.05 * horizonMask;
//    // Tone mapping (ACES approximation)
//    color = color / (color + float3(1.0));
//    
//    // Exposure
//    color *= uniforms.exposure;
//
//    return float4(color, 1.0);
//}

//fragment float4 skyboxragment(VertexOut in [[stage_in]],
//                                constant RMDLSkyboxUniforms& uniforms [[buffer(1)]])
//{
//    float3 rd = normalize(in.viewRay);
//    
//    // ========================================================================
//    // OPTION 1: Skybox Perlin 3D Pure (comme Blender)
//    // ========================================================================
//    
//    // Utilise direction du ray comme position 3D
//    float3 samplePos = rd * 10.0; // Scale pour contrôler la fréquence
//    
//    // Sample le Perlin 3D avec animation temporelle
//    float noise = perlinFBM3D(samplePos + float3(0, uniforms.timeOfDay * 2.0, 0));
//    
//    // Remap [-1,1] vers [0,1]
//    noise = noise * 0.5 + 0.5;
//    
//    // Colorisation (gradient bleu -> blanc -> orange)
//    float3 color;
//    if (noise < 0.4) {
//        // Zones sombres = bleu profond
//        color = mix(float3(0.1, 0.2, 0.4), float3(0.3, 0.5, 0.8), noise / 0.4);
//    } else if (noise < 0.6) {
//        // Milieu = blanc/gris
//        color = mix(float3(0.3, 0.5, 0.8), float3(0.9, 0.9, 0.95), (noise - 0.4) / 0.2);
//    } else {
//        // Zones claires = orange/jaune
//        color = mix(float3(0.9, 0.9, 0.95), float3(1.0, 0.8, 0.5), (noise - 0.6) / 0.4);
//    }
//    
//    // ========================================================================
//    // OPTION 2: Atmospheric + Perlin Clouds (mix des deux)
//    // Décommente si tu veux garder l'atmosphère réaliste
//    // ========================================================================
//    /*
//    float3 ro = uniforms.cameraPos + float3(0, uniforms.planetRadius, 0);
//    float3 baseColor = atmosphericScattering(ro, rd, uniforms.sunDir, uniforms);
//    
//    // Perlin 3D pour nuages volumétriques
//    float3 cloudPos = rd * 5.0 + float3(uniforms.timeOfDay * 0.5, 0, 0);
//    float cloudNoise = perlinFBM3D(cloudPos);
//    cloudNoise = saturate(cloudNoise * 0.5 + 0.5);
//    
//    // Mix atmosphère + nuages
//    float cloudDensity = pow(cloudNoise, 2.0); // Contraste
//    color = mix(baseColor, float3(1.0), cloudDensity * 0.7);
//
//    // Sun disk
//    float sunDot = dot(rd, uniforms.sunDir);
//    if (sunDot > 0.9995) {
//        color += float3(1.0) * uniforms.sunIntensity * 0.1;
//    }
//    float sunHalo = pow(max(0.0, sunDot), 32.0);
//    color += float3(1.0, 0.9, 0.7) * sunHalo * 0.3;
