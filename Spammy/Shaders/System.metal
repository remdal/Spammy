//
//  System.metal
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include <metal_stdlib>
using namespace metal;

#include "../RMDLMainRenderer_shared.h" // RMDLCameraUniforms
#import "Helpers.metal" // PBR functions & Noises

// COMMON STRUCTURES

struct FullscreenVertexOut
{
    float4 position [[position]];
    float2 uv;
};

struct PostProcessParams
{
    float exposure;
    float gamma;
    float bloomIntensity;
    float bloomThreshold;
    float vignetteIntensity;
    float vignetteRadius;
    float chromaticAberration;
    float filmGrain;
    float2 screenSize;
    float time;
    float _pad;
};

// ============================================
// FULLSCREEN VERTEX SHADER
// ============================================

vertex FullscreenVertexOut fullscreen_vertex(uint vertexID [[vertex_id]]) {
    FullscreenVertexOut out;
    
    // Fullscreen triangle
    out.uv = float2((vertexID << 1) & 2, vertexID & 2);
    out.position = float4(out.uv * 2.0 - 1.0, 0.0, 1.0);
    out.uv.y = 1.0 - out.uv.y;
    
    return out;
}

// ============================================
// TONE MAPPING FUNCTIONS
// ============================================

// ACES Filmic Tone Mapping
float3 acesFilm(float3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Reinhard
float3 reinhard(float3 x) {
    return x / (x + 1.0);
}

// Uncharted 2
float3 uncharted2Tonemap(float3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 uncharted2(float3 color) {
    float W = 11.2;
    float exposureBias = 2.0;
    float3 curr = uncharted2Tonemap(exposureBias * color);
    float3 whiteScale = 1.0 / uncharted2Tonemap(float3(W));
    return curr * whiteScale;
}

// ============================================
// BLOOM THRESHOLD KERNEL
// ============================================

kernel void bloomThreshold(
    texture2d<float, access::read> input [[texture(0)]],
    texture2d<float, access::write> output [[texture(1)]],
    constant PostProcessParams& params [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]]
) {
    float4 color = input.read(gid);
    
    float brightness = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    
    if (brightness > params.bloomThreshold) {
        float excess = brightness - params.bloomThreshold;
        float factor = excess / (excess + 1.0);
        output.write(float4(color.rgb * factor, 1.0), gid);
    } else {
        output.write(float4(0.0, 0.0, 0.0, 1.0), gid);
    }
}

// ============================================
// GAUSSIAN BLUR KERNELS
// ============================================

constant float gaussianWeights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

kernel void bloomBlurHorizontal(
    texture2d<float, access::read> input [[texture(0)]],
    texture2d<float, access::write> output [[texture(1)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint width = input.get_width();
    float3 result = input.read(gid).rgb * gaussianWeights[0];
    
    for (int i = 1; i < 5; i++) {
        uint2 offsetLeft = uint2(max(int(gid.x) - i, 0), gid.y);
        uint2 offsetRight = uint2(min(gid.x + i, width - 1), gid.y);
        
        result += input.read(offsetLeft).rgb * gaussianWeights[i];
        result += input.read(offsetRight).rgb * gaussianWeights[i];
    }
    
    output.write(float4(result, 1.0), gid);
}

kernel void bloomBlurVertical(
    texture2d<float, access::read> input [[texture(0)]],
    texture2d<float, access::write> output [[texture(1)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint height = input.get_height();
    float3 result = input.read(gid).rgb * gaussianWeights[0];
    
    for (int i = 1; i < 5; i++) {
        uint2 offsetDown = uint2(gid.x, max(int(gid.y) - i, 0));
        uint2 offsetUp = uint2(gid.x, min(gid.y + i, height - 1));
        
        result += input.read(offsetDown).rgb * gaussianWeights[i];
        result += input.read(offsetUp).rgb * gaussianWeights[i];
    }
    
    output.write(float4(result, 1.0), gid);
}

// ============================================
// FINAL COMPOSITE FRAGMENT SHADER
// ============================================

fragment float4 postprocess_composite(
    FullscreenVertexOut in [[stage_in]],
    texture2d<float> sceneTexture [[texture(0)]],
    texture2d<float> bloomTexture [[texture(1)]],
    texture2d<float> depthTexture [[texture(2)]],
    constant PostProcessParams& params [[buffer(0)]],
    sampler texSampler [[sampler(0)]]
) {
    float2 uv = in.uv;
    
    // Chromatic aberration
    float2 direction = uv - 0.5;
    float dist = length(direction) * params.chromaticAberration;
    
    float3 color;
    color.r = sceneTexture.sample(texSampler, uv + direction * dist * 0.01).r;
    color.g = sceneTexture.sample(texSampler, uv).g;
    color.b = sceneTexture.sample(texSampler, uv - direction * dist * 0.01).b;
    
    // Add bloom
    float3 bloom = bloomTexture.sample(texSampler, uv).rgb;
    color += bloom * params.bloomIntensity;
    
    // Exposure
    color *= params.exposure;
    
    // Tone mapping (ACES)
    color = acesFilm(color);
    
    // Gamma correction
    color = pow(color, float3(1.0 / params.gamma));
    
    // Vignette
    float vignette = 1.0 - smoothstep(params.vignetteRadius, params.vignetteRadius + 0.3,
                                       length(uv - 0.5) * 1.5);
    color *= mix(1.0 - params.vignetteIntensity, 1.0, vignette);
    
    // Film grain
    if (params.filmGrain > 0.0) {
        float noise = fract(sin(dot(uv * params.time, float2(12.9898, 78.233))) * 43758.5453);
        color += (noise - 0.5) * params.filmGrain * 0.1;
    }
    
    return float4(color, 1.0);
}

// ============================================
// FXAA
// ============================================

fragment float4 fxaa_fragment(
    FullscreenVertexOut in [[stage_in]],
    texture2d<float> inputTexture [[texture(0)]],
    constant float2& texelSize [[buffer(0)]],
    sampler texSampler [[sampler(0)]]
) {
    float2 uv = in.uv;
    
    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0 / 8.0;
    float FXAA_REDUCE_MIN = 1.0 / 128.0;
    
    float3 rgbNW = inputTexture.sample(texSampler, uv + float2(-1.0, -1.0) * texelSize).rgb;
    float3 rgbNE = inputTexture.sample(texSampler, uv + float2(1.0, -1.0) * texelSize).rgb;
    float3 rgbSW = inputTexture.sample(texSampler, uv + float2(-1.0, 1.0) * texelSize).rgb;
    float3 rgbSE = inputTexture.sample(texSampler, uv + float2(1.0, 1.0) * texelSize).rgb;
    float3 rgbM = inputTexture.sample(texSampler, uv).rgb;
    
    float3 luma = float3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    float2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(float2(FXAA_SPAN_MAX), max(float2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * texelSize;
    
    float3 rgbA = 0.5 * (
        inputTexture.sample(texSampler, uv + dir * (1.0 / 3.0 - 0.5)).rgb +
        inputTexture.sample(texSampler, uv + dir * (2.0 / 3.0 - 0.5)).rgb
    );
    
    float3 rgbB = rgbA * 0.5 + 0.25 * (
        inputTexture.sample(texSampler, uv + dir * -0.5).rgb +
        inputTexture.sample(texSampler, uv + dir * 0.5).rgb
    );
    
    float lumaB = dot(rgbB, luma);
    
    if (lumaB < lumaMin || lumaB > lumaMax) {
        return float4(rgbA, 1.0);
    } else {
        return float4(rgbB, 1.0);
    }
}

// ============================================
// ATMOSPHERIC SCATTERING
// ============================================

fragment float4 atmosphere_fragment(
    FullscreenVertexOut in [[stage_in]],
    texture2d<float> sceneTexture [[texture(0)]],
    texture2d<float> depthTexture [[texture(1)]],
    constant float4x4& invViewProj [[buffer(0)]],
    constant float3& cameraPos [[buffer(1)]],
    constant float3& sunDirection [[buffer(2)]],
    sampler texSampler [[sampler(0)]]
) {
    float2 uv = in.uv;
    float3 sceneColor = sceneTexture.sample(texSampler, uv).rgb;
    float depth = depthTexture.sample(texSampler, uv).r;
    
    // Reconstruct world position
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    clipPos.y = -clipPos.y;
    float4 worldPos = invViewProj * clipPos;
    worldPos /= worldPos.w;
    
    float3 viewDir = normalize(worldPos.xyz - cameraPos);
    float distance = length(worldPos.xyz - cameraPos);
    
    // Sky color
    float3 skyColorZenith = float3(0.2, 0.4, 0.8);
    float3 skyColorHorizon = float3(0.6, 0.75, 0.9);
    
    float sunDot = max(dot(viewDir, -sunDirection), 0.0);
    
    // Rayleigh scattering approximation
    float3 rayleigh = float3(0.5, 0.7, 1.0);
    float rayleighPhase = 0.75 * (1.0 + sunDot * sunDot);
    
    // Mie scattering (sun glow)
    float miePhase = pow(sunDot, 8.0) * 0.5;
    float3 sunColor = float3(1.0, 0.9, 0.7);
    
    // Height fog
    float fogDensity = 0.0003;
    float fogFactor = 1.0 - exp(-distance * fogDensity);
    
    // Apply fog based on depth
    if (depth < 1.0) {
        float3 fogColor = mix(skyColorHorizon, skyColorZenith * rayleigh * rayleighPhase, 0.5);
        fogColor += sunColor * miePhase;
        sceneColor = mix(sceneColor, fogColor, saturate(fogFactor));
    } else {
        // Sky
        float horizonFactor = pow(1.0 - abs(viewDir.y), 3.0);
        float3 skyColor = mix(skyColorZenith, skyColorHorizon, horizonFactor);
        skyColor += sunColor * miePhase * 2.0;
        sceneColor = skyColor;
    }
    
    return float4(sceneColor, 1.0);
}

// ============================================
// DEPTH OF FIELD
// ============================================

fragment float4 dof_fragment(
    FullscreenVertexOut in [[stage_in]],
    texture2d<float> sceneTexture [[texture(0)]],
    texture2d<float> blurredTexture [[texture(1)]],
    texture2d<float> depthTexture [[texture(2)]],
    constant float& focusDistance [[buffer(0)]],
    constant float& focusRange [[buffer(1)]],
    constant float& maxBlur [[buffer(2)]],
    sampler texSampler [[sampler(0)]]
) {
    float2 uv = in.uv;
    
    float3 sharp = sceneTexture.sample(texSampler, uv).rgb;
    float3 blurred = blurredTexture.sample(texSampler, uv).rgb;
    float depth = depthTexture.sample(texSampler, uv).r;
    
    // Linearize depth (assuming reverse-z)
    float near = 0.1;
    float far = 1000.0;
    float linearDepth = near * far / (far - depth * (far - near));
    
    // Calculate circle of confusion
    float coc = abs(linearDepth - focusDistance) / focusRange;
    coc = saturate(coc * maxBlur);
    
    float3 result = mix(sharp, blurred, coc);
    
    return float4(result, 1.0);
}

// ============================================
// SSAO
// ============================================

constant float3 ssaoKernel[16] = {
    float3(0.2024537, 0.8413493, -0.5007771),
    float3(0.1570458, -0.5707482, 0.8064527),
    float3(-0.0355363, -0.2161776, 0.2748879),
    float3(-0.4685254, 0.6749564, -0.1476315),
    float3(0.5733426, -0.1214756, 0.4426182),
    float3(-0.3386589, 0.3261568, -0.1876826),
    float3(0.0889458, -0.4685254, -0.1779456),
    float3(-0.1687426, 0.2561547, 0.5645782),
    float3(0.4156426, 0.1247854, -0.2487532),
    float3(-0.2854726, -0.4512456, 0.3256489),
    float3(0.1425689, 0.3145782, 0.6524578),
    float3(-0.5124578, -0.2145789, -0.1457896),
    float3(0.3256478, -0.1547896, 0.4789652),
    float3(-0.1245789, 0.5789456, -0.2547896),
    float3(0.4578965, 0.2145896, 0.1547896),
    float3(-0.2547896, -0.3547896, -0.4547896)
};

kernel void ssao_compute(
    texture2d<float, access::read> depthTexture [[texture(0)]],
    texture2d<float, access::read> normalTexture [[texture(1)]],
    texture2d<float, access::write> aoTexture [[texture(2)]],
    constant float4x4& projection [[buffer(0)]],
    constant float4x4& invProjection [[buffer(1)]],
    constant float& radius [[buffer(2)]],
    constant float& bias [[buffer(3)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint width = depthTexture.get_width();
    uint height = depthTexture.get_height();
    
    if (gid.x >= width || gid.y >= height) return;
    
    float2 uv = float2(gid) / float2(width, height);
    float depth = depthTexture.read(gid).r;
    
    if (depth >= 1.0) {
        aoTexture.write(float4(1.0), gid);
        return;
    }
    
    // Reconstruct view position
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    clipPos.y = -clipPos.y;
    float4 viewPos = invProjection * clipPos;
    viewPos /= viewPos.w;
    
    float3 normal = normalTexture.read(gid).rgb * 2.0 - 1.0;
    
    // Random rotation based on position
    float randomAngle = fract(sin(dot(float2(gid), float2(12.9898, 78.233))) * 43758.5453) * 2.0 * M_PI_F;
    float3 randomVec = float3(cos(randomAngle), sin(randomAngle), 0.0);
    
    // Create TBN
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    
    for (int i = 0; i < 16; i++) {
        float3 sampleVec = TBN * ssaoKernel[i];
        float3 samplePos = viewPos.xyz + sampleVec * radius;
        
        // Project sample
        float4 offset = projection * float4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        offset.y = 1.0 - offset.y;
        
        uint2 sampleGid = uint2(offset.xy * float2(width, height));
        sampleGid = clamp(sampleGid, uint2(0), uint2(width - 1, height - 1));
        
        float sampleDepth = depthTexture.read(sampleGid).r;
        
        // Reconstruct sample view pos
        float4 sampleClip = float4(offset.xy * 2.0 - 1.0, sampleDepth, 1.0);
        sampleClip.y = -sampleClip.y;
        float4 sampleView = invProjection * sampleClip;
        sampleView /= sampleView.w;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleView.z));
        occlusion += (sampleView.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / 16.0);
    
    aoTexture.write(float4(occlusion), gid);
}

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float4 tangent [[attribute(2)]];
    float2 uv [[attribute(3)]];
    float2 uv2 [[attribute(4)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 worldPosition;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float2 uv;
    float2 uv2;
    float4 shadowCoord;
    float clipDepth;
};

struct CameraData {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 invViewProjectionMatrix;
    float3 position;
    float nearPlane;
    float3 forward;
    float farPlane;
    float2 screenSize;
    float time;
    float deltaTime;
};

struct LightData {
    float3 direction;
    float intensity;
    float3 color;
    float shadowBias;
    float4x4 lightSpaceMatrix;
};

struct TerrainPushConstants {
    float2 chunkWorldPos;
    float lodScale;
    uint biomeId;
    uint lodLevel;
    float morphFactor;
    float _pad0;
    float _pad1;
};

struct BiomeParams {
    float2 worldOffset;
    float scale;
    uint64_t seed;
};

struct BiomeData {
    float3 groundColor;
    float heightScale;
    float3 cliffColor;
    float cliffThreshold;
    float noiseFrequency;
    float noiseAmplitude;
    float vegetationDensity;
    float rockDensity;
};

// ============================================
// NOISE FUNCTIONS
// ============================================

float hash(float2 p) {
    float3 p3 = fract(float3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise2D(float2 p) {
    float2 i = floor(p);
    float2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + float2(1.0, 0.0));
    float c = hash(i + float2(0.0, 1.0));
    float d = hash(i + float2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(float2 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise2D(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

// ============================================
// TRIPLANAR MAPPING
// ============================================

float3 triplanarNormal(texture2d<float> normalMap, sampler s,
                       float3 worldPos, float3 worldNormal, float scale) {
    float3 blending = abs(worldNormal);
    blending = normalize(max(blending, 0.00001));
    float b = blending.x + blending.y + blending.z;
    blending /= b;
    
    float3 xNorm = normalMap.sample(s, worldPos.yz * scale).rgb * 2.0 - 1.0;
    float3 yNorm = normalMap.sample(s, worldPos.xz * scale).rgb * 2.0 - 1.0;
    float3 zNorm = normalMap.sample(s, worldPos.xy * scale).rgb * 2.0 - 1.0;
    
    // Swizzle pour axes
    xNorm = float3(xNorm.xy + worldNormal.zy, abs(xNorm.z) * worldNormal.x);
    yNorm = float3(yNorm.xy + worldNormal.xz, abs(yNorm.z) * worldNormal.y);
    zNorm = float3(zNorm.xy + worldNormal.xy, abs(zNorm.z) * worldNormal.z);
    
    return normalize(xNorm * blending.x + yNorm * blending.y + zNorm * blending.z);
}

float4 triplanarSample(texture2d<float> tex, sampler s,
                       float3 worldPos, float3 worldNormal, float scale) {
    float3 blending = abs(worldNormal);
    blending = normalize(max(blending, 0.00001));
    float b = blending.x + blending.y + blending.z;
    blending /= b;
    
    float4 xAxis = tex.sample(s, worldPos.yz * scale);
    float4 yAxis = tex.sample(s, worldPos.xz * scale);
    float4 zAxis = tex.sample(s, worldPos.xy * scale);
    
    return xAxis * blending.x + yAxis * blending.y + zAxis * blending.z;
}

// ============================================
// VERTEX SHADER
// ============================================

vertex VertexOut terrain_vertex(
    VertexIn in [[stage_in]],
    constant CameraData& camera [[buffer(0)]],
    constant TerrainPushConstants& terrain [[buffer(1)]],
    constant LightData& light [[buffer(2)]]
) {
    VertexOut out;
    
    float3 worldPos = in.position;
    
    out.worldPosition = worldPos;
    out.position = camera.viewProjectionMatrix * float4(worldPos, 1.0);
    out.clipDepth = out.position.z / out.position.w;
    
    out.normal = normalize(in.normal);
    out.tangent = normalize(in.tangent.xyz);
    out.bitangent = cross(out.normal, out.tangent) * in.tangent.w;
    
    out.uv = in.uv;
    out.uv2 = in.uv2;
    
    out.shadowCoord = light.lightSpaceMatrix * float4(worldPos, 1.0);
    
    return out;
}

// ============================================
// FRAGMENT SHADER - TERRAIN PBR
// ============================================

fragment float4 terrain_fragment(
    VertexOut in [[stage_in]],
    constant CameraData& camera [[buffer(0)]],
    constant TerrainPushConstants& terrain [[buffer(1)]],
    constant LightData& light [[buffer(2)]],
    constant BiomeData* biomes [[buffer(3)]],
    texture2d<float> heightmap [[texture(0)]],
    texture2d<float> normalMap [[texture(1)]],
    texture2d<float> albedoArray [[texture(2)]],
    texture2d<float> roughnessArray [[texture(3)]],
    texture2d<float> shadowMap [[texture(4)]],
    sampler texSampler [[sampler(0)]],
    sampler shadowSampler [[sampler(1)]]
) {
    BiomeData biome = biomes[terrain.biomeId];
    
    // Calcul de la pente pour cliff/ground blending
    float slope = 1.0 - dot(in.normal, float3(0, 1, 0));
    float cliffFactor = smoothstep(biome.cliffThreshold - 0.1, biome.cliffThreshold + 0.1, slope);
    
    // Albedo avec variation de bruit
    float noiseDetail = fbm(in.worldPosition.xz * 0.1, 4) * 0.2;
    float3 groundAlbedo = biome.groundColor * (0.9 + noiseDetail);
    float3 cliffAlbedo = biome.cliffColor * (0.85 + noiseDetail * 0.5);
    float3 albedo = mix(groundAlbedo, cliffAlbedo, cliffFactor);
    
    // Variation de hauteur
    float heightVariation = saturate((in.worldPosition.y - 10.0) / 100.0);
    albedo = mix(albedo, albedo * 0.7 + float3(0.2, 0.2, 0.25), heightVariation * 0.5);
    
    // Normal mapping triplanar
    float3 N = in.normal;
    if (!is_null_texture(normalMap)) {
        float3 sampledNormal = triplanarNormal(normalMap, texSampler,
                                                in.worldPosition, in.normal, 0.1);
        
        // Blend avec géométrie
        float3x3 TBN = float3x3(in.tangent, in.bitangent, in.normal);
        N = normalize(TBN * sampledNormal);
    }
    
    // Material properties
    float roughness = mix(biome.rockDensity * 0.3 + 0.5, 0.85, cliffFactor);
    roughness += fbm(in.worldPosition.xz * 0.5, 3) * 0.15;
    roughness = saturate(roughness);
    
    float metallic = mix(0.0, 0.05, cliffFactor);
    float ao = 1.0 - slope * 0.3;
    
    // View direction
    float3 V = normalize(camera.position - in.worldPosition);
    float3 L = normalize(-light.direction);
    float3 H = normalize(V + L);
    
    // PBR calculations
    float3 F0 = float3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    float3 specular = numerator / denominator;
    
    // Shadow
    float3 shadowCoord = in.shadowCoord.xyz / in.shadowCoord.w;
    shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
    shadowCoord.y = 1.0 - shadowCoord.y;
    
    float shadow = 1.0;
    if (shadowCoord.x >= 0.0 && shadowCoord.x <= 1.0 &&
        shadowCoord.y >= 0.0 && shadowCoord.y <= 1.0 &&
        !is_null_texture(shadowMap)) {
        
        float currentDepth = shadowCoord.z;
        
        // PCF soft shadows
        float2 texelSize = 1.0 / float2(shadowMap.get_width(), shadowMap.get_height());
        shadow = 0.0;
        
        for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
                float pcfDepth = shadowMap.sample(shadowSampler,
                    shadowCoord.xy + float2(x, y) * texelSize).r;
                shadow += currentDepth - light.shadowBias > pcfDepth ? 0.0 : 1.0;
            }
        }
        shadow /= 25.0;
    }
    
    // Direct lighting
    float3 radiance = light.color * light.intensity;
    float3 Lo = (kD * albedo / M_PI_F + specular) * radiance * NdotL * shadow;
    
    // Ambient
    float3 ambient = float3(0.03) * albedo * ao;
    
    // Hemisphere ambient
    float3 skyColor = float3(0.4, 0.6, 0.9);
    float3 groundColor = float3(0.2, 0.15, 0.1);
    float3 hemisphereAmbient = mix(groundColor, skyColor, N.y * 0.5 + 0.5) * 0.15;
    ambient += hemisphereAmbient * albedo * ao;
    
    float3 color = ambient + Lo;
    
    // Distance fog
    float fogDistance = length(camera.position - in.worldPosition);
    float fogFactor = 1.0 - exp(-fogDistance * 0.0008);
    float3 fogColor = float3(0.6, 0.7, 0.85);
    color = mix(color, fogColor, saturate(fogFactor));
    
    return float4(color, 1.0);
}

// ============================================
// SHADOW PASS
// ============================================

struct ShadowVertexOut {
    float4 position [[position]];
};

vertex ShadowVertexOut shadow_vertex(
    VertexIn in [[stage_in]],
    constant float4x4& lightSpaceMatrix [[buffer(0)]]
) {
    ShadowVertexOut out;
    out.position = lightSpaceMatrix * float4(in.position, 1.0);
    return out;
}

fragment float4 shadow_fragment(ShadowVertexOut in [[stage_in]]) {
    return float4(in.position.z, 0, 0, 1);
}

// ============================================
// HEIGHTMAP GENERATION KERNEL
// ============================================

kernel void generateHeightmap(
    texture2d<float, access::write> output [[texture(0)]],
    constant uint* permutation [[buffer(0)]],
    constant float4* gradients [[buffer(1)]],
    constant NoiseParams& params [[buffer(2)]],
    constant BiomeData* biomes [[buffer(3)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint width = output.get_width();
    uint height = output.get_height();
    
    if (gid.x >= width || gid.y >= height) return;
    
    float2 worldPos = params.worldOffset + float2(gid) * params.scale;
    BiomeData biome = biomes[params.biomeId];
    
    uint seed = uint(params.seed);
    
    // Base terrain avec FBM
    float baseNoise = fbm2D(worldPos * biome.noiseFrequency, 6, 0.5, 2.1, seed);
    
    // Ridged noise pour montagnes
    float ridged = 1.0 - abs(fbm2D(worldPos * biome.noiseFrequency * 0.5, 4, 0.6, 2.2, seed + 1000));
    ridged = ridged * ridged;
    
    // Voronoi pour variation
    VoronoiResult vor = voronoi2D(worldPos * 0.01, seed + 2000);
    float voronoiHeight = smoothstep(0.0, 0.3, vor.distance1) * 0.3;
    
    // Domain warping pour intérêt
    float warped = warpedNoise(worldPos * 0.003, seed + 3000);
    
    // Combine
    float heightValue = biome.heightScale * (
        baseNoise * 0.4 +
        ridged * 0.3 +
        voronoiHeight * 0.2 +
        warped * 0.1
    );
    
    // Spawn area modifier
    float2 spawnCenter = float2(0.0);
    float distToSpawn = length(worldPos - spawnCenter);
    float spawnRadius = 128.0;
    float safeRadius = 256.0;
    
    if (distToSpawn < safeRadius) {
        float t = saturate(distToSpawn / safeRadius);
        float craterShape = 1.0 - cos(min(distToSpawn / spawnRadius, 1.0) * M_PI_F * 0.5);
        float flattenFactor = smoothstep(0.0, 1.0, t);
        
        // Aplatir vers le centre
        heightValue = mix(-8.0 * (1.0 - craterShape), heightValue, flattenFactor);
    }
    
    output.write(float4(heightValue, 0, 0, 1), gid);
}

// ============================================
// NORMAL MAP GENERATION KERNEL
// ============================================

kernel void generateNormalMap(
    texture2d<float, access::read> heightmap [[texture(0)]],
    texture2d<float, access::write> normalMap [[texture(1)]],
    constant float& heightScale [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint width = heightmap.get_width();
    uint height = heightmap.get_height();
    
    if (gid.x >= width || gid.y >= height) return;
    
    // Échantillonner les hauteurs voisines
    float hL = heightmap.read(uint2(max(int(gid.x) - 1, 0), gid.y)).r;
    float hR = heightmap.read(uint2(min(gid.x + 1, width - 1), gid.y)).r;
    float hD = heightmap.read(uint2(gid.x, max(int(gid.y) - 1, 0))).r;
    float hU = heightmap.read(uint2(gid.x, min(gid.y + 1, height - 1))).r;
    
    // Calculer la normale
    float3 normal;
    normal.x = (hL - hR) * heightScale;
    normal.y = 2.0;
    normal.z = (hD - hU) * heightScale;
    normal = normalize(normal);
    
    // Encoder en [0, 1]
    float3 encoded = normal * 0.5 + 0.5;
    
    normalMap.write(float4(encoded, 1.0), gid);
}

// ============================================
// BIOME MAP GENERATION KERNEL
// ============================================

kernel void generateBiomeMap(
    texture2d<float, access::write> output [[texture(0)]],
    constant uint* permutation [[buffer(0)]],
    constant BiomeParams& params [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint width = output.get_width();
    uint height = output.get_height();
    
    if (gid.x >= width || gid.y >= height) return;
    
    float2 worldPos = params.worldOffset + float2(gid) * params.scale;
    uint seed = uint(params.seed);
    
    // Utiliser Voronoi pour les biomes
    VoronoiResult vor = voronoi2D(worldPos * 0.002, seed);
    
    // Biome ID basé sur le hash de la cellule
    uint biomeId = vor.cellId % 8;
    
    // Distance pour blend
    float edgeDistance = vor.distance2 - vor.distance1;
    float blendFactor = smoothstep(0.0, 0.1, edgeDistance);
    
    output.write(float4(float(biomeId) / 255.0, blendFactor, edgeDistance, 1.0), gid);
}

// ============================================
// SPLAT MAP GENERATION KERNEL
// ============================================

kernel void generateSplatMap(
    texture2d<float, access::read> heightmap [[texture(0)]],
    texture2d<float, access::read> normalMap [[texture(1)]],
    texture2d<float, access::write> splatMap [[texture(2)]],
    constant uint& seed [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]]
) {
    uint width = heightmap.get_width();
    uint height = heightmap.get_height();
    
    if (gid.x >= width || gid.y >= height) return;
    
    float terrainHeight = heightmap.read(gid).r;
    float3 normal = normalMap.read(gid).rgb * 2.0 - 1.0;
    
    float slope = 1.0 - normal.y;
    
    // Channels: R=grass, G=rock, B=sand, A=snow
    float4 splat = float4(0);
    
    // Grass sur pentes douces et basse altitude
    splat.r = smoothstep(0.4, 0.2, slope) * smoothstep(80.0, 20.0, terrainHeight);
    
    // Rock sur pentes raides
    splat.g = smoothstep(0.3, 0.6, slope);
    
    // Sand en basse altitude
    splat.b = smoothstep(5.0, -5.0, terrainHeight) * (1.0 - splat.g);
    
    // Snow en haute altitude
    splat.a = smoothstep(60.0, 100.0, terrainHeight) * smoothstep(0.5, 0.3, slope);
    
    // Normaliser
    float total = splat.r + splat.g + splat.b + splat.a;
    if (total > 0.0) {
        splat /= total;
    }
    
    splatMap.write(splat, gid);
}












//// STRUCTURES COMMUNES
//
//struct TerrainUniforms
//{
//    float4x4 modelMatrix;
//    float4x4 viewProjectionMatrix;
//    float3 cameraPosition;
//    float time;
//    float4 fogColor;
//    float fogDensity;
//    float maxRenderDistance;
//    uint seed;
//    float heightScale;
//};
//
//struct GridUniforms
//{
//    uint3 dimensions;
//    uint totalVoxels;
//    float3 voxelSize;
//    float padding;
//    float3 gridOrigin;
//    float padding2;
//};
//
//struct Voxel
//{
//    uint16_t blockType;
//    uint8_t flags;
//    uint8_t lightLevel;
//};
//
//struct TerrainVertex
//{
//    float3 position [[attribute(0)]];
//    float3 normal [[attribute(1)]];
//    float3 tangent [[attribute(2)]];
//    float2 texCoord [[attribute(3)]];
//    float4 color [[attribute(4)]];
//    float height [[attribute(5)]];
//    float biomeId [[attribute(6)]];
//};
//
//struct Vertex
//{
//    float3 position [[attribute(0)]];
//    float3 normal [[attribute(1)]];
//    float2 texCoord [[attribute(2)]];
//    uint blockType [[attribute(3)]];
//};
//
//struct VertexOut
//{
//    float4 position [[position]];
//    float3 worldPosition;
//    float3 normal;
//    float3 tangent;
//    float2 texCoord;
//    float4 color;
//    float height;
//    float biomeId;
//};
//
//struct VoxelVertexOut
//{
//    float4 position [[position]];
//    float3 worldPosition;
//    float3 normal;
//    float2 texCoord;
//    float blockType;
//};
//
//// NOISE FUNCTIONS
//
//// Hash function pour génération pseudo-aléatoire
//float hash(float2 p)
//{
//    p = fract(p * float2(123.34, 456.21));
//    p += dot(p, p + 45.32);
//    return fract(p.x * p.y);
//}
//
//float hash3D(float3 p) {
//    p = fract(p * float3(0.1031, 0.1030, 0.0973));
//    p += dot(p, p.yzx + 33.33);
//    return fract((p.x + p.y) * p.z);
//}
//
//// Perlin noise 2D
//float perlinNoise(float2 p)
//{
//    float2 i = floor(p);
//    float2 f = fract(p);
//    
//    float a = hash(i);
//    float b = hash(i + float2(1.0, 0.0));
//    float c = hash(i + float2(0.0, 1.0));
//    float d = hash(i + float2(1.0, 1.0));
//    
//    float2 u = f * f * (3.0 - 2.0 * f);
//    
//    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
//}
//
//// Fractal Brownian Motion (FBM)
//float fbm(float2 p, int octaves, float persistence, float lacunarity)
//{
//    float value = 0.0;
//    float amplitude = 1.0;
//    float frequency = 1.0;
//    float maxValue = 0.0;
//    
//    for (int i = 0; i < octaves; i++) {
//        value += amplitude * perlinNoise(p * frequency);
//        maxValue += amplitude;
//        amplitude *= persistence;
//        frequency *= lacunarity;
//    }
//    
//    return value / maxValue;
//}
//
//// Simplex noise 3D (approximation)
//float simplexNoise3D(float3 p)
//{
//    const float3 s = float3(1.70158, 1.70158, 1.70158);
//    float3 i = floor(p + dot(p, s / 3.0));
//    float3 f = p - i + dot(i, 1.0 / 6.0);
//    
//    return hash3D(i) * 2.0 - 1.0;
//}
//
//// Voronoi noise (retourne distance au plus proche point)
//float2 voronoiNoise(float2 p, device const float2* points, uint numPoints)
//{
//    float minDist = 10000.0;
//    float secondMinDist = 10000.0;
//    
//    for (uint i = 0; i < numPoints; i++) {
//        float dist = length(p - points[i]);
//        
//        if (dist < minDist) {
//            secondMinDist = minDist;
//            minDist = dist;
//        } else if (dist < secondMinDist) {
//            secondMinDist = dist;
//        }
//    }
//    
//    return float2(minDist, secondMinDist);
//}
//
//// COMPUTE SHADERS - GÉNÉRATION DE TERRAIN GPU
//
//// Génération de heightmap avec Voronoi
//kernel void voronoiHeightmapGenerator(texture2d<float, access::write> heightmap [[texture(0)]],
//                                      texture2d<float, access::write> biomeMap [[texture(1)]],
//                                      device const float2* voronoiPoints [[buffer(0)]],
//                                      device const float* biomeTypes [[buffer(1)]],
//                                      constant uint& numPoints [[buffer(2)]],
//                                      constant TerrainUniforms& uniforms [[buffer(3)]],
//                                      uint2 gid [[thread_position_in_grid]])
//{
//    float2 dimensions = float2(heightmap.get_width(), heightmap.get_height());
//    float2 uv = float2(gid) / dimensions;
//    float2 worldPos = (uv - 0.5) * 1024.0; // World size
//    
//    // Find nearest Voronoi cells
//    float minDist = 10000.0;
//    float secondMinDist = 10000.0;
//    uint nearestBiome = 0;
//    
//    for (uint i = 0; i < numPoints; i++) {
//        float dist = length(worldPos - voronoiPoints[i]);
//        
//        if (dist < minDist) {
//            secondMinDist = minDist;
//            minDist = dist;
//            nearestBiome = i;
//        } else if (dist < secondMinDist) {
//            secondMinDist = dist;
//        }
//    }
//    
//    float biomeId = biomeTypes[nearestBiome];
//    float edgeFactor = smoothstep(0.0, 10.0, secondMinDist - minDist);
//    
//    // Generate height based on biome type
//    float height = 0.0;
//    float baseNoise = fbm(worldPos * 0.01, 6, 0.5, 2.0);
//    
//    if (biomeId == 0.0) { // Spawn Crater
//        float craterDist = length(worldPos);
//        height = max(0.0, 10.0 - craterDist * 0.1) + baseNoise * 2.0;
//    }
//    else if (biomeId == 1.0) { // Voronoi Plains
//        height = minDist * 0.05 + baseNoise * 5.0;
//    }
//    else if (biomeId == 2.0) { // Voronoi Islands
//        float islandHeight = fbm(worldPos * 0.005, 8, 0.6, 2.1);
//        height = (islandHeight > 0.3) ? (islandHeight * 20.0) : -5.0;
//    }
//    else if (biomeId == 3.0) { // Voronoi Terrace
//        float terraceHeight = minDist * 0.1;
//        height = floor(terraceHeight / 5.0) * 5.0 + baseNoise * 1.0;
//    }
//    else if (biomeId == 4.0) { // Voronoi Eroded
//        float erosion = fbm(worldPos * 0.02, 4, 0.4, 2.5);
//        height = minDist * 0.03 + erosion * 15.0 - 5.0;
//    }
//    else if (biomeId == 5.0) { // Planet
//        float3 spherePos = float3(worldPos.x, 0.0, worldPos.y);
//        float sphereHeight = 50.0 - length(spherePos);
//        height = sphereHeight + baseNoise * 3.0;
//    }
//    else if (biomeId == 6.0) { // Moon (craters)
//        float craterNoise = 0.0;
//        for (int i = 0; i < 5; i++) {
//            float2 craterPos = worldPos + float2(hash(float2(i * 123.4, i * 456.7)) * 100.0);
//            float craterDist = length(craterPos);
//            craterNoise += max(0.0, 8.0 - craterDist) * 0.5;
//        }
//        height = baseNoise * 2.0 - craterNoise;
//    }
//    else if (biomeId == 7.0) { // Ocean
//        height = -20.0 + baseNoise * 2.0;
//    }
//    
//    height *= uniforms.heightScale;
//    
//    heightmap.write(float4(height, 0.0, 0.0, 1.0), gid);
//    biomeMap.write(float4(biomeId, edgeFactor, 0.0, 1.0), gid);
//}
//
//// Calcul des normales à partir de la heightmap
//kernel void calculateNormals(texture2d<float, access::read> heightmap [[texture(0)]],
//                             texture2d<float, access::write> normalMap [[texture(1)]],
//                             constant float& heightScale [[buffer(0)]],
//                             uint2 gid [[thread_position_in_grid]])
//{
//    uint2 dim = uint2(heightmap.get_width(), heightmap.get_height());
//    
//    if (gid.x >= dim.x || gid.y >= dim.y) return;
//    
//    float h = heightmap.read(gid).r;
//    float hL = (gid.x > 0) ? heightmap.read(gid + uint2(-1, 0)).r : h;
//    float hR = (gid.x < dim.x - 1) ? heightmap.read(gid + uint2(1, 0)).r : h;
//    float hD = (gid.y > 0) ? heightmap.read(gid + uint2(0, -1)).r : h;
//    float hU = (gid.y < dim.y - 1) ? heightmap.read(gid + uint2(0, 1)).r : h;
//    
//    float3 normal;
//    normal.x = (hL - hR) * heightScale;
//    normal.y = 2.0;
//    normal.z = (hD - hU) * heightScale;
//    normal = normalize(normal);
//    
//    normalMap.write(float4(normal * 0.5 + 0.5, 1.0), gid);
//}
//
//// Érosion hydraulique simplifiée (GPU)
//kernel void hydraulicErosion(texture2d<float, access::read> heightmapIn [[texture(0)]],
//                             texture2d<float, access::write> heightmapOut [[texture(1)]],
//                             constant float& erosionStrength [[buffer(0)]],
//                             uint2 gid [[thread_position_in_grid]])
//{
//    float h = heightmapIn.read(gid).r;
//    
//    // Find lowest neighbor
//    float minHeight = h;
//    int2 flowDir = int2(0, 0);
//    
//    for (int dy = -1; dy <= 1; dy++) {
//        for (int dx = -1; dx <= 1; dx++) {
//            if (dx == 0 && dy == 0) continue;
//            
//            int2 neighbor = int2(gid) + int2(dx, dy);
//            float neighborHeight = heightmapIn.read(uint2(neighbor)).r;
//            
//            if (neighborHeight < minHeight) {
//                minHeight = neighborHeight;
//                flowDir = int2(dx, dy);
//            }
//        }
//    }
//    
//    // Erode if there's flow
//    if (minHeight < h) {
//        float erosion = (h - minHeight) * erosionStrength;
//        h -= erosion * 0.5;
//    }
//    
//    heightmapOut.write(float4(h, 0.0, 0.0, 1.0), gid);
//}
//
//// VERTEX SHADERS
//
//vertex VertexOut terrainVertex(TerrainVertex in [[stage_in]],
//                               constant TerrainUniforms& uniforms [[buffer(1)]])
//{
//    VertexOut out;
//
//    float4 worldPos = float4(in.position, 1.0); // uniforms.modelMatrix * 
//    out.position = uniforms.viewProjectionMatrix * worldPos;
//    out.worldPosition = in.position;
////    out.worldPosition = worldPos.xyz;
////    out.normal = (uniforms.modelMatrix * float4(in.normal, 0.0)).xyz;
//    out.normal = in.normal;
////    out.tangent = (uniforms.modelMatrix * float4(in.tangent, 0.0)).xyz;
//    out.tangent = in.tangent;
//    out.texCoord = in.texCoord;
//    out.color = in.color;
//    out.height = in.height;
//    out.biomeId = in.biomeId;
//    
//    return out;
//}
//
//vertex VoxelVertexOut voxelMeshVertex(Vertex in [[stage_in]],
//                                      constant float4x4& viewProjection [[buffer(1)]],
//                                      constant GridUniforms& gridUniforms [[buffer(2)]])
//{
//    VoxelVertexOut out;
//    
//    out.position = viewProjection * float4(in.position, 1.0);
//    out.worldPosition = in.position;
//    out.normal = in.normal;
//    out.texCoord = in.texCoord;
//    out.blockType = in.blockType;
//    
//    return out;
//}
//
//// Grid edges pour edit mode
//struct GridEdgeVertexOut {
//    float4 position [[position]];
//};
//
//vertex GridEdgeVertexOut voxelGridEdgeVertex(
//    const device float3* vertices [[buffer(0)]],
//    constant float4x4& viewProjection [[buffer(1)]],
//    uint vid [[vertex_id]]
//)
//{
//    GridEdgeVertexOut out;
//    out.position = viewProjection * float4(vertices[vid], 1.0);
//    return out;
//}
//
//// FRAGMENT SHADERS - PBR RENDERING
//
//// PBR helper functions in Helpers.metal
//
////fragment float4 terrainFragment(VertexOut in [[stage_in]],
////                                constant TerrainUniforms& uniforms [[buffer(0)]],
////                                texture2d<float> albedoArray [[texture(0)]],
////                                texture2d<float> normalMapArray [[texture(1)]],
////                                texture2d<float> roughnessArray [[texture(2)]],
////                                texture2d<float> metallicArray [[texture(3)]],
////                                texture2d<float> aoArray [[texture(4)]],
////                                sampler textureSampler [[sampler(0)]])
////{
////    // Sample material textures based on biome
////    int biomeIndex = int(in.biomeId);
////    float2 texCoord = in.texCoord * 10.0; // Tiling
////    
////    float4 albedo = albedoArray.sample(textureSampler, texCoord);
////    float3 normalMap = normalMapArray.sample(textureSampler, texCoord).rgb * 2.0 - 1.0;
////    float roughness = roughnessArray.sample(textureSampler, texCoord).r;
////    float metallic = metallicArray.sample(textureSampler, texCoord).r;
////    float ao = aoArray.sample(textureSampler, texCoord).r;
//
//fragment float4 terrainFragment(VertexOut in [[stage_in]],
//                                constant TerrainUniforms& uniforms [[buffer(0)]],
//                                texture2d_array<float> albedoArray [[texture(0)]],
//                                texture2d_array<float> normalMapArray [[texture(1)]],
//                                texture2d_array<float> roughnessArray [[texture(2)]],
//                                texture2d_array<float> metallicArray [[texture(3)]],
//                                texture2d_array<float> aoArray [[texture(4)]],
//                                sampler textureSampler [[sampler(0)]])
//{
//    // Sample material textures based on biome
////    int biomeIndex = int(in.biomeId);
//    float2 texCoord = in.texCoord * 10.0; // Tiling
//
//    float4 albedo = albedoArray.sample(textureSampler, texCoord, in.biomeId);
//    float3 normalMap = normalMapArray.sample(textureSampler, texCoord, in.biomeId).rgb * 2.0 - 1.0;
//    float roughness = roughnessArray.sample(textureSampler, texCoord, in.biomeId).r;
//    float metallic = metallicArray.sample(textureSampler, texCoord, in.biomeId).r;
//    float ao = aoArray.sample(textureSampler, texCoord, in.biomeId).r;
//    
//    // Blend with biome color
//    albedo.rgb = mix(albedo.rgb, in.color.rgb, 0.3);
//    
//    // Transform normal from tangent space to world space
//    float3 N = normalize(in.normal);
//    float3 T = normalize(in.tangent);
//    float3 B = cross(N, T);
//    float3x3 TBN = float3x3(T, B, N);
//    N = normalize(TBN * normalMap);
//    
//    float3 V = normalize(uniforms.cameraPosition - in.worldPosition);
//    
//    // PBR lighting
//    float3 F0 = float3(0.04);
//    F0 = mix(F0, albedo.rgb, metallic);
//    
//    float3 Lo = float3(0.0);
//    
//    // Simple directional light (sun)
//    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
//    float3 lightColor = float3(1.0, 0.95, 0.8) * 3.0;
//    
//    float3 L = lightDir;
//    float3 H = normalize(V + L);
//    
//    float NDF = distributionGGX(N, H, roughness);
//    float G = geometrySmith(N, V, L, roughness);
//    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
//    
//    float3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
//    float3 specular = numerator / denominator;
//    
//    float3 kS = F;
//    float3 kD = float3(1.0) - kS;
//    kD *= 1.0 - metallic;
//    
//    float NdotL = max(dot(N, L), 0.0);
//    Lo += (kD * albedo.rgb / M_PI_F + specular) * lightColor * NdotL;
//    
//    // Ambient lighting
//    float3 ambient = float3(0.15, 0.18, 0.22) * albedo.rgb * ao;
//    float3 color = ambient + Lo;
//    
//    // Fog
//    float distance = length(in.worldPosition - uniforms.cameraPosition);
//    float fogFactor = exp(-distance * uniforms.fogDensity);
//    color = mix(uniforms.fogColor.rgb, color, fogFactor);
//    
//    // Tone mapping and gamma correction
//    color = color / (color + float3(1.0));
//    color = pow(color, float3(1.0 / 2.2));
//    
//    return float4(color, 1.0);
//}
//
//fragment float4 voxelMeshFragment(VoxelVertexOut in [[stage_in]],
//                                  constant GridUniforms& uniforms [[buffer(0)]],
//                                  texture2d<float> blockTextures [[texture(0)]],
//                                  sampler textureSampler [[sampler(0)]])
//{
//    // Simple voxel rendering with texture atlas
//    int blockType = int(in.blockType);
//    float2 texCoord = in.texCoord;
//    
//    // Atlas lookup based on block type
//    float atlasSize = 16.0; // 16x16 atlas
//    float2 atlasCoord = float2(blockType % 16, blockType / 16) / atlasSize;
//    texCoord = atlasCoord + texCoord / atlasSize;
//    
//    float4 color = blockTextures.sample(textureSampler, texCoord);
//    
//    // Simple lighting based on normal
//    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
//    float diffuse = max(dot(in.normal, lightDir), 0.0);
//    float ambient = 0.3;
//    
//    color.rgb *= (ambient + diffuse * 0.7);
//    
//    return color;
//}
//
//fragment float4 voxelGridEdgeFragment(GridEdgeVertexOut in [[stage_in]],
//                                      constant float4& color [[buffer(0)]])
//{
//    return color;
//}
//
//// COMPUTE SHADERS - VOXEL OPERATIONS
//
//// Greedy meshing algorithm (GPU version)
//kernel void greedyMeshGeneration(device const Voxel* voxels [[buffer(0)]],
//                                 device Vertex* vertices [[buffer(1)]],
//                                 device uint* indices [[buffer(2)]],
//                                 device atomic_uint* vertexCount [[buffer(3)]],
//                                 device atomic_uint* indexCount [[buffer(4)]],
//                                 constant GridUniforms& uniforms [[buffer(5)]],
//                                 uint3 gid [[thread_position_in_grid]])
//{
//    // This would implement greedy meshing on GPU
//    // Simplified version here - full implementation would be more complex
//    
//    uint3 dim = uniforms.dimensions;
//    if (gid.x >= dim.x || gid.y >= dim.y || gid.z >= dim.z) return;
//    
//    uint index = gid.x + gid.y * dim.x + gid.z * dim.x * dim.y;
//    Voxel voxel = voxels[index];
//    
//    if ((voxel.flags & 0x01) == 0) return; // Not active
//    
//    // Check if voxel is visible (has at least one face exposed)
//    // Generate faces only for visible sides
//}
//
//// Voxel culling
//kernel void voxelCulling(device Voxel* voxels [[buffer(0)]],
//                         constant GridUniforms& uniforms [[buffer(1)]],
//                         uint3 gid [[thread_position_in_grid]]
//)
//{
//    uint3 dim = uniforms.dimensions;
//    if (gid.x >= dim.x || gid.y >= dim.y || gid.z >= dim.z) return;
//    
//    uint index = gid.x + gid.y * dim.x + gid.z * dim.x * dim.y;
//    
//    if ((voxels[index].flags & 0x01) == 0) return; // Not active
//    
//    // Check all 6 neighbors
//    bool visible = false;
//    
//    int3 offsets[6] = {
//        int3(-1, 0, 0), int3(1, 0, 0),
//        int3(0, -1, 0), int3(0, 1, 0),
//        int3(0, 0, -1), int3(0, 0, 1)
//    };
//    
//    for (int i = 0; i < 6; i++) {
//        int3 neighborPos = int3(gid) + offsets[i];
//        
//        if (neighborPos.x < 0 || neighborPos.x >= int(dim.x) ||
//            neighborPos.y < 0 || neighborPos.y >= int(dim.y) ||
//            neighborPos.z < 0 || neighborPos.z >= int(dim.z)) {
//            visible = true;
//            break;
//        }
//        
//        uint neighborIndex = neighborPos.x + neighborPos.y * dim.x + neighborPos.z * dim.x * dim.y;
//        
//        if ((voxels[neighborIndex].flags & 0x01) == 0) {
//            visible = true;
//            break;
//        }
//    }
//    
//    // Update visibility flag
//    if (visible) {
//        voxels[index].flags |= 0x02;
//    } else {
//        voxels[index].flags &= ~0x02;
//    }
//}
//
//// ============================================================================
//// INVENTORY UI SHADERS
//// ============================================================================
//
//struct UIVertex {
//    float2 position;
//    float2 texCoord;
//    float4 color;
//};
//
//struct UIVertexOut {
//    float4 position [[position]];
//    float2 texCoord;
//    float4 color;
//};
//
//vertex UIVertexOut inventoryUIVertex(
//    const device UIVertex* vertices [[buffer(0)]],
//    constant float& screenWidth [[buffer(1)]],
//    constant float& screenHeight [[buffer(2)]],
//    uint vid [[vertex_id]]
//)
//{
//    UIVertex in = vertices[vid];
//    UIVertexOut out;
//    
//    // Convert screen space to NDC
//    float2 ndc;
//    ndc.x = (in.position.x / screenWidth) * 2.0 - 1.0;
//    ndc.y = 1.0 - (in.position.y / screenHeight) * 2.0;
//    
//    out.position = float4(ndc, 0.0, 1.0);
//    out.texCoord = in.texCoord;
//    out.color = in.color;
//    
//    return out;
//}
//
//fragment float4 inventoryUIFragment(
//    UIVertexOut in [[stage_in]],
//    texture2d<float> uiTexture [[texture(0)]],
//    sampler texSampler [[sampler(0)]]
//)
//{
//    float4 texColor = uiTexture.sample(texSampler, in.texCoord);
//    return texColor * in.color;
//}
//
//// ============================================================================
//// VEHICLE GRID SHADERS
//// ============================================================================
//
//struct VehicleGridVertex {
//    float3 position [[attribute(0)]];
//    float3 normal [[attribute(1)]];
//    float4 color [[attribute(2)]];
//};
//
//struct VehicleGridVertexOut {
//    float4 position [[position]];
//    float3 worldPosition;
//    float3 normal;
//    float4 color;
//};
//
//vertex VehicleGridVertexOut vehicleGridVertex(
//    VehicleGridVertex in [[stage_in]],
//    constant float4x4& viewProjection [[buffer(1)]],
//    constant float4x4& modelMatrix [[buffer(2)]]
//)
//{
//    VehicleGridVertexOut out;
//    
//    float4 worldPos = modelMatrix * float4(in.position, 1.0);
//    out.position = viewProjection * worldPos;
//    out.worldPosition = worldPos.xyz;
//    out.normal = (modelMatrix * float4(in.normal, 0.0)).xyz;
//    out.color = in.color;
//    
//    return out;
//}
//
//fragment float4 vehicleGridFragment(VehicleGridVertexOut in [[stage_in]])
//{
//    // Simple lighting
//    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
//    float3 normal = normalize(in.normal);
//    float diffuse = max(dot(normal, lightDir), 0.0);
//    float ambient = 0.3;
//    
//    float3 color = in.color.rgb * (ambient + diffuse * 0.7);
//    
//    // Add grid lines for build mode
//    float3 gridPos = fract(in.worldPosition);
//    float gridLine = 0.0;
//    
//    if (gridPos.x < 0.05 || gridPos.x > 0.95 ||
//        gridPos.y < 0.05 || gridPos.y > 0.95 ||
//        gridPos.z < 0.05 || gridPos.z > 0.95) {
//        gridLine = 0.2;
//    }
//    
//    color = mix(color, float3(0.0, 1.0, 1.0), gridLine);
//    
//    return float4(color, in.color.a);
//}
//
//// ============================================================================
//// PHYSICS COMPUTE SHADERS
//// ============================================================================
//
//struct PhysicsEntity {
//    float3 position;
//    float3 velocity;
//    float3 acceleration;
//    float3 angularVelocity;
//    float4 rotation;
//    
//    float mass;
//    float radius;
//    float friction;
//    float restitution;
//    
//    uint flags; // isStatic, onGround, affectedByGravity packed
//};
//
//struct PhysicsUniforms {
//    float3 gravity;
//    float deltaTime;
//    float airDrag;
//    float groundDrag;
//    uint entityCount;
//    float padding;
//};
//
//kernel void physicsIntegration(
//    device PhysicsEntity* entities [[buffer(0)]],
//    constant PhysicsUniforms& uniforms [[buffer(1)]],
//    uint gid [[thread_position_in_grid]]
//)
//{
//    if (gid >= uniforms.entityCount) return;
//    
//    PhysicsEntity entity = entities[gid];
//    
//    // Check if static
//    if (entity.flags & 0x01) return;
//    
//    // Apply gravity
//    if (entity.flags & 0x04) { // affectedByGravity
//        entity.acceleration += uniforms.gravity;
//    }
//    
//    // Apply drag
//    bool onGround = (entity.flags & 0x02) != 0;
//    float drag = onGround ? uniforms.groundDrag : uniforms.airDrag;
//    entity.velocity *= (1.0 - drag * uniforms.deltaTime);
//    
//    // Integrate velocity
//    entity.velocity += entity.acceleration * uniforms.deltaTime;
//    
//    // Integrate position
//    entity.position += entity.velocity * uniforms.deltaTime;
//    
//    // Integrate rotation
//    if (length(entity.angularVelocity) > 0.001) {
//        float angle = length(entity.angularVelocity) * uniforms.deltaTime;
//        float3 axis = normalize(entity.angularVelocity);
//        
//        // Quaternion from axis angle
//        float halfAngle = angle * 0.5;
//        float s = sin(halfAngle);
//        float4 deltaRot = float4(axis * s, cos(halfAngle));
//        
//        // Multiply quaternions
//        float4 q1 = entity.rotation;
//        float4 q2 = deltaRot;
//        entity.rotation = float4(
//            q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
//            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
//            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
//            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
//        );
//        
//        // Normalize
//        entity.rotation = normalize(entity.rotation);
//    }
//    
//    // Reset acceleration
//    entity.acceleration = float3(0.0);
//    
//    // Write back
//    entities[gid] = entity;
//}
//
//kernel void collisionDetection(
//    device PhysicsEntity* entities [[buffer(0)]],
//    constant PhysicsUniforms& uniforms [[buffer(1)]],
//    device atomic_uint* collisionPairs [[buffer(2)]],
//    uint gid [[thread_position_in_grid]]
//)
//{
//    if (gid >= uniforms.entityCount) return;
//    
//    PhysicsEntity entityA = entities[gid];
//    if (entityA.flags & 0x01) return; // Skip static entities
//    
//    // Check against all other entities
//    for (uint i = 0; i < uniforms.entityCount; i++) {
//        if (i == gid) continue;
//        
//        PhysicsEntity entityB = entities[i];
//        
//        float3 delta = entityB.position - entityA.position;
//        float distance = length(delta);
//        float minDist = entityA.radius + entityB.radius;
//        
//        if (distance < minDist && distance > 0.001) {
//            // Collision detected
//            float3 normal = delta / distance;
//            float penetration = minDist - distance;
//            
//            // Resolve collision (simplified)
//            if (!(entityB.flags & 0x01)) { // If B is not static
//                float totalMass = entityA.mass + entityB.mass;
//                float ratio = entityB.mass / totalMass;
//                
//                entityA.position -= normal * penetration * ratio;
//            } else {
//                entityA.position -= normal * penetration;
//            }
//            
//            // Mark as on ground if collision normal is upward
//            if (normal.y > 0.7) {
//                entityA.flags |= 0x02; // Set onGround flag
//            }
//        }
//    }
//    
//    // Write back
//    entities[gid] = entityA;
//}
//
//// ============================================================================
//// ADVANCED TERRAIN GENERATION - MULTI-BIOME VORONOI
//// ============================================================================
//
//struct VoronoiCell {
//    float2 center;
//    uint biomeType;
//    float radius;
//    float4 color;
//};
//
//// Generate Voronoi diagram with multiple biome types
//kernel void advancedVoronoiGenerator(
//    texture2d<float, access::write> heightmap [[texture(0)]],
//    texture2d<float, access::write> biomeMap [[texture(1)]],
//    texture2d<float, access::write> voronoiTexture [[texture(2)]],
//    device const VoronoiCell* cells [[buffer(0)]],
//    constant uint& numCells [[buffer(1)]],
//    constant uint& seed [[buffer(2)]],
//    constant float& worldSize [[buffer(3)]],
//    uint2 gid [[thread_position_in_grid]]
//)
//{
//    float2 dimensions = float2(heightmap.get_width(), heightmap.get_height());
//    float2 uv = float2(gid) / dimensions;
//    float2 worldPos = (uv - 0.5) * worldSize;
//    
//    // Find nearest 3 cells for smooth transitions
//    float dist1 = 10000.0;
//    float dist2 = 10000.0;
//    float dist3 = 10000.0;
//    uint cell1 = 0;
//    uint cell2 = 0;
//    uint cell3 = 0;
//    
//    for (uint i = 0; i < numCells; i++) {
//        float dist = length(worldPos - cells[i].center);
//        
//        if (dist < dist1) {
//            dist3 = dist2;
//            cell3 = cell2;
//            dist2 = dist1;
//            cell2 = cell1;
//            dist1 = dist;
//            cell1 = i;
//        } else if (dist < dist2) {
//            dist3 = dist2;
//            cell3 = cell2;
//            dist2 = dist;
//            cell2 = i;
//        } else if (dist < dist3) {
//            dist3 = dist;
//            cell3 = i;
//        }
//    }
//    
//    // Calculate blend weights
//    float totalDist = dist1 + dist2 + dist3;
//    float weight1 = 1.0 - (dist1 / totalDist);
//    float weight2 = 1.0 - (dist2 / totalDist);
//    float weight3 = 1.0 - (dist3 / totalDist);
//    float totalWeight = weight1 + weight2 + weight3;
//    
//    weight1 /= totalWeight;
//    weight2 /= totalWeight;
//    weight3 /= totalWeight;
//    
//    // Generate height based on biome blend
//    float height = 0.0;
//    float4 color = float4(0.0);
//    
//    // Process each cell's contribution
//    for (uint i = 0; i < 3; i++) {
//        uint cellIdx = (i == 0) ? cell1 : ((i == 1) ? cell2 : cell3);
//        float weight = (i == 0) ? weight1 : ((i == 1) ? weight2 : weight3);
//        
//        VoronoiCell cell = cells[cellIdx];
//        float localHeight = 0.0;
//        
//        float distFromCenter = length(worldPos - cell.center);
//        float normalizedDist = distFromCenter / cell.radius;
//        
//        // Biome-specific height generation
//        if (cell.biomeType == 0) { // Spawn Crater
//            localHeight = max(0.0, 20.0 - distFromCenter * 0.2);
//            localHeight += fbm(worldPos * 0.01, 4, 0.5, 2.0) * 3.0;
//        }
//        else if (cell.biomeType == 1) { // Voronoi Plains
//            localHeight = distFromCenter * 0.03;
//            localHeight += fbm(worldPos * 0.008, 6, 0.55, 2.1) * 8.0;
//        }
//        else if (cell.biomeType == 2) { // Voronoi Islands
//            float islandNoise = fbm(worldPos * 0.003, 8, 0.6, 2.0);
//            if (islandNoise > 0.35) {
//                localHeight = (islandNoise - 0.35) * 40.0;
//            } else {
//                localHeight = -15.0; // Water
//            }
//        }
//        else if (cell.biomeType == 3) { // Voronoi Terrace
//            float terraceBase = distFromCenter * 0.08;
//            float terraceStep = 8.0;
//            localHeight = floor(terraceBase / terraceStep) * terraceStep;
//            localHeight += fbm(worldPos * 0.02, 3, 0.4, 2.3) * 2.0;
//        }
//        else if (cell.biomeType == 4) { // Voronoi Eroded
//            float baseHeight = distFromCenter * 0.04;
//            float erosion = fbm(worldPos * 0.015, 5, 0.45, 2.4);
//            
//            // Create valleys
//            float valley = abs(erosion - 0.5) * 2.0;
//            valley = 1.0 - smoothstep(0.0, 0.5, valley);
//            
//            localHeight = baseHeight + erosion * 18.0 - valley * 12.0;
//        }
//        else if (cell.biomeType == 5) { // Planet (spherical)
//            float3 spherePos = float3(worldPos.x, 0.0, worldPos.y);
//            float sphereHeight = 60.0 - length(spherePos - float3(cell.center.x, 0, cell.center.y));
//            localHeight = max(sphereHeight, -10.0);
//            localHeight += fbm(worldPos * 0.012, 5, 0.5, 2.0) * 5.0;
//        }
//        else if (cell.biomeType == 6) { // Moon (cratered)
//            localHeight = fbm(worldPos * 0.01, 4, 0.5, 2.0) * 8.0;
//            
//            // Add craters
//            for (int c = 0; c < 8; c++) {
//                float2 craterCenter = cell.center + float2(
//                    hash(float2(c * 123.4 + seed, c * 456.7)) * 60.0 - 30.0,
//                    hash(float2(c * 789.1 + seed, c * 321.5)) * 60.0 - 30.0
//                );
//                
//                float craterDist = length(worldPos - craterCenter);
//                float craterRadius = 8.0 + hash(float2(c + seed)) * 5.0;
//                
//                if (craterDist < craterRadius) {
//                    float craterDepth = (1.0 - craterDist / craterRadius);
//                    craterDepth = pow(craterDepth, 2.0);
//                    localHeight -= craterDepth * 6.0;
//                    
//                    // Crater rim
//                    if (craterDist > craterRadius * 0.85) {
//                        localHeight += 2.0;
//                    }
//                }
//            }
//        }
//        else if (cell.biomeType == 7) { // Ocean
//            localHeight = -25.0 + fbm(worldPos * 0.005, 6, 0.5, 2.0) * 3.0;
//        }
//        else if (cell.biomeType == 8) { // Crystal Caves (surface entry)
//            localHeight = fbm(worldPos * 0.01, 6, 0.5, 2.0) * 10.0;
//            
//            // Create cave entrances
//            float caveNoise = fbm(worldPos * 0.03, 4, 0.6, 2.5);
//            if (caveNoise < 0.3) {
//                localHeight -= 15.0 * (0.3 - caveNoise);
//            }
//        }
//        else if (cell.biomeType == 9) { // Floating Islands
//            float islandPattern = fbm(worldPos * 0.004, 7, 0.65, 2.0);
//            
//            if (islandPattern > 0.45) {
//                localHeight = 40.0 + (islandPattern - 0.45) * 60.0;
//                localHeight += fbm(worldPos * 0.02, 4, 0.5, 2.0) * 8.0;
//            } else {
//                localHeight = -20.0; // Air gap
//            }
//        }
//        else if (cell.biomeType == 10) { // Lava Fields
//            localHeight = fbm(worldPos * 0.008, 6, 0.5, 2.0) * 12.0;
//            
//            // Lava pools (depressions)
//            float lavaPattern = fbm(worldPos * 0.025, 3, 0.6, 2.2);
//            if (lavaPattern < 0.35) {
//                localHeight -= 8.0;
//            }
//        }
//        else if (cell.biomeType == 11) { // Ice Shelf
//            localHeight = fbm(worldPos * 0.006, 5, 0.5, 2.0) * 6.0;
//            
//            // Smooth, flat top
//            localHeight = mix(localHeight, 5.0, 0.6);
//            
//            // Crevasses
//            float crevasse = abs(fbm(worldPos * 0.04, 2, 0.5, 2.0) - 0.5);
//            if (crevasse < 0.1) {
//                localHeight -= 10.0 * (0.1 - crevasse);
//            }
//        }
//        
//        height += localHeight * weight;
//        color += cell.color * weight;
//    }
//    
//    // Edge detection for blending visualization
//    float edgeDist = dist2 - dist1;
//    float edgeFactor = smoothstep(0.0, 10.0, edgeDist);
//    
//    // Write outputs
//    heightmap.write(float4(height, 0.0, 0.0, 1.0), gid);
//    biomeMap.write(float4(cells[cell1].biomeType, edgeFactor, weight1, 1.0), gid);
//    voronoiTexture.write(float4(dist1, dist2, dist3, 1.0), gid);
//}
//
//// Thermal erosion simulation (makes terrain more realistic)
//kernel void thermalErosion(
//    texture2d<float, access::read> heightmapIn [[texture(0)]],
//    texture2d<float, access::write> heightmapOut [[texture(1)]],
//    constant float& talusAngle [[buffer(0)]],
//    constant float& erosionRate [[buffer(1)]],
//    uint2 gid [[thread_position_in_grid]]
//)
//{
//    float h = heightmapIn.read(gid).r;
//    float maxDiff = tan(talusAngle);
//    
//    float totalMaterial = 0.0;
//    int erosionCount = 0;
//    
//    // Check all 8 neighbors
//    for (int dy = -1; dy <= 1; dy++) {
//        for (int dx = -1; dx <= 1; dx++) {
//            if (dx == 0 && dy == 0) continue;
//            
//            int2 neighbor = int2(gid) + int2(dx, dy);
//            if (neighbor.x < 0 || neighbor.y < 0) continue;
//            
//            float nh = heightmapIn.read(uint2(neighbor)).r;
//            float diff = h - nh;
//            
//            if (diff > maxDiff) {
//                float excess = diff - maxDiff;
//                totalMaterial += excess * erosionRate;
//                erosionCount++;
//            }
//        }
//    }
//    
//    // Distribute eroded material
//    if (erosionCount > 0) {
//        h -= totalMaterial;
//    }
//    
//    heightmapOut.write(float4(h, 0.0, 0.0, 1.0), gid);
//}
