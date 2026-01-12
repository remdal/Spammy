//
//  RMDLSystem.hpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#ifndef RMDLSystem_hpp
#define RMDLSystem_hpp

#include <Metal/Metal.hpp>

#include "RMDLMathUtils.hpp"

#include <simd/simd.h>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>

// Forward declarations
class BiomeGen;

// Terrain chunk for streaming
struct TerrainChunk {
    static constexpr int CHUNK_SIZE = 64;
    static constexpr int VERTEX_RESOLUTION = 128; // Vertices per chunk side
    
    simd::int2 chunkCoord;
    float lodLevel;
    bool needsUpdate;
    bool visible;
    
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    size_t indexCount;
    
    // Bounding box for culling
    simd::float3 boundsMin;
    simd::float3 boundsMax;
    
    TerrainChunk() : chunkCoord{0,0}, lodLevel(0), needsUpdate(true),
                     visible(true), vertexBuffer(nullptr), indexBuffer(nullptr),
                     indexCount(0) {}
};

// Vertex structure for terrain
struct TerrainVertex {
    simd::float3 position;
    simd::float3 normal;
    simd::float3 tangent;
    simd::float2 texCoord;
    simd::float4 color;      // Biome color/blending
    float height;
    float biomeId;           // Which biome this vertex belongs to
};

// Uniforms for terrain rendering
struct TerrainUniforms {
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjectionMatrix;
    simd::float3 cameraPosition;
    float time;
    simd::float4 fogColor;
    float fogDensity;
    float maxRenderDistance;
    uint32_t seed;
    float heightScale;
};

// Biome types
enum class BiomeType : uint32_t {
    SpawnCrater = 0,        // Starting area - relatively flat crater
    VoronoiPlains = 1,      // Standard Voronoi cells
    VoronoiIslands = 2,     // Voronoi with water gaps
    VoronoiTerrace = 3,     // Stepped Voronoi
    VoronoiEroded = 4,      // Eroded Voronoi with valleys
    Planet = 5,              // Spherical planet-like terrain
    Moon = 6,                // Crater-heavy lunar terrain
    Ocean = 7,               // Deep water biome
    CrystalCaves = 8,       // Underground crystal formations
    FloatingIslands = 9,    // Detached floating landmasses
    LavaFields = 10,        // Volcanic terrain
    IceShelf = 11,          // Frozen tundra
    COUNT
};

// Biome parameters
struct BiomeParams {
    BiomeType type;
    simd::float2 center;     // Voronoi center point
    float radius;
    float heightOffset;
    float roughness;
    simd::float4 color;
    float transitionWidth;
};

class TerrainSystem {
public:
    TerrainSystem(MTL::Device* device,
                  MTL::PixelFormat colorFormat,
                  MTL::PixelFormat depthFormat,
                  MTL::Library* library);
    ~TerrainSystem();
    
    // Initialization
    void initialize(uint32_t seed);
    void setSeed(uint32_t seed) { m_seed = seed; }
    
    // Update and rendering
    void update(float deltaTime, const simd::float3& cameraPosition);
    void render(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection);
    
    // Chunk management
    void streamChunks(const simd::float3& cameraPosition);
    void unloadDistantChunks(const simd::float3& cameraPosition);
    
    // Height queries
    float getHeightAt(float x, float z) const;
    simd::float3 getNormalAt(float x, float z) const;
    BiomeType getBiomeAt(float x, float z) const;
    
    // Collision
    bool checkTerrainCollision(const simd::float3& position, float radius) const;
    simd::float3 resolveTerrainCollision(const simd::float3& position,
                                         const simd::float3& velocity,
                                         float radius) const;
    
    // Settings
    void setHeightScale(float scale) { m_heightScale = scale; }
    void setRenderDistance(float distance) { m_maxRenderDistance = distance; }
    void setLODEnabled(bool enabled) { m_lodEnabled = enabled; }
    
    // Debug
    void setWireframe(bool enabled) { m_wireframe = enabled; }
    size_t getChunkCount() const { return m_chunks.size(); }
    
private:
    MTL::Device* m_device;
    MTL::Library* m_library;
    
    // Rendering
    MTL::RenderPipelineState* m_terrainPipeline;
    MTL::DepthStencilState* m_depthStencilState;
    MTL::Buffer* m_uniformsBuffer;
    TerrainUniforms m_uniforms;
    
    // Compute shaders for GPU terrain generation
    MTL::ComputePipelineState* m_voronoiGeneratorPipeline;
    MTL::ComputePipelineState* m_heightmapGeneratorPipeline;
    MTL::ComputePipelineState* m_normalCalculatorPipeline;
    MTL::ComputePipelineState* m_erosionPipeline;
    
    // Textures
    MTL::Texture* m_heightmapTexture;       // GPU-generated heightmap
    MTL::Texture* m_normalTexture;           // Computed normals
    MTL::Texture* m_biomeTexture;            // Biome ID map
    MTL::Texture* m_voronoiTexture;          // Voronoi diagram
    
    // Material textures (arrays for different biomes)
    MTL::Texture* m_albedoArray;
    MTL::Texture* m_normalMapArray;
    MTL::Texture* m_roughnessArray;
    MTL::Texture* m_metallicArray;
    MTL::Texture* m_aoArray;
    
    // Chunks
    std::vector<std::unique_ptr<TerrainChunk>> m_chunks;
    
    // Biome system
    std::vector<BiomeParams> m_biomes;
    std::unique_ptr<BiomeGen> m_biomeGenerator;
    
    // Parameters
    uint32_t m_seed;
    float m_heightScale;
    float m_maxRenderDistance;
    bool m_lodEnabled;
    bool m_wireframe;
    float m_time;
    
    // Noise parameters
    int m_octaves;
    float m_persistence;
    float m_lacunarity;
    float m_noiseScale;
    
    // Helpers
    void createPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat);
    void createTextures();
    void loadMaterialTextures(const std::string& resourcePath);
    void generateBiomeMap();
    void generateHeightmapGPU();
    void generateNormalsGPU();
    void applyErosionGPU(int iterations);
    
    TerrainChunk* getOrCreateChunk(int chunkX, int chunkZ);
    void generateChunkMesh(TerrainChunk* chunk);
    void generateChunkMeshGPU(TerrainChunk* chunk);
    
    float sampleHeightmap(float x, float z) const;
    simd::float2 worldToUV(float x, float z) const;
    
    // Biome generation
    void initializeSpawnBiome();
    void generateVoronoiBiomes();
};

// Separate biome generator for complex Voronoi calculations
class BiomeGen {
public:
    BiomeGen(uint32_t seed);
    
    struct VoronoiCell {
        simd::float2 center;
        BiomeType biomeType;
        float radius;
        simd::float4 color;
    };
    
    void generate(float worldSize, int numCells);
    const std::vector<VoronoiCell>& getCells() const { return m_cells; }
    
    BiomeType getBiomeAt(const simd::float2& position) const;
    simd::float2 getNearestCellCenter(const simd::float2& position) const;
    float getDistanceToNearestEdge(const simd::float2& position) const;
    
    // GPU compute shader helpers
    MTL::Buffer* getCellBuffer() const { return m_cellBuffer; }
    void uploadToGPU(MTL::Device* device);
    
private:
    std::mt19937 m_rng;
    std::vector<VoronoiCell> m_cells;
    MTL::Buffer* m_cellBuffer;

    BiomeType determineBiomeType(float distanceFromSpawn, float randomValue);
};

#endif /* RMDLSystem_hpp */
