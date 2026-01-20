//
//  RMDLSystem.hpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#ifndef RMDLSystem_hpp
#define RMDLSystem_hpp

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "RMDLMathUtils.hpp"
#include "Utils/NoiseGen.hpp"

#include <dispatch/dispatch.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <simd/simd.h>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <array>
#include <string>
#include <atomic>
#include <unordered_map>
#include <unordered_set>

namespace GameOver {

struct GameStates
{
    uint32_t Score = 0;
    
    
};
}

namespace OfficialConfig {

constexpr uint32_t CHUNK_SIZE = 8;
constexpr uint32_t CHUNK_HEIGHT = 256;
constexpr float TERRAIN_SCALE = 1.0f;
constexpr uint32_t VIEW_DISTANCE_CHUNKS = 12;
constexpr uint32_t MAX_LOADED_CHUNKS = 512;

constexpr uint32_t LOD_LEVELS = 5;
constexpr float LOD_DISTANCES[LOD_LEVELS] = { 64.0f, 128.0f, 256.0f, 512.0f, 1024.0f };

constexpr float GRAVITY = -9.81f;
constexpr float PHYSICS_TIMESTEP = 1.0f / 60.0f;
constexpr uint32_t MAX_PHYSICS_SUBSTEPS = 4;

constexpr uint32_t SHADOW_MAP_SIZE = 4096;
constexpr uint32_t MAX_POINT_LIGHTS = 64;
constexpr float HDR_EXPOSURE = 1.2f;

constexpr uint32_t BIOME_COUNT = 8;
constexpr float BIOME_BLEND_DISTANCE = 32.0f;

constexpr float SPAWN_CRATER_RADIUS = 128.0f;
constexpr float SPAWN_CRATER_DEPTH = 8.0f;
constexpr float SPAWN_SAFE_RADIUS = 256.0f;
}

namespace Types {

    struct AABB
    {
        simd::float3 min;
        simd::float3 max;
        
        bool intersects(const AABB& other) const
        {
            return (min.x <= other.max.x && max.x >= other.min.x) &&
                   (min.y <= other.max.y && max.y >= other.min.y) &&
                   (min.z <= other.max.z && max.z >= other.min.z);
        }
        
        bool contains(simd::float3 point) const
        {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y &&
                   point.z >= min.z && point.z <= max.z;
        }
    };
    
    struct Ray
    {
        simd::float3 origin;
        simd::float3 direction;
    };
    
    struct RayHit
    {
        bool hit;
        float distance;
        simd::float3 position;
        simd::float3 normal;
        uint32_t materialId;
    };
    
    struct ChunkCoord
    {
        int32_t x;
        int32_t z;
        
        bool operator==(const ChunkCoord& other) const
        {
            return x == other.x && z == other.z;
        }
        
        uint64_t hash() const
        {
            uint64_t hx = static_cast<uint64_t>(x + 0x7FFFFFFF);
            uint64_t hz = static_cast<uint64_t>(z + 0x7FFFFFFF);
            return (hx << 32) | hz;
        }
    };
}

// GPU Structs - alignés pour Metal - à mettre dans shared main
namespace GPU {
    struct alignas(16) Vertex {
        simd::float3 position;
        float _pad0;
        simd::float3 normal;
        float _pad1;
        simd::float4 tangent;
        simd::float2 uv;
        simd::float2 uv2;
    };
    
    struct alignas(16) CameraData {
        simd::float4x4 viewMatrix;
        simd::float4x4 projectionMatrix;
        simd::float4x4 viewProjectionMatrix;
        simd::float4x4 invViewProjectionMatrix;
        simd::float3 position;
        float nearPlane;
        simd::float3 forward;
        float farPlane;
        simd::float2 screenSize;
        float time;
        float deltaTime;
    };
    
    struct alignas(16) LightData {
        simd::float3 direction;
        float intensity;
        simd::float3 color;
        float shadowBias;
        simd::float4x4 lightSpaceMatrix;
    };
    
