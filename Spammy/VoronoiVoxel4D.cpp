//
//  VoronoiVoxel4D.cpp
//  Spammy
//
//  Created by Rémy on 25/12/2025.
//

#include "VoronoiVoxel4D.hpp"

BiomeGenerator::BiomeGenerator(uint32_t seed) : seed(seed)
{
    generateBiomeMap();
}

void BiomeGenerator::generateBiomeMap()
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> posDist(-500.0f, 500.0f);
    std::uniform_real_distribution<float> radiusDist(80.0f, 200.0f);
    
    // Biome principal occupe 60% du monde
    regions.push_back({
        {0.0f, 0.0f},
        400.0f, // 250
        BiomeTypes::PERLIN_VORONOI_MIXED,
        0.1f
    });
    
    for (int i = 0; i < 2; ++i) // 4
    {
        regions.push_back({
            {posDist(rng), posDist(rng)},
            radiusDist(rng) * 0.5f,
            BiomeTypes::VOLCANO_ACTIVE,
            0.8f
        });
    }
    
    regions.push_back({
        {posDist(rng), posDist(rng)},
        radiusDist(rng),
        BiomeTypes::VORONOI_4D_VOID,
        1.0f
    });
    
    for (int i = 0; i < 3; ++i) {
        regions.push_back({
            {posDist(rng), posDist(rng)},
            radiusDist(rng) * 0.7f,
            BiomeTypes::SNOW_PARTICLES,
            0.1f
        });
    }
    
    regions.push_back({
        {posDist(rng), posDist(rng)},
        100.0f,
        BiomeTypes::CHAOS,
        1.0f
    });
}

BiomeTypes BiomeGenerator::getBiomeAt(float worldX, float worldZ)
{
    simd::float2 pos = {worldX, worldZ};
    
    // Trouve le biome le plus proche
    float minDist = 10000.0f;
    BiomeTypes closestBiome = BiomeTypes::PERLIN_VORONOI_MIXED;
    
    for (const auto& region : regions) {
        simd::float2 diff = pos - region.center;
        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
        
        if (dist < region.radius && dist < minDist) {
            minDist = dist;
            closestBiome = region.type;
        }
    }
    return closestBiome;
}

float BiomeGenerator::getBiomeBlend(float worldX, float worldZ, BiomeTypes type)
{
    simd::float2 pos = {worldX, worldZ};
    
    for (const auto& region : regions) {
        if (region.type == type) {
            simd::float2 diff = pos - region.center;
            float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
            
            // Transition douce aux bordures
            float blend = 1.0f - (dist / region.radius);
            return fmaxf(0.0f, fminf(1.0f, blend)) * region.intensity;
        }
    }
    return 0.0f;
}

float BiomeGenerator::perlinNoise3D(float x, float y, float z)
{
    int xi = (int)floorf(x) & 255;
    int yi = (int)floorf(y) & 255;
    int zi = (int)floorf(z) & 255;
    
    float xf = x - floorf(x);
    float yf = y - floorf(y);
    float zf = z - floorf(z);
    
    float u = xf * xf * (3.0f - 2.0f * xf);
    float v = yf * yf * (3.0f - 2.0f * yf);
    float w = zf * zf * (3.0f - 2.0f * zf);
    
    auto hash = [](int x, int y, int z) -> float {
        int n = x + y * 57 + z * 997;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    };

    float a = hash(xi, yi, zi);
    float b = hash(xi + 1, yi, zi);
    float c = hash(xi, yi + 1, zi);
    float d = hash(xi + 1, yi + 1, zi);
    float e = hash(xi, yi, zi + 1);
    float f = hash(xi + 1, yi, zi + 1);
    float g = hash(xi, yi + 1, zi + 1);
    float h = hash(xi + 1, yi + 1, zi + 1);
    
    float x1 = a + u * (b - a);
    float x2 = c + u * (d - c);
    float y1 = x1 + v * (x2 - x1);
    
    float x3 = e + u * (f - e);
    float x4 = g + u * (h - g);
    float y2 = x3 + v * (x4 - x3);
    
    return y1 + w * (y2 - y1);
}

