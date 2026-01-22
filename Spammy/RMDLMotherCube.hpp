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

#include "RMDLFontLoader.h"

namespace cube {

struct alignas(16) BlockGPUInstance
{
    simd::float4x4 modelMatrix;
    simd::float4 color;
    simd::float4 params;
};

struct alignas(16) BlockVertex
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 uv;
    simd::float4 color;
};

struct alignas(16) BlockUniforms
{
    simd::float4x4 viewProj;
    simd::float3 camPos;
    float time;
    simd::float3 lightDir;
    float _pad;
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

    CubeBasic = 0,
    CubeArmored,
    CubeLight,
    Wedge,
    Corner,
    Beam,

    WheelSmall = 100,
    WheelMedium,
    WheelLarge,
    ThrusterSmall,
    ThrusterLarge,
    Hover,
    
    Cockpit = 200,
    CommandSeat,
    RemoteControl,
    AICore,
    
    RobotHead = 300,
    RobotEye,
    Antenna,
    Light,
    
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
    uint8_t textureType = 0;
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
    
    union {
        struct { float maxSpeed, torque, grip; } wheel;
        struct { float thrust, fuelConsumption; } thruster;
        struct { float capacity, rechargeRate; } battery;
        struct { float range, accuracy; } weapon;
    } stats;
};

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

class BlockTextureManager
{
public:
    BlockTextureManager(MTL::Device* device) : m_device(device) {}
    ~BlockTextureManager() { cleanup(); }
    
    // return i for array
    uint32_t loadTexture(const std::string& path);
    
    void createDefaultTexture()
    {
        if (!m_stagingTextures.empty()) return;
        
        auto desc = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA16Unorm, 4, 4, false);
        desc->setUsage(MTL::TextureUsageShaderRead);
        desc->setStorageMode(MTL::StorageModeShared);
        
        MTL::Texture* defaultTex = m_device->newTexture(desc);
        
        // Fill with white
        uint32_t white[16];
        for (int i = 0; i < 16; i++) white[i] = 0xFFFFFFFF;
        
        defaultTex->replaceRegion(MTL::Region(0, 0, 0, 4, 4, 1), 0, white, 4 * 4);
        
        m_stagingTextures.push_back(defaultTex);
        m_textureIndices["default"] = 0;
        m_textureSize = 4;
        
        printf("Created default white texture\n");
    }
    
    // Crée le texture array final (appeler après tous les loadTexture)
    void buildTextureArray(MTL::CommandQueue* queue);
    
    // Accessors
    MTL::Texture* getTextureArray() const { return m_textureArray; }
    MTL::SamplerState* getSampler() const { return m_sampler; }
    uint32_t getTextureIndex(const std::string& name) const;
    
    void cleanup();
    
private:
    MTL::Device* m_device;
    MTL::Texture* m_textureArray = nullptr;
    MTL::SamplerState* m_sampler = nullptr;
    
    std::vector<MTL::Texture*> m_stagingTextures;
    std::unordered_map<std::string, uint32_t> m_textureIndices;
    
    uint32_t m_textureSize = 2048;
};

inline uint32_t BlockTextureManager::loadTexture(const std::string& path)
{
    // Extract filename for indexing
    size_t lastSlash = path.find_last_of("/\\");
    std::string name = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
    
    if (m_textureIndices.count(name)) {
        return m_textureIndices[name];
    }
    
    MTL::Texture* tex = loadSingleTexture(path, m_device);
    if (!tex) return 0;
    
    uint32_t index = (uint32_t)m_stagingTextures.size();
    m_stagingTextures.push_back(tex);
    m_textureIndices[name] = index;
    
    if (index == 0)
            m_textureSize = (uint32_t)tex->width();
    
    return index;
}

inline void BlockTextureManager::buildTextureArray(MTL::CommandQueue* queue)
{
    createDefaultTexture();
    if (m_stagingTextures.empty()) return;
    
    uint32_t size = (uint32_t)m_stagingTextures[0]->width();
    
    auto desc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA16Unorm, m_textureSize, m_textureSize, true);
    desc->setTextureType(MTL::TextureType2DArray);
    desc->setArrayLength(m_stagingTextures.size());
    desc->setUsage(MTL::TextureUsageShaderRead);
    desc->setStorageMode(MTL::StorageModePrivate);
    desc->setMipmapLevelCount(1 + (uint32_t)log2(size));
    
    m_textureArray = m_device->newTexture(desc);
    
    // Copy textures to array
    auto cmdBuf = queue->commandBuffer();
    auto blit = cmdBuf->blitCommandEncoder();
    
    for (size_t i = 0; i < m_stagingTextures.size(); i++)
    {
        auto src = m_stagingTextures[i];
        uint32_t srcW = (uint32_t)src->width();
        uint32_t srcH = (uint32_t)src->height();
        
        // Copy (si tailles différentes, ça va crop/pad - idéalement resize avant)
        uint32_t copyW = std::min(srcW, size);
        uint32_t copyH = std::min(srcH, size);
        
        blit->copyFromTexture(src, 0, 0, MTL::Origin(0, 0, 0),
                              MTL::Size(copyW, copyH, 1), // textureW
                              m_textureArray, i, 0, MTL::Origin(0, 0, 0));
    }
    
    blit->generateMipmaps(m_textureArray);
    blit->endEncoding();
    cmdBuf->commit();
    cmdBuf->waitUntilCompleted();
    
    // Create sampler
    auto samplerDesc = MTL::SamplerDescriptor::alloc()->init();
    samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMipFilter(MTL::SamplerMipFilterLinear);
    samplerDesc->setSAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setTAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setMaxAnisotropy(8);
    m_sampler = m_device->newSamplerState(samplerDesc);
    samplerDesc->release();
    
    // Release staging textures
    for (auto tex : m_stagingTextures) {
        tex->release();
    }
    m_stagingTextures.clear();
}