    struct alignas(16) MaterialData {
        simd::float3 albedo;
        float metallic;
        float roughness;
        float ao;
        float normalStrength;
        float heightScale;
        uint32_t flags;
        uint32_t albedoTexture;
        uint32_t normalTexture;
        uint32_t roughnessMetallicTexture;
    };
    
    struct alignas(16) TerrainPushConstants {
        simd::float2 chunkWorldPos;
        float lodScale;
        uint32_t biomeId;
        uint32_t lodLevel;
        float morphFactor;
        float _pad0;
        float _pad1;
    };
    
    struct alignas(16) BiomeData {
        simd::float3 groundColor;
        float heightScale;
        simd::float3 cliffColor;
        float cliffThreshold;
        float noiseFrequency;
        float noiseAmplitude;
        float vegetationDensity;
        float rockDensity;
    };
}

enum class BiomeType : uint32_t {
    Spawn = 0,          // Cratère de spawn plat
    Plains,             // Plaines douces
    Hills,              // Collines
    Mountains,          // Montagnes escarpées
    Desert,             // Désert avec dunes
    Tundra,             // Toundra glacée
    Forest,             // Forêt dense
    Volcanic,           // Terrain volcanique
    Planet,
    Moon,
    Ocean
};

struct BiomeDefinition
{
    BiomeType type;
    std::string name;
    
    // Terrain generation
    float baseHeight;
    float heightVariation;
    float noiseFrequency;
    int octaves;
    float persistence;
    float lacunarity;
    
    // Appearance
    simd::float3 groundColor;
    simd::float3 cliffColor;
    float cliffAngleThreshold;
    float roughnessBase;
    float metallicBase;
    
    // Features
    float vegetationDensity;
    float rockDensity;
    float waterLevel;
    
    // Blending
    float transitionWidth;
};

class BiomeManager
{
public:
    BiomeManager(MTL::Device* device);
    ~BiomeManager();
    
    BiomeType getBiomeAt(float x, float z, uint64_t seed) const;
    float getBlendFactor(float x, float z, BiomeType biome, uint64_t seed) const;
    const BiomeDefinition& getDefinition(BiomeType type) const;
    
    void updateGPUBuffer();
    MTL::Buffer* getBiomeBuffer() const { return _biomeBuffer; }
    
    // Spawn area
    bool isInSpawnArea(float x, float z) const;
    float getSpawnModifier(float x, float z) const;

private:
    void initializeBiomes();
    float voronoiBiome(float x, float z, uint64_t seed) const;
    
    MTL::Device* _device;
    MTL::Buffer* _biomeBuffer;
    std::array<BiomeDefinition, OfficialConfig::BIOME_COUNT> _biomes;
    
    simd::float2 _spawnCenter;
};

enum class ChunkState : uint32_t {
    Unloaded = 0,
    Generating,
    MeshBuilding,
    Ready,
    Unloading
};

struct ChunkMeshData {
    std::vector<GPU::Vertex> vertices;
    std::vector<uint32_t> indices;
    Types::AABB bounds;
};

struct ChunkCollisionData {
    std::vector<float> heightfield;
    uint32_t resolution;
    float minHeight;
    float maxHeight;
};

class ChunkMap
{
public:
    ChunkMap(MTL::Device* device, Types::ChunkCoord coord);
    ~ChunkMap();
    
    // Non-copyable
    ChunkMap(const ChunkMap&) = delete;
    ChunkMap& operator=(const ChunkMap&) = delete;
    
    // State
    ChunkState getState() const { return _state.load(); }
    void setState(ChunkState state) { _state.store(state); }
    
    Types::ChunkCoord getCoord() const { return _coord; }
    simd::float2 getWorldPosition() const;
    const Types::AABB& getBounds() const { return _bounds; }
    
