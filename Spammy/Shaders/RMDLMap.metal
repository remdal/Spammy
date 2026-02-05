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

struct ChunkParams
{
    float2 origin;
    uint size;
    uint padding;
};

struct TerrainVertexLisse
{
    float3 position;
    float3 normal;
    float2 uv;
    uint biomeID;
};

struct BiomeParams
{
    float heightMultiplier;
    float noiseFrequency;
    float baseOffset;
    float friction;
};

constant BiomeParams biomeTable[] = {
    {0.00f, 0.020f,  0.0f, 0.8f},  // 0: SafeZone - 100% plat
    {0.08f, 0.015f,  0.0f, 0.7f},  // 1: Plains - quasi plat
    {0.06f, 0.010f,  0.0f, 0.4f},  // 2: Desert - plat, dunes légères
    {0.20f, 0.025f,  2.0f, 0.6f},  // 3: Forest - vallonné
    {0.35f, 0.020f,  0.0f, 0.7f},  // 4: Hills - collines
    {1.00f, 0.018f, 15.0f, 0.5f},  // 5: Mountains - relief max
    {0.70f, 0.022f,  8.0f, 0.3f},  // 6: Volcanic - relief moyen
    {0.05f, 0.008f, -6.0f, 0.1f},  // 7: Ocean - plat, en dessous
    {0.25f, 0.020f,  4.0f, 0.2f},  // 8: Frozen - vallonné
};

constant float3 biomeColors[] = {
    float3(0.45, 0.75, 0.35),  // 0: SafeZone - vert herbe
    float3(0.55, 0.72, 0.40),  // 1: Plains - vert prairie
    float3(0.85, 0.75, 0.50),  // 2: Desert - sable doré
    float3(0.25, 0.45, 0.20),  // 3: Forest - vert foncé
    float3(0.50, 0.58, 0.42),  // 4: Hills - vert-gris
    float3(0.55, 0.52, 0.50),  // 5: Mountains - gris roche
    float3(0.35, 0.22, 0.18),  // 6: Volcanic - brun-rouge
    float3(0.25, 0.45, 0.65),  // 7: Ocean - bleu profond
    float3(0.85, 0.90, 0.95),  // 8: Frozen - blanc-bleu
};

