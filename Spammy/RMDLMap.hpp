//
//  RMDLMap.hpp
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#ifndef RMDLMap_hpp
#define RMDLMap_hpp

//#include "TerrainGenerator.h"
#include "Metal/Metal.hpp"
#include "simd/simd.h"



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
