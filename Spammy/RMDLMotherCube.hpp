//
//  RMDLMotherCube.hpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#ifndef RMDLMotherCube_hpp
#define RMDLMotherCube_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace cube {

struct BlockVertex
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 uv;
    simd::float4 color;
};

struct BlockFace
{
    enum Direction : uint8_t { PosX, NegX, PosY, NegY, PosZ, NegZ, Count };
};

struct AttachmentPoint
{
    simd::float3 localPos;
    simd::float3 normal;
    BlockFace::Direction face;
    bool occupied = false;
};

enum class BlockCategory : uint8_t {
    Structure,
    Propulsion,
    Control,
    Weapon,
    Utility,
    Cosmetic
};

enum class BlockType : uint16_t {
    // Structure
    CubeBasic = 0,
    CubeArmored,
    CubeLight,
    Wedge,
    Corner,
    Beam,

    // Propulsion
    WheelSmall = 100,
    WheelMedium,
    WheelLarge,
    ThrusterSmall,
    ThrusterLarge,
    Hover,
    
    // Control
    Cockpit = 200,
    CommandSeat,
    RemoteControl,
    AICore,
    
    // Cosmetic / Robot
    RobotHead = 300,
    RobotEye,
    Antenna,
    Light,
    
    // Utility
    Battery = 400,
    Generator,
    FuelTank,
    Radar,
    
    Blender = 500,
    Ico
};

struct BlockDefinition
{
    BlockType type;
    BlockCategory category;
    const char* name;
    const char* description;
    
    float mass;
    float health;
    float cost;
    
    simd::float3 size;
    simd::float4 baseColor;
    
    bool canRotate = true;
    bool isTransparent = false;
    bool hasAnimation = false;
    
    std::vector<AttachmentPoint> attachments;
    
    // Stats spécifiques
    union {
        struct { float maxSpeed, torque, grip; } wheel;
        struct { float thrust, fuelConsumption; } thruster;
        struct { float capacity, rechargeRate; } battery;
        struct { float range, accuracy; } weapon;
    } stats;
};

// ============================================================================
// BLOCK INSTANCE - Runtime data for placed blocks
// ============================================================================

struct BlockInstance {
    uint32_t id;
    BlockType type;
    
    simd::int3 gridPos;
    uint8_t rotation;            // 0-23 (24 orientations possibles)
    
    simd::float4 tintColor;
    float damage;
    bool powered;
    bool active;
    
    simd::float4x4 getModelMatrix() const {
        simd::float4x4 t = simd::float4x4(1.0f);
        t.columns[3] = simd::float4{(float)gridPos.x, (float)gridPos.y, (float)gridPos.z, 1.0f};
        return t * getRotationMatrix();
    }
    
    simd::float4x4 getRotationMatrix() const;
};

// ============================================================================
// BLOCK MESH GENERATOR
// ============================================================================

class BlockMeshGenerator {
public:
    static void generateCube(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                            simd::float3 size, simd::float4 color);
    
    static void generateWedge(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                             simd::float3 size, simd::float4 color);
    
    static void generateWheel(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                             float radius, float width, int segments, simd::float4 color);
    
    static void generateCockpit(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                               simd::float3 size, simd::float4 color);
    
    static void generateRobotHead(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                                  simd::float4 color);
    
    static void generateThruster(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                                float radius, float length, simd::float4 color);
    
    static void generateIcosphere(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float4 c);
    
private:
    static void addQuad(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                       simd::float3 p0, simd::float3 p1, simd::float3 p2, simd::float3 p3,
                       simd::float3 normal, simd::float4 color);
    
    static void addTriangle(std::vector<BlockVertex>& verts, std::vector<uint16_t>& indices,
                           simd::float3 p0, simd::float3 p1, simd::float3 p2,
                           simd::float3 normal, simd::float4 color);
};

// ============================================================================
// BLOCK REGISTRY - Singleton catalog of all block definitions
// ============================================================================

class BlockRegistry {
public:
    static BlockRegistry& instance() {
        static BlockRegistry reg;
        return reg;
    }
    
