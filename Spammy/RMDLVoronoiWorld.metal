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

vertex VoxelFragmentInput voxel_vertex(VoxelVertex in [[stage_in]],
                                       constant RMDLCameraUniforms& camera [[buffer(1)]])
{
    VoxelFragmentInput out;
    float4 worldPos = float4(in.position, 1.0);
    out.position = camera.viewProjectionMatrix * worldPos;
    out.worldPosition = in.position;
    out.color = in.color;
    out.normal = in.normal;
    return out;
}

fragment float4 voxel_fragment(VoxelFragmentInput in [[stage_in]],
                               constant RMDLCameraUniforms& camera [[buffer(1)]])
{
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float3 normal = normalize(in.normal);
    
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.3;
    float lighting = ambient + diffuse * 0.7;
    
    float distance = length(in.worldPosition - camera.position);
    float fogStart = 50.0;
    float fogEnd = 150.0;
    float fogFactor = smoothstep(fogStart, fogEnd, distance);
    float4 fogColor = float4(0.6, 0.8, 1.0, 1.0);
    
    float4 litColor = in.color * lighting;
    return mix(litColor, fogColor, fogFactor);
}

constant int CHUNK_SIZE = 16;
constant int CHUNK_HEIGHT = 64;

constant int permutation[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

struct BiomeRegion
{
    float2 center;
    float radius;
    uint type;
    float intensity;
    float3 _padding;
};

struct ChunkGenerationParams
{
    int chunkX;
    int chunkZ;
    float currentTime;
    uint seed;
    uint numBiomeRegions;
    float3 _padding;
};

uint hash_gpu(int x, int y, int z)
{
    uint h = 2166136261u;
    h = (h ^ uint(x)) * 16777619u;
    h = (h ^ uint(y)) * 16777619u;
    h = (h ^ uint(z)) * 16777619u;
    return h;
}

float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float grad(int hash, float x, float y, float z)
{
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float perlinNoise3D_GPU(float3 p)
{
    int3 pi = int3(floor(p));
    float3 pf = fract(p);
    
    float u = fade(pf.x);
    float v = fade(pf.y);
    float w = fade(pf.z);
    
    float c000 = sin(float(hash_gpu(pi.x, pi.y, pi.z) % 360u) * 0.017453292); // * 1 degré into radian
    float c100 = sin(float(hash_gpu(pi.x+1, pi.y, pi.z) % 360u) * 0.017453292);
    float c010 = sin(float(hash_gpu(pi.x, pi.y+1, pi.z) % 360u) * 0.017453292);
    float c110 = sin(float(hash_gpu(pi.x+1, pi.y+1, pi.z) % 360u) * 0.017453292);
    float c001 = sin(float(hash_gpu(pi.x, pi.y, pi.z+1) % 360u) * 0.017453292);
    float c101 = sin(float(hash_gpu(pi.x+1, pi.y, pi.z+1) % 360u) * 0.017453292);
    float c011 = sin(float(hash_gpu(pi.x, pi.y+1, pi.z+1) % 360u) * 0.017453292);
    float c111 = sin(float(hash_gpu(pi.x+1, pi.y+1, pi.z+1) % 360u) * 0.017453292);
    
    float x1 = mix(c000, c100, u);
    float x2 = mix(c010, c110, u);
    float y1 = mix(x1, x2, v);
    
    float x3 = mix(c001, c101, u);
    float x4 = mix(c011, c111, u);
    float y2 = mix(x3, x4, v);
    
    return mix(y1, y2, w);
}

float voronoiNoise3D_GPU(float3 p)
{
    int3 pi = int3(floor(p));
    float minDist = 10000.0;
    
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dz = -1; dz <= 1; dz++)
            {
                int3 cell = pi + int3(dx, dy, dz);
                
                uint h = hash_gpu(cell.x, cell.y, cell.z);
                float3 cellPoint = float3(cell) + float3(float(h % 1000u) / 1000.0,
                                                         float((h / 1000u) % 1000u) / 1000.0,
                                                         float((h / 1000000u) % 1000u) / 1000.0);
                float dist = length(p - cellPoint);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

uint generatePerlinVoronoi_GPU(int3 worldPos, float blend)
{
    float3 p = float3(worldPos);
    
    float perlin = 0.0;
    perlin += perlinNoise3D_GPU(p * 0.01) * 1.0;
    perlin += perlinNoise3D_GPU(p * 0.02) * 0.5;
    perlin += perlinNoise3D_GPU(p * 0.04) * 0.25;
    
    float voronoi = voronoiNoise3D_GPU(p * 0.05);
    
    float mixed = perlin * 0.6 + (1.0 - voronoi * 0.5) * 0.4;
    mixed *= blend;
    
    float baseHeight = 32.0 + perlin * 15.0;
    
    if (worldPos.y < baseHeight + mixed * 10.0) {
        if (worldPos.y < 5) return 8;
        if (worldPos.y < baseHeight - 8) return 1;
        if (worldPos.y < baseHeight - 4) return 6;
        if (worldPos.y < baseHeight - 1) return 1;
        
        if (voronoi < 0.25) return 5;
        if (voronoi < 0.4) return 4;
        return 1;
    }
    return 0;
}

uint generateVolcano_GPU(int3 worldPos, float distToCenter)
{
    const float coneHeight = 50.0;
    const float coneRadius = 40.0;
    
    float heightFactor = max(0.0, (coneHeight - worldPos.y) / coneHeight);
    float radiusAtHeight = coneRadius * heightFactor;
    
    bool inCrater = (worldPos.y > 45 && worldPos.y < 55 && distToCenter < 12.0);
    
    if (inCrater)
    {
        float lavaLevel = 48.0 + sin(worldPos.x * 0.2 + worldPos.z * 0.2) * 1.5;
        if (worldPos.y < lavaLevel) return 2;
        return 0;
    }
    if (distToCenter < radiusAtHeight && worldPos.y < coneHeight && worldPos.y > 5)
    {
        float noise = perlinNoise3D_GPU(float3(worldPos) * 0.1);
        
        if (noise > 0.4) return 1;
        if (noise > 0.1) return 6;
        return 2;
    }
    if (worldPos.y < 15 && distToCenter < coneRadius * 1.5)
        return 1;
    return 0;
}

uint generateWTF_GPU(int3 worldPos, float blend)
{
    float3 p = float3(worldPos);
    
    float gravityNoise = perlinNoise3D_GPU(float3(p.x, 0, p.z) * 0.03);
    float platformNoise = voronoiNoise3D_GPU(float3(p.x, p.y, p.z) * float3(0.02, 0.08, 0.02));
    
    if (platformNoise < 0.15)
    {
        uint h = hash_gpu(worldPos.x / 5, worldPos.y / 5, worldPos.z / 5);
        uint types[6] = {2, 3, 4, 7, 5, 6};
        return types[h % 6];
    }
    
    if (gravityNoise > 0.4 && worldPos.y > 25 && worldPos.y < 50)
    {
        float spiralAngle = atan2(p.z, p.x) + worldPos.y * 0.15;
        float spiralDist = length(float2(p.x, p.z));
        float targetRadius = 20.0 + sin(spiralAngle * 3.0) * 8.0;
        
        if (abs(spiralDist - targetRadius) < 3.0)
            return 7;
    }
    
    if (worldPos.y < 20)
    {
        float baseNoise = perlinNoise3D_GPU(p * 0.05);
        if (baseNoise > 0.2)
            return (baseNoise > 0.5) ? 6 : 1;
    }
    return 0;
}

kernel void generateChunkTerrain(device uint8_t* outputBlocks [[buffer(0)]],
                                 constant BiomeRegion* biomeRegions [[buffer(1)]],
                                 constant ChunkGenerationParams& params [[buffer(2)]],
                                 uint3 gid [[thread_position_in_grid]])
{
    if (gid.x >= CHUNK_SIZE || gid.y >= CHUNK_HEIGHT || gid.z >= CHUNK_SIZE)
        return;

    int3 worldPos = int3(params.chunkX * CHUNK_SIZE + int(gid.x),
                         int(gid.y),
                         params.chunkZ * CHUNK_SIZE + int(gid.z));

    float2 pos2D = float2(worldPos.x, worldPos.z);

    float minDist = 10000.0;
    uint closestBiomeType = 0;
    float closestBlend = 1.0;

    for (uint i = 0; i < params.numBiomeRegions; i++)
    {
        float2 diff = pos2D - biomeRegions[i].center;
        float dist = length(diff);

        if (dist < biomeRegions[i].radius && dist < minDist)
        {
            minDist = dist;
            closestBiomeType = biomeRegions[i].type;
    
            float blend = 1.0 - (dist / biomeRegions[i].radius);
            closestBlend = clamp(blend, 0.0, 1.0) * biomeRegions[i].intensity;
        }
    }

    uint blockType = 0;

    if (closestBiomeType == 0)
        blockType = generatePerlinVoronoi_GPU(worldPos, closestBlend);
    else if (closestBiomeType == 1)
    {
        float2 volcanoCenter = float2(0, 0);
        for (uint i = 0; i < params.numBiomeRegions; i++)
        {
            if (biomeRegions[i].type == 1)
            {
                volcanoCenter = biomeRegions[i].center;
                break;
            }
        }
        float dx = worldPos.x - volcanoCenter.x;
        float dz = worldPos.z - volcanoCenter.y;
        float distToCenter = sqrt(dx*dx + dz*dz);
        blockType = generateVolcano_GPU(worldPos, distToCenter);
    }
    else if (closestBiomeType == 2) {
        blockType = generatePerlinVoronoi_GPU(worldPos, closestBlend);
    }
    else if (closestBiomeType == 3) {
        blockType = generatePerlinVoronoi_GPU(worldPos, closestBlend);
    }
    else if (closestBiomeType == 4) {
        blockType = generateWTF_GPU(worldPos, closestBlend);
    }
    
    if (worldPos.y < 2 || worldPos.y > CHUNK_HEIGHT - 2) {
        blockType = 0;
    }
    
    uint index = gid.x + gid.y * CHUNK_SIZE + gid.z * CHUNK_SIZE * CHUNK_HEIGHT;
    outputBlocks[index] = uint8_t(blockType);
}
//fragment float4 voxel_fragment(VoxelFragmentInput in [[stage_in]], constant RMDLCameraUniforms& camera [[buffer(1)]])
//{
//    // Lumière directionnelle simple (soleil)
//    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
//    float3 normal = normalize(in.normal);
//    
//    // Diffuse lighting
//    float diffuse = max(dot(normal, lightDir), 0.0);
//    float ambient = 0.3;
//    float lighting = ambient + diffuse * 0.7;
//    
//    // Fog basé sur la distance
//    float distance = length(in.worldPosition - camera.position);
//    float fogStart = 50.0;
//    float fogEnd = 150.0;
//    float fogFactor = smoothstep(fogStart, fogEnd, distance);
//    float4 fogColor = float4(0.6, 0.8, 1.0, 1.0); // Bleu ciel
//    
//    float4 litColor = in.color * lighting;
//    return mix(litColor, fogColor, fogFactor);
//}

//constant int CHUNK_SIZE = 16;
//constant int CHUNK_HEIGHT = 64;

//float perlinFBM(float3 p, int octaves, float persistence, float lacunarity) {
//    float total = 0.0;
//    float frequency = 1.0;
//    float amplitude = 1.0;
//    float maxValue = 0.0;
//    
//    for(int i = 0; i < octaves; i++) {
//        total += perlin3D(p * frequency) * amplitude;
//        maxValue += amplitude;
//        amplitude *= persistence;
//        frequency *= lacunarity;
//    }
//    
//    return total / maxValue;
//}
//
//float3 hash3D(uint seed, uint index, float width, float height)
//{
//    uint s = seed + index * 1000u;
//    uint x = s * 747796405u + 2891336453u;
//    uint y = x * 747796405u + 2891336453u;
//    uint z = y * 747796405u + 2891336453u;
//    
//    return float3(
//        float(x) / 4294967295.0 * width,
//        float(y) / 4294967295.0 * height,
//        float(z) / 4294967295.0 * 100.0
//    );
//}
//
//float voronoi3D(float3 p, uint seed, int count, float width, float height) {
//    float minDist = 999999.0;
//    
//    for (int i = 0; i < count; i++)
//    {
//        float3 point = hash3D(seed, uint(i), width, height);
//        float dist = distance(p, point);
//        minDist = min(minDist, dist);
//    }
//    
//    return clamp(minDist / (width * 0.3), 0.0, 1.0);
//}
//
//float combinedNoise(float3 p, constant Generation& params)
//{
//    float3 scaledP = p / params.scale;
//    
//    float perlin = perlinFBM(scaledP, params.octaves, params.persistence, params.lacunarity);
//    float voronoi = voronoi3D(p, params.seed, params.voronoiCount,
//                              float(params.width), float(params.height));
//    
//    float combined = perlin * 0.5 + (1.0 - voronoi) * 0.5;
//    combined = pow(combined * 0.5 + 0.5, 1.5);
//    
//    return combined;
//}
//
//uint determineBlockType(float height, float metallic)
//{
//    if (height < 0.3) return 1;
//    if (height < 0.6) return 2;
//    if (height < 0.85) return (metallic > 0.85) ? 4 : 2;
//    return 3;
//}
//
//kernel void generateTerrain(device TerrainBlocks* blocks [[buffer(0)]],
//                            constant Generation& params [[buffer(1)]],
//                            uint2 gid [[thread_position_in_grid]])
//{
//    if (gid.x >= params.width || gid.y >= params.height) return;
//    
//    uint idx = gid.y * params.width + gid.x;
//    float3 position = float3(float(gid.x), float(gid.y), 0.0);
//    
//    float heightValue = combinedNoise(position, params);
//    
//    float3 metallicPos = position / (params.scale * 0.5);
//    metallicPos.z += 100.0;
//    float metallicValue = perlinFBM(metallicPos, params.octaves,
//                                    params.persistence, params.lacunarity) * 0.5 + 0.5;
//    
//    blocks[idx].height = heightValue;
//    blocks[idx].metallic = metallicValue;
//    blocks[idx].roughness = 1.0 - (heightValue * 0.3);
//    blocks[idx].blockType = determineBlockType(heightValue, metallicValue);
//}
//
//kernel void renderHeightmap(device TerrainBlocks* blocks [[buffer(0)]],
//                            texture2d<float, access::write> outTexture [[texture(0)]],
//                            constant Generation& params [[buffer(1)]],
//                            uint2 gid [[thread_position_in_grid]])
//{
//    if (gid.x >= params.width || gid.y >= params.height) return;
//    
//    uint idx = gid.y * params.width + gid.x;
//    float4 color;
//    
//    if (blocks[idx].blockType == 1) color = float4(0.4, 0.3, 0.2, 1.0);
//    else if (blocks[idx].blockType == 2) color = float4(0.5, 0.5, 0.5, 1.0);
//    else if (blocks[idx].blockType == 3) color = float4(1.0, 1.0, 1.0, 1.0);
//    else if (blocks[idx].blockType == 4) color = float4(0.8, 0.6, 0.2, 1.0);
//    else color = float4(0.0, 0.0, 0.0, 1.0);
//    
//    color.rgb *= (0.5 + blocks[idx].height * 0.5);
//    outTexture.write(color, gid);
//}
//
//kernel void renderMetallicMap(device TerrainBlocks* blocks [[buffer(0)]],texture2d<float,
//                              access::write> outTexture [[texture(0)]],
//                              constant Generation& params [[buffer(1)]],
//                              uint2 gid [[thread_position_in_grid]])
//{
//    if (gid.x >= params.width || gid.y >= params.height) return;
//    uint idx = gid.y * params.width + gid.x;
//    float m = blocks[idx].metallic;
//    outTexture.write(float4(m, m, m, 1.0), gid);
//}
//
//kernel void renderRoughnessMap(device TerrainBlocks* blocks [[buffer(0)]],
//                               texture2d<float, access::write> outTexture [[texture(0)]],
//                               constant Generation& params [[buffer(1)]],
//                               uint2 gid [[thread_position_in_grid]])
//{
//    if (gid.x >= params.width || gid.y >= params.height) return;
//    uint idx = gid.y * params.width + gid.x;
//    float r = blocks[idx].roughness;
//    outTexture.write(float4(r, r, r, 1.0), gid);
//}
//
//float perlin3D(float3 p)
//{
//    int X = int(floor(p.x)) & 255;
//    int Y = int(floor(p.y)) & 255;
//    int Z = int(floor(p.z)) & 255;
//    
//    float x = p.x - floor(p.x);
//    float y = p.y - floor(p.y);
//    float z = p.z - floor(p.z);
//    
//    float u = fade(x);
//    float v = fade(y);
//    float w = fade(z);
//    
//    int A = permutation[X] + Y;
//    int AA = permutation[A] + Z;
//    int AB = permutation[A + 1] + Z;
//    int B = permutation[X + 1] + Y;
//    int BA = permutation[B] + Z;
//    int BB = permutation[B + 1] + Z;
//    
//    float x1 = mix(grad(permutation[AA], x, y, z),
//                   grad(permutation[BA], x-1, y, z), u);
//    float x2 = mix(grad(permutation[AB], x, y-1, z),
//                   grad(permutation[BB], x-1, y-1, z), u);
//    float y1 = mix(x1, x2, v);
//    
//    float x3 = mix(grad(permutation[AA+1], x, y, z-1),
//                   grad(permutation[BA+1], x-1, y, z-1), u);
//    float x4 = mix(grad(permutation[AB+1], x, y-1, z-1),
//                   grad(permutation[BB+1], x-1, y-1, z-1), u);
//    float y2 = mix(x3, x4, v);
//    
//    return mix(y1, y2, w);
//}
//
//// Voronoi Noise 3D optimisé GPU
//float voronoiNoise3D_GPU(float3 p) {
//    int3 pi = int3(floor(p));
//    float minDist = 10000.0;
//    
//    for (int dx = -1; dx <= 1; dx++) {
//        for (int dy = -1; dy <= 1; dy++) {
//            for (int dz = -1; dz <= 1; dz++) {
//                int3 cell = pi + int3(dx, dy, dz);
//                
//                uint h = hash_gpu(cell.x, cell.y, cell.z);
//                float3 cellPoint = float3(cell) + float3(
//                    float(h % 1000u) / 1000.0,
//                    float((h / 1000u) % 1000u) / 1000.0,
//                    float((h / 1000000u) % 1000u) / 1000.0
//                );
//                
//                float dist = length(p - cellPoint);
//                minDist = min(minDist, dist);
//            }
//        }
//    }
//    
//    return minDist;
//}
//
//uint generatePerlinVoronoi_GPU(int3 worldPos, float blend) {
//    float3 p = float3(worldPos);
//    
//    // Multi-octave Perlin
//    float perlin = 0.0;
//    perlin += perlinNoise3D_GPU(p * 0.01) * 1.0;
//    perlin += perlinNoise3D_GPU(p * 0.02) * 0.5;
//    perlin += perlinNoise3D_GPU(p * 0.04) * 0.25;
//    
//    // Voronoi
//    float voronoi = voronoiNoise3D_GPU(p * 0.05);
//    
//    float mixed = perlin * 0.6 + (1.0 - voronoi * 0.5) * 0.4;
//    mixed *= blend;
//    
//    float baseHeight = 32.0 + perlin * 15.0;
//    
//    if (worldPos.y < baseHeight + mixed * 10.0) {
//        if (worldPos.y < 5) return 8;  // VOID_MATTER
//        if (worldPos.y < baseHeight - 8) return 1;  // STONE
//        if (worldPos.y < baseHeight - 4) return 6;  // METAL
//        if (worldPos.y < baseHeight - 1) return 1;  // STONE
//        
//        if (voronoi < 0.25) return 5;  // ORGANIC
//        if (voronoi < 0.4) return 4;   // CRYSTAL_GREEN
//        return 1;  // STONE
//    }
//    
//    return 0;  // AIR
//}
//
//uint generateVolcano_GPU(int3 worldPos, float distToCenter) {
//    const float coneHeight = 50.0;
//    const float coneRadius = 40.0;
//    
//    float heightFactor = max(0.0, (coneHeight - worldPos.y) / coneHeight);
//    float radiusAtHeight = coneRadius * heightFactor;
//    
//    bool inCrater = (worldPos.y > 45 && worldPos.y < 55 && distToCenter < 12.0);
//    
//    if (inCrater) {
//        float lavaLevel = 48.0 + sin(worldPos.x * 0.2 + worldPos.z * 0.2) * 1.5;
//        if (worldPos.y < lavaLevel) return 2;  // CRYSTAL_RED (lave)
//        return 0;  // AIR
//    }
//    
//    if (distToCenter < radiusAtHeight && worldPos.y < coneHeight && worldPos.y > 5) {
//        float noise = perlinNoise3D_GPU(float3(worldPos) * 0.1);
//        
//        if (noise > 0.4) return 1;   // STONE
//        if (noise > 0.1) return 6;   // METAL
//        return 2;  // CRYSTAL_RED (magma)
//    }
//    
//    if (worldPos.y < 15 && distToCenter < coneRadius * 1.5) {
//        return 1;  // STONE
//    }
//    
//    return 0;  // AIR
//}
//
//uint generateWTF_GPU(int3 worldPos, float blend) {
//    float3 p = float3(worldPos);
//    
//    float gravityNoise = perlinNoise3D_GPU(float3(p.x, 0, p.z) * 0.03);
//    float platformNoise = voronoiNoise3D_GPU(float3(p.x, p.y, p.z) * float3(0.02, 0.08, 0.02));
//    
//    if (platformNoise < 0.15) {
//        uint h = hash_gpu(worldPos.x / 5, worldPos.y / 5, worldPos.z / 5);
//        uint types[] = {2, 3, 4, 7, 5, 6};  // Couleurs variées
//        return types[h % 6];
//    }
//    
//    // Spirales
//    if (gravityNoise > 0.4 && worldPos.y > 25 && worldPos.y < 50) {
//        float spiralAngle = atan2(p.z, p.x) + worldPos.y * 0.15;
//        float spiralDist = length(float2(p.x, p.z));
//        float targetRadius = 20.0 + sin(spiralAngle * 3.0) * 8.0;
//        
//        if (abs(spiralDist - targetRadius) < 3.0) {
//            return 7;  // GLOWING
//        }
//    }
//    
//    // Sol chaotique
//    if (worldPos.y < 20) {
//        float baseNoise = perlinNoise3D_GPU(p * 0.05);
//        if (baseNoise > 0.2) {
//            return (baseNoise > 0.5) ? 6 : 1;  // METAL ou STONE
//        }
//    }
//    
//    return 0;  // AIR
//}
//
//kernel void generateChunkTerrain(
//    device uint8_t* outputBlocks [[buffer(0)]],          // Sortie : blocks[16][64][16]
//    constant BiomeRegion* biomeRegions [[buffer(1)]],    // Entrée : régions de biomes
//    constant ChunkGenerationParams& params [[buffer(2)]], // Entrée : paramètres
//    uint3 gid [[thread_position_in_grid]])               // Position du thread
//{
//    // Chaque thread traite un voxel (x, y, z) du chunk
//    if (gid.x >= CHUNK_SIZE || gid.y >= CHUNK_HEIGHT || gid.z >= CHUNK_SIZE) {
//        return;
//    }
//    
//    // Position mondiale du voxel
//    int3 worldPos = int3(
//        params.chunkX * CHUNK_SIZE + int(gid.x),
//        int(gid.y),
//        params.chunkZ * CHUNK_SIZE + int(gid.z)
//    );
//    
//    // Détermine le biome à cette position
//    float2 pos2D = float2(worldPos.x, worldPos.z);
//    
//    float minDist = 10000.0;
//    uint closestBiomeType = 0;  // PERLIN_VORONOI_MIXED par défaut
//    float closestBlend = 1.0;
//    
//    for (uint i = 0; i < params.numBiomeRegions; i++) {
//        float2 diff = pos2D - biomeRegions[i].center;
//        float dist = length(diff);
//        
//        if (dist < biomeRegions[i].radius && dist < minDist) {
//            minDist = dist;
//            closestBiomeType = biomeRegions[i].type;
//            
//            float blend = 1.0 - (dist / biomeRegions[i].radius);
//            closestBlend = clamp(blend, 0.0, 1.0) * biomeRegions[i].intensity;
//        }
//    }
//    
//    // Génère le bloc selon le biome
//    uint blockType = 0;  // AIR par défaut
//    
//    switch (closestBiomeType) {
//        case 0:  // PERLIN_VORONOI_MIXED
//            blockType = generatePerlinVoronoi_GPU(worldPos, closestBlend);
//            break;
//            
//        case 1: {  // VOLCANO_ACTIVE
//            float2 volcanoCenter = float2(0, 0);  // Simplifié, devrait venir des régions
//            for (uint i = 0; i < params.numBiomeRegions; i++) {
//                if (biomeRegions[i].type == 1) {
//                    volcanoCenter = biomeRegions[i].center;
//                    break;
//                }
//            }
//            float dx = worldPos.x - volcanoCenter.x;
//            float dz = worldPos.z - volcanoCenter.y;
//            float distToCenter = sqrt(dx*dx + dz*dz);
//            blockType = generateVolcano_GPU(worldPos, distToCenter);
//            break;
//        }
//            
//        case 2:  // VORONOI_4D_VOID
//            // TODO: Implémenter Voronoi 4D sur GPU
//            blockType = generatePerlinVoronoi_GPU(worldPos, closestBlend);
//            break;
//            
//        case 3:  // SNOW_PARTICLES
//            blockType = generatePerlinVoronoi_GPU(worldPos, closestBlend);
//            break;
//            
//        case 4:
//            blockType = generateWTF_GPU(worldPos, closestBlend);
//            break;
//    }
//    
//    // Limites verticales
//    if (worldPos.y < 2 || worldPos.y > CHUNK_HEIGHT - 2) {
//        blockType = 0;  // AIR
//    }
//    
//    // Écrit dans le buffer de sortie [x][y][z]
//    uint index = gid.x + gid.y * CHUNK_SIZE + gid.z * CHUNK_SIZE * CHUNK_HEIGHT;
//    outputBlocks[index] = uint8_t(blockType);
//}
//
//// ============================================
//// KERNEL BONUS : Génération de mesh (culling GPU)
//// ============================================
//
//struct VoxelVertexV2 {
//    float3 position;
//    float4 color;
//    float3 normal;
//};
//
//kernel void generateChunkMesh(
//    device VoxelVertexV2* vertices [[buffer(0)]],
//    device uint32_t* indices [[buffer(1)]],
//    device atomic_uint* vertexCounter [[buffer(2)]],
//    device atomic_uint* indexCounter [[buffer(3)]],
//    constant uint8_t* blocks [[buffer(4)]],
//    constant int3& chunkPos [[buffer(5)]],
//    uint3 gid [[thread_position_in_grid]])
//{
//    if (gid.x >= CHUNK_SIZE || gid.y >= CHUNK_HEIGHT || gid.z >= CHUNK_SIZE) {
//        return;
//    }
//    
//    uint index = gid.x + gid.y * CHUNK_SIZE + gid.z * CHUNK_SIZE * CHUNK_HEIGHT;
//    uint8_t blockType = blocks[index];
//    
//    if (blockType == 0) return;  // AIR - pas de mesh
//    
//    // TODO: Culling des faces + ajout des vertices/indices
//    // (Code complet trop long, voir implémentation CPU comme référence)
//}