float BiomeGenerator::voronoiNoise3D(float x, float y, float z)
{
    int xi = (int)floorf(x);
    int yi = (int)floorf(y);
    int zi = (int)floorf(z);
    
    float minDist = 10000.0f;
    
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                int cellX = xi + dx;
                int cellY = yi + dy;
                int cellZ = zi + dz;
                
                // Point aléatoire dans la cellule
                uint32_t h = hash(cellX, cellY, cellZ);
                float px = cellX + (h % 1000) / 1000.0f;
                float py = cellY + ((h / 1000) % 1000) / 1000.0f;
                float pz = cellZ + ((h / 1000000) % 1000) / 1000.0f;
                
                float dx2 = x - px;
                float dy2 = y - py;
                float dz2 = z - pz;
                float dist = sqrtf(dx2*dx2 + dy2*dy2 + dz2*dz2);
                
                minDist = fminf(minDist, dist);
            }
        }
    }
    return minDist;
}

BlockType BiomeGenerator::generatePerlinVoronoi(int x, int y, int z, float blend)
{
    float perlin = 0.0f;
    perlin += perlinNoise3D(x * 0.01f, y * 0.01f, z * 0.01f) * 1.0f;
    perlin += perlinNoise3D(x * 0.02f, y * 0.02f, z * 0.02f) * 0.5f;
    perlin += perlinNoise3D(x * 0.04f, y * 0.04f, z * 0.04f) * 0.25f;

    float voronoi = voronoiNoise3D(x * 0.05f, y * 0.05f, z * 0.05f);

    float mixed = perlin * 0.6f + (1.0f - voronoi) * 0.4f;
    mixed *= blend;

    float baseHeight = 50.0f + perlin * 30.0f;
    
    if (y < baseHeight + mixed * 20.0f) {
        // Couches géologiques
        if (y < baseHeight - 10) return BlockType::STONE;
        if (y < baseHeight - 5) return BlockType::METAL;
        if (y < baseHeight - 2) return BlockType::STONE;
        
        // Surface
        if (voronoi < 0.3f) return BlockType::ORGANIC;
        return BlockType::CRYSTAL_GREEN;
    }
    
    return BlockType::AIR;
}

BlockType BiomeGenerator::generateVolcano(int x, int y, int z, float distToCenter)
{
    float coneHeight = 80.0f;
    float coneRadius = 60.0f;
    
    float heightFactor = (coneHeight - y) / coneHeight;
    float radiusAtHeight = coneRadius * heightFactor;
    
    float dist2D = distToCenter;

    bool inCrater = (y > 70.0f && y < 85.0f && dist2D < 15.0f);
    
    if (inCrater) {
        float lavaLevel = 75.0f + sinf(x * 0.1f + z * 0.1f) * 2.0f;
        if (y < lavaLevel) {
            return BlockType::CRYSTAL_RED;
        }
        return BlockType::AIR;
    }
    
    if (dist2D < radiusAtHeight && y < coneHeight) {
        // Couches de roche volcanique
        float noise = perlinNoise3D(x * 0.1f, y * 0.1f, z * 0.1f);
        
        if (noise > 0.3f) return BlockType::STONE;
        if (noise > 0.0f) return BlockType::METAL;  // Roche sombre
        return BlockType::CRYSTAL_RED;  // Veines de magma
    }
    
    return BlockType::AIR;
}

