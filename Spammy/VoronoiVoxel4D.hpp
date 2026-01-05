//
//  VoronoiVoxel4D.hpp
//  Spammy
//
//  Created by Rémy on 25/12/2025.
//

#ifndef VoronoiVoxel4D_hpp
#define VoronoiVoxel4D_hpp

#include <Metal/Metal.hpp>

#include <stdio.h>
#include <simd/simd.h>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>

#include "RMDLUtils.hpp"
#include "RMDLMainRenderer_shared.h"

static constexpr int CHUNK_SIZE = 64;
static constexpr int CHUNK_HEIGHT = 64;
static constexpr float VOXEL_SIZE = 1.0f;

enum class BlockType : uint8_t {
    AIR = 0,
    STONE,
    CRYSTAL_RED,
    CRYSTAL_BLUE,
    CRYSTAL_GREEN,
    ORGANIC,
    METAL,
    GLOWING,
    VOID_MATTER
};

const simd::float4 BLOCK_COLORS[] = {
    {0.0f, 0.0f, 0.0f, 0.0f},      // AIR
    {0.4f, 0.4f, 0.45f, 1.0f},     // STONE
    {0.9f, 0.2f, 0.2f, 1.0f},      // CRYSTAL_RED
    {0.2f, 0.4f, 0.9f, 1.0f},      // CRYSTAL_BLUE
    {0.2f, 0.9f, 0.4f, 1.0f},      // CRYSTAL_GREEN
    {0.6f, 0.5f, 0.3f, 1.0f},      // ORGANIC
    {0.7f, 0.7f, 0.8f, 1.0f},      // METAL
    {1.0f, 0.9f, 0.5f, 1.0f},      // GLOWING
    {0.1f, 0.05f, 0.15f, 1.0f}     // VOID_MATTER
};

struct VoxelVertex
{
    simd::float3 position;
    simd::float4 color;
    simd::float3 normal;
};

// Point Voronoi dans l'espace 4D
struct VoronoiSite4D
{
    simd::float4 position;
    BlockType blockType;
    float influence;
};

// Hash pour positions cohérentes
uint32_t hash(int x, int y, int z);

enum class BiomeType {
    PERLIN_VORONOI_MIXED,
    VOLCANO_ACTIVE,
    VORONOI_4D_VOID,
    SNOW_PARTICLES,
    CHAOS // Physics & Obj WTF
};

struct BiomeRegion
{
    simd::float2 center;
    float radius;
    BiomeType type;
    float intensity; // 0-1 pour transition
};

struct SnowFlake {
    simd::float3 position;
    simd::float3 velocity;
    float size;
    float rotation;
    float lifetime;
};

class EnhancedSnowSystem {
public:
    void update(float dt, BiomeType currentBiome);
    void spawnFlakesInBiome(simd::float3 center, float radius);
    
private:
    std::vector<SnowFlake> flakes;
    MTL::Buffer* particleBuffer;
};

class BiomeGenerator
{
public:
    BiomeGenerator(uint32_t seed);

    BiomeType getBiomeAt(float worldX, float worldZ);
    float getBiomeBlend(float worldX, float worldZ, BiomeType type);

    BlockType generatePerlinVoronoi(int x, int y, int z, float blend);
    BlockType generateVolcano(int x, int y, int z, float distToCenter);
    BlockType generateVoronoi4D(int x, int y, int z, float time);
    BlockType generateWTF(int x, int y, int z, float blend);

private:
    std::vector<BiomeRegion> regions;
    uint32_t seed;

    void generateBiomeMap();
    float perlinNoise3D(float x, float y, float z);
    float voronoiNoise3D(float x, float y, float z);
};

class VoronoiVoxel4D
{
public:
    VoronoiVoxel4D(uint32_t seed = 89);
    ~VoronoiVoxel4D();
    
    // Génère sites pour une région 3D à un temps donné
    void generateSitesForRegion(int chunkX, int chunkZ, float time);
    void generateSitesForRegion2(int chunkX, int chunkZ, float time);
    
    // Trouve le site Voronoi le plus proche en 4D
    VoronoiSite4D* findClosestSite(simd::float3 worldPos, float time);
    
    // Distance 4D entre deux points
    float distance4D(simd::float4 a, simd::float4 b);
    
    // Génère le bloc à cette position en utilisant Voronoi
    BlockType getBlockAtPosition(int worldX, int worldY, int worldZ, float time);
    
    // Worley noise (distance au plus proche site)
    float worleyNoise(simd::float3 pos, float time);
    float worleyNoise2(simd::float3 pos, float time);
    
    void setTimeOffset(float t) { timeOffset = t; }
    float getTimeOffset() const { return timeOffset; }

private:
    std::vector<VoronoiSite4D>  sites;
    std::mt19937                rng;
    float                       timeOffset; // 4ème dimension = temps
};

class Chunk
{
public:
    int             chunkX, chunkZ;
    BlockType       blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    
    MTL::Buffer*    vertexBuffer;
    MTL::Buffer*    indexBuffer;
    uint32_t        indexCount;
    bool            needsRebuild;
    
    Chunk(int x, int z);
    ~Chunk();
    
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);
    bool isBlockSolid(int x, int y, int z) const;
    
    void rebuildMesh(MTL::Device* device);
    
private:
    void addCubeFace(std::vector<VoxelVertex>& vertices,
                     std::vector<uint32_t>& indices,
                     simd::float3 pos,
                     BlockType type,
                     int face);
};

class VoxelWorld
{
public:
    VoxelWorld(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary);
    ~VoxelWorld();
    
    Chunk* getChunk(int chunkX, int chunkZ);
    
    BlockType getBlock(int worldX, int worldY, int worldZ);
    void setBlock(int worldX, int worldY, int worldZ, BlockType type);
    void removeBlock(int worldX, int worldY, int worldZ);
    
    bool raycast(simd::float3 origin, simd::float3 direction,
                 float maxDistance,
                 simd::int3& hitBlock,
                 simd::int3& adjacentBlock);

    void createPipeline(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat);
    
    void update(float dt, simd::float3 cameraPos);
    void render(MTL::RenderCommandEncoder* encoder,
                simd::float4x4 viewProjectionMatrix);
    
    // Génération Voronoi 4D
    void generateTerrainVoronoi(int chunkX, int chunkZ);
    
    // Anime la 4ème dimension
    void updateTime(float dt)
    {
        currentTime += dt * 0.1f; // Vitesse d'évolution
        voronoiGen.setTimeOffset(currentTime);
    }
    
    MTL::Device*                _pDevice;
    MTL::DepthStencilState*     _pDepthStencilState;
    MTL::RenderPipelineState*   _pPipelineState;

    void setBiomeGenerator(std::unique_ptr<BiomeGenerator> gen) { biomeGen = std::move(gen); }

    BlockType getBlockAtPositionBiomed(int worldX, int worldY, int worldZ, float time);
    
private:
    std::unordered_map<uint64_t, Chunk*> chunks;
    VoronoiVoxel4D voronoiGen;
    float currentTime;
    
    const int RENDER_DISTANCE = 2; // default 12
    
    uint64_t chunkKey(int x, int z) const {
        return ((uint64_t)(x) << 32) | ((uint64_t)(z) & 0xFFFFFFFF);
    }
    
    void worldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ, int& localX, int& localZ);

    std::unique_ptr<BiomeGenerator> biomeGen;
};

#endif /* VoronoiVoxel4D_hpp */
