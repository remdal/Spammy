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
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <future>
#include <cstdint>
#include <array>

#include "RMDLUtils.hpp"
#include "RMDLMainRenderer_shared.h"

static constexpr int CHUNK_SIZE = 16;
static constexpr int CHUNK_HEIGHT = 128;
static constexpr float VOXELSIZE = 0.99f;
static constexpr int RENDER_DISTANCE = 12;
static constexpr int WORLD_SEED = 89;

enum class BlockType : uint8_t {
    AIR = 0,
    STONE,
    CRYSTAL_RED,
    CRYSTAL_BLUE,
    CRYSTAL_GREEN,
    ORGANIC,
    METAL,
    GLOWING,
    VOID_MATTER,
    Grass,
    Dirt,
    Stone,
    Sand,
    Water,
    VolcanicRock,
    Obsidian,
    MoonRock,
    Ice,
    Snow,
    Wood,
    Leaves,
    GrassDetailed,
    StoneDetailed,
    SandDetailed,
    COUNT
};

struct BlockMeta
{
    uint8_t orientation : 3;
    uint8_t state : 5;
};

const simd::float4 BLOCK_COLORS[] = {
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.4f, 0.4f, 0.45f, 1.0f},
    {0.9f, 0.2f, 0.2f, 1.0f},
    {0.2f, 0.4f, 0.9f, 1.0f},
    {0.2f, 0.9f, 0.4f, 1.0f},
    {0.6f, 0.5f, 0.3f, 1.0f},
    {0.7f, 0.7f, 0.8f, 1.0f},
    {1.0f, 0.9f, 0.5f, 1.0f},
    {0.1f, 0.05f, 0.15f, 1.0f}
};

struct TerrainGenerationTask
{
    int chunkX;
    int chunkZ;
    float currentTime;
    std::promise<bool> completion;
    
    TerrainGenerationTask(int x, int z, float time)
        : chunkX(x), chunkZ(z), currentTime(time) {}
};

struct GeneratedChunkData
{
    int chunkX;
    int chunkZ;
    std::vector<uint8_t> blockData;
    bool isReady;
};

struct VoxelVertex
{
    simd::float3 position;
    simd::float4 color;
    simd::float3 normal;
};

struct VoronoiSite4D
{
    simd::float4 position;
    BlockType blockType;
    float influence;
};

uint32_t hash(int x, int y, int z);

enum class BiomeTypes {
    PERLIN_VORONOI_MIXED,
    VOLCANO_ACTIVE,
    VORONOI_4D_VOID,
    SNOW_PARTICLES,
    CHAOS
};

struct BiomeRegion
{
    simd::float2 center;
    float radius;
    BiomeTypes type;
    float intensity; // 0-1 pour transition
};

struct SnowFlake
{
    simd::float3 position;
    simd::float3 velocity;
    float size;
    float rotation;
    float lifetime;
};

class EnhancedSnowSystem
{
public:
    void update(float dt, BiomeTypes currentBiome);
    void spawnFlakesInBiome(simd::float3 center, float radius);
    
private:
    std::vector<SnowFlake>  flakes;
    MTL::Buffer*            particleBuffer;
};

class BiomeGenerator
{
public:
    BiomeGenerator(uint32_t seed);

    BiomeTypes getBiomeAt(float worldX, float worldZ);
    float getBiomeBlend(float worldX, float worldZ, BiomeTypes type);

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

    void createPipeline(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Device* device);
    
    void update(float dt, simd::float3 cameraPos, MTL::Device* device);
    void render(MTL::RenderCommandEncoder* encoder,
                simd::float4x4 viewProjectionMatrix);
    
    void generateTerrainVoronoi(int chunkX, int chunkZ);
    
    void updateTime(float dt)
    {
        currentTime += dt * 0.1f;
        voronoiGen.setTimeOffset(currentTime);
    }

    MTL::DepthStencilState*     m_depthStencilState;
    MTL::RenderPipelineState*   m_renderPipelineState;

    void setBiomeGenerator(std::unique_ptr<BiomeGenerator> gen) { biomeGen = std::move(gen); }

    BlockType getBlockAtPositionBiomed(int worldX, int worldY, int worldZ, float time);
    
private:
    std::unordered_map<uint64_t, Chunk*> chunks;
    VoronoiVoxel4D voronoiGen;
    float currentTime;
    
    uint64_t chunkKey(int x, int z) const {
        return ((uint64_t)(x) << 32) | ((uint64_t)(z) & 0xFFFFFFFF);
    }
    
    void worldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ, int& localX, int& localZ);

    std::unique_ptr<BiomeGenerator> biomeGen;
};

#endif /* VoronoiVoxel4D_hpp */