BlockType BiomeGenerator::generateWTF(int x, int y, int z, float blend)
{
    float gravityNoise = perlinNoise3D(x * 0.05f, 0, z * 0.05f);
    
    // Plateformes flottantes aléatoires
    float platformNoise = voronoiNoise3D(x * 0.02f, y * 0.1f, z * 0.02f);
    
    if (platformNoise < 0.2f) {
        // Matériaux changeants
        uint32_t h = hash(x / 5, y / 5, z / 5);
        BlockType types[] = {
            BlockType::CRYSTAL_RED,
            BlockType::CRYSTAL_BLUE,
            BlockType::CRYSTAL_GREEN,
            BlockType::GLOWING,
            BlockType::ORGANIC
        };
        return types[h % 5];
    }
    
    // Structures impossibles
    if (gravityNoise > 0.5f && y > 60.0f) {
        float spiralAngle = atan2f(z, x) + y * 0.1f;
        float spiralDist = sqrtf(x*x + z*z);
        
        if (fabs(spiralDist - 30.0f - sinf(spiralAngle) * 10.0f) < 3.0f) {
            return BlockType::GLOWING;  // Spirale lumineuse
        }
    }
    
    return BlockType::AIR;
}

VoronoiVoxel4D::VoronoiVoxel4D(uint32_t seed)
    : rng(seed), timeOffset(0.0f)
{
}

VoronoiVoxel4D::~VoronoiVoxel4D()
{
}

uint32_t hash(int x, int y, int z)
{
    uint32_t h = 2166136261u;
    h = (h ^ x) * 16777619u;
    h = (h ^ y) * 16777619u;
    h = (h ^ z) * 16777619u;
    return h;
}

void VoronoiVoxel4D::generateSitesForRegion(int chunkX, int chunkZ, float time)
{
    sites.clear();
    
    // Génère 50-80 sites par région autour du chunk
    const int regionSize = 3; // 3x3 chunks
    int numSites = 50 + (hash(chunkX, chunkZ, (int)time) % 30);
    
    for (int i = 0; i < numSites; ++i)
    {
        uint32_t h = hash(chunkX * 1000 + i, chunkZ * 1000, (int)(time * 100));
        std::mt19937 localRng(h);
        std::uniform_real_distribution<float> dist(-regionSize * CHUNK_SIZE * 0.5f, regionSize * CHUNK_SIZE * 1.5f);
        std::uniform_real_distribution<float> distTime(-10.0f, 10.0f);
        std::uniform_real_distribution<float> distInfluence(8.0f, 25.0f);
        
        VoronoiSite4D site;
        site.position = { chunkX * CHUNK_SIZE + dist(localRng), dist(localRng) * 0.3f + 40.0f, chunkZ * CHUNK_SIZE + dist(localRng), time + distTime(localRng) }; // Centré 40 / Y
        
        // Type de bloc basé sur position 4D
        float typeNoise = sinf(site.position.x * 0.1f) * cosf(site.position.w * 0.2f);
        if (typeNoise > 0.6f) site.blockType = BlockType::CRYSTAL_RED;
        else if (typeNoise > 0.3f) site.blockType = BlockType::CRYSTAL_BLUE;
        else if (typeNoise > 0.0f) site.blockType = BlockType::CRYSTAL_GREEN;
        else if (typeNoise > -0.3f) site.blockType = BlockType::ORGANIC;
        else if (typeNoise > -0.6f) site.blockType = BlockType::METAL;
        else site.blockType = BlockType::STONE;
        
        site.influence = distInfluence(localRng);
        sites.push_back(site);
    }
}

