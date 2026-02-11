//
//  BlenderTest.metal
//  Spammy
//
//  Created by Rémy on 18/12/2025.
//

#include <metal_stdlib>
using namespace metal;

#include "../RMDLMainRenderer_shared.h"

struct VertexBlender
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexBlenderFull
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    int4 joints [[attribute(3)]];
    float4 weights [[attribute(4)]];
};

struct BlenderUniforms
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjectionMatrix;
};

struct BlenderUniformsFull
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjectionMatrix;
    simd::float4x4 boneMatrices[28];
};

struct VertexOut
{
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
    float3 worldPos;
};

vertex VertexOut vertexmain(VertexBlender in [[stage_in]],
                            constant BlenderUniforms& uniforms [[buffer(1)]])
{
    VertexOut out;
    float4 worldPos = uniforms.modelMatrix * float4(in.position, 1.0);
    out.position = uniforms.viewProjectionMatrix * worldPos;
    out.worldPos = worldPos.xyz;
    out.normal = normalize((uniforms.modelMatrix * float4(in.normal, 0.0)).xyz);
    out.texCoord = in.texCoord;
    return out;
}

vertex VertexOut vertex_full(VertexBlenderFull in [[stage_in]],
                            constant BlenderUniformsFull& uniforms [[buffer(1)]])
{
    float4x4 skin = uniforms.boneMatrices[in.joints.x] * in.weights.x + uniforms.boneMatrices[in.joints.y] * in.weights.y + uniforms.boneMatrices[in.joints.z] * in.weights.z + uniforms.boneMatrices[in.joints.w] * in.weights.w;
    
    float4 skinnedPos = skin * float4(in.position, 1.0);
    float3 skinnedNormal = normalize((skin * float4(in.normal, 0.0)).xyz);
    
    VertexOut out;
    float4 worldPos = uniforms.modelMatrix * skinnedPos;
    out.position = uniforms.viewProjectionMatrix * worldPos;
    out.worldPos = worldPos.xyz;
    out.normal = normalize((uniforms.modelMatrix * float4(skinnedNormal, 0.0)).xyz);
    out.texCoord = in.texCoord;
    return out;
}

fragment float4 fragmentmain(VertexOut in [[stage_in]],
                             texture2d<float> diffuseTexture [[texture(0)]],
                             texture2d<float> normalTexture [[texture(1)]],
                             texture2d<float> roughnessTexture [[texture(2)]],
                             texture2d<float> metallicTexture [[texture(3)]],
                             sampler textureSampler [[sampler(0)]],
                             constant RMDLUniforms& uniforms [[buffer(1)]])
{
    float4 albedo = diffuseTexture.sample(textureSampler, in.texCoord);
    float3 normal = normalTexture.sample(textureSampler, in.texCoord).rgb;
    float roughness = roughnessTexture.sample(textureSampler, in.texCoord).g;
    float metallic = metallicTexture.sample(textureSampler, in.texCoord).b;
    
    // [0,1] → [-1,1]
    normal = normal * 2.0 - 1.0;
    
    // Construire TBN (tangent devrait venir du vertex shader idéalement)
    float3 N = normalize(in.normal);
    float3 T = normalize(cross(N, float3(0.0, 1.0, 0.0)));
    if (length(cross(N, float3(0.0, 1.0, 0.0))) < 0.001)
        T = normalize(cross(N, float3(1.0, 0.0, 0.0)));
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 normalIze = normalize(TBN * normal);
    
                                                        // Lighting avec la normale perturbée
    float3 lightDir = normalize(uniforms.sunDirection); // float3(1.0, 1.0, 0.5));
    float NdotL = max(dot(normalIze, lightDir), 0.0);
    
    float3 diffuse = albedo.rgb * NdotL * uniforms.sunColor;
    float3 ambient = albedo.rgb * 0.3;
    
    // Specular
    float3 viewDir = normalize(-in.worldPos); // and : scene.cameraPosition - in.worldPos);  // ← Meilleur calcul
    float3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalIze, halfDir), 0.0), 32.0 * (1.0 - roughness));
    float3 specular = float3(1.0) * spec * (1.0 - roughness);
    
    float3 finalColor = ambient + diffuse + specular;
    
    return float4(finalColor, albedo.a);
}
