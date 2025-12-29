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

struct AnimatedSpriteBlender {
    float4x4 modelMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 boneMatrices[3];
    float time;
};

struct VertexOut {
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
    float3 worldPos;
};

vertex VertexOut vertexmain(VertexBlender in [[stage_in]],
    constant AnimatedSpriteBlender& uniforms [[buffer(1)]]
) {
    // Skinning (1 bone in your case)
    float4x4 skinMatrix = uniforms.boneMatrices[in.joints.x] * in.weights.x;
    
    float4 skinnedPos = skinMatrix * float4(in.position, 1.0);
    float4 skinnedNorm = skinMatrix * float4(in.normal, 0.0);
    
    float4 worldPos = uniforms.modelMatrix * skinnedPos;
    
    VertexOut out;
    out.position = uniforms.viewProjectionMatrix * worldPos;
    out.worldPos = worldPos.xyz;
    out.normal = normalize((uniforms.modelMatrix * skinnedNorm).xyz);
    out.texCoord = in.texCoord;
    
    return out;
}

fragment float4 fragmentmain(VertexOut in [[stage_in]],
                             constant AnimatedSpriteBlender& uniforms [[buffer(0)]],
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