void VoronoiVoxel4D::generateSitesForRegion2(int chunkX, int chunkZ, float time)
{
    sites.clear();
    
    // Génère 80-150 sites pour structure organique dense
    int numSites = 80 + (hash(chunkX, chunkZ, (int)time) % 70);
    
    for (int i = 0; i < numSites; ++i) {
        uint32_t h = hash(chunkX * 1000 + i, chunkZ * 1000, (int)(time * 100));
        std::mt19937 localRng(h);
        std::uniform_real_distribution<float> distXZ(-CHUNK_SIZE * 2.0f, CHUNK_SIZE * 3.0f);
        std::uniform_real_distribution<float> distY(10.0f, CHUNK_HEIGHT - 10.0f);
        std::uniform_real_distribution<float> distInfluence(12.0f, 35.0f);
        
        VoronoiSite4D site;
        site.position = {
            chunkX * CHUNK_SIZE + distXZ(localRng),
            distY(localRng),
            chunkZ * CHUNK_SIZE + distXZ(localRng),
            time + localRng() % 20 - 10.0f
        };
        
        // Biome basé sur hauteur Y + noise 3D
        float heightFactor = site.position.y / CHUNK_HEIGHT;
        float noise3D = sinf(site.position.x * 0.08f) * cosf(site.position.y * 0.05f) * sinf(site.position.z * 0.08f);
        
        if (heightFactor > 0.75f)
            site.blockType = (noise3D > 0) ? BlockType::CRYSTAL_BLUE : BlockType::GLOWING;
        else if (heightFactor > 0.5f)
        {
            if (noise3D > 0.4f)
                site.blockType = BlockType::ORGANIC;
            else if (noise3D > 0.0f)
                site.blockType = BlockType::CRYSTAL_GREEN;
            else
                site.blockType = BlockType::STONE;
        }
        else if (heightFactor > 0.25f)
        {
            if (noise3D > 0.5f)
                site.blockType = BlockType::CRYSTAL_RED;
            else if (noise3D > 0.0f)
                site.blockType = BlockType::METAL;
            else
                site.blockType = BlockType::STONE;
        }
        else
            site.blockType = (noise3D > 0) ? BlockType::VOID_MATTER : BlockType::STONE;
        
        site.influence = distInfluence(localRng);
        sites.push_back(site);
    }
}

float VoronoiVoxel4D::distance4D(simd::float4 a, simd::float4 b)
{
    simd::float4 diff = a - b;
    return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z + diff.w * diff.w);
}

VoronoiSite4D* VoronoiVoxel4D::findClosestSite(simd::float3 worldPos, float time)
{
    simd::float4 pos4D = {worldPos.x, worldPos.y, worldPos.z, time};

    VoronoiSite4D* closest = nullptr;
    float minDist = std::numeric_limits<float>::max();

    for (auto& site : sites)
    {
        float dist = distance4D(pos4D, site.position);
        if (dist < minDist)
        {
            minDist = dist;
            closest = &site;
        }
    }
    return closest;
}

float VoronoiVoxel4D::worleyNoise(simd::float3 pos, float time)
{
    VoronoiSite4D* site = findClosestSite(pos, time);
    if (!site) return 1.0f;
    
    simd::float4 pos4D = {pos.x, pos.y, pos.z, time};
    float dist = distance4D(pos4D, site->position);
    
    return fminf(1.0f, dist / site->influence);
}

float VoronoiVoxel4D::worleyNoise2(simd::float3 pos, float time)
{
    // Distance aux 3 sites les plus proches
    simd::float4 pos4D = {pos.x, pos.y, pos.z, time};
    
    float distances[3] =
    {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };
    
    for (auto& site : sites)
    {
        float dist = distance4D(pos4D, site.position);
        
        if (dist < distances[0])
        {
            distances[2] = distances[1];
            distances[1] = distances[0];
            distances[0] = dist;
        } else if (dist < distances[1]) {
            distances[2] = distances[1];
            distances[1] = dist;
        } else if (dist < distances[2]) {
            distances[2] = dist;
        }
    }
    
    // Combine les distances pour créer des patterns
    float f1 = distances[0] / 30.0f;
    float f2 = distances[1] / 30.0f;
    
    return f2 - f1; // Frontières de cellules
}

BlockType VoronoiVoxel4D::getBlockAtPosition(int worldX, int worldY, int worldZ, float time)
{
    simd::float3 pos = {(float)worldX, (float)worldY, (float)worldZ};
    
    VoronoiSite4D* site = findClosestSite(pos, time);
    if (!site) return BlockType::AIR; // STONE
    
    simd::float4 pos4D = {pos.x, pos.y, pos.z, time};
    float dist = distance4D(pos4D, site->position);
    float worley = worleyNoise(pos, time);
    float density = 1.0f - (dist / site->influence);

    density += worley * 0.3f;

    if (density < 0.4f) return BlockType::AIR;

    // Transitions douces entre matériaux
    if (density > 0.8f) {
        return site->blockType; // Coeur solide
    } else if (density > 0.6f) {
        // Zone de transition
        if (site->blockType == BlockType::ORGANIC) return BlockType::CRYSTAL_GREEN;
        if (site->blockType == BlockType::CRYSTAL_RED) return BlockType::METAL;
        return site->blockType;
    } else {
        // Périphérie poreuse
        return (worley > 0.15f) ? BlockType::STONE : BlockType::AIR;
    }

    // Distance au site détermine le type
    if (dist < site->influence * 0.3f) {
        return site->blockType; // Coeur du site
    } else if (dist < site->influence * 0.6f) {
        // Zone de transition
        return (site->blockType == BlockType::CRYSTAL_RED) ? BlockType::STONE : site->blockType;
    } else if (dist < site->influence) {
        return BlockType::STONE;
    }
    
    return BlockType::AIR;
}

