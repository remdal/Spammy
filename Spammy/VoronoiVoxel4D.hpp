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

static constexpr int CHUNK_SIZE = 32;
static constexpr int CHUNK_HEIGHT = 128;
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

struct VoxelVertex {
    simd::float3 position;
    simd::float4 color;
    simd::float3 normal;
};

// Point Voronoi dans l'espace 4D
struct VoronoiSite4D {
    simd::float4 position;
    BlockType blockType;
    float influence;
};

class VoronoiVoxel4D
{
public:
    VoronoiVoxel4D(uint32_t seed = 89);
    ~VoronoiVoxel4D();
    
    // Génère sites pour une région 3D à un temps donné
    void generateSitesForRegion(int chunkX, int chunkZ, float time);
    
    // Trouve le site Voronoi le plus proche en 4D
    VoronoiSite4D* findClosestSite(simd::float3 worldPos, float time);
    
    // Distance 4D entre deux points
    float distance4D(simd::float4 a, simd::float4 b);
    
    // Génère le bloc à cette position en utilisant Voronoi
    BlockType getBlockAtPosition(int worldX, int worldY, int worldZ, float time);
    
    // Worley noise (distance au plus proche site)
    float worleyNoise(simd::float3 pos, float time);
    
    void setTimeOffset(float t) { timeOffset = t; }
    float getTimeOffset() const { return timeOffset; }
private:
    std::vector<VoronoiSite4D> sites;
    std::mt19937 rng;
    float timeOffset; // 4ème dimension = temps
    
    // Hash pour positions cohérentes
    uint32_t hash(int x, int y, int z);
};

class Chunk
{
public:
    int chunkX, chunkZ;
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    uint32_t indexCount;
    bool needsRebuild;
    
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

class VoxelWorld {
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
    void updateTime(float dt) {
        currentTime += dt * 0.1f; // Vitesse d'évolution
        voronoiGen.setTimeOffset(currentTime);
    }
    
    MTL::Device*                _pDevice;
    MTL::DepthStencilState*     _pDepthStencilState;
    MTL::RenderPipelineState*   _pPipelineState;
    
private:
    std::unordered_map<uint64_t, Chunk*> chunks;
    VoronoiVoxel4D voronoiGen;
    float currentTime;
    
    const int RENDER_DISTANCE = 3; // Plus de chunks avec M2 Pro (default 12)
    
    uint64_t chunkKey(int x, int z) const {
        return ((uint64_t)(x) << 32) | ((uint64_t)(z) & 0xFFFFFFFF);
    }
    
    void worldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ, int& localX, int& localZ);
};

//class VoxelCamera {
//public:
//    simd::float3 position;
//    float yaw;
//    float pitch;
//    
//    VoxelCamera();
//    
//    simd::float3 forward() const;
//    simd::float3 right() const;
//    simd::float3 up() const;
//    
//    void move(simd::float3 direction, float speed, float dt);
//    void rotate(float deltaYaw, float deltaPitch);
//    
//    simd::float4x4 viewMatrix() const;
//    simd::float4x4 projectionMatrix(float aspect, float fov, float near, float far) const;
//};

#endif /* VoronoiVoxel4D_hpp */