//struct VoxelBlock {
//    BlockType type;
//    BlockMeta meta;
//    
//    VoxelBlock() : type(BlockType::Air), meta{0, 0} {}
//    explicit VoxelBlock(BlockType t) : type(t), meta{0, 0} {}
//    
//    bool isSolid() const { return type != BlockType::Air && type != BlockType::Water; }
//    bool isTransparent() const { return type == BlockType::Air || type == BlockType::Water; }
//};
//
//// Coordonnées chunk (monde divisé en chunks)
//struct ChunkCoord {
//    int32_t x, z;
//    
//    bool operator==(const ChunkCoord& other) const {
//        return x == other.x && z == other.z;
//    }
//    
//    struct Hash {
//        size_t operator()(const ChunkCoord& coord) const {
//            return std::hash<int64_t>()((int64_t(coord.x) << 32) | uint32_t(coord.z));
//        }
//    };
//};
//
//// Position bloc dans le monde
//struct BlockPos {
//    int32_t x, y, z;
//    
//    ChunkCoord toChunkCoord() const {
//        return {x >> 5, z >> 5};  // Division par 32
//    }
//    
//    simd::int3 toLocalPos() const {
//        return {x & 31, y, z & 31};  // Modulo 32
//    }
//};
//
//// Biomes disponibles
//enum class BiomeType : uint8_t {
//    Plains,         // Plaines centrales plates
//    Forest,
//    Desert,
//    Ocean,
//    Volcanic,       // Volcan avec lave
//    Moon,           // Surface lunaire
//    Surreal,        // Biome étrange
//    IceSheet,
//    COUNT
//};
//
//// Paramètres génération par biome
//struct BiomeParams {
//    float baseHeight;       // Hauteur de base (0-1)
//    float heightVariation;  // Variation terrain
//    float temperature;      // -1 à 1
//    float humidity;         // 0 à 1
//    float roughness;        // Pour normal maps
//    BlockType surfaceBlock;
//    BlockType underBlock;
//    
//    static BiomeParams forType(BiomeType type);
//};
//
//// Données chunk en mémoire (optimisé pour cache)
//struct ChunkData {
//    ChunkCoord coord;
//    std::array<VoxelBlock, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> blocks;
//    BiomeType dominantBiome;
//    bool isDirty = true;        // Nécessite re-mesh
//    bool isGenerated = false;   // Terrain généré
//    uint32_t lastAccess = 0;    // Pour LRU cache
//    
//    VoxelBlock& getBlock(int x, int y, int z) {
//        return blocks[x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE];
//    }
//    
//    const VoxelBlock& getBlock(int x, int y, int z) const {
//        return blocks[x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE];
//    }
//};
//
//// Vertex pour rendu (PBR complet)
//struct VoxelVertex {
//    simd::float3 position;
//    simd::float3 normal;
//    simd::float2 texCoord;
//    simd::float3 tangent;
//    simd::float3 bitangent;
//    uint32_t blockType;  // Pour texture array
//    float ao;            // Ambient occlusion pré-calculé
//};
//
//// Uniforms pour shaders
//struct VoxelUniforms {
//    simd::float4x4 modelViewProjection;
//    simd::float4x4 modelMatrix;
//    simd::float4x4 normalMatrix;
//    simd::float3 cameraPosition;
//    float time;
//    simd::float3 sunDirection;
//    float _pad0;
//    simd::float3 sunColor;
//    float sunIntensity;
//};
//
//// Propriétés matériau PBR par type de bloc
//struct BlockMaterial {
//    simd::float3 albedo;
//    float metallic;
//    float roughness;
//    float ao;
//    simd::float2 texScale;  // Échelle texture
//    uint32_t hasNormalMap : 1;
//    uint32_t hasRoughnessMap : 1;
//    uint32_t hasAOMap : 1;
//    uint32_t reserved : 29;
//    
//    static BlockMaterial forBlockType(BlockType type);
//};
//
//inline BiomeParams BiomeParams::forType(BiomeType type) {
//    switch(type) {
//        case BiomeType::Plains:
//            return {0.4f, 0.05f, 0.6f, 0.5f, 0.3f, BlockType::Grass, BlockType::Dirt};
//        case BiomeType::Forest:
//            return {0.45f, 0.15f, 0.5f, 0.7f, 0.4f, BlockType::Grass, BlockType::Dirt};
//        case BiomeType::Desert:
//            return {0.35f, 0.2f, 0.9f, 0.1f, 0.2f, BlockType::Sand, BlockType::Sand};
//        case BiomeType::Ocean:
//            return {0.15f, 0.1f, 0.5f, 1.0f, 0.1f, BlockType::Sand, BlockType::Stone};
//        case BiomeType::Volcanic:
//            return {0.5f, 0.3f, 1.0f, 0.0f, 0.8f, BlockType::VolcanicRock, BlockType::Obsidian};
//        case BiomeType::Moon:
//            return {0.4f, 0.25f, -1.0f, 0.0f, 0.6f, BlockType::MoonRock, BlockType::MoonRock};
//        case BiomeType::Surreal:
//            return {0.6f, 0.4f, 0.0f, 0.5f, 0.9f, BlockType::Stone, BlockType::Dirt};
//        case BiomeType::IceSheet:
//            return {0.42f, 0.08f, -0.8f, 0.9f, 0.2f, BlockType::Ice, BlockType::Snow};
//        default:
//            return {0.5f, 0.1f, 0.5f, 0.5f, 0.5f, BlockType::Grass, BlockType::Dirt};
//    }
//}
//
//inline BlockMaterial BlockMaterial::forBlockType(BlockType type) {
//    switch(type) {
//        case BlockType::GrassDetailed:
//            return {{0.3f, 0.6f, 0.2f}, 0.0f, 0.8f, 1.0f, {1.0f, 1.0f}, 1, 1, 1, 0};
//        case BlockType::StoneDetailed:
//            return {{0.5f, 0.5f, 0.5f}, 0.0f, 0.9f, 1.0f, {1.0f, 1.0f}, 1, 1, 1, 0};
//        case BlockType::Sand:
//            return {{0.9f, 0.8f, 0.6f}, 0.0f, 0.7f, 1.0f, {2.0f, 2.0f}, 1, 1, 0, 0};
//        case BlockType::Water:
//            return {{0.1f, 0.3f, 0.6f}, 0.0f, 0.1f, 1.0f, {1.0f, 1.0f}, 0, 0, 0, 0};
//        default:
//            return {{0.8f, 0.8f, 0.8f}, 0.0f, 0.5f, 1.0f, {1.0f, 1.0f}, 0, 0, 0, 0};
//    }
//}
//
//#endif