BlockType VoxelWorld::getBlockAtPositionBiomed(int worldX, int worldY, int worldZ, float time)
{
    if (!biomeGen) {
        return voronoiGen.getBlockAtPosition(worldX, worldY, worldZ, time);
    }
    
    BiomeTypes biome = biomeGen->getBiomeAt(worldX, worldZ);
    float blend = biomeGen->getBiomeBlend(worldX, worldZ, biome);
    
    switch (biome) {
        case BiomeTypes::PERLIN_VORONOI_MIXED:
            return biomeGen->generatePerlinVoronoi(worldX, worldY, worldZ, blend);
            
        case BiomeTypes::VOLCANO_ACTIVE: {
            simd::float2 center = {0, 0};
            float dist = sqrtf(worldX*worldX + worldZ*worldZ);
            return biomeGen->generateVolcano(worldX, worldY, worldZ, dist);
        }
//        case BiomeType::VOLCANO_ACTIVE:
//        {
////            const auto& regions = biomeGen->getRegions();
//            float minDist = 10000.0f;
//            simd::float2 volcanoCenter = {0, 0};
//            for (const auto& region : regions)
//            {
//                if (region.type == BiomeType::VOLCANO_ACTIVE)
//                {
//                    simd::float2 pos = {(float)worldX, (float)worldZ};
//                    simd::float2 diff = pos - region.center;
//                    float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
//                    if (dist < minDist)
//                    {
//                        minDist = dist;
//                        volcanoCenter = region.center;
//                    }
//                }
//            }
//            float dx = worldX - volcanoCenter.x;
//            float dz = worldZ - volcanoCenter.y;
//            float distToCenter = sqrtf(dx*dx + dz*dz);
//            return biomeGen->generateVolcano(worldX, worldY, worldZ, distToCenter);
//        }
            
        case BiomeTypes::VORONOI_4D_VOID:
            return voronoiGen.getBlockAtPosition(worldX, worldY, worldZ, time);  // Votre système actuel
            
        case BiomeTypes::SNOW_PARTICLES:
            // Terrain de base + flag pour particules de neige
            return biomeGen->generatePerlinVoronoi(worldX, worldY, worldZ, blend);
            
        case BiomeTypes::CHAOS:
            return biomeGen->generateWTF(worldX, worldY, worldZ, blend);
    }
    
    return BlockType::AIR;
}


Chunk::Chunk(int x, int z) : chunkX(x), chunkZ(z), vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0), needsRebuild(true)
{
    ft_memset(blocks, 0, sizeof(blocks));
//    for (int x = 0; x < CHUNK_SIZE; ++x)
//    {
//        for (int y = 0; y < CHUNK_HEIGHT; ++y)
//        {
//            for (int z = 0; z < CHUNK_SIZE; ++z)
//                blocks[x][y][z] = BlockType::AIR;
//        }
//    }
}

Chunk::~Chunk()
{
    vertexBuffer->release();
    indexBuffer->release();
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
        return BlockType::AIR;
    return blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, BlockType type)
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
        return;
    blocks[x][y][z] = type;
    needsRebuild = true;
}

bool Chunk::isBlockSolid(int x, int y, int z) const
{
    BlockType type = getBlock(x, y, z);
    return type != BlockType::AIR;
}