inline uint32_t BlockTextureManager::getTextureIndex(const std::string& name) const {
    auto it = m_textureIndices.find(name);
    return (it != m_textureIndices.end()) ? it->second : 0;
}

inline void BlockTextureManager::cleanup() {
    if (m_textureArray) { m_textureArray->release(); m_textureArray = nullptr; }
    if (m_sampler) { m_sampler->release(); m_sampler = nullptr; }
    for (auto tex : m_stagingTextures) tex->release();
    m_stagingTextures.clear();
}

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

class BlockRegistry
{
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

class BlockRenderer
{
public:
    BlockRenderer(MTL::Device* device, MTL::PixelFormat colorFormat,
                  MTL::PixelFormat depthFormat, MTL::Library* library, const std::string& resourcesPath, MTL::CommandQueue* commandQueue);
    ~BlockRenderer();
    
    void buildMeshes();
    void loadTextures(const std::string& resourcePath, MTL::CommandQueue* queue);
    void updateInstances(const std::vector<BlockInstance>& blocks, float time);
    void render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos, float time);
    
    void setGhostBlock(BlockType type, simd::int3 pos, uint8_t rot);
    void clearGhost() { m_hasGhost = false; }
    
    BlockTextureManager& textures() { return m_textures; }
    
private:
    void createPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat);
    void createMeshBuffers();
    
    MTL::Device* m_device;
    MTL::Library* m_library;
    MTL::RenderPipelineState* m_pipeline = nullptr;
    MTL::DepthStencilState* m_depthState = nullptr;
    MTL::RenderPipelineState* m_pipelineOpaque = nullptr;
    MTL::RenderPipelineState* m_pipelineTransparent = nullptr;
    MTL::DepthStencilState* m_depthStateOpaque = nullptr;
    MTL::DepthStencilState* m_depthStateTransparent = nullptr;
    
    MTL::Buffer* m_vertexBuffer = nullptr;
    MTL::Buffer* m_indexBuffer = nullptr;
    uint32_t m_totalVertices = 0;
    uint32_t m_totalIndices = 0;
    
    // Instance data - triple buffering
    static constexpr uint32_t kMaxInstances = 8192;
    static constexpr uint32_t kBufferCount = 3;
    MTL::Buffer* m_instanceBuffers[kBufferCount] = {};
    MTL::Buffer* m_uniformBuffers[kBufferCount] = {};
    uint32_t m_bufferIndex = 0;
    
    // Par type de bloc: offset et count dans le buffer unifié
    struct MeshRange
    {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t indexCount;
        uint32_t textureIndex;
        bool transparent;
    };
    std::unordered_map<BlockType, MeshRange> m_meshRanges;
    
    struct DrawBatch
    {
        BlockType type;
        uint32_t instanceOffset;
        uint32_t instanceCount;
    };
    std::vector<DrawBatch> m_opaqueBatches;
    std::vector<DrawBatch> m_transparentBatches;
    
    std::vector<BlockGPUInstance> m_gpuInstances;
    uint32_t m_instanceCount = 0;
    
    // Ghost block preview
    bool m_hasGhost = false;
    BlockType m_ghostType;
    simd::int3 m_ghostPos;
    uint8_t m_ghostRot;
    
    BlockTextureManager m_textures;
//    std::unordered_map<std::string, NS::SharedPtr<MTL::Texture>> m_textureAssets;
//    MTL::Texture*               m_diffuseTexture = nullptr;
};

class BlockSystem
{
public:
    BlockSystem(MTL::Device* device, MTL::PixelFormat colorFormat,
                MTL::PixelFormat depthFormat, MTL::Library* library, const std::string& resourcesPath, MTL::CommandQueue* commandQueue);
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
    void render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos, float time);
    
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