    // Mesh
    void uploadMesh(const ChunkMeshData& data);
    MTL::Buffer* getVertexBuffer() const { return _vertexBuffer; }
    MTL::Buffer* getIndexBuffer() const { return _indexBuffer; }
    uint32_t getIndexCount() const { return _indexCount; }
    
    // LOD
    void setLODLevel(uint32_t lod) { _currentLOD = lod; }
    uint32_t getLODLevel() const { return _currentLOD; }
    
    // Textures
    MTL::Texture* getHeightmap() const { return _heightmap; }
    MTL::Texture* getNormalMap() const { return _normalMap; }
    MTL::Texture* getSplatMap() const { return _splatMap; }
    
    void setHeightmap(MTL::Texture* tex);
    void setNormalMap(MTL::Texture* tex);
    void setSplatMap(MTL::Texture* tex);
    
    // Collision
    void setCollisionData(ChunkCollisionData&& data);
    float getHeightAt(float localX, float localZ) const;
    simd::float3 getNormalAt(float localX, float localZ) const;
    
    // Biome
    void setBiomeId(uint32_t id) { _biomeId = id; }
    uint32_t getBiomeId() const { return _biomeId; }
    
    // Distance pour priorité
    void updateDistance(simd::float3 cameraPos);
    float getDistanceToCamera() const { return _distanceToCamera; }

private:
    MTL::Device* _device;
    Types::ChunkCoord _coord;
    std::atomic<ChunkState> _state;
    
    // Mesh data
    MTL::Buffer* _vertexBuffer;
    MTL::Buffer* _indexBuffer;
    uint32_t _indexCount;
    Types::AABB _bounds;
    
    // LOD
    uint32_t _currentLOD;
    std::array<MTL::Buffer*, OfficialConfig::LOD_LEVELS> _lodIndexBuffers;
    std::array<uint32_t, OfficialConfig::LOD_LEVELS> _lodIndexCounts;
    
    // Textures
    MTL::Texture* _heightmap;
    MTL::Texture* _normalMap;
    MTL::Texture* _splatMap;
    
    // Collision
    ChunkCollisionData _collisionData;
    
    // Biome
    uint32_t _biomeId;
    
    // Camera distance
    float _distanceToCamera;
};

struct TerrainGenerationRequest {
    Types::ChunkCoord coord;
    uint32_t priority;
    std::function<void(std::shared_ptr<ChunkMap>)> callback;
    
    bool operator<(const TerrainGenerationRequest& other) const {
        return priority > other.priority; // Min-heap
    }
};

class TerrainGenerator {
public:
    TerrainGenerator(MTL::Device* device, MTL::CommandQueue* queue,
                    NoiseGenerator* noise, BiomeManager* biomes);
    ~TerrainGenerator();
    
    void requestChunk(Types::ChunkCoord coord, uint32_t priority,
                     std::function<void(std::shared_ptr<ChunkMap>)> callback);
    
    void cancelRequest(Types::ChunkCoord coord);
    void processRequests();
    
    void setLODLevel(uint32_t lod) { _currentLOD = lod; }
    
    // Statistiques
    uint32_t getPendingCount() const { return _pendingCount.load(); }
    uint32_t getGeneratedCount() const { return _generatedCount.load(); }

private:
    void workerThread();
    void generateChunk(const TerrainGenerationRequest& request);
    
    ChunkMeshData buildMeshCPU(Types::ChunkCoord coord, uint32_t lod,
                               const std::vector<float>& heights);
    
    void generateHeightsGPU(MTL::CommandBuffer* cmd, Types::ChunkCoord coord,
                           MTL::Buffer* outputBuffer, uint32_t resolution);
    
    MTL::Device* _device;
    MTL::CommandQueue* _commandQueue;
    NoiseGenerator* _noiseGenerator;
    BiomeManager* _biomeManager;
    
    // Threading
    std::vector<std::thread> _workers;
    std::priority_queue<TerrainGenerationRequest> _requestQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;
    std::atomic<bool> _running;
    
