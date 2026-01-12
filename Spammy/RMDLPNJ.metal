//
//  RMDLPNJ.metal
//  Spammy
//
//  Created by Rémy on 03/01/2026.
//

#include <metal_stdlib>
using namespace metal;

#include "RMDLMainRenderer_shared.h"
#import "Helpers.metal"

struct Vertex
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    float3 tangent [[attribute(3)]];
    float3 bitangent [[attribute(4)]];
    float4 boneWeights [[attribute(5)]];
    uint4 boneIndices [[attribute(6)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 worldPosition;
    float3 worldNormal;
    float3 worldTangent;
    float3 worldBitangent;
    float2 texCoord;
};

struct InstanceData
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

struct BoneTransform
{
    float4x4 transform;
};

vertex VertexOut vertex_main_pnj(Vertex in [[stage_in]],
                                 constant InstanceData& instance [[buffer(1)]],
                                 constant RMDLCameraUniforms& camera [[buffer(2)]],
                                 constant BoneTransform* bones [[buffer(3)]],
                                 uint vertexID [[vertex_id]])
{
    float4x4 skinMatrix =
        bones[in.boneIndices.x].transform * in.boneWeights.x +
        bones[in.boneIndices.y].transform * in.boneWeights.y +
        bones[in.boneIndices.z].transform * in.boneWeights.z +
        bones[in.boneIndices.w].transform * in.boneWeights.w;
    
    float4 skinnedPosition = skinMatrix * float4(in.position, 1.0);
    float3 skinnedNormal = (skinMatrix * float4(in.normal, 0.0)).xyz;
    float3 skinnedTangent = (skinMatrix * float4(in.tangent, 0.0)).xyz;
    float3 skinnedBitangent = (skinMatrix * float4(in.bitangent, 0.0)).xyz;
    
    VertexOut out;
    
    float4 worldPosition = instance.modelMatrix * skinnedPosition;
    out.worldPosition = worldPosition.xyz;
    out.position = camera.projectionMatrix * camera.viewMatrix * worldPosition;
    
    out.worldNormal = normalize((instance.normalMatrix * float4(skinnedNormal, 0.0)).xyz);
    out.worldTangent = normalize((instance.normalMatrix * float4(skinnedTangent, 0.0)).xyz);
    out.worldBitangent = normalize((instance.normalMatrix * float4(skinnedBitangent, 0.0)).xyz);
    out.texCoord = in.texCoord;
    
    return out;
}

struct MaterialUniforms
{
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    float3 emissive;
    float emissiveStrength;
};

struct LightUniforms
{
    float3 position;
    float3 color;
    float intensity;
    float3 ambientColor;
    float ambientIntensity;
};

fragment float4 fragment_main_pnj(VertexOut in [[stage_in]],
                                  constant MaterialUniforms& material [[buffer(0)]],
                                  constant LightUniforms& light [[buffer(1)]],
                                  constant RMDLCameraUniforms& camera [[buffer(2)]],
                                  texture2d<float> albedoTexture [[texture(0)]],
                                  texture2d<float> normalTexture [[texture(1)]],
                                  texture2d<float> metallicTexture [[texture(2)]],
                                  texture2d<float> roughnessTexture [[texture(3)]],
                                  texture2d<float> aoTexture [[texture(4)]],
                                  texture2d<float> emissiveTexture [[texture(5)]],
                                  sampler textureSampler [[sampler(0)]])
{
    float3 albedo = albedoTexture.sample(textureSampler, in.texCoord).rgb * material.albedo;
    float3 normalMap = normalTexture.sample(textureSampler, in.texCoord).rgb * 2.0 - 1.0;
    float metallic = metallicTexture.sample(textureSampler, in.texCoord).r * material.metallic;
    float roughness = roughnessTexture.sample(textureSampler, in.texCoord).r * material.roughness;
    float ao = aoTexture.sample(textureSampler, in.texCoord).r * material.ao;
    float3 emissive = emissiveTexture.sample(textureSampler, in.texCoord).rgb * material.emissive * material.emissiveStrength;
    
    // Transform normal from tangent space to world space
    float3x3 TBN = float3x3(normalize(in.worldTangent), normalize(in.worldBitangent), normalize(in.worldNormal));
    float3 N = normalize(TBN * normalMap);
    float3 V = normalize(camera.position - in.worldPosition);
    float3 L = normalize(light.position - in.worldPosition);
    float3 H = normalize(V + L);
    
    // Calculate PBR lighting
    float3 F0 = mix(float3(0.04), albedo, metallic);

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    
    float NdotL = max(dot(N, L), 0.0);
    float3 radiance = light.color * light.intensity;
    
    float3 Lo = (kD * albedo / M_PI_F + specular) * radiance * NdotL; //PI
    
    // Ambient lighting with AO
    float3 ambient = light.ambientColor * light.ambientIntensity * albedo * ao;
    
    // Final color
    float3 color = ambient + Lo + emissive;
    
    // Tone mapping (ACES)
    color = color / (color + float3(1.0));
    
    // Gamma correction
    color = pow(color, float3(1.0 / 2.2));
    
    return float4(color, 1.0);
}