uint hashWithSeed(uint x, uint seed)
{
    x ^= seed;
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

uint hash2(int2 p, uint seed)
{
    return hashWithSeed(uint(p.x) + hashWithSeed(uint(p.y), seed), seed);
}

float hashFloat(int2 p, uint seed)
{
    return float(hash2(p, seed)) / float(0xFFFFFFFF);
}

float smoothNoise(float2 p, uint seed)
{
    float2 i = floor(p);
    float2 f = fract(p);
    
    // Quintic interpolation (plus smooth que cubic)
    float2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    
    int2 ip = int2(i);
    
    float a = hashFloat(ip, seed);
    float b = hashFloat(ip + int2(1, 0), seed);
    float c = hashFloat(ip + int2(0, 1), seed);
    float d = hashFloat(ip + int2(1, 1), seed);
    
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(float2 p, uint seed, int octaves)
{
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

struct VoronoiResult
{
    float distance;
    float cellRandom;
    float2 cellCenter;
};

VoronoiResult voronoiCell(float2 p, uint seed)
{
    float2 i = floor(p);
    float2 f = fract(p);
    
    VoronoiResult result;
    result.distance = 999.0;
    result.cellRandom = 0.0;
    result.cellCenter = float2(0);
    
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 neighbor = float2(x, y);
            int2 cellPos = int2(i) + int2(x, y);
            
            // Point aléatoire dans la cellule [0.15, 0.85]
            float2 randomOffset = float2(
                hashFloat(cellPos + int2(1000, 0), seed),
                hashFloat(cellPos + int2(0, 1000), seed)
            ) * 0.7 + 0.15;
            
            float2 point = neighbor + randomOffset;
            float dist = length(f - point);
            
            if (dist < result.distance) {
                result.distance = dist;
                result.cellCenter = float2(cellPos) + randomOffset;
                result.cellRandom = hashFloat(cellPos + int2(5000, 5000), seed);
            }
        }
    }
    
    return result;
}

uint classifyBiome(float2 worldPos, float2 center, float flatRadius, uint seed)
{
    float dist = length(worldPos - center);
    
    // Zone de spawn
    if (dist < flatRadius)
        return 0;
    
    // Transition douce autour du spawn
    float spawnBlend = smoothstep(flatRadius, flatRadius * 1.5, dist);
    
    // Voronoi pour régions (~150 unités par cellule)
    VoronoiResult v = voronoiCell(worldPos * 0.007, seed + 8888);
    float cellRandom = v.cellRandom;
    
    // Facteurs environnementaux
    float temperature = fbm(worldPos * 0.003, seed + 1111, 3);
    float humidity = fbm(worldPos * 0.004, seed + 2222, 3);
    float hostility = smoothstep(flatRadius * 2.0, flatRadius * 8.0, dist);
    
    // Distribution : ~30% flat, ~40% rolling, ~30% mountains
    uint biome;
    
    if (cellRandom < 0.30) {
        // FLAT (30%) : Plains, Desert, Ocean
        if (humidity > 0.68) {
            biome = 7; // Ocean
        } else if (temperature > 0.58 && humidity < 0.35) {
            biome = 2; // Desert
        } else {
            biome = 1; // Plains
        }
    }
    else if (cellRandom < 0.70) {
        // ROLLING (40%) : Forest, Hills, Frozen
        if (temperature < 0.28) {
            biome = 8; // Frozen
        } else if (humidity > 0.52) {
            biome = 3; // Forest
        } else {
            biome = 4; // Hills
        }
    }
    else {
        // MOUNTAINOUS (30%) : Mountains, Volcanic
        if (hostility > 0.5 && temperature > 0.65) {
            biome = 6; // Volcanic
        } else {
            biome = 5; // Mountains
        }
    }
    
    if (spawnBlend < 0.5)// && (biome == 5 || biome == 6 || biome == 7))
        biome = 1;
    
    return biome;
}

float getTerrainHeight(float2 worldPos, uint biome, uint seed, float maxHeight)
{
    BiomeParams params = biomeTable[min(biome, 8u)];
    
    // Bruit de base multi-octave
    float noise = fbm(worldPos * params.noiseFrequency, seed, 6);
    noise = (noise - 0.5) * 2.0; // Centrer sur [-1, 1]
    
    // Appliquer le multiplicateur du biome
    float height = noise * maxHeight * params.heightMultiplier;
    
    // Ajouter l'offset de base
    height += params.baseOffset;
    
    // Variation locale pour casser la monotonie
    float detail = fbm(worldPos * 0.1, seed + 3333, 3);
    height += (detail - 0.5) * 2.0 * params.heightMultiplier;
    
    return height;
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
    
    uint biome = classifyBiome(worldPos, config.center, config.flatRadius, config.seed);
    
    float height = getTerrainHeight(worldPos, biome, config.seed, config.maxHeight);
    
    // Transition douce vers zone plate centrale
    float dist = length(worldPos - config.center);
    float flatBlend = 1.0 - smoothstep(config.flatRadius * 0.7, config.flatRadius * 1.2, dist);
    height = mix(height, 0.0, flatBlend);
    
    vertices[idx].position = float3(worldPos.x, height, worldPos.y);
    vertices[idx].uv = localPos / float(chunk.size);
    vertices[idx].biomeID = biome;
    vertices[idx].normal = float3(0, 1, 0); // Placeholder
}

kernel void computeNormalsKernel(device TerrainVertexLisse* vertices [[buffer(0)]],
                                 constant ChunkParams& chunk [[buffer(1)]],
                                 constant TerrainConfigLisse& config [[buffer(2)]],
                                 uint2 gid [[thread_position_in_grid]])
{
    if (gid.x > chunk.size || gid.y > chunk.size) return;
    
    float2 worldPos = chunk.origin + float2(gid);
    float eps = 1.0;
    
    uint idx = gid.y * (chunk.size + 1) + gid.x;
    
    // Recalculer les hauteurs voisines (évite les problèmes de bord de chunk)
    uint biome = vertices[idx].biomeID;
    
    float hC = vertices[idx].position.y;
    float hL = getTerrainHeight(worldPos + float2(-eps, 0), biome, config.seed, config.maxHeight);
    float hR = getTerrainHeight(worldPos + float2(eps, 0), biome, config.seed, config.maxHeight);
    float hU = getTerrainHeight(worldPos + float2(0, -eps), biome, config.seed, config.maxHeight);
    float hD = getTerrainHeight(worldPos + float2(0, eps), biome, config.seed, config.maxHeight);
    
    // Appliquer flatBlend comme dans le kernel principal
    float dist = length(worldPos - config.center);
    float flatBlend = 1.0 - smoothstep(config.flatRadius * 0.7, config.flatRadius * 1.2, dist);
    hL = mix(hL, 0.0, flatBlend);
    hR = mix(hR, 0.0, flatBlend);
    hU = mix(hU, 0.0, flatBlend);
    hD = mix(hD, 0.0, flatBlend);
    
    float3 normal = normalize(float3(hL - hR, 2.0 * eps, hU - hD));
    
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
    float3 biomeColor;
    float biomeBlend;
    uint biomeID;
};

struct TerrainUniforms
{
    float4x4 viewProjection;
    float3 cameraPos;
    float time;
};

vertex TerrainVertexOut terrainVertexShader(TerrainVertexIn in [[stage_in]],
                                            constant float4x4& viewProjection [[buffer(1)]])
{
    TerrainVertexOut out;
    out.worldPos = in.position;
    out.position = viewProjection * float4(in.position, 1.0);
    out.normal = in.normal;
    out.uv = in.uv;
    uint biomeIdx = min(in.biomeID, 8u);
    out.biomeColor = biomeColors[biomeIdx];
    return out;
}

fragment float4 terrainFragmentShader(TerrainVertexOut in [[stage_in]],
                                      constant float3& cameraPos [[buffer(0)]],
                                      constant float& dayTime [[buffer(1)]])
{
    uint biomeIdx = min(in.biomeID, 8u);
    float3 baseColor = in.biomeColor;
    
    // Variation de couleur selon la hauteur
    float heightFactor = saturate(in.worldPos.y / 60.0);
    baseColor = mix(baseColor, baseColor * 0.7, heightFactor * 0.3);
    
    // Variation selon la pente (normale)
    float slope = 1.0 - in.normal.y;
    float3 rockColor = float3(0.45, 0.42, 0.40);
    baseColor = mix(baseColor, rockColor, smoothstep(0.3, 0.7, slope));
    
    float3 lightDir = normalize(float3(0.4, 0.8, 0.3));
    float ndotl = max(dot(in.normal, lightDir), 0.0);
    
    float ambient = 0.35;
    float diffuse = ndotl * 0.65;
    
    // Ombre douce basée sur la hauteur (AO fake)
    float ao = smoothstep(-10.0, 5.0, in.worldPos.y);
    ambient *= mix(0.6, 1.0, ao);
    
    float3 litColor = baseColor * (ambient + diffuse);
    
    float dist = length(in.worldPos - cameraPos - 50);
    float fogStart = 350.0;
    float fogEnd = 800.0;
    float fogFactor = smoothstep(fogStart, fogEnd, dist);
    
    // Cycle solaire avec sin/cos
    float sunAngle = dayTime * 6.28318; // 0 → 2π
    float r = 0.5 + 0.5 * sin(sunAngle * 2.0);
    float g = 0.4 + 0.4 * sin(sunAngle - 1.57); // décalé de π/2
    float b = 0.3 + 0.6 * max(0.0, sin(sunAngle - 1.57));

    float3 fogColor = float3(r, g, b);

    litColor = mix(litColor, fogColor, fogFactor);
    
    return float4(litColor, 1.0);
}

struct PhysicsSample
{
    float height;
    float3 normal;
    uint biomeID;
    float friction;
};

kernel void sampleTerrainPhysics(device const float2* queryPositions [[buffer(0)]],
                                 device PhysicsSample* results [[buffer(1)]],
                                 constant TerrainConfigLisse& config [[buffer(2)]],
                                 uint gid [[thread_position_in_grid]])
{
    float2 worldPos = queryPositions[gid];
    
    // Classification et hauteur
    uint biome = classifyBiome(worldPos, config.center, config.flatRadius, config.seed);
    float height = getTerrainHeight(worldPos, biome, config.seed, config.maxHeight);
    
    // Transition zone plate
    float dist = length(worldPos - config.center);
    float flatBlend = 1.0 - smoothstep(config.flatRadius * 0.7, config.flatRadius * 1.2, dist);
    height = mix(height, 0.0, flatBlend);
    
    // Normale par différences finies
    float eps = 0.5;
    
    float hL = getTerrainHeight(worldPos + float2(-eps, 0), biome, config.seed, config.maxHeight);
    float hR = getTerrainHeight(worldPos + float2(eps, 0), biome, config.seed, config.maxHeight);
    float hD = getTerrainHeight(worldPos + float2(0, -eps), biome, config.seed, config.maxHeight);
    float hU = getTerrainHeight(worldPos + float2(0, eps), biome, config.seed, config.maxHeight);
    
    hL = mix(hL, 0.0, flatBlend);
    hR = mix(hR, 0.0, flatBlend);
    hD = mix(hD, 0.0, flatBlend);
    hU = mix(hU, 0.0, flatBlend);
    
    float3 normal = normalize(float3(hL - hR, 2.0 * eps, hD - hU));
    if (normal.y < 0.0) normal = -normal;
    
    // Friction du biome
    BiomeParams params = biomeTable[min(biome, 8u)];
    
    results[gid].height = height;
    results[gid].normal = normal;
    results[gid].biomeID = biome;
    results[gid].friction = params.friction;
}
