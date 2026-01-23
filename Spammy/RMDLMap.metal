//
//  RMDLMap.metal
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct TerrainConfigLisse
{
    uint seed;
    float flatRadius;
    float maxHeight;
    float flatness;
    float2 center;
};

struct ChunkParams {
    float2 origin;
    uint size;
    uint padding;
};

struct TerrainVertexLisse {
    float3 position;
    float3 normal;
    float2 uv;
    uint biomeID;
};

// Hash pour seed
uint hash(uint x, uint seed) {
    x ^= seed;
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

float hashFloat(uint x, uint y, uint seed) {
    uint h = hash(x + hash(y, seed), seed);
    return float(h) / float(0xFFFFFFFF);
}

// Smooth noise avec interpolation
float smoothNoise(float2 p, uint seed) {
    float2 i = floor(p);
    float2 f = fract(p);
    
    // Smoothstep pour éviter les artefacts
    float2 u = f * f * (3.0 - 2.0 * f);
    
    uint ix = uint(i.x);
    uint iy = uint(i.y);
    
    float a = hashFloat(ix, iy, seed);
    float b = hashFloat(ix + 1, iy, seed);
    float c = hashFloat(ix, iy + 1, seed);
    float d = hashFloat(ix + 1, iy + 1, seed);
    
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// FBM pour terrain naturel
float fbm(float2 p, uint seed, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        value += amplitude * smoothNoise(p * frequency, seed + uint(i) * 1337);
        maxValue += amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value / maxValue;
}

// Classification biome basée sur distance + noise
uint classifyBiome(float2 worldPos, float2 center, float flatRadius, uint seed)
{
    float dist = length(worldPos - center);

    if (dist < flatRadius)
        return 0; // SafeZone
    
    // LOD
//    if (dist < 200) size = 128;
//    else if (dist < 600) size = 64;
//    else if (dist < 1500) size = 16;
    
    // Biome noise
//    float biomeNoise = fbm(worldPos * 0.01, seed + 9999, 3);
    float temperature = fbm(worldPos * 0.005, seed + 1111, 2);
    float humidity = fbm(worldPos * 0.007, seed + 2222, 2);
    
    // Plus on s'éloigne, plus les biomes sont hostiles
    float hostility = smoothstep(flatRadius, flatRadius * 5.0, dist);
    
    if (hostility > 0.8 && temperature > 0.6) return 5; // Volcanic
    if (hostility > 0.7 && temperature < 0.3) return 6; // Frozen
    if (humidity > 0.7) return 7; // Ocean
    if (humidity < 0.3 && temperature > 0.5) return 2; // Desert
    if (humidity > 0.4 && temperature > 0.3) return 3; // Forest
    if (hostility > 0.5) return 4; // Mountains
    
    return 1; // Plains
}

// Hauteur selon biome
float getBiomeHeight(uint biome, float baseHeight, float flatness) {
    float multiplier = 1.0;
    
    switch (biome) {
        case 0: return 0.0; // SafeZone = plat
        case 1: multiplier = 0.3; break;  // Plains
        case 2: multiplier = 0.2; break;  // Desert
        case 3: multiplier = 0.4; break;  // Forest
        case 4: multiplier = 1.0; break;  // Mountains
        case 5: multiplier = 0.8; break;  // Volcanic
        case 6: multiplier = 0.6; break;  // Frozen
        case 7: multiplier = -0.5; break; // Ocean (négatif)
    }
    
    return baseHeight * multiplier * (1.0 - flatness * 0.7);
}

kernel void terrainGenerateKernel(device TerrainVertexLisse* vertices [[buffer(0)]],
                                  constant TerrainConfigLisse& config [[buffer(1)]],
                                  constant ChunkParams& chunk [[buffer(2)]],
                                  uint2 gid [[thread_position_in_grid]])
{
    if (gid.x > chunk.size || gid.y > chunk.size) return;
    
    uint idx = gid.y * (chunk.size + 1) + gid.x;
    
    float2 localPos = float2(gid);
    float2 worldPos = chunk.origin + localPos;
    
    // Distance du centre
    float distFromCenter = length(worldPos - config.center);
    
    // Transition douce vers zone plate
    float flatBlend = 1.0 - smoothstep(config.flatRadius * 0.5, config.flatRadius, distFromCenter);
    
    // Hauteur de base avec FBM
    float baseHeight = fbm(worldPos * 0.02, config.seed, 6);
    baseHeight = (baseHeight - 0.5) * 2.0; // [-1, 1]
    baseHeight *= config.maxHeight;
    
    // Classification biome
    uint biome = classifyBiome(worldPos, config.center, config.flatRadius, config.seed);
    
    // Hauteur finale avec blend zone plate
    float finalHeight = getBiomeHeight(biome, baseHeight, config.flatness);
    finalHeight = mix(finalHeight, 0.0, flatBlend);
    
    vertices[idx].position = float3(worldPos.x, finalHeight, worldPos.y);
    vertices[idx].uv = localPos / float(chunk.size);
    vertices[idx].biomeID = biome;
    vertices[idx].normal = float3(0, 1, 0); // Placeholder, calculé au pass 2
}

kernel void computeNormalsKernel(device TerrainVertexLisse* vertices [[buffer(0)]],
                                 constant ChunkParams& chunk [[buffer(1)]],
                                 uint2 gid [[thread_position_in_grid]])
{
    if (gid.x > chunk.size || gid.y > chunk.size) return;
    
    uint idx = gid.y * (chunk.size + 1) + gid.x;
    uint stride = chunk.size + 1;
    
    // Sample voisins pour gradient
    float3 center = vertices[idx].position;
    
    float3 left = center;
    float3 right = center;
    float3 up = center;
    float3 down = center;
    
    if (gid.x > 0) left = vertices[idx - 1].position;
    if (gid.x < chunk.size) right = vertices[idx + 1].position;
    if (gid.y > 0) up = vertices[idx - stride].position;
    if (gid.y < chunk.size) down = vertices[idx + stride].position;
    
    // Gradient-based normal (plus précis que cross product)
    float3 dx = right - left;
    float3 dz = down - up;
    
    float3 normal = normalize(cross(dz, dx));
    
    // Assure que la normale pointe vers le haut
//    if (normal.y < 0) normal = -normal;
    
    vertices[idx].normal = normal;
}

struct TerrainVertexIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
    uint biomeID    [[attribute(3)]];
};

struct TerrainVertexOut
{
    float4 position [[position]];
    float3 worldPos;
    float3 normal;
    float2 uv;
    uint biomeID;
};

vertex TerrainVertexOut terrainVertexShader(TerrainVertexIn in [[stage_in]],
                                            constant float4x4& viewProjection [[buffer(1)]])
{
    TerrainVertexOut out;
    
    out.worldPos = in.position;
    out.position = viewProjection * float4(in.position, 1.0);
    out.normal = in.normal;
    out.uv = in.uv;
    out.biomeID = in.biomeID;
    
    return out;
}

constant float3 biomeColors[] = {
    float3(0.3, 0.8, 0.3),  // 0: SafeZone - vert clair
    float3(0.4, 0.7, 0.3),  // 1: Plains - vert
    float3(0.9, 0.8, 0.5),  // 2: Desert - sable
    float3(0.2, 0.5, 0.2),  // 3: Forest - vert foncé
    float3(0.5, 0.5, 0.5),  // 4: Mountains - gris
    float3(0.3, 0.2, 0.2),  // 5: Volcanic - rouge sombre
    float3(0.8, 0.9, 1.0),  // 6: Frozen - blanc bleuté
    float3(0.2, 0.4, 0.8)   // 7: Ocean - bleu
};

fragment float4 terrainFragmentShader(TerrainVertexOut in [[stage_in]])
{
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float ndotl = max(dot(in.normal, lightDir), 0.0);
    float ambient = 0.3;
    float diffuse = ndotl * 0.7;
    
    uint biomeIdx = min(in.biomeID, 7u);
    float3 baseColor = biomeColors[biomeIdx];
    
    float4 finalColor = float4(baseColor * (ambient + diffuse), 1.0);
    
    float distance = length(in.worldPos - in.worldPos + 100);
    float fogStart = 100.0;
    float fogEnd = 250.0;
    float fogFactor = smoothstep(fogStart, fogEnd, distance);
    float4 fogColor = float4(0.6, 0.8, 1.0, 1.0);
    
    return mix(finalColor, fogColor, fogFactor);
    
    return float4(finalColor);
}
