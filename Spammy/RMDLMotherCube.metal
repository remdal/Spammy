//
//  RMDLMotherCube.metal
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#include <metal_stdlib>
using namespace metal;

#import "Helpers.metal"

struct alignas(16) BlockGPUInstance
{
    simd::float4x4 modelMatrix;
    simd::float4 color;
    simd::float4 params;
};

struct alignas(16) BlockUniforms
{
    simd::float4x4 viewProj;
    simd::float3 camPos;
    float time;
    simd::float3 lightDir;
    float _pad;
};

struct BlockFragIn {
    float4 position [[position]];
    float3 worldPos;
    float3 worldNormal;
    float2 uv;
    float4 color;
    float damage;
    float powered;
    float time;
    uint textureIndex;
};

struct alignas(16) BlockVertex
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
    float4 color    [[attribute(3)]];
};

struct BlockInstance
{
    float4x4 modelMatrix;
    float4 tintColor;
    float4 params;  // x=damage, y=powered, z=animPhase, w=blockTypeId
};

struct BlockVertexOut
{
    float4 position [[position]];
    float3 worldPos;
    float3 normal;
    float2 uv;
    float4 color;
    float damage;
    float powered;
    float animPhase;
    float blockType;
};

vertex BlockVertexOut blockVertex(
    BlockVertex in [[stage_in]],
    constant BlockInstance* instances [[buffer(1)]],
    constant float4x4& viewProj [[buffer(2)]],
    constant float3& camPos [[buffer(3)]],
    uint instanceId [[instance_id]]
) {
    BlockInstance inst = instances[instanceId];
    
    float4 worldPos = inst.modelMatrix * float4(in.position, 1.0);
    float3 worldNormal = normalize((inst.modelMatrix * float4(in.normal, 0.0)).xyz);
    
    BlockVertexOut out;
    out.position = viewProj * worldPos;
    out.worldPos = worldPos.xyz;
    out.normal = worldNormal;
    out.uv = in.uv;
    out.color = in.color * inst.tintColor;
    out.damage = inst.params.x;
    out.powered = inst.params.y;
    out.animPhase = inst.params.z;
    
    return out;
}

// ============================================================================
// FRAGMENT SHADER
// ============================================================================