void Chunk::addCubeFace(std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices, simd::float3 pos, BlockType type, int face)
{
    simd::float4 color = BLOCK_COLORS[(int)type];
    uint32_t baseIndex = (uint32_t)vertices.size();

    simd::float3 normals[6] = { {0, 1, 0}, {0, -1, 0}, {0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {-1, 0, 0} };

    simd::float3 faceVertices[6][4] =
    {
        {{0,1,0}, {1,1,0}, {1,1,1}, {0,1,1}},
        {{0,0,0}, {0,0,1}, {1,0,1}, {1,0,0}},
        {{0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}},
        {{1,0,1}, {0,0,1}, {0,1,1}, {1,1,1}},
        {{1,0,0}, {1,0,1}, {1,1,1}, {1,1,0}},
        {{0,0,1}, {0,0,0}, {0,1,0}, {0,1,1}}
    };

    float brightness[6] = {1.0f, 0.5f, 0.7f, 0.7f, 0.9f, 0.6f};
    simd::float4 shadedColor = color * brightness[face];
    shadedColor.w = color.w;

    float variation = (hash((int)pos.x, (int)pos.y, (int)pos.z) % 100) / 1000.0f;
    shadedColor.x += variation;
    shadedColor.y += variation;
    shadedColor.z += variation;

    for (int i = 0; i < 4; ++i)
    {
        VoxelVertex v;
        v.position = pos + faceVertices[face][i] * VOXELSIZE;
        v.color = shadedColor;
        v.normal = normals[face];
        vertices.push_back(v);
    }

    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

void Chunk::rebuildMesh(MTL::Device* device)
{
    if (!needsRebuild)
        return;
    
    std::vector<VoxelVertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * 128 * 4);
    indices.reserve(CHUNK_SIZE * CHUNK_SIZE * 128 * 6);
    
    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_SIZE; ++z)
            {
                BlockType type = blocks[x][y][z];
                if (type == BlockType::AIR)
                    continue;
                
                simd::float3 worldPos = { (float)(chunkX * CHUNK_SIZE + x), (float)y, (float)(chunkZ * CHUNK_SIZE + z) };
                
                if (!isBlockSolid(x, y + 1, z))
                    addCubeFace(vertices, indices, worldPos, type, 0);
                if (!isBlockSolid(x, y - 1, z))
                    addCubeFace(vertices, indices, worldPos, type, 1);
                if (!isBlockSolid(x, y, z - 1))
                    addCubeFace(vertices, indices, worldPos, type, 2);
                if (!isBlockSolid(x, y, z + 1))
                    addCubeFace(vertices, indices, worldPos, type, 3);
                if (!isBlockSolid(x + 1, y, z))
                    addCubeFace(vertices, indices, worldPos, type, 4);
                if (!isBlockSolid(x - 1, y, z))
                    addCubeFace(vertices, indices, worldPos, type, 5);
            }
        }
    }

    if (vertexBuffer)
        vertexBuffer->release();
    if (indexBuffer)
        indexBuffer->release();

    if (vertices.empty())
    {
        vertexBuffer = nullptr;
        indexBuffer = nullptr;
        indexCount = 0;
    }
    else
    {
        vertexBuffer = device->newBuffer(vertices.data(), vertices.size() * sizeof(VoxelVertex), MTL::ResourceStorageModeShared);
        indexBuffer = device->newBuffer(indices.data(), indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
        indexCount = (uint32_t)indices.size();
    }
    needsRebuild = false;
}


VoxelWorld::VoxelWorld(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary)
: voronoiGen(89), currentTime(0.0f)
{
    createPipeline(pShaderLibrary, pPixelFormat, pDepthPixelFormat, pDevice);
}

