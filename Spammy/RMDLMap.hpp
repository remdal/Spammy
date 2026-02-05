//
//  RMDLMap.hpp
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#ifndef RMDLMap_hpp
#define RMDLMap_hpp

#include "Metal/Metal.hpp"

#include "simd/simd.h"
#include <unordered_map>
#include <functional>
#include <vector>

#include <cstdint>

struct TerrainConfigLisse
{
    uint32_t seed;
    float flatRadius;
    float maxHeight;
    float flatness;          // 0.0 = montagneux, 1.0 = très plat
    simd::float2 center;     // point de référence (0,0)
};

struct TerrainVertexLisse
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 uv;
    uint32_t biomeID;
//    simd::float3 biomeColor;
};

enum class BiomeTypeLisse : uint32_t {
    SafeZone = 0,
    Plains,
    Desert,
    Forest,
    Mountains,
    Volcanic,
    Frozen,
    Ocean,
    Canyon,
    Jungle,
    Hills,
    Mars,
    Moon
};

class TerrainGeneratorLisse {
public:
    TerrainGeneratorLisse(MTL::Device* device, const TerrainConfigLisse& config);
    ~TerrainGeneratorLisse();
    
    void generate(MTL::CommandBuffer* cmd, simd::float2 chunkOrigin, uint32_t chunkSize);
    
    MTL::Buffer* getVertexBuffer() const { return m_vertexBuffer; }
    MTL::Buffer* getIndexBuffer() const { return m_indexBuffer; }
    uint32_t getIndexCount() const { return m_indexCount; }
    
private:
    void buildComputePipeline();
    void buildNormalsPipeline();
    
    MTL::Device* m_device;
    MTL::ComputePipelineState* m_terrainPipeline;
    MTL::ComputePipelineState* m_normalsPipeline;
    MTL::Buffer* m_vertexBuffer;
    MTL::Buffer* m_indexBuffer;
    MTL::Buffer* m_configBuffer;
    
    TerrainConfigLisse m_config;
    uint32_t m_indexCount;
};

struct ChunkCoord {
    int32_t x;
    int32_t z;
    
    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
};

struct ChunkCoordHashLisse
{
    size_t operator()(const ChunkCoord& c) const {
        return std::hash<int64_t>{}((int64_t(c.x) << 32) | uint32_t(c.z));
    }
};

struct TerrainChunkLisse
{
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    uint32_t indexCount;
    simd::float3 worldOrigin;
    bool ready;
};

struct PhysicsSample
{
    float height;
    simd::float3 normal;
    uint biomeID;
    float friction;
};

class InfiniteTerrainManager {
public:
    InfiniteTerrainManager(MTL::Device* device, uint32_t seed);
    ~InfiniteTerrainManager();
    
    void update(simd::float3 playerPos, MTL::CommandBuffer* cmd);
    void render(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection, const simd::float3& cameraPosition);
    
    void setViewDistance(uint32_t chunks) { m_viewDistance = chunks; }
    void setChunkSize(uint32_t size) { m_chunkSize = size; }
    void setFlatRadius(float radius) { m_config.flatRadius = radius; }
    
    float getHeightAt(float x, float z, MTL::CommandBuffer* cmd);
    void requestHeightAt(float x, float z, MTL::CommandBuffer* cmd);
    float getLastHeight() const { return m_lastHeight; }
    
private:
    ChunkCoord worldToChunk(simd::float3 pos) const;
    void generateChunk(ChunkCoord coord, MTL::CommandBuffer* cmd);
    void unloadDistantChunks(ChunkCoord playerChunk);
    
    MTL::Device* m_device;
    TerrainConfigLisse m_config;
    
    std::unordered_map<ChunkCoord, TerrainChunkLisse, ChunkCoordHashLisse> m_chunks;
    
    MTL::RenderPipelineState* m_renderPipeline = nullptr;
    MTL::DepthStencilState* m_depthState = nullptr;
    void buildRenderPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat);
    
    MTL::ComputePipelineState* m_terrainPipeline;
    MTL::ComputePipelineState* m_normalsPipeline;
    MTL::Buffer* m_configBuffer;
    
    uint32_t m_chunkSize = 64;      // vertices par côté
    uint32_t m_viewDistance = 8;
    float m_chunkWorldSize = 64.0f; // taille monde par chunk
    float totalTime = 0;
    float dayTime = 0;
    
    MTL::ComputePipelineState* m_heightSamplePipeline = nullptr;
    MTL::Buffer* m_queryBuffer[2] = {nullptr, nullptr};
    MTL::Buffer* m_resultBuffer[2] = {nullptr, nullptr};
    uint32_t m_currentBuffer = 0;
    float m_lastHeight = 0.0f;  // Résultat du frame précédent
    bool m_heightReady = false;
};


//#include <unordered_map>
//#include <functional>
//
//struct ChunkCoord {
//    int32_t x;
//    int32_t z;
//    
//    bool operator==(const ChunkCoord& other) const {
//        return x == other.x && z == other.z;
//    }
//};
//
//struct ChunkCoordHash {
//    size_t operator()(const ChunkCoord& c) const {
//        return std::hash<int64_t>{}((int64_t(c.x) << 32) | uint32_t(c.z));
//    }
//};
//
//struct TerrainChunk {
//    MTL::Buffer* vertexBuffer;
//    MTL::Buffer* indexBuffer;
//    uint32_t indexCount;
//    simd::float3 worldOrigin;
//    bool ready;
//};
//
//class InfiniteTerrainManager {
//public:
//    InfiniteTerrainManager(MTL::Device* device, uint32_t seed);
//    ~InfiniteTerrainManager();
//    
//    void update(simd::float3 playerPos, MTL::CommandBuffer* cmd);
//    void render(MTL::RenderCommandEncoder* encoder);
//    
//    void setViewDistance(uint32_t chunks) { m_viewDistance = chunks; }
//    void setChunkSize(uint32_t size) { m_chunkSize = size; }
//    void setFlatRadius(float radius) { m_config.flatRadius = radius; }
//    
//private:
//    ChunkCoord worldToChunk(simd::float3 pos) const;
//    void generateChunk(ChunkCoord coord, MTL::CommandBuffer* cmd);
//    void unloadDistantChunks(ChunkCoord playerChunk);
//    
//    MTL::Device* m_device;
//    TerrainConfig m_config;
//    
//    std::unordered_map<ChunkCoord, TerrainChunk, ChunkCoordHash> m_chunks;
//    
//    MTL::ComputePipelineState* m_terrainPipeline;
//    MTL::ComputePipelineState* m_normalsPipeline;
//    MTL::Buffer* m_configBuffer;
//    
//    uint32_t m_chunkSize = 64;      // vertices par côté
//    uint32_t m_viewDistance = 8;    // chunks autour du joueur
//    float m_chunkWorldSize = 64.0f; // taille monde par chunk
//};

#endif /* RMDLMap_hpp */