fragment float4 blockFragment(
    BlockVertexOut in [[stage_in]],
    constant float3& camPos [[buffer(0)]]
) {
    // Direction lumière (soleil)
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float3 lightColor = float3(1.0, 0.98, 0.95);
    float3 ambientColor = float3(0.15, 0.18, 0.25);
    
    // View direction
    float3 viewDir = normalize(camPos - in.worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    
    // Diffuse
    float NdotL = max(dot(in.normal, lightDir), 0.0);
    float3 diffuse = lightColor * NdotL;
    
    // Specular (Blinn-Phong)
    float NdotH = max(dot(in.normal, halfDir), 0.0);
    float spec = pow(NdotH, 32.0) * 0.5;
    float3 specular = lightColor * spec;
    
    // Fresnel rim
    float fresnel = pow(1.0 - max(dot(in.normal, viewDir), 0.0), 3.0);
    float3 rim = float3(0.1, 0.15, 0.2) * fresnel;
    
    // Base color
    float3 baseColor = in.color.rgb;
    
    // Damage effect (red tint + cracks pattern)
    if (in.damage > 0.0) {
        float crackPattern = fract(sin(dot(in.worldPos.xz, float2(12.9898, 78.233))) * 43758.5453);
        float crackMask = step(0.8 - in.damage * 0.3, crackPattern);
        baseColor = mix(baseColor, float3(0.3, 0.1, 0.1), crackMask * in.damage);
    }
    
    // Powered glow effect (subtle pulse)
    if (in.powered > 0.5) {
        float pulse = 0.5 + 0.5 * sin(in.animPhase * 3.0);
        float3 glowColor = float3(0.2, 0.5, 1.0);
        
        // Add glow based on block type (params.w contains block type)
        // Robot head eyes, battery, etc.
        float glowIntensity = 0.0;
        
        // Eye glow for robot head (simplified check)
        if (in.color.b > 0.7 && in.color.r < 0.2) {  // Cyan-ish = eye
            glowIntensity = 0.8 + 0.2 * pulse;
            glowColor = float3(0.0, 0.8, 1.0);
        }
        
        baseColor += glowColor * glowIntensity * 0.3;
    }
    
    // Final color
    float3 finalColor = baseColor * (ambientColor + diffuse) + specular + rim;
    
    // Tone mapping (simple Reinhard)
    finalColor = finalColor / (finalColor + 1.0);
    
    // Gamma
    finalColor = pow(finalColor, float3(1.0 / 2.2));
    
    return float4(finalColor, in.color.a);
}

// ============================================================================
// GHOST BLOCK SHADER (semi-transparent preview)
// ============================================================================

fragment float4 blockFragmentGhost(
    BlockVertexOut in [[stage_in]],
    constant float3& camPos [[buffer(0)]]
) {
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float NdotL = max(dot(in.normal, lightDir), 0.0) * 0.5 + 0.5;
    
    float3 baseColor = in.color.rgb * NdotL;
    
    // Hologram effect
    float scanline = sin(in.worldPos.y * 50.0 + in.animPhase * 5.0) * 0.5 + 0.5;
    float alpha = in.color.a * (0.3 + 0.1 * scanline);
    
    // Edge highlight
    float3 viewDir = normalize(camPos - in.worldPos);
    float edge = 1.0 - abs(dot(in.normal, viewDir));
    baseColor += float3(0.2, 0.5, 1.0) * edge * 0.5;
    
    return float4(baseColor, alpha);
}

// ============================================================================
// OUTLINE SHADER (selection highlight)
// ============================================================================

struct OutlineVertexOut {
    float4 position [[position]];
    float4 color;
};

vertex OutlineVertexOut blockOutlineVertex(
    BlockVertex in [[stage_in]],
    constant BlockInstance* instances [[buffer(1)]],
    constant float4x4& viewProj [[buffer(2)]],
    uint instanceId [[instance_id]]
) {
    BlockInstance inst = instances[instanceId];
    
    // Expand along normal for outline
    float3 expandedPos = in.position + in.normal * 0.02;
    float4 worldPos = inst.modelMatrix * float4(expandedPos, 1.0);
    
    OutlineVertexOut out;
    out.position = viewProj * worldPos;
    out.color = float4(1.0, 0.8, 0.2, 1.0);  // Yellow selection
    
    return out;
}

fragment float4 blockOutlineFragment(OutlineVertexOut in [[stage_in]]) {
    return in.color;
}

// ============================================================================
// WHEEL ANIMATION SHADER
// ============================================================================

struct WheelInstance {
    float4x4 modelMatrix;
    float4 tintColor;
    float rotationAngle;  // Wheel spin
    float steerAngle;     // Y-axis rotation for steering
    float2 padding;
};

vertex BlockVertexOut wheelVertex(
    BlockVertex in [[stage_in]],
    constant WheelInstance* instances [[buffer(1)]],
    constant float4x4& viewProj [[buffer(2)]],
    constant float3& camPos [[buffer(3)]],
    uint instanceId [[instance_id]]
) {
    WheelInstance inst = instances[instanceId];
    
    // Apply wheel rotation around Z axis (spin)
    float c = cos(inst.rotationAngle);
    float s = sin(inst.rotationAngle);
    float3x3 spinRot = float3x3(
        float3(c, s, 0),
        float3(-s, c, 0),
        float3(0, 0, 1)
    );
    
    // Apply steering rotation around Y axis
    float cs = cos(inst.steerAngle);
    float ss = sin(inst.steerAngle);
    float3x3 steerRot = float3x3(
        float3(cs, 0, ss),
        float3(0, 1, 0),
        float3(-ss, 0, cs)
    );
    
    float3 rotatedPos = steerRot * spinRot * in.position;
    float3 rotatedNormal = steerRot * spinRot * in.normal;
    
    float4 worldPos = inst.modelMatrix * float4(rotatedPos, 1.0);
    float3 worldNormal = normalize((inst.modelMatrix * float4(rotatedNormal, 0.0)).xyz);
    
    BlockVertexOut out;
    out.position = viewProj * worldPos;
    out.worldPos = worldPos.xyz;
    out.normal = worldNormal;
    out.uv = in.uv;
    out.color = in.color * inst.tintColor;
    out.damage = 0;
    out.powered = 1;
    out.animPhase = 0;
    
    return out;
}

// ============================================================================
// THRUSTER EFFECT SHADER
// ============================================================================

struct ThrusterVertexOut {
    float4 position [[position]];
    float3 worldPos;
    float2 uv;
    float intensity;
    float time;
};

vertex ThrusterVertexOut thrusterEffectVertex(
    uint vertexId [[vertex_id]],
    constant float4x4& modelMatrix [[buffer(0)]],
    constant float4x4& viewProj [[buffer(1)]],
    constant float& intensity [[buffer(2)]],
    constant float& time [[buffer(3)]]
) {
    // Quad billboard pour l'effet de flamme
    float2 corners[4] = {
        float2(-1, 0), float2(1, 0), float2(-1, 1), float2(1, 1)
    };
    
    float2 corner = corners[vertexId];
    float scale = 0.3 + intensity * 0.5;
    
    // Animate flame
    float wave = sin(time * 20.0 + corner.y * 5.0) * 0.1 * intensity;
    corner.x += wave;
    corner *= scale;
    
    float4 localPos = float4(corner.x, corner.y * 0.8, 0, 1);
    float4 worldPos = modelMatrix * localPos;
    
    ThrusterVertexOut out;
    out.position = viewProj * worldPos;
    out.worldPos = worldPos.xyz;
    out.uv = corner * 0.5 + 0.5;
    out.intensity = intensity;
    out.time = time;
    
    return out;
}

fragment float4 thrusterEffectFragment(ThrusterVertexOut in [[stage_in]]) {
    // Distance from center
    float2 centered = in.uv - 0.5;
    float dist = length(centered);
    
    // Flame shape
    float flame = 1.0 - smoothstep(0.0, 0.5, dist);
    flame *= 1.0 - in.uv.y;  // Fade toward top
    
    // Noise for turbulence
    float noise = fract(sin(dot(in.uv + in.time, float2(12.9898, 78.233))) * 43758.5453);
    flame *= 0.8 + 0.2 * noise;
    
    // Color gradient: blue core -> orange -> yellow tip
    float3 coreColor = float3(0.2, 0.5, 1.0);
    float3 midColor = float3(1.0, 0.5, 0.1);
    float3 tipColor = float3(1.0, 0.9, 0.3);
    
    float3 color = mix(coreColor, midColor, in.uv.y);
    color = mix(color, tipColor, pow(in.uv.y, 2.0));
    
    float alpha = flame * in.intensity;
    
    return float4(color * (1.0 + alpha), alpha);
}

float hashcube(float2 p) {
    return fract(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

float hash3(float3 p) {
    return fract(sin(dot(p, float3(127.1, 311.7, 74.7))) * 43758.5453);
}

float noiseforcube(float2 p) {
    float2 i = floor(p);
    float2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hashcube(i);
    float b = hashcube(i + float2(1, 0));
    float c = hashcube(i + float2(0, 1));
    float d = hashcube(i + float2(1, 1));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float noise3(float3 p) {
    float3 i = floor(p);
    float3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = mix(
        mix(mix(hash3(i), hash3(i + float3(1,0,0)), f.x),
            mix(hash3(i + float3(0,1,0)), hash3(i + float3(1,1,0)), f.x), f.y),
        mix(mix(hash3(i + float3(0,0,1)), hash3(i + float3(1,0,1)), f.x),
            mix(hash3(i + float3(0,1,1)), hash3(i + float3(1,1,1)), f.x), f.y),
        f.z);
    return n;
}

float fbmcube(float2 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noiseforcube(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// ============================================================================
// PROCEDURAL TEXTURES
// ============================================================================

// Metal brushed / acier brossé
float3 textureMetal(float3 worldPos, float3 normal, float3 baseColor) {
    float2 uv = worldPos.xz * 10.0;
    
    // Scratches directionnelles
    float scratches = noiseforcube(float2(uv.x * 50.0, uv.y * 2.0)) * 0.5 + noiseforcube(float2(uv.x * 100.0, uv.y * 4.0)) * 0.3;
    
    // Variation subtile
    float variation = fbmcube(uv * 5.0, 3) * 0.1;
    
    return baseColor * (0.9 + scratches * 0.15 + variation);
}

// Rubber / caoutchouc (pour pneus)
float3 textureRubber(float3 worldPos, float3 normal, float3 baseColor) {
    float3 p = worldPos * 20.0;
    
    // Micro-texture granuleuse
    float grain = noise3(p * 10.0) * 0.5 + noise3(p * 20.0) * 0.25;
    
    // Usure sur les bords
    float wear = smoothstep(0.3, 0.7, abs(normal.y)) * 0.1;
    
    return baseColor * (0.85 + grain * 0.2 - wear);
}

// Panel / panneau tech avec lignes
float3 texturePanel(float3 worldPos, float3 normal, float3 baseColor, float blockId) {
    // Projette UV selon la normale dominante
    float2 uv;
    if (abs(normal.y) > 0.5) {
        uv = worldPos.xz;
    } else if (abs(normal.x) > 0.5) {
        uv = worldPos.yz;
    } else {
        uv = worldPos.xy;
    }
    uv *= 2.0;
    
    // Grille de panneaux
    float2 grid = fract(uv * 2.0);
    float2 gridId = floor(uv * 2.0);
    
    // Lignes de séparation
    float lineX = smoothstep(0.02, 0.04, grid.x) * smoothstep(0.02, 0.04, 1.0 - grid.x);
    float lineY = smoothstep(0.02, 0.04, grid.y) * smoothstep(0.02, 0.04, 1.0 - grid.y);
    float lines = lineX * lineY;
    
    // Variation par panneau
    float panelVar = hashcube(gridId + blockId) * 0.1;
    
    // Rivets aux coins
    float2 corner = abs(grid - 0.5);
    float rivet = 1.0 - smoothstep(0.35, 0.4, max(corner.x, corner.y));
    rivet *= smoothstep(0.42, 0.45, max(corner.x, corner.y));
    
    float3 color = baseColor * (0.9 + panelVar);
    color *= lines;
    color = mix(color, color * 0.7, rivet * 0.5);
    
    return color;
}

// Glow / émissif (pour yeux robot, thrusters)
float3 textureGlow(float3 worldPos, float time, float3 glowColor, float intensity) {
    float pulse = 0.8 + 0.2 * sin(time * 3.0);
    float flicker = 0.95 + 0.05 * noiseforcube(float2(time * 10.0, 0.0));
    
    // Core plus brillant
    float core = 1.0 + intensity * 0.5;
    
    return glowColor * pulse * flicker * core;
}

// Dirt / usure
float applyDirt(float3 worldPos, float3 normal, float amount) {
    // Accumulation dans les creux (AO fake)
    float ao = smoothstep(-0.2, 0.5, normal.y) * 0.3 + 0.7;
    
    // Dirt aléatoire
    float dirt = fbmcube(worldPos.xz * 3.0, 4);
    dirt = smoothstep(0.3, 0.7, dirt) * amount;
    
    return ao * (1.0 - dirt * 0.3);
}

// ============================================================================
// BLOCK TYPE TEXTURING
// ============================================================================

// blockType: 0=metal, 1=rubber, 2=panel, 3=glow
float3 getProceduralColor(float3 worldPos, float3 normal, float3 baseColor,
                          int blockType, float blockId, float time, float damage) {
    float3 color;
    
    switch (blockType) {
        case 0: // Metal (cubes, structure)
            color = textureMetal(worldPos, normal, baseColor);
            break;
            
        case 1: // Rubber (wheels)
            color = textureRubber(worldPos, normal, baseColor);
            break;
            
        case 2: // Panel (cockpit, armor)
            color = texturePanel(worldPos, normal, baseColor, blockId);
            break;
            
        case 3: // Glow (eyes, thrusters)
            color = textureGlow(worldPos, time, baseColor, 1.0);
            return color; // Skip dirt for emissive
            
        default:
            color = baseColor;
            break;
    }
    
    // Apply dirt/wear
    color *= applyDirt(worldPos, normal, 0.3 + damage * 0.5);
    
    // Damage cracks
    if (damage > 0.0) {
        float crack = step(0.8 - damage * 0.3, noiseforcube(worldPos.xz * 20.0));
        color = mix(color, float3(0.1, 0.05, 0.02), crack * damage);
    }
    
    return color;
}

fragment float4 blockFragmentProcedural(
    BlockVertexOut in [[stage_in]],
    constant float3& camPos [[buffer(0)]],
    constant float& time [[buffer(1)]]
) {
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float3 lightColor = float3(1.0, 0.98, 0.95);
    float3 ambientColor = float3(0.15, 0.18, 0.25);
    
    float3 viewDir = normalize(camPos - in.worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    
    // Get procedural texture
    int blockType = int(in.blockType);
    float blockId = fract(in.worldPos.x * 0.1 + in.worldPos.z * 0.07);
    
    float3 baseColor = getProceduralColor(
        in.worldPos,
        in.normal,
        in.color.rgb,
        blockType,
        blockId,
        time,
        in.damage
    );
    
    // Lighting
    float NdotL = max(dot(in.normal, lightDir), 0.0);
    float3 diffuse = lightColor * NdotL;
    
    float NdotH = max(dot(in.normal, halfDir), 0.0);
    float spec = pow(NdotH, 32.0) * 0.5;
    
    // Metallic reflection for metal blocks
    if (blockType == 0 || blockType == 2) {
        spec *= 1.5;
    }
    
    float3 specular = lightColor * spec;
    
    // Fresnel
    float fresnel = pow(1.0 - max(dot(in.normal, viewDir), 0.0), 3.0);
    float3 rim = float3(0.1, 0.15, 0.2) * fresnel;
    
    // Emissive for glow blocks
    float3 emissive = float3(0);
    if (blockType == 3 && in.powered > 0.5) {
        emissive = baseColor * 2.0;
        baseColor *= 0.3; // Reduce diffuse for emissive
    }
    
    float3 finalColor = baseColor * (ambientColor + diffuse) + specular + rim + emissive;
    
    // Tonemap
    finalColor = finalColor / (finalColor + 1.0);
    finalColor = pow(finalColor, float3(1.0 / 2.2));
    
    return float4(finalColor, in.color.a);
}


























// ============================================================================
// VERTEX SHADER
// ============================================================================

vertex BlockFragIn blockVertexOptimized(
    BlockVertex in [[stage_in]],
    constant BlockInstance* instances [[buffer(1)]],
    constant BlockUniforms& uniforms [[buffer(2)]],
    uint instanceId [[instance_id]]
                                        ) {
    BlockInstance inst = instances[instanceId];
    
    float4 worldPos = inst.modelMatrix * float4(in.position, 1.0);
    float3 worldNormal = normalize((inst.modelMatrix * float4(in.normal, 0.0)).xyz);
    
    BlockFragIn out;
    out.position = uniforms.viewProj * worldPos;
    out.worldPos = worldPos.xyz;
    out.worldNormal = worldNormal;
    out.uv = in.uv;
    out.color = in.color * inst.tintColor;
    out.damage = inst.params.x;
    out.powered = inst.params.y;
    out.time = inst.params.z;
    out.textureIndex = uint(inst.params.w);
    
    return out;
}

float geometrySmithCube(float NdotV, float NdotL, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

fragment float4 blockFragmentPBR(
    BlockFragIn in [[stage_in]],
    constant BlockUniforms& uniforms [[buffer(0)]],
    texture2d_array<float> textures [[texture(0)]],
    sampler texSampler [[sampler(0)]])
{
    float4 texColor = textures.sample(texSampler, in.uv, in.textureIndex);
    float3 albedo = texColor.rgb * in.color.rgb;
    
    // Material params (could be from texture)
    float metallic = 0.1;
    float roughness = 0.5;
    float ao = 1.0;
    
    // Vectors
    float3 N = normalize(in.worldNormal);
    float3 V = normalize(uniforms.camPos - in.worldPos);
    float3 L = normalize(uniforms.lightDir);
    float3 H = normalize(V + L);
    
    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    
    // F0 for dielectrics/metals
    float3 F0 = mix(float3(0.04), albedo, metallic);
    
    // Cook-Torrance BRDF
    float D = distributionGGX(N, H, roughness);
    float G = geometrySmithCube(NdotV, NdotL, roughness);
    float3 F = fresnelSchlick(HdotV, F0);
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    
    float3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    float3 specular = numerator / denominator;
    
    // Light contribution
    float3 radiance = float3(2.5); // Light intensity
    float3 Lo = (kD * albedo / M_PI_F + specular) * radiance * NdotL;
    
    // Ambient (simple IBL approximation)
    float3 ambient = float3(0.03) * albedo * ao;
    
    // Fresnel rim
    float fresnel = pow(1.0 - NdotV, 4.0);
    float3 rim = float3(0.1, 0.12, 0.15) * fresnel * (1.0 - roughness);
    
    // Damage effect
    if (in.damage > 0.0) {
        float crack = step(0.7 - in.damage * 0.3,
                          fract(sin(dot(in.worldPos.xz * 20.0, float2(12.9898, 78.233))) * 43758.5453));
        albedo = mix(albedo, float3(0.15, 0.08, 0.05), crack * in.damage);
    }
    
    // Powered glow
    float3 emissive = float3(0);
    if (in.powered > 0.5 && in.color.b > 0.6) {
        float pulse = 0.7 + 0.3 * sin(in.time * 4.0);
        emissive = in.color.rgb * pulse * 0.5;
    }
    
    float3 color = ambient + Lo + rim + emissive;
    
    // Tonemap (ACES)
    color = (color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14);
    
    // Gamma
    color = pow(saturate(color), float3(1.0 / 2.2));
    
    return float4(color, in.color.a);
}

// ============================================================================
// FRAGMENT SHADER - TRANSPARENT (Glass, Ghost)
// ============================================================================

fragment float4 blockFragmentTransparent(
    BlockFragIn in [[stage_in]],
    constant BlockUniforms& uniforms [[buffer(0)]],
    texture2d_array<float> textures [[texture(0)]],
    sampler texSampler [[sampler(0)]]
) {
    float4 texColor = textures.sample(texSampler, in.uv, in.textureIndex);
    
    float3 N = normalize(in.worldNormal);
    float3 V = normalize(uniforms.camPos - in.worldPos);
    float NdotV = max(dot(N, V), 0.0);
    
    // Fresnel for glass
    float fresnel = pow(1.0 - NdotV, 3.0);
    
    // Base glass color
    float3 glassColor = in.color.rgb * texColor.rgb;
    
    // Reflection tint
    float3 reflectColor = float3(0.6, 0.7, 0.8);
    
    // Mix based on fresnel
    float3 color = mix(glassColor * 0.3, reflectColor, fresnel * 0.7);
    
    // Scanline effect for ghost/hologram
    if (in.color.a < 0.9) {
        float scanline = sin(in.worldPos.y * 50.0 + in.time * 3.0) * 0.5 + 0.5;
        color += float3(0.1, 0.3, 0.5) * scanline * 0.2;
    }
    
    // Alpha based on fresnel and base alpha
    float alpha = in.color.a * (0.3 + fresnel * 0.5);
    
    return float4(color, alpha);
}

// ============================================================================
// SIMPLE FALLBACK (no textures)
// ============================================================================

fragment float4 blockFragmentSimple(
    BlockFragIn in [[stage_in]],
    constant BlockUniforms& uniforms [[buffer(0)]]
) {
    float3 N = normalize(in.worldNormal);
    float3 L = normalize(uniforms.lightDir);
    float3 V = normalize(uniforms.camPos - in.worldPos);
    float3 H = normalize(L + V);
    
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    
    // Simple lighting
    float3 ambient = float3(0.15, 0.17, 0.2);
    float3 diffuse = float3(1.0) * NdotL;
    float3 specular = float3(0.5) * pow(NdotH, 32.0);
    float fresnel = pow(1.0 - NdotV, 3.0) * 0.2;
    
    float3 color = in.color.rgb * (ambient + diffuse) + specular + fresnel;
    
    // Gamma
    color = pow(saturate(color), float3(1.0 / 2.2));
    
    return float4(color, in.color.a);
}
