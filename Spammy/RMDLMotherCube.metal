//
//  RMDLMotherCube.metal
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct BlockVertex {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
    float4 color    [[attribute(3)]];
};

struct BlockInstance {
    float4x4 modelMatrix;
    float4 tintColor;
    float4 params;  // x=damage, y=powered, z=animPhase, w=blockTypeId
};

struct BlockVertexOut {
    float4 position [[position]];
    float3 worldPos;
    float3 normal;
    float2 uv;
    float4 color;
    float damage;
    float powered;
    float animPhase;
};

// ============================================================================
// VERTEX SHADER
// ============================================================================

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
