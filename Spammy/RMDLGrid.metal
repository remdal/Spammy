//
//  RMDLGrid.metal
//  Spammy
//
//  Created by Rémy on 16/01/2026.
//

#include <metal_stdlib>
using namespace metal;

#include "RMDLMainRenderer_shared.h"

struct GridVertexIn
{
    float3 position [[attribute(0)]];
};

struct GridVertexOut
{
    float4 position [[position]];
    float3 worldPosition;
    float distanceToCamera;
};

vertex GridVertexOut gridVertexShader(GridVertexIn in [[stage_in]],
                                       constant GridUniforms& uniforms [[buffer(1)]])
{
    GridVertexOut out;
    
    float3 worldPos = in.position + uniforms.gridCenter;
    out.worldPosition = worldPos;
    out.position = uniforms.viewProjectionMatrix * float4(worldPos, 1.0);
    out.distanceToCamera = length(worldPos - uniforms.cameraPosition);
    
    return out;
}

fragment float4 gridFragmentShader(GridVertexOut in [[stage_in]],
                                    constant GridUniforms& uniforms [[buffer(0)]])
{
    float4 color = uniforms.edgeColor;
    
    // Fade basé sur la distance
    float fadeFactor = 1.0 - saturate(in.distanceToCamera / uniforms.fadeDistance);
    fadeFactor = fadeFactor * fadeFactor; // Courbe quadratique pour un fade plus naturel
    
    color.a *= fadeFactor;
    
    // Discard si trop transparent
    if (color.a < 0.01)
        discard_fragment();
    
    return color;
}