    // Pipelines
    MTL::ComputePipelineState* _heightGenPipeline;
    MTL::ComputePipelineState* _normalGenPipeline;
    MTL::ComputePipelineState* _meshGenPipeline;
    
    // État
    uint32_t _currentLOD;
    std::atomic<uint32_t> _pendingCount;
    std::atomic<uint32_t> _generatedCount;
};

struct ChunkCoordHash {
    size_t operator()(const Types::ChunkCoord& coord) const {
        return coord.hash();
    }
};

class TerrainManager {
public:
    TerrainManager(MTL::Device* device, MTL::CommandQueue* queue,
                  NoiseGenerator* noise, BiomeManager* biomes);
    ~TerrainManager();
    
    // Update
    void update(simd::float3 cameraPosition, float deltaTime);
    
    // Rendering
    void render(MTL::RenderCommandEncoder* encoder, const GPU::CameraData& camera);
    
    // Collision
    float getHeightAt(float x, float z) const;
    simd::float3 getNormalAt(float x, float z) const;
    bool raycast(const Types::Ray& ray, Types::RayHit& hit, float maxDistance) const;
    
    // Chunks
    ChunkMap* getChunkAt(float x, float z) const;
    ChunkMap* getChunk(Types::ChunkCoord coord) const;
    
    // Stats
    uint32_t getLoadedChunkCount() const { return static_cast<uint32_t>(_chunks.size()); }
    uint32_t getVisibleChunkCount() const { return _visibleChunkCount; }

private:
    void updateChunkLoading(simd::float3 cameraPosition);
    void updateChunkLODs(simd::float3 cameraPosition);
    void unloadDistantChunks(simd::float3 cameraPosition);
    
    uint32_t calculateLOD(float distance) const;
    uint32_t calculatePriority(Types::ChunkCoord coord, simd::float3 cameraPos) const;
    
    void onChunkGenerated(std::shared_ptr<ChunkMap> chunk);
    
    MTL::Device* _device;
    TerrainGenerator* _generator;
    
    std::unordered_map<Types::ChunkCoord, std::shared_ptr<ChunkMap>, ChunkCoordHash> _chunks;
    std::unordered_set<uint64_t> _pendingChunks;
    mutable std::mutex _chunksMutex;
    
    simd::float3 _lastCameraPosition;
    uint32_t _visibleChunkCount;
    
    // Render state
    MTL::RenderPipelineState* _terrainPipeline;
    MTL::DepthStencilState* _depthState;
};

struct PhysicsBody {
    uint32_t id;
    bool active;
    bool isStatic;
    bool useGravity;
    
    // Transform
    simd::float3 position;
    simd::quatf rotation;
    simd::float3 scale;
    
    // Velocity
    simd::float3 linearVelocity;
    simd::float3 angularVelocity;
    
    // Properties
    float mass;
    float invMass;
    simd::float3 inertia;
    simd::float3 invInertia;
    float friction;
    float restitution;
    float linearDamping;
    float angularDamping;
    
    // Collision
    Types::AABB bounds;
    float radius;
    bool onGround;
    simd::float3 groundNormal;
    
    // Forces
    simd::float3 accumulatedForce;
    simd::float3 accumulatedTorque;
};

struct CollisionContact {
    uint32_t bodyA;
    uint32_t bodyB;
    simd::float3 point;
    simd::float3 normal;
    float penetration;
};

class PhysicsSystem {
public:
    PhysicsSystem(TerrainManager* terrain);
    ~PhysicsSystem();
    
    // Simulation
    void update(float deltaTime);
    void fixedUpdate();
    
    // Bodies
    uint32_t createBody();
    void destroyBody(uint32_t id);
    PhysicsBody* getBody(uint32_t id);
    
    // Forces
    void applyForce(uint32_t id, simd::float3 force);
    void applyForceAtPoint(uint32_t id, simd::float3 force, simd::float3 point);
    void applyImpulse(uint32_t id, simd::float3 impulse);
    void applyTorque(uint32_t id, simd::float3 torque);
    
