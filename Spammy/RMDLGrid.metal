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

struct VertexIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
    uchar  planeIdx [[attribute(3)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 worldPos;
    float3 worldNormal;
    float2 uv;
    float  distToCamera;
    uint   planeIndex;
};

vertex VertexOut vehicleGridVertex(VertexIn in [[stage_in]],
                                   constant GridCommandant::VehicleGridUniforms& u [[buffer(1)]])
{
    float4 worldPos4 = u.modelMatrix * float4(in.position, 1.0);
    float3 worldPos = worldPos4.xyz;
    float3 worldNormal = normalize((u.modelMatrix * float4(in.normal, 0.0)).xyz);
    
    VertexOut out;
    out.position = u.viewProjectionMatrix * worldPos4;
    out.worldPos = worldPos;
    out.worldNormal = worldNormal;
    out.uv = in.uv;
    out.distToCamera = length(worldPos - u.cameraPosition);
    out.planeIndex = in.planeIdx;
    return out;
}

fragment float4 vehicleGridFragment(VertexOut in [[stage_in]],
                                    constant GridCommandant::VehicleGridUniforms& u [[buffer(0)]])
{
    // Sélection couleur par plan
    float4 baseColor;
    switch (in.planeIndex) {
        case 0: baseColor = u.gridColorXY; break;
        case 1: baseColor = u.gridColorXZ; break;
        case 2: baseColor = u.gridColorYZ; break;
        default: baseColor = float4(1, 1, 1, 0.5);
    }
    
    // Fade avec la distance
    float fadeFactor = 1.0 - smoothstep(u.fadeDistance * 0.5, u.fadeDistance, in.distToCamera);
    
    // Effet de pulsation
    float pulse = 1.0 + u.pulseIntensity * sin(u.time * 3.0);
    
    // Glow au centre des lignes
    float centerGlow = 1.0 - abs(in.uv.y - 0.5) * 2.0;
    centerGlow = pow(centerGlow, 2.0) * 0.5 + 0.5;
    
    // Distance au centre de la grille pour fade radial
    float3 localPos = in.worldPos - u.gridCenter;
    float radialDist = length(localPos) / (float(u.gridExtent) * u.cellSize);
    float radialFade = 1.0 - smoothstep(0.6, 1.0, radialDist);
    
    // Fresnel pour meilleure visibilité des plans de face
    float3 viewDir = normalize(u.cameraPosition - in.worldPos);
    float fresnel = abs(dot(viewDir, in.worldNormal));
    fresnel = pow(fresnel, 0.5); // Adoucir l'effet
    
    // Combinaison finale
    float4 finalColor = baseColor;
    finalColor.rgb *= pulse * centerGlow;
    finalColor.a *= fadeFactor * radialFade * fresnel;
    
    // Seuil pour éviter les artefacts
    if (finalColor.a < 0.01) discard_fragment();
    
    return finalColor;
}

