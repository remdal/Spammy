//
//  RectangleUI.metal
//  Spammy
//
//  Created by Rémy on 02/01/2026.
//

#include <metal_stdlib>
using namespace metal;

#include "../RMDLMainRenderer_shared.h"

struct RasterizerData
{
    float4 position [[position]];
    float4 color;
};

vertex RasterizerData vertexShaderRectangle(uint vertexID [[vertex_id]],
                                           constant VertexRectangle *vertexData [[buffer(0)]],
                                           constant simd_uint2 *viewportSizePointer [[buffer(1)]])
{
    RasterizerData out;

    simd_float2 pixelSpacePosition = vertexData[vertexID].position.xy;

    simd_float2 viewportSize = simd_float2(*viewportSizePointer);

    out.position.xy = pixelSpacePosition / (viewportSize / 2.0);
    out.position.z = 0.0;
    out.position.w = 1.0;
    out.color = vertexData[vertexID].color;

    return out;
}

fragment float4 fragmentShaderRectangle(RasterizerData in [[stage_in]])
{
    return in.color;
}
