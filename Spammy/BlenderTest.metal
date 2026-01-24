//
//  BlenderTest.metal
//  Spammy
//
//  Created by Rémy on 18/12/2025.
//

#include <metal_stdlib>
using namespace metal;

struct VertexBlender
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    int4 joints [[attribute(3)]];
    float4 weights [[attribute(4)]];
};

struct BlenderUniforms
{
    float4x4 modelMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 boneMatrices[28];
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
{    // Skinning matrix - combinaison pondérée des bones
    float4x4 skin = float4x4(0);
    
    if (in.joints.x >= 0) skin += uniforms.boneMatrices[in.joints.x] * in.weights.x;
    if (in.joints.y >= 0) skin += uniforms.boneMatrices[in.joints.y] * in.weights.y;
    if (in.joints.z >= 0) skin += uniforms.boneMatrices[in.joints.z] * in.weights.z;
    if (in.joints.w >= 0) skin += uniforms.boneMatrices[in.joints.w] * in.weights.w;
    
    // Fallback si pas de bones
    float totalWeight = in.weights.x + in.weights.y + in.weights.z + in.weights.w;
    if (totalWeight < 0.001)
        skin = float4x4(1);
    
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
                             constant BlenderUniforms& uniforms [[buffer(0)]],
                             texture2d<float> diffuseTexture [[texture(0)]],
                             texture2d<float> normalTexture [[texture(1)]],
                             texture2d<float> roughnessTexture [[texture(2)]],
                             texture2d<float> metallicTexture [[texture(3)]],
                             sampler textureSampler [[sampler(0)]])
{
    float4 baseColor = diffuseTexture.sample(textureSampler, in.texCoord);
    float3 normalMap = normalTexture.sample(textureSampler, in.texCoord).rgb;
    float roughness = roughnessTexture.sample(textureSampler, in.texCoord).g;
    float metallic = metallicTexture.sample(textureSampler, in.texCoord).b;
    
    // Normal mapping (TBN matrix)
    normalMap = normalMap * 2.0 - 1.0;
    float3 N = normalize(in.normal);
    float3 T = normalize(cross(N, float3(0.0, 1.0, 0.0)));
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    float3 normal = normalize(TBN * normalMap);
    
    // Simple PBR lighting
    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    float3 diffuse = baseColor.rgb * NdotL;
    float3 ambient = baseColor.rgb * 0.3;
    
    // Specular
    float3 viewDir = normalize(-in.worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0 * (1.0 - roughness));
    float3 specular = float3(1.0) * spec * (1.0 - roughness);
    
    float3 finalColor = ambient + diffuse + specular;
    
    return float4(finalColor, baseColor.a);
}