    const BlockDefinition* get(BlockType type) const {
        auto it = m_definitions.find(type);
        return it != m_definitions.end() ? &it->second : nullptr;
    }
    
    void registerBlock(BlockDefinition def) {
        m_definitions[def.type] = std::move(def);
    }
    
    const auto& all() const { return m_definitions; }
    
private:
    BlockRegistry() { registerDefaults(); }
    void registerDefaults();
    
    std::unordered_map<BlockType, BlockDefinition> m_definitions;
};

// ============================================================================
// GPU INSTANCE DATA
// ============================================================================

struct BlockGPUInstance {
    simd::float4x4 modelMatrix;
    simd::float4 color;
    simd::float4 params;  // x=damage, y=powered, z=animPhase, w=blockTypeId
};

// ============================================================================
// BLOCK RENDERER - Batched instanced rendering
// ============================================================================

class BlockRenderer {
public:
    BlockRenderer(MTL::Device* device, MTL::PixelFormat colorFormat,
                  MTL::PixelFormat depthFormat, MTL::Library* library);
    ~BlockRenderer();
    
    void buildMeshes();
    void updateInstances(const std::vector<BlockInstance>& blocks, float time);
    void render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos);
    
    void setGhostBlock(BlockType type, simd::int3 pos, uint8_t rot);
    void clearGhost() { m_hasGhost = false; }
    
private:
    void createPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat);
    void createMeshBuffers();
    
    MTL::Device* m_device;
    MTL::Library* m_library;
    MTL::RenderPipelineState* m_pipeline = nullptr;
    MTL::DepthStencilState* m_depthState = nullptr;
    
    // Par type de bloc: offset et count dans le buffer unifié
    struct MeshRange {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t indexCount;
    };
    std::unordered_map<BlockType, MeshRange> m_meshRanges;
    
    MTL::Buffer* m_vertexBuffer = nullptr;
    MTL::Buffer* m_indexBuffer = nullptr;
    MTL::Buffer* m_instanceBuffer = nullptr;
    
    std::vector<BlockGPUInstance> m_gpuInstances;
    uint32_t m_instanceCount = 0;
    
    // Ghost block preview
    bool m_hasGhost = false;
    BlockType m_ghostType;
    simd::int3 m_ghostPos;
    uint8_t m_ghostRot;
};

// ============================================================================
// BLOCK SYSTEM - Main manager
// ============================================================================

class BlockSystem {
public:
    BlockSystem(MTL::Device* device, MTL::PixelFormat colorFormat,
                MTL::PixelFormat depthFormat, MTL::Library* library);
    
    // Block management
    uint32_t addBlock(BlockType type, simd::int3 pos, uint8_t rotation = 0);
    bool removeBlock(uint32_t id);
    bool removeBlockAt(simd::int3 pos);
    BlockInstance* getBlock(uint32_t id);
    BlockInstance* getBlockAt(simd::int3 pos);
    
    // Validation
    bool canPlaceAt(BlockType type, simd::int3 pos, uint8_t rotation) const;
    bool hasNeighbor(simd::int3 pos) const;
    
    // Color customization
    void setBlockColor(uint32_t id, simd::float4 color);
    
    // Update & Render
    void update(float dt);
    void render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos);
    
    // Ghost preview
    void setGhostBlock(BlockType type, simd::int3 pos, uint8_t rot);
    void clearGhost();
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    void deserialize(const std::vector<uint8_t>& data);
    
    // Stats
    float totalMass() const;
    simd::float3 centerOfMass() const;
    size_t blockCount() const { return m_blocks.size(); }
    
private:
    std::unique_ptr<BlockRenderer> m_renderer;
    std::vector<BlockInstance> m_blocks;
    std::unordered_map<uint64_t, uint32_t> m_gridMap;  // gridPos hash -> block id
    uint32_t m_nextId = 1;
    float m_time = 0.0f;
    
    uint64_t hashPos(simd::int3 p) const {
        return (uint64_t(p.x + 512) << 20) | (uint64_t(p.y + 512) << 10) | uint64_t(p.z + 512);
    }
};

}


#endif /* RMDLMotherCube_hpp */