VoxelWorld::~VoxelWorld()
{
    for (auto& [key, chunk] : chunks)
        delete chunk;
    m_renderPipelineState->release();
}
//void VoxelWorld::generateTerrainBiomed(int chunkX, int chunkZ)
//{
//    Chunk* chunk = getChunk(chunkX, chunkZ);
//    
//    for (int x = 0; x < CHUNK_SIZE; ++x) {
//        for (int z = 0; z < CHUNK_SIZE; ++z) {
//            int worldX = chunkX * CHUNK_SIZE + x;
//            int worldZ = chunkZ * CHUNK_SIZE + z;
//            y
//            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
//                BlockType type = voronoiGen.getBlockAtPositionBiomed(
//                    worldX, y, worldZ, currentTime
//                );
//                chunk->setBlock(x, y, z, type);
//            }
//        }
//    }
//}

inline int floordiv(int a, int b)
{
    int r = a / b;
    if ((a ^ b) < 0 && a % b)
        --r;
    return r;
}

inline int floormod(int a, int b)
{
    int m = a % b;
    if (m < 0)
        m += b;
    return m;
}

void VoxelWorld::worldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ, int& localX, int& localZ)
{
    chunkX = worldX >> 5; // Division par 32
    chunkZ = worldZ >> 5;
    localX = worldX & 31; // Modulo 32
    localZ = worldZ & 31;
    
    if (worldX < 0 && localX != 0)
    {
        chunkX--;
        localX = CHUNK_SIZE + localX;
    }
    if (worldZ < 0 && localZ != 0)
    {
        chunkZ--;
        localZ = CHUNK_SIZE + localZ;
    }
}

Chunk* VoxelWorld::getChunk(int chunkX, int chunkZ)
{
    uint64_t key = chunkKey(chunkX, chunkZ);
    auto it = chunks.find(key);

    if (it != chunks.end())
        return it->second;

    Chunk* chunk = new Chunk(chunkX, chunkZ);
    chunks[key] = chunk;
    generateTerrainVoronoi(chunkX, chunkZ);
    return chunk;
}

void VoxelWorld::generateTerrainVoronoi(int chunkX, int chunkZ)
{
    Chunk* chunk = getChunk(chunkX, chunkZ);

    voronoiGen.generateSitesForRegion(chunkX, chunkZ, currentTime);

    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE; ++z)
        {
            int worldX = chunkX * CHUNK_SIZE + x;
            int worldZ = chunkZ * CHUNK_SIZE + z;

            for (int y = 0; y < CHUNK_HEIGHT; ++y)
            {
//                BlockType type = voronoiGen.getBlockAtPosition(worldX, y, worldZ, currentTime);
                BlockType type;
                if (biomeGen)
                    type = getBlockAtPositionBiomed(worldX, y, worldZ, currentTime);
                else
                    type = voronoiGen.getBlockAtPosition(worldX, y, worldZ, currentTime);

                if (y < 5 || y > 100)
                    type = BlockType::AIR;

                chunk->setBlock(x, y, z, type);
            }
        }
    }
}

BlockType VoxelWorld::getBlock(int worldX, int worldY, int worldZ)
{
    if (worldY < 0 || worldY >= CHUNK_HEIGHT)
        return BlockType::AIR;

    int chunkX, chunkZ, localX, localZ;
    worldToChunk(worldX, worldZ, chunkX, chunkZ, localX, localZ);

    Chunk* chunk = getChunk(chunkX, chunkZ);
    return chunk->getBlock(localX, worldY, localZ);
}

void VoxelWorld::setBlock(int worldX, int worldY, int worldZ, BlockType type)
{
    if (worldY < 0 || worldY >= CHUNK_HEIGHT)
        return;

    int chunkX, chunkZ, localX, localZ;
    worldToChunk(worldX, worldZ, chunkX, chunkZ, localX, localZ);

    Chunk* chunk = getChunk(chunkX, chunkZ);
    chunk->setBlock(localX, worldY, localZ, type);
}

void VoxelWorld::removeBlock(int worldX, int worldY, int worldZ)
{
    setBlock(worldX, worldY, worldZ, BlockType::AIR);
}

