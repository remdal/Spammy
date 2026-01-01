//
//  RMDLPerlinXVoronoiMap.metal
//  Spammy
//
//  Created by Rémy on 01/01/2026.
//

#include <metal_stdlib>
using namespace metal;

struct TerrainBlocks
{
    float height;
    float metallic;
    float roughness;
    uint blockType;
};

struct Generation
{
    uint width;
    uint height;
    float scale;
    int octaves;
    float persistence;
    float lacunarity;
    uint seed;
    int voronoiCount;
};

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

float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float perlin3D(float3 p) {
    int X = int(floor(p.x)) & 255;
    int Y = int(floor(p.y)) & 255;
    int Z = int(floor(p.z)) & 255;
    
    float x = p.x - floor(p.x);
    float y = p.y - floor(p.y);
    float z = p.z - floor(p.z);
    
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);
    
    int A = permutation[X] + Y;
    int AA = permutation[A] + Z;
    int AB = permutation[A + 1] + Z;
    int B = permutation[X + 1] + Y;
    int BA = permutation[B] + Z;
    int BB = permutation[B + 1] + Z;
    
    float x1 = mix(grad(permutation[AA], x, y, z),
                   grad(permutation[BA], x-1, y, z), u);
    float x2 = mix(grad(permutation[AB], x, y-1, z),
                   grad(permutation[BB], x-1, y-1, z), u);
    float y1 = mix(x1, x2, v);
    
    float x3 = mix(grad(permutation[AA+1], x, y, z-1),
                   grad(permutation[BA+1], x-1, y, z-1), u);
    float x4 = mix(grad(permutation[AB+1], x, y-1, z-1),
                   grad(permutation[BB+1], x-1, y-1, z-1), u);
    float y2 = mix(x3, x4, v);
    
    return mix(y1, y2, w);
}

float perlinFBM(float3 p, int octaves, float persistence, float lacunarity) {
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxValue = 0.0;
    
    for(int i = 0; i < octaves; i++) {
        total += perlin3D(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

float3 hash3D(uint seed, uint index, float width, float height)
{
    uint s = seed + index * 1000u;
    uint x = s * 747796405u + 2891336453u;
    uint y = x * 747796405u + 2891336453u;
    uint z = y * 747796405u + 2891336453u;
    
    return float3(
        float(x) / 4294967295.0 * width,
        float(y) / 4294967295.0 * height,
        float(z) / 4294967295.0 * 100.0
    );
}

float voronoi3D(float3 p, uint seed, int count, float width, float height) {
    float minDist = 999999.0;
    
    for (int i = 0; i < count; i++)
    {
        float3 point = hash3D(seed, uint(i), width, height);
        float dist = distance(p, point);
        minDist = min(minDist, dist);
    }
    
    return clamp(minDist / (width * 0.3), 0.0, 1.0);
}

float combinedNoise(float3 p, constant Generation& params)
{
    float3 scaledP = p / params.scale;
    
    float perlin = perlinFBM(scaledP, params.octaves, params.persistence, params.lacunarity);
    float voronoi = voronoi3D(p, params.seed, params.voronoiCount,
                              float(params.width), float(params.height));
    
    float combined = perlin * 0.5 + (1.0 - voronoi) * 0.5;
    combined = pow(combined * 0.5 + 0.5, 1.5);
    
    return combined;
}

uint determineBlockType(float height, float metallic)
{
    if (height < 0.3) return 1;
    if (height < 0.6) return 2;
    if (height < 0.85) return (metallic > 0.85) ? 4 : 2;
    return 3;
}

kernel void generateTerrain(device TerrainBlocks* blocks [[buffer(0)]],
                            constant Generation& params [[buffer(1)]],
                            uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= params.width || gid.y >= params.height) return;
    
    uint idx = gid.y * params.width + gid.x;
    float3 position = float3(float(gid.x), float(gid.y), 0.0);
    
    float heightValue = combinedNoise(position, params);
    
    float3 metallicPos = position / (params.scale * 0.5);
    metallicPos.z += 100.0;
    float metallicValue = perlinFBM(metallicPos, params.octaves,
                                    params.persistence, params.lacunarity) * 0.5 + 0.5;
    
    blocks[idx].height = heightValue;
    blocks[idx].metallic = metallicValue;
    blocks[idx].roughness = 1.0 - (heightValue * 0.3);
    blocks[idx].blockType = determineBlockType(heightValue, metallicValue);
}

kernel void renderHeightmap(device TerrainBlocks* blocks [[buffer(0)]],
                            texture2d<float, access::write> outTexture [[texture(0)]],
                            constant Generation& params [[buffer(1)]],
                            uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= params.width || gid.y >= params.height) return;
    
    uint idx = gid.y * params.width + gid.x;
    float4 color;
    
    if (blocks[idx].blockType == 1) color = float4(0.4, 0.3, 0.2, 1.0);
    else if (blocks[idx].blockType == 2) color = float4(0.5, 0.5, 0.5, 1.0);
    else if (blocks[idx].blockType == 3) color = float4(1.0, 1.0, 1.0, 1.0);
    else if (blocks[idx].blockType == 4) color = float4(0.8, 0.6, 0.2, 1.0);
    else color = float4(0.0, 0.0, 0.0, 1.0);
    
    color.rgb *= (0.5 + blocks[idx].height * 0.5);
    outTexture.write(color, gid);
}

kernel void renderMetallicMap(device TerrainBlocks* blocks [[buffer(0)]],texture2d<float,
                              access::write> outTexture [[texture(0)]],
                              constant Generation& params [[buffer(1)]],
                              uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= params.width || gid.y >= params.height) return;
    uint idx = gid.y * params.width + gid.x;
    float m = blocks[idx].metallic;
    outTexture.write(float4(m, m, m, 1.0), gid);
}

kernel void renderRoughnessMap(device TerrainBlocks* blocks [[buffer(0)]],
                               texture2d<float, access::write> outTexture [[texture(0)]],
                               constant Generation& params [[buffer(1)]],
                               uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= params.width || gid.y >= params.height) return;
    uint idx = gid.y * params.width + gid.x;
    float r = blocks[idx].roughness;
    outTexture.write(float4(r, r, r, 1.0), gid);
}
