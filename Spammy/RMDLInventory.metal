//
//  RMDLInventory.metal
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct InventoryPanelVertexIn
{
    float2 position     [[attribute(0)]];
    float2 texCoord     [[attribute(1)]];
    float4 color        [[attribute(2)]];
    float cornerRadius  [[attribute(3)]];
    float borderWidth   [[attribute(4)]];
    uint slotIndex      [[attribute(5)]];
    uint flags          [[attribute(6)]];
};

struct InventoryPanelVertexOut
{
    float4 position [[position]];
    float2 texCoord;
    float4 color;
    float2 quadSize;
    float cornerRadius;
    float borderWidth;
    uint slotIndex;
    uint flags;
};

struct InventoryPanelUniformsGPU
{
    float2 screenSize;
    float2 panelOrigin;
    float2 panelSize;
    float2 slotDimensions;
    float slotSpacing;
    float time;
    int hoveredSlotIndex;
    int selectedSlotIndex;
    float4 panelBackgroundColor;
    float4 slotNormalColor;
    float4 slotHoveredColor;
    float4 slotSelectedColor;
    float4 titleBarColor;
    float titleBarHeight;
    float panelCornerRadius;
    float slotCornerRadius;
    float borderThickness;
};

float roundedBoxSDF(float2 centerPos, float2 halfSize, float radius)
{
    float2 q = abs(centerPos) - halfSize + radius;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

vertex InventoryPanelVertexOut inventoryPanelVertexShader(InventoryPanelVertexIn in [[stage_in]],
                                                          constant InventoryPanelUniformsGPU& uniforms [[buffer(1)]])
{
    InventoryPanelVertexOut out;
    
    float2 clipPos = in.position * 2.0 - 1.0;
    clipPos.y = -clipPos.y;
    
    out.position = float4(clipPos, 0.0, 1.0);
    out.texCoord = in.texCoord;
    out.color = in.color;
    out.cornerRadius = in.cornerRadius;
    out.borderWidth = in.borderWidth;
    out.slotIndex = in.slotIndex;
    out.flags = in.flags;
    out.quadSize = float2(1.0);
    
    return out;
}

fragment float4 inventoryPanelFragmentShader(InventoryPanelVertexOut in [[stage_in]],
                                             constant InventoryPanelUniformsGPU& uniforms [[buffer(0)]])
{
    float2 uv = in.texCoord;
    float2 center = uv - 0.5;
    float2 halfSize = float2(0.5 - 0.02);
    
    float radius = in.cornerRadius * 0.02;
    float d = roundedBoxSDF(center, halfSize, radius);
    
    float aa = fwidth(d) * 1.5;
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    
    float4 fillColor = in.color;
    
    // Slot rendering
    if (in.flags == 2)
    {
        // Border glow for hovered/selected
        if ((int)in.slotIndex == uniforms.hoveredSlotIndex || (int)in.slotIndex == uniforms.selectedSlotIndex)
        {
            float borderD = abs(d) - in.borderWidth * 0.015;
            float borderAlpha = 1.0 - smoothstep(-aa, aa, borderD);
            float3 borderColor = float3(0.4, 0.6, 0.9);
            
            if ((int)in.slotIndex == uniforms.selectedSlotIndex)
            {
                float pulse = sin(uniforms.time * 3.0) * 0.15 + 0.85;
                borderColor = float3(0.3, 0.7, 1.0) * pulse;
            }
            
            fillColor.rgb = mix(fillColor.rgb, borderColor, borderAlpha * 0.7);
        }
        
        // Inner shadow
        float innerShadow = smoothstep(0.0, 0.15, -d);
        fillColor.rgb *= (0.85 + innerShadow * 0.15);
    }
    
    // Icon rendering
    if (in.flags == 4)
    {
        // Slight bevel effect
        float bevel = 1.0 - smoothstep(0.0, 0.1, -d) * 0.2;
        fillColor.rgb *= bevel;
        
        // Shine
        float shine = pow(max(0.0, 1.0 - uv.y * 1.5), 2.0) * 0.25;
        fillColor.rgb += shine;
    }
    
    // Background panels
    if (in.flags == 1)
    {
        // Subtle gradient
        float gradient = mix(1.0, 0.92, uv.y);
        fillColor.rgb *= gradient;
        
        // Edge highlight
        float edge = 1.0 - smoothstep(0.0, 0.03, -d);
        fillColor.rgb += edge * 0.08;
    }
    
    fillColor.a *= alpha;
    
    return fillColor;
}
