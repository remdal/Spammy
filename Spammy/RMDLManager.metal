//
//  RMDLManager.metal
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct TerraBlockVertex
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
    float4 color    [[attribute(3)]];
};

struct TerraBlockInstance
{
    float4x4 modelMatrix;
    float4 tint;
    uint typeID;
    uint state;         // 0=solid, 1=ghost, 2=damaged
    float healthRatio;
    float _pad;
};

struct TerraUniforms
{
    float4x4 viewProjection;
    float3 cameraPos;
    float time;
    float3 sunDir;
    float _pad;
};

struct TerraVaryings
{
    float4 position [[position]];
    float3 worldPos;
    float3 worldNormal;
    float2 uv;
    float4 color;
    float healthRatio;
    uint state;
};

vertex TerraVaryings terraBlockVS(TerraBlockVertex in [[stage_in]],
                                  constant TerraUniforms& uniforms [[buffer(1)]],
                                  constant TerraBlockInstance* instances [[buffer(2)]],
                                  uint instanceID [[instance_id]])
{
    TerraVaryings out;
    
    TerraBlockInstance inst = instances[instanceID];
    
    float4 worldPos = inst.modelMatrix * float4(in.position, 1.0);
    out.position = uniforms.viewProjection * worldPos;
    out.worldPos = worldPos.xyz;
    
    float3x3 normalMat = float3x3(inst.modelMatrix[0].xyz,
                                   inst.modelMatrix[1].xyz,
                                   inst.modelMatrix[2].xyz);
    out.worldNormal = normalize(normalMat * in.normal);
    
    out.uv = in.uv;
    out.color = in.color * inst.tint;
    out.healthRatio = inst.healthRatio;
    out.state = inst.state;
    
    return out;
}

fragment float4 terraBlockFS(TerraVaryings in [[stage_in]],
                             constant TerraUniforms& uniforms [[buffer(1)]])
{
    float3 N = normalize(in.worldNormal);
    float3 L = normalize(uniforms.sunDir);
    float3 V = normalize(uniforms.cameraPos - in.worldPos);
    float3 H = normalize(L + V);
    
    // Lighting
    float NdotL = max(dot(N, L), 0.0);
    float ambient = 0.35;
    float diffuse = NdotL * 0.55;
    
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 48.0) * 0.35;
    
    float3 baseColor = in.color.rgb;
    
    // Damage effect
    if (in.healthRatio < 1.0) {
        float damage = 1.0 - in.healthRatio;
        
        // Darken and redden
        baseColor = mix(baseColor, float3(0.15, 0.08, 0.05), damage * 0.5);
        
        // Scratch pattern
        float scratch = fract(sin(dot(in.uv * 25.0, float2(12.9898, 78.233))) * 43758.5453);
        if (scratch < damage * 0.25) {
            baseColor *= 0.6;
        }
    }
    
    float3 finalColor = baseColor * (ambient + diffuse) + float3(1.0) * spec;
    
    // Fresnel rim
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);
    finalColor += float3(0.25, 0.45, 0.7) * fresnel * 0.2;
    
    // Edge highlight
    float edge = 1.0 - smoothstep(0.0, 0.15, min(min(in.uv.x, 1.0 - in.uv.x), min(in.uv.y, 1.0 - in.uv.y)));
    finalColor += float3(0.3) * edge * 0.15;
    
    return float4(finalColor, in.color.a);
}

// ────────────────────────────────────────────────────────────────────────────
// FRAGMENT SHADER - GHOST BLOCK (placement preview)
// ────────────────────────────────────────────────────────────────────────────

fragment float4 terraGhostFS(
    TerraVaryings in [[stage_in]],
    constant TerraUniforms& uniforms [[buffer(1)]])
{
    float3 N = normalize(in.worldNormal);
    float3 V = normalize(uniforms.cameraPos - in.worldPos);
    
    // Pulsing
    float pulse = sin(uniforms.time * 4.5) * 0.12 + 0.88;
    
    // Holographic grid
    float2 grid = fract(in.uv * 5.0);
    float gridLine = step(0.88, grid.x) + step(0.88, grid.y);
    gridLine = min(gridLine, 1.0);
    
    // Scanning line effect
    float scan = step(0.98, fract(in.worldPos.y * 2.0 - uniforms.time * 1.5));
    
    // Fresnel glow
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 2.5);
    
    float3 color = in.color.rgb * pulse;
    color += gridLine * 0.25;
    color += scan * 0.15;
    color += fresnel * 0.35;
    
    float alpha = in.color.a * (0.45 + gridLine * 0.15 + fresnel * 0.25 + scan * 0.1);
    
    return float4(color, alpha);
}

// ────────────────────────────────────────────────────────────────────────────
// INVENTORY UI SHADERS (optionnel - peut utiliser ton UI existant)
// ────────────────────────────────────────────────────────────────────────────

struct InventoryUIVertex {
    float2 position [[attribute(0)]];
    float2 uv       [[attribute(1)]];
};

struct InventoryUIVaryings {
    float4 position [[position]];
    float2 uv;
    float2 localPos;
};

struct InventoryUIUniforms {
    float2 screenSize;
    float2 panelPosition;
    float2 panelSize;
    float2 slotSize;
    float slotPadding;
    float time;
    int hoveredSlot;
    int selectedSlot;
};

vertex InventoryUIVaryings inventoryUIVS(
    InventoryUIVertex in [[stage_in]],
    constant InventoryUIUniforms& uniforms [[buffer(1)]],
    uint instanceID [[instance_id]])
{
    InventoryUIVaryings out;
    
    // Calculer position du slot
    int col = instanceID % 10;
    int row = instanceID / 10;
    
    float2 slotPos;
    slotPos.x = uniforms.panelPosition.x + col * (uniforms.slotSize.x + uniforms.slotPadding);
    slotPos.y = uniforms.panelPosition.y + row * (uniforms.slotSize.y + uniforms.slotPadding);
    
    float2 vertPos = slotPos + in.position * uniforms.slotSize;
    
    // Convert to clip space
    out.position = float4(vertPos * 2.0 - 1.0, 0.0, 1.0);
    out.position.y = -out.position.y;
    
    out.uv = in.uv;
    out.localPos = in.position;
    
    return out;
}

fragment float4 inventoryUIFS(
    InventoryUIVaryings in [[stage_in]],
    constant InventoryUIUniforms& uniforms [[buffer(1)]])
{
    // Rounded rectangle
    float2 center = in.localPos - 0.5;
    float2 size = float2(0.45);
    float radius = 0.08;
    
    float2 q = abs(center) - size + radius;
    float d = min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
    
    float aa = fwidth(d) * 1.2;
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    
    // Background color
    float3 bgColor = float3(0.12, 0.14, 0.18);
    
    // Border
    float borderD = abs(d) - 0.02;
    float border = 1.0 - smoothstep(-aa, aa, borderD);
    float3 borderColor = float3(0.35, 0.4, 0.5);
    
    float3 finalColor = mix(bgColor, borderColor, border * 0.6);
    
    // Hover/selection highlight
    float highlight = sin(uniforms.time * 3.0) * 0.1 + 0.9;
    finalColor *= highlight;
    
    return float4(finalColor, alpha * 0.92);
}
