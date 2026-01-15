//
//  RMDLMouse.metal.metal
//  Spammy
//
//  Created by Rémy on 15/01/2026.
//

#include <metal_stdlib>
using namespace metal;

#import "RMDLMainRenderer_shared.h"
#import "Helpers.metal"

struct LightingVtxOut
{
    float4 position [[position]];
};

vertex LightingVtxOut LightingVs(uint vid [[vertex_id]])
{
    const float2 vertices[] =
    {
        float2(-1, -1),
        float2(-1,  3),
        float2( 3, -1)
    };

    LightingVtxOut out;
    out.position = float4(vertices[vid], 1.0, 1.0);
    return out;
}

struct LightingPsOut
{
    float4 backbuffer [[color(0)]];
};

static float3 GetWorldPositionAndViewDirFromDepth(uint2 tid, float depth, constant RMDLUniforms& uniforms, thread float3& outViewDirection)
{
    float4 ndc;
    ndc.xy = (float2(tid) + 0.5) * uniforms.invScreenSize;
    ndc.xy = ndc.xy * 2 - 1;
    ndc.y *= -1;

    ndc.z = depth;
    ndc.w = 1;

    float4 worldPosition = uniforms.cameraUniforms.invViewProjectionMatrix * ndc;
    worldPosition.xyz /= worldPosition.w;

    ndc.z = 1.f;
    float4 viewDir = uniforms.cameraUniforms.invOrientationProjectionMatrix * ndc;
    viewDir /= viewDir.w;
    outViewDirection = viewDir.xyz;

    return worldPosition.xyz;
}

float evaluateModificationCross(simd::float2 worldPosition, simd::float4 mousePosition, float size)
{
    float dist = simd::length(worldPosition - mousePosition.xz);
    dist /= size;
    return saturate(min(2 - dist, 1.0 / (1.0 + pow(dist * 2.0, 4))));
}

float4 visualizeModificationCross(constant RMDLUniforms& globalUniforms, float3 worldPosition, float4 mousePosition)
{
    float cross = evaluateModificationCross(worldPosition.xz, mousePosition, globalUniforms.brushSize);
    float3 color = globalUniforms.mouseState.z == 2 ? float3(0.9, 0.5, 0.5) :
    globalUniforms.mouseState.z == 1 ? float3(0.5, 0.9, 0.5) : float3(0.5, 0.5, 0.7);

    float waveVisibility = min(cross, (1.f - cross) * 20 + 0.5f);
    return float4(color, waveVisibility * (0.5 + .2 * sin(cross * 20.0 + GAME_TIME * 5.0)));
}

fragment LightingPsOut LightingPs(LightingVtxOut                   in              [[stage_in]],
                                  texture2d <float, access::read>  gBuffer0        [[texture (0)]],
                                  texture2d <float, access::read>  gBuffer1        [[texture (1)]],
                                  depth2d <float, access::read>    depthBuffer     [[texture (2)]],
                                  depth2d_array <float>            shadowMap       [[texture (3)]],
                                  texturecube <float>              cubemap         [[texture (4)]],
                                  texture2d <float>                perlinMap       [[texture (5)]],
                                  constant RMDLUniforms&           uniforms        [[buffer  (0)]],
                                  constant float4&                 mouseWorldPos   [[buffer  (1)]])
{
    constexpr sampler colorSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear);

    const uint2 pixelPos = uint2(floor(in.position.xy));

    const float depth = depthBuffer.read(pixelPos);

    float3 viewDir;
    const float3 worldPosition =
    GetWorldPositionAndViewDirFromDepth(pixelPos, depth, uniforms, viewDir);

    if (depth == 1)
    {
        float3 cubemapColor = cubemap.sample(colorSampler, viewDir, level(0)).xyz;

        LightingPsOut res;
        res.backbuffer = float4(cubemapColor, 1);
        return res;
    }

    float4 gBuffer0Value = gBuffer0.read(pixelPos);
    float4 gBuffer1Value = gBuffer1.read(pixelPos);

    BrdfProperties brdfProps = UnpackBrdfProperties(gBuffer0Value, gBuffer1Value);

//    const float shadowAmount = evaluate_shadow(uniforms, worldPosition, depth, shadowMap, perlinMap);

    // Sun direction is hardcoded since we use a fixed cubemap
    const float3 sunDirection = float3(-1, 0.7, -0.5);

    // How much illumination the current fragment receives
    // - it depends on the normal and whether or not it is shadowed
    const float nDotL = saturate(dot(sunDirection, brdfProps.normal)) /** shadowAmount*/ * 1.2;

    // For the ambient color, we'll sample the cubemap. Another approach can be considerered however using have an irradiance map
    // - Note: we don't want to sample too close to the horizon, as the texture becomes very white at altitude 0, due to the haze / scattering
    const float3 ambientDirectionUp = float3(0,1,0);
    const float3 ambientDirectionHoriz = normalize(float3(-sunDirection.x, 0.1, -sunDirection.z));
    const float3 ambientDirection = normalize(mix(ambientDirectionHoriz, ambientDirectionUp, brdfProps.normal.y));
    const float3 ambientColorBase = saturate(cubemap.sample(colorSampler, ambientDirection, level(0)).xyz * 1.5 + 0.1);
    const float3 ambientColor = ambientColorBase * max(0.05, brdfProps.normal.y);

    float3 color = brdfProps.albedo * (ambientColor + float3(nDotL));

    // Add an atmospherics blend; it is absolutely empirical.
    float hazeAmount;
    {
        const float near = 0.992;
        const float far = 1.0;
        const float invFarByNear = 1.0 / (far - near);
        const float approxlinDepth = saturate((depth - near) * invFarByNear);
        hazeAmount = pow(approxlinDepth, 10) * 0.3;
    }
    const float3 hazeColor = saturate(cubemap.sample(colorSampler, float3(0, 1, 0)).xyz * 3.0 + 0.1);
    color = mix(color, hazeColor, float3(hazeAmount));

    float4 brush = visualizeModificationCross(uniforms, worldPosition, mouseWorldPos);
    color = mix(color, brush.xyz, brush.w);

    LightingPsOut res;
    res.backbuffer = float4(color, 1);
    return res;
}

kernel void mousePositionUpdate(depth2d<float, access::read> depthBuffer [[texture(0)]],
                                constant RMDLUniforms& globalUniforms    [[buffer(0)]],
                                device float4& outWorldPos               [[buffer(1)]],
                                uint2 tid                                [[thread_position_in_grid]])
{
    const uint2 mousePos    = uint2(floor(globalUniforms.mouseState.xy));
    const float mouseDepth  = depthBuffer.read(mousePos);
    float3 viewDir;
    const float3 worldMousePosition = GetWorldPositionAndViewDirFromDepth(mousePos, mouseDepth, globalUniforms, viewDir);
    outWorldPos = float4(worldMousePosition, 666.0);
}
