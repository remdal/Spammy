//
//  RMDLInventory.metal
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct InventoryVertexIn
{
    float2 position     [[attribute(0)]];
    float2 texCoord     [[attribute(1)]];
    float4 color        [[attribute(2)]];
    float cornerRadius  [[attribute(3)]];
    float borderWidth   [[attribute(4)]];
    uint elementType    [[attribute(5)]];
    uint slotIndex      [[attribute(6)]];
};

struct InventoryVertexOut
{
    float4 position [[position]];
    float2 texCoord;
    float4 color;
    float cornerRadius;
    float borderWidth;
    uint elementType;
    uint slotIndex;
};

struct InventoryUniformsGPU
{
    float2 screenSize;
    float2 panelOrigin;
    float2 slotDimensions;
    float slotSpacing;
    float time;
    int hoveredSlotIndex;
    int selectedSlotIndex;
    float4 slotNormalColor;
    float4 slotHoveredColor;
    float4 slotSelectedColor;
    float4 handleColor;
    float slotCornerRadius;
    float scrollOffset;
    float maxScrollOffset;
    float handleSize;
};

float roundedRectSDF(float2 p, float2 halfSize, float r) {
    float2 d = abs(p) - halfSize + r;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
}

vertex InventoryVertexOut inventoryPanelVertexShader(
    InventoryVertexIn in [[stage_in]],
    constant InventoryUniformsGPU& uniforms [[buffer(1)]])
{
    InventoryVertexOut out;
    
    float2 clip = in.position * 2.0 - 1.0;
    clip.y = -clip.y;
    
    out.position = float4(clip, 0.0, 1.0);
    out.texCoord = in.texCoord;
    out.color = in.color;
    out.cornerRadius = in.cornerRadius;
    out.borderWidth = in.borderWidth;
    out.elementType = in.elementType;
    out.slotIndex = in.slotIndex;
    
    return out;
}

fragment float4 inventorySlotFragmentShader(InventoryVertexOut in [[stage_in]],
                                            constant InventoryUniformsGPU& uniforms [[buffer(0)]])
{
    float2 uv = in.texCoord;
    float2 center = uv - 0.5;
    float2 halfSize = float2(0.5 - 0.025);
    float radius = in.cornerRadius * 0.018;
    
    float d = roundedRectSDF(center, halfSize, radius);
    float aa = fwidth(d) * 1.4;
    float shape = 1.0 - smoothstep(-aa, aa, d);
    
    float4 col = in.color;
    
    if (in.elementType == 0) {
        // Inner shadow/depth
        float innerShadow = smoothstep(0.0, 0.12, -d);
        col.rgb *= 0.88 + innerShadow * 0.12;
        
        // Subtle top highlight
        float topLight = pow(1.0 - uv.y, 2.5) * 0.08;
        col.rgb += topLight;
        
        // Border glow for selected/hovered
        if ((int)in.slotIndex == uniforms.selectedSlotIndex) {
            float borderD = abs(d) - in.borderWidth * 0.012;
            float borderMask = 1.0 - smoothstep(-aa, aa, borderD);
            float pulse = sin(uniforms.time * 2.8) * 0.12 + 0.88;
            float3 glowColor = float3(0.35, 0.65, 1.0) * pulse;
            col.rgb = mix(col.rgb, glowColor, borderMask * 0.75);
        }
        else if ((int)in.slotIndex == uniforms.hoveredSlotIndex) {
            float borderD = abs(d) - in.borderWidth * 0.01;
            float borderMask = 1.0 - smoothstep(-aa, aa, borderD);
            col.rgb = mix(col.rgb, float3(0.55, 0.6, 0.7), borderMask * 0.5);
        }
    }
    // Icon
    else if (in.elementType == 1) {
        float bevel = 1.0 - smoothstep(0.0, 0.08, -d) * 0.15;
        col.rgb *= bevel;
        
        float shine = pow(max(0.0, 1.0 - uv.y * 1.4), 2.2) * 0.22;
        col.rgb += shine;
    }
    // Handle
    else if (in.elementType == 2) {
        float centerGlow = 1.0 - length(center) * 1.6;
        centerGlow = max(0.0, centerGlow);
        col.rgb += centerGlow * 0.15;
        
        float edgeFade = smoothstep(0.0, 0.08, -d);
        col.a *= 0.7 + edgeFade * 0.3;
    }
    // Scroll indicator
    else if (in.elementType == 3) {
        float pulse = sin(uniforms.time * 3.5) * 0.25 + 0.75;
        col.a *= pulse;
    }
    
    col.a *= shape;
    return col;
}

fragment float4 inventoryIconFragmentShader(InventoryVertexOut in [[stage_in]],
                                            constant InventoryUniformsGPU& uniforms [[buffer(0)]],
                                            texture2d<float> iconTexture [[texture(0)]],
                                            sampler iconSampler [[sampler(0)]])
{
    float2 uv = in.texCoord;
    float4 texColor = iconTexture.sample(iconSampler, uv);
    
    float2 center = uv - 0.5;
    float2 halfSize = float2(0.5 - 0.02);
    float radius = 0.06;
    float d = roundedRectSDF(center, halfSize, radius);
    float aa = fwidth(d) * 1.4;
    float shape = 1.0 - smoothstep(-aa, aa, d);
    
    texColor.a *= shape;
    texColor.rgb *= in.color.rgb;
    
    return texColor;
}