void VoxelWorld::createPipeline(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device)
{
    NS::Error* error = nullptr;

    NS::SharedPtr<MTL::Function> pVertexFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("voxel_vertex")));
    NS::SharedPtr<MTL::Function> pFragmentFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("voxel_fragment")));

    NS::SharedPtr<MTL::RenderPipelineDescriptor> pRenderDescriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pRenderDescriptor->setVertexFunction(pVertexFunction.get());
    pRenderDescriptor->setFragmentFunction(pFragmentFunction.get());
    pRenderDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pRenderDescriptor->setDepthAttachmentPixelFormat(depthPixelFormat);
    
    NS::SharedPtr<MTL::VertexDescriptor> pVertexDesc = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
    pVertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(0)->setOffset(0);
    pVertexDesc->attributes()->object(0)->setBufferIndex(0);

    pVertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
    pVertexDesc->attributes()->object(1)->setOffset(sizeof(simd::float3));
    pVertexDesc->attributes()->object(1)->setBufferIndex(0);

    pVertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(2)->setOffset(sizeof(simd::float3) + sizeof(simd::float4));
    pVertexDesc->attributes()->object(2)->setBufferIndex(0);

    pVertexDesc->layouts()->object(0)->setStride(sizeof(VoxelVertex));
    pVertexDesc->layouts()->object(0)->setStepRate(1);
    pVertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);

    pRenderDescriptor->setVertexDescriptor(pVertexDesc.get());
    m_renderPipelineState = device->newRenderPipelineState(pRenderDescriptor.get(), &error);
    m_depthStencilState = device->newDepthStencilState(depthStencilDescriptor.get());
}

void VoxelWorld::update(float dt, simd::float3 cameraPos, MTL::Device* device)
{
    int camChunkX = (int)floorf(cameraPos.x / CHUNK_SIZE);
    int camChunkZ = (int)floorf(cameraPos.z / CHUNK_SIZE);

    for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; ++x)
    {
        for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; ++z)
        {
            int chunkX = camChunkX + x;
            int chunkZ = camChunkZ + z;
            Chunk* chunk = getChunk(chunkX, chunkZ);

            if (chunk->needsRebuild)
                chunk->rebuildMesh(device);
        }
    }
    for (auto it = chunks.begin(); it != chunks.end();)
    {
        Chunk* chunk = it->second;
        int dx = abs(chunk->chunkX - camChunkX);
        int dz = abs(chunk->chunkZ - camChunkZ);

        if (dx > RENDER_DISTANCE + 3 || dz > RENDER_DISTANCE + 3)
        {
            delete chunk;
            it = chunks.erase(it);
        }
        else
            ++it;
    }
}

void VoxelWorld::render(MTL::RenderCommandEncoder* renderCommandEncoder, simd::float4x4 viewProjectionMatrix)
{
    renderCommandEncoder->setRenderPipelineState(m_renderPipelineState);
    renderCommandEncoder->setDepthStencilState(m_depthStencilState);
    renderCommandEncoder->setCullMode(MTL::CullModeBack);
    for (auto& [key, chunk] : chunks)
    {
        if (chunk->indexCount == 0)
            continue;
        renderCommandEncoder->setVertexBuffer(chunk->vertexBuffer, 0, 0);
        renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, chunk->indexCount, MTL::IndexTypeUInt32, chunk->indexBuffer, 0);
    }
}

bool VoxelWorld::raycast(simd::float3 origin, simd::float3 direction, float maxDistance, simd::int3& hitBlock, simd::int3& adjacentBlock)
{
    simd::float3 pos = origin;
    simd::float3 step = simd::normalize(direction) * 0.1f;
    simd::float3 prevPos = pos;

    for (float dist = 0; dist < maxDistance; dist += 0.1f)
    {
        int x = (int)floorf(pos.x);
        int y = (int)floorf(pos.y);
        int z = (int)floorf(pos.z);

        if (getBlock(x, y, z) != BlockType::AIR)
        {
            hitBlock = {x, y, z};
            adjacentBlock = { (int)floorf(prevPos.x), (int)floorf(prevPos.y), (int)floorf(prevPos.z) };
            return true;
        }
        prevPos = pos;
        pos += step;
    }
    return false;
}