    // Queries
    bool raycast(const Types::Ray& ray, Types::RayHit& hit, float maxDistance, uint32_t mask = ~0u) const;
    void overlapSphere(simd::float3 center, float radius, std::vector<uint32_t>& results) const;
    
    // Configuration
    void setGravity(simd::float3 gravity) { _gravity = gravity; }
    simd::float3 getGravity() const { return _gravity; }

private:
    void integrateVelocities(float dt);
    void integratePositions(float dt);
    void detectCollisions();
    void resolveCollisions();
    void collideWithTerrain(PhysicsBody& body);
    
    TerrainManager* _terrain;
    
    std::vector<PhysicsBody> _bodies;
    std::vector<uint32_t> _freeIds;
    std::vector<CollisionContact> _contacts;
    
    simd::float3 _gravity;
    float _timeAccumulator;
    uint32_t _nextId;
    
    mutable std::mutex _mutex;
};

class RenderSystem {
public:
    RenderSystem(MTL::Device* device, MTL::PixelFormat colorFormat,
                MTL::PixelFormat depthFormat, NS::UInteger width, NS::UInteger height,
                const std::string& shaderPath);
    ~RenderSystem();
    
    void resize(NS::UInteger width, NS::UInteger height);
    
    void beginFrame();
    void render(MTL::CommandBuffer* cmd, CA::MetalDrawable* drawable,
               TerrainManager* terrain, BiomeManager* biomes,
               const GPU::CameraData& camera, const GPU::LightData& light);
    void endFrame();
    
    // Configuration
    void setExposure(float exposure) { _postParams.exposure = exposure; }
    void setBloomIntensity(float intensity) { _postParams.bloomIntensity = intensity; }
    void setVignetteIntensity(float intensity) { _postParams.vignetteIntensity = intensity; }
    
    MTL::CommandQueue* getCommandQueue() const { return _commandQueue; }

private:
    void createRenderTargets();
    void createPipelines(const std::string& shaderPath);
    void createSamplers();
    
    void shadowPass(MTL::CommandBuffer* cmd, TerrainManager* terrain,
                   const GPU::LightData& light);
    void geometryPass(MTL::CommandBuffer* cmd, TerrainManager* terrain,
                     BiomeManager* biomes, const GPU::CameraData& camera,
                     const GPU::LightData& light);
    void ssaoPass(MTL::CommandBuffer* cmd, const GPU::CameraData& camera);
    void bloomPass(MTL::CommandBuffer* cmd);
    void compositePass(MTL::CommandBuffer* cmd, MTL::Texture* target);
    
    MTL::Device* _device;
    MTL::CommandQueue* _commandQueue;
    MTL::Library* _shaderLibrary;
    
    NS::UInteger _width;
    NS::UInteger _height;
    MTL::PixelFormat _colorFormat;
    MTL::PixelFormat _depthFormat;
    
    // Render targets
    MTL::Texture* _colorTarget;
    MTL::Texture* _depthTarget;
    MTL::Texture* _normalTarget;
    MTL::Texture* _shadowMap;
    MTL::Texture* _aoTarget;
    MTL::Texture* _bloomTarget;
    MTL::Texture* _bloomBlurTargets[2];
    
    // Pipelines
    MTL::RenderPipelineState* _terrainPipeline;
    MTL::RenderPipelineState* _shadowPipeline;
    MTL::RenderPipelineState* _compositePipeline;
    MTL::RenderPipelineState* _fxaaPipeline;
    MTL::ComputePipelineState* _ssaoPipeline;
    MTL::ComputePipelineState* _bloomThresholdPipeline;
    MTL::ComputePipelineState* _bloomBlurHPipeline;
    MTL::ComputePipelineState* _bloomBlurVPipeline;
    
    // Depth stencil states
    MTL::DepthStencilState* _depthWriteState;
    MTL::DepthStencilState* _depthReadState;
    MTL::DepthStencilState* _noDepthState;
    
