//
//  RMDLVoronoiWorld.metal
//  Spammy
//
//  Created by Rémy on 25/12/2025.
//

#include <metal_stdlib>
using namespace metal;

#include "RMDLMainRenderer_shared.h"

struct VoxelVertex
{
    float3 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float3 normal [[attribute(2)]];
};

struct VoxelFragmentInput
{
    float4 position [[position]];
    float4 color;
    float3 normal;
    float3 worldPosition;
};

vertex VoxelFragmentInput voxel_vertex(VoxelVertex in [[stage_in]], constant RMDLCameraUniforms& camera [[buffer(1)]])
{
    VoxelFragmentInput out;
    float4 worldPos = float4(in.position, 1.0);
    out.position = camera.viewProjectionMatrix * worldPos;
    out.worldPosition = in.position;
    out.color = in.color;
    out.normal = in.normal;
    return out;
}

fragment float4 voxel_fragment(VoxelFragmentInput in [[stage_in]], constant RMDLCameraUniforms& camera [[buffer(1)]])
{
    // Lumière directionnelle simple (soleil)
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float3 normal = normalize(in.normal);
    
    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.3;
    float lighting = ambient + diffuse * 0.7;
    
    // Fog basé sur la distance
    float distance = length(in.worldPosition - camera.position);
    float fogStart = 50.0;
    float fogEnd = 150.0;
    float fogFactor = smoothstep(fogStart, fogEnd, distance);
    float4 fogColor = float4(0.6, 0.8, 1.0, 1.0); // Bleu ciel
    
    float4 litColor = in.color * lighting;
    return mix(litColor, fogColor, fogFactor);
}