    // Samplers
    MTL::SamplerState* _linearSampler;
    MTL::SamplerState* _nearestSampler;
    MTL::SamplerState* _shadowSampler;
    
    // Uniform buffers
    MTL::Buffer* _cameraBuffer;
    MTL::Buffer* _lightBuffer;
    
    // Post-process params
    struct PostProcessParams {
        float exposure = 1.2f;
        float gamma = 2.2f;
        float bloomIntensity = 0.3f;
        float bloomThreshold = 1.0f;
        float vignetteIntensity = 0.3f;
        float vignetteRadius = 0.7f;
        float chromaticAberration = 0.5f;
        float filmGrain = 0.02f;
        simd::float2 screenSize;
        float time = 0.0f;
        float _pad;
    } _postParams;
    
    float _frameTime;
};

































//// Terrain chunk for streaming
//struct TerrainChunk
//{
//    static constexpr int CHUNK_SIZE = 8;
//    static constexpr int VERTEX_RESOLUTION = 166; // Vertices per chunk side
//    
//    simd::int2 chunkCoord;
//    float lodLevel;
//    bool needsUpdate;
//    bool visible;
//    
//    MTL::Buffer* vertexBuffer;
//    MTL::Buffer* indexBuffer;
//    size_t indexCount;
//    
//    // Bounding box for culling
//    simd::float3 boundsMin;
//    simd::float3 boundsMax;
//    
//    TerrainChunk() : chunkCoord{0,0}, lodLevel(0), needsUpdate(true),
//                     visible(true), vertexBuffer(nullptr), indexBuffer(nullptr),
//                     indexCount(0) {}
//};
//
//struct TerrainVertex
//{
//    simd::float3 position;
//    simd::float3 normal;
//    simd::float3 tangent;
//    simd::float2 texCoord;
//    simd::float4 color;      // Biome color/blending
//    float height;
//    float biomeId;           // Which biome this vertex belongs to
//};
//
//struct TerrainUniforms
//{
//    simd::float4x4 modelMatrix;
//    simd::float4x4 viewProjectionMatrix;
//    simd::float3 cameraPosition;
//    float time;
//    simd::float4 fogColor;
//    float fogDensity;
//    float maxRenderDistance;
//    uint32_t seed;
//    float heightScale;
//};
//
//// Biome parameters
//struct BiomeParams
//{
//    BiomeType type;
//    simd::float2 center;     // Voronoi center point
//    float radius;
//    float heightOffset;
//    float roughness;
//    simd::float4 color;
//    float transitionWidth;
//};
//
//class TerrainSystem
//{
//public:
//    TerrainSystem(MTL::Device* device,
//                  MTL::PixelFormat colorFormat,
//                  MTL::PixelFormat depthFormat,
//                  MTL::Library* library);
//    ~TerrainSystem();
//    
//    // Initialization
//    void initialize(uint32_t seed);
//    void setSeed(uint32_t seed) { m_seed = seed; }
//    
//    // Update and rendering
//    void update(float deltaTime, const simd::float3& cameraPosition);
//    void render(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection);
//    
//    // Chunk management
//    void streamChunks(const simd::float3& cameraPosition);
//    void unloadDistantChunks(const simd::float3& cameraPosition);
//    
//    // Height queries
//    float getHeightAt(float x, float z) const;
//    simd::float3 getNormalAt(float x, float z) const;
//    BiomeType getBiomeAt(float x, float z) const;
//    
//    // Collision
//    bool checkTerrainCollision(const simd::float3& position, float radius) const;
//    simd::float3 resolveTerrainCollision(const simd::float3& position,
//                                         const simd::float3& velocity,
//                                         float radius) const;
//    
//    // Settings
//    void setHeightScale(float scale) { m_heightScale = scale; }
//    void setRenderDistance(float distance) { m_maxRenderDistance = distance; }
//    void setLODEnabled(bool enabled) { m_lodEnabled = enabled; }
//    
//    // Debug
//    void setWireframe(bool enabled) { m_wireframe = enabled; }
//    size_t getChunkCount() const { return m_chunks.size(); }
//    
//private:
//    MTL::Device* m_device;
//    MTL::Library* m_library;
//    
//    // Rendering
//    MTL::RenderPipelineState* m_terrainPipeline;
//    MTL::DepthStencilState* m_depthStencilState;
//    MTL::Buffer* m_uniformsBuffer;
//    TerrainUniforms m_uniforms;
//    
//    // Compute shaders for GPU terrain generation
//    MTL::ComputePipelineState* m_voronoiGeneratorPipeline;
//    MTL::ComputePipelineState* m_heightmapGeneratorPipeline;
//    MTL::ComputePipelineState* m_normalCalculatorPipeline;
//    MTL::ComputePipelineState* m_erosionPipeline;
//    
//    // Textures
//    MTL::Texture* m_heightmapTexture;       // GPU-generated heightmap
//    MTL::Texture* m_normalTexture;           // Computed normals
//    MTL::Texture* m_biomeTexture;            // Biome ID map
//    MTL::Texture* m_voronoiTexture;          // Voronoi diagram
//    
//    // Material textures (arrays for different biomes)
//    MTL::Texture* m_albedoArray;
//    MTL::Texture* m_normalMapArray;
//    MTL::Texture* m_roughnessArray;
//    MTL::Texture* m_metallicArray;
//    MTL::Texture* m_aoArray;
//    
//    // Chunks
//    std::vector<std::unique_ptr<TerrainChunk>> m_chunks;
//    
//    // Biome system
//    std::vector<BiomeParams> m_biomes;
////    std::unique_ptr<BiomeGen> m_biomeGenerator;
//    
//    // Parameters
//    uint32_t m_seed;
//    float m_heightScale;
//    float m_maxRenderDistance;
//    bool m_lodEnabled;
//    bool m_wireframe;
//    float m_time;
//    
//    // Noise parameters
//    int m_octaves;
//    float m_persistence;
//    float m_lacunarity;
//    float m_noiseScale;
//    
//    // Helpers
//    void createPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat);
//    void createTextures();
//    void loadMaterialTextures(const std::string& resourcePath);
//    void generateBiomeMap();
//    void generateHeightmapGPU();
//    void generateNormalsGPU();
//    void applyErosionGPU(int iterations);
//    
//    TerrainChunk* getOrCreateChunk(int chunkX, int chunkZ);
//    void generateChunkMesh(TerrainChunk* chunk);
//    void generateChunkMeshGPU(TerrainChunk* chunk);
//    
//    float sampleHeightmap(float x, float z) const;
//    simd::float2 worldToUV(float x, float z) const;
//    
//    // Biome generation
//    void initializeSpawnBiome();
//    void generateVoronoiBiomes();
//};
//
//// Separate biome generator for complex Voronoi calculations
//class BiomeGen {
//public:
//    BiomeGen(uint32_t seed);
//    
//    struct VoronoiCell {
//        simd::float2 center;
//        BiomeType biomeType;
//        float radius;
//        simd::float4 color;
//    };
//    
//    void generate(float worldSize, int numCells);
//    const std::vector<VoronoiCell>& getCells() const { return m_cells; }
//    
//    BiomeType getBiomeAt(const simd::float2& position) const;
//    simd::float2 getNearestCellCenter(const simd::float2& position) const;
//    float getDistanceToNearestEdge(const simd::float2& position) const;
//    
//    // GPU compute shader helpers
//    MTL::Buffer* getCellBuffer() const { return m_cellBuffer; }
//    void uploadToGPU(MTL::Device* device);
//    
//private:
//    std::mt19937 m_rng;
//    std::vector<VoronoiCell> m_cells;
//    MTL::Buffer* m_cellBuffer;
//
//    BiomeType determineBiomeType(float distanceFromSpawn, float randomValue);
//};

#endif /* RMDLSystem_hpp */

