//
//  RMDLMotherCube.cpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#include "RMDLMotherCube.hpp"

#include <cmath>
#include <simd/simd.h>

namespace cube {

void BlockRegistry::registerDefaults()
{
    registerBlock({
        .type = BlockType::CubeBasic,
        .category = BlockCategory::Structure,
        .name = "Basic Cube",
        .description = "Standard structural block",
        .mass = 1.0f, .health = 100.0f, .cost = 10.0f,
        .size = {1, 1, 1},
        .baseColor = {0.6f, 0.62f, 0.65f, 1.0f},
        .attachments = {
            {{0,0.5f,0}, {0,1,0}, BlockFace::PosY},
            {{0,-0.5f,0}, {0,-1,0}, BlockFace::NegY},
            {{0.5f,0,0}, {1,0,0}, BlockFace::PosX},
            {{-0.5f,0,0}, {-1,0,0}, BlockFace::NegX},
            {{0,0,0.5f}, {0,0,1}, BlockFace::PosZ},
            {{0,0,-0.5f}, {0,0,-1}, BlockFace::NegZ},
        }
    });
    
    registerBlock({
        .type = BlockType::CubeArmored,
        .category = BlockCategory::Structure,
        .name = "Armored Cube",
        .description = "Heavy reinforced block",
        .mass = 3.0f, .health = 300.0f, .cost = 50.0f,
        .size = {1, 1, 1},
        .baseColor = {0.35f, 0.38f, 0.42f, 1.0f},
    });
    
    registerBlock({
        .type = BlockType::Wedge,
        .category = BlockCategory::Structure,
        .name = "Wedge",
        .description = "Angled structural block",
        .mass = 0.5f, .health = 50.0f, .cost = 15.0f,
        .size = {1, 1, 1},
        .baseColor = {0.6f, 0.62f, 0.65f, 1.0f},
    });
    
    // === PROPULSION ===
    registerBlock({
        .type = BlockType::WheelSmall,
        .category = BlockCategory::Propulsion,
        .name = "Small Wheel",
        .description = "Compact wheel for light vehicles",
        .mass = 0.8f, .health = 80.0f, .cost = 30.0f,
        .size = {1, 1, 0.5f},
        .baseColor = {0.8f, 0.4f, 0.1f, 1.0f},
        .stats = {.wheel = {.maxSpeed = 30.0f, .torque = 50.0f, .grip = 0.8f}}
    });
    
    registerBlock({
        .type = BlockType::WheelMedium,
        .category = BlockCategory::Propulsion,
        .name = "Medium Wheel",
        .description = "Balanced all-terrain wheel",
        .mass = 1.5f, .health = 120.0f, .cost = 60.0f,
        .size = {1, 1, 0.6f},
        .baseColor = {0.9f, 0.5f, 0.15f, 1.0f},
        .stats = {.wheel = {.maxSpeed = 25.0f, .torque = 100.0f, .grip = 1.0f}}
    });
    
    registerBlock({
        .type = BlockType::WheelLarge,
        .category = BlockCategory::Propulsion,
        .name = "Large Wheel",
        .description = "Heavy-duty wheel for big vehicles",
        .mass = 3.0f, .health = 200.0f, .cost = 120.0f,
        .size = {1.5f, 1.5f, 0.8f},
        .baseColor = {1.0f, 0.6f, 0.2f, 1.0f},
        .stats = {.wheel = {.maxSpeed = 20.0f, .torque = 200.0f, .grip = 1.2f}}
    });
    
    registerBlock({
        .type = BlockType::ThrusterSmall,
        .category = BlockCategory::Propulsion,
        .name = "Small Thruster",
        .description = "Compact jet thruster",
        .mass = 0.5f, .health = 40.0f, .cost = 80.0f,
        .size = {0.5f, 0.5f, 1},
        .baseColor = {0.5f, 0.55f, 0.6f, 1.0f},
        .stats = {.thruster = {.thrust = 500.0f, .fuelConsumption = 1.0f}}
    });
    
    // === CONTROL ===
    registerBlock({
        .type = BlockType::Cockpit,
        .category = BlockCategory::Control,
        .name = "Cockpit",
        .description = "Command center with pilot seat",
        .mass = 2.0f, .health = 150.0f, .cost = 200.0f,
        .size = {1, 1, 2},
        .baseColor = {0.4f, 0.45f, 0.5f, 1.0f},
        .isTransparent = true,
    });
    
    registerBlock({
        .type = BlockType::CommandSeat,
        .category = BlockCategory::Control,
        .name = "Command Seat",
        .description = "Exposed command seat",
        .mass = 0.5f, .health = 50.0f, .cost = 100.0f,
        .size = {1, 1, 1},
        .baseColor = {0.3f, 0.35f, 0.4f, 1.0f},
    });
    
    // === ROBOT / COSMETIC ===
    registerBlock({
        .type = BlockType::RobotHead,
        .category = BlockCategory::Cosmetic,
        .name = "Robot Head",
        .description = "Mecha-style head unit with sensors",
        .mass = 1.0f, .health = 100.0f, .cost = 150.0f,
        .size = {1, 1, 1},
        .baseColor = {0.5f, 0.52f, 0.55f, 1.0f},
        .hasAnimation = true,
    });
    
    registerBlock({
        .type = BlockType::Antenna,
        .category = BlockCategory::Cosmetic,
        .name = "Antenna",
        .description = "Communication antenna",
        .mass = 0.1f, .health = 20.0f, .cost = 25.0f,
        .size = {0.2f, 1, 0.2f},
        .baseColor = {0.7f, 0.72f, 0.75f, 1.0f},
    });
    
    // === UTILITY ===
    registerBlock({
        .type = BlockType::Battery,
        .category = BlockCategory::Utility,
        .name = "Battery",
        .description = "Energy storage unit",
        .mass = 1.5f, .health = 80.0f, .cost = 100.0f,
        .size = {1, 1, 1},
        .baseColor = {0.2f, 0.5f, 0.8f, 1.0f},
        .stats = {.battery = {.capacity = 1000.0f, .rechargeRate = 10.0f}}
    });
    
//    registerBlock({.type = BlockType::Blender,})
    registerBlock({ .type = BlockType::Blender,
                    .textureType = 0,
                    .category = BlockCategory::Cosmetic,
                    .name = "Icosphere",
                    .description = "Blender import",
                    .mass = 1.0f, .health = 100.0f, .cost = 50.0f,
                    .size = {1, 1, 1},
                    .baseColor = {0.6f, 0.62f, 0.65f, 1.0f}, });
    registerBlock({ .type = BlockType::WTF,
                    .name = "WTF",
                    .description = "Blender import Script",
                    .mass = 1.0f, .health = 100.0f, .cost = 50.0f,
                    .size = {1, 1, 1},
                    .baseColor = {0.6f, 0.62f, 0.65f, 1.0f},
                    .stats = {.battery = {.capacity = 1000.0f, .rechargeRate = 10.0f}}, });
}

BlockRenderer::BlockRenderer(MTL::Device* device, MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* library, const std::string& resourcesPath, MTL::CommandQueue* commandQueue)
: m_device(device), m_library(library), m_textures(device)
{
    createPipeline(colorFormat, depthFormat);
    for (int i = 0; i < kBufferCount; i++)
    {
        m_instanceBuffers[i] = device->newBuffer(kMaxInstances * sizeof(BlockGPUInstance), MTL::ResourceStorageModeShared);
        m_uniformBuffers[i] = device->newBuffer(sizeof(BlockUniforms), MTL::ResourceStorageModeShared);
    }
    
    m_gpuInstances.reserve(kMaxInstances);
    
    loadTextures(resourcesPath, commandQueue);
    buildMeshes();
}

BlockRenderer::~BlockRenderer()
{
    if (m_pipeline) m_pipeline->release();
    if (m_depthState) m_depthState->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    if (m_pipelineOpaque) m_pipelineOpaque->release();
    if (m_pipelineTransparent) m_pipelineTransparent->release();
    if (m_depthStateOpaque) m_depthStateOpaque->release();
    if (m_depthStateTransparent) m_depthStateTransparent->release();
    for (int i = 0; i < kBufferCount; i++)
    {
        if (m_instanceBuffers[i]) m_instanceBuffers[i]->release();
        if (m_uniformBuffers[i]) m_uniformBuffers[i]->release();
    }
}

void BlockRenderer::createPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat)
{
    NS::Error* err = nullptr;
 
    auto vertFn = m_library->newFunction(MTLSTR("blockVertexOptimized"));
    auto fragOpaque = m_library->newFunction(MTLSTR("blockFragmentPBR"));
    auto fragTransparent = m_library->newFunction(MTLSTR("blockFragmentTransparent"));
    
    // Vertex descriptor
    auto vertDesc = MTL::VertexDescriptor::alloc()->init();
    
    // Position
    vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertDesc->attributes()->object(0)->setOffset(offsetof(BlockVertex, position));
    vertDesc->attributes()->object(0)->setBufferIndex(0);
    // Normal
    vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertDesc->attributes()->object(1)->setOffset(offsetof(BlockVertex, normal));
    vertDesc->attributes()->object(1)->setBufferIndex(0);
    // UV
    vertDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertDesc->attributes()->object(2)->setOffset(offsetof(BlockVertex, uv));
    vertDesc->attributes()->object(2)->setBufferIndex(0);
    // Color
    vertDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat4);
    vertDesc->attributes()->object(3)->setOffset(offsetof(BlockVertex, color));
    vertDesc->attributes()->object(3)->setBufferIndex(0);
    
    vertDesc->layouts()->object(0)->setStride(sizeof(BlockVertex));
    vertDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    // Opaque pipeline
    auto descOpaque = MTL::RenderPipelineDescriptor::alloc()->init();
    descOpaque->setVertexFunction(vertFn);
    descOpaque->setFragmentFunction(fragOpaque);
    descOpaque->setVertexDescriptor(vertDesc);
    descOpaque->setDepthAttachmentPixelFormat(depthFormat);
    descOpaque->colorAttachments()->object(0)->setPixelFormat(colorFormat);
    
    m_pipelineOpaque = m_device->newRenderPipelineState(descOpaque, &err);
    
    // Transparent pipeline
    auto descTrans = MTL::RenderPipelineDescriptor::alloc()->init();
    descTrans->setVertexFunction(vertFn);
    descTrans->setFragmentFunction(fragTransparent);
    descTrans->setVertexDescriptor(vertDesc);
    descTrans->setDepthAttachmentPixelFormat(depthFormat);
    
    auto colorAtt = descTrans->colorAttachments()->object(0);
    colorAtt->setPixelFormat(colorFormat);
    colorAtt->setBlendingEnabled(true);
    colorAtt->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    colorAtt->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    colorAtt->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    colorAtt->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    m_pipelineTransparent = m_device->newRenderPipelineState(descTrans, &err);
    
    // Depth states
    auto depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    m_depthStateOpaque = m_device->newDepthStencilState(depthDesc);
    
    depthDesc->setDepthWriteEnabled(false);
    m_depthStateTransparent = m_device->newDepthStencilState(depthDesc);
    
    vertFn->release();
    fragOpaque->release();
    fragTransparent->release();
    descOpaque->release();
    descTrans->release();
    vertDesc->release();
    depthDesc->release();
}

void BlockRenderer::loadTextures(const std::string& resourcePath, MTL::CommandQueue* queue)
{
//    std::vector<std::string> texturePaths = {
//        resourcePath + "/textures/metal_diffuse.png",
//        resourcePath + "/textures/block_diffuse.png",
//        resourcePath + "/textures/panel_diffuse.png",
//    };
//    
//    for (const auto& path : texturePaths) {
//        m_textures.loadTexture(path);  // Ignore les échecs silencieusement
//    }
    // Load block textures
    m_textures.loadTexture(resourcePath + "/diffusecube.png");
    
//    m_textures.loadTexture(resourcePath + "/textures/rubber_diffuse.png");
//    m_textures.loadTexture(resourcePath + "/textures/panel_diffuse.png");
//    m_textures.loadTexture(resourcePath + "/textures/glass_diffuse.png");
//    m_textures.loadTexture(resourcePath + "/textures/glow_diffuse.png");
    
    m_textures.buildTextureArray(queue);
}

void BlockRenderer::buildMeshes()
{
    std::vector<BlockVertex> allVerts;
    std::vector<uint16_t> allIndices;
    
    auto& registry = BlockRegistry::instance();
    
    for (auto& [type, def] : registry.all()) {
        MeshRange range;
        range.vertexOffset = (uint32_t)allVerts.size();
        range.indexOffset = (uint32_t)allIndices.size();
        range.textureIndex = def.textureType;
        range.transparent = def.isTransparent;
        
        std::vector<BlockVertex> verts;
        std::vector<uint16_t> indices;
        
        switch (type) {
            case BlockType::CubeBasic:
            case BlockType::CubeArmored:
            case BlockType::CubeLight:
            case BlockType::Battery:
                BlockMeshGenerator::generateCube(verts, indices, def.size, def.baseColor);
                break;
                
            case BlockType::Wedge:
            case BlockType::Corner:
                BlockMeshGenerator::generateWedge(verts, indices, def.size, def.baseColor);
                break;
                
            case BlockType::WheelSmall:
                BlockMeshGenerator::generateWheel(verts, indices, 0.4f, 0.25f, 16, def.baseColor);
                break;
            case BlockType::WheelMedium:
                BlockMeshGenerator::generateWheel(verts, indices, 0.45f, 0.3f, 20, def.baseColor);
                break;
            case BlockType::WheelLarge:
                BlockMeshGenerator::generateWheel(verts, indices, 0.6f, 0.4f, 24, def.baseColor);
                break;
                
            case BlockType::Cockpit:
            case BlockType::CommandSeat:
                BlockMeshGenerator::generateCockpit(verts, indices, def.size, def.baseColor);
                break;
                
            case BlockType::RobotHead:
                BlockMeshGenerator::generateRobotHead(verts, indices, def.baseColor);
                break;
                
            case BlockType::ThrusterSmall:
            case BlockType::ThrusterLarge:
                BlockMeshGenerator::generateThruster(verts, indices, 0.2f, 0.5f, def.baseColor);
                break;
            case BlockType::Blender:
                BlockMeshGenerator::generateIcosphere(verts, indices, def.baseColor);
                break;
            case BlockType::WTF:
                BlockMeshGenerator::generateWTF(verts, indices, def.baseColor);
            
                
            default:
                BlockMeshGenerator::generateCube(verts, indices, def.size, def.baseColor);
                break;
        }
        
        // Offset indices
        for (auto& idx : indices) idx += range.vertexOffset;
        
        range.indexCount = (uint32_t)indices.size();
        m_meshRanges[type] = range;
        
        allVerts.insert(allVerts.end(), verts.begin(), verts.end());
        allIndices.insert(allIndices.end(), indices.begin(), indices.end());
    }
    
    m_vertexBuffer = m_device->newBuffer(allVerts.data(), allVerts.size() * sizeof(BlockVertex), MTL::ResourceStorageModeShared);
    m_indexBuffer = m_device->newBuffer(allIndices.data(), allIndices.size() * sizeof(uint16_t), MTL::ResourceStorageModeShared);
    
    // Pre-allocate instance buffer (max 4096 blocks)
    m_instanceBuffers[0] = m_device->newBuffer(4096 * sizeof(BlockGPUInstance), MTL::ResourceStorageModeShared);
    m_instanceBuffers[1] = m_device->newBuffer(4096 * sizeof(BlockGPUInstance), MTL::ResourceStorageModeShared);
    m_instanceBuffers[2] = m_device->newBuffer(4096 * sizeof(BlockGPUInstance), MTL::ResourceStorageModeShared);
}

void BlockRenderer::updateInstances(const std::vector<BlockInstance>& blocks, float time) {
    m_gpuInstances.clear();
//    m_gpuInstances.reserve(blocks.size() + 1);
    m_opaqueBatches.clear();
    m_transparentBatches.clear();

    std::unordered_map<BlockType, std::vector<uint32_t>> typeGroups;
    
    for (uint32_t i = 0; i < blocks.size(); i++) {
        typeGroups[blocks[i].type].push_back(i);
    }
    
    // Build batches
    for (auto& [type, indices] : typeGroups) {
        auto rangeIt = m_meshRanges.find(type);
        if (rangeIt == m_meshRanges.end()) continue;
        
        const auto& range = rangeIt->second;
        
        DrawBatch batch;
        batch.type = type;
        batch.instanceOffset = (uint32_t)m_gpuInstances.size();
        batch.instanceCount = (uint32_t)indices.size();
        
        for (uint32_t idx : indices) {
            const auto& block = blocks[idx];
            
            BlockGPUInstance gpu;
            gpu.modelMatrix = block.getModelMatrix();
            gpu.color = block.tintColor;
            gpu.params = {block.damage, block.powered ? 1.0f : 0.0f,
                         time, (float)range.textureIndex};
            m_gpuInstances.push_back(gpu);
        }
        
        if (range.transparent) {
            m_transparentBatches.push_back(batch);
        } else {
            m_opaqueBatches.push_back(batch);
        }
    }
    
    // Ghost block
    if (m_hasGhost) {
        BlockInstance ghost;
        ghost.gridPos = m_ghostPos;
        ghost.rotation = m_ghostRot;
        ghost.type = m_ghostType;
        ghost.tintColor = {1, 1, 1, 0.5f};
        
        auto rangeIt = m_meshRanges.find(m_ghostType);
        if (rangeIt != m_meshRanges.end()) {
            DrawBatch batch;
            batch.type = m_ghostType;
            batch.instanceOffset = (uint32_t)m_gpuInstances.size();
            batch.instanceCount = 1;
            
            BlockGPUInstance gpu;
            gpu.modelMatrix = ghost.getModelMatrix();
            gpu.color = ghost.tintColor;
            gpu.params = {0, 0, time, (float)rangeIt->second.textureIndex};
            m_gpuInstances.push_back(gpu);
            
            m_transparentBatches.push_back(batch);
        }
    }
    
    m_instanceCount = (uint32_t)m_gpuInstances.size();
}

void BlockRenderer::render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos, float time)
{
    if (m_instanceCount == 0) return;
    m_bufferIndex = (m_bufferIndex + 1) % kBufferCount;
    
    BlockUniforms uniforms;
    uniforms.viewProj = viewProj;
    uniforms.camPos = camPos;
    uniforms.time = time;
    uniforms.lightDir = simd::normalize(simd::float3{0.5f, 1.0f, 0.3f});
    memcpy(m_uniformBuffers[m_bufferIndex]->contents(), &uniforms, sizeof(uniforms));
    memcpy(m_instanceBuffers[m_bufferIndex]->contents(), m_gpuInstances.data(),  m_instanceCount * sizeof(BlockGPUInstance));
    
    enc->setVertexBuffer(m_vertexBuffer, 0, 0);
    enc->setVertexBuffer(m_instanceBuffers[m_bufferIndex], 0, 1);
    enc->setVertexBuffer(m_uniformBuffers[m_bufferIndex], 0, 2);
    enc->setFragmentBuffer(m_uniformBuffers[m_bufferIndex], 0, 0);
    
    if (m_textures.getTextureArray())
    {
        enc->setFragmentTexture(m_textures.getTextureArray(), 0);
        enc->setFragmentSamplerState(m_textures.getSampler(), 0);
    }
    
    // === OPAQUE PASS ===
    enc->setRenderPipelineState(m_pipelineOpaque);
    enc->setDepthStencilState(m_depthStateOpaque);
    enc->setCullMode(MTL::CullModeBack);
    
    for (const auto& batch : m_opaqueBatches) {
        const auto& range = m_meshRanges[batch.type];
        
        enc->drawIndexedPrimitives(
                                   MTL::PrimitiveTypeTriangle,
                                   range.indexCount,
                                   MTL::IndexTypeUInt16,
                                   m_indexBuffer,
                                   range.indexOffset * sizeof(uint16_t),
                                   batch.instanceCount,
                                   0,  // baseVertex
                                   batch.instanceOffset
                                   );
    }
    
    // === TRANSPARENT PASS ===
    if (!m_transparentBatches.empty()) {
        enc->setRenderPipelineState(m_pipelineTransparent);
        enc->setDepthStencilState(m_depthStateTransparent);
        enc->setCullMode(MTL::CullModeNone);
        
        for (const auto& batch : m_transparentBatches) {
            const auto& range = m_meshRanges[batch.type];
            
            enc->drawIndexedPrimitives(
                                       MTL::PrimitiveTypeTriangle,
                                       range.indexCount,
                                       MTL::IndexTypeUInt16,
                                       m_indexBuffer,
                                       range.indexOffset * sizeof(uint16_t),
                                       batch.instanceCount,
                                       0,
                                       batch.instanceOffset
                                       );
        }
    }
}

void BlockRenderer::setGhostBlock(BlockType type, simd::int3 pos, uint8_t rot)
{
    m_hasGhost = true;
    m_ghostType = type;
    m_ghostPos = pos;
    m_ghostRot = rot;
}

// ============================================================================
// BLOCK SYSTEM
// ============================================================================

BlockSystem::BlockSystem(MTL::Device* device, MTL::PixelFormat colorFormat,
                         MTL::PixelFormat depthFormat, MTL::Library* library, const std::string& resourcesPath, MTL::CommandQueue* commandQueue) {
    m_renderer = std::make_unique<BlockRenderer>(device, colorFormat, depthFormat, library, resourcesPath, commandQueue);
}

uint32_t BlockSystem::addBlock(BlockType type, simd::int3 pos, uint8_t rotation) {
    if (!canPlaceAt(type, pos, rotation)) return 0;
    
    BlockInstance block;
    block.id = m_nextId++;
    block.type = type;
    block.gridPos = pos;
    block.rotation = rotation;
    block.tintColor = {1, 1, 1, 1};
    block.damage = 0;
    block.powered = true;
    block.active = true;
    
    m_blocks.push_back(block);
    m_gridMap[hashPos(pos)] = block.id;
    
    return block.id;
}

bool BlockSystem::removeBlock(uint32_t id) {
    auto it = std::find_if(m_blocks.begin(), m_blocks.end(),
                           [id](const BlockInstance& b) { return b.id == id; });
    if (it == m_blocks.end()) return false;
    
    m_gridMap.erase(hashPos(it->gridPos));
    m_blocks.erase(it);
    return true;
}

bool BlockSystem::removeBlockAt(simd::int3 pos) {
    auto it = m_gridMap.find(hashPos(pos));
    if (it == m_gridMap.end()) return false;
    return removeBlock(it->second);
}

BlockInstance* BlockSystem::getBlock(uint32_t id) {
    auto it = std::find_if(m_blocks.begin(), m_blocks.end(),
                           [id](const BlockInstance& b) { return b.id == id; });
    return it != m_blocks.end() ? &(*it) : nullptr;
}

BlockInstance* BlockSystem::getBlockAt(simd::int3 pos) {
    auto it = m_gridMap.find(hashPos(pos));
    if (it == m_gridMap.end()) return nullptr;
    return getBlock(it->second);
}

bool BlockSystem::canPlaceAt(BlockType type, simd::int3 pos, uint8_t rotation) const
{
//    return m_gridMap.find(hashPos(pos)) == m_gridMap.end();
    if (m_gridMap.find(hashPos(pos)) != m_gridMap.end()) return false;
    
    // First block can be placed anywhere
    if (m_blocks.empty()) return true;
    
    // Must have at least one neighbor
    return hasNeighbor(pos);
}

bool BlockSystem::hasNeighbor(simd::int3 pos) const
{
    static const simd::int3 offsets[6] = {
        {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
    };
    for (const auto& off : offsets)
    {
        simd::int3 neighbor = pos + off;
        if (m_gridMap.find(hashPos(neighbor)) != m_gridMap.end()) return true;
    }
    return false;
}

void BlockSystem::setBlockColor(uint32_t id, simd::float4 color)
{
    if (auto* block = getBlock(id)) block->tintColor = color;
}

void BlockSystem::update(float delta)
{
    m_time += delta;
    m_renderer->updateInstances(m_blocks, m_time);
}

void BlockSystem::render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos, float time)
{
    m_renderer->render(enc, viewProj, camPos, time);
}

void BlockSystem::setGhostBlock(BlockType type, simd::int3 pos, uint8_t rot) {
    m_renderer->setGhostBlock(type, pos, rot);
}

void BlockSystem::clearGhost() { m_renderer->clearGhost(); }

float BlockSystem::totalMass() const {
    float mass = 0;
    for (const auto& b : m_blocks) {
        if (auto* def = BlockRegistry::instance().get(b.type)) mass += def->mass;
    }
    return mass;
}

simd::float3 BlockSystem::centerOfMass() const
{
    simd::float3 com = {0, 0, 0};
    float totalM = 0;
    for (const auto& b : m_blocks)
    {
        if (auto* def = BlockRegistry::instance().get(b.type)) // RAII 
        {
            simd::float3 pos = {(float)b.gridPos.x, (float)b.gridPos.y, (float)b.gridPos.z};
            com += pos * def->mass;
            totalM += def->mass;
        }
    }
    return totalM > 0 ? com / totalM : simd::float3{0, 0, 0};
}

simd::float4x4 BlockInstance::getRotationMatrix() const
{
    // 24 orientations: 6 faces × 4 rotations autour de la normale
    static const simd::float4x4 rotations[24] =
    {   // Face +Y (up) - 4 rotations
        simd::float4x4(simd_matrix(simd::float4{1,0,0,0}, simd::float4{0,1,0,0}, simd::float4{0,0,1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,1,0}, simd::float4{0,1,0,0}, simd::float4{-1,0,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{-1,0,0,0}, simd::float4{0,1,0,0}, simd::float4{0,0,-1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,-1,0}, simd::float4{0,1,0,0}, simd::float4{1,0,0,0}, simd::float4{0,0,0,1})),
        // Face -Y (down)
        simd::float4x4(simd_matrix(simd::float4{1,0,0,0}, simd::float4{0,-1,0,0}, simd::float4{0,0,-1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,-1,0}, simd::float4{0,-1,0,0}, simd::float4{-1,0,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{-1,0,0,0}, simd::float4{0,-1,0,0}, simd::float4{0,0,1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,1,0}, simd::float4{0,-1,0,0}, simd::float4{1,0,0,0}, simd::float4{0,0,0,1})),
        // Face +X
        simd::float4x4(simd_matrix(simd::float4{0,1,0,0}, simd::float4{-1,0,0,0}, simd::float4{0,0,1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,1,0}, simd::float4{-1,0,0,0}, simd::float4{0,-1,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,-1,0,0}, simd::float4{-1,0,0,0}, simd::float4{0,0,-1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,-1,0}, simd::float4{-1,0,0,0}, simd::float4{0,1,0,0}, simd::float4{0,0,0,1})),
        // Face -X
        simd::float4x4(simd_matrix(simd::float4{0,-1,0,0}, simd::float4{1,0,0,0}, simd::float4{0,0,1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,1,0}, simd::float4{1,0,0,0}, simd::float4{0,1,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,1,0,0}, simd::float4{1,0,0,0}, simd::float4{0,0,-1,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,0,-1,0}, simd::float4{1,0,0,0}, simd::float4{0,-1,0,0}, simd::float4{0,0,0,1})),
        // Face +Z
        simd::float4x4(simd_matrix(simd::float4{1,0,0,0}, simd::float4{0,0,-1,0}, simd::float4{0,1,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,1,0,0}, simd::float4{0,0,-1,0}, simd::float4{-1,0,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{-1,0,0,0}, simd::float4{0,0,-1,0}, simd::float4{0,-1,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,-1,0,0}, simd::float4{0,0,-1,0}, simd::float4{1,0,0,0}, simd::float4{0,0,0,1})),
        // Face -Z
        simd::float4x4(simd_matrix(simd::float4{1,0,0,0}, simd::float4{0,0,1,0}, simd::float4{0,-1,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,-1,0,0}, simd::float4{0,0,1,0}, simd::float4{-1,0,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{-1,0,0,0}, simd::float4{0,0,1,0}, simd::float4{0,1,0,0}, simd::float4{0,0,0,1})),
        simd::float4x4(simd_matrix(simd::float4{0,1,0,0}, simd::float4{0,0,1,0}, simd::float4{1,0,0,0}, simd::float4{0,0,0,1})),
    };
    return rotations[rotation % 24];
}

void BlockMeshGenerator::addQuad(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float3 p0, simd::float3 p1, simd::float3 p2, simd::float3 p3, simd::float3 n, simd::float4 c)
{
    uint16_t base = (uint16_t)v.size();
    v.push_back({p0, n, {0,0}, c});
    v.push_back({p1, n, {1,0}, c});
    v.push_back({p2, n, {1,1}, c});
    v.push_back({p3, n, {0,1}, c});
    idx.insert(idx.end(), { base, uint16_t(base + 1), uint16_t(base + 2), base, uint16_t(base + 2), uint16_t(base + 3) });
}

void BlockMeshGenerator::addTriangle(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float3 p0, simd::float3 p1, simd::float3 p2, simd::float3 n, simd::float4 c)
{
    uint16_t base = (uint16_t)v.size();
    v.push_back({p0, n, {0,0}, c});
    v.push_back({p1, n, {1,0}, c});
    v.push_back({p2, n, {0.5f,1}, c});
    idx.insert(idx.end(), {base, uint16_t(base+1), uint16_t(base+2)});
}

void BlockMeshGenerator::generateCube(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float3 sz, simd::float4 c)
{
    float x = sz.x * 0.5f, y = sz.y * 0.5f, z = sz.z * 0.5f;
    
    // +Y (top)
    addQuad(v, idx, { -x, y, -z }, { x, y, -z }, { x, y, z }, { -x, y, z }, { 0, 1, 0 }, c);
    // -Y (bottom)
    addQuad(v, idx, {-x,-y,z}, {x,-y,z}, {x,-y,-z}, {-x,-y,-z}, {0,-1,0}, c * 0.8f);
    // +X (right)
    addQuad(v, idx, {x,-y,-z}, {x,-y,z}, {x,y,z}, {x,y,-z}, {1,0,0}, c * 0.9f);
    // -X (left)
    addQuad(v, idx, {-x,-y,z}, {-x,-y,-z}, {-x,y,-z}, {-x,y,z}, {-1,0,0}, c * 0.9f);
    // +Z (front)
    addQuad(v, idx, {-x,-y,z}, {-x,y,z}, {x,y,z}, {x,-y,z}, {0,0,1}, c * 0.85f);
    // -Z (back)
    addQuad(v, idx, {x,-y,-z}, {x,y,-z}, {-x,y,-z}, {-x,-y,-z}, {0,0,-1}, c * 0.85f);
}

void BlockMeshGenerator::generateWedge(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float3 sz, simd::float4 c)
{
    float x = sz.x * 0.5f, y = sz.y * 0.5f, z = sz.z * 0.5f;
    
    // Bottom face
    addQuad(v, idx, {-x,-y,z}, {x,-y,z}, {x,-y,-z}, {-x,-y,-z}, {0,-1,0}, c * 0.8f);
    // Back face (vertical)
    addQuad(v, idx, {x,-y,-z}, {x,y,-z}, {-x,y,-z}, {-x,-y,-z}, {0,0,-1}, c * 0.85f);
    // Slope face
    simd::float3 slopeN = simd::normalize(simd::float3{0, z, y});
    addQuad(v, idx, {-x,-y,z}, {-x,y,-z}, {x,y,-z}, {x,-y,z}, slopeN, c);
    // Left triangle
    simd::float3 leftN = {-1, 0, 0};
    addTriangle(v, idx, {-x,-y,-z}, {-x,y,-z}, {-x,-y,z}, leftN, c * 0.9f);
    // Right triangle
    addTriangle(v, idx, {x,-y,z}, {x,y,-z}, {x,-y,-z}, {1,0,0}, c * 0.9f);
}

void BlockMeshGenerator::generateWheel(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, float radius, float width, int seg, simd::float4 c)
{
    float hw = width * 0.5f;
    float innerR = radius * 0.3f;   // Moyeu
    float rimR = radius * 0.85f;    // Jante
    
    simd::float4 tireColor = {0.15f, 0.15f, 0.18f, 1.0f};
    simd::float4 rimColor = {0.7f, 0.72f, 0.75f, 1.0f};
    simd::float4 hubColor = c;
    
    // Pneu extérieur (tore simplifié)
    for (int i = 0; i < seg; i++) {
        float a0 = (float)i / seg * M_PI * 2.0f;
        float a1 = (float)(i + 1) / seg * M_PI * 2.0f;
        float c0 = cosf(a0), s0 = sinf(a0);
        float c1 = cosf(a1), s1 = sinf(a1);
        
        // Bande de roulement
        simd::float3 n0 = {c0, s0, 0};
        simd::float3 n1 = {c1, s1, 0};
        addQuad(v, idx,
            {radius*c0, radius*s0, -hw}, {radius*c1, radius*s1, -hw},
            {radius*c1, radius*s1, hw}, {radius*c0, radius*s0, hw},
            simd::normalize(n0 + n1), tireColor);
        
        // Flanc gauche
        addQuad(v, idx,
            {rimR*c0, rimR*s0, -hw}, {rimR*c1, rimR*s1, -hw},
            {radius*c1, radius*s1, -hw}, {radius*c0, radius*s0, -hw},
            {0, 0, -1}, tireColor * 0.9f);
        
        // Flanc droit
        addQuad(v, idx,
            {radius*c0, radius*s0, hw}, {radius*c1, radius*s1, hw},
            {rimR*c1, rimR*s1, hw}, {rimR*c0, rimR*s0, hw},
            {0, 0, 1}, tireColor * 0.9f);
    }
    
    // Jante avec rayons
    int spokes = 5;
    for (int i = 0; i < seg; i++) {
        float a0 = (float)i / seg * M_PI * 2.0f;
        float a1 = (float)(i + 1) / seg * M_PI * 2.0f;
        float c0 = cosf(a0), s0 = sinf(a0);
        float c1 = cosf(a1), s1 = sinf(a1);
        
        // Face jante gauche
        addQuad(v, idx,
            {innerR*c0, innerR*s0, -hw*0.8f}, {innerR*c1, innerR*s1, -hw*0.8f},
            {rimR*c1, rimR*s1, -hw}, {rimR*c0, rimR*s0, -hw},
            {0, 0, -1}, rimColor);
        
        // Face jante droite
        addQuad(v, idx,
            {rimR*c0, rimR*s0, hw}, {rimR*c1, rimR*s1, hw},
            {innerR*c1, innerR*s1, hw*0.8f}, {innerR*c0, innerR*s0, hw*0.8f},
            {0, 0, 1}, rimColor);
    }
    
    // Moyeu central
    for (int i = 0; i < seg; i++) {
        float a0 = (float)i / seg * M_PI * 2.0f;
        float a1 = (float)(i + 1) / seg * M_PI * 2.0f;
        addQuad(v, idx,
            {innerR*cosf(a0), innerR*sinf(a0), -hw*0.8f},
            {innerR*cosf(a1), innerR*sinf(a1), -hw*0.8f},
            {innerR*cosf(a1), innerR*sinf(a1), hw*0.8f},
            {innerR*cosf(a0), innerR*sinf(a0), hw*0.8f},
            {cosf(a0), sinf(a0), 0}, hubColor);
    }
}

void BlockMeshGenerator::generateCockpit(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float3 sz, simd::float4 c)
{
    float x = sz.x * 0.5f, y = sz.y * 0.5f, z = sz.z * 0.5f;
    
    simd::float4 frameColor = c;
    simd::float4 glassColor = {0.3f, 0.5f, 0.7f, 0.6f};
    
    // Base solide
    addQuad(v, idx, {-x,-y,z}, {x,-y,z}, {x,-y,-z}, {-x,-y,-z}, {0,-1,0}, frameColor * 0.8f);
    
    // Côtés avec angle
    float topInset = 0.2f;
    // Gauche
    addQuad(v, idx,
        {-x,-y,z}, {-x,-y,-z}, {-x+topInset,y,-z*0.7f}, {-x+topInset,y,z*0.5f},
        {-1,0.3f,0}, frameColor * 0.9f);
    // Droite
    addQuad(v, idx,
        {x,-y,-z}, {x,-y,z}, {x-topInset,y,z*0.5f}, {x-topInset,y,-z*0.7f},
        {1,0.3f,0}, frameColor * 0.9f);
    
    // Arrière (solide)
    addQuad(v, idx,
        {x,-y,-z}, {-x,-y,-z}, {-x+topInset,y,-z*0.7f}, {x-topInset,y,-z*0.7f},
        {0,0.2f,-1}, frameColor * 0.85f);
    
    // Verrière avant (verre incliné)
    addQuad(v, idx,
        {-x,-y,z}, {-x+topInset,y,z*0.5f}, {x-topInset,y,z*0.5f}, {x,-y,z},
        simd::normalize(simd::float3{0, 0.5f, 1}), glassColor);
    
    // Toit
    addQuad(v, idx,
        {-x+topInset,y,z*0.5f}, {-x+topInset,y,-z*0.7f},
        {x-topInset,y,-z*0.7f}, {x-topInset,y,z*0.5f},
        {0,1,0}, frameColor);
}

void BlockMeshGenerator::generateRobotHead(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float4 c)
{
    simd::float4 mainColor = c;
    simd::float4 accentColor = {0.2f, 0.25f, 0.3f, 1.0f};
    simd::float4 eyeColor = {0.0f, 0.8f, 1.0f, 1.0f};  // Cyan lumineux
    simd::float4 detailColor = c * 0.7f;
    
    // === CRÂNE PRINCIPAL (forme angulaire) ===
    float w = 0.45f, h = 0.4f, d = 0.4f;
    
    // Top (légèrement plus petit, crête centrale)
    addQuad(v, idx, {-w*0.8f,h,-d*0.7f}, {w*0.8f,h,-d*0.7f},
            {w*0.8f,h,d*0.6f}, {-w*0.8f,h,d*0.6f}, {0,1,0}, mainColor);
    
    // Crête centrale sur le dessus
    addQuad(v, idx, {-0.05f,h+0.08f,-d*0.5f}, {0.05f,h+0.08f,-d*0.5f},
            {0.05f,h+0.08f,d*0.3f}, {-0.05f,h+0.08f,d*0.3f}, {0,1,0}, accentColor);
    addQuad(v, idx, {-0.05f,h,-d*0.5f}, {-0.05f,h+0.08f,-d*0.5f},
            {-0.05f,h+0.08f,d*0.3f}, {-0.05f,h,d*0.3f}, {-1,0,0}, accentColor*0.9f);
    addQuad(v, idx, {0.05f,h+0.08f,-d*0.5f}, {0.05f,h,-d*0.5f},
            {0.05f,h,d*0.3f}, {0.05f,h+0.08f,d*0.3f}, {1,0,0}, accentColor*0.9f);
    
    // Front face (visière angulaire)
    float visorY = 0.15f;
    addQuad(v, idx, {-w,visorY,d}, {w,visorY,d}, {w*0.8f,h,d*0.6f}, {-w*0.8f,h,d*0.6f},
            simd::normalize(simd::float3{0,0.3f,1}), mainColor * 0.95f);
    
    // === VISIÈRE / YEUX ===
    float eyeW = 0.15f, eyeH = 0.06f;
    float eyeY = visorY + 0.08f;
    // Œil gauche
    addQuad(v, idx, {-w*0.7f,eyeY-eyeH,d*0.85f}, {-w*0.7f+eyeW*2,eyeY-eyeH,d*0.85f},
            {-w*0.7f+eyeW*2,eyeY+eyeH,d*0.85f}, {-w*0.7f,eyeY+eyeH,d*0.85f},
            {0,0,1}, eyeColor);
    // Œil droit
    addQuad(v, idx, {w*0.7f-eyeW*2,eyeY-eyeH,d*0.85f}, {w*0.7f,eyeY-eyeH,d*0.85f},
            {w*0.7f,eyeY+eyeH,d*0.85f}, {w*0.7f-eyeW*2,eyeY+eyeH,d*0.85f},
            {0,0,1}, eyeColor);
    
    // === MÂCHOIRE / MENTON ===
    float jawY = -0.25f;
    // Front jaw
    addQuad(v, idx, {-w*0.6f,jawY,d*0.8f}, {w*0.6f,jawY,d*0.8f},
            {w,visorY,d}, {-w,visorY,d}, {0,-0.2f,1}, mainColor * 0.85f);
    
    // Grille de ventilation sur la "bouche"
    for (int i = 0; i < 4; i++) {
        float slotX = -0.2f + i * 0.13f;
        addQuad(v, idx,
            {slotX, jawY+0.02f, d*0.82f}, {slotX+0.08f, jawY+0.02f, d*0.82f},
            {slotX+0.08f, visorY-0.05f, d*0.95f}, {slotX, visorY-0.05f, d*0.95f},
            {0,0,1}, accentColor);
    }
    
    // === CÔTÉS (joues/audio receptors) ===
    // Gauche
    addQuad(v, idx, {-w,visorY,d}, {-w*0.8f,h,d*0.6f}, {-w*0.8f,h,-d*0.7f}, {-w,visorY,-d*0.5f},
            {-1,0.2f,0}, mainColor * 0.9f);
    addQuad(v, idx, {-w,jawY,-d*0.3f}, {-w,visorY,-d*0.5f}, {-w*0.8f,h,-d*0.7f}, {-w*0.7f,jawY,-d*0.5f},
            {-1,0,0}, mainColor * 0.88f);
    addQuad(v, idx, {-w*0.6f,jawY,d*0.8f}, {-w,visorY,d}, {-w,visorY,-d*0.5f}, {-w,jawY,-d*0.3f},
            {-0.8f,-0.2f,0.3f}, mainColor * 0.85f);
    
    // Droite (miroir)
    addQuad(v, idx, {w*0.8f,h,d*0.6f}, {w,visorY,d}, {w,visorY,-d*0.5f}, {w*0.8f,h,-d*0.7f},
            {1,0.2f,0}, mainColor * 0.9f);
    addQuad(v, idx, {w,visorY,-d*0.5f}, {w,jawY,-d*0.3f}, {w*0.7f,jawY,-d*0.5f}, {w*0.8f,h,-d*0.7f},
            {1,0,0}, mainColor * 0.88f);
    addQuad(v, idx, {w,visorY,d}, {w*0.6f,jawY,d*0.8f}, {w,jawY,-d*0.3f}, {w,visorY,-d*0.5f},
            {0.8f,-0.2f,0.3f}, mainColor * 0.85f);
    
    // Audio receptors (cercles sur les côtés)
    int audioSeg = 8;
    float audioR = 0.08f;
    simd::float3 audioLPos = {-w-0.01f, visorY+0.05f, 0.0f};
    simd::float3 audioRPos = {w+0.01f, visorY+0.05f, 0.0f};
    for (int i = 0; i < audioSeg; i++) {
        float a0 = (float)i / audioSeg * M_PI * 2.0f;
        float a1 = (float)(i+1) / audioSeg * M_PI * 2.0f;
        // Gauche
        addTriangle(v, idx, audioLPos,
            audioLPos + simd::float3{0, audioR*cosf(a0), audioR*sinf(a0)},
            audioLPos + simd::float3{0, audioR*cosf(a1), audioR*sinf(a1)},
            {-1,0,0}, detailColor);
        // Droite
        addTriangle(v, idx, audioRPos,
            audioRPos + simd::float3{0, audioR*cosf(a1), audioR*sinf(a1)},
            audioRPos + simd::float3{0, audioR*cosf(a0), audioR*sinf(a0)},
            {1,0,0}, detailColor);
    }
    
    // === ARRIÈRE DU CRÂNE ===
    addQuad(v, idx, {w*0.8f,h,-d*0.7f}, {-w*0.8f,h,-d*0.7f},
            {-w*0.7f,jawY,-d*0.5f}, {w*0.7f,jawY,-d*0.5f}, {0,0.1f,-1}, mainColor * 0.8f);
    
    // Connecteur cou
    addQuad(v, idx, {-0.15f,jawY,-d*0.3f}, {0.15f,jawY,-d*0.3f},
            {0.15f,jawY,d*0.5f}, {-0.15f,jawY,d*0.5f}, {0,-1,0}, accentColor);
    
    // === DESSOUS ===
    addQuad(v, idx, {-w*0.6f,jawY,d*0.8f}, {-w,jawY,-d*0.3f},
            {w,jawY,-d*0.3f}, {w*0.6f,jawY,d*0.8f}, {0,-1,0}, mainColor * 0.75f);
}

void BlockMeshGenerator::generateThruster(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, float radius, float length, simd::float4 c)
{
    int seg = 16;
    float innerR = radius * 0.6f;
    simd::float4 nozzleColor = {0.3f, 0.32f, 0.35f, 1.0f};
    simd::float4 glowColor = {1.0f, 0.5f, 0.1f, 1.0f};
    
    // Corps externe
    for (int i = 0; i < seg; i++) {
        float a0 = (float)i / seg * M_PI * 2.0f;
        float a1 = (float)(i+1) / seg * M_PI * 2.0f;
        float c0 = cosf(a0), s0 = sinf(a0), c1 = cosf(a1), s1 = sinf(a1);
        
        // Cône extérieur
        addQuad(v, idx,
            {radius*0.8f*c0, radius*0.8f*s0, 0},
            {radius*0.8f*c1, radius*0.8f*s1, 0},
            {radius*c1, radius*s1, -length},
            {radius*c0, radius*s0, -length},
            {c0, s0, 0.3f}, c);
        
        // Bord arrière
        addQuad(v, idx,
            {radius*c0, radius*s0, -length},
            {radius*c1, radius*s1, -length},
            {innerR*c1, innerR*s1, -length},
            {innerR*c0, innerR*s0, -length},
            {0, 0, -1}, nozzleColor);
        
        // Intérieur (glow)
        addQuad(v, idx,
            {innerR*c1, innerR*s1, -length},
            {innerR*c0, innerR*s0, -length},
            {innerR*0.5f*c0, innerR*0.5f*s0, -length*0.3f},
            {innerR*0.5f*c1, innerR*0.5f*s1, -length*0.3f},
            {-c0, -s0, 0}, glowColor);
    }
    
    // Cap avant
    for (int i = 0; i < seg; i++)
    {
        float a0 = (float)i / seg * M_PI * 2.0f;
        float a1 = (float)(i+1) / seg * M_PI * 2.0f;
        addTriangle(v, idx, { 0, 0, 0 }, { radius * 0.8f * cosf(a1), radius * 0.8f * sinf(a1), 0 }, { radius * 0.8f * cosf(a0), radius * 0.8f * sinf(a0), 0 }, { 0, 0, 1 }, c * 0.9f);
    }
}

void BlockMeshGenerator::generateIcosphere(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float4 c)
{
    uint16_t base = (uint16_t)v.size();
    v.push_back({{0.00000f,-0.50000f,-0.00000f},{0.1024f,-0.9435f,0.3151f},{0.1818f,0.0000f},c});
    v.push_back({{0.21266f,-0.42533f,0.15451f},{0.1024f,-0.9435f,0.3151f},{0.2273f,0.0787f},c});
    v.push_back({{-0.08123f,-0.42533f,0.25000f},{0.1024f,-0.9435f,0.3151f},{0.1364f,0.0787f},c});
    v.push_back({{0.36180f,-0.22361f,0.26286f},{0.7002f,-0.6617f,0.2680f},{0.2727f,0.1575f},c});
    v.push_back({{0.21266f,-0.42533f,0.15451f},{0.7002f,-0.6617f,0.2680f},{0.3182f,0.0787f},c});
    v.push_back({{0.42532f,-0.26287f,-0.00000f},{0.7002f,-0.6617f,0.2680f},{0.3636f,0.1575f},c});
    v.push_back({{0.00000f,-0.50000f,-0.00000f},{-0.2680f,-0.9435f,0.1947f},{0.9091f,0.0000f},c});
    v.push_back({{-0.08123f,-0.42533f,0.25000f},{-0.2680f,-0.9435f,0.1947f},{0.9545f,0.0787f},c});
    v.push_back({{-0.26286f,-0.42533f,-0.00000f},{-0.2680f,-0.9435f,0.1947f},{0.8636f,0.0787f},c});
    v.push_back({{0.00000f,-0.50000f,-0.00000f},{-0.2680f,-0.9435f,-0.1947f},{0.7273f,0.0000f},c});
    v.push_back({{-0.26286f,-0.42533f,-0.00000f},{-0.2680f,-0.9435f,-0.1947f},{0.7727f,0.0787f},c});
    v.push_back({{-0.08123f,-0.42533f,-0.25000f},{-0.2680f,-0.9435f,-0.1947f},{0.6818f,0.0787f},c});
    v.push_back({{0.00000f,-0.50000f,-0.00000f},{0.1024f,-0.9435f,-0.3151f},{0.5455f,0.0000f},c});
    v.push_back({{-0.08123f,-0.42533f,-0.25000f},{0.1024f,-0.9435f,-0.3151f},{0.5909f,0.0787f},c});
    v.push_back({{0.21266f,-0.42533f,-0.15451f},{0.1024f,-0.9435f,-0.3151f},{0.5000f,0.0787f},c});
    v.push_back({{0.36180f,-0.22361f,0.26286f},{0.9050f,-0.3304f,0.2680f},{0.2727f,0.1575f},c});
    v.push_back({{0.42532f,-0.26287f,-0.00000f},{0.9050f,-0.3304f,0.2680f},{0.3636f,0.1575f},c});
    v.push_back({{0.47553f,0.00000f,0.15451f},{0.9050f,-0.3304f,0.2680f},{0.3182f,0.2362f},c});
    v.push_back({{-0.13819f,-0.22361f,0.42532f},{0.0247f,-0.3304f,0.9435f},{0.0909f,0.1575f},c});
    v.push_back({{0.13143f,-0.26287f,0.40451f},{0.0247f,-0.3304f,0.9435f},{0.1818f,0.1575f},c});
    v.push_back({{0.00000f,0.00000f,0.50000f},{0.0247f,-0.3304f,0.9435f},{0.1364f,0.2362f},c});
    v.push_back({{-0.44721f,-0.22361f,-0.00000f},{-0.8897f,-0.3304f,0.3151f},{0.8182f,0.1575f},c});
    v.push_back({{-0.34409f,-0.26287f,0.25000f},{-0.8897f,-0.3304f,0.3151f},{0.9091f,0.1575f},c});
    v.push_back({{-0.47553f,0.00000f,0.15451f},{-0.8897f,-0.3304f,0.3151f},{0.8636f,0.2362f},c});
    v.push_back({{-0.13819f,-0.22361f,-0.42532f},{-0.5746f,-0.3304f,-0.7488f},{0.6364f,0.1575f},c});
    v.push_back({{-0.34409f,-0.26287f,-0.25000f},{-0.5746f,-0.3304f,-0.7488f},{0.7273f,0.1575f},c});
    v.push_back({{-0.29389f,0.00000f,-0.40451f},{-0.5746f,-0.3304f,-0.7488f},{0.6818f,0.2362f},c});
    v.push_back({{0.36180f,-0.22361f,-0.26286f},{0.5346f,-0.3304f,-0.7779f},{0.4545f,0.1575f},c});
    v.push_back({{0.13143f,-0.26287f,-0.40451f},{0.5346f,-0.3304f,-0.7779f},{0.5455f,0.1575f},c});
    v.push_back({{0.29389f,0.00000f,-0.40451f},{0.5346f,-0.3304f,-0.7779f},{0.5000f,0.2362f},c});
    v.push_back({{0.36180f,-0.22361f,0.26286f},{0.8026f,-0.1256f,0.5831f},{0.2727f,0.1575f},c});
    v.push_back({{0.47553f,0.00000f,0.15451f},{0.8026f,-0.1256f,0.5831f},{0.3182f,0.2362f},c});
    v.push_back({{0.29389f,0.00000f,0.40451f},{0.8026f,-0.1256f,0.5831f},{0.2273f,0.2362f},c});
    v.push_back({{-0.13819f,-0.22361f,0.42532f},{-0.3066f,-0.1256f,0.9435f},{0.0909f,0.1575f},c});
    v.push_back({{0.00000f,0.00000f,0.50000f},{-0.3066f,-0.1256f,0.9435f},{0.1364f,0.2362f},c});
    v.push_back({{-0.29389f,0.00000f,0.40451f},{-0.3066f,-0.1256f,0.9435f},{0.0455f,0.2362f},c});
    v.push_back({{-0.44721f,-0.22361f,-0.00000f},{-0.9921f,-0.1256f,0.0000f},{0.8182f,0.1575f},c});
    v.push_back({{-0.47553f,0.00000f,0.15451f},{-0.9921f,-0.1256f,0.0000f},{0.8636f,0.2362f},c});
    v.push_back({{-0.47553f,0.00000f,-0.15451f},{-0.9921f,-0.1256f,0.0000f},{0.7727f,0.2362f},c});
    v.push_back({{-0.13819f,-0.22361f,-0.42532f},{-0.3066f,-0.1256f,-0.9435f},{0.6364f,0.1575f},c});
    v.push_back({{-0.29389f,0.00000f,-0.40451f},{-0.3066f,-0.1256f,-0.9435f},{0.6818f,0.2362f},c});
    v.push_back({{0.00000f,0.00000f,-0.50000f},{-0.3066f,-0.1256f,-0.9435f},{0.5909f,0.2362f},c});
    v.push_back({{0.36180f,-0.22361f,-0.26286f},{0.8026f,-0.1256f,-0.5831f},{0.4545f,0.1575f},c});
    v.push_back({{0.29389f,0.00000f,-0.40451f},{0.8026f,-0.1256f,-0.5831f},{0.5000f,0.2362f},c});
    v.push_back({{0.47553f,0.00000f,-0.15451f},{0.8026f,-0.1256f,-0.5831f},{0.4091f,0.2362f},c});
    v.push_back({{0.13819f,0.22361f,0.42532f},{0.4089f,0.6617f,0.6284f},{0.1818f,0.3149f},c});
    v.push_back({{0.34409f,0.26287f,0.25000f},{0.4089f,0.6617f,0.6284f},{0.2727f,0.3149f},c});
    v.push_back({{0.08123f,0.42533f,0.25000f},{0.4089f,0.6617f,0.6284f},{0.2273f,0.3937f},c});
    v.push_back({{-0.36180f,0.22361f,0.26286f},{-0.4713f,0.6617f,0.5831f},{0.0000f,0.3149f},c});
    v.push_back({{-0.13143f,0.26287f,0.40451f},{-0.4713f,0.6617f,0.5831f},{0.0909f,0.3149f},c});
    v.push_back({{-0.21266f,0.42533f,0.15451f},{-0.4713f,0.6617f,0.5831f},{0.0455f,0.3937f},c});
    v.push_back({{-0.36180f,0.22361f,-0.26286f},{-0.7002f,0.6617f,-0.2680f},{0.7273f,0.3149f},c});
    v.push_back({{-0.42532f,0.26287f,-0.00000f},{-0.7002f,0.6617f,-0.2680f},{0.8182f,0.3149f},c});
    v.push_back({{-0.21266f,0.42533f,-0.15451f},{-0.7002f,0.6617f,-0.2680f},{0.7727f,0.3937f},c});
    v.push_back({{0.13819f,0.22361f,-0.42532f},{0.0385f,0.6617f,-0.7488f},{0.5455f,0.3149f},c});
    v.push_back({{-0.13143f,0.26287f,-0.40451f},{0.0385f,0.6617f,-0.7488f},{0.6364f,0.3149f},c});
    v.push_back({{0.08123f,0.42533f,-0.25000f},{0.0385f,0.6617f,-0.7488f},{0.5909f,0.3937f},c});
    v.push_back({{0.44721f,0.22361f,-0.00000f},{0.7240f,0.6617f,-0.1947f},{0.3636f,0.3149f},c});
    v.push_back({{0.34409f,0.26287f,-0.25000f},{0.7240f,0.6617f,-0.1947f},{0.4545f,0.3149f},c});
    v.push_back({{0.26286f,0.42533f,-0.00000f},{0.7240f,0.6617f,-0.1947f},{0.4091f,0.3937f},c});
    v.push_back({{0.26286f,0.42533f,-0.00000f},{0.2680f,0.9435f,-0.1947f},{0.4091f,0.3937f},c});
    v.push_back({{0.08123f,0.42533f,-0.25000f},{0.2680f,0.9435f,-0.1947f},{0.5000f,0.3937f},c});
    v.push_back({{0.00000f,0.50000f,-0.00000f},{0.2680f,0.9435f,-0.1947f},{0.4545f,0.4724f},c});
    v.push_back({{0.26286f,0.42533f,-0.00000f},{0.4911f,0.7947f,-0.3568f},{0.4091f,0.3937f},c});
    v.push_back({{0.34409f,0.26287f,-0.25000f},{0.4911f,0.7947f,-0.3568f},{0.4545f,0.3149f},c});
    v.push_back({{0.08123f,0.42533f,-0.25000f},{0.4911f,0.7947f,-0.3568f},{0.5000f,0.3937f},c});
    v.push_back({{0.34409f,0.26287f,-0.25000f},{0.4089f,0.6617f,-0.6284f},{0.4545f,0.3149f},c});
    v.push_back({{0.13819f,0.22361f,-0.42532f},{0.4089f,0.6617f,-0.6284f},{0.5455f,0.3149f},c});
    v.push_back({{0.08123f,0.42533f,-0.25000f},{0.4089f,0.6617f,-0.6284f},{0.5000f,0.3937f},c});
    v.push_back({{0.08123f,0.42533f,-0.25000f},{-0.1024f,0.9435f,-0.3151f},{0.5909f,0.3937f},c});
    v.push_back({{-0.21266f,0.42533f,-0.15451f},{-0.1024f,0.9435f,-0.3151f},{0.6818f,0.3937f},c});
    v.push_back({{0.00000f,0.50000f,-0.00000f},{-0.1024f,0.9435f,-0.3151f},{0.6364f,0.4724f},c});
    v.push_back({{0.08123f,0.42533f,-0.25000f},{-0.1876f,0.7947f,-0.5773f},{0.5909f,0.3937f},c});
    v.push_back({{-0.13143f,0.26287f,-0.40451f},{-0.1876f,0.7947f,-0.5773f},{0.6364f,0.3149f},c});
    v.push_back({{-0.21266f,0.42533f,-0.15451f},{-0.1876f,0.7947f,-0.5773f},{0.6818f,0.3937f},c});
    v.push_back({{-0.13143f,0.26287f,-0.40451f},{-0.4713f,0.6617f,-0.5831f},{0.6364f,0.3149f},c});
    v.push_back({{-0.36180f,0.22361f,-0.26286f},{-0.4713f,0.6617f,-0.5831f},{0.7273f,0.3149f},c});
    v.push_back({{-0.21266f,0.42533f,-0.15451f},{-0.4713f,0.6617f,-0.5831f},{0.6818f,0.3937f},c});
    v.push_back({{-0.21266f,0.42533f,-0.15451f},{-0.3313f,0.9435f,-0.0000f},{0.7727f,0.3937f},c});
    v.push_back({{-0.21266f,0.42533f,0.15451f},{-0.3313f,0.9435f,-0.0000f},{0.8636f,0.3937f},c});
    v.push_back({{0.00000f,0.50000f,-0.00000f},{-0.3313f,0.9435f,-0.0000f},{0.8182f,0.4724f},c});
    v.push_back({{-0.21266f,0.42533f,-0.15451f},{-0.6071f,0.7947f,-0.0000f},{0.7727f,0.3937f},c});
    v.push_back({{-0.42532f,0.26287f,-0.00000f},{-0.6071f,0.7947f,-0.0000f},{0.8182f,0.3149f},c});
    v.push_back({{-0.21266f,0.42533f,0.15451f},{-0.6071f,0.7947f,-0.0000f},{0.8636f,0.3937f},c});
    v.push_back({{-0.42532f,0.26287f,-0.00000f},{-0.7002f,0.6617f,0.2680f},{0.8182f,0.3149f},c});
    v.push_back({{-0.36180f,0.22361f,0.26286f},{-0.7002f,0.6617f,0.2680f},{0.9091f,0.3149f},c});
    v.push_back({{-0.21266f,0.42533f,0.15451f},{-0.7002f,0.6617f,0.2680f},{0.8636f,0.3937f},c});
    v.push_back({{-0.21266f,0.42533f,0.15451f},{-0.1024f,0.9435f,0.3151f},{0.0455f,0.3937f},c});
    v.push_back({{0.08123f,0.42533f,0.25000f},{-0.1024f,0.9435f,0.3151f},{0.1364f,0.3937f},c});
    v.push_back({{0.00000f,0.50000f,-0.00000f},{-0.1024f,0.9435f,0.3151f},{0.0909f,0.4724f},c});
    v.push_back({{-0.21266f,0.42533f,0.15451f},{-0.1876f,0.7947f,0.5773f},{0.0455f,0.3937f},c});
    v.push_back({{-0.13143f,0.26287f,0.40451f},{-0.1876f,0.7947f,0.5773f},{0.0909f,0.3149f},c});
    v.push_back({{0.08123f,0.42533f,0.25000f},{-0.1876f,0.7947f,0.5773f},{0.1364f,0.3937f},c});
    v.push_back({{-0.13143f,0.26287f,0.40451f},{0.0385f,0.6617f,0.7488f},{0.0909f,0.3149f},c});
    v.push_back({{0.13819f,0.22361f,0.42532f},{0.0385f,0.6617f,0.7488f},{0.1818f,0.3149f},c});
    v.push_back({{0.08123f,0.42533f,0.25000f},{0.0385f,0.6617f,0.7488f},{0.1364f,0.3937f},c});
    v.push_back({{0.08123f,0.42533f,0.25000f},{0.2680f,0.9435f,0.1947f},{0.2273f,0.3937f},c});
    v.push_back({{0.26286f,0.42533f,-0.00000f},{0.2680f,0.9435f,0.1947f},{0.3182f,0.3937f},c});
    v.push_back({{0.00000f,0.50000f,-0.00000f},{0.2680f,0.9435f,0.1947f},{0.2727f,0.4724f},c});
    v.push_back({{0.08123f,0.42533f,0.25000f},{0.4911f,0.7947f,0.3568f},{0.2273f,0.3937f},c});
    v.push_back({{0.34409f,0.26287f,0.25000f},{0.4911f,0.7947f,0.3568f},{0.2727f,0.3149f},c});
    v.push_back({{0.26286f,0.42533f,-0.00000f},{0.4911f,0.7947f,0.3568f},{0.3182f,0.3937f},c});
    v.push_back({{0.34409f,0.26287f,0.25000f},{0.7240f,0.6617f,0.1947f},{0.2727f,0.3149f},c});
    v.push_back({{0.44721f,0.22361f,-0.00000f},{0.7240f,0.6617f,0.1947f},{0.3636f,0.3149f},c});
    v.push_back({{0.26286f,0.42533f,-0.00000f},{0.7240f,0.6617f,0.1947f},{0.3182f,0.3937f},c});
    v.push_back({{0.47553f,0.00000f,-0.15451f},{0.7947f,0.1876f,-0.5773f},{0.4091f,0.2362f},c});
    v.push_back({{0.29389f,0.00000f,-0.40451f},{0.7947f,0.1876f,-0.5773f},{0.5000f,0.2362f},c});
    v.push_back({{0.34409f,0.26287f,-0.25000f},{0.7947f,0.1876f,-0.5773f},{0.4545f,0.3149f},c});
    v.push_back({{0.29389f,0.00000f,-0.40451f},{0.5746f,0.3304f,-0.7488f},{0.5000f,0.2362f},c});
    v.push_back({{0.13819f,0.22361f,-0.42532f},{0.5746f,0.3304f,-0.7488f},{0.5455f,0.3149f},c});
    v.push_back({{0.34409f,0.26287f,-0.25000f},{0.5746f,0.3304f,-0.7488f},{0.4545f,0.3149f},c});
    v.push_back({{0.00000f,0.00000f,-0.50000f},{-0.0247f,0.3304f,-0.9435f},{0.5909f,0.2362f},c});
    v.push_back({{-0.13143f,0.26287f,-0.40451f},{-0.0247f,0.3304f,-0.9435f},{0.6364f,0.3149f},c});
    v.push_back({{0.13819f,0.22361f,-0.42532f},{-0.0247f,0.3304f,-0.9435f},{0.5455f,0.3149f},c});
    v.push_back({{0.00000f,0.00000f,-0.50000f},{-0.3035f,0.1876f,-0.9342f},{0.5909f,0.2362f},c});
    v.push_back({{-0.29389f,0.00000f,-0.40451f},{-0.3035f,0.1876f,-0.9342f},{0.6818f,0.2362f},c});
    v.push_back({{-0.13143f,0.26287f,-0.40451f},{-0.3035f,0.1876f,-0.9342f},{0.6364f,0.3149f},c});
    v.push_back({{-0.29389f,0.00000f,-0.40451f},{-0.5346f,0.3304f,-0.7779f},{0.6818f,0.2362f},c});
    v.push_back({{-0.36180f,0.22361f,-0.26286f},{-0.5346f,0.3304f,-0.7779f},{0.7273f,0.3149f},c});
    v.push_back({{-0.13143f,0.26287f,-0.40451f},{-0.5346f,0.3304f,-0.7779f},{0.6364f,0.3149f},c});
    v.push_back({{-0.47553f,0.00000f,-0.15451f},{-0.9050f,0.3304f,-0.2680f},{0.7727f,0.2362f},c});
    v.push_back({{-0.42532f,0.26287f,-0.00000f},{-0.9050f,0.3304f,-0.2680f},{0.8182f,0.3149f},c});
    v.push_back({{-0.36180f,0.22361f,-0.26286f},{-0.9050f,0.3304f,-0.2680f},{0.7273f,0.3149f},c});
    v.push_back({{-0.47553f,0.00000f,-0.15451f},{-0.9822f,0.1876f,-0.0000f},{0.7727f,0.2362f},c});
    v.push_back({{-0.47553f,0.00000f,0.15451f},{-0.9822f,0.1876f,-0.0000f},{0.8636f,0.2362f},c});
    v.push_back({{-0.42532f,0.26287f,-0.00000f},{-0.9822f,0.1876f,-0.0000f},{0.8182f,0.3149f},c});
    v.push_back({{-0.47553f,0.00000f,0.15451f},{-0.9050f,0.3304f,0.2680f},{0.8636f,0.2362f},c});
    v.push_back({{-0.36180f,0.22361f,0.26286f},{-0.9050f,0.3304f,0.2680f},{0.9091f,0.3149f},c});
    v.push_back({{-0.42532f,0.26287f,-0.00000f},{-0.9050f,0.3304f,0.2680f},{0.8182f,0.3149f},c});
    v.push_back({{-0.29389f,0.00000f,0.40451f},{-0.5346f,0.3304f,0.7779f},{0.0455f,0.2362f},c});
    v.push_back({{-0.13143f,0.26287f,0.40451f},{-0.5346f,0.3304f,0.7779f},{0.0909f,0.3149f},c});
    v.push_back({{-0.36180f,0.22361f,0.26286f},{-0.5346f,0.3304f,0.7779f},{0.0000f,0.3149f},c});
    v.push_back({{-0.29389f,0.00000f,0.40451f},{-0.3035f,0.1876f,0.9342f},{0.0455f,0.2362f},c});
    v.push_back({{0.00000f,0.00000f,0.50000f},{-0.3035f,0.1876f,0.9342f},{0.1364f,0.2362f},c});
    v.push_back({{-0.13143f,0.26287f,0.40451f},{-0.3035f,0.1876f,0.9342f},{0.0909f,0.3149f},c});
    v.push_back({{0.00000f,0.00000f,0.50000f},{-0.0247f,0.3304f,0.9435f},{0.1364f,0.2362f},c});
    v.push_back({{0.13819f,0.22361f,0.42532f},{-0.0247f,0.3304f,0.9435f},{0.1818f,0.3149f},c});
    v.push_back({{-0.13143f,0.26287f,0.40451f},{-0.0247f,0.3304f,0.9435f},{0.0909f,0.3149f},c});
    v.push_back({{0.29389f,0.00000f,0.40451f},{0.5746f,0.3304f,0.7488f},{0.2273f,0.2362f},c});
    v.push_back({{0.34409f,0.26287f,0.25000f},{0.5746f,0.3304f,0.7488f},{0.2727f,0.3149f},c});
    v.push_back({{0.13819f,0.22361f,0.42532f},{0.5746f,0.3304f,0.7488f},{0.1818f,0.3149f},c});
    v.push_back({{0.29389f,0.00000f,0.40451f},{0.7947f,0.1876f,0.5773f},{0.2273f,0.2362f},c});
    v.push_back({{0.47553f,0.00000f,0.15451f},{0.7947f,0.1876f,0.5773f},{0.3182f,0.2362f},c});
    v.push_back({{0.34409f,0.26287f,0.25000f},{0.7947f,0.1876f,0.5773f},{0.2727f,0.3149f},c});
    v.push_back({{0.29389f,0.00000f,-0.40451f},{0.3066f,0.1256f,-0.9435f},{0.5000f,0.2362f},c});
    v.push_back({{0.00000f,0.00000f,-0.50000f},{0.3066f,0.1256f,-0.9435f},{0.5909f,0.2362f},c});
    v.push_back({{0.13819f,0.22361f,-0.42532f},{0.3066f,0.1256f,-0.9435f},{0.5455f,0.3149f},c});
    v.push_back({{0.29389f,0.00000f,-0.40451f},{0.3035f,-0.1876f,-0.9342f},{0.5000f,0.2362f},c});
    v.push_back({{0.13143f,-0.26287f,-0.40451f},{0.3035f,-0.1876f,-0.9342f},{0.5455f,0.1575f},c});
    v.push_back({{0.00000f,0.00000f,-0.50000f},{0.3035f,-0.1876f,-0.9342f},{0.5909f,0.2362f},c});
    v.push_back({{0.13143f,-0.26287f,-0.40451f},{0.0247f,-0.3304f,-0.9435f},{0.5455f,0.1575f},c});
    v.push_back({{-0.13819f,-0.22361f,-0.42532f},{0.0247f,-0.3304f,-0.9435f},{0.6364f,0.1575f},c});
    v.push_back({{0.00000f,0.00000f,-0.50000f},{0.0247f,-0.3304f,-0.9435f},{0.5909f,0.2362f},c});
    v.push_back({{-0.29389f,0.00000f,-0.40451f},{-0.8026f,0.1256f,-0.5831f},{0.6818f,0.2362f},c});
    v.push_back({{-0.47553f,0.00000f,-0.15451f},{-0.8026f,0.1256f,-0.5831f},{0.7727f,0.2362f},c});
    v.push_back({{-0.36180f,0.22361f,-0.26286f},{-0.8026f,0.1256f,-0.5831f},{0.7273f,0.3149f},c});
    v.push_back({{-0.29389f,0.00000f,-0.40451f},{-0.7947f,-0.1876f,-0.5773f},{0.6818f,0.2362f},c});
    v.push_back({{-0.34409f,-0.26287f,-0.25000f},{-0.7947f,-0.1876f,-0.5773f},{0.7273f,0.1575f},c});
    v.push_back({{-0.47553f,0.00000f,-0.15451f},{-0.7947f,-0.1876f,-0.5773f},{0.7727f,0.2362f},c});
    v.push_back({{-0.34409f,-0.26287f,-0.25000f},{-0.8897f,-0.3304f,-0.3151f},{0.7273f,0.1575f},c});
    v.push_back({{-0.44721f,-0.22361f,-0.00000f},{-0.8897f,-0.3304f,-0.3151f},{0.8182f,0.1575f},c});
    v.push_back({{-0.47553f,0.00000f,-0.15451f},{-0.8897f,-0.3304f,-0.3151f},{0.7727f,0.2362f},c});
    v.push_back({{-0.47553f,0.00000f,0.15451f},{-0.8026f,0.1256f,0.5831f},{0.8636f,0.2362f},c});
    v.push_back({{-0.29389f,0.00000f,0.40451f},{-0.8026f,0.1256f,0.5831f},{0.9545f,0.2362f},c});
    v.push_back({{-0.36180f,0.22361f,0.26286f},{-0.8026f,0.1256f,0.5831f},{0.9091f,0.3149f},c});
    v.push_back({{-0.47553f,0.00000f,0.15451f},{-0.7947f,-0.1876f,0.5773f},{0.8636f,0.2362f},c});
    v.push_back({{-0.34409f,-0.26287f,0.25000f},{-0.7947f,-0.1876f,0.5773f},{0.9091f,0.1575f},c});
    v.push_back({{-0.29389f,0.00000f,0.40451f},{-0.7947f,-0.1876f,0.5773f},{0.9545f,0.2362f},c});
    v.push_back({{-0.34409f,-0.26287f,0.25000f},{-0.5746f,-0.3304f,0.7488f},{0.9091f,0.1575f},c});
    v.push_back({{-0.13819f,-0.22361f,0.42532f},{-0.5746f,-0.3304f,0.7488f},{1.0000f,0.1575f},c});
    v.push_back({{-0.29389f,0.00000f,0.40451f},{-0.5746f,-0.3304f,0.7488f},{0.9545f,0.2362f},c});
    v.push_back({{0.00000f,0.00000f,0.50000f},{0.3066f,0.1256f,0.9435f},{0.1364f,0.2362f},c});
    v.push_back({{0.29389f,0.00000f,0.40451f},{0.3066f,0.1256f,0.9435f},{0.2273f,0.2362f},c});
    v.push_back({{0.13819f,0.22361f,0.42532f},{0.3066f,0.1256f,0.9435f},{0.1818f,0.3149f},c});
    v.push_back({{0.00000f,0.00000f,0.50000f},{0.3035f,-0.1876f,0.9342f},{0.1364f,0.2362f},c});
    v.push_back({{0.13143f,-0.26287f,0.40451f},{0.3035f,-0.1876f,0.9342f},{0.1818f,0.1575f},c});
    v.push_back({{0.29389f,0.00000f,0.40451f},{0.3035f,-0.1876f,0.9342f},{0.2273f,0.2362f},c});
    v.push_back({{0.13143f,-0.26287f,0.40451f},{0.5346f,-0.3304f,0.7779f},{0.1818f,0.1575f},c});
    v.push_back({{0.36180f,-0.22361f,0.26286f},{0.5346f,-0.3304f,0.7779f},{0.2727f,0.1575f},c});
    v.push_back({{0.29389f,0.00000f,0.40451f},{0.5346f,-0.3304f,0.7779f},{0.2273f,0.2362f},c});
    v.push_back({{0.47553f,0.00000f,0.15451f},{0.9921f,0.1256f,-0.0000f},{0.3182f,0.2362f},c});
    v.push_back({{0.47553f,0.00000f,-0.15451f},{0.9921f,0.1256f,-0.0000f},{0.4091f,0.2362f},c});
    v.push_back({{0.44721f,0.22361f,-0.00000f},{0.9921f,0.1256f,-0.0000f},{0.3636f,0.3149f},c});
    v.push_back({{0.47553f,0.00000f,0.15451f},{0.9822f,-0.1876f,-0.0000f},{0.3182f,0.2362f},c});
    v.push_back({{0.42532f,-0.26287f,-0.00000f},{0.9822f,-0.1876f,-0.0000f},{0.3636f,0.1575f},c});
    v.push_back({{0.47553f,0.00000f,-0.15451f},{0.9822f,-0.1876f,-0.0000f},{0.4091f,0.2362f},c});
    v.push_back({{0.42532f,-0.26287f,-0.00000f},{0.9050f,-0.3304f,-0.2680f},{0.3636f,0.1575f},c});
    v.push_back({{0.36180f,-0.22361f,-0.26286f},{0.9050f,-0.3304f,-0.2680f},{0.4545f,0.1575f},c});
    v.push_back({{0.47553f,0.00000f,-0.15451f},{0.9050f,-0.3304f,-0.2680f},{0.4091f,0.2362f},c});
    v.push_back({{0.21266f,-0.42533f,-0.15451f},{0.4713f,-0.6617f,-0.5831f},{0.5000f,0.0787f},c});
    v.push_back({{0.13143f,-0.26287f,-0.40451f},{0.4713f,-0.6617f,-0.5831f},{0.5455f,0.1575f},c});
    v.push_back({{0.36180f,-0.22361f,-0.26286f},{0.4713f,-0.6617f,-0.5831f},{0.4545f,0.1575f},c});
    v.push_back({{0.21266f,-0.42533f,-0.15451f},{0.1876f,-0.7947f,-0.5773f},{0.5000f,0.0787f},c});
    v.push_back({{-0.08123f,-0.42533f,-0.25000f},{0.1876f,-0.7947f,-0.5773f},{0.5909f,0.0787f},c});
    v.push_back({{0.13143f,-0.26287f,-0.40451f},{0.1876f,-0.7947f,-0.5773f},{0.5455f,0.1575f},c});
    v.push_back({{-0.08123f,-0.42533f,-0.25000f},{-0.0385f,-0.6617f,-0.7488f},{0.5909f,0.0787f},c});
    v.push_back({{-0.13819f,-0.22361f,-0.42532f},{-0.0385f,-0.6617f,-0.7488f},{0.6364f,0.1575f},c});
    v.push_back({{0.13143f,-0.26287f,-0.40451f},{-0.0385f,-0.6617f,-0.7488f},{0.5455f,0.1575f},c});
    v.push_back({{-0.08123f,-0.42533f,-0.25000f},{-0.4089f,-0.6617f,-0.6284f},{0.6818f,0.0787f},c});
    v.push_back({{-0.34409f,-0.26287f,-0.25000f},{-0.4089f,-0.6617f,-0.6284f},{0.7273f,0.1575f},c});
    v.push_back({{-0.13819f,-0.22361f,-0.42532f},{-0.4089f,-0.6617f,-0.6284f},{0.6364f,0.1575f},c});
    v.push_back({{-0.08123f,-0.42533f,-0.25000f},{-0.4911f,-0.7947f,-0.3568f},{0.6818f,0.0787f},c});
    v.push_back({{-0.26286f,-0.42533f,-0.00000f},{-0.4911f,-0.7947f,-0.3568f},{0.7727f,0.0787f},c});
    v.push_back({{-0.34409f,-0.26287f,-0.25000f},{-0.4911f,-0.7947f,-0.3568f},{0.7273f,0.1575f},c});
    v.push_back({{-0.26286f,-0.42533f,-0.00000f},{-0.7240f,-0.6617f,-0.1947f},{0.7727f,0.0787f},c});
    v.push_back({{-0.44721f,-0.22361f,-0.00000f},{-0.7240f,-0.6617f,-0.1947f},{0.8182f,0.1575f},c});
    v.push_back({{-0.34409f,-0.26287f,-0.25000f},{-0.7240f,-0.6617f,-0.1947f},{0.7273f,0.1575f},c});
    v.push_back({{-0.26286f,-0.42533f,-0.00000f},{-0.7240f,-0.6617f,0.1947f},{0.8636f,0.0787f},c});
    v.push_back({{-0.34409f,-0.26287f,0.25000f},{-0.7240f,-0.6617f,0.1947f},{0.9091f,0.1575f},c});
    v.push_back({{-0.44721f,-0.22361f,-0.00000f},{-0.7240f,-0.6617f,0.1947f},{0.8182f,0.1575f},c});
    v.push_back({{-0.26286f,-0.42533f,-0.00000f},{-0.4911f,-0.7947f,0.3568f},{0.8636f,0.0787f},c});
    v.push_back({{-0.08123f,-0.42533f,0.25000f},{-0.4911f,-0.7947f,0.3568f},{0.9545f,0.0787f},c});
    v.push_back({{-0.34409f,-0.26287f,0.25000f},{-0.4911f,-0.7947f,0.3568f},{0.9091f,0.1575f},c});
    v.push_back({{-0.08123f,-0.42533f,0.25000f},{-0.4089f,-0.6617f,0.6284f},{0.9545f,0.0787f},c});
    v.push_back({{-0.13819f,-0.22361f,0.42532f},{-0.4089f,-0.6617f,0.6284f},{1.0000f,0.1575f},c});
    v.push_back({{-0.34409f,-0.26287f,0.25000f},{-0.4089f,-0.6617f,0.6284f},{0.9091f,0.1575f},c});
    v.push_back({{0.42532f,-0.26287f,-0.00000f},{0.7002f,-0.6617f,-0.2680f},{0.3636f,0.1575f},c});
    v.push_back({{0.21266f,-0.42533f,-0.15451f},{0.7002f,-0.6617f,-0.2680f},{0.4091f,0.0787f},c});
    v.push_back({{0.36180f,-0.22361f,-0.26286f},{0.7002f,-0.6617f,-0.2680f},{0.4545f,0.1575f},c});
    v.push_back({{0.42532f,-0.26287f,-0.00000f},{0.6071f,-0.7947f,-0.0000f},{0.3636f,0.1575f},c});
    v.push_back({{0.21266f,-0.42533f,0.15451f},{0.6071f,-0.7947f,-0.0000f},{0.3182f,0.0787f},c});
    v.push_back({{0.21266f,-0.42533f,-0.15451f},{0.6071f,-0.7947f,-0.0000f},{0.4091f,0.0787f},c});
    v.push_back({{0.21266f,-0.42533f,0.15451f},{0.3313f,-0.9435f,-0.0000f},{0.3182f,0.0787f},c});
    v.push_back({{0.00000f,-0.50000f,-0.00000f},{0.3313f,-0.9435f,-0.0000f},{0.3636f,0.0000f},c});
    v.push_back({{0.21266f,-0.42533f,-0.15451f},{0.3313f,-0.9435f,-0.0000f},{0.4091f,0.0787f},c});
    v.push_back({{-0.08123f,-0.42533f,0.25000f},{-0.0385f,-0.6617f,0.7488f},{0.1364f,0.0787f},c});
    v.push_back({{0.13143f,-0.26287f,0.40451f},{-0.0385f,-0.6617f,0.7488f},{0.1818f,0.1575f},c});
    v.push_back({{-0.13819f,-0.22361f,0.42532f},{-0.0385f,-0.6617f,0.7488f},{0.0909f,0.1575f},c});
    v.push_back({{-0.08123f,-0.42533f,0.25000f},{0.1876f,-0.7947f,0.5773f},{0.1364f,0.0787f},c});
    v.push_back({{0.21266f,-0.42533f,0.15451f},{0.1876f,-0.7947f,0.5773f},{0.2273f,0.0787f},c});
    v.push_back({{0.13143f,-0.26287f,0.40451f},{0.1876f,-0.7947f,0.5773f},{0.1818f,0.1575f},c});
    v.push_back({{0.21266f,-0.42533f,0.15451f},{0.4713f,-0.6617f,0.5831f},{0.2273f,0.0787f},c});
    v.push_back({{0.36180f,-0.22361f,0.26286f},{0.4713f,-0.6617f,0.5831f},{0.2727f,0.1575f},c});
    v.push_back({{0.13143f,-0.26287f,0.40451f},{0.4713f,-0.6617f,0.5831f},{0.1818f,0.1575f},c});
    idx.insert(idx.end(), {
        uint16_t(base+0),uint16_t(base+1),uint16_t(base+2),uint16_t(base+3),uint16_t(base+4),uint16_t(base+5),uint16_t(base+6),uint16_t(base+7),uint16_t(base+8),uint16_t(base+9),uint16_t(base+10),uint16_t(base+11),
        uint16_t(base+12),uint16_t(base+13),uint16_t(base+14),uint16_t(base+15),uint16_t(base+16),uint16_t(base+17),uint16_t(base+18),uint16_t(base+19),uint16_t(base+20),uint16_t(base+21),uint16_t(base+22),uint16_t(base+23),
        uint16_t(base+24),uint16_t(base+25),uint16_t(base+26),uint16_t(base+27),uint16_t(base+28),uint16_t(base+29),uint16_t(base+30),uint16_t(base+31),uint16_t(base+32),uint16_t(base+33),uint16_t(base+34),uint16_t(base+35),
        uint16_t(base+36),uint16_t(base+37),uint16_t(base+38),uint16_t(base+39),uint16_t(base+40),uint16_t(base+41),uint16_t(base+42),uint16_t(base+43),uint16_t(base+44),uint16_t(base+45),uint16_t(base+46),uint16_t(base+47),
        uint16_t(base+48),uint16_t(base+49),uint16_t(base+50),uint16_t(base+51),uint16_t(base+52),uint16_t(base+53),uint16_t(base+54),uint16_t(base+55),uint16_t(base+56),uint16_t(base+57),uint16_t(base+58),uint16_t(base+59),
        uint16_t(base+60),uint16_t(base+61),uint16_t(base+62),uint16_t(base+63),uint16_t(base+64),uint16_t(base+65),uint16_t(base+66),uint16_t(base+67),uint16_t(base+68),uint16_t(base+69),uint16_t(base+70),uint16_t(base+71),
        uint16_t(base+72),uint16_t(base+73),uint16_t(base+74),uint16_t(base+75),uint16_t(base+76),uint16_t(base+77),uint16_t(base+78),uint16_t(base+79),uint16_t(base+80),uint16_t(base+81),uint16_t(base+82),uint16_t(base+83),
        uint16_t(base+84),uint16_t(base+85),uint16_t(base+86),uint16_t(base+87),uint16_t(base+88),uint16_t(base+89),uint16_t(base+90),uint16_t(base+91),uint16_t(base+92),uint16_t(base+93),uint16_t(base+94),uint16_t(base+95),
        uint16_t(base+96),uint16_t(base+97),uint16_t(base+98),uint16_t(base+99),uint16_t(base+100),uint16_t(base+101),uint16_t(base+102),uint16_t(base+103),uint16_t(base+104),uint16_t(base+105),uint16_t(base+106),uint16_t(base+107),
        uint16_t(base+108),uint16_t(base+109),uint16_t(base+110),uint16_t(base+111),uint16_t(base+112),uint16_t(base+113),uint16_t(base+114),uint16_t(base+115),uint16_t(base+116),uint16_t(base+117),uint16_t(base+118),uint16_t(base+119),
        uint16_t(base+120),uint16_t(base+121),uint16_t(base+122),uint16_t(base+123),uint16_t(base+124),uint16_t(base+125),uint16_t(base+126),uint16_t(base+127),uint16_t(base+128),uint16_t(base+129),uint16_t(base+130),uint16_t(base+131),
        uint16_t(base+132),uint16_t(base+133),uint16_t(base+134),uint16_t(base+135),uint16_t(base+136),uint16_t(base+137),uint16_t(base+138),uint16_t(base+139),uint16_t(base+140),uint16_t(base+141),uint16_t(base+142),uint16_t(base+143),
        uint16_t(base+144),uint16_t(base+145),uint16_t(base+146),uint16_t(base+147),uint16_t(base+148),uint16_t(base+149),uint16_t(base+150),uint16_t(base+151),uint16_t(base+152),uint16_t(base+153),uint16_t(base+154),uint16_t(base+155),
        uint16_t(base+156),uint16_t(base+157),uint16_t(base+158),uint16_t(base+159),uint16_t(base+160),uint16_t(base+161),uint16_t(base+162),uint16_t(base+163),uint16_t(base+164),uint16_t(base+165),uint16_t(base+166),uint16_t(base+167),
        uint16_t(base+168),uint16_t(base+169),uint16_t(base+170),uint16_t(base+171),uint16_t(base+172),uint16_t(base+173),uint16_t(base+174),uint16_t(base+175),uint16_t(base+176),uint16_t(base+177),uint16_t(base+178),uint16_t(base+179),
        uint16_t(base+180),uint16_t(base+181),uint16_t(base+182),uint16_t(base+183),uint16_t(base+184),uint16_t(base+185),uint16_t(base+186),uint16_t(base+187),uint16_t(base+188),uint16_t(base+189),uint16_t(base+190),uint16_t(base+191),
        uint16_t(base+192),uint16_t(base+193),uint16_t(base+194),uint16_t(base+195),uint16_t(base+196),uint16_t(base+197),uint16_t(base+198),uint16_t(base+199),uint16_t(base+200),uint16_t(base+201),uint16_t(base+202),uint16_t(base+203),
        uint16_t(base+204),uint16_t(base+205),uint16_t(base+206),uint16_t(base+207),uint16_t(base+208),uint16_t(base+209),uint16_t(base+210),uint16_t(base+211),uint16_t(base+212),uint16_t(base+213),uint16_t(base+214),uint16_t(base+215),
        uint16_t(base+216),uint16_t(base+217),uint16_t(base+218),uint16_t(base+219),uint16_t(base+220),uint16_t(base+221),uint16_t(base+222),uint16_t(base+223),uint16_t(base+224),uint16_t(base+225),uint16_t(base+226),uint16_t(base+227),
        uint16_t(base+228),uint16_t(base+229),uint16_t(base+230),uint16_t(base+231),uint16_t(base+232),uint16_t(base+233),
    });
}

void BlockMeshGenerator::generateWTF(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx, simd::float4 c)
{
    uint16_t base = (uint16_t)v.size();
    v.push_back({{0.41366f,-0.12874f,0.04317f},{-0.0336f,-0.9234f,0.3825f},{0.3881f,0.7707f},c});
    v.push_back({{0.40498f,-0.13737f,0.02159f},{-0.0336f,-0.9234f,0.3825f},{0.4231f,0.7597f},c});
    v.push_back({{0.41366f,-0.13737f,0.02235f},{-0.0336f,-0.9234f,0.3825f},{0.4227f,0.7753f},c});
    v.push_back({{0.41366f,-0.07631f,0.08340f},{-0.0796f,-0.3815f,0.9209f},{0.0015f,0.3581f},c});
    v.push_back({{0.38419f,-0.09714f,0.07223f},{-0.0796f,-0.3815f,0.9209f},{0.0602f,0.3971f},c});
    v.push_back({{0.41366f,-0.09714f,0.07478f},{-0.0796f,-0.3815f,0.9209f},{0.0058f,0.3971f},c});
    v.push_back({{0.41366f,-0.09714f,0.07478f},{-0.0687f,-0.6073f,0.7915f},{0.0058f,0.3971f},c});
    v.push_back({{0.38969f,-0.11502f,0.05897f},{-0.0687f,-0.6073f,0.7915f},{0.0569f,0.4306f},c});
    v.push_back({{0.41366f,-0.11502f,0.06105f},{-0.0687f,-0.6073f,0.7915f},{0.0126f,0.4306f},c});
    v.push_back({{0.41366f,-0.11502f,0.06105f},{-0.0530f,-0.7922f,0.6079f},{0.3608f,0.7635f},c});
    v.push_back({{0.39680f,-0.12874f,0.04170f},{-0.0530f,-0.7922f,0.6079f},{0.3888f,0.7405f},c});
    v.push_back({{0.41366f,-0.12874f,0.04317f},{-0.0530f,-0.7922f,0.6079f},{0.3881f,0.7707f},c});
    v.push_back({{0.41366f,-0.05396f,0.08634f},{-0.0852f,-0.1301f,0.9878f},{-0.0000f,0.3163f},c});
    v.push_back({{0.38070f,-0.07631f,0.08056f},{-0.0852f,-0.1301f,0.9878f},{0.0623f,0.3581f},c});
    v.push_back({{0.41366f,-0.07631f,0.08340f},{-0.0852f,-0.1301f,0.9878f},{0.0015f,0.3581f},c});
    v.push_back({{0.38969f,-0.11502f,0.05897f},{-0.1513f,-0.8034f,0.5760f},{0.3618f,0.7205f},c});
    v.push_back({{0.38038f,-0.12874f,0.03739f},{-0.1513f,-0.8034f,0.5760f},{0.3948f,0.7110f},c});
    v.push_back({{0.39680f,-0.12874f,0.04170f},{-0.1513f,-0.8034f,0.5760f},{0.3888f,0.7405f},c});
    v.push_back({{0.37951f,-0.05396f,0.08340f},{-0.2405f,-0.1351f,0.9612f},{0.0631f,0.3163f},c});
    v.push_back({{0.34741f,-0.07631f,0.07223f},{-0.2405f,-0.1351f,0.9612f},{0.1266f,0.3581f},c});
    v.push_back({{0.38070f,-0.07631f,0.08056f},{-0.2405f,-0.1351f,0.9612f},{0.0623f,0.3581f},c});
    v.push_back({{0.39680f,-0.12874f,0.04170f},{-0.0961f,-0.9284f,0.3590f},{0.3888f,0.7405f},c});
    v.push_back({{0.39664f,-0.13737f,0.01935f},{-0.0961f,-0.9284f,0.3590f},{0.4262f,0.7447f},c});
    v.push_back({{0.40498f,-0.13737f,0.02159f},{-0.0961f,-0.9284f,0.3590f},{0.4231f,0.7597f},c});
    v.push_back({{0.38070f,-0.07631f,0.08056f},{-0.2256f,-0.3942f,0.8909f},{0.0623f,0.3581f},c});
    v.push_back({{0.35469f,-0.09714f,0.06476f},{-0.2256f,-0.3942f,0.8909f},{0.1171f,0.3971f},c});
    v.push_back({{0.38419f,-0.09714f,0.07223f},{-0.2256f,-0.3942f,0.8909f},{0.0602f,0.3971f},c});
    v.push_back({{0.38419f,-0.09714f,0.07223f},{-0.1953f,-0.6221f,0.7582f},{0.0602f,0.3971f},c});
    v.push_back({{0.36602f,-0.11502f,0.05287f},{-0.1953f,-0.6221f,0.7582f},{0.1026f,0.4306f},c});
    v.push_back({{0.38969f,-0.11502f,0.05897f},{-0.1953f,-0.6221f,0.7582f},{0.0569f,0.4306f},c});
    v.push_back({{0.35469f,-0.09714f,0.06476f},{-0.3104f,-0.6546f,0.6893f},{0.1171f,0.3971f},c});
    v.push_back({{0.34447f,-0.11502f,0.04317f},{-0.3104f,-0.6546f,0.6893f},{0.1464f,0.4306f},c});
    v.push_back({{0.36602f,-0.11502f,0.05287f},{-0.3104f,-0.6546f,0.6893f},{0.1026f,0.4306f},c});
    v.push_back({{0.36602f,-0.11502f,0.05287f},{-0.2341f,-0.8252f,0.5141f},{0.3702f,0.6780f},c});
    v.push_back({{0.36531f,-0.12874f,0.03053f},{-0.2341f,-0.8252f,0.5141f},{0.4056f,0.6839f},c});
    v.push_back({{0.38038f,-0.12874f,0.03739f},{-0.2341f,-0.8252f,0.5141f},{0.3948f,0.7110f},c});
    v.push_back({{0.34489f,-0.05396f,0.07478f},{-0.3641f,-0.1459f,0.9198f},{0.1298f,0.3163f},c});
    v.push_back({{0.31393f,-0.07631f,0.05897f},{-0.3641f,-0.1459f,0.9198f},{0.1936f,0.3581f},c});
    v.push_back({{0.34741f,-0.07631f,0.07223f},{-0.3641f,-0.1459f,0.9198f},{0.1266f,0.3581f},c});
    v.push_back({{0.38038f,-0.12874f,0.03739f},{-0.1477f,-0.9375f,0.3152f},{0.3948f,0.7110f},c});
    v.push_back({{0.38906f,-0.13737f,0.01580f},{-0.1477f,-0.9375f,0.3152f},{0.4318f,0.7311f},c});
    v.push_back({{0.39664f,-0.13737f,0.01935f},{-0.1477f,-0.9375f,0.3152f},{0.4262f,0.7447f},c});
    v.push_back({{0.34741f,-0.07631f,0.07223f},{-0.3513f,-0.4226f,0.8355f},{0.1266f,0.3581f},c});
    v.push_back({{0.32643f,-0.09714f,0.05287f},{-0.3513f,-0.4226f,0.8355f},{0.1741f,0.3971f},c});
    v.push_back({{0.35469f,-0.09714f,0.06476f},{-0.3513f,-0.4226f,0.8355f},{0.1171f,0.3971f},c});
    v.push_back({{0.36531f,-0.12874f,0.03053f},{-0.1847f,-0.9484f,0.2576f},{0.4056f,0.6839f},c});
    v.push_back({{0.38261f,-0.13737f,0.01117f},{-0.1847f,-0.9484f,0.2576f},{0.4395f,0.7195f},c});
    v.push_back({{0.38906f,-0.13737f,0.01580f},{-0.1847f,-0.9484f,0.2576f},{0.4318f,0.7311f},c});
    v.push_back({{0.31393f,-0.07631f,0.05897f},{-0.4498f,-0.4890f,0.7473f},{0.1936f,0.3581f},c});
    v.push_back({{0.30070f,-0.09714f,0.03739f},{-0.4498f,-0.4890f,0.7473f},{0.2282f,0.3971f},c});
    v.push_back({{0.32643f,-0.09714f,0.05287f},{-0.4498f,-0.4890f,0.7473f},{0.1741f,0.3971f},c});
    v.push_back({{0.32643f,-0.09714f,0.05287f},{-0.4046f,-0.7165f,0.5683f},{0.3726f,0.5974f},c});
    v.push_back({{0.32671f,-0.11502f,0.03053f},{-0.4046f,-0.7165f,0.5683f},{0.4064f,0.6073f},c});
    v.push_back({{0.34447f,-0.11502f,0.04317f},{-0.4046f,-0.7165f,0.5683f},{0.3854f,0.6393f},c});
    v.push_back({{0.34447f,-0.11502f,0.04317f},{-0.3320f,-0.8605f,0.3864f},{0.3854f,0.6393f},c});
    v.push_back({{0.35490f,-0.12874f,0.02159f},{-0.3320f,-0.8605f,0.3864f},{0.4207f,0.6652f},c});
    v.push_back({{0.36531f,-0.12874f,0.03053f},{-0.3320f,-0.8605f,0.3864f},{0.4056f,0.6839f},c});
    v.push_back({{0.30971f,-0.05396f,0.06105f},{-0.4300f,-0.1637f,0.8879f},{0.2001f,0.3163f},c});
    v.push_back({{0.27826f,-0.07631f,0.04170f},{-0.4300f,-0.1637f,0.8879f},{0.2665f,0.3581f},c});
    v.push_back({{0.31393f,-0.07631f,0.05897f},{-0.4300f,-0.1637f,0.8879f},{0.1936f,0.3581f},c});
    v.push_back({{0.32671f,-0.11502f,0.03053f},{-0.3501f,-0.8964f,0.2717f},{0.4064f,0.6073f},c});
    v.push_back({{0.34682f,-0.12874f,0.01117f},{-0.3501f,-0.8964f,0.2717f},{0.4388f,0.6507f},c});
    v.push_back({{0.35490f,-0.12874f,0.02159f},{-0.3501f,-0.8964f,0.2717f},{0.4207f,0.6652f},c});
    v.push_back({{0.26602f,-0.05396f,0.04317f},{-0.5078f,-0.3305f,0.7955f},{0.2879f,0.3163f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{-0.5078f,-0.3305f,0.7955f},{0.3334f,0.3581f},c});
    v.push_back({{0.27826f,-0.07631f,0.04170f},{-0.5078f,-0.3305f,0.7955f},{0.2665f,0.3581f},c});
    v.push_back({{0.35490f,-0.12874f,0.02159f},{-0.2199f,-0.9537f,0.2051f},{0.4207f,0.6652f},c});
    v.push_back({{0.37758f,-0.13737f,0.00578f},{-0.2199f,-0.9537f,0.2051f},{0.4488f,0.7105f},c});
    v.push_back({{0.38261f,-0.13737f,0.01117f},{-0.2199f,-0.9537f,0.2051f},{0.4395f,0.7195f},c});
    v.push_back({{0.27826f,-0.07631f,0.04170f},{-0.4765f,-0.6386f,0.6042f},{0.2665f,0.3581f},c});
    v.push_back({{0.27783f,-0.09714f,0.01935f},{-0.4765f,-0.6386f,0.6042f},{0.2785f,0.3971f},c});
    v.push_back({{0.30070f,-0.09714f,0.03739f},{-0.4765f,-0.6386f,0.6042f},{0.2282f,0.3971f},c});
    v.push_back({{0.30070f,-0.09714f,0.03739f},{-0.4534f,-0.8056f,0.3814f},{0.3979f,0.5512f},c});
    v.push_back({{0.31432f,-0.11502f,0.01580f},{-0.4534f,-0.8056f,0.3814f},{0.4319f,0.5851f},c});
    v.push_back({{0.32671f,-0.11502f,0.03053f},{-0.4534f,-0.8056f,0.3814f},{0.4064f,0.6073f},c});
    v.push_back({{0.27783f,-0.09714f,0.01935f},{-0.4168f,-0.8885f,0.1917f},{0.4282f,0.5101f},c});
    v.push_back({{0.30705f,-0.11502f,0.00000f},{-0.4168f,-0.8885f,0.1917f},{0.4599f,0.5720f},c});
    v.push_back({{0.31432f,-0.11502f,0.01580f},{-0.4168f,-0.8885f,0.1917f},{0.4319f,0.5851f},c});
    v.push_back({{0.31432f,-0.11502f,0.01580f},{-0.3625f,-0.9162f,0.1709f},{0.4319f,0.5851f},c});
    v.push_back({{0.34155f,-0.12874f,0.00000f},{-0.3625f,-0.9162f,0.1709f},{0.4586f,0.6412f},c});
    v.push_back({{0.34682f,-0.12874f,0.01117f},{-0.3625f,-0.9162f,0.1709f},{0.4388f,0.6507f},c});
    v.push_back({{0.24391f,-0.05396f,0.02235f},{0.0725f,-0.0248f,0.9971f},{0.3382f,0.3163f},c});
    v.push_back({{0.23120f,-0.06393f,0.02302f},{0.0725f,-0.0248f,0.9971f},{0.3608f,0.3350f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{0.0725f,-0.0248f,0.9971f},{0.3334f,0.3581f},c});
    v.push_back({{0.34682f,-0.12874f,0.01117f},{-0.2440f,-0.9592f,0.1428f},{0.4388f,0.6507f},c});
    v.push_back({{0.37420f,-0.13737f,0.00000f},{-0.2440f,-0.9592f,0.1428f},{0.4590f,0.7044f},c});
    v.push_back({{0.37758f,-0.13737f,0.00578f},{-0.2440f,-0.9592f,0.1428f},{0.4488f,0.7105f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{-0.4791f,-0.7618f,0.4361f},{0.4286f,0.4433f},c});
    v.push_back({{0.26022f,-0.09714f,-0.00000f},{-0.4791f,-0.7618f,0.4361f},{0.4615f,0.4785f},c});
    v.push_back({{0.27783f,-0.09714f,0.01935f},{-0.4791f,-0.7618f,0.4361f},{0.4282f,0.5101f},c});
    v.push_back({{0.23122f,-0.06393f,-0.02294f},{-0.6171f,-0.7827f,-0.0812f},{0.5130f,0.4088f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{-0.6171f,-0.7827f,-0.0812f},{0.5075f,0.4433f},c});
    v.push_back({{0.24391f,-0.07631f,-0.00000f},{-0.6171f,-0.7827f,-0.0812f},{0.4677f,0.4382f},c});
    v.push_back({{0.37420f,-0.13737f,0.00000f},{-0.2440f,-0.9592f,-0.1428f},{0.4590f,0.7044f},c});
    v.push_back({{0.34682f,-0.12874f,-0.01117f},{-0.2440f,-0.9592f,-0.1428f},{0.4796f,0.6507f},c});
    v.push_back({{0.37758f,-0.13737f,-0.00578f},{-0.2440f,-0.9592f,-0.1428f},{0.4699f,0.7105f},c});
    v.push_back({{0.26022f,-0.09714f,-0.00000f},{-0.4791f,-0.7618f,-0.4361f},{0.4615f,0.4785f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{-0.4791f,-0.7618f,-0.4361f},{0.5075f,0.4433f},c});
    v.push_back({{0.27783f,-0.09714f,-0.01935f},{-0.4791f,-0.7618f,-0.4361f},{0.4989f,0.5101f},c});
    v.push_back({{0.30705f,-0.11502f,0.00000f},{-0.4168f,-0.8885f,-0.1917f},{0.4599f,0.5720f},c});
    v.push_back({{0.27783f,-0.09714f,-0.01935f},{-0.4168f,-0.8885f,-0.1917f},{0.4989f,0.5101f},c});
    v.push_back({{0.31432f,-0.11502f,-0.01580f},{-0.4168f,-0.8885f,-0.1917f},{0.4896f,0.5851f},c});
    v.push_back({{0.34155f,-0.12874f,0.00000f},{-0.3625f,-0.9162f,-0.1709f},{0.4586f,0.6412f},c});
    v.push_back({{0.31432f,-0.11502f,-0.01580f},{-0.3625f,-0.9162f,-0.1709f},{0.4896f,0.5851f},c});
    v.push_back({{0.34682f,-0.12874f,-0.01117f},{-0.3625f,-0.9162f,-0.1709f},{0.4796f,0.6507f},c});
    v.push_back({{0.34682f,-0.12874f,-0.01117f},{-0.3501f,-0.8964f,-0.2717f},{0.4796f,0.6507f},c});
    v.push_back({{0.32671f,-0.11502f,-0.03053f},{-0.3501f,-0.8964f,-0.2717f},{0.5180f,0.6073f},c});
    v.push_back({{0.35490f,-0.12874f,-0.02159f},{-0.3501f,-0.8964f,-0.2717f},{0.4996f,0.6652f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{-0.5078f,-0.3305f,-0.7955f},{0.9237f,0.2889f},c});
    v.push_back({{0.26602f,-0.05396f,-0.04317f},{-0.5078f,-0.3305f,-0.7955f},{0.9641f,0.2448f},c});
    v.push_back({{0.27826f,-0.07631f,-0.04170f},{-0.5078f,-0.3305f,-0.7955f},{0.9614f,0.2851f},c});
    v.push_back({{0.37758f,-0.13737f,-0.00578f},{-0.2199f,-0.9537f,-0.2051f},{0.4699f,0.7105f},c});
    v.push_back({{0.35490f,-0.12874f,-0.02159f},{-0.2199f,-0.9537f,-0.2051f},{0.4996f,0.6652f},c});
    v.push_back({{0.38261f,-0.13737f,-0.01117f},{-0.2199f,-0.9537f,-0.2051f},{0.4804f,0.7195f},c});
    v.push_back({{0.27783f,-0.09714f,-0.01935f},{-0.4765f,-0.6386f,-0.6042f},{0.4989f,0.5101f},c});
    v.push_back({{0.27826f,-0.07631f,-0.04170f},{-0.4765f,-0.6386f,-0.6042f},{0.5479f,0.4998f},c});
    v.push_back({{0.30070f,-0.09714f,-0.03739f},{-0.4765f,-0.6386f,-0.6042f},{0.5345f,0.5511f},c});
    v.push_back({{0.31432f,-0.11502f,-0.01580f},{-0.4534f,-0.8056f,-0.3814f},{0.4896f,0.5851f},c});
    v.push_back({{0.30070f,-0.09714f,-0.03739f},{-0.4534f,-0.8056f,-0.3814f},{0.5345f,0.5511f},c});
    v.push_back({{0.32671f,-0.11502f,-0.03053f},{-0.4534f,-0.8056f,-0.3814f},{0.5180f,0.6073f},c});
    v.push_back({{0.30070f,-0.09714f,-0.03739f},{-0.4498f,-0.4890f,-0.7473f},{0.5345f,0.5511f},c});
    v.push_back({{0.31393f,-0.07631f,-0.05897f},{-0.4498f,-0.4890f,-0.7473f},{0.5836f,0.5639f},c});
    v.push_back({{0.32643f,-0.09714f,-0.05287f},{-0.4498f,-0.4890f,-0.7473f},{0.5658f,0.5973f},c});
    v.push_back({{0.32671f,-0.11502f,-0.03053f},{-0.4046f,-0.7165f,-0.5683f},{0.5180f,0.6073f},c});
    v.push_back({{0.32643f,-0.09714f,-0.05287f},{-0.4046f,-0.7165f,-0.5683f},{0.5658f,0.5973f},c});
    v.push_back({{0.34447f,-0.11502f,-0.04317f},{-0.4046f,-0.7165f,-0.5683f},{0.5431f,0.6392f},c});
    v.push_back({{0.30971f,-0.05396f,0.06105f},{-0.3788f,-0.0000f,0.9255f},{0.2001f,0.3163f},c});
    v.push_back({{0.26602f,0.05396f,0.04317f},{-0.3788f,-0.0000f,0.9255f},{0.2879f,0.1143f},c});
    v.push_back({{0.26602f,-0.05396f,0.04317f},{-0.3788f,-0.0000f,0.9255f},{0.2879f,0.3163f},c});
    v.push_back({{0.35490f,-0.12874f,-0.02159f},{-0.3320f,-0.8605f,-0.3864f},{0.4996f,0.6652f},c});
    v.push_back({{0.34447f,-0.11502f,-0.04317f},{-0.3320f,-0.8605f,-0.3864f},{0.5431f,0.6392f},c});
    v.push_back({{0.36531f,-0.12874f,-0.03053f},{-0.3320f,-0.8605f,-0.3864f},{0.5171f,0.6839f},c});
    v.push_back({{0.27826f,-0.07631f,-0.04170f},{-0.4300f,-0.1637f,-0.8879f},{0.6295f,0.3248f},c});
    v.push_back({{0.30971f,-0.05396f,-0.06105f},{-0.4300f,-0.1637f,-0.8879f},{0.6706f,0.2829f},c});
    v.push_back({{0.31393f,-0.07631f,-0.05897f},{-0.4300f,-0.1637f,-0.8879f},{0.6793f,0.3247f},c});
    v.push_back({{0.24391f,-0.05396f,-0.02235f},{-0.6856f,-0.0000f,-0.7280f},{0.9252f,0.2475f},c});
    v.push_back({{0.26602f,0.05396f,-0.04317f},{-0.6856f,-0.0000f,-0.7280f},{0.9641f,0.0432f},c});
    v.push_back({{0.26602f,-0.05396f,-0.04317f},{-0.6856f,-0.0000f,-0.7280f},{0.9641f,0.2448f},c});
    v.push_back({{0.38261f,-0.13737f,-0.01117f},{-0.1847f,-0.9484f,-0.2576f},{0.4804f,0.7195f},c});
    v.push_back({{0.36531f,-0.12874f,-0.03053f},{-0.1847f,-0.9484f,-0.2576f},{0.5171f,0.6839f},c});
    v.push_back({{0.38906f,-0.13737f,-0.01580f},{-0.1847f,-0.9484f,-0.2576f},{0.4896f,0.7311f},c});
    v.push_back({{0.31393f,-0.07631f,-0.05897f},{-0.3641f,-0.1459f,-0.9198f},{0.6793f,0.3247f},c});
    v.push_back({{0.34489f,-0.05396f,-0.07478f},{-0.3641f,-0.1459f,-0.9198f},{0.7221f,0.2829f},c});
    v.push_back({{0.34741f,-0.07631f,-0.07223f},{-0.3641f,-0.1459f,-0.9198f},{0.7282f,0.3247f},c});
    v.push_back({{0.26602f,-0.05396f,-0.04317f},{-0.3788f,-0.0000f,-0.9255f},{0.6072f,0.2829f},c});
    v.push_back({{0.30971f,0.05396f,-0.06105f},{-0.3788f,-0.0000f,-0.9255f},{0.6706f,0.0809f},c});
    v.push_back({{0.30971f,-0.05396f,-0.06105f},{-0.3788f,-0.0000f,-0.9255f},{0.6706f,0.2829f},c});
    v.push_back({{0.38906f,-0.13737f,-0.01580f},{-0.1477f,-0.9375f,-0.3152f},{0.4896f,0.7311f},c});
    v.push_back({{0.38038f,-0.12874f,-0.03739f},{-0.1477f,-0.9375f,-0.3152f},{0.5314f,0.7109f},c});
    v.push_back({{0.39664f,-0.13737f,-0.01935f},{-0.1477f,-0.9375f,-0.3152f},{0.4969f,0.7447f},c});
    v.push_back({{0.32643f,-0.09714f,-0.05287f},{-0.3513f,-0.4226f,-0.8355f},{0.5658f,0.5973f},c});
    v.push_back({{0.34741f,-0.07631f,-0.07223f},{-0.3513f,-0.4226f,-0.8355f},{0.6117f,0.6240f},c});
    v.push_back({{0.35469f,-0.09714f,-0.06476f},{-0.3513f,-0.4226f,-0.8355f},{0.5908f,0.6481f},c});
    v.push_back({{0.34489f,-0.05396f,-0.07478f},{-0.3634f,-0.0000f,-0.9316f},{0.7221f,0.2829f},c});
    v.push_back({{0.30971f,0.05396f,-0.06105f},{-0.3634f,-0.0000f,-0.9316f},{0.6706f,0.0809f},c});
    v.push_back({{0.34489f,0.05396f,-0.07478f},{-0.3634f,-0.0000f,-0.9316f},{0.7221f,0.0808f},c});
    v.push_back({{0.34447f,-0.11502f,-0.04317f},{-0.3104f,-0.6546f,-0.6893f},{0.5431f,0.6392f},c});
    v.push_back({{0.35469f,-0.09714f,-0.06476f},{-0.3104f,-0.6546f,-0.6893f},{0.5908f,0.6481f},c});
    v.push_back({{0.36602f,-0.11502f,-0.05287f},{-0.3104f,-0.6546f,-0.6893f},{0.5634f,0.6779f},c});
    v.push_back({{0.34489f,-0.05396f,-0.07478f},{-0.2418f,-0.0000f,-0.9703f},{0.7221f,0.2829f},c});
    v.push_back({{0.37951f,0.05396f,-0.08340f},{-0.2418f,-0.0000f,-0.9703f},{0.7762f,0.0808f},c});
    v.push_back({{0.37951f,-0.05396f,-0.08340f},{-0.2418f,-0.0000f,-0.9703f},{0.7762f,0.2828f},c});
    v.push_back({{0.36531f,-0.12874f,-0.03053f},{-0.2341f,-0.8252f,-0.5141f},{0.5171f,0.6839f},c});
    v.push_back({{0.36602f,-0.11502f,-0.05287f},{-0.2341f,-0.8252f,-0.5141f},{0.5634f,0.6779f},c});
    v.push_back({{0.38038f,-0.12874f,-0.03739f},{-0.2341f,-0.8252f,-0.5141f},{0.5314f,0.7109f},c});
    v.push_back({{0.36602f,-0.11502f,-0.05287f},{-0.1953f,-0.6221f,-0.7582f},{0.5634f,0.6779f},c});
    v.push_back({{0.38419f,-0.09714f,-0.07223f},{-0.1953f,-0.6221f,-0.7582f},{0.6078f,0.7010f},c});
    v.push_back({{0.38969f,-0.11502f,-0.05897f},{-0.1953f,-0.6221f,-0.7582f},{0.5773f,0.7204f},c});
    v.push_back({{0.34489f,-0.05396f,0.07478f},{-0.3634f,0.0000f,0.9316f},{0.1298f,0.3163f},c});
    v.push_back({{0.30971f,0.05396f,0.06105f},{-0.3634f,0.0000f,0.9316f},{0.2001f,0.1143f},c});
    v.push_back({{0.30971f,-0.05396f,0.06105f},{-0.3634f,0.0000f,0.9316f},{0.2001f,0.3163f},c});
    v.push_back({{0.38038f,-0.12874f,-0.03739f},{-0.1513f,-0.8034f,-0.5760f},{0.5314f,0.7109f},c});
    v.push_back({{0.38969f,-0.11502f,-0.05897f},{-0.1513f,-0.8034f,-0.5760f},{0.5773f,0.7204f},c});
    v.push_back({{0.39680f,-0.12874f,-0.04170f},{-0.1513f,-0.8034f,-0.5760f},{0.5412f,0.7404f},c});
    v.push_back({{0.34741f,-0.07631f,-0.07223f},{-0.2405f,-0.1351f,-0.9612f},{0.7282f,0.3247f},c});
    v.push_back({{0.37951f,-0.05396f,-0.08340f},{-0.2405f,-0.1351f,-0.9612f},{0.7762f,0.2828f},c});
    v.push_back({{0.38070f,-0.07631f,-0.08056f},{-0.2405f,-0.1351f,-0.9612f},{0.7802f,0.3247f},c});
    v.push_back({{0.37951f,-0.05396f,0.08340f},{-0.2418f,0.0000f,0.9703f},{0.0631f,0.3163f},c});
    v.push_back({{0.34489f,0.05396f,0.07478f},{-0.2418f,0.0000f,0.9703f},{0.1298f,0.1143f},c});
    v.push_back({{0.34489f,-0.05396f,0.07478f},{-0.2418f,0.0000f,0.9703f},{0.1298f,0.3163f},c});
    v.push_back({{0.39664f,-0.13737f,-0.01935f},{-0.0961f,-0.9284f,-0.3590f},{0.4969f,0.7447f},c});
    v.push_back({{0.39680f,-0.12874f,-0.04170f},{-0.0961f,-0.9284f,-0.3590f},{0.5412f,0.7404f},c});
    v.push_back({{0.40498f,-0.13737f,-0.02159f},{-0.0961f,-0.9284f,-0.3590f},{0.5020f,0.7597f},c});
    v.push_back({{0.37951f,-0.05396f,-0.08340f},{-0.0858f,-0.0000f,-0.9963f},{0.7762f,0.2828f},c});
    v.push_back({{0.41366f,0.05396f,-0.08634f},{-0.0858f,-0.0000f,-0.9963f},{0.8334f,0.0808f},c});
    v.push_back({{0.41366f,-0.05396f,-0.08634f},{-0.0858f,-0.0000f,-0.9963f},{0.8334f,0.2828f},c});
    v.push_back({{0.35469f,-0.09714f,-0.06476f},{-0.2256f,-0.3942f,-0.8909f},{0.7461f,0.3637f},c});
    v.push_back({{0.38070f,-0.07631f,-0.08056f},{-0.2256f,-0.3942f,-0.8909f},{0.7802f,0.3247f},c});
    v.push_back({{0.38419f,-0.09714f,-0.07223f},{-0.2256f,-0.3942f,-0.8909f},{0.7921f,0.3637f},c});
    v.push_back({{0.41366f,-0.05396f,0.08634f},{-0.0858f,0.0000f,0.9963f},{-0.0000f,0.3163f},c});
    v.push_back({{0.37951f,0.05396f,0.08340f},{-0.0858f,0.0000f,0.9963f},{0.0631f,0.1143f},c});
    v.push_back({{0.37951f,-0.05396f,0.08340f},{-0.0858f,0.0000f,0.9963f},{0.0631f,0.3163f},c});
    v.push_back({{0.38419f,-0.09714f,-0.07223f},{-0.0796f,-0.3815f,-0.9209f},{0.7921f,0.3637f},c});
    v.push_back({{0.41366f,-0.07631f,-0.08340f},{-0.0796f,-0.3815f,-0.9209f},{0.8355f,0.3247f},c});
    v.push_back({{0.41366f,-0.09714f,-0.07478f},{-0.0796f,-0.3815f,-0.9209f},{0.8415f,0.3636f},c});
    v.push_back({{0.26602f,-0.05396f,0.04317f},{-0.6856f,-0.0000f,0.7280f},{0.2879f,0.3163f},c});
    v.push_back({{0.24391f,0.05396f,0.02235f},{-0.6856f,-0.0000f,0.7280f},{0.3382f,0.1143f},c});
    v.push_back({{0.24391f,-0.05396f,0.02235f},{-0.6856f,-0.0000f,0.7280f},{0.3382f,0.3163f},c});
    v.push_back({{0.38969f,-0.11502f,-0.05897f},{-0.0687f,-0.6073f,-0.7915f},{0.5773f,0.7204f},c});
    v.push_back({{0.41366f,-0.09714f,-0.07478f},{-0.0687f,-0.6073f,-0.7915f},{0.6159f,0.7539f},c});
    v.push_back({{0.41366f,-0.11502f,-0.06105f},{-0.0687f,-0.6073f,-0.7915f},{0.5838f,0.7634f},c});
    v.push_back({{0.24097f,-0.05396f,-0.00000f},{-0.9914f,-0.0000f,-0.1305f},{0.8833f,0.2479f},c});
    v.push_back({{0.24391f,0.05396f,-0.02235f},{-0.9914f,-0.0000f,-0.1305f},{0.9252f,0.0459f},c});
    v.push_back({{0.24391f,-0.05396f,-0.02235f},{-0.9914f,-0.0000f,-0.1305f},{0.9252f,0.2475f},c});
    v.push_back({{0.39680f,-0.12874f,-0.04170f},{-0.0530f,-0.7922f,-0.6079f},{0.5412f,0.7404f},c});
    v.push_back({{0.41366f,-0.11502f,-0.06105f},{-0.0530f,-0.7922f,-0.6079f},{0.5838f,0.7634f},c});
    v.push_back({{0.41366f,-0.12874f,-0.04317f},{-0.0530f,-0.7922f,-0.6079f},{0.5458f,0.7707f},c});
    v.push_back({{0.38070f,-0.07631f,-0.08056f},{-0.0852f,-0.1301f,-0.9878f},{0.7802f,0.3247f},c});
    v.push_back({{0.41366f,-0.05396f,-0.08634f},{-0.0852f,-0.1301f,-0.9878f},{0.8334f,0.2828f},c});
    v.push_back({{0.41366f,-0.07631f,-0.08340f},{-0.0852f,-0.1301f,-0.9878f},{0.8355f,0.3247f},c});
    v.push_back({{0.24391f,-0.05396f,0.02235f},{-0.9914f,-0.0000f,0.1305f},{0.8415f,0.2475f},c});
    v.push_back({{0.24097f,0.05396f,-0.00000f},{-0.9914f,-0.0000f,0.1305f},{0.8833f,0.0463f},c});
    v.push_back({{0.24097f,-0.05396f,-0.00000f},{-0.9914f,-0.0000f,0.1305f},{0.8833f,0.2479f},c});
    v.push_back({{0.40498f,-0.13737f,-0.02159f},{-0.0336f,-0.9234f,-0.3825f},{0.5020f,0.7597f},c});
    v.push_back({{0.41366f,-0.12874f,-0.04317f},{-0.0336f,-0.9234f,-0.3825f},{0.5458f,0.7707f},c});
    v.push_back({{0.41366f,-0.13737f,-0.02235f},{-0.0336f,-0.9234f,-0.3825f},{0.5044f,0.7752f},c});
    v.push_back({{0.43600f,-0.13737f,0.00000f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43524f,-0.13737f,0.00578f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.11502f,0.06105f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.12874f,0.04317f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.13737f,-0.02235f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.12874f,-0.04317f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43301f,-0.13737f,0.01117f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.07631f,-0.08340f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.05396f,-0.08634f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41944f,-0.13737f,0.02159f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.13737f,0.02235f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43301f,0.13737f,-0.01117f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42946f,0.13737f,-0.01580f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.05396f,0.08634f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.05396f,0.08634f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.13737f,0.02235f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42946f,0.13737f,0.01580f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43301f,0.13737f,0.01117f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41944f,-0.13737f,-0.02159f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42483f,-0.13737f,-0.01935f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.09714f,-0.07478f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.12874f,0.04317f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42946f,-0.13737f,-0.01580f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42946f,-0.13737f,0.01580f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.11502f,0.06105f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43524f,0.13737f,-0.00578f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41944f,0.13737f,0.02159f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42483f,0.13737f,0.01935f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.11502f,-0.06105f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43600f,0.13737f,0.00000f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.09714f,0.07478f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.07631f,0.08340f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.12874f,-0.04317f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.13737f,-0.02235f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.09714f,-0.07478f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.11502f,-0.06105f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43301f,-0.13737f,-0.01117f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.05396f,-0.08634f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,-0.07631f,-0.08340f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42483f,-0.13737f,0.01935f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41944f,0.13737f,-0.02159f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43524f,-0.13737f,-0.00578f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.42483f,0.13737f,-0.01935f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.43524f,0.13737f,0.00578f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.07631f,0.08340f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.09714f,0.07478f},{0.0000f,1.0000f,-0.0000f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.09714f,0.07478f},{-0.0684f,0.6076f,0.7913f},{0.0058f,0.0335f},c});
    v.push_back({{0.38969f,0.11502f,0.05897f},{-0.0684f,0.6076f,0.7913f},{0.0569f,0.0000f},c});
    v.push_back({{0.38419f,0.09714f,0.07223f},{-0.0684f,0.6076f,0.7913f},{0.0602f,0.0335f},c});
    v.push_back({{0.41366f,0.07631f,0.08340f},{-0.0794f,0.3817f,0.9209f},{0.0015f,0.0724f},c});
    v.push_back({{0.38419f,0.09714f,0.07223f},{-0.0794f,0.3817f,0.9209f},{0.0602f,0.0335f},c});
    v.push_back({{0.38070f,0.07631f,0.08056f},{-0.0794f,0.3817f,0.9209f},{0.0623f,0.0724f},c});
    v.push_back({{0.41366f,0.12874f,0.04317f},{-0.0334f,0.9234f,0.3824f},{0.0270f,0.4349f},c});
    v.push_back({{0.40498f,0.13737f,0.02159f},{-0.0334f,0.9234f,0.3824f},{0.0619f,0.4463f},c});
    v.push_back({{0.39680f,0.12874f,0.04170f},{-0.0334f,0.9234f,0.3824f},{0.0278f,0.4653f},c});
    v.push_back({{0.41366f,0.05396f,0.08634f},{-0.0851f,0.1302f,0.9878f},{0.0000f,0.1143f},c});
    v.push_back({{0.38070f,0.07631f,0.08056f},{-0.0851f,0.1302f,0.9878f},{0.0623f,0.0724f},c});
    v.push_back({{0.37951f,0.05396f,0.08340f},{-0.0851f,0.1302f,0.9878f},{0.0631f,0.1143f},c});
    v.push_back({{0.41366f,0.11502f,0.06105f},{-0.0528f,0.7924f,0.6078f},{-0.0000f,0.4417f},c});
    v.push_back({{0.39680f,0.12874f,0.04170f},{-0.0528f,0.7924f,0.6078f},{0.0278f,0.4653f},c});
    v.push_back({{0.38969f,0.11502f,0.05897f},{-0.0528f,0.7924f,0.6078f},{0.0011f,0.4849f},c});
    v.push_back({{0.38969f,0.11502f,0.05897f},{-0.1482f,0.8044f,0.5753f},{0.0011f,0.4849f},c});
    v.push_back({{0.38038f,0.12874f,0.03739f},{-0.1482f,0.8044f,0.5753f},{0.0338f,0.4949f},c});
    v.push_back({{0.36602f,0.11502f,0.05287f},{-0.1482f,0.8044f,0.5753f},{0.0096f,0.5277f},c});
    v.push_back({{0.38419f,0.09714f,0.07223f},{-0.1917f,0.6245f,0.7571f},{0.0602f,0.0335f},c});
    v.push_back({{0.36602f,0.11502f,0.05287f},{-0.1917f,0.6245f,0.7571f},{0.1026f,-0.0000f},c});
    v.push_back({{0.35469f,0.09714f,0.06476f},{-0.1917f,0.6245f,0.7571f},{0.1172f,0.0335f},c});
    v.push_back({{0.38070f,0.07631f,0.08056f},{-0.2228f,0.3973f,0.8902f},{0.0623f,0.0724f},c});
    v.push_back({{0.35469f,0.09714f,0.06476f},{-0.2228f,0.3973f,0.8902f},{0.1172f,0.0335f},c});
    v.push_back({{0.34741f,0.07631f,0.07223f},{-0.2228f,0.3973f,0.8902f},{0.1266f,0.0724f},c});
    v.push_back({{0.39680f,0.12874f,0.04170f},{-0.0943f,0.9285f,0.3591f},{0.0278f,0.4653f},c});
    v.push_back({{0.39664f,0.13737f,0.01935f},{-0.0943f,0.9285f,0.3591f},{0.0650f,0.4613f},c});
    v.push_back({{0.38038f,0.12874f,0.03739f},{-0.0943f,0.9285f,0.3591f},{0.0338f,0.4949f},c});
    v.push_back({{0.37951f,0.05396f,0.08340f},{-0.2395f,0.1366f,0.9612f},{0.0631f,0.1143f},c});
    v.push_back({{0.34741f,0.07631f,0.07223f},{-0.2395f,0.1366f,0.9612f},{0.1266f,0.0724f},c});
    v.push_back({{0.34489f,0.05396f,0.07478f},{-0.2395f,0.1366f,0.9612f},{0.1298f,0.1143f},c});
    v.push_back({{0.34741f,0.07631f,0.07223f},{-0.3301f,0.4424f,0.8338f},{0.1266f,0.0724f},c});
    v.push_back({{0.32643f,0.09714f,0.05287f},{-0.3301f,0.4424f,0.8338f},{0.1741f,0.0335f},c});
    v.push_back({{0.31393f,0.07631f,0.05897f},{-0.3301f,0.4424f,0.8338f},{0.1936f,0.0724f},c});
    v.push_back({{0.38038f,0.12874f,0.03739f},{-0.1442f,0.9375f,0.3166f},{0.0338f,0.4949f},c});
    v.push_back({{0.38906f,0.13737f,0.01580f},{-0.1442f,0.9375f,0.3166f},{0.0707f,0.4750f},c});
    v.push_back({{0.36531f,0.12874f,0.03053f},{-0.1442f,0.9375f,0.3166f},{0.0447f,0.5221f},c});
    v.push_back({{0.34489f,0.05396f,0.07478f},{-0.3591f,0.1534f,0.9206f},{0.1298f,0.1143f},c});
    v.push_back({{0.31393f,0.07631f,0.05897f},{-0.3591f,0.1534f,0.9206f},{0.1936f,0.0724f},c});
    v.push_back({{0.30971f,0.05396f,0.06105f},{-0.3591f,0.1534f,0.9206f},{0.2001f,0.1143f},c});
    v.push_back({{0.36602f,0.11502f,0.05287f},{-0.2316f,0.8257f,0.5143f},{0.0096f,0.5277f},c});
    v.push_back({{0.36531f,0.12874f,0.03053f},{-0.2316f,0.8257f,0.5143f},{0.0447f,0.5221f},c});
    v.push_back({{0.34447f,0.11502f,0.04317f},{-0.2316f,0.8257f,0.5143f},{0.0249f,0.5666f},c});
    v.push_back({{0.35469f,0.09714f,0.06476f},{-0.2894f,0.6653f,0.6882f},{0.1172f,0.0335f},c});
    v.push_back({{0.34447f,0.11502f,0.04317f},{-0.2894f,0.6653f,0.6882f},{0.1464f,0.0000f},c});
    v.push_back({{0.32643f,0.09714f,0.05287f},{-0.2894f,0.6653f,0.6882f},{0.1741f,0.0335f},c});
    v.push_back({{0.32643f,0.09714f,0.05287f},{-0.3506f,0.7333f,0.5825f},{0.1741f,0.0335f},c});
    v.push_back({{0.32671f,0.11502f,0.03053f},{-0.3506f,0.7333f,0.5825f},{0.1847f,0.0000f},c});
    v.push_back({{0.30070f,0.09714f,0.03739f},{-0.3506f,0.7333f,0.5825f},{0.2282f,0.0335f},c});
    v.push_back({{0.31393f,0.07631f,0.05897f},{-0.3645f,0.5486f,0.7525f},{0.1936f,0.0724f},c});
    v.push_back({{0.30070f,0.09714f,0.03739f},{-0.3645f,0.5486f,0.7525f},{0.2282f,0.0335f},c});
    v.push_back({{0.27826f,0.07631f,0.04170f},{-0.3645f,0.5486f,0.7525f},{0.2665f,0.0724f},c});
    v.push_back({{0.36531f,0.12874f,0.03053f},{-0.2055f,0.9490f,0.2392f},{0.0447f,0.5221f},c});
    v.push_back({{0.38261f,0.13737f,0.01117f},{-0.2055f,0.9490f,0.2392f},{0.0784f,0.4866f},c});
    v.push_back({{0.35490f,0.12874f,0.02159f},{-0.2055f,0.9490f,0.2392f},{0.0598f,0.5409f},c});
    v.push_back({{0.30971f,0.05396f,0.06105f},{-0.3658f,0.2592f,0.8939f},{0.2001f,0.1143f},c});
    v.push_back({{0.27826f,0.07631f,0.04170f},{-0.3658f,0.2592f,0.8939f},{0.2665f,0.0724f},c});
    v.push_back({{0.26602f,0.05396f,0.04317f},{-0.3658f,0.2592f,0.8939f},{0.2879f,0.1143f},c});
    v.push_back({{0.34447f,0.11502f,0.04317f},{-0.2912f,0.8648f,0.4090f},{0.0249f,0.5666f},c});
    v.push_back({{0.35490f,0.12874f,0.02159f},{-0.2912f,0.8648f,0.4090f},{0.0598f,0.5409f},c});
    v.push_back({{0.32671f,0.11502f,0.03053f},{-0.2912f,0.8648f,0.4090f},{0.0459f,0.5986f},c});
    v.push_back({{0.26602f,0.05396f,0.04317f},{-0.6814f,0.1113f,0.7234f},{0.2879f,0.1143f},c});
    v.push_back({{0.24675f,0.07631f,0.02159f},{-0.6814f,0.1113f,0.7234f},{0.3334f,0.0725f},c});
    v.push_back({{0.24391f,0.05396f,0.02235f},{-0.6814f,0.1113f,0.7234f},{0.3382f,0.1143f},c});
    v.push_back({{0.32671f,0.11502f,0.03053f},{-0.3382f,0.8970f,0.2845f},{0.0459f,0.5986f},c});
    v.push_back({{0.34682f,0.12874f,0.01117f},{-0.3382f,0.8970f,0.2845f},{0.0779f,0.5555f},c});
    v.push_back({{0.31432f,0.11502f,0.01580f},{-0.3382f,0.8970f,0.2845f},{0.0714f,0.6210f},c});
    v.push_back({{0.30070f,0.09714f,0.03739f},{-0.3566f,0.8175f,0.4522f},{0.0378f,0.6544f},c});
    v.push_back({{0.31432f,0.11502f,0.01580f},{-0.3566f,0.8175f,0.4522f},{0.0714f,0.6210f},c});
    v.push_back({{0.27783f,0.09714f,0.01935f},{-0.3566f,0.8175f,0.4522f},{0.0681f,0.6957f},c});
    v.push_back({{0.27826f,0.07631f,0.04170f},{-0.4013f,0.6663f,0.6286f},{0.2665f,0.0724f},c});
    v.push_back({{0.27783f,0.09714f,0.01935f},{-0.4013f,0.6663f,0.6286f},{0.2785f,0.0335f},c});
    v.push_back({{0.24675f,0.07631f,0.02159f},{-0.4013f,0.6663f,0.6286f},{0.3334f,0.0725f},c});
    v.push_back({{0.35490f,0.12874f,0.02159f},{-0.2356f,0.9545f,0.1829f},{0.0598f,0.5409f},c});
    v.push_back({{0.37758f,0.13737f,0.00578f},{-0.2356f,0.9545f,0.1829f},{0.0876f,0.4957f},c});
    v.push_back({{0.34682f,0.12874f,0.01117f},{-0.2356f,0.9545f,0.1829f},{0.0779f,0.5555f},c});
    v.push_back({{0.24675f,0.07631f,0.02159f},{-0.7832f,0.6132f,0.1031f},{0.0690f,0.7621f},c});
    v.push_back({{0.26022f,0.09714f,-0.00000f},{-0.7832f,0.6132f,0.1031f},{0.1014f,0.7275f},c});
    v.push_back({{0.24391f,0.07631f,-0.00000f},{-0.7832f,0.6132f,0.1031f},{0.1081f,0.7673f},c});
    v.push_back({{0.34682f,0.12874f,0.01117f},{-0.2536f,0.9599f,0.1196f},{0.0779f,0.5555f},c});
    v.push_back({{0.37420f,0.13737f,0.00000f},{-0.2536f,0.9599f,0.1196f},{0.0978f,0.5018f},c});
    v.push_back({{0.34155f,0.12874f,0.00000f},{-0.2536f,0.9599f,0.1196f},{0.0977f,0.5650f},c});
    v.push_back({{0.24391f,0.05396f,0.02235f},{-0.9831f,0.1294f,0.1294f},{0.8415f,0.0459f},c});
    v.push_back({{0.24391f,0.07631f,-0.00000f},{-0.9831f,0.1294f,0.1294f},{0.8833f,0.0042f},c});
    v.push_back({{0.24097f,0.05396f,-0.00000f},{-0.9831f,0.1294f,0.1294f},{0.8833f,0.0463f},c});
    v.push_back({{0.31432f,0.11502f,0.01580f},{-0.3643f,0.9161f,0.1676f},{0.0714f,0.6210f},c});
    v.push_back({{0.34155f,0.12874f,0.00000f},{-0.3643f,0.9161f,0.1676f},{0.0977f,0.5650f},c});
    v.push_back({{0.30705f,0.11502f,0.00000f},{-0.3643f,0.9161f,0.1676f},{0.0994f,0.6341f},c});
    v.push_back({{0.27783f,0.09714f,0.01935f},{-0.3393f,0.8885f,0.3088f},{0.0681f,0.6957f},c});
    v.push_back({{0.30705f,0.11502f,0.00000f},{-0.3393f,0.8885f,0.3088f},{0.0994f,0.6341f},c});
    v.push_back({{0.26022f,0.09714f,-0.00000f},{-0.3393f,0.8885f,0.3088f},{0.1014f,0.7275f},c});
    v.push_back({{0.30705f,0.11502f,0.00000f},{-0.3393f,0.8885f,-0.3088f},{0.0994f,0.6341f},c});
    v.push_back({{0.27783f,0.09714f,-0.01935f},{-0.3393f,0.8885f,-0.3088f},{0.1387f,0.6957f},c});
    v.push_back({{0.26022f,0.09714f,-0.00000f},{-0.3393f,0.8885f,-0.3088f},{0.1014f,0.7275f},c});
    v.push_back({{0.26022f,0.09714f,-0.00000f},{-0.7832f,0.6132f,-0.1031f},{0.1014f,0.7275f},c});
    v.push_back({{0.24675f,0.07631f,-0.02159f},{-0.7832f,0.6132f,-0.1031f},{0.1478f,0.7621f},c});
    v.push_back({{0.24391f,0.07631f,-0.00000f},{-0.7832f,0.6132f,-0.1031f},{0.1081f,0.7673f},c});
    v.push_back({{0.37420f,0.13737f,0.00000f},{-0.2536f,0.9599f,-0.1196f},{0.0978f,0.5018f},c});
    v.push_back({{0.34682f,0.12874f,-0.01117f},{-0.2536f,0.9599f,-0.1196f},{0.1186f,0.5555f},c});
    v.push_back({{0.34155f,0.12874f,0.00000f},{-0.2536f,0.9599f,-0.1196f},{0.0977f,0.5650f},c});
    v.push_back({{0.24097f,0.05396f,-0.00000f},{-0.9831f,0.1294f,-0.1294f},{0.8833f,0.0463f},c});
    v.push_back({{0.24675f,0.07631f,-0.02159f},{-0.9831f,0.1294f,-0.1294f},{0.9237f,0.0039f},c});
    v.push_back({{0.24391f,0.05396f,-0.02235f},{-0.9831f,0.1294f,-0.1294f},{0.9252f,0.0459f},c});
    v.push_back({{0.34155f,0.12874f,0.00000f},{-0.3643f,0.9161f,-0.1676f},{0.0977f,0.5650f},c});
    v.push_back({{0.31432f,0.11502f,-0.01580f},{-0.3643f,0.9161f,-0.1676f},{0.1290f,0.6210f},c});
    v.push_back({{0.30705f,0.11502f,0.00000f},{-0.3643f,0.9161f,-0.1676f},{0.0994f,0.6341f},c});
    v.push_back({{0.24675f,0.07631f,-0.02159f},{-0.6814f,0.1113f,-0.7234f},{0.9237f,0.0039f},c});
    v.push_back({{0.26602f,0.05396f,-0.04317f},{-0.6814f,0.1113f,-0.7234f},{0.9641f,0.0432f},c});
    v.push_back({{0.24391f,0.05396f,-0.02235f},{-0.6814f,0.1113f,-0.7234f},{0.9252f,0.0459f},c});
    v.push_back({{0.34682f,0.12874f,-0.01117f},{-0.3382f,0.8970f,-0.2845f},{0.1186f,0.5555f},c});
    v.push_back({{0.32671f,0.11502f,-0.03053f},{-0.3382f,0.8970f,-0.2845f},{0.1573f,0.5986f},c});
    v.push_back({{0.31432f,0.11502f,-0.01580f},{-0.3382f,0.8970f,-0.2845f},{0.1290f,0.6210f},c});
    v.push_back({{0.31432f,0.11502f,-0.01580f},{-0.3566f,0.8175f,-0.4522f},{0.1290f,0.6210f},c});
    v.push_back({{0.30070f,0.09714f,-0.03739f},{-0.3566f,0.8175f,-0.4522f},{0.1741f,0.6544f},c});
    v.push_back({{0.27783f,0.09714f,-0.01935f},{-0.3566f,0.8175f,-0.4522f},{0.1387f,0.6957f},c});
    v.push_back({{0.27783f,0.09714f,-0.01935f},{-0.4013f,0.6663f,-0.6286f},{0.1387f,0.6957f},c});
    v.push_back({{0.27826f,0.07631f,-0.04170f},{-0.4013f,0.6663f,-0.6286f},{0.1880f,0.7053f},c});
    v.push_back({{0.24675f,0.07631f,-0.02159f},{-0.4013f,0.6663f,-0.6286f},{0.1478f,0.7621f},c});
    v.push_back({{0.37758f,0.13737f,-0.00578f},{-0.2356f,0.9545f,-0.1829f},{0.1087f,0.4957f},c});
    v.push_back({{0.35490f,0.12874f,-0.02159f},{-0.2356f,0.9545f,-0.1829f},{0.1385f,0.5409f},c});
    v.push_back({{0.34682f,0.12874f,-0.01117f},{-0.2356f,0.9545f,-0.1829f},{0.1186f,0.5555f},c});
    v.push_back({{0.30070f,0.09714f,-0.03739f},{-0.3645f,0.5486f,-0.7525f},{0.1741f,0.6544f},c});
    v.push_back({{0.31393f,0.07631f,-0.05897f},{-0.3645f,0.5486f,-0.7525f},{0.2235f,0.6409f},c});
    v.push_back({{0.27826f,0.07631f,-0.04170f},{-0.3645f,0.5486f,-0.7525f},{0.1880f,0.7053f},c});
    v.push_back({{0.38261f,0.13737f,-0.01117f},{-0.2055f,0.9490f,-0.2392f},{0.1191f,0.4866f},c});
    v.push_back({{0.36531f,0.12874f,-0.03053f},{-0.2055f,0.9490f,-0.2392f},{0.1560f,0.5221f},c});
    v.push_back({{0.35490f,0.12874f,-0.02159f},{-0.2055f,0.9490f,-0.2392f},{0.1385f,0.5409f},c});
    v.push_back({{0.27826f,0.07631f,-0.04170f},{-0.3658f,0.2592f,-0.8939f},{0.6295f,0.0391f},c});
    v.push_back({{0.30971f,0.05396f,-0.06105f},{-0.3658f,0.2592f,-0.8939f},{0.6706f,0.0809f},c});
    v.push_back({{0.26602f,0.05396f,-0.04317f},{-0.3658f,0.2592f,-0.8939f},{0.6072f,0.0809f},c});
    v.push_back({{0.35490f,0.12874f,-0.02159f},{-0.2912f,0.8648f,-0.4090f},{0.1385f,0.5409f},c});
    v.push_back({{0.34447f,0.11502f,-0.04317f},{-0.2912f,0.8648f,-0.4090f},{0.1823f,0.5666f},c});
    v.push_back({{0.32671f,0.11502f,-0.03053f},{-0.2912f,0.8648f,-0.4090f},{0.1573f,0.5986f},c});
    v.push_back({{0.32671f,0.11502f,-0.03053f},{-0.3506f,0.7333f,-0.5825f},{0.1573f,0.5986f},c});
    v.push_back({{0.32643f,0.09714f,-0.05287f},{-0.3506f,0.7333f,-0.5825f},{0.2053f,0.6080f},c});
    v.push_back({{0.30070f,0.09714f,-0.03739f},{-0.3506f,0.7333f,-0.5825f},{0.1741f,0.6544f},c});
    v.push_back({{0.34447f,0.11502f,-0.04317f},{-0.2894f,0.6653f,-0.6882f},{0.1823f,0.5666f},c});
    v.push_back({{0.35469f,0.09714f,-0.06476f},{-0.2894f,0.6653f,-0.6882f},{0.2301f,0.5570f},c});
    v.push_back({{0.32643f,0.09714f,-0.05287f},{-0.2894f,0.6653f,-0.6882f},{0.2053f,0.6080f},c});
    v.push_back({{0.32643f,0.09714f,-0.05287f},{-0.3301f,0.4424f,-0.8338f},{0.2053f,0.6080f},c});
    v.push_back({{0.34741f,0.07631f,-0.07223f},{-0.3301f,0.4424f,-0.8338f},{0.2514f,0.5805f},c});
    v.push_back({{0.31393f,0.07631f,-0.05897f},{-0.3301f,0.4424f,-0.8338f},{0.2235f,0.6409f},c});
    v.push_back({{0.38906f,0.13737f,-0.01580f},{-0.1442f,0.9375f,-0.3166f},{0.1283f,0.4750f},c});
    v.push_back({{0.38038f,0.12874f,-0.03739f},{-0.1442f,0.9375f,-0.3166f},{0.1702f,0.4949f},c});
    v.push_back({{0.36531f,0.12874f,-0.03053f},{-0.1442f,0.9375f,-0.3166f},{0.1560f,0.5221f},c});
    v.push_back({{0.31393f,0.07631f,-0.05897f},{-0.3591f,0.1534f,-0.9206f},{0.6793f,0.0390f},c});
    v.push_back({{0.34489f,0.05396f,-0.07478f},{-0.3591f,0.1534f,-0.9206f},{0.7221f,0.0808f},c});
    v.push_back({{0.30971f,0.05396f,-0.06105f},{-0.3591f,0.1534f,-0.9206f},{0.6706f,0.0809f},c});
    v.push_back({{0.36531f,0.12874f,-0.03053f},{-0.2316f,0.8257f,-0.5143f},{0.1560f,0.5221f},c});
    v.push_back({{0.36602f,0.11502f,-0.05287f},{-0.2316f,0.8257f,-0.5143f},{0.2024f,0.5277f},c});
    v.push_back({{0.34447f,0.11502f,-0.04317f},{-0.2316f,0.8257f,-0.5143f},{0.1823f,0.5666f},c});
    v.push_back({{0.34741f,0.07631f,-0.07223f},{-0.2395f,0.1366f,-0.9612f},{0.7282f,0.0390f},c});
    v.push_back({{0.37951f,0.05396f,-0.08340f},{-0.2395f,0.1366f,-0.9612f},{0.7762f,0.0808f},c});
    v.push_back({{0.34489f,0.05396f,-0.07478f},{-0.2395f,0.1366f,-0.9612f},{0.7221f,0.0808f},c});
    v.push_back({{0.38038f,0.12874f,-0.03739f},{-0.1482f,0.8044f,-0.5753f},{0.1702f,0.4949f},c});
    v.push_back({{0.38969f,0.11502f,-0.05897f},{-0.1482f,0.8044f,-0.5753f},{0.2162f,0.4849f},c});
    v.push_back({{0.36602f,0.11502f,-0.05287f},{-0.1482f,0.8044f,-0.5753f},{0.2024f,0.5277f},c});
    v.push_back({{0.36602f,0.11502f,-0.05287f},{-0.1917f,0.6245f,-0.7571f},{0.2024f,0.5277f},c});
    v.push_back({{0.38419f,0.09714f,-0.07223f},{-0.1917f,0.6245f,-0.7571f},{0.2470f,0.5037f},c});
    v.push_back({{0.35469f,0.09714f,-0.06476f},{-0.1917f,0.6245f,-0.7571f},{0.2301f,0.5570f},c});
    v.push_back({{0.35469f,0.09714f,-0.06476f},{-0.2228f,0.3973f,-0.8902f},{0.7461f,0.0000f},c});
    v.push_back({{0.38070f,0.07631f,-0.08056f},{-0.2228f,0.3973f,-0.8902f},{0.7802f,0.0390f},c});
    v.push_back({{0.34741f,0.07631f,-0.07223f},{-0.2228f,0.3973f,-0.8902f},{0.7282f,0.0390f},c});
    v.push_back({{0.39664f,0.13737f,-0.01935f},{-0.0943f,0.9285f,-0.3591f},{0.1356f,0.4613f},c});
    v.push_back({{0.39680f,0.12874f,-0.04170f},{-0.0943f,0.9285f,-0.3591f},{0.1799f,0.4653f},c});
    v.push_back({{0.38038f,0.12874f,-0.03739f},{-0.0943f,0.9285f,-0.3591f},{0.1702f,0.4949f},c});
    v.push_back({{0.38419f,0.09714f,-0.07223f},{-0.0794f,0.3817f,-0.9209f},{0.7921f,0.0000f},c});
    v.push_back({{0.41366f,0.07631f,-0.08340f},{-0.0794f,0.3817f,-0.9209f},{0.8355f,0.0390f},c});
    v.push_back({{0.38070f,0.07631f,-0.08056f},{-0.0794f,0.3817f,-0.9209f},{0.7802f,0.0390f},c});
    v.push_back({{0.40498f,0.13737f,-0.02159f},{-0.0334f,0.9234f,-0.3824f},{0.1406f,0.4463f},c});
    v.push_back({{0.41366f,0.12874f,-0.04317f},{-0.0334f,0.9234f,-0.3824f},{0.1845f,0.4349f},c});
    v.push_back({{0.39680f,0.12874f,-0.04170f},{-0.0334f,0.9234f,-0.3824f},{0.1799f,0.4653f},c});
    v.push_back({{0.38070f,0.07631f,-0.08056f},{-0.0851f,0.1302f,-0.9878f},{0.7802f,0.0390f},c});
    v.push_back({{0.41366f,0.05396f,-0.08634f},{-0.0851f,0.1302f,-0.9878f},{0.8334f,0.0808f},c});
    v.push_back({{0.37951f,0.05396f,-0.08340f},{-0.0851f,0.1302f,-0.9878f},{0.7762f,0.0808f},c});
    v.push_back({{0.39680f,0.12874f,-0.04170f},{-0.0528f,0.7924f,-0.6078f},{0.1799f,0.4653f},c});
    v.push_back({{0.41366f,0.11502f,-0.06105f},{-0.0528f,0.7924f,-0.6078f},{0.2227f,0.4417f},c});
    v.push_back({{0.38969f,0.11502f,-0.05897f},{-0.0528f,0.7924f,-0.6078f},{0.2162f,0.4849f},c});
    v.push_back({{0.38969f,0.11502f,-0.05897f},{-0.0684f,0.6076f,-0.7913f},{0.2162f,0.4849f},c});
    v.push_back({{0.41366f,0.09714f,-0.07478f},{-0.0684f,0.6076f,-0.7913f},{0.2550f,0.4506f},c});
    v.push_back({{0.38419f,0.09714f,-0.07223f},{-0.0684f,0.6076f,-0.7913f},{0.2470f,0.5037f},c});
    v.push_back({{0.41366f,-0.09714f,-0.07478f},{0.1039f,-0.6055f,-0.7891f},{0.8481f,0.8060f},c});
    v.push_back({{0.42946f,-0.11502f,-0.05897f},{0.1039f,-0.6055f,-0.7891f},{0.8865f,0.8394f},c});
    v.push_back({{0.41366f,-0.11502f,-0.06105f},{0.1039f,-0.6055f,-0.7891f},{0.8576f,0.8394f},c});
    v.push_back({{0.41366f,-0.11502f,-0.06105f},{0.0799f,-0.7908f,-0.6068f},{0.0936f,0.7673f},c});
    v.push_back({{0.42483f,-0.12874f,-0.04170f},{0.0799f,-0.7908f,-0.6068f},{0.1149f,0.8103f},c});
    v.push_back({{0.41366f,-0.12874f,-0.04317f},{0.0799f,-0.7908f,-0.6068f},{0.0952f,0.8060f},c});
    v.push_back({{0.41366f,-0.05396f,-0.08634f},{0.1294f,-0.1294f,-0.9831f},{0.8400f,0.7251f},c});
    v.push_back({{0.43524f,-0.07631f,-0.08056f},{0.1294f,-0.1294f,-0.9831f},{0.8815f,0.7670f},c});
    v.push_back({{0.41366f,-0.07631f,-0.08340f},{0.1294f,-0.1294f,-0.9831f},{0.8420f,0.7670f},c});
    v.push_back({{0.41366f,-0.12874f,-0.04317f},{0.0503f,-0.9227f,-0.3822f},{0.0952f,0.8060f},c});
    v.push_back({{0.41944f,-0.13737f,-0.02159f},{0.0503f,-0.9227f,-0.3822f},{0.1038f,0.8498f},c});
    v.push_back({{0.41366f,-0.13737f,-0.02235f},{0.0503f,-0.9227f,-0.3822f},{0.0936f,0.8476f},c});
    v.push_back({{0.41366f,-0.09714f,-0.07478f},{0.1207f,-0.3799f,-0.9171f},{0.8481f,0.8060f},c});
    v.push_back({{0.43524f,-0.07631f,-0.08056f},{0.1207f,-0.3799f,-0.9171f},{0.8815f,0.7670f},c});
    v.push_back({{0.43301f,-0.09714f,-0.07223f},{0.1207f,-0.3799f,-0.9171f},{0.8835f,0.8059f},c});
    v.push_back({{0.43600f,-0.05396f,-0.08340f},{0.3795f,-0.1294f,-0.9161f},{0.8809f,0.7251f},c});
    v.push_back({{0.45536f,-0.07631f,-0.07223f},{0.3795f,-0.1294f,-0.9161f},{0.9223f,0.7670f},c});
    v.push_back({{0.43524f,-0.07631f,-0.08056f},{0.3795f,-0.1294f,-0.9161f},{0.8815f,0.7670f},c});
    v.push_back({{0.42483f,-0.12874f,-0.04170f},{0.1475f,-0.9227f,-0.3562f},{0.1149f,0.8103f},c});
    v.push_back({{0.42483f,-0.13737f,-0.01935f},{0.1475f,-0.9227f,-0.3562f},{0.1129f,0.8547f},c});
    v.push_back({{0.41944f,-0.13737f,-0.02159f},{0.1475f,-0.9227f,-0.3562f},{0.1038f,0.8498f},c});
    v.push_back({{0.43524f,-0.07631f,-0.08056f},{0.3540f,-0.3799f,-0.8546f},{0.8815f,0.7670f},c});
    v.push_back({{0.45104f,-0.09714f,-0.06476f},{0.3540f,-0.3799f,-0.8546f},{0.9200f,0.8059f},c});
    v.push_back({{0.43301f,-0.09714f,-0.07223f},{0.3540f,-0.3799f,-0.8546f},{0.8835f,0.8059f},c});
    v.push_back({{0.43301f,-0.09714f,-0.07223f},{0.3046f,-0.6055f,-0.7353f},{0.8835f,0.8059f},c});
    v.push_back({{0.44418f,-0.11502f,-0.05287f},{0.3046f,-0.6055f,-0.7353f},{0.9164f,0.8394f},c});
    v.push_back({{0.42946f,-0.11502f,-0.05897f},{0.3046f,-0.6055f,-0.7353f},{0.8865f,0.8394f},c});
    v.push_back({{0.42483f,-0.12874f,-0.04170f},{0.2342f,-0.7908f,-0.5655f},{0.1149f,0.8103f},c});
    v.push_back({{0.44418f,-0.11502f,-0.05287f},{0.2342f,-0.7908f,-0.5655f},{0.1462f,0.7866f},c});
    v.push_back({{0.43524f,-0.12874f,-0.03739f},{0.2342f,-0.7908f,-0.5655f},{0.1324f,0.8196f},c});
    v.push_back({{0.44418f,-0.11502f,-0.05287f},{0.3726f,-0.7908f,-0.4856f},{0.6159f,0.6410f},c});
    v.push_back({{0.44418f,-0.12874f,-0.03053f},{0.3726f,-0.7908f,-0.4856f},{0.6378f,0.6833f},c});
    v.push_back({{0.43524f,-0.12874f,-0.03739f},{0.3726f,-0.7908f,-0.4856f},{0.6202f,0.6830f},c});
    v.push_back({{0.45683f,-0.05396f,-0.07478f},{0.6036f,-0.1294f,-0.7867f},{0.9230f,0.7251f},c});
    v.push_back({{0.47263f,-0.07631f,-0.05897f},{0.6036f,-0.1294f,-0.7867f},{0.9615f,0.7670f},c});
    v.push_back({{0.45536f,-0.07631f,-0.07223f},{0.6036f,-0.1294f,-0.7867f},{0.9223f,0.7670f},c});
    v.push_back({{0.43524f,-0.12874f,-0.03739f},{0.2347f,-0.9227f,-0.3058f},{0.1324f,0.8196f},c});
    v.push_back({{0.42946f,-0.13737f,-0.01580f},{0.2347f,-0.9227f,-0.3058f},{0.1202f,0.8618f},c});
    v.push_back({{0.42483f,-0.13737f,-0.01935f},{0.2347f,-0.9227f,-0.3058f},{0.1129f,0.8547f},c});
    v.push_back({{0.45104f,-0.09714f,-0.06476f},{0.5631f,-0.3799f,-0.7339f},{0.9200f,0.8059f},c});
    v.push_back({{0.47263f,-0.07631f,-0.05897f},{0.5631f,-0.3799f,-0.7339f},{0.9615f,0.7670f},c});
    v.push_back({{0.46653f,-0.09714f,-0.05287f},{0.5631f,-0.3799f,-0.7339f},{0.9552f,0.8060f},c});
    v.push_back({{0.45104f,-0.09714f,-0.06476f},{0.4845f,-0.6055f,-0.6314f},{0.6185f,0.5999f},c});
    v.push_back({{0.45683f,-0.11502f,-0.04317f},{0.4845f,-0.6055f,-0.6314f},{0.6408f,0.6415f},c});
    v.push_back({{0.44418f,-0.11502f,-0.05287f},{0.4845f,-0.6055f,-0.6314f},{0.6159f,0.6410f},c});
    v.push_back({{0.47263f,-0.07631f,-0.05897f},{0.7339f,-0.3799f,-0.5631f},{0.6617f,0.5629f},c});
    v.push_back({{0.47841f,-0.09714f,-0.03739f},{0.7339f,-0.3799f,-0.5631f},{0.6817f,0.6081f},c});
    v.push_back({{0.46653f,-0.09714f,-0.05287f},{0.7339f,-0.3799f,-0.5631f},{0.6490f,0.6004f},c});
    v.push_back({{0.46653f,-0.09714f,-0.05287f},{0.6314f,-0.6055f,-0.4845f},{0.6490f,0.6004f},c});
    v.push_back({{0.46653f,-0.11502f,-0.03053f},{0.6314f,-0.6055f,-0.4845f},{0.6676f,0.6478f},c});
    v.push_back({{0.45683f,-0.11502f,-0.04317f},{0.6314f,-0.6055f,-0.4845f},{0.6408f,0.6415f},c});
    v.push_back({{0.44418f,-0.12874f,-0.03053f},{0.4856f,-0.7908f,-0.3726f},{0.6378f,0.6833f},c});
    v.push_back({{0.46653f,-0.11502f,-0.03053f},{0.4856f,-0.7908f,-0.3726f},{0.6676f,0.6478f},c});
    v.push_back({{0.45104f,-0.12874f,-0.02159f},{0.4856f,-0.7908f,-0.3726f},{0.6567f,0.6878f},c});
    v.push_back({{0.47471f,-0.05396f,-0.06105f},{0.7867f,-0.1294f,-0.6036f},{0.9637f,0.7251f},c});
    v.push_back({{0.48588f,-0.07631f,-0.04170f},{0.7867f,-0.1294f,-0.6036f},{0.9966f,0.7670f},c});
    v.push_back({{0.47263f,-0.07631f,-0.05897f},{0.7867f,-0.1294f,-0.6036f},{0.9615f,0.7670f},c});
    v.push_back({{0.44418f,-0.12874f,-0.03053f},{0.3058f,-0.9227f,-0.2347f},{0.6378f,0.6833f},c});
    v.push_back({{0.43301f,-0.13737f,-0.01117f},{0.3058f,-0.9227f,-0.2347f},{0.6500f,0.7254f},c});
    v.push_back({{0.42946f,-0.13737f,-0.01580f},{0.3058f,-0.9227f,-0.2347f},{0.6402f,0.7231f},c});
    v.push_back({{0.48843f,-0.05396f,-0.04317f},{0.9161f,-0.1294f,-0.3795f},{0.7161f,0.5404f},c});
    v.push_back({{0.49422f,-0.07631f,-0.02159f},{0.9161f,-0.1294f,-0.3795f},{0.7349f,0.5875f},c});
    v.push_back({{0.48588f,-0.07631f,-0.04170f},{0.9161f,-0.1294f,-0.3795f},{0.6983f,0.5715f},c});
    v.push_back({{0.43301f,-0.13737f,-0.01117f},{0.3562f,-0.9227f,-0.1475f},{0.6500f,0.7254f},c});
    v.push_back({{0.45536f,-0.12874f,-0.01117f},{0.3562f,-0.9227f,-0.1475f},{0.6756f,0.6961f},c});
    v.push_back({{0.43524f,-0.13737f,-0.00578f},{0.3562f,-0.9227f,-0.1475f},{0.6598f,0.7297f},c});
    v.push_back({{0.48588f,-0.07631f,-0.04170f},{0.8546f,-0.3799f,-0.3540f},{0.6983f,0.5715f},c});
    v.push_back({{0.48588f,-0.09714f,-0.01935f},{0.8546f,-0.3799f,-0.3540f},{0.7145f,0.6225f},c});
    v.push_back({{0.47841f,-0.09714f,-0.03739f},{0.8546f,-0.3799f,-0.3540f},{0.6817f,0.6081f},c});
    v.push_back({{0.47841f,-0.09714f,-0.03739f},{0.7353f,-0.6055f,-0.3046f},{0.6817f,0.6081f},c});
    v.push_back({{0.47263f,-0.11502f,-0.01580f},{0.7353f,-0.6055f,-0.3046f},{0.6943f,0.6595f},c});
    v.push_back({{0.46653f,-0.11502f,-0.03053f},{0.7353f,-0.6055f,-0.3046f},{0.6676f,0.6478f},c});
    v.push_back({{0.45104f,-0.12874f,-0.02159f},{0.5655f,-0.7908f,-0.2342f},{0.6567f,0.6878f},c});
    v.push_back({{0.47263f,-0.11502f,-0.01580f},{0.5655f,-0.7908f,-0.2342f},{0.6943f,0.6595f},c});
    v.push_back({{0.45536f,-0.12874f,-0.01117f},{0.5655f,-0.7908f,-0.2342f},{0.6756f,0.6961f},c});
    v.push_back({{0.48588f,-0.09714f,-0.01935f},{0.7891f,-0.6055f,-0.1039f},{0.7145f,0.6225f},c});
    v.push_back({{0.47471f,-0.11502f,0.00000f},{0.7891f,-0.6055f,-0.1039f},{0.7193f,0.6758f},c});
    v.push_back({{0.47263f,-0.11502f,-0.01580f},{0.7891f,-0.6055f,-0.1039f},{0.6943f,0.6595f},c});
    v.push_back({{0.45536f,-0.12874f,-0.01117f},{0.6068f,-0.7908f,-0.0799f},{0.6756f,0.6961f},c});
    v.push_back({{0.47471f,-0.11502f,0.00000f},{0.6068f,-0.7908f,-0.0799f},{0.7193f,0.6758f},c});
    v.push_back({{0.45683f,-0.12874f,0.00000f},{0.6068f,-0.7908f,-0.0799f},{0.6933f,0.7076f},c});
    v.push_back({{0.49706f,-0.05396f,-0.02235f},{0.9831f,-0.1294f,-0.1294f},{0.7540f,0.5570f},c});
    v.push_back({{0.49706f,-0.07631f,0.00000f},{0.9831f,-0.1294f,-0.1294f},{0.7690f,0.6098f},c});
    v.push_back({{0.49422f,-0.07631f,-0.02159f},{0.9831f,-0.1294f,-0.1294f},{0.7349f,0.5875f},c});
    v.push_back({{0.45536f,-0.12874f,-0.01117f},{0.3822f,-0.9227f,-0.0503f},{0.6756f,0.6961f},c});
    v.push_back({{0.43600f,-0.13737f,0.00000f},{0.3822f,-0.9227f,-0.0503f},{0.6689f,0.7356f},c});
    v.push_back({{0.43524f,-0.13737f,-0.00578f},{0.3822f,-0.9227f,-0.0503f},{0.6598f,0.7297f},c});
    v.push_back({{0.48588f,-0.09714f,-0.01935f},{0.9171f,-0.3799f,-0.1207f},{0.7145f,0.6225f},c});
    v.push_back({{0.49706f,-0.07631f,0.00000f},{0.9171f,-0.3799f,-0.1207f},{0.7690f,0.6098f},c});
    v.push_back({{0.48843f,-0.09714f,0.00000f},{0.9171f,-0.3799f,-0.1207f},{0.7451f,0.6425f},c});
    v.push_back({{0.48843f,-0.09714f,0.00000f},{0.9171f,-0.3799f,0.1207f},{0.7451f,0.6425f},c});
    v.push_back({{0.49422f,-0.07631f,0.02159f},{0.9171f,-0.3799f,0.1207f},{0.7983f,0.6369f},c});
    v.push_back({{0.48588f,-0.09714f,0.01935f},{0.9171f,-0.3799f,0.1207f},{0.7714f,0.6667f},c});
    v.push_back({{0.48843f,-0.09714f,0.00000f},{0.7891f,-0.6055f,0.1039f},{0.7451f,0.6425f},c});
    v.push_back({{0.47263f,-0.11502f,0.01580f},{0.7891f,-0.6055f,0.1039f},{0.7408f,0.6956f},c});
    v.push_back({{0.47471f,-0.11502f,0.00000f},{0.7891f,-0.6055f,0.1039f},{0.7193f,0.6758f},c});
    v.push_back({{0.47471f,-0.11502f,0.00000f},{0.6068f,-0.7908f,0.0799f},{0.7193f,0.6758f},c});
    v.push_back({{0.45536f,-0.12874f,0.01117f},{0.6068f,-0.7908f,0.0799f},{0.7085f,0.7216f},c});
    v.push_back({{0.45683f,-0.12874f,0.00000f},{0.6068f,-0.7908f,0.0799f},{0.6933f,0.7076f},c});
    v.push_back({{0.50000f,-0.05396f,0.00000f},{0.9831f,-0.1294f,0.1294f},{0.7893f,0.5801f},c});
    v.push_back({{0.49422f,-0.07631f,0.02159f},{0.9831f,-0.1294f,0.1294f},{0.7983f,0.6369f},c});
    v.push_back({{0.49706f,-0.07631f,0.00000f},{0.9831f,-0.1294f,0.1294f},{0.7690f,0.6098f},c});
    v.push_back({{0.45683f,-0.12874f,0.00000f},{0.3822f,-0.9227f,0.0503f},{0.6933f,0.7076f},c});
    v.push_back({{0.43524f,-0.13737f,0.00578f},{0.3822f,-0.9227f,0.0503f},{0.6767f,0.7429f},c});
    v.push_back({{0.43600f,-0.13737f,0.00000f},{0.3822f,-0.9227f,0.0503f},{0.6689f,0.7356f},c});
    v.push_back({{0.47263f,-0.11502f,0.01580f},{0.5655f,-0.7908f,0.2342f},{0.7408f,0.6956f},c});
    v.push_back({{0.45104f,-0.12874f,0.02159f},{0.5655f,-0.7908f,0.2342f},{0.7202f,0.7372f},c});
    v.push_back({{0.45536f,-0.12874f,0.01117f},{0.5655f,-0.7908f,0.2342f},{0.7085f,0.7216f},c});
    v.push_back({{0.49706f,-0.05396f,0.02235f},{0.9161f,-0.1294f,0.3795f},{0.4898f,0.3732f},c});
    v.push_back({{0.48588f,-0.07631f,0.04170f},{0.9161f,-0.1294f,0.3795f},{0.5347f,0.4044f},c});
    v.push_back({{0.49422f,-0.07631f,0.02159f},{0.9161f,-0.1294f,0.3795f},{0.4942f,0.4088f},c});
    v.push_back({{0.45536f,-0.12874f,0.01117f},{0.3562f,-0.9227f,0.1475f},{0.7085f,0.7216f},c});
    v.push_back({{0.43301f,-0.13737f,0.01117f},{0.3562f,-0.9227f,0.1475f},{0.6828f,0.7510f},c});
    v.push_back({{0.43524f,-0.13737f,0.00578f},{0.3562f,-0.9227f,0.1475f},{0.6767f,0.7429f},c});
    v.push_back({{0.49422f,-0.07631f,0.02159f},{0.8546f,-0.3799f,0.3540f},{0.7983f,0.6369f},c});
    v.push_back({{0.47841f,-0.09714f,0.03739f},{0.8546f,-0.3799f,0.3540f},{0.7916f,0.6937f},c});
    v.push_back({{0.48588f,-0.09714f,0.01935f},{0.8546f,-0.3799f,0.3540f},{0.7714f,0.6667f},c});
    v.push_back({{0.48588f,-0.09714f,0.01935f},{0.7353f,-0.6055f,0.3046f},{0.7714f,0.6667f},c});
    v.push_back({{0.46653f,-0.11502f,0.03053f},{0.7353f,-0.6055f,0.3046f},{0.7573f,0.7176f},c});
    v.push_back({{0.47263f,-0.11502f,0.01580f},{0.7353f,-0.6055f,0.3046f},{0.7408f,0.6956f},c});
    v.push_back({{0.47841f,-0.09714f,0.03739f},{0.6314f,-0.6055f,0.4845f},{0.7916f,0.6937f},c});
    v.push_back({{0.45683f,-0.11502f,0.04317f},{0.6314f,-0.6055f,0.4845f},{0.7677f,0.7403f},c});
    v.push_back({{0.46653f,-0.11502f,0.03053f},{0.6314f,-0.6055f,0.4845f},{0.7573f,0.7176f},c});
    v.push_back({{0.46653f,-0.11502f,0.03053f},{0.4856f,-0.7908f,0.3726f},{0.7573f,0.7176f},c});
    v.push_back({{0.44418f,-0.12874f,0.03053f},{0.4856f,-0.7908f,0.3726f},{0.7275f,0.7532f},c});
    v.push_back({{0.45104f,-0.12874f,0.02159f},{0.4856f,-0.7908f,0.3726f},{0.7202f,0.7372f},c});
    v.push_back({{0.48843f,-0.05396f,0.04317f},{0.7867f,-0.1294f,0.6036f},{0.5318f,0.3686f},c});
    v.push_back({{0.47263f,-0.07631f,0.05897f},{0.7867f,-0.1294f,0.6036f},{0.5732f,0.3958f},c});
    v.push_back({{0.48588f,-0.07631f,0.04170f},{0.7867f,-0.1294f,0.6036f},{0.5347f,0.4044f},c});
    v.push_back({{0.45104f,-0.12874f,0.02159f},{0.3058f,-0.9227f,0.2347f},{0.7202f,0.7372f},c});
    v.push_back({{0.42946f,-0.13737f,0.01580f},{0.3058f,-0.9227f,0.2347f},{0.6866f,0.7592f},c});
    v.push_back({{0.43301f,-0.13737f,0.01117f},{0.3058f,-0.9227f,0.2347f},{0.6828f,0.7510f},c});
    v.push_back({{0.47841f,-0.09714f,0.03739f},{0.7339f,-0.3799f,0.5631f},{0.7916f,0.6937f},c});
    v.push_back({{0.47263f,-0.07631f,0.05897f},{0.7339f,-0.3799f,0.5631f},{0.8351f,0.6978f},c});
    v.push_back({{0.46653f,-0.09714f,0.05287f},{0.7339f,-0.3799f,0.5631f},{0.8044f,0.7214f},c});
    v.push_back({{0.44418f,-0.12874f,0.03053f},{0.2347f,-0.9227f,0.3058f},{0.1924f,0.7726f},c});
    v.push_back({{0.42483f,-0.13737f,0.01935f},{0.2347f,-0.9227f,0.3058f},{0.1592f,0.7778f},c});
    v.push_back({{0.42946f,-0.13737f,0.01580f},{0.2347f,-0.9227f,0.3058f},{0.1620f,0.7673f},c});
    v.push_back({{0.46653f,-0.09714f,0.05287f},{0.5631f,-0.3799f,0.7339f},{0.8044f,0.7214f},c});
    v.push_back({{0.45536f,-0.07631f,0.07223f},{0.5631f,-0.3799f,0.7339f},{0.8400f,0.7275f},c});
    v.push_back({{0.45104f,-0.09714f,0.06476f},{0.5631f,-0.3799f,0.7339f},{0.8088f,0.7480f},c});
    v.push_back({{0.45683f,-0.11502f,0.04317f},{0.4845f,-0.6055f,0.6314f},{0.7677f,0.7403f},c});
    v.push_back({{0.45104f,-0.09714f,0.06476f},{0.4845f,-0.6055f,0.6314f},{0.8088f,0.7480f},c});
    v.push_back({{0.44418f,-0.11502f,0.05287f},{0.4845f,-0.6055f,0.6314f},{0.7713f,0.7620f},c});
    v.push_back({{0.44418f,-0.12874f,0.03053f},{0.3726f,-0.7908f,0.4856f},{0.7275f,0.7532f},c});
    v.push_back({{0.44418f,-0.11502f,0.05287f},{0.3726f,-0.7908f,0.4856f},{0.7713f,0.7620f},c});
    v.push_back({{0.43524f,-0.12874f,0.03739f},{0.3726f,-0.7908f,0.4856f},{0.7301f,0.7685f},c});
    v.push_back({{0.47471f,-0.05396f,0.06105f},{0.6036f,-0.1294f,0.7867f},{0.5717f,0.3597f},c});
    v.push_back({{0.45536f,-0.07631f,0.07223f},{0.6036f,-0.1294f,0.7867f},{0.6072f,0.3836f},c});
    v.push_back({{0.47263f,-0.07631f,0.05897f},{0.6036f,-0.1294f,0.7867f},{0.5732f,0.3958f},c});
    v.push_back({{0.43524f,-0.12874f,0.03739f},{0.2342f,-0.7908f,0.5655f},{0.7301f,0.7685f},c});
    v.push_back({{0.42946f,-0.11502f,0.05897f},{0.2342f,-0.7908f,0.5655f},{0.7678f,0.7814f},c});
    v.push_back({{0.42483f,-0.12874f,0.04170f},{0.2342f,-0.7908f,0.5655f},{0.7276f,0.7822f},c});
    v.push_back({{0.45683f,-0.05396f,0.07478f},{0.3795f,-0.1294f,0.9161f},{0.2550f,0.7469f},c});
    v.push_back({{0.43524f,-0.07631f,0.08056f},{0.3795f,-0.1294f,0.9161f},{0.2910f,0.7887f},c});
    v.push_back({{0.45536f,-0.07631f,0.07223f},{0.3795f,-0.1294f,0.9161f},{0.2589f,0.7887f},c});
    v.push_back({{0.43524f,-0.12874f,0.03739f},{0.1475f,-0.9227f,0.3562f},{0.1870f,0.7930f},c});
    v.push_back({{0.41944f,-0.13737f,0.02159f},{0.1475f,-0.9227f,0.3562f},{0.1539f,0.7872f},c});
    v.push_back({{0.42483f,-0.13737f,0.01935f},{0.1475f,-0.9227f,0.3562f},{0.1592f,0.7778f},c});
    v.push_back({{0.45104f,-0.09714f,0.06476f},{0.3540f,-0.3799f,0.8546f},{0.2704f,0.8277f},c});
    v.push_back({{0.43524f,-0.07631f,0.08056f},{0.3540f,-0.3799f,0.8546f},{0.2910f,0.7887f},c});
    v.push_back({{0.43301f,-0.09714f,0.07223f},{0.3540f,-0.3799f,0.8546f},{0.2992f,0.8277f},c});
    v.push_back({{0.44418f,-0.11502f,0.05287f},{0.3046f,-0.6055f,0.7353f},{0.2887f,0.8612f},c});
    v.push_back({{0.43301f,-0.09714f,0.07223f},{0.3046f,-0.6055f,0.7353f},{0.2992f,0.8277f},c});
    v.push_back({{0.42946f,-0.11502f,0.05897f},{0.3046f,-0.6055f,0.7353f},{0.3122f,0.8612f},c});
    v.push_back({{0.43301f,-0.09714f,0.07223f},{0.1039f,-0.6055f,0.7891f},{0.2992f,0.8277f},c});
    v.push_back({{0.41366f,-0.11502f,0.06105f},{0.1039f,-0.6055f,0.7891f},{0.3397f,0.8612f},c});
    v.push_back({{0.42946f,-0.11502f,0.05897f},{0.1039f,-0.6055f,0.7891f},{0.3122f,0.8612f},c});
    v.push_back({{0.42483f,-0.12874f,0.04170f},{0.0799f,-0.7908f,0.6068f},{0.1769f,0.8111f},c});
    v.push_back({{0.41366f,-0.11502f,0.06105f},{0.0799f,-0.7908f,0.6068f},{0.1724f,0.8524f},c});
    v.push_back({{0.41366f,-0.12874f,0.04317f},{0.0799f,-0.7908f,0.6068f},{0.1627f,0.8258f},c});
    v.push_back({{0.43600f,-0.05396f,0.08340f},{0.1294f,-0.1294f,0.9831f},{0.2882f,0.7469f},c});
    v.push_back({{0.41366f,-0.07631f,0.08340f},{0.1294f,-0.1294f,0.9831f},{0.3286f,0.7887f},c});
    v.push_back({{0.43524f,-0.07631f,0.08056f},{0.1294f,-0.1294f,0.9831f},{0.2910f,0.7887f},c});
    v.push_back({{0.42483f,-0.12874f,0.04170f},{0.0503f,-0.9227f,0.3822f},{0.1769f,0.8111f},c});
    v.push_back({{0.41366f,-0.13737f,0.02235f},{0.0503f,-0.9227f,0.3822f},{0.1466f,0.7948f},c});
    v.push_back({{0.41944f,-0.13737f,0.02159f},{0.0503f,-0.9227f,0.3822f},{0.1539f,0.7872f},c});
    v.push_back({{0.43301f,-0.09714f,0.07223f},{0.1207f,-0.3799f,0.9171f},{0.2992f,0.8277f},c});
    v.push_back({{0.41366f,-0.07631f,0.08340f},{0.1207f,-0.3799f,0.9171f},{0.3286f,0.7887f},c});
    v.push_back({{0.41366f,-0.09714f,0.07478f},{0.1207f,-0.3799f,0.9171f},{0.3329f,0.8277f},c});
    v.push_back({{0.50000f,-0.05396f,0.00000f},{0.9914f,0.0000f,0.1305f},{0.4487f,0.3732f},c});
    v.push_back({{0.49706f,0.05396f,0.02235f},{0.9914f,0.0000f,0.1305f},{0.4707f,0.1901f},c});
    v.push_back({{0.49706f,-0.05396f,0.02235f},{0.9914f,0.0000f,0.1305f},{0.4898f,0.3732f},c});
    v.push_back({{0.45683f,-0.05396f,0.07478f},{0.3827f,0.0000f,0.9239f},{0.2550f,0.7469f},c});
    v.push_back({{0.43600f,0.05396f,0.08340f},{0.3827f,0.0000f,0.9239f},{0.2882f,0.5449f},c});
    v.push_back({{0.43600f,-0.05396f,0.08340f},{0.3827f,0.0000f,0.9239f},{0.2882f,0.7469f},c});
    v.push_back({{0.43600f,-0.05396f,-0.08340f},{0.3827f,-0.0000f,-0.9239f},{0.8809f,0.7251f},c});
    v.push_back({{0.45683f,0.05396f,-0.07478f},{0.3827f,-0.0000f,-0.9239f},{0.9230f,0.5231f},c});
    v.push_back({{0.45683f,-0.05396f,-0.07478f},{0.3827f,-0.0000f,-0.9239f},{0.9230f,0.7251f},c});
    v.push_back({{0.41366f,-0.05396f,0.08634f},{0.1305f,0.0000f,0.9914f},{0.3271f,0.7469f},c});
    v.push_back({{0.43600f,0.05396f,0.08340f},{0.1305f,0.0000f,0.9914f},{0.2882f,0.5449f},c});
    v.push_back({{0.41366f,0.05396f,0.08634f},{0.1305f,0.0000f,0.9914f},{0.3271f,0.5449f},c});
    v.push_back({{0.41366f,-0.05396f,-0.08634f},{0.1305f,-0.0000f,-0.9914f},{0.8400f,0.7251f},c});
    v.push_back({{0.43600f,0.05396f,-0.08340f},{0.1305f,-0.0000f,-0.9914f},{0.8809f,0.5231f},c});
    v.push_back({{0.43600f,-0.05396f,-0.08340f},{0.1305f,-0.0000f,-0.9914f},{0.8809f,0.7251f},c});
    v.push_back({{0.47471f,-0.05396f,0.06105f},{0.6088f,0.0000f,0.7934f},{0.5717f,0.3597f},c});
    v.push_back({{0.45683f,0.05396f,0.07478f},{0.6088f,0.0000f,0.7934f},{0.5877f,0.1640f},c});
    v.push_back({{0.45683f,-0.05396f,0.07478f},{0.6088f,0.0000f,0.7934f},{0.6068f,0.3471f},c});
    v.push_back({{0.45683f,-0.05396f,-0.07478f},{0.6088f,0.0000f,-0.7934f},{0.9230f,0.7251f},c});
    v.push_back({{0.47471f,0.05396f,-0.06105f},{0.6088f,0.0000f,-0.7934f},{0.9637f,0.5231f},c});
    v.push_back({{0.47471f,-0.05396f,-0.06105f},{0.6088f,0.0000f,-0.7934f},{0.9637f,0.7251f},c});
    v.push_back({{0.47471f,-0.05396f,-0.06105f},{0.7934f,0.0000f,-0.6088f},{0.9637f,0.7251f},c});
    v.push_back({{0.48843f,0.05396f,-0.04317f},{0.7934f,0.0000f,-0.6088f},{1.0000f,0.5231f},c});
    v.push_back({{0.48843f,-0.05396f,-0.04317f},{0.7934f,0.0000f,-0.6088f},{1.0000f,0.7251f},c});
    v.push_back({{0.48843f,-0.05396f,-0.04317f},{0.9239f,0.0000f,-0.3827f},{0.7161f,0.5404f},c});
    v.push_back({{0.49706f,0.05396f,-0.02235f},{0.9239f,0.0000f,-0.3827f},{0.8400f,0.4254f},c});
    v.push_back({{0.49706f,-0.05396f,-0.02235f},{0.9239f,0.0000f,-0.3827f},{0.7540f,0.5570f},c});
    v.push_back({{0.49706f,-0.05396f,-0.02235f},{0.9914f,-0.0000f,-0.1305f},{0.4112f,0.3686f},c});
    v.push_back({{0.50000f,0.05396f,0.00000f},{0.9914f,-0.0000f,-0.1305f},{0.4296f,0.1901f},c});
    v.push_back({{0.50000f,-0.05396f,0.00000f},{0.9914f,-0.0000f,-0.1305f},{0.4487f,0.3732f},c});
    v.push_back({{0.49706f,-0.05396f,0.02235f},{0.9239f,0.0000f,0.3827f},{0.4898f,0.3732f},c});
    v.push_back({{0.48843f,0.05396f,0.04317f},{0.9239f,0.0000f,0.3827f},{0.5126f,0.1855f},c});
    v.push_back({{0.48843f,-0.05396f,0.04317f},{0.9239f,0.0000f,0.3827f},{0.5318f,0.3686f},c});
    v.push_back({{0.48843f,-0.05396f,0.04317f},{0.7934f,0.0000f,0.6088f},{0.5318f,0.3686f},c});
    v.push_back({{0.47471f,0.05396f,0.06105f},{0.7934f,0.0000f,0.6088f},{0.5525f,0.1766f},c});
    v.push_back({{0.47471f,-0.05396f,0.06105f},{0.7934f,0.0000f,0.6088f},{0.5717f,0.3597f},c});
    v.push_back({{0.41366f,0.12874f,-0.04317f},{0.0503f,0.9227f,-0.3822f},{0.0055f,0.9246f},c});
    v.push_back({{0.41944f,0.13737f,-0.02159f},{0.0503f,0.9227f,-0.3822f},{0.0187f,0.8819f},c});
    v.push_back({{0.42483f,0.12874f,-0.04170f},{0.0503f,0.9227f,-0.3822f},{0.0256f,0.9225f},c});
    v.push_back({{0.41366f,0.07631f,-0.08340f},{0.1294f,0.1294f,-0.9831f},{0.8420f,0.4813f},c});
    v.push_back({{0.43600f,0.05396f,-0.08340f},{0.1294f,0.1294f,-0.9831f},{0.8809f,0.5231f},c});
    v.push_back({{0.41366f,0.05396f,-0.08634f},{0.1294f,0.1294f,-0.9831f},{0.8400f,0.5231f},c});
    v.push_back({{0.41366f,0.11502f,-0.06105f},{0.0799f,0.7908f,-0.6068f},{0.0000f,0.9630f},c});
    v.push_back({{0.42483f,0.12874f,-0.04170f},{0.0799f,0.7908f,-0.6068f},{0.0256f,0.9225f},c});
    v.push_back({{0.42946f,0.11502f,-0.05897f},{0.0799f,0.7908f,-0.6068f},{0.0284f,0.9600f},c});
    v.push_back({{0.41366f,0.09714f,-0.07478f},{0.1039f,0.6055f,-0.7891f},{0.8481f,0.4423f},c});
    v.push_back({{0.42946f,0.11502f,-0.05897f},{0.1039f,0.6055f,-0.7891f},{0.8865f,0.4088f},c});
    v.push_back({{0.43301f,0.09714f,-0.07223f},{0.1039f,0.6055f,-0.7891f},{0.8835f,0.4423f},c});
    v.push_back({{0.41366f,0.07631f,-0.08340f},{0.1207f,0.3799f,-0.9171f},{0.8420f,0.4813f},c});
    v.push_back({{0.43301f,0.09714f,-0.07223f},{0.1207f,0.3799f,-0.9171f},{0.8835f,0.4423f},c});
    v.push_back({{0.43524f,0.07631f,-0.08056f},{0.1207f,0.3799f,-0.9171f},{0.8815f,0.4813f},c});
    v.push_back({{0.42946f,0.11502f,-0.05897f},{0.3046f,0.6055f,-0.7353f},{0.8865f,0.4088f},c});
    v.push_back({{0.45104f,0.09714f,-0.06476f},{0.3046f,0.6055f,-0.7353f},{0.9200f,0.4423f},c});
    v.push_back({{0.43301f,0.09714f,-0.07223f},{0.3046f,0.6055f,-0.7353f},{0.8835f,0.4423f},c});
    v.push_back({{0.43524f,0.07631f,-0.08056f},{0.3540f,0.3799f,-0.8546f},{0.8815f,0.4813f},c});
    v.push_back({{0.45104f,0.09714f,-0.06476f},{0.3540f,0.3799f,-0.8546f},{0.9200f,0.4423f},c});
    v.push_back({{0.45536f,0.07631f,-0.07223f},{0.3540f,0.3799f,-0.8546f},{0.9223f,0.4813f},c});
    v.push_back({{0.42483f,0.12874f,-0.04170f},{0.1475f,0.9227f,-0.3562f},{0.0256f,0.9225f},c});
    v.push_back({{0.42483f,0.13737f,-0.01935f},{0.1475f,0.9227f,-0.3562f},{0.0283f,0.8781f},c});
    v.push_back({{0.43524f,0.12874f,-0.03739f},{0.1475f,0.9227f,-0.3562f},{0.0441f,0.9151f},c});
    v.push_back({{0.43524f,0.07631f,-0.08056f},{0.3795f,0.1294f,-0.9161f},{0.8815f,0.4813f},c});
    v.push_back({{0.45683f,0.05396f,-0.07478f},{0.3795f,0.1294f,-0.9161f},{0.9230f,0.5231f},c});
    v.push_back({{0.43600f,0.05396f,-0.08340f},{0.3795f,0.1294f,-0.9161f},{0.8809f,0.5231f},c});
    v.push_back({{0.42483f,0.12874f,-0.04170f},{0.2342f,0.7908f,-0.5655f},{0.0256f,0.9225f},c});
    v.push_back({{0.44418f,0.11502f,-0.05287f},{0.2342f,0.7908f,-0.5655f},{0.0547f,0.9496f},c});
    v.push_back({{0.42946f,0.11502f,-0.05897f},{0.2342f,0.7908f,-0.5655f},{0.0284f,0.9600f},c});
    v.push_back({{0.45683f,0.05396f,-0.07478f},{0.6036f,0.1294f,-0.7867f},{0.9230f,0.5231f},c});
    v.push_back({{0.47263f,0.07631f,-0.05897f},{0.6036f,0.1294f,-0.7867f},{0.9615f,0.4813f},c});
    v.push_back({{0.47471f,0.05396f,-0.06105f},{0.6036f,0.1294f,-0.7867f},{0.9637f,0.5231f},c});
    v.push_back({{0.43524f,0.12874f,-0.03739f},{0.3726f,0.7908f,-0.4856f},{0.0441f,0.9151f},c});
    v.push_back({{0.45683f,0.11502f,-0.04317f},{0.3726f,0.7908f,-0.4856f},{0.0769f,0.9326f},c});
    v.push_back({{0.44418f,0.11502f,-0.05287f},{0.3726f,0.7908f,-0.4856f},{0.0547f,0.9496f},c});
    v.push_back({{0.44418f,0.11502f,-0.05287f},{0.4845f,0.6055f,-0.6314f},{0.9164f,0.4088f},c});
    v.push_back({{0.46653f,0.09714f,-0.05287f},{0.4845f,0.6055f,-0.6314f},{0.9552f,0.4423f},c});
    v.push_back({{0.45104f,0.09714f,-0.06476f},{0.4845f,0.6055f,-0.6314f},{0.9200f,0.4423f},c});
    v.push_back({{0.45536f,0.07631f,-0.07223f},{0.5631f,0.3799f,-0.7339f},{0.9223f,0.4813f},c});
    v.push_back({{0.46653f,0.09714f,-0.05287f},{0.5631f,0.3799f,-0.7339f},{0.9552f,0.4423f},c});
    v.push_back({{0.47263f,0.07631f,-0.05897f},{0.5631f,0.3799f,-0.7339f},{0.9615f,0.4813f},c});
    v.push_back({{0.42483f,0.13737f,-0.01935f},{0.2347f,0.9227f,-0.3058f},{0.0283f,0.8781f},c});
    v.push_back({{0.44418f,0.12874f,-0.03053f},{0.2347f,0.9227f,-0.3058f},{0.0599f,0.9031f},c});
    v.push_back({{0.43524f,0.12874f,-0.03739f},{0.2347f,0.9227f,-0.3058f},{0.0441f,0.9151f},c});
    v.push_back({{0.46653f,0.09714f,-0.05287f},{0.7339f,0.3799f,-0.5631f},{0.9552f,0.4423f},c});
    v.push_back({{0.48588f,0.07631f,-0.04170f},{0.7339f,0.3799f,-0.5631f},{0.9966f,0.4813f},c});
    v.push_back({{0.47263f,0.07631f,-0.05897f},{0.7339f,0.3799f,-0.5631f},{0.9615f,0.4813f},c});
    v.push_back({{0.42946f,0.13737f,-0.01580f},{0.3058f,0.9227f,-0.2347f},{0.0365f,0.8718f},c});
    v.push_back({{0.45104f,0.12874f,-0.02159f},{0.3058f,0.9227f,-0.2347f},{0.0717f,0.8871f},c});
    v.push_back({{0.44418f,0.12874f,-0.03053f},{0.3058f,0.9227f,-0.2347f},{0.0599f,0.9031f},c});
    v.push_back({{0.47471f,0.05396f,-0.06105f},{0.7867f,0.1294f,-0.6036f},{0.9637f,0.5231f},c});
    v.push_back({{0.48588f,0.07631f,-0.04170f},{0.7867f,0.1294f,-0.6036f},{0.9966f,0.4813f},c});
    v.push_back({{0.48843f,0.05396f,-0.04317f},{0.7867f,0.1294f,-0.6036f},{1.0000f,0.5231f},c});
    v.push_back({{0.44418f,0.12874f,-0.03053f},{0.4856f,0.7908f,-0.3726f},{0.0599f,0.9031f},c});
    v.push_back({{0.46653f,0.11502f,-0.03053f},{0.4856f,0.7908f,-0.3726f},{0.0936f,0.9100f},c});
    v.push_back({{0.45683f,0.11502f,-0.04317f},{0.4856f,0.7908f,-0.3726f},{0.0769f,0.9326f},c});
    v.push_back({{0.45683f,0.11502f,-0.04317f},{0.6314f,0.6055f,-0.4845f},{0.9451f,0.4088f},c});
    v.push_back({{0.47841f,0.09714f,-0.03739f},{0.6314f,0.6055f,-0.4845f},{0.9866f,0.4423f},c});
    v.push_back({{0.46653f,0.09714f,-0.05287f},{0.6314f,0.6055f,-0.4845f},{0.9552f,0.4423f},c});
    v.push_back({{0.47841f,0.09714f,-0.03739f},{0.7353f,0.6055f,-0.3046f},{0.3695f,0.0960f},c});
    v.push_back({{0.47263f,0.11502f,-0.01580f},{0.7353f,0.6055f,-0.3046f},{0.4078f,0.0634f},c});
    v.push_back({{0.48588f,0.09714f,-0.01935f},{0.7353f,0.6055f,-0.3046f},{0.3966f,0.1038f},c});
    v.push_back({{0.47841f,0.09714f,-0.03739f},{0.8546f,0.3799f,-0.3540f},{0.3695f,0.0960f},c});
    v.push_back({{0.49422f,0.07631f,-0.02159f},{0.8546f,0.3799f,-0.3540f},{0.3912f,0.1454f},c});
    v.push_back({{0.48588f,0.07631f,-0.04170f},{0.8546f,0.3799f,-0.3540f},{0.3610f,0.1368f},c});
    v.push_back({{0.45104f,0.12874f,-0.02159f},{0.3562f,0.9227f,-0.1475f},{0.0717f,0.8871f},c});
    v.push_back({{0.43524f,0.13737f,-0.00578f},{0.3562f,0.9227f,-0.1475f},{0.0463f,0.8539f},c});
    v.push_back({{0.45536f,0.12874f,-0.01117f},{0.3562f,0.9227f,-0.1475f},{0.0788f,0.8684f},c});
    v.push_back({{0.48588f,0.07631f,-0.04170f},{0.9161f,0.1294f,-0.3795f},{0.3610f,0.1368f},c});
    v.push_back({{0.49706f,0.05396f,-0.02235f},{0.9161f,0.1294f,-0.3795f},{0.3921f,0.1855f},c});
    v.push_back({{0.48843f,0.05396f,-0.04317f},{0.9161f,0.1294f,-0.3795f},{0.3608f,0.1766f},c});
    v.push_back({{0.45104f,0.12874f,-0.02159f},{0.5655f,0.7908f,-0.2342f},{0.4085f,0.0226f},c});
    v.push_back({{0.47263f,0.11502f,-0.01580f},{0.5655f,0.7908f,-0.2342f},{0.4078f,0.0634f},c});
    v.push_back({{0.46653f,0.11502f,-0.03053f},{0.5655f,0.7908f,-0.2342f},{0.3857f,0.0571f},c});
    v.push_back({{0.49422f,0.07631f,-0.02159f},{0.9831f,0.1294f,-0.1294f},{0.3912f,0.1454f},c});
    v.push_back({{0.50000f,0.05396f,0.00000f},{0.9831f,0.1294f,-0.1294f},{0.4296f,0.1901f},c});
    v.push_back({{0.49706f,0.05396f,-0.02235f},{0.9831f,0.1294f,-0.1294f},{0.3921f,0.1855f},c});
    v.push_back({{0.47263f,0.11502f,-0.01580f},{0.6068f,0.7908f,-0.0799f},{0.4078f,0.0634f},c});
    v.push_back({{0.45683f,0.12874f,0.00000f},{0.6068f,0.7908f,-0.0799f},{0.4429f,0.0294f},c});
    v.push_back({{0.47471f,0.11502f,0.00000f},{0.6068f,0.7908f,-0.0799f},{0.4343f,0.0667f},c});
    v.push_back({{0.47263f,0.11502f,-0.01580f},{0.7891f,0.6055f,-0.1039f},{0.4078f,0.0634f},c});
    v.push_back({{0.48843f,0.09714f,0.00000f},{0.7891f,0.6055f,-0.1039f},{0.4291f,0.1077f},c});
    v.push_back({{0.48588f,0.09714f,-0.01935f},{0.7891f,0.6055f,-0.1039f},{0.3966f,0.1038f},c});
    v.push_back({{0.48588f,0.09714f,-0.01935f},{0.9171f,0.3799f,-0.1207f},{0.3966f,0.1038f},c});
    v.push_back({{0.49706f,0.07631f,0.00000f},{0.9171f,0.3799f,-0.1207f},{0.4274f,0.1498f},c});
    v.push_back({{0.49422f,0.07631f,-0.02159f},{0.9171f,0.3799f,-0.1207f},{0.3912f,0.1454f},c});
    v.push_back({{0.43524f,0.13737f,-0.00578f},{0.3822f,0.9227f,-0.0503f},{0.0463f,0.8539f},c});
    v.push_back({{0.45683f,0.12874f,0.00000f},{0.3822f,0.9227f,-0.0503f},{0.0808f,0.8481f},c});
    v.push_back({{0.45536f,0.12874f,-0.01117f},{0.3822f,0.9227f,-0.0503f},{0.0788f,0.8684f},c});
    v.push_back({{0.48843f,0.09714f,0.00000f},{0.9171f,0.3799f,0.1207f},{0.4291f,0.1077f},c});
    v.push_back({{0.49422f,0.07631f,0.02159f},{0.9171f,0.3799f,0.1207f},{0.4672f,0.1498f},c});
    v.push_back({{0.49706f,0.07631f,0.00000f},{0.9171f,0.3799f,0.1207f},{0.4274f,0.1498f},c});
    v.push_back({{0.45683f,0.12874f,0.00000f},{0.3822f,0.9227f,0.0503f},{0.0808f,0.8481f},c});
    v.push_back({{0.43524f,0.13737f,0.00578f},{0.3822f,0.9227f,0.0503f},{0.0455f,0.8328f},c});
    v.push_back({{0.45536f,0.12874f,0.01117f},{0.3822f,0.9227f,0.0503f},{0.0774f,0.8276f},c});
    v.push_back({{0.49706f,0.07631f,0.00000f},{0.9831f,0.1294f,0.1294f},{0.4274f,0.1498f},c});
    v.push_back({{0.49706f,0.05396f,0.02235f},{0.9831f,0.1294f,0.1294f},{0.4707f,0.1901f},c});
    v.push_back({{0.50000f,0.05396f,0.00000f},{0.9831f,0.1294f,0.1294f},{0.4296f,0.1901f},c});
    v.push_back({{0.45683f,0.12874f,0.00000f},{0.6068f,0.7908f,0.0799f},{0.4429f,0.0294f},c});
    v.push_back({{0.47263f,0.11502f,0.01580f},{0.6068f,0.7908f,0.0799f},{0.4634f,0.0667f},c});
    v.push_back({{0.47471f,0.11502f,0.00000f},{0.6068f,0.7908f,0.0799f},{0.4343f,0.0667f},c});
    v.push_back({{0.48843f,0.09714f,0.00000f},{0.7891f,0.6055f,0.1039f},{0.4291f,0.1077f},c});
    v.push_back({{0.47263f,0.11502f,0.01580f},{0.7891f,0.6055f,0.1039f},{0.4634f,0.0667f},c});
    v.push_back({{0.48588f,0.09714f,0.01935f},{0.7891f,0.6055f,0.1039f},{0.4647f,0.1077f},c});
    v.push_back({{0.48588f,0.09714f,0.01935f},{0.7353f,0.6055f,0.3046f},{0.4647f,0.1077f},c});
    v.push_back({{0.46653f,0.11502f,0.03053f},{0.7353f,0.6055f,0.3046f},{0.4931f,0.0634f},c});
    v.push_back({{0.47841f,0.09714f,0.03739f},{0.7353f,0.6055f,0.3046f},{0.5010f,0.1038f},c});
    v.push_back({{0.49422f,0.07631f,0.02159f},{0.8546f,0.3799f,0.3540f},{0.4672f,0.1498f},c});
    v.push_back({{0.47841f,0.09714f,0.03739f},{0.8546f,0.3799f,0.3540f},{0.5010f,0.1038f},c});
    v.push_back({{0.48588f,0.07631f,0.04170f},{0.8546f,0.3799f,0.3540f},{0.5077f,0.1454f},c});
    v.push_back({{0.43524f,0.13737f,0.00578f},{0.3562f,0.9227f,0.1475f},{0.0455f,0.8328f},c});
    v.push_back({{0.45104f,0.12874f,0.02159f},{0.3562f,0.9227f,0.1475f},{0.0690f,0.8084f},c});
    v.push_back({{0.45536f,0.12874f,0.01117f},{0.3562f,0.9227f,0.1475f},{0.0774f,0.8276f},c});
    v.push_back({{0.49706f,0.05396f,0.02235f},{0.9161f,0.1294f,0.3795f},{0.4707f,0.1901f},c});
    v.push_back({{0.48588f,0.07631f,0.04170f},{0.9161f,0.1294f,0.3795f},{0.5077f,0.1454f},c});
    v.push_back({{0.48843f,0.05396f,0.04317f},{0.9161f,0.1294f,0.3795f},{0.5126f,0.1855f},c});
    v.push_back({{0.47263f,0.11502f,0.01580f},{0.5655f,0.7908f,0.2342f},{0.4634f,0.0667f},c});
    v.push_back({{0.45104f,0.12874f,0.02159f},{0.5655f,0.7908f,0.2342f},{0.4844f,0.0271f},c});
    v.push_back({{0.46653f,0.11502f,0.03053f},{0.5655f,0.7908f,0.2342f},{0.4931f,0.0634f},c});
    v.push_back({{0.45104f,0.12874f,0.02159f},{0.3058f,0.9227f,0.2347f},{0.0690f,0.8084f},c});
    v.push_back({{0.42946f,0.13737f,0.01580f},{0.3058f,0.9227f,0.2347f},{0.0345f,0.8143f},c});
    v.push_back({{0.44418f,0.12874f,0.03053f},{0.3058f,0.9227f,0.2347f},{0.0560f,0.7918f},c});
    v.push_back({{0.48843f,0.05396f,0.04317f},{0.7867f,0.1294f,0.6036f},{0.5126f,0.1855f},c});
    v.push_back({{0.47263f,0.07631f,0.05897f},{0.7867f,0.1294f,0.6036f},{0.5462f,0.1368f},c});
    v.push_back({{0.47471f,0.05396f,0.06105f},{0.7867f,0.1294f,0.6036f},{0.5525f,0.1766f},c});
    v.push_back({{0.45104f,0.12874f,0.02159f},{0.4856f,0.7908f,0.3726f},{0.4844f,0.0271f},c});
    v.push_back({{0.45683f,0.11502f,0.04317f},{0.4856f,0.7908f,0.3726f},{0.5213f,0.0571f},c});
    v.push_back({{0.46653f,0.11502f,0.03053f},{0.4856f,0.7908f,0.3726f},{0.4931f,0.0634f},c});
    v.push_back({{0.46653f,0.11502f,0.03053f},{0.6314f,0.6055f,0.4845f},{0.4931f,0.0634f},c});
    v.push_back({{0.46653f,0.09714f,0.05287f},{0.6314f,0.6055f,0.4845f},{0.5355f,0.0960f},c});
    v.push_back({{0.47841f,0.09714f,0.03739f},{0.6314f,0.6055f,0.4845f},{0.5010f,0.1038f},c});
    v.push_back({{0.47841f,0.09714f,0.03739f},{0.7339f,0.3799f,0.5631f},{0.5010f,0.1038f},c});
    v.push_back({{0.47263f,0.07631f,0.05897f},{0.7339f,0.3799f,0.5631f},{0.5462f,0.1368f},c});
    v.push_back({{0.48588f,0.07631f,0.04170f},{0.7339f,0.3799f,0.5631f},{0.5077f,0.1454f},c});
    v.push_back({{0.46653f,0.09714f,0.05287f},{0.5631f,0.3799f,0.7339f},{0.5355f,0.0960f},c});
    v.push_back({{0.45536f,0.07631f,0.07223f},{0.5631f,0.3799f,0.7339f},{0.5801f,0.1246f},c});
    v.push_back({{0.47263f,0.07631f,0.05897f},{0.5631f,0.3799f,0.7339f},{0.5462f,0.1368f},c});
    v.push_back({{0.44418f,0.12874f,0.03053f},{0.2347f,0.9227f,0.3058f},{0.0560f,0.7918f},c});
    v.push_back({{0.42483f,0.13737f,0.01935f},{0.2347f,0.9227f,0.3058f},{0.0259f,0.8076f},c});
    v.push_back({{0.43524f,0.12874f,0.03739f},{0.2347f,0.9227f,0.3058f},{0.0394f,0.7789f},c});
    v.push_back({{0.47471f,0.05396f,0.06105f},{0.6036f,0.1294f,0.7867f},{0.5525f,0.1766f},c});
    v.push_back({{0.45536f,0.07631f,0.07223f},{0.6036f,0.1294f,0.7867f},{0.5801f,0.1246f},c});
    v.push_back({{0.45683f,0.05396f,0.07478f},{0.6036f,0.1294f,0.7867f},{0.5877f,0.1640f},c});
    v.push_back({{0.44418f,0.12874f,0.03053f},{0.3726f,0.7908f,0.4856f},{0.5044f,0.0226f},c});
    v.push_back({{0.44418f,0.11502f,0.05287f},{0.3726f,0.7908f,0.4856f},{0.5461f,0.0482f},c});
    v.push_back({{0.45683f,0.11502f,0.04317f},{0.3726f,0.7908f,0.4856f},{0.5213f,0.0571f},c});
    v.push_back({{0.46653f,0.09714f,0.05287f},{0.4845f,0.6055f,0.6314f},{0.5355f,0.0960f},c});
    v.push_back({{0.44418f,0.11502f,0.05287f},{0.4845f,0.6055f,0.6314f},{0.5461f,0.0482f},c});
    v.push_back({{0.45104f,0.09714f,0.06476f},{0.4845f,0.6055f,0.6314f},{0.5660f,0.0851f},c});
    v.push_back({{0.43524f,0.12874f,0.03739f},{0.2342f,0.7908f,0.5655f},{0.5219f,0.0163f},c});
    v.push_back({{0.42946f,0.11502f,0.05897f},{0.2342f,0.7908f,0.5655f},{0.5659f,0.0373f},c});
    v.push_back({{0.44418f,0.11502f,0.05287f},{0.2342f,0.7908f,0.5655f},{0.5461f,0.0482f},c});
    v.push_back({{0.45104f,0.09714f,0.06476f},{0.3046f,0.6055f,0.7353f},{0.5660f,0.0851f},c});
    v.push_back({{0.42946f,0.11502f,0.05897f},{0.3046f,0.6055f,0.7353f},{0.5659f,0.0373f},c});
    v.push_back({{0.43301f,0.09714f,0.07223f},{0.3046f,0.6055f,0.7353f},{0.5902f,0.0718f},c});
    v.push_back({{0.45104f,0.09714f,0.06476f},{0.3540f,0.3799f,0.8546f},{0.5660f,0.0851f},c});
    v.push_back({{0.43524f,0.07631f,0.08056f},{0.3540f,0.3799f,0.8546f},{0.6072f,0.1097f},c});
    v.push_back({{0.45536f,0.07631f,0.07223f},{0.3540f,0.3799f,0.8546f},{0.5801f,0.1246f},c});
    v.push_back({{0.43524f,0.12874f,0.03739f},{0.1475f,0.9227f,0.3562f},{0.0394f,0.7789f},c});
    v.push_back({{0.41944f,0.13737f,0.02159f},{0.1475f,0.9227f,0.3562f},{0.0160f,0.8032f},c});
    v.push_back({{0.42483f,0.12874f,0.04170f},{0.1475f,0.9227f,0.3562f},{0.0203f,0.7705f},c});
    v.push_back({{0.45536f,0.07631f,0.07223f},{0.3795f,0.1294f,0.9161f},{0.2589f,0.5031f},c});
    v.push_back({{0.43600f,0.05396f,0.08340f},{0.3795f,0.1294f,0.9161f},{0.2882f,0.5449f},c});
    v.push_back({{0.45683f,0.05396f,0.07478f},{0.3795f,0.1294f,0.9161f},{0.2550f,0.5449f},c});
    v.push_back({{0.42483f,0.12874f,0.04170f},{0.0503f,0.9227f,0.3822f},{0.0203f,0.7705f},c});
    v.push_back({{0.41366f,0.13737f,0.02235f},{0.0503f,0.9227f,0.3822f},{0.0055f,0.8015f},c});
    v.push_back({{0.41366f,0.12874f,0.04317f},{0.0503f,0.9227f,0.3822f},{0.0000f,0.7673f},c});
    v.push_back({{0.43600f,0.05396f,0.08340f},{0.1294f,0.1294f,0.9831f},{0.2882f,0.5449f},c});
    v.push_back({{0.41366f,0.07631f,0.08340f},{0.1294f,0.1294f,0.9831f},{0.3286f,0.5030f},c});
    v.push_back({{0.41366f,0.05396f,0.08634f},{0.1294f,0.1294f,0.9831f},{0.3271f,0.5449f},c});
    v.push_back({{0.42483f,0.12874f,0.04170f},{0.0799f,0.7908f,0.6068f},{0.5359f,0.0086f},c});
    v.push_back({{0.41366f,0.11502f,0.06105f},{0.0799f,0.7908f,0.6068f},{0.5793f,0.0251f},c});
    v.push_back({{0.42946f,0.11502f,0.05897f},{0.0799f,0.7908f,0.6068f},{0.5659f,0.0373f},c});
    v.push_back({{0.43301f,0.09714f,0.07223f},{0.1039f,0.6055f,0.7891f},{0.2992f,0.4641f},c});
    v.push_back({{0.41366f,0.11502f,0.06105f},{0.1039f,0.6055f,0.7891f},{0.3397f,0.4306f},c});
    v.push_back({{0.41366f,0.09714f,0.07478f},{0.1039f,0.6055f,0.7891f},{0.3329f,0.4641f},c});
    v.push_back({{0.43301f,0.09714f,0.07223f},{0.1207f,0.3799f,0.9171f},{0.2992f,0.4641f},c});
    v.push_back({{0.41366f,0.07631f,0.08340f},{0.1207f,0.3799f,0.9171f},{0.3286f,0.5030f},c});
    v.push_back({{0.43524f,0.07631f,0.08056f},{0.1207f,0.3799f,0.9171f},{0.2910f,0.5030f},c});
    v.push_back({{-0.37048f,-0.03238f,-0.02159f},{0.3714f,0.9285f,-0.0000f},{0.8793f,0.6291f},c});
    v.push_back({{-0.41366f,-0.01511f,0.03885f},{0.3714f,0.9285f,-0.0000f},{0.9425f,0.7169f},c});
    v.push_back({{-0.37048f,-0.03238f,0.02159f},{0.3714f,0.9285f,-0.0000f},{0.8793f,0.6918f},c});
    v.push_back({{-0.37048f,-0.03238f,0.02159f},{0.3714f,0.0000f,0.9285f},{0.9449f,0.7796f},c});
    v.push_back({{-0.41366f,-0.09282f,0.03885f},{0.3714f,0.0000f,0.9285f},{0.9928f,0.8673f},c});
    v.push_back({{-0.37048f,-0.07555f,0.02159f},{0.3714f,0.0000f,0.9285f},{0.9449f,0.8423f},c});
    v.push_back({{-0.37048f,-0.07555f,0.02159f},{0.3714f,-0.9285f,-0.0000f},{0.8817f,0.7796f},c});
    v.push_back({{-0.41366f,-0.09282f,-0.03885f},{0.3714f,-0.9285f,-0.0000f},{0.9449f,0.8673f},c});
    v.push_back({{-0.37048f,-0.07555f,-0.02159f},{0.3714f,-0.9285f,-0.0000f},{0.8817f,0.8423f},c});
    v.push_back({{-0.37048f,-0.07555f,-0.02159f},{0.3714f,0.0000f,-0.9285f},{0.9521f,0.1936f},c});
    v.push_back({{-0.41366f,-0.01511f,-0.03885f},{0.3714f,0.0000f,-0.9285f},{1.0000f,0.2813f},c});
    v.push_back({{-0.37048f,-0.03238f,-0.02159f},{0.3714f,0.0000f,-0.9285f},{0.9521f,0.2562f},c});
    v.push_back({{-0.50000f,-0.05396f,-0.00000f},{0.1644f,-0.9864f,-0.0000f},{0.7792f,0.3597f},c});
    v.push_back({{-0.37048f,-0.03238f,-0.02159f},{0.1644f,-0.9864f,-0.0000f},{0.7173f,0.1788f},c});
    v.push_back({{-0.37048f,-0.03238f,0.02159f},{0.1644f,-0.9864f,-0.0000f},{0.7792f,0.1685f},c});
    v.push_back({{-0.37048f,-0.03238f,-0.02159f},{0.1644f,0.0000f,0.9864f},{0.8530f,-0.0000f},c});
    v.push_back({{-0.50000f,-0.05396f,-0.00000f},{0.1644f,0.0000f,0.9864f},{0.8217f,0.1608f},c});
    v.push_back({{-0.37048f,-0.07555f,-0.02159f},{0.1644f,0.0000f,0.9864f},{0.7904f,-0.0000f},c});
    v.push_back({{-0.50000f,-0.05396f,-0.00000f},{0.1644f,0.9864f,-0.0000f},{0.7792f,0.5510f},c});
    v.push_back({{-0.37048f,-0.07555f,0.02159f},{0.1644f,0.9864f,-0.0000f},{0.7173f,0.3700f},c});
    v.push_back({{-0.37048f,-0.07555f,-0.02159f},{0.1644f,0.9864f,-0.0000f},{0.7792f,0.3597f},c});
    v.push_back({{-0.50000f,-0.05396f,-0.00000f},{0.1644f,-0.0000f,-0.9864f},{0.2873f,0.8765f},c});
    v.push_back({{-0.37048f,-0.03238f,0.02159f},{0.1644f,-0.0000f,-0.9864f},{0.2560f,0.7157f},c});
    v.push_back({{-0.37048f,-0.07555f,0.02159f},{0.1644f,-0.0000f,-0.9864f},{0.3187f,0.7157f},c});
    v.push_back({{-0.41366f,-0.09282f,-0.03885f},{0.1483f,0.0000f,0.9889f},{0.5950f,0.4505f},c});
    v.push_back({{-0.32731f,-0.00216f,-0.05181f},{0.1483f,0.0000f,0.9889f},{0.4870f,0.3189f},c});
    v.push_back({{-0.41366f,-0.01511f,-0.03885f},{0.1483f,0.0000f,0.9889f},{0.5950f,0.3377f},c});
    v.push_back({{-0.41366f,-0.01511f,0.03885f},{0.1483f,0.0000f,-0.9889f},{0.6151f,0.3001f},c});
    v.push_back({{-0.32731f,-0.10577f,0.05181f},{0.1483f,0.0000f,-0.9889f},{0.5071f,0.1685f},c});
    v.push_back({{-0.41366f,-0.09282f,0.03885f},{0.1483f,0.0000f,-0.9889f},{0.6151f,0.1873f},c});
    v.push_back({{-0.41366f,-0.09282f,-0.03885f},{0.1483f,0.9889f,-0.0000f},{0.3778f,0.0188f},c});
    v.push_back({{-0.32731f,-0.10577f,0.05181f},{0.1483f,0.9889f,-0.0000f},{0.2520f,0.1504f},c});
    v.push_back({{-0.32731f,-0.10577f,-0.05181f},{0.1483f,0.9889f,-0.0000f},{0.2520f,0.0000f},c});
    v.push_back({{-0.41366f,-0.01511f,0.03885f},{0.1483f,-0.9889f,-0.0000f},{0.3817f,0.1873f},c});
    v.push_back({{-0.32731f,-0.00216f,-0.05181f},{0.1483f,-0.9889f,-0.0000f},{0.2560f,0.3189f},c});
    v.push_back({{-0.32731f,-0.00216f,0.05181f},{0.1483f,-0.9889f,-0.0000f},{0.2560f,0.1685f},c});
    v.push_back({{-0.32731f,-0.00216f,0.05181f},{-0.6000f,0.0000f,0.8000f},{0.3187f,0.7157f},c});
    v.push_back({{-0.36185f,-0.07987f,0.02590f},{-0.6000f,0.0000f,0.8000f},{0.3795f,0.8285f},c});
    v.push_back({{-0.32731f,-0.10577f,0.05181f},{-0.6000f,0.0000f,0.8000f},{0.3187f,0.8661f},c});
    v.push_back({{-0.32731f,-0.00216f,-0.05181f},{-0.6000f,0.0000f,-0.8000f},{0.8184f,0.7545f},c});
    v.push_back({{-0.36185f,-0.07987f,-0.02590f},{-0.6000f,0.0000f,-0.8000f},{0.8793f,0.6417f},c});
    v.push_back({{-0.36185f,-0.02806f,-0.02590f},{-0.6000f,0.0000f,-0.8000f},{0.8793f,0.7169f},c});
    v.push_back({{-0.32731f,-0.00216f,-0.05181f},{-0.6000f,0.8000f,-0.0000f},{0.2032f,0.7098f},c});
    v.push_back({{-0.36185f,-0.02806f,0.02590f},{-0.6000f,0.8000f,-0.0000f},{0.2525f,0.8226f},c});
    v.push_back({{-0.32731f,-0.00216f,0.05181f},{-0.6000f,0.8000f,-0.0000f},{0.2032f,0.8602f},c});
    v.push_back({{-0.32731f,-0.10577f,0.05181f},{-0.6000f,-0.8000f,-0.0000f},{0.8530f,0.0000f},c});
    v.push_back({{-0.36185f,-0.07987f,-0.02590f},{-0.6000f,-0.8000f,-0.0000f},{0.9023f,0.1128f},c});
    v.push_back({{-0.32731f,-0.10577f,-0.05181f},{-0.6000f,-0.8000f,-0.0000f},{0.8530f,0.1504f},c});
    v.push_back({{-0.36185f,-0.02806f,0.02590f},{0.0000f,-0.0000f,-1.0000f},{0.7432f,0.7483f},c});
    v.push_back({{-0.25392f,-0.07987f,0.02590f},{0.0000f,-0.0000f,-1.0000f},{0.8184f,0.6041f},c});
    v.push_back({{-0.36185f,-0.07987f,0.02590f},{0.0000f,-0.0000f,-1.0000f},{0.8184f,0.7483f},c});
    v.push_back({{-0.36185f,-0.07987f,-0.02590f},{0.0000f,-0.0000f,1.0000f},{0.1280f,0.8540f},c});
    v.push_back({{-0.25392f,-0.02806f,-0.02590f},{0.0000f,-0.0000f,1.0000f},{0.2032f,0.7098f},c});
    v.push_back({{-0.36185f,-0.02806f,-0.02590f},{0.0000f,-0.0000f,1.0000f},{0.2032f,0.8540f},c});
    v.push_back({{-0.36185f,-0.02806f,-0.02590f},{0.0000f,-1.0000f,-0.0000f},{0.3840f,0.7788f},c});
    v.push_back({{-0.25392f,-0.02806f,0.02590f},{0.0000f,-1.0000f,-0.0000f},{0.4592f,0.6221f},c});
    v.push_back({{-0.36185f,-0.02806f,0.02590f},{0.0000f,-1.0000f,-0.0000f},{0.4592f,0.7788f},c});
    v.push_back({{-0.36185f,-0.07987f,0.02590f},{-0.0000f,1.0000f,-0.0000f},{0.7151f,0.1567f},c});
    v.push_back({{-0.25392f,-0.07987f,-0.02590f},{-0.0000f,1.0000f,-0.0000f},{0.7904f,0.0000f},c});
    v.push_back({{-0.36185f,-0.07987f,-0.02590f},{-0.0000f,1.0000f,-0.0000f},{0.7904f,0.1567f},c});
    v.push_back({{-0.25392f,-0.02806f,-0.02590f},{0.2334f,0.9724f,-0.0000f},{0.9039f,0.4477f},c});
    v.push_back({{-0.29709f,-0.01770f,0.03626f},{0.2334f,0.9724f,-0.0000f},{0.9669f,0.5380f},c});
    v.push_back({{-0.25392f,-0.02806f,0.02590f},{0.2334f,0.9724f,-0.0000f},{0.9039f,0.5229f},c});
    v.push_back({{-0.25392f,-0.07987f,0.02590f},{0.2334f,-0.9724f,-0.0000f},{0.9023f,0.0150f},c});
    v.push_back({{-0.29709f,-0.09023f,-0.03626f},{0.2334f,-0.9724f,-0.0000f},{0.9653f,0.1053f},c});
    v.push_back({{-0.25392f,-0.07987f,-0.02590f},{0.2334f,-0.9724f,-0.0000f},{0.9023f,0.0903f},c});
    v.push_back({{-0.25392f,-0.02806f,0.02590f},{0.2334f,0.0000f,0.9724f},{0.8184f,0.9043f},c});
    v.push_back({{-0.29709f,-0.09023f,0.03626f},{0.2334f,0.0000f,0.9724f},{0.8702f,0.9945f},c});
    v.push_back({{-0.25392f,-0.07987f,0.02590f},{0.2334f,0.0000f,0.9724f},{0.8184f,0.9795f},c});
    v.push_back({{-0.25392f,-0.07987f,-0.02590f},{0.2334f,0.0000f,-0.9724f},{0.6791f,0.9055f},c});
    v.push_back({{-0.29709f,-0.01770f,-0.03626f},{0.2334f,0.0000f,-0.9724f},{0.7309f,0.9958f},c});
    v.push_back({{-0.25392f,-0.02806f,-0.02590f},{0.2334f,0.0000f,-0.9724f},{0.6791f,0.9807f},c});
    v.push_back({{-0.29709f,-0.09023f,0.03626f},{0.2444f,0.9697f,-0.0000f},{0.1260f,0.1369f},c});
    v.push_back({{-0.21075f,-0.11199f,-0.05802f},{0.2444f,0.9697f,-0.0000f},{-0.0000f,-0.0000f},c});
    v.push_back({{-0.29709f,-0.09023f,-0.03626f},{0.2444f,0.9697f,-0.0000f},{0.1260f,0.0316f},c});
    v.push_back({{-0.29709f,-0.01770f,0.03626f},{0.2444f,0.0000f,-0.9697f},{0.4808f,0.1369f},c});
    v.push_back({{-0.21075f,-0.11199f,0.05802f},{0.2444f,0.0000f,-0.9697f},{0.3778f,-0.0000f},c});
    v.push_back({{-0.29709f,-0.09023f,0.03626f},{0.2444f,0.0000f,-0.9697f},{0.4808f,0.0316f},c});
    v.push_back({{-0.29709f,-0.09023f,-0.03626f},{0.2444f,0.0000f,0.9697f},{0.4870f,0.4558f},c});
    v.push_back({{-0.21075f,0.00406f,-0.05802f},{0.2444f,0.0000f,0.9697f},{0.3840f,0.3189f},c});
    v.push_back({{-0.29709f,-0.01770f,-0.03626f},{0.2444f,0.0000f,0.9697f},{0.4870f,0.3505f},c});
    v.push_back({{-0.29709f,-0.01770f,-0.03626f},{0.2444f,-0.9697f,-0.0000f},{0.2520f,0.1369f},c});
    v.push_back({{-0.21075f,0.00406f,0.05802f},{0.2444f,-0.9697f,-0.0000f},{0.1260f,-0.0000f},c});
    v.push_back({{-0.29709f,-0.01770f,0.03626f},{0.2444f,-0.9697f,-0.0000f},{0.2520f,0.0316f},c});
    v.push_back({{-0.21075f,0.00406f,-0.05802f},{-0.5578f,0.8300f,-0.0000f},{0.7792f,0.1685f},c});
    v.push_back({{-0.25392f,-0.02495f,0.02901f},{-0.5578f,0.8300f,-0.0000f},{0.8409f,0.2948f},c});
    v.push_back({{-0.21075f,0.00406f,0.05802f},{-0.5578f,0.8300f,-0.0000f},{0.7792f,0.3370f},c});
    v.push_back({{-0.21075f,-0.11199f,0.05802f},{-0.5578f,-0.8300f,-0.0000f},{0.7792f,0.3370f},c});
    v.push_back({{-0.25392f,-0.08298f,-0.02901f},{-0.5578f,-0.8300f,-0.0000f},{0.8409f,0.4633f},c});
    v.push_back({{-0.21075f,-0.11199f,-0.05802f},{-0.5578f,-0.8300f,-0.0000f},{0.7792f,0.5054f},c});
    v.push_back({{-0.21075f,0.00406f,0.05802f},{-0.5578f,0.0000f,0.8300f},{0.5949f,0.6041f},c});
    v.push_back({{-0.25392f,-0.08298f,0.02901f},{-0.5578f,0.0000f,0.8300f},{0.6690f,0.7304f},c});
    v.push_back({{-0.21075f,-0.11199f,0.05802f},{-0.5578f,0.0000f,0.8300f},{0.5949f,0.7726f},c});
    v.push_back({{-0.21075f,-0.11199f,-0.05802f},{-0.5578f,0.0000f,-0.8300f},{0.6690f,0.6041f},c});
    v.push_back({{-0.25392f,-0.02495f,-0.02901f},{-0.5578f,0.0000f,-0.8300f},{0.7432f,0.7304f},c});
    v.push_back({{-0.21075f,0.00406f,-0.05802f},{-0.5578f,0.0000f,-0.8300f},{0.6690f,0.7726f},c});
    v.push_back({{-0.25392f,-0.02495f,0.02901f},{0.0000f,-0.0000f,-1.0000f},{0.5949f,0.8735f},c});
    v.push_back({{-0.17837f,-0.08298f,0.02901f},{0.0000f,-0.0000f,-1.0000f},{0.6791f,0.7726f},c});
    v.push_back({{-0.25392f,-0.08298f,0.02901f},{0.0000f,-0.0000f,-1.0000f},{0.6791f,0.8735f},c});
    v.push_back({{-0.25392f,-0.08298f,-0.02901f},{0.0000f,-0.0000f,1.0000f},{0.3840f,0.8797f},c});
    v.push_back({{-0.17837f,-0.02495f,-0.02901f},{0.0000f,-0.0000f,1.0000f},{0.4682f,0.7788f},c});
    v.push_back({{-0.25392f,-0.02495f,-0.02901f},{0.0000f,-0.0000f,1.0000f},{0.4682f,0.8797f},c});
    v.push_back({{-0.25392f,-0.02495f,-0.02901f},{0.0000f,-1.0000f,-0.0000f},{0.0000f,0.8314f},c});
    v.push_back({{-0.17837f,-0.02495f,0.02901f},{0.0000f,-1.0000f,-0.0000f},{0.0842f,0.7217f},c});
    v.push_back({{-0.25392f,-0.02495f,0.02901f},{0.0000f,-1.0000f,-0.0000f},{0.0842f,0.8314f},c});
    v.push_back({{-0.25392f,-0.08298f,0.02901f},{-0.0000f,1.0000f,-0.0000f},{0.4870f,0.8317f},c});
    v.push_back({{-0.17837f,-0.08298f,-0.02901f},{-0.0000f,1.0000f,-0.0000f},{0.5712f,0.7220f},c});
    v.push_back({{-0.25392f,-0.08298f,-0.02901f},{-0.0000f,1.0000f,-0.0000f},{0.5712f,0.8317f},c});
    v.push_back({{-0.17837f,-0.08298f,-0.02901f},{0.2596f,0.0000f,-0.9657f},{0.2032f,0.8770f},c});
    v.push_back({{-0.22154f,-0.01335f,-0.04062f},{0.2596f,0.0000f,-0.9657f},{0.2543f,0.9781f},c});
    v.push_back({{-0.17837f,-0.02495f,-0.02901f},{0.2596f,0.0000f,-0.9657f},{0.2032f,0.9613f},c});
    v.push_back({{-0.17837f,-0.02495f,-0.02901f},{0.2596f,0.9657f,-0.0000f},{0.6791f,0.7894f},c});
    v.push_back({{-0.22154f,-0.01335f,0.04062f},{0.2596f,0.9657f,-0.0000f},{0.7421f,0.8905f},c});
    v.push_back({{-0.17837f,-0.02495f,0.02901f},{0.2596f,0.9657f,-0.0000f},{0.6791f,0.8736f},c});
    v.push_back({{-0.17837f,-0.08298f,0.02901f},{0.2596f,-0.9657f,-0.0000f},{0.8409f,0.4495f},c});
    v.push_back({{-0.22154f,-0.09458f,-0.04062f},{0.2596f,-0.9657f,-0.0000f},{0.9039f,0.5506f},c});
    v.push_back({{-0.17837f,-0.08298f,-0.02901f},{0.2596f,-0.9657f,-0.0000f},{0.8409f,0.5337f},c});
    v.push_back({{-0.17837f,-0.02495f,0.02901f},{0.2596f,0.0000f,0.9657f},{0.3323f,0.8933f},c});
    v.push_back({{-0.22154f,-0.09458f,0.04062f},{0.2596f,0.0000f,0.9657f},{0.3834f,0.9944f},c});
    v.push_back({{-0.17837f,-0.08298f,0.02901f},{0.2596f,0.0000f,0.9657f},{0.3323f,0.9775f},c});
    v.push_back({{-0.22154f,-0.01335f,-0.04062f},{-0.1592f,-0.9872f,-0.0000f},{0.2373f,0.7098f},c});
    v.push_back({{-0.14599f,-0.02553f,0.02843f},{-0.1592f,-0.9872f,-0.0000f},{0.1280f,0.6095f},c});
    v.push_back({{-0.22154f,-0.01335f,0.04062f},{-0.1592f,-0.9872f,-0.0000f},{0.2373f,0.5918f},c});
    v.push_back({{-0.22154f,-0.09458f,0.04062f},{-0.1592f,0.9872f,-0.0000f},{0.3653f,0.7157f},c});
    v.push_back({{-0.14599f,-0.08240f,-0.02843f},{-0.1592f,0.9872f,-0.0000f},{0.2560f,0.6154f},c});
    v.push_back({{-0.22154f,-0.09458f,-0.04062f},{-0.1592f,0.9872f,-0.0000f},{0.3653f,0.5977f},c});
    v.push_back({{-0.22154f,-0.01335f,0.04062f},{-0.1592f,0.0000f,-0.9872f},{0.1079f,0.7217f},c});
    v.push_back({{-0.14599f,-0.08240f,0.02843f},{-0.1592f,0.0000f,-0.9872f},{0.0000f,0.6215f},c});
    v.push_back({{-0.22154f,-0.09458f,0.04062f},{-0.1592f,0.0000f,-0.9872f},{0.1079f,0.6038f},c});
    v.push_back({{-0.22154f,-0.01335f,-0.04062f},{-0.1592f,-0.0000f,0.9872f},{0.5949f,0.6041f},c});
    v.push_back({{-0.14599f,-0.08240f,-0.02843f},{-0.1592f,-0.0000f,0.9872f},{0.4870f,0.7043f},c});
    v.push_back({{-0.14599f,-0.02553f,-0.02843f},{-0.1592f,-0.0000f,0.9872f},{0.4870f,0.6218f},c});
    v.push_back({{-0.14599f,-0.02553f,0.02843f},{0.3675f,0.0000f,0.9300f},{0.9041f,0.1932f},c});
    v.push_back({{-0.18916f,-0.09945f,0.04549f},{0.3675f,0.0000f,0.9300f},{0.9521f,0.3006f},c});
    v.push_back({{-0.14599f,-0.08240f,0.02843f},{0.3675f,0.0000f,0.9300f},{0.9041f,0.2758f},c});
    v.push_back({{-0.14599f,-0.08240f,-0.02843f},{0.3675f,0.0000f,-0.9300f},{0.9041f,0.3253f},c});
    v.push_back({{-0.18916f,-0.00847f,-0.04549f},{0.3675f,0.0000f,-0.9300f},{0.9521f,0.4327f},c});
    v.push_back({{-0.14599f,-0.02553f,-0.02843f},{0.3675f,0.0000f,-0.9300f},{0.9041f,0.4079f},c});
    v.push_back({{-0.14599f,-0.02553f,-0.02843f},{0.3675f,0.9300f,-0.0000f},{0.8409f,0.1932f},c});
    v.push_back({{-0.18916f,-0.00847f,0.04549f},{0.3675f,0.9300f,-0.0000f},{0.9041f,0.3006f},c});
    v.push_back({{-0.14599f,-0.02553f,0.02843f},{0.3675f,0.9300f,-0.0000f},{0.8409f,0.2758f},c});
    v.push_back({{-0.14599f,-0.08240f,0.02843f},{0.3675f,-0.9300f,-0.0000f},{0.8409f,0.3253f},c});
    v.push_back({{-0.18916f,-0.09945f,-0.04549f},{0.3675f,-0.9300f,-0.0000f},{0.9041f,0.4327f},c});
    v.push_back({{-0.14599f,-0.08240f,-0.02843f},{0.3675f,-0.9300f,-0.0000f},{0.8409f,0.4079f},c});
    v.push_back({{-0.18916f,-0.00847f,-0.04549f},{-0.2062f,-0.9785f,-0.0000f},{0.1247f,0.6038f},c});
    v.push_back({{-0.10282f,-0.02667f,0.02729f},{-0.2062f,-0.9785f,-0.0000f},{-0.0000f,0.4981f},c});
    v.push_back({{-0.18916f,-0.00847f,0.04549f},{-0.2062f,-0.9785f,-0.0000f},{0.1247f,0.4717f},c});
    v.push_back({{-0.18916f,-0.09945f,0.04549f},{-0.2062f,0.9785f,-0.0000f},{0.6055f,0.1321f},c});
    v.push_back({{-0.10282f,-0.08126f,-0.02729f},{-0.2062f,0.9785f,-0.0000f},{0.4808f,0.0264f},c});
    v.push_back({{-0.18916f,-0.09945f,-0.04549f},{-0.2062f,0.9785f,-0.0000f},{0.6055f,0.0000f},c});
    v.push_back({{-0.18916f,-0.09945f,0.04549f},{-0.2062f,0.0000f,-0.9785f},{0.2537f,0.4597f},c});
    v.push_back({{-0.10282f,-0.02667f,0.02729f},{-0.2062f,0.0000f,-0.9785f},{0.1280f,0.5654f},c});
    v.push_back({{-0.10282f,-0.08126f,0.02729f},{-0.2062f,0.0000f,-0.9785f},{0.1280f,0.4862f},c});
    v.push_back({{-0.18916f,-0.00847f,-0.04549f},{-0.2062f,-0.0000f,0.9785f},{0.3817f,0.4656f},c});
    v.push_back({{-0.10282f,-0.08126f,-0.02729f},{-0.2062f,-0.0000f,0.9785f},{0.2560f,0.5713f},c});
    v.push_back({{-0.10282f,-0.02667f,-0.02729f},{-0.2062f,-0.0000f,0.9785f},{0.2560f,0.4920f},c});
    v.push_back({{-0.10282f,-0.08126f,-0.02729f},{0.4446f,-0.8958f,-0.0000f},{0.8184f,0.8615f},c});
    v.push_back({{-0.14599f,-0.10268f,0.04872f},{0.4446f,-0.8958f,-0.0000f},{0.8817f,0.7545f},c});
    v.push_back({{-0.14599f,-0.10268f,-0.04872f},{0.4446f,-0.8958f,-0.0000f},{0.8817f,0.8892f},c});
    v.push_back({{-0.10282f,-0.08126f,0.02729f},{0.4446f,0.0000f,0.8958f},{0.0782f,0.9384f},c});
    v.push_back({{-0.14599f,-0.00525f,0.04872f},{0.4446f,0.0000f,0.8958f},{0.1251f,0.8314f},c});
    v.push_back({{-0.14599f,-0.10268f,0.04872f},{0.4446f,0.0000f,0.8958f},{0.1251f,0.9661f},c});
    v.push_back({{-0.10282f,-0.02667f,-0.02729f},{0.4446f,0.0000f,-0.8958f},{0.9425f,0.7111f},c});
    v.push_back({{-0.14599f,-0.10268f,-0.04872f},{0.4446f,0.0000f,-0.8958f},{0.9893f,0.6041f},c});
    v.push_back({{-0.14599f,-0.00525f,-0.04872f},{0.4446f,0.0000f,-0.8958f},{0.9893f,0.7388f},c});
    v.push_back({{-0.10282f,-0.02667f,0.02729f},{0.4446f,0.8958f,0.0000f},{0.7432f,0.8553f},c});
    v.push_back({{-0.14599f,-0.00525f,-0.04872f},{0.4446f,0.8958f,0.0000f},{0.8065f,0.7483f},c});
    v.push_back({{-0.14599f,-0.00525f,0.04872f},{0.4446f,0.8958f,0.0000f},{0.8065f,0.8830f},c});
    v.push_back({{-0.14599f,-0.00525f,0.04872f},{0.0000f,-0.0000f,-1.0000f},{0.4870f,0.4693f},c});
    v.push_back({{-0.05965f,-0.10268f,0.04872f},{0.0000f,-0.0000f,-1.0000f},{0.6024f,0.6041f},c});
    v.push_back({{-0.14599f,-0.10268f,0.04872f},{0.0000f,-0.0000f,-1.0000f},{0.4870f,0.6041f},c});
    v.push_back({{-0.14599f,-0.10268f,-0.04872f},{0.0000f,-0.0000f,1.0000f},{0.5950f,0.3189f},c});
    v.push_back({{-0.05965f,-0.00524f,-0.04872f},{0.0000f,-0.0000f,1.0000f},{0.7104f,0.4536f},c});
    v.push_back({{-0.14599f,-0.00525f,-0.04872f},{0.0000f,-0.0000f,1.0000f},{0.5950f,0.4536f},c});
    v.push_back({{-0.14599f,-0.00525f,-0.04872f},{0.0000f,-1.0000f,-0.0000f},{0.2533f,0.4597f},c});
    v.push_back({{-0.05965f,-0.00524f,0.04872f},{0.0000f,-1.0000f,-0.0000f},{0.1280f,0.3250f},c});
    v.push_back({{-0.14599f,-0.00525f,0.04872f},{0.0000f,-1.0000f,-0.0000f},{0.2533f,0.3250f},c});
    v.push_back({{-0.14599f,-0.10268f,0.04872f},{-0.0000f,1.0000f,-0.0000f},{0.5071f,0.3032f},c});
    v.push_back({{-0.05965f,-0.10268f,-0.04872f},{-0.0000f,1.0000f,-0.0000f},{0.3817f,0.1685f},c});
    v.push_back({{-0.14599f,-0.10268f,-0.04872f},{-0.0000f,1.0000f,-0.0000f},{0.5071f,0.1685f},c});
    v.push_back({{-0.05965f,-0.00524f,-0.04872f},{-0.0613f,0.9981f,-0.0000f},{0.6055f,0.0000f},c});
    v.push_back({{-0.13520f,-0.00988f,0.04408f},{-0.0613f,0.9981f,-0.0000f},{0.7151f,0.1314f},c});
    v.push_back({{-0.05965f,-0.00524f,0.04872f},{-0.0613f,0.9981f,-0.0000f},{0.6055f,0.1347f},c});
    v.push_back({{-0.05965f,-0.10268f,0.04872f},{-0.0613f,-0.9981f,-0.0000f},{0.6024f,0.4693f},c});
    v.push_back({{-0.13520f,-0.09804f,-0.04408f},{-0.0613f,-0.9981f,-0.0000f},{0.7120f,0.6007f},c});
    v.push_back({{-0.05965f,-0.10268f,-0.04872f},{-0.0613f,-0.9981f,-0.0000f},{0.6024f,0.6041f},c});
    v.push_back({{-0.05965f,-0.10268f,0.04872f},{-0.0613f,0.0000f,0.9981f},{0.3840f,0.6221f},c});
    v.push_back({{-0.13520f,-0.00988f,0.04408f},{-0.0613f,0.0000f,0.9981f},{0.4863f,0.4908f},c});
    v.push_back({{-0.13520f,-0.09804f,0.04408f},{-0.0613f,0.0000f,0.9981f},{0.4863f,0.6188f},c});
    v.push_back({{-0.05965f,-0.10268f,-0.04872f},{-0.0613f,0.0000f,-0.9981f},{0.6151f,0.1685f},c});
    v.push_back({{-0.13520f,-0.00988f,-0.04408f},{-0.0613f,0.0000f,-0.9981f},{0.7173f,0.2998f},c});
    v.push_back({{-0.05965f,-0.00524f,-0.04872f},{-0.0613f,0.0000f,-0.9981f},{0.6151f,0.3032f},c});
    v.push_back({{-0.13520f,-0.00988f,0.04408f},{-0.0408f,0.0000f,-0.9992f},{0.2560f,0.4656f},c});
    v.push_back({{-0.02727f,-0.09364f,0.03967f},{-0.0408f,0.0000f,-0.9992f},{0.3776f,0.3189f},c});
    v.push_back({{-0.13520f,-0.09804f,0.04408f},{-0.0408f,0.0000f,-0.9992f},{0.3840f,0.4656f},c});
    v.push_back({{-0.13520f,-0.09804f,-0.04408f},{-0.0408f,0.0000f,0.9992f},{0.0000f,0.4717f},c});
    v.push_back({{-0.02727f,-0.01429f,-0.03967f},{-0.0408f,0.0000f,0.9992f},{0.1216f,0.3250f},c});
    v.push_back({{-0.13520f,-0.00988f,-0.04408f},{-0.0408f,0.0000f,0.9992f},{0.1280f,0.4717f},c});
    v.push_back({{-0.13520f,-0.00988f,-0.04408f},{-0.0408f,-0.9992f,-0.0000f},{0.1280f,0.3250f},c});
    v.push_back({{-0.02727f,-0.01429f,0.03967f},{-0.0408f,-0.9992f,-0.0000f},{0.2496f,0.1685f},c});
    v.push_back({{-0.13520f,-0.00988f,0.04408f},{-0.0408f,-0.9992f,-0.0000f},{0.2560f,0.3250f},c});
    v.push_back({{-0.13520f,-0.09804f,0.04408f},{-0.0408f,0.9992f,-0.0000f},{0.0000f,0.3250f},c});
    v.push_back({{-0.02727f,-0.09364f,-0.03967f},{-0.0408f,0.9992f,-0.0000f},{0.1216f,0.1685f},c});
    v.push_back({{-0.13520f,-0.09804f,-0.04408f},{-0.0408f,0.9992f,-0.0000f},{0.1280f,0.3250f},c});
    v.push_back({{-0.02727f,-0.09364f,0.03967f},{-0.5224f,0.0000f,0.8527f},{0.4592f,0.7373f},c});
    v.push_back({{-0.04022f,-0.02223f,0.03174f},{-0.5224f,0.0000f,0.8527f},{0.4810f,0.6336f},c});
    v.push_back({{-0.04022f,-0.08570f,0.03174f},{-0.5224f,0.0000f,0.8527f},{0.4810f,0.7258f},c});
    v.push_back({{-0.02727f,-0.01429f,-0.03967f},{-0.5224f,0.0000f,-0.8527f},{0.5712f,0.8372f},c});
    v.push_back({{-0.04022f,-0.08570f,-0.03174f},{-0.5224f,0.0000f,-0.8527f},{0.5931f,0.7335f},c});
    v.push_back({{-0.04022f,-0.02223f,-0.03174f},{-0.5224f,0.0000f,-0.8527f},{0.5931f,0.8257f},c});
    v.push_back({{-0.02727f,-0.01429f,-0.03967f},{-0.5224f,0.8527f,-0.0000f},{0.2373f,0.5918f},c});
    v.push_back({{-0.04022f,-0.02223f,0.03174f},{-0.5224f,0.8527f,-0.0000f},{0.2558f,0.6955f},c});
    v.push_back({{-0.02727f,-0.01429f,0.03967f},{-0.5224f,0.8527f,-0.0000f},{0.2373f,0.7070f},c});
    v.push_back({{-0.02727f,-0.09364f,0.03967f},{-0.5224f,-0.8527f,-0.0000f},{0.3653f,0.5977f},c});
    v.push_back({{-0.04022f,-0.08570f,-0.03174f},{-0.5224f,-0.8527f,-0.0000f},{0.3838f,0.7014f},c});
    v.push_back({{-0.02727f,-0.09364f,-0.03967f},{-0.5224f,-0.8527f,-0.0000f},{0.3653f,0.7129f},c});
    v.push_back({{-0.04022f,-0.02223f,-0.03174f},{-0.0587f,-0.9983f,-0.0000f},{0.0782f,0.9236f},c});
    v.push_back({{0.01374f,-0.02540f,0.02856f},{-0.0587f,-0.9983f,-0.0000f},{-0.0000f,0.8360f},c});
    v.push_back({{-0.04022f,-0.02223f,0.03174f},{-0.0587f,-0.9983f,-0.0000f},{0.0782f,0.8314f},c});
    v.push_back({{-0.04022f,-0.08570f,0.03174f},{-0.0587f,0.9983f,-0.0000f},{0.5652f,0.9238f},c});
    v.push_back({{0.01374f,-0.08253f,-0.02856f},{-0.0587f,0.9983f,-0.0000f},{0.4870f,0.8363f},c});
    v.push_back({{-0.04022f,-0.08570f,-0.03174f},{-0.0587f,0.9983f,-0.0000f},{0.5652f,0.8317f},c});
    v.push_back({{-0.04022f,-0.02223f,0.03174f},{-0.0587f,0.0000f,-0.9983f},{0.2019f,0.9461f},c});
    v.push_back({{0.01374f,-0.08253f,0.02856f},{-0.0587f,0.0000f,-0.9983f},{0.1280f,0.8586f},c});
    v.push_back({{-0.04022f,-0.08570f,0.03174f},{-0.0587f,0.0000f,-0.9983f},{0.2019f,0.8540f},c});
    v.push_back({{-0.04022f,-0.08570f,-0.03174f},{-0.0587f,0.0000f,0.9983f},{0.9556f,0.9595f},c});
    v.push_back({{0.01374f,-0.02540f,-0.02856f},{-0.0587f,0.0000f,0.9983f},{0.8817f,0.8719f},c});
    v.push_back({{-0.04022f,-0.02223f,-0.03174f},{-0.0587f,0.0000f,0.9983f},{0.9556f,0.8673f},c});
    v.push_back({{0.01374f,-0.02540f,-0.02856f},{-0.2558f,0.9667f,-0.0000f},{0.9521f,0.2813f},c});
    v.push_back({{-0.00784f,-0.03111f,0.02285f},{-0.2558f,0.9667f,-0.0000f},{0.9833f,0.3560f},c});
    v.push_back({{0.01374f,-0.02540f,0.02856f},{-0.2558f,0.9667f,-0.0000f},{0.9521f,0.3642f},c});
    v.push_back({{0.01374f,-0.08253f,0.02856f},{-0.2558f,-0.9667f,-0.0000f},{0.9556f,0.8673f},c});
    v.push_back({{-0.00784f,-0.07682f,-0.02285f},{-0.2558f,-0.9667f,-0.0000f},{0.9867f,0.9420f},c});
    v.push_back({{0.01374f,-0.08253f,-0.02856f},{-0.2558f,-0.9667f,-0.0000f},{0.9556f,0.9503f},c});
    v.push_back({{0.01374f,-0.08253f,0.02856f},{-0.2558f,0.0000f,0.9667f},{0.0842f,0.8047f},c});
    v.push_back({{-0.00784f,-0.03111f,0.02285f},{-0.2558f,0.0000f,0.9667f},{0.1163f,0.7300f},c});
    v.push_back({{-0.00784f,-0.07682f,0.02285f},{-0.2558f,0.0000f,0.9667f},{0.1163f,0.7964f},c});
    v.push_back({{0.01374f,-0.08253f,-0.02856f},{-0.2558f,0.0000f,-0.9667f},{0.7792f,0.5054f},c});
    v.push_back({{-0.00784f,-0.03111f,-0.02285f},{-0.2558f,0.0000f,-0.9667f},{0.8113f,0.5801f},c});
    v.push_back({{0.01374f,-0.02540f,-0.02856f},{-0.2558f,0.0000f,-0.9667f},{0.7792f,0.5884f},c});
    v.push_back({{-0.00784f,-0.07682f,0.02285f},{0.0610f,-0.0000f,-0.9981f},{0.4570f,0.8847f},c});
    v.push_back({{0.04828f,-0.02769f,0.02628f},{0.0610f,-0.0000f,-0.9981f},{0.3840f,0.9560f},c});
    v.push_back({{0.04828f,-0.08024f,0.02628f},{0.0610f,-0.0000f,-0.9981f},{0.3840f,0.8797f},c});
    v.push_back({{-0.00784f,-0.07682f,-0.02285f},{0.0610f,0.0000f,0.9981f},{0.8162f,0.9543f},c});
    v.push_back({{0.04828f,-0.02769f,-0.02628f},{0.0610f,0.0000f,0.9981f},{0.7432f,0.8830f},c});
    v.push_back({{-0.00784f,-0.03111f,-0.02285f},{0.0610f,0.0000f,0.9981f},{0.8162f,0.8880f},c});
    v.push_back({{-0.00784f,-0.03111f,-0.02285f},{0.0610f,-0.9981f,-0.0000f},{0.2610f,0.9580f},c});
    v.push_back({{0.04828f,-0.02769f,0.02628f},{0.0610f,-0.9981f,-0.0000f},{0.3323f,0.8765f},c});
    v.push_back({{-0.00784f,-0.03111f,0.02285f},{0.0610f,-0.9981f,-0.0000f},{0.3273f,0.9580f},c});
    v.push_back({{-0.00784f,-0.07682f,0.02285f},{0.0610f,0.9981f,-0.0000f},{0.5998f,0.9551f},c});
    v.push_back({{0.04828f,-0.08024f,-0.02628f},{0.0610f,0.9981f,-0.0000f},{0.6712f,0.8735f},c});
    v.push_back({{-0.00784f,-0.07682f,-0.02285f},{0.0610f,0.9981f,-0.0000f},{0.6662f,0.9551f},c});
    v.push_back({{0.04828f,-0.08024f,0.02628f},{-0.2366f,-0.9716f,-0.0000f},{0.5949f,0.8735f},c});
    v.push_back({{0.00511f,-0.06973f,-0.01577f},{-0.2366f,-0.9716f,-0.0000f},{0.6712f,0.8735f},c});
    v.push_back({{0.04828f,-0.08024f,-0.02628f},{-0.2366f,-0.9716f,-0.0000f},{0.6712f,0.8735f},c});
    v.push_back({{0.04828f,-0.02769f,0.02628f},{-0.2366f,0.0000f,0.9716f},{0.3840f,0.9560f},c});
    v.push_back({{0.00511f,-0.06973f,0.01577f},{-0.2366f,0.0000f,0.9716f},{0.3840f,0.8797f},c});
    v.push_back({{0.04828f,-0.08024f,0.02628f},{-0.2366f,0.0000f,0.9716f},{0.3840f,0.8797f},c});
    v.push_back({{0.04828f,-0.08024f,-0.02628f},{-0.2366f,0.0000f,-0.9716f},{0.7432f,0.9593f},c});
    v.push_back({{0.00511f,-0.03820f,-0.01577f},{-0.2366f,0.0000f,-0.9716f},{0.7432f,0.8830f},c});
    v.push_back({{0.04828f,-0.02769f,-0.02628f},{-0.2366f,0.0000f,-0.9716f},{0.7432f,0.8830f},c});
    v.push_back({{0.04828f,-0.02769f,-0.02628f},{-0.2366f,0.9716f,-0.0000f},{0.2560f,0.8765f},c});
    v.push_back({{0.00511f,-0.03820f,0.01577f},{-0.2366f,0.9716f,-0.0000f},{0.3323f,0.8765f},c});
    v.push_back({{0.04828f,-0.02769f,0.02628f},{-0.2366f,0.9716f,-0.0000f},{0.3323f,0.8765f},c});
    v.push_back({{0.00511f,-0.03820f,-0.01577f},{0.0000f,-1.0000f,-0.0000f},{0.2560f,0.8765f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{0.0000f,-1.0000f,-0.0000f},{0.3323f,0.8765f},c});
    v.push_back({{0.00511f,-0.03820f,0.01577f},{0.0000f,-1.0000f,-0.0000f},{0.3323f,0.8765f},c});
    v.push_back({{0.00511f,-0.06973f,0.01577f},{-0.0000f,1.0000f,-0.0000f},{0.5949f,0.8735f},c});
    v.push_back({{0.11304f,-0.06973f,-0.01577f},{-0.0000f,1.0000f,-0.0000f},{0.6712f,0.8735f},c});
    v.push_back({{0.00511f,-0.06973f,-0.01577f},{-0.0000f,1.0000f,-0.0000f},{0.6712f,0.8735f},c});
    v.push_back({{0.00511f,-0.03820f,0.01577f},{0.0000f,-0.0000f,-1.0000f},{0.3840f,0.9560f},c});
    v.push_back({{0.11304f,-0.06973f,0.01577f},{0.0000f,-0.0000f,-1.0000f},{0.3840f,0.8797f},c});
    v.push_back({{0.00511f,-0.06973f,0.01577f},{0.0000f,-0.0000f,-1.0000f},{0.3840f,0.8797f},c});
    v.push_back({{0.00511f,-0.06973f,-0.01577f},{0.0000f,-0.0000f,1.0000f},{0.7432f,0.9593f},c});
    v.push_back({{0.11304f,-0.03820f,-0.01577f},{0.0000f,-0.0000f,1.0000f},{0.7432f,0.8830f},c});
    v.push_back({{0.00511f,-0.03820f,-0.01577f},{0.0000f,-0.0000f,1.0000f},{0.7432f,0.8830f},c});
    v.push_back({{-0.33040f,-0.03454f,-0.11011f},{0.3710f,0.9285f,0.0178f},{0.8468f,0.9257f},c});
    v.push_back({{-0.36660f,-0.01900f,-0.16631f},{0.3710f,0.9285f,0.0178f},{0.9050f,0.8443f},c});
    v.push_back({{-0.36996f,-0.01900f,-0.09645f},{0.3710f,0.9285f,0.0178f},{0.9050f,0.9489f},c});
    v.push_back({{-0.33040f,-0.03454f,-0.11011f},{0.3264f,0.0000f,0.9452f},{0.0000f,0.8522f},c});
    v.push_back({{-0.36996f,-0.08893f,-0.09645f},{0.3264f,0.0000f,0.9452f},{0.0606f,0.9335f},c});
    v.push_back({{-0.33040f,-0.07339f,-0.11011f},{0.3264f,0.0000f,0.9452f},{0.0000f,0.9103f},c});
    v.push_back({{-0.33040f,-0.07339f,-0.11011f},{0.3710f,-0.9285f,0.0178f},{0.8889f,0.0658f},c});
    v.push_back({{-0.36660f,-0.08893f,-0.16631f},{0.3710f,-0.9285f,0.0178f},{0.9476f,0.1472f},c});
    v.push_back({{-0.32854f,-0.07339f,-0.14892f},{0.3710f,-0.9285f,0.0178f},{0.8889f,0.1239f},c});
    v.push_back({{-0.32854f,-0.07339f,-0.14892f},{0.4155f,0.0000f,-0.9096f},{0.0606f,0.8522f},c});
    v.push_back({{-0.36660f,-0.01900f,-0.16631f},{0.4155f,0.0000f,-0.9096f},{0.1148f,0.9335f},c});
    v.push_back({{-0.32854f,-0.03454f,-0.14892f},{0.4155f,0.0000f,-0.9096f},{0.0606f,0.9103f},c});
    v.push_back({{-0.44590f,-0.05396f,-0.13511f},{0.1642f,-0.9864f,0.0079f},{0.7568f,0.7407f},c});
    v.push_back({{-0.32854f,-0.03454f,-0.14892f},{0.1642f,-0.9864f,0.0079f},{0.7277f,0.5657f},c});
    v.push_back({{-0.33040f,-0.03454f,-0.11011f},{0.1642f,-0.9864f,0.0079f},{0.7858f,0.5657f},c});
    v.push_back({{-0.32854f,-0.03454f,-0.14892f},{0.1169f,0.0000f,0.9931f},{0.7722f,0.0426f},c});
    v.push_back({{-0.44590f,-0.05396f,-0.13511f},{0.1169f,0.0000f,0.9931f},{0.7432f,0.2192f},c});
    v.push_back({{-0.32854f,-0.07339f,-0.14892f},{0.1169f,0.0000f,0.9931f},{0.7141f,0.0426f},c});
    v.push_back({{-0.44590f,-0.05396f,-0.13511f},{0.1642f,0.9864f,0.0079f},{0.7615f,0.4106f},c});
    v.push_back({{-0.33040f,-0.07339f,-0.11011f},{0.1642f,0.9864f,0.0079f},{0.7324f,0.2362f},c});
    v.push_back({{-0.32854f,-0.07339f,-0.14892f},{0.1642f,0.9864f,0.0079f},{0.7905f,0.2362f},c});
    v.push_back({{-0.44590f,-0.05396f,-0.13511f},{0.2115f,-0.0000f,-0.9774f},{0.1454f,0.8385f},c});
    v.push_back({{-0.33040f,-0.03454f,-0.11011f},{0.2115f,-0.0000f,-0.9774f},{0.1163f,0.6883f},c});
    v.push_back({{-0.33040f,-0.07339f,-0.11011f},{0.2115f,-0.0000f,-0.9774f},{0.1745f,0.6883f},c});
    v.push_back({{-0.36660f,-0.08893f,-0.16631f},{0.1007f,0.0000f,0.9949f},{0.3525f,0.4233f},c});
    v.push_back({{-0.28842f,-0.00734f,-0.17422f},{0.1007f,0.0000f,0.9949f},{0.2350f,0.3013f},c});
    v.push_back({{-0.36660f,-0.01900f,-0.16631f},{0.1007f,0.0000f,0.9949f},{0.3525f,0.3187f},c});
    v.push_back({{-0.36996f,-0.01900f,-0.09645f},{0.1956f,0.0000f,-0.9807f},{0.6321f,0.4233f},c});
    v.push_back({{-0.29290f,-0.10059f,-0.08108f},{0.1956f,0.0000f,-0.9807f},{0.5312f,0.3013f},c});
    v.push_back({{-0.36996f,-0.08893f,-0.09645f},{0.1956f,0.0000f,-0.9807f},{0.6321f,0.3187f},c});
    v.push_back({{-0.36996f,-0.08893f,-0.09645f},{0.1482f,0.9889f,0.0071f},{0.1163f,0.4344f},c});
    v.push_back({{-0.28842f,-0.10059f,-0.17422f},{0.1482f,0.9889f,0.0071f},{0.0000f,0.3124f},c});
    v.push_back({{-0.36660f,-0.08893f,-0.16631f},{0.1482f,0.9889f,0.0071f},{0.1163f,0.3298f},c});
    v.push_back({{-0.36660f,-0.01900f,-0.16631f},{0.1482f,-0.9889f,0.0071f},{0.2330f,0.4235f},c});
    v.push_back({{-0.29290f,-0.00734f,-0.08108f},{0.1482f,-0.9889f,0.0071f},{0.1163f,0.3015f},c});
    v.push_back({{-0.36996f,-0.01900f,-0.09645f},{0.1482f,-0.9889f,0.0071f},{0.2330f,0.3189f},c});
    v.push_back({{-0.29290f,-0.00734f,-0.08108f},{-0.6377f,0.0000f,0.7703f},{0.1745f,0.6883f},c});
    v.push_back({{-0.32283f,-0.07728f,-0.10586f},{-0.6377f,0.0000f,0.7703f},{0.2318f,0.7929f},c});
    v.push_back({{-0.29290f,-0.10059f,-0.08108f},{-0.6377f,0.0000f,0.7703f},{0.1745f,0.8278f},c});
    v.push_back({{-0.28842f,-0.10059f,-0.17422f},{-0.5609f,0.0000f,-0.8279f},{0.7905f,0.2362f},c});
    v.push_back({{-0.32059f,-0.03065f,-0.15243f},{-0.5609f,0.0000f,-0.8279f},{0.8468f,0.3408f},c});
    v.push_back({{-0.28842f,-0.00734f,-0.17422f},{-0.5609f,0.0000f,-0.8279f},{0.7905f,0.3756f},c});
    v.push_back({{-0.28842f,-0.00734f,-0.17422f},{-0.5993f,0.8000f,-0.0288f},{0.0697f,0.6837f},c});
    v.push_back({{-0.32283f,-0.03065f,-0.10586f},{-0.5993f,0.8000f,-0.0288f},{0.1161f,0.7883f},c});
    v.push_back({{-0.29290f,-0.00734f,-0.08108f},{-0.5993f,0.8000f,-0.0288f},{0.0697f,0.8231f},c});
    v.push_back({{-0.28842f,-0.10059f,-0.17422f},{-0.5993f,-0.8000f,-0.0288f},{0.8854f,0.5552f},c});
    v.push_back({{-0.32283f,-0.07728f,-0.10586f},{-0.5993f,-0.8000f,-0.0288f},{0.9309f,0.4506f},c});
    v.push_back({{-0.32059f,-0.07728f,-0.15243f},{-0.5993f,-0.8000f,-0.0288f},{0.9309f,0.5203f},c});
    v.push_back({{-0.32283f,-0.03065f,-0.10586f},{0.0480f,0.0000f,-0.9988f},{0.7375f,0.5501f},c});
    v.push_back({{-0.22580f,-0.07728f,-0.10120f},{0.0480f,0.0000f,-0.9988f},{0.8073f,0.4157f},c});
    v.push_back({{-0.32283f,-0.07728f,-0.10586f},{0.0480f,0.0000f,-0.9988f},{0.8073f,0.5501f},c});
    v.push_back({{-0.32059f,-0.07728f,-0.15243f},{-0.0480f,0.0000f,0.9988f},{0.3525f,0.8319f},c});
    v.push_back({{-0.22357f,-0.03065f,-0.14777f},{-0.0480f,0.0000f,0.9988f},{0.4222f,0.6878f},c});
    v.push_back({{-0.32059f,-0.03065f,-0.15243f},{-0.0480f,0.0000f,0.9988f},{0.4222f,0.8319f},c});
    v.push_back({{-0.32283f,-0.03065f,-0.10586f},{-0.0000f,-1.0000f,-0.0000f},{0.0697f,0.8289f},c});
    v.push_back({{-0.22357f,-0.03065f,-0.14777f},{-0.0000f,-1.0000f,-0.0000f},{0.0000f,0.6837f},c});
    v.push_back({{-0.22580f,-0.03065f,-0.10120f},{-0.0000f,-1.0000f,-0.0000f},{0.0697f,0.6837f},c});
    v.push_back({{-0.32283f,-0.07728f,-0.10586f},{-0.0000f,1.0000f,-0.0000f},{0.6265f,0.8203f},c});
    v.push_back({{-0.22357f,-0.07728f,-0.14777f},{-0.0000f,1.0000f,-0.0000f},{0.6962f,0.6750f},c});
    v.push_back({{-0.32059f,-0.07728f,-0.15243f},{-0.0000f,1.0000f,-0.0000f},{0.6962f,0.8203f},c});
    v.push_back({{-0.22357f,-0.03065f,-0.14777f},{0.2331f,0.9724f,0.0112f},{0.9050f,0.2501f},c});
    v.push_back({{-0.26506f,-0.02133f,-0.09375f},{0.2331f,0.9724f,0.0112f},{0.9632f,0.3338f},c});
    v.push_back({{-0.22580f,-0.03065f,-0.10120f},{0.2331f,0.9724f,0.0112f},{0.9050f,0.3199f},c});
    v.push_back({{-0.22357f,-0.07728f,-0.14777f},{0.2331f,-0.9724f,0.0112f},{0.9050f,0.9280f},c});
    v.push_back({{-0.26506f,-0.08660f,-0.09375f},{0.2331f,-0.9724f,0.0112f},{0.9634f,0.8443f},c});
    v.push_back({{-0.26193f,-0.08660f,-0.15895f},{0.2331f,-0.9724f,0.0112f},{0.9634f,0.9420f},c});
    v.push_back({{-0.22580f,-0.03065f,-0.10120f},{0.1865f,0.0000f,0.9825f},{0.9012f,0.5796f},c});
    v.push_back({{-0.26506f,-0.08660f,-0.09375f},{0.1865f,0.0000f,0.9825f},{0.9606f,0.6633f},c});
    v.push_back({{-0.22580f,-0.07728f,-0.10120f},{0.1865f,0.0000f,0.9825f},{0.9012f,0.6493f},c});
    v.push_back({{-0.22357f,-0.07728f,-0.14777f},{0.2798f,0.0000f,-0.9601f},{0.9309f,0.4297f},c});
    v.push_back({{-0.26193f,-0.02133f,-0.15895f},{0.2798f,0.0000f,-0.9601f},{0.9794f,0.5133f},c});
    v.push_back({{-0.22357f,-0.03065f,-0.14777f},{0.2798f,0.0000f,-0.9601f},{0.9309f,0.4994f},c});
    v.push_back({{-0.26506f,-0.08660f,-0.09375f},{0.2441f,0.9697f,0.0117f},{0.1163f,0.2831f},c});
    v.push_back({{-0.18337f,-0.10619f,-0.17478f},{0.2441f,0.9697f,0.0117f},{0.0000f,0.1562f},c});
    v.push_back({{-0.26193f,-0.08660f,-0.15895f},{0.2441f,0.9697f,0.0117f},{0.1163f,0.1855f},c});
    v.push_back({{-0.26506f,-0.08660f,-0.09375f},{0.2906f,-0.0000f,-0.9568f},{0.4510f,0.0293f},c});
    v.push_back({{-0.18838f,-0.00174f,-0.07046f},{0.2906f,-0.0000f,-0.9568f},{0.3546f,0.1562f},c});
    v.push_back({{-0.18838f,-0.10619f,-0.07046f},{0.2906f,-0.0000f,-0.9568f},{0.3546f,-0.0000f},c});
    v.push_back({{-0.26193f,-0.08660f,-0.15895f},{0.1976f,0.0000f,0.9803f},{0.1190f,0.1269f},c});
    v.push_back({{-0.18337f,-0.00174f,-0.17478f},{0.1976f,0.0000f,0.9803f},{0.0000f,0.0000f},c});
    v.push_back({{-0.26193f,-0.02133f,-0.15895f},{0.1976f,0.0000f,0.9803f},{0.1190f,0.0293f},c});
    v.push_back({{-0.26506f,-0.02133f,-0.09375f},{0.2441f,-0.9697f,0.0117f},{0.2359f,0.0293f},c});
    v.push_back({{-0.18337f,-0.00174f,-0.17478f},{0.2441f,-0.9697f,0.0117f},{0.1190f,0.1562f},c});
    v.push_back({{-0.18838f,-0.00174f,-0.07046f},{0.2441f,-0.9697f,0.0117f},{0.1190f,-0.0000f},c});
    v.push_back({{-0.18337f,-0.00174f,-0.17478f},{-0.5571f,0.8300f,-0.0268f},{0.7722f,0.0426f},c});
    v.push_back({{-0.22594f,-0.02785f,-0.09840f},{-0.5571f,0.8300f,-0.0268f},{0.8302f,0.1597f},c});
    v.push_back({{-0.18838f,-0.00174f,-0.07046f},{-0.5571f,0.8300f,-0.0268f},{0.7722f,0.1988f},c});
    v.push_back({{-0.18337f,-0.10619f,-0.17478f},{-0.5571f,-0.8300f,-0.0268f},{0.7858f,0.7219f},c});
    v.push_back({{-0.22594f,-0.08007f,-0.09840f},{-0.5571f,-0.8300f,-0.0268f},{0.8429f,0.6047f},c});
    v.push_back({{-0.22343f,-0.08007f,-0.15056f},{-0.5571f,-0.8300f,-0.0268f},{0.8429f,0.6828f},c});
    v.push_back({{-0.18838f,-0.10619f,-0.07046f},{-0.5969f,0.0000f,0.8023f},{0.4542f,0.7190f},c});
    v.push_back({{-0.22594f,-0.02785f,-0.09840f},{-0.5969f,0.0000f,0.8023f},{0.5226f,0.6019f},c});
    v.push_back({{-0.22594f,-0.08007f,-0.09840f},{-0.5969f,0.0000f,0.8023f},{0.5226f,0.6800f},c});
    v.push_back({{-0.18337f,-0.10619f,-0.17478f},{-0.5173f,0.0000f,-0.8558f},{0.6455f,0.0426f},c});
    v.push_back({{-0.22343f,-0.02785f,-0.15056f},{-0.5173f,0.0000f,-0.8558f},{0.7141f,0.1597f},c});
    v.push_back({{-0.18337f,-0.00174f,-0.17478f},{-0.5173f,0.0000f,-0.8558f},{0.6455f,0.1988f},c});
    v.push_back({{-0.22594f,-0.02785f,-0.09840f},{0.0480f,0.0000f,-0.9988f},{0.8073f,0.5098f},c});
    v.push_back({{-0.15802f,-0.08007f,-0.09514f},{0.0480f,0.0000f,-0.9988f},{0.8854f,0.4157f},c});
    v.push_back({{-0.22594f,-0.08007f,-0.09840f},{0.0480f,0.0000f,-0.9988f},{0.8854f,0.5098f},c});
    v.push_back({{-0.22343f,-0.08007f,-0.15056f},{-0.0480f,0.0000f,0.9988f},{0.4445f,0.8199f},c});
    v.push_back({{-0.15552f,-0.02785f,-0.14730f},{-0.0480f,0.0000f,0.9988f},{0.5226f,0.7190f},c});
    v.push_back({{-0.22343f,-0.02785f,-0.15056f},{-0.0480f,0.0000f,0.9988f},{0.5226f,0.8199f},c});
    v.push_back({{-0.22594f,-0.02785f,-0.09840f},{-0.0000f,-1.0000f,-0.0000f},{0.6093f,0.7922f},c});
    v.push_back({{-0.15552f,-0.02785f,-0.14730f},{-0.0000f,-1.0000f,-0.0000f},{0.5312f,0.6906f},c});
    v.push_back({{-0.15802f,-0.02785f,-0.09514f},{-0.0000f,-1.0000f,-0.0000f},{0.6093f,0.6906f},c});
    v.push_back({{-0.22594f,-0.08007f,-0.09840f},{0.0000f,1.0000f,-0.0000f},{0.2350f,0.7923f},c});
    v.push_back({{-0.15552f,-0.08007f,-0.14730f},{0.0000f,1.0000f,-0.0000f},{0.3131f,0.6906f},c});
    v.push_back({{-0.22343f,-0.08007f,-0.15056f},{0.0000f,1.0000f,-0.0000f},{0.3131f,0.7923f},c});
    v.push_back({{-0.15552f,-0.08007f,-0.14730f},{0.3056f,0.0000f,-0.9522f},{0.7986f,0.8624f},c});
    v.push_back({{-0.19382f,-0.01741f,-0.15960f},{0.3056f,0.0000f,-0.9522f},{0.8468f,0.9561f},c});
    v.push_back({{-0.15552f,-0.02785f,-0.14730f},{0.3056f,0.0000f,-0.9522f},{0.7986f,0.9405f},c});
    v.push_back({{-0.15802f,-0.02785f,-0.09514f},{0.2593f,0.9657f,0.0125f},{0.6265f,0.9140f},c});
    v.push_back({{-0.19382f,-0.01741f,-0.15960f},{0.2593f,0.9657f,0.0125f},{0.6847f,0.8203f},c});
    v.push_back({{-0.19733f,-0.01741f,-0.08657f},{0.2593f,0.9657f,0.0125f},{0.6847f,0.9296f},c});
    v.push_back({{-0.15802f,-0.08007f,-0.09514f},{0.2593f,-0.9657f,0.0125f},{0.4445f,0.8355f},c});
    v.push_back({{-0.19382f,-0.09052f,-0.15960f},{0.2593f,-0.9657f,0.0125f},{0.5030f,0.9293f},c});
    v.push_back({{-0.15552f,-0.08007f,-0.14730f},{0.2593f,-0.9657f,0.0125f},{0.4445f,0.9136f},c});
    v.push_back({{-0.15802f,-0.02785f,-0.09514f},{0.2130f,0.0000f,0.9771f},{0.2889f,0.8079f},c});
    v.push_back({{-0.19733f,-0.09052f,-0.08657f},{0.2130f,0.0000f,0.9771f},{0.3486f,0.9016f},c});
    v.push_back({{-0.15802f,-0.08007f,-0.09514f},{0.2130f,0.0000f,0.9771f},{0.2889f,0.8860f},c});
    v.push_back({{-0.19382f,-0.01741f,-0.15960f},{-0.1590f,-0.9872f,-0.0076f},{0.7277f,0.6750f},c});
    v.push_back({{-0.12889f,-0.02838f,-0.09426f},{-0.1590f,-0.9872f,-0.0076f},{0.6265f,0.5821f},c});
    v.push_back({{-0.19733f,-0.01741f,-0.08657f},{-0.1590f,-0.9872f,-0.0076f},{0.7277f,0.5657f},c});
    v.push_back({{-0.19733f,-0.09052f,-0.08657f},{-0.1590f,0.9872f,-0.0076f},{0.1016f,0.6837f},c});
    v.push_back({{-0.12643f,-0.07955f,-0.14538f},{-0.1590f,0.9872f,-0.0076f},{0.0000f,0.5907f},c});
    v.push_back({{-0.19382f,-0.09052f,-0.15960f},{-0.1590f,0.9872f,-0.0076f},{0.1016f,0.5743f},c});
    v.push_back({{-0.19733f,-0.01741f,-0.08657f},{-0.1117f,0.0000f,-0.9937f},{0.7324f,0.4106f},c});
    v.push_back({{-0.12889f,-0.07955f,-0.09426f},{-0.1117f,0.0000f,-0.9937f},{0.6321f,0.3177f},c});
    v.push_back({{-0.19733f,-0.09052f,-0.08657f},{-0.1117f,0.0000f,-0.9937f},{0.7324f,0.3013f},c});
    v.push_back({{-0.19382f,-0.09052f,-0.15960f},{-0.2064f,0.0000f,0.9785f},{0.7375f,0.5501f},c});
    v.push_back({{-0.12643f,-0.02838f,-0.14538f},{-0.2064f,0.0000f,0.9785f},{0.6387f,0.4572f},c});
    v.push_back({{-0.19382f,-0.01741f,-0.15960f},{-0.2064f,0.0000f,0.9785f},{0.7375f,0.4408f},c});
    v.push_back({{-0.12889f,-0.02838f,-0.09426f},{0.3224f,0.0000f,0.9466f},{0.7253f,0.7636f},c});
    v.push_back({{-0.16843f,-0.09491f,-0.08079f},{0.3224f,0.0000f,0.9466f},{0.7858f,0.8631f},c});
    v.push_back({{-0.12889f,-0.07955f,-0.09426f},{0.3224f,0.0000f,0.9466f},{0.7253f,0.8402f},c});
    v.push_back({{-0.12643f,-0.07955f,-0.14538f},{0.4117f,0.0000f,-0.9113f},{0.2350f,0.8152f},c});
    v.push_back({{-0.16451f,-0.01302f,-0.16258f},{0.4117f,0.0000f,-0.9113f},{0.2889f,0.9147f},c});
    v.push_back({{-0.12643f,-0.02838f,-0.14538f},{0.4117f,0.0000f,-0.9113f},{0.2350f,0.8918f},c});
    v.push_back({{-0.12643f,-0.02838f,-0.14538f},{0.3671f,0.9300f,0.0176f},{0.8468f,0.2591f},c});
    v.push_back({{-0.16843f,-0.01302f,-0.08079f},{0.3671f,0.9300f,0.0176f},{0.9050f,0.3586f},c});
    v.push_back({{-0.12889f,-0.02838f,-0.09426f},{0.3671f,0.9300f,0.0176f},{0.8468f,0.3357f},c});
    v.push_back({{-0.12889f,-0.07955f,-0.09426f},{0.3671f,-0.9300f,0.0176f},{0.8468f,0.7448f},c});
    v.push_back({{-0.16451f,-0.09491f,-0.16258f},{0.3671f,-0.9300f,0.0176f},{0.9054f,0.8443f},c});
    v.push_back({{-0.12643f,-0.07955f,-0.14538f},{0.3671f,-0.9300f,0.0176f},{0.8468f,0.8214f},c});
    v.push_back({{-0.16843f,-0.01302f,-0.08079f},{-0.1869f,-0.9822f,0.0161f},{0.6455f,0.0426f},c});
    v.push_back({{-0.08767f,-0.02940f,-0.14249f},{-0.1869f,-0.9822f,0.0161f},{0.5300f,0.1406f},c});
    v.push_back({{-0.08852f,-0.02842f,-0.09237f},{-0.1869f,-0.9822f,0.0161f},{0.5300f,0.0671f},c});
    v.push_back({{-0.16843f,-0.09491f,-0.08079f},{-0.2060f,0.9785f,-0.0099f},{0.1161f,0.5743f},c});
    v.push_back({{-0.08767f,-0.07853f,-0.14249f},{-0.2060f,0.9785f,-0.0099f},{-0.0000f,0.4764f},c});
    v.push_back({{-0.16451f,-0.09491f,-0.16258f},{-0.2060f,0.9785f,-0.0099f},{0.1161f,0.4519f},c});
    v.push_back({{-0.16843f,-0.01302f,-0.08079f},{-0.1590f,0.0000f,-0.9873f},{0.2332f,0.5634f},c});
    v.push_back({{-0.09003f,-0.07853f,-0.09342f},{-0.1590f,0.0000f,-0.9873f},{0.1163f,0.4654f},c});
    v.push_back({{-0.16843f,-0.09491f,-0.08079f},{-0.1590f,0.0000f,-0.9873f},{0.2332f,0.4409f},c});
    v.push_back({{-0.16451f,-0.09491f,-0.16258f},{-0.2529f,0.0000f,0.9675f},{0.6421f,0.2875f},c});
    v.push_back({{-0.08767f,-0.02940f,-0.14249f},{-0.2529f,0.0000f,0.9675f},{0.5300f,0.1895f},c});
    v.push_back({{-0.16451f,-0.01302f,-0.16258f},{-0.2529f,0.0000f,0.9675f},{0.6421f,0.1650f},c});
    v.push_back({{-0.08767f,-0.07853f,-0.14249f},{0.4450f,-0.8953f,0.0214f},{0.8302f,0.1418f},c});
    v.push_back({{-0.13014f,-0.09806f,-0.07629f},{0.4450f,-0.8953f,0.0214f},{0.8889f,0.0426f},c});
    v.push_back({{-0.12593f,-0.09806f,-0.16388f},{0.4450f,-0.8953f,0.0214f},{0.8889f,0.1675f},c});
    v.push_back({{-0.09003f,-0.07853f,-0.09342f},{0.3928f,0.0000f,0.9196f},{0.7858f,0.8211f},c});
    v.push_back({{-0.13014f,-0.01036f,-0.07629f},{0.3928f,0.0000f,0.9196f},{0.8468f,0.7219f},c});
    v.push_back({{-0.13014f,-0.09806f,-0.07629f},{0.3928f,0.0000f,0.9196f},{0.8468f,0.8468f},c});
    v.push_back({{-0.08767f,-0.07853f,-0.14249f},{0.4880f,0.0000f,-0.8729f},{0.5312f,0.8180f},c});
    v.push_back({{-0.12593f,-0.01036f,-0.16388f},{0.4880f,0.0000f,-0.8729f},{0.5873f,0.9172f},c});
    v.push_back({{-0.08767f,-0.02940f,-0.14249f},{0.4880f,0.0000f,-0.8729f},{0.5312f,0.8914f},c});
    v.push_back({{-0.08767f,-0.02940f,-0.14249f},{0.3943f,0.9189f,-0.0113f},{0.8429f,0.5914f},c});
    v.push_back({{-0.13014f,-0.01036f,-0.07629f},{0.3943f,0.9189f,-0.0113f},{0.9012f,0.6906f},c});
    v.push_back({{-0.08852f,-0.02842f,-0.09237f},{0.3943f,0.9189f,-0.0113f},{0.8429f,0.6649f},c});
    v.push_back({{-0.13014f,-0.01036f,-0.07629f},{0.0511f,0.0000f,-0.9987f},{0.6387f,0.5657f},c});
    v.push_back({{-0.05214f,-0.09781f,-0.07230f},{0.0511f,0.0000f,-0.9987f},{0.5312f,0.4408f},c});
    v.push_back({{-0.13014f,-0.09806f,-0.07629f},{0.0511f,0.0000f,-0.9987f},{0.6387f,0.4408f},c});
    v.push_back({{-0.12593f,-0.01036f,-0.16388f},{-0.0511f,-0.0000f,0.9987f},{0.3503f,0.4408f},c});
    v.push_back({{-0.04794f,-0.09781f,-0.15989f},{-0.0511f,-0.0000f,0.9987f},{0.2350f,0.5657f},c});
    v.push_back({{-0.04794f,-0.01012f,-0.15989f},{-0.0511f,-0.0000f,0.9987f},{0.2350f,0.4408f},c});
    v.push_back({{-0.13014f,-0.01036f,-0.07629f},{0.0031f,-1.0000f,0.0002f},{0.4687f,0.4379f},c});
    v.push_back({{-0.04794f,-0.01012f,-0.15989f},{0.0031f,-1.0000f,0.0002f},{0.3525f,0.5628f},c});
    v.push_back({{-0.05214f,-0.01012f,-0.07230f},{0.0031f,-1.0000f,0.0002f},{0.3525f,0.4379f},c});
    v.push_back({{-0.13014f,-0.09806f,-0.07629f},{-0.0031f,1.0000f,-0.0002f},{0.4699f,0.2811f},c});
    v.push_back({{-0.04794f,-0.09781f,-0.15989f},{-0.0031f,1.0000f,-0.0002f},{0.3537f,0.1562f},c});
    v.push_back({{-0.12593f,-0.09806f,-0.16388f},{-0.0031f,1.0000f,-0.0002f},{0.4699f,0.1562f},c});
    v.push_back({{-0.04794f,-0.01012f,-0.15989f},{-0.0612f,0.9981f,-0.0029f},{0.3525f,0.5628f},c});
    v.push_back({{-0.11986f,-0.01429f,-0.07973f},{-0.0612f,0.9981f,-0.0029f},{0.4542f,0.6846f},c});
    v.push_back({{-0.05214f,-0.01012f,-0.07230f},{-0.0612f,0.9981f,-0.0029f},{0.3525f,0.6878f},c});
    v.push_back({{-0.04794f,-0.09781f,-0.15989f},{-0.0612f,-0.9981f,-0.0029f},{0.1163f,0.6883f},c});
    v.push_back({{-0.11986f,-0.09364f,-0.07973f},{-0.0612f,-0.9981f,-0.0029f},{0.2179f,0.5665f},c});
    v.push_back({{-0.11605f,-0.09364f,-0.15898f},{-0.0612f,-0.9981f,-0.0029f},{0.2179f,0.6852f},c});
    v.push_back({{-0.05214f,-0.01012f,-0.07230f},{-0.1091f,0.0000f,0.9940f},{0.2350f,0.5657f},c});
    v.push_back({{-0.11986f,-0.09364f,-0.07973f},{-0.1091f,0.0000f,0.9940f},{0.3355f,0.6875f},c});
    v.push_back({{-0.05214f,-0.09781f,-0.07230f},{-0.1091f,0.0000f,0.9940f},{0.2350f,0.6906f},c});
    v.push_back({{-0.04794f,-0.09781f,-0.15989f},{-0.0133f,0.0000f,-0.9999f},{0.5312f,0.5657f},c});
    v.push_back({{-0.11605f,-0.01429f,-0.15898f},{-0.0133f,0.0000f,-0.9999f},{0.6265f,0.6875f},c});
    v.push_back({{-0.04794f,-0.01012f,-0.15989f},{-0.0133f,0.0000f,-0.9999f},{0.5312f,0.6906f},c});
    v.push_back({{-0.11986f,-0.01429f,-0.07973f},{0.0072f,0.0000f,-1.0000f},{0.3525f,0.4379f},c});
    v.push_back({{-0.02265f,-0.08967f,-0.07903f},{0.0072f,0.0000f,-1.0000f},{0.4652f,0.3013f},c});
    v.push_back({{-0.11986f,-0.09364f,-0.07973f},{0.0072f,0.0000f,-1.0000f},{0.4712f,0.4379f},c});
    v.push_back({{-0.11605f,-0.09364f,-0.15898f},{-0.0887f,0.0000f,0.9961f},{0.2359f,0.1433f},c});
    v.push_back({{-0.01922f,-0.01826f,-0.15036f},{-0.0887f,0.0000f,0.9961f},{0.3487f,0.0000f},c});
    v.push_back({{-0.11605f,-0.01429f,-0.15898f},{-0.0887f,0.0000f,0.9961f},{0.3546f,0.1433f},c});
    v.push_back({{-0.11605f,-0.01429f,-0.15898f},{-0.0408f,-0.9992f,-0.0020f},{0.2350f,0.3013f},c});
    v.push_back({{-0.02265f,-0.01826f,-0.07903f},{-0.0408f,-0.9992f,-0.0020f},{0.3477f,0.1562f},c});
    v.push_back({{-0.11986f,-0.01429f,-0.07973f},{-0.0408f,-0.9992f,-0.0020f},{0.3537f,0.3013f},c});
    v.push_back({{-0.11986f,-0.09364f,-0.07973f},{-0.0408f,0.9992f,-0.0020f},{0.1163f,0.3015f},c});
    v.push_back({{-0.01922f,-0.08967f,-0.15036f},{-0.0408f,0.9992f,-0.0020f},{0.2291f,0.1562f},c});
    v.push_back({{-0.11605f,-0.09364f,-0.15898f},{-0.0408f,0.9992f,-0.0020f},{0.2350f,0.3015f},c});
    v.push_back({{-0.02265f,-0.01826f,-0.07903f},{-0.5627f,0.0000f,0.8267f},{0.9476f,0.0404f},c});
    v.push_back({{-0.03395f,-0.08253f,-0.08672f},{-0.5627f,0.0000f,0.8267f},{0.9674f,0.1365f},c});
    v.push_back({{-0.02265f,-0.08967f,-0.07903f},{-0.5627f,0.0000f,0.8267f},{0.9476f,0.1472f},c});
    v.push_back({{-0.01922f,-0.08967f,-0.15036f},{-0.4809f,0.0000f,-0.8768f},{0.4222f,0.6878f},c});
    v.push_back({{-0.03121f,-0.02540f,-0.14379f},{-0.4809f,0.0000f,-0.8768f},{0.4424f,0.7839f},c});
    v.push_back({{-0.01922f,-0.01826f,-0.15036f},{-0.4809f,0.0000f,-0.8768f},{0.4222f,0.7946f},c});
    v.push_back({{-0.02265f,-0.01826f,-0.07903f},{-0.5218f,0.8527f,-0.0251f},{0.4510f,0.1068f},c});
    v.push_back({{-0.03121f,-0.02540f,-0.14379f},{-0.5218f,0.8527f,-0.0251f},{0.4684f,0.0107f},c});
    v.push_back({{-0.03395f,-0.02540f,-0.08672f},{-0.5218f,0.8527f,-0.0251f},{0.4684f,0.0961f},c});
    v.push_back({{-0.02265f,-0.08967f,-0.07903f},{-0.5218f,-0.8527f,-0.0251f},{0.4253f,0.7946f},c});
    v.push_back({{-0.03121f,-0.08253f,-0.14379f},{-0.5218f,-0.8527f,-0.0251f},{0.4424f,0.8907f},c});
    v.push_back({{-0.01922f,-0.08967f,-0.15036f},{-0.5218f,-0.8527f,-0.0251f},{0.4253f,0.9014f},c});
    v.push_back({{-0.03395f,-0.02540f,-0.08672f},{-0.0586f,-0.9983f,-0.0028f},{0.1888f,0.8385f},c});
    v.push_back({{0.01717f,-0.02826f,-0.13860f},{-0.0586f,-0.9983f,-0.0028f},{0.1163f,0.9197f},c});
    v.push_back({{0.01470f,-0.02826f,-0.08725f},{-0.0586f,-0.9983f,-0.0028f},{0.1163f,0.8428f},c});
    v.push_back({{-0.03121f,-0.08253f,-0.14379f},{-0.0586f,0.9983f,-0.0028f},{0.4251f,0.8319f},c});
    v.push_back({{0.01470f,-0.07967f,-0.08725f},{-0.0586f,0.9983f,-0.0028f},{0.3525f,0.9130f},c});
    v.push_back({{0.01717f,-0.07967f,-0.13860f},{-0.0586f,0.9983f,-0.0028f},{0.3525f,0.8361f},c});
    v.push_back({{-0.03395f,-0.02540f,-0.08672f},{-0.0107f,0.0000f,-0.9999f},{0.7941f,0.9486f},c});
    v.push_back({{0.01470f,-0.07967f,-0.08725f},{-0.0107f,0.0000f,-0.9999f},{0.7253f,0.8674f},c});
    v.push_back({{-0.03395f,-0.08253f,-0.08672f},{-0.0107f,0.0000f,-0.9999f},{0.7941f,0.8631f},c});
    v.push_back({{-0.03121f,-0.08253f,-0.14379f},{-0.1065f,0.0000f,0.9943f},{0.7136f,0.2875f},c});
    v.push_back({{0.01717f,-0.02826f,-0.13860f},{-0.1065f,0.0000f,0.9943f},{0.6421f,0.2063f},c});
    v.push_back({{-0.03121f,-0.02540f,-0.14379f},{-0.1065f,0.0000f,0.9943f},{0.7136f,0.2021f},c});
    v.push_back({{0.01470f,-0.02826f,-0.08725f},{-0.2555f,0.9667f,-0.0123f},{0.2028f,0.9047f},c});
    v.push_back({{-0.00248f,-0.03340f,-0.13440f},{-0.2555f,0.9667f,-0.0123f},{0.2318f,0.8355f},c});
    v.push_back({{-0.00446f,-0.03340f,-0.09331f},{-0.2555f,0.9667f,-0.0123f},{0.2318f,0.8970f},c});
    v.push_back({{0.01470f,-0.07967f,-0.08725f},{-0.2555f,-0.9667f,-0.0123f},{0.6962f,0.8519f},c});
    v.push_back({{-0.00248f,-0.07453f,-0.13440f},{-0.2555f,-0.9667f,-0.0123f},{0.7251f,0.9211f},c});
    v.push_back({{0.01717f,-0.07967f,-0.13860f},{-0.2555f,-0.9667f,-0.0123f},{0.6962f,0.9288f},c});
    v.push_back({{0.01470f,-0.07967f,-0.08725f},{-0.3019f,0.0000f,0.9533f},{0.5030f,0.8968f},c});
    v.push_back({{-0.00446f,-0.03340f,-0.09331f},{-0.3019f,0.0000f,0.9533f},{0.5308f,0.8276f},c});
    v.push_back({{-0.00446f,-0.07453f,-0.09331f},{-0.3019f,0.0000f,0.9533f},{0.5308f,0.8891f},c});
    v.push_back({{0.01717f,-0.07967f,-0.13860f},{-0.2092f,0.0000f,-0.9779f},{0.3131f,0.6906f},c});
    v.push_back({{-0.00248f,-0.03340f,-0.13440f},{-0.2092f,0.0000f,-0.9779f},{0.3429f,0.7598f},c});
    v.push_back({{0.01717f,-0.02826f,-0.13860f},{-0.2092f,0.0000f,-0.9779f},{0.3131f,0.7675f},c});
    v.push_back({{-0.00446f,-0.03340f,-0.09331f},{0.1088f,0.0000f,-0.9941f},{0.3031f,0.9809f},c});
    v.push_back({{0.04585f,-0.07762f,-0.08781f},{0.1088f,0.0000f,-0.9941f},{0.2350f,0.9147f},c});
    v.push_back({{-0.00446f,-0.07453f,-0.09331f},{0.1088f,0.0000f,-0.9941f},{0.3031f,0.9194f},c});
    v.push_back({{-0.00248f,-0.07453f,-0.13440f},{0.0130f,0.0000f,0.9999f},{0.9100f,0.7974f},c});
    v.push_back({{0.04812f,-0.03031f,-0.13506f},{0.0130f,0.0000f,0.9999f},{0.9762f,0.7219f},c});
    v.push_back({{-0.00248f,-0.03340f,-0.13440f},{0.0130f,0.0000f,0.9999f},{0.9716f,0.7974f},c});
    v.push_back({{-0.00248f,-0.03340f,-0.13440f},{0.0609f,-0.9981f,0.0029f},{0.8935f,0.2228f},c});
    v.push_back({{0.04585f,-0.03031f,-0.08781f},{0.0609f,-0.9981f,0.0029f},{0.9597f,0.1472f},c});
    v.push_back({{-0.00446f,-0.03340f,-0.09331f},{0.0609f,-0.9981f,0.0029f},{0.9551f,0.2228f},c});
    v.push_back({{-0.00248f,-0.07453f,-0.13440f},{0.0609f,0.9981f,0.0029f},{0.9711f,0.4094f},c});
    v.push_back({{0.04585f,-0.07762f,-0.08781f},{0.0609f,0.9981f,0.0029f},{0.9050f,0.3338f},c});
    v.push_back({{0.04812f,-0.07762f,-0.13506f},{0.0609f,0.9981f,0.0029f},{0.9757f,0.3338f},c});
    v.push_back({{0.04585f,-0.03031f,-0.08781f},{-0.3575f,0.0000f,0.9339f},{0.4445f,0.9293f},c});
    v.push_back({{0.00341f,-0.06342f,-0.10405f},{-0.3575f,0.0000f,0.9339f},{0.5058f,0.9788f},c});
    v.push_back({{0.04585f,-0.07762f,-0.08781f},{-0.3575f,0.0000f,0.9339f},{0.4445f,1.0000f},c});
    v.push_back({{0.04812f,-0.07762f,-0.13506f},{-0.2663f,0.0000f,-0.9639f},{0.5195f,0.9172f},c});
    v.push_back({{0.00432f,-0.04450f,-0.12295f},{-0.2663f,0.0000f,-0.9639f},{0.5873f,0.9667f},c});
    v.push_back({{0.04812f,-0.03031f,-0.13506f},{-0.2663f,0.0000f,-0.9639f},{0.5195f,0.9879f},c});
    v.push_back({{0.04585f,-0.03031f,-0.08781f},{-0.3119f,0.9500f,-0.0150f},{0.3525f,0.9880f},c});
    v.push_back({{0.00432f,-0.04450f,-0.12295f},{-0.3119f,0.9500f,-0.0150f},{0.4170f,0.9385f},c});
    v.push_back({{0.00341f,-0.04450f,-0.10405f},{-0.3119f,0.9500f,-0.0150f},{0.4170f,0.9668f},c});
    v.push_back({{0.04812f,-0.07762f,-0.13506f},{-0.3119f,-0.9500f,-0.0150f},{0.1163f,0.9947f},c});
    v.push_back({{0.00341f,-0.06342f,-0.10405f},{-0.3119f,-0.9500f,-0.0150f},{0.1804f,0.9452f},c});
    v.push_back({{0.00432f,-0.06342f,-0.12295f},{-0.3119f,-0.9500f,-0.0150f},{0.1804f,0.9735f},c});
    v.push_back({{0.00341f,-0.04450f,-0.10405f},{-0.0000f,-1.0000f,-0.0000f},{0.5312f,0.5228f},c});
    v.push_back({{0.02823f,-0.04450f,-0.11901f},{-0.0000f,-1.0000f,-0.0000f},{0.4903f,0.5085f},c});
    v.push_back({{0.02405f,-0.04450f,-0.10055f},{-0.0000f,-1.0000f,-0.0000f},{0.5156f,0.4957f},c});
    v.push_back({{0.00341f,-0.06342f,-0.10405f},{0.0000f,1.0000f,-0.0000f},{0.4699f,0.2641f},c});
    v.push_back({{0.02823f,-0.06342f,-0.11901f},{0.0000f,1.0000f,-0.0000f},{0.5108f,0.2498f},c});
    v.push_back({{0.00432f,-0.06342f,-0.12295f},{0.0000f,1.0000f,-0.0000f},{0.4925f,0.2811f},c});
    v.push_back({{0.00341f,-0.04450f,-0.10405f},{0.1672f,0.0000f,-0.9859f},{0.4684f,0.0000f},c});
    v.push_back({{0.02405f,-0.06342f,-0.10055f},{0.1672f,0.0000f,-0.9859f},{0.4957f,0.0283f},c});
    v.push_back({{0.00341f,-0.06342f,-0.10405f},{0.1672f,0.0000f,-0.9859f},{0.4684f,0.0283f},c});
    v.push_back({{0.00432f,-0.06342f,-0.12295f},{-0.1628f,0.0000f,0.9867f},{0.8302f,0.2358f},c});
    v.push_back({{0.02823f,-0.04450f,-0.11901f},{-0.1628f,0.0000f,0.9867f},{0.8585f,0.2006f},c});
    v.push_back({{0.00432f,-0.04450f,-0.12295f},{-0.1628f,0.0000f,0.9867f},{0.8585f,0.2358f},c});
    v.push_back({{0.02405f,-0.06342f,-0.10055f},{0.0000f,1.0000f,0.0000f},{0.4856f,0.2370f},c});
    v.push_back({{0.05109f,-0.06342f,-0.11097f},{0.0000f,1.0000f,0.0000f},{0.5233f,0.2158f},c});
    v.push_back({{0.02405f,-0.04450f,-0.10055f},{0.3359f,0.0000f,-0.9419f},{0.6962f,0.8519f},c});
    v.push_back({{0.04377f,-0.06342f,-0.09352f},{0.3359f,0.0000f,-0.9419f},{0.7245f,0.8263f},c});
    v.push_back({{0.02405f,-0.06342f,-0.10055f},{0.3359f,0.0000f,-0.9419f},{0.7245f,0.8519f},c});
    v.push_back({{0.02823f,-0.06342f,-0.11901f},{-0.3317f,0.0000f,0.9434f},{0.8302f,0.2006f},c});
    v.push_back({{0.05109f,-0.04450f,-0.11097f},{-0.3317f,0.0000f,0.9434f},{0.8585f,0.1675f},c});
    v.push_back({{0.02823f,-0.04450f,-0.11901f},{-0.3317f,0.0000f,0.9434f},{0.8585f,0.2006f},c});
    v.push_back({{0.05109f,-0.04450f,-0.11097f},{-0.0000f,-1.0000f,-0.0000f},{0.4778f,0.4745f},c});
    v.push_back({{0.04377f,-0.04450f,-0.09352f},{-0.0000f,-1.0000f,-0.0000f},{0.5049f,0.4663f},c});
    v.push_back({{0.04377f,-0.04450f,-0.09352f},{0.4943f,0.0000f,-0.8693f},{0.6962f,0.8263f},c});
    v.push_back({{0.06197f,-0.06342f,-0.08317f},{0.4943f,0.0000f,-0.8693f},{0.7245f,0.7979f},c});
    v.push_back({{0.04377f,-0.06342f,-0.09352f},{0.4943f,0.0000f,-0.8693f},{0.7245f,0.8263f},c});
    v.push_back({{0.05109f,-0.06342f,-0.11097f},{-0.4905f,0.0000f,0.8715f},{0.5873f,0.9680f},c});
    v.push_back({{0.07221f,-0.04450f,-0.09908f},{-0.4905f,0.0000f,0.8715f},{0.6156f,0.9338f},c});
    v.push_back({{0.05109f,-0.04450f,-0.11097f},{-0.4905f,0.0000f,0.8715f},{0.6156f,0.9680f},c});
    v.push_back({{0.07221f,-0.04450f,-0.09908f},{0.0000f,-1.0000f,-0.0000f},{0.4713f,0.4388f},c});
    v.push_back({{0.06197f,-0.04450f,-0.08317f},{0.0000f,-1.0000f,-0.0000f},{0.4994f,0.4354f},c});
    v.push_back({{0.04377f,-0.06342f,-0.09352f},{-0.0000f,1.0000f,-0.0000f},{0.4963f,0.2076f},c});
    v.push_back({{0.07221f,-0.06342f,-0.09908f},{-0.0000f,1.0000f,-0.0000f},{0.5298f,0.1801f},c});
    v.push_back({{0.09094f,-0.04450f,-0.08371f},{-0.0000f,-1.0000f,-0.0000f},{0.4712f,0.4026f},c});
    v.push_back({{0.07810f,-0.04450f,-0.06982f},{-0.0000f,-1.0000f,-0.0000f},{0.4994f,0.4041f},c});
    v.push_back({{0.06197f,-0.06342f,-0.08317f},{0.0000f,1.0000f,0.0000f},{0.5017f,0.1767f},c});
    v.push_back({{0.09094f,-0.06342f,-0.08371f},{0.0000f,1.0000f,0.0000f},{0.5300f,0.1439f},c});
    v.push_back({{0.06197f,-0.04450f,-0.08317f},{0.6378f,0.0000f,-0.7702f},{0.6962f,0.7979f},c});
    v.push_back({{0.07810f,-0.06342f,-0.06982f},{0.6378f,0.0000f,-0.7702f},{0.7245f,0.7676f},c});
    v.push_back({{0.06197f,-0.06342f,-0.08317f},{0.6378f,0.0000f,-0.7702f},{0.7245f,0.7979f},c});
    v.push_back({{0.07221f,-0.06342f,-0.09908f},{-0.6343f,0.0000f,0.7731f},{0.5873f,0.9338f},c});
    v.push_back({{0.09094f,-0.04450f,-0.08371f},{-0.6343f,0.0000f,0.7731f},{0.6156f,0.8981f},c});
    v.push_back({{0.07221f,-0.04450f,-0.09908f},{-0.6343f,0.0000f,0.7731f},{0.6156f,0.9338f},c});
    v.push_back({{0.07810f,-0.04450f,-0.06982f},{0.7618f,0.0000f,-0.6478f},{0.6962f,0.7676f},c});
    v.push_back({{0.09166f,-0.06342f,-0.05386f},{0.7618f,0.0000f,-0.6478f},{0.7245f,0.7365f},c});
    v.push_back({{0.07810f,-0.06342f,-0.06982f},{0.7618f,0.0000f,-0.6478f},{0.7245f,0.7676f},c});
    v.push_back({{0.09094f,-0.06342f,-0.08371f},{-0.7589f,0.0000f,0.6512f},{0.5873f,0.8981f},c});
    v.push_back({{0.10672f,-0.04450f,-0.06532f},{-0.7589f,0.0000f,0.6512f},{0.6156f,0.8618f},c});
    v.push_back({{0.09094f,-0.04450f,-0.08371f},{-0.7589f,0.0000f,0.6512f},{0.6156f,0.8981f},c});
    v.push_back({{0.10672f,-0.04450f,-0.06532f},{0.0000f,-1.0000f,-0.0000f},{0.4773f,0.3669f},c});
    v.push_back({{0.09166f,-0.04450f,-0.05386f},{0.0000f,-1.0000f,-0.0000f},{0.5049f,0.3733f},c});
    v.push_back({{0.07810f,-0.06342f,-0.06982f},{0.0000f,1.0000f,-0.0000f},{0.5017f,0.1454f},c});
    v.push_back({{0.10672f,-0.06342f,-0.06532f},{0.0000f,1.0000f,-0.0000f},{0.5238f,0.1082f},c});
    v.push_back({{0.09166f,-0.04450f,-0.05386f},{0.8627f,0.0000f,-0.5056f},{0.6962f,0.7365f},c});
    v.push_back({{0.10225f,-0.06342f,-0.03580f},{0.8627f,0.0000f,-0.5056f},{0.7245f,0.7053f},c});
    v.push_back({{0.09166f,-0.06342f,-0.05386f},{0.8627f,0.0000f,-0.5056f},{0.7245f,0.7365f},c});
    v.push_back({{0.10672f,-0.06342f,-0.06532f},{-0.8605f,0.0000f,0.5095f},{0.5873f,0.8618f},c});
    v.push_back({{0.11907f,-0.04450f,-0.04446f},{-0.8605f,0.0000f,0.5095f},{0.6156f,0.8262f},c});
    v.push_back({{0.10672f,-0.04450f,-0.06532f},{-0.8605f,0.0000f,0.5095f},{0.6156f,0.8618f},c});
    v.push_back({{0.11907f,-0.04450f,-0.04446f},{0.0000f,-1.0000f,-0.0000f},{0.4896f,0.3328f},c});
    v.push_back({{0.10225f,-0.04450f,-0.03580f},{0.0000f,-1.0000f,-0.0000f},{0.5156f,0.3439f},c});
    v.push_back({{0.09166f,-0.06342f,-0.05386f},{-0.0000f,1.0000f,0.0000f},{0.4963f,0.1146f},c});
    v.push_back({{0.11907f,-0.06342f,-0.04446f},{-0.0000f,1.0000f,0.0000f},{0.5116f,0.0741f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{0.0000f,-1.0000f,-0.0000f},{0.5075f,0.3013f},c});
    v.push_back({{0.10954f,-0.04450f,-0.01617f},{0.0000f,-1.0000f,-0.0000f},{0.5312f,0.3167f},c});
    v.push_back({{0.10225f,-0.06342f,-0.03580f},{-0.0000f,1.0000f,0.0000f},{0.4856f,0.0852f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{-0.0000f,1.0000f,0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.10225f,-0.04450f,-0.03580f},{0.9374f,0.0000f,-0.3481f},{0.6962f,0.7053f},c});
    v.push_back({{0.10954f,-0.06342f,-0.01617f},{0.9374f,0.0000f,-0.3481f},{0.7245f,0.6750f},c});
    v.push_back({{0.10225f,-0.06342f,-0.03580f},{0.9374f,0.0000f,-0.3481f},{0.7245f,0.7053f},c});
    v.push_back({{0.11907f,-0.06342f,-0.04446f},{-0.9359f,0.0000f,0.3523f},{0.5873f,0.8262f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{-0.9359f,0.0000f,0.3523f},{0.6156f,0.7922f},c});
    v.push_back({{0.11907f,-0.04450f,-0.04446f},{-0.9359f,0.0000f,0.3523f},{0.6156f,0.8262f},c});
    v.push_back({{-0.33040f,-0.03454f,0.11011f},{0.3710f,0.9285f,-0.0178f},{0.8815f,0.7314f},c});
    v.push_back({{-0.36660f,-0.01900f,0.16631f},{0.3710f,0.9285f,-0.0178f},{0.9397f,0.8130f},c});
    v.push_back({{-0.32854f,-0.03454f,0.14892f},{0.3710f,0.9285f,-0.0178f},{0.8815f,0.7897f},c});
    v.push_back({{-0.32854f,-0.03454f,0.14892f},{0.4155f,0.0000f,0.9096f},{0.4689f,0.8541f},c});
    v.push_back({{-0.36660f,-0.08893f,0.16631f},{0.4155f,0.0000f,0.9096f},{0.5284f,0.9358f},c});
    v.push_back({{-0.32854f,-0.07339f,0.14892f},{0.4155f,0.0000f,0.9096f},{0.4689f,0.9124f},c});
    v.push_back({{-0.33040f,-0.07339f,0.11011f},{0.3710f,-0.9285f,-0.0178f},{0.8784f,0.3785f},c});
    v.push_back({{-0.36660f,-0.08893f,0.16631f},{0.3710f,-0.9285f,-0.0178f},{0.9376f,0.2968f},c});
    v.push_back({{-0.36996f,-0.08893f,0.09645f},{0.3710f,-0.9285f,-0.0178f},{0.9376f,0.4018f},c});
    v.push_back({{-0.33040f,-0.07339f,0.11011f},{0.3264f,0.0000f,-0.9452f},{0.8795f,0.4455f},c});
    v.push_back({{-0.36996f,-0.01900f,0.09645f},{0.3264f,0.0000f,-0.9452f},{0.9380f,0.5272f},c});
    v.push_back({{-0.33040f,-0.03454f,0.11011f},{0.3264f,0.0000f,-0.9452f},{0.8795f,0.5039f},c});
    v.push_back({{-0.44590f,-0.05396f,0.13511f},{0.1642f,-0.9864f,-0.0079f},{0.7155f,0.7382f},c});
    v.push_back({{-0.33040f,-0.03454f,0.11011f},{0.1642f,-0.9864f,-0.0079f},{0.6863f,0.5622f},c});
    v.push_back({{-0.32854f,-0.03454f,0.14892f},{0.1642f,-0.9864f,-0.0079f},{0.7447f,0.5622f},c});
    v.push_back({{-0.33040f,-0.03454f,0.11011f},{0.2115f,0.0000f,0.9774f},{0.7036f,0.2968f},c});
    v.push_back({{-0.44590f,-0.05396f,0.13511f},{0.2115f,0.0000f,0.9774f},{0.7036f,0.4755f},c});
    v.push_back({{-0.33040f,-0.07339f,0.11011f},{0.2115f,0.0000f,0.9774f},{0.6461f,0.3063f},c});
    v.push_back({{-0.44590f,-0.05396f,0.13511f},{0.1642f,0.9864f,-0.0079f},{0.7911f,0.4715f},c});
    v.push_back({{-0.32854f,-0.07339f,0.14892f},{0.1642f,0.9864f,-0.0079f},{0.7619f,0.2968f},c});
    v.push_back({{-0.33040f,-0.07339f,0.11011f},{0.1642f,0.9864f,-0.0079f},{0.8203f,0.2968f},c});
    v.push_back({{-0.44590f,-0.05396f,0.13511f},{0.1169f,-0.0000f,-0.9931f},{0.7328f,0.4720f},c});
    v.push_back({{-0.32854f,-0.03454f,0.14892f},{0.1169f,-0.0000f,-0.9931f},{0.7036f,0.2968f},c});
    v.push_back({{-0.32854f,-0.07339f,0.14892f},{0.1169f,-0.0000f,-0.9931f},{0.7619f,0.2968f},c});
    v.push_back({{-0.36996f,-0.08893f,0.09645f},{0.1956f,0.0000f,0.9807f},{0.4698f,0.2793f},c});
    v.push_back({{-0.29290f,-0.00734f,0.08108f},{0.1956f,0.0000f,0.9807f},{0.3524f,0.1568f},c});
    v.push_back({{-0.36996f,-0.01900f,0.09645f},{0.1956f,0.0000f,0.9807f},{0.4698f,0.1743f},c});
    v.push_back({{-0.36660f,-0.08893f,0.16631f},{0.1007f,-0.0000f,-0.9949f},{0.4718f,0.0175f},c});
    v.push_back({{-0.28842f,-0.00734f,0.17422f},{0.1007f,-0.0000f,-0.9949f},{0.3550f,0.1400f},c});
    v.push_back({{-0.28842f,-0.10059f,0.17422f},{0.1007f,-0.0000f,-0.9949f},{0.3550f,0.0000f},c});
    v.push_back({{-0.36660f,-0.08893f,0.16631f},{0.1482f,0.9889f,-0.0071f},{0.4689f,0.5593f},c});
    v.push_back({{-0.29290f,-0.10059f,0.08108f},{0.1482f,0.9889f,-0.0071f},{0.3524f,0.4368f},c});
    v.push_back({{-0.36996f,-0.08893f,0.09645f},{0.1482f,0.9889f,-0.0071f},{0.4689f,0.4543f},c});
    v.push_back({{-0.36996f,-0.01900f,0.09645f},{0.1482f,-0.9889f,-0.0071f},{0.4696f,0.4193f},c});
    v.push_back({{-0.28842f,-0.00734f,0.17422f},{0.1482f,-0.9889f,-0.0071f},{0.3524f,0.2968f},c});
    v.push_back({{-0.36660f,-0.01900f,0.16631f},{0.1482f,-0.9889f,-0.0071f},{0.4696f,0.3143f},c});
    v.push_back({{-0.28842f,-0.10059f,0.17422f},{-0.5609f,0.0000f,0.8279f},{0.8767f,0.1445f},c});
    v.push_back({{-0.32059f,-0.03065f,0.15243f},{-0.5609f,0.0000f,0.8279f},{0.9214f,0.0394f},c});
    v.push_back({{-0.32059f,-0.07728f,0.15243f},{-0.5609f,0.0000f,0.8279f},{0.9214f,0.1095f},c});
    v.push_back({{-0.29290f,-0.10059f,0.08108f},{-0.6377f,0.0000f,-0.7703f},{0.8754f,0.1502f},c});
    v.push_back({{-0.32283f,-0.03065f,0.10586f},{-0.6377f,0.0000f,-0.7703f},{0.9219f,0.2552f},c});
    v.push_back({{-0.29290f,-0.00734f,0.08108f},{-0.6377f,0.0000f,-0.7703f},{0.8754f,0.2902f},c});
    v.push_back({{-0.28842f,-0.00734f,0.17422f},{-0.5993f,0.8000f,0.0288f},{0.8740f,0.7022f},c});
    v.push_back({{-0.32283f,-0.03065f,0.10586f},{-0.5993f,0.8000f,0.0288f},{0.9210f,0.5972f},c});
    v.push_back({{-0.32059f,-0.03065f,0.15243f},{-0.5993f,0.8000f,0.0288f},{0.9210f,0.6672f},c});
    v.push_back({{-0.28842f,-0.10059f,0.17422f},{-0.5993f,-0.8000f,0.0288f},{0.5389f,0.6851f},c});
    v.push_back({{-0.32283f,-0.07728f,0.10586f},{-0.5993f,-0.8000f,0.0288f},{0.5841f,0.7901f},c});
    v.push_back({{-0.29290f,-0.10059f,0.08108f},{-0.5993f,-0.8000f,0.0288f},{0.5389f,0.8251f},c});
    v.push_back({{-0.32059f,-0.03065f,0.15243f},{-0.0480f,0.0000f,-0.9988f},{0.7447f,0.7080f},c});
    v.push_back({{-0.22357f,-0.07728f,0.14777f},{-0.0480f,0.0000f,-0.9988f},{0.8147f,0.5622f},c});
    v.push_back({{-0.32059f,-0.07728f,0.15243f},{-0.0480f,0.0000f,-0.9988f},{0.8147f,0.7080f},c});
    v.push_back({{-0.32283f,-0.07728f,0.10586f},{0.0480f,0.0000f,0.9988f},{0.4689f,0.8308f},c});
    v.push_back({{-0.22580f,-0.03065f,0.10120f},{0.0480f,0.0000f,0.9988f},{0.5389f,0.6851f},c});
    v.push_back({{-0.32283f,-0.03065f,0.10586f},{0.0480f,0.0000f,0.9988f},{0.5389f,0.8308f},c});
    v.push_back({{-0.32283f,-0.03065f,0.10586f},{0.0000f,-1.0000f,-0.0000f},{0.7472f,0.1502f},c});
    v.push_back({{-0.22357f,-0.03065f,0.14777f},{0.0000f,-1.0000f,-0.0000f},{0.8172f,0.0044f},c});
    v.push_back({{-0.32059f,-0.03065f,0.15243f},{0.0000f,-1.0000f,-0.0000f},{0.8172f,0.1502f},c});
    v.push_back({{-0.32283f,-0.07728f,0.10586f},{-0.0000f,1.0000f,0.0000f},{0.8169f,0.2960f},c});
    v.push_back({{-0.22357f,-0.07728f,0.14777f},{-0.0000f,1.0000f,0.0000f},{0.7469f,0.1502f},c});
    v.push_back({{-0.22580f,-0.07728f,0.10120f},{-0.0000f,1.0000f,0.0000f},{0.8169f,0.1502f},c});
    v.push_back({{-0.22357f,-0.03065f,0.14777f},{0.2331f,0.9724f,-0.0112f},{0.8958f,0.9150f},c});
    v.push_back({{-0.26506f,-0.02133f,0.09375f},{0.2331f,0.9724f,-0.0112f},{0.9540f,0.8310f},c});
    v.push_back({{-0.26193f,-0.02133f,0.15895f},{0.2331f,0.9724f,-0.0112f},{0.9540f,0.9290f},c});
    v.push_back({{-0.22357f,-0.07728f,0.14777f},{0.2331f,-0.9724f,-0.0112f},{0.0000f,0.8649f},c});
    v.push_back({{-0.26506f,-0.08660f,0.09375f},{0.2331f,-0.9724f,-0.0112f},{0.0589f,0.9489f},c});
    v.push_back({{-0.22580f,-0.07728f,0.10120f},{0.2331f,-0.9724f,-0.0112f},{-0.0000f,0.9349f},c});
    v.push_back({{-0.22357f,-0.03065f,0.14777f},{0.2798f,0.0000f,0.9601f},{0.5845f,0.8584f},c});
    v.push_back({{-0.26193f,-0.08660f,0.15895f},{0.2798f,0.0000f,0.9601f},{0.6435f,0.9424f},c});
    v.push_back({{-0.22357f,-0.07728f,0.14777f},{0.2798f,0.0000f,0.9601f},{0.5845f,0.9284f},c});
    v.push_back({{-0.22580f,-0.07728f,0.10120f},{0.1865f,0.0000f,-0.9825f},{0.0589f,0.8649f},c});
    v.push_back({{-0.26506f,-0.02133f,0.09375f},{0.1865f,0.0000f,-0.9825f},{0.1173f,0.9489f},c});
    v.push_back({{-0.22580f,-0.03065f,0.10120f},{0.1865f,0.0000f,-0.9825f},{0.0589f,0.9349f},c});
    v.push_back({{-0.26506f,-0.08660f,0.09375f},{0.2441f,0.9697f,-0.0117f},{0.2332f,0.1862f},c});
    v.push_back({{-0.18337f,-0.10619f,0.17478f},{0.2441f,0.9697f,-0.0117f},{0.1168f,0.3136f},c});
    v.push_back({{-0.18838f,-0.10619f,0.07046f},{0.2441f,0.9697f,-0.0117f},{0.1168f,0.1568f},c});
    v.push_back({{-0.26193f,-0.08660f,0.15895f},{0.1976f,-0.0000f,-0.9803f},{0.1168f,0.1862f},c});
    v.push_back({{-0.18337f,-0.00174f,0.17478f},{0.1976f,-0.0000f,-0.9803f},{0.0000f,0.3136f},c});
    v.push_back({{-0.18337f,-0.10619f,0.17478f},{0.1976f,-0.0000f,-0.9803f},{0.0000f,0.1568f},c});
    v.push_back({{-0.26506f,-0.08660f,0.09375f},{0.2906f,0.0000f,0.9568f},{0.1181f,0.1274f},c});
    v.push_back({{-0.18838f,-0.00174f,0.07046f},{0.2906f,0.0000f,0.9568f},{0.0000f,0.0000f},c});
    v.push_back({{-0.26506f,-0.02133f,0.09375f},{0.2906f,0.0000f,0.9568f},{0.1181f,0.0294f},c});
    v.push_back({{-0.26506f,-0.02133f,0.09375f},{0.2441f,-0.9697f,-0.0117f},{0.2359f,0.1274f},c});
    v.push_back({{-0.18337f,-0.00174f,0.17478f},{0.2441f,-0.9697f,-0.0117f},{0.1181f,-0.0000f},c});
    v.push_back({{-0.26193f,-0.02133f,0.15895f},{0.2441f,-0.9697f,-0.0117f},{0.2359f,0.0294f},c});
    v.push_back({{-0.18337f,-0.00174f,0.17478f},{-0.5571f,0.8300f,0.0268f},{0.5845f,0.8444f},c});
    v.push_back({{-0.22594f,-0.02785f,0.09840f},{-0.5571f,0.8300f,0.0268f},{0.6431f,0.7268f},c});
    v.push_back({{-0.22343f,-0.02785f,0.15056f},{-0.5571f,0.8300f,0.0268f},{0.6431f,0.8052f},c});
    v.push_back({{-0.18337f,-0.10619f,0.17478f},{-0.5571f,-0.8300f,0.0268f},{0.0581f,0.6941f},c});
    v.push_back({{-0.22594f,-0.08007f,0.09840f},{-0.5571f,-0.8300f,0.0268f},{0.1148f,0.8117f},c});
    v.push_back({{-0.18838f,-0.10619f,0.07046f},{-0.5571f,-0.8300f,0.0268f},{0.0581f,0.8509f},c});
    v.push_back({{-0.18337f,-0.00174f,0.17478f},{-0.5173f,0.0000f,0.8558f},{0.1148f,0.6941f},c});
    v.push_back({{-0.22343f,-0.08007f,0.15056f},{-0.5173f,0.0000f,0.8558f},{0.1709f,0.8117f},c});
    v.push_back({{-0.18337f,-0.10619f,0.17478f},{-0.5173f,0.0000f,0.8558f},{0.1148f,0.8509f},c});
    v.push_back({{-0.18838f,-0.10619f,0.07046f},{-0.5969f,0.0000f,-0.8023f},{0.0000f,0.6941f},c});
    v.push_back({{-0.22594f,-0.02785f,0.09840f},{-0.5969f,0.0000f,-0.8023f},{0.0581f,0.8117f},c});
    v.push_back({{-0.18838f,-0.00174f,0.07046f},{-0.5969f,0.0000f,-0.8023f},{-0.0000f,0.8509f},c});
    v.push_back({{-0.22343f,-0.02785f,0.15056f},{-0.0480f,0.0000f,-0.9988f},{0.2332f,0.7986f},c});
    v.push_back({{-0.15552f,-0.08007f,0.14730f},{-0.0480f,0.0000f,-0.9988f},{0.3116f,0.6965f},c});
    v.push_back({{-0.22343f,-0.08007f,0.15056f},{-0.0480f,0.0000f,-0.9988f},{0.3116f,0.7986f},c});
    v.push_back({{-0.22594f,-0.08007f,0.09840f},{0.0480f,0.0000f,0.9988f},{0.6663f,0.8402f},c});
    v.push_back({{-0.15802f,-0.02785f,0.09514f},{0.0480f,0.0000f,0.9988f},{0.7447f,0.7382f},c});
    v.push_back({{-0.22594f,-0.02785f,0.09840f},{0.0480f,0.0000f,0.9988f},{0.7447f,0.8402f},c});
    v.push_back({{-0.22594f,-0.02785f,0.09840f},{0.0000f,-1.0000f,-0.0000f},{0.7447f,0.8100f},c});
    v.push_back({{-0.15552f,-0.02785f,0.14730f},{0.0000f,-1.0000f,-0.0000f},{0.8231f,0.7080f},c});
    v.push_back({{-0.22343f,-0.02785f,0.15056f},{0.0000f,-1.0000f,-0.0000f},{0.8231f,0.8100f},c});
    v.push_back({{-0.22594f,-0.08007f,0.09840f},{0.0000f,1.0000f,-0.0000f},{0.4308f,0.8043f},c});
    v.push_back({{-0.15552f,-0.08007f,0.14730f},{0.0000f,1.0000f,-0.0000f},{0.3524f,0.7022f},c});
    v.push_back({{-0.15802f,-0.08007f,0.09514f},{0.0000f,1.0000f,-0.0000f},{0.4308f,0.7022f},c});
    v.push_back({{-0.15802f,-0.08007f,0.09514f},{0.2130f,0.0000f,-0.9771f},{0.7447f,0.8257f},c});
    v.push_back({{-0.19733f,-0.01741f,0.08657f},{0.2130f,0.0000f,-0.9771f},{0.8031f,0.9198f},c});
    v.push_back({{-0.15802f,-0.02785f,0.09514f},{0.2130f,0.0000f,-0.9771f},{0.7447f,0.9041f},c});
    v.push_back({{-0.15802f,-0.02785f,0.09514f},{0.2593f,0.9657f,-0.0125f},{0.1709f,0.8351f},c});
    v.push_back({{-0.19382f,-0.01741f,0.15960f},{0.2593f,0.9657f,-0.0125f},{0.2291f,0.9292f},c});
    v.push_back({{-0.15552f,-0.02785f,0.14730f},{0.2593f,0.9657f,-0.0125f},{0.1709f,0.9135f},c});
    v.push_back({{-0.15802f,-0.08007f,0.09514f},{0.2593f,-0.9657f,-0.0125f},{0.3524f,0.8984f},c});
    v.push_back({{-0.19382f,-0.09052f,0.15960f},{0.2593f,-0.9657f,-0.0125f},{0.4113f,0.8043f},c});
    v.push_back({{-0.19733f,-0.09052f,0.08657f},{0.2593f,-0.9657f,-0.0125f},{0.4113f,0.9141f},c});
    v.push_back({{-0.15552f,-0.02785f,0.14730f},{0.3056f,0.0000f,0.9522f},{0.2914f,0.8143f},c});
    v.push_back({{-0.19382f,-0.09052f,0.15960f},{0.3056f,0.0000f,0.9522f},{0.3505f,0.9084f},c});
    v.push_back({{-0.15552f,-0.08007f,0.14730f},{0.3056f,0.0000f,0.9522f},{0.2914f,0.8927f},c});
    v.push_back({{-0.19382f,-0.01741f,0.15960f},{-0.1590f,-0.9872f,0.0076f},{0.7472f,0.0404f},c});
    v.push_back({{-0.12889f,-0.02838f,0.09426f},{-0.1590f,-0.9872f,0.0076f},{0.6459f,0.1337f},c});
    v.push_back({{-0.12643f,-0.02838f,0.14538f},{-0.1590f,-0.9872f,0.0076f},{0.6459f,0.0569f},c});
    v.push_back({{-0.19382f,-0.09052f,0.15960f},{-0.1590f,0.9872f,0.0076f},{0.1022f,0.6941f},c});
    v.push_back({{-0.12889f,-0.07955f,0.09426f},{-0.1590f,0.9872f,0.0076f},{0.0000f,0.6008f},c});
    v.push_back({{-0.19733f,-0.09052f,0.08657f},{-0.1590f,0.9872f,0.0076f},{0.1022f,0.5843f},c});
    v.push_back({{-0.19382f,-0.01741f,0.15960f},{-0.2064f,0.0000f,-0.9785f},{0.2042f,0.6941f},c});
    v.push_back({{-0.12643f,-0.07955f,0.14538f},{-0.2064f,0.0000f,-0.9785f},{0.1022f,0.6008f},c});
    v.push_back({{-0.19382f,-0.09052f,0.15960f},{-0.2064f,0.0000f,-0.9785f},{0.2042f,0.5843f},c});
    v.push_back({{-0.19733f,-0.09052f,0.08657f},{-0.1117f,0.0000f,0.9937f},{0.7469f,0.2599f},c});
    v.push_back({{-0.12889f,-0.02838f,0.09426f},{-0.1117f,0.0000f,0.9937f},{0.6459f,0.1666f},c});
    v.push_back({{-0.19733f,-0.01741f,0.08657f},{-0.1117f,0.0000f,0.9937f},{0.7469f,0.1502f},c});
    v.push_back({{-0.12643f,-0.02838f,0.14538f},{0.4117f,0.0000f,0.9113f},{0.8172f,0.0275f},c});
    v.push_back({{-0.16451f,-0.09491f,0.16258f},{0.4117f,0.0000f,0.9113f},{0.8767f,0.1274f},c});
    v.push_back({{-0.12643f,-0.07955f,0.14538f},{0.4117f,0.0000f,0.9113f},{0.8172f,0.1043f},c});
    v.push_back({{-0.12889f,-0.07955f,0.09426f},{0.3224f,0.0000f,-0.9466f},{0.8231f,0.7311f},c});
    v.push_back({{-0.16843f,-0.01302f,0.08079f},{0.3224f,0.0000f,-0.9466f},{0.8815f,0.8310f},c});
    v.push_back({{-0.12889f,-0.02838f,0.09426f},{0.3224f,0.0000f,-0.9466f},{0.8231f,0.8079f},c});
    v.push_back({{-0.12889f,-0.02838f,0.09426f},{0.3671f,0.9300f,-0.0176f},{0.2332f,0.8216f},c});
    v.push_back({{-0.16451f,-0.01302f,0.16258f},{0.3671f,0.9300f,-0.0176f},{0.2914f,0.9215f},c});
    v.push_back({{-0.12643f,-0.02838f,0.14538f},{0.3671f,0.9300f,-0.0176f},{0.2332f,0.8985f},c});
    v.push_back({{-0.12889f,-0.07955f,0.09426f},{0.3671f,-0.9300f,-0.0176f},{0.8203f,0.5221f},c});
    v.push_back({{-0.16451f,-0.09491f,0.16258f},{0.3671f,-0.9300f,-0.0176f},{0.8795f,0.4222f},c});
    v.push_back({{-0.16843f,-0.09491f,0.08079f},{0.3671f,-0.9300f,-0.0176f},{0.8795f,0.5451f},c});
    v.push_back({{-0.16843f,-0.01302f,0.08079f},{-0.2060f,-0.9785f,0.0099f},{0.5845f,0.6851f},c});
    v.push_back({{-0.08767f,-0.02940f,0.14249f},{-0.2060f,-0.9785f,0.0099f},{0.4689f,0.5868f},c});
    v.push_back({{-0.16451f,-0.01302f,0.16258f},{-0.2060f,-0.9785f,0.0099f},{0.5845f,0.5622f},c});
    v.push_back({{-0.16451f,-0.09491f,0.16258f},{-0.2060f,0.9785f,0.0099f},{0.5865f,0.4197f},c});
    v.push_back({{-0.09003f,-0.07853f,0.09342f},{-0.2060f,0.9785f,0.0099f},{0.4696f,0.3214f},c});
    v.push_back({{-0.16843f,-0.09491f,0.08079f},{-0.2060f,0.9785f,0.0099f},{0.5865f,0.2968f},c});
    v.push_back({{-0.16451f,-0.01302f,0.16258f},{-0.2529f,0.0000f,-0.9675f},{0.5863f,0.2797f},c});
    v.push_back({{-0.08767f,-0.07853f,0.14249f},{-0.2529f,0.0000f,-0.9675f},{0.4698f,0.1814f},c});
    v.push_back({{-0.16451f,-0.09491f,0.16258f},{-0.2529f,0.0000f,-0.9675f},{0.5863f,0.1568f},c});
    v.push_back({{-0.16843f,-0.09491f,0.08079f},{-0.1590f,0.0000f,0.9873f},{0.3484f,0.6965f},c});
    v.push_back({{-0.09003f,-0.02940f,0.09342f},{-0.1590f,0.0000f,0.9873f},{0.2332f,0.5982f},c});
    v.push_back({{-0.16843f,-0.01302f,0.08079f},{-0.1590f,0.0000f,0.9873f},{0.3484f,0.5736f},c});
    v.push_back({{-0.08767f,-0.07853f,0.14249f},{0.4440f,-0.8958f,-0.0213f},{0.8147f,0.5880f},c});
    v.push_back({{-0.12976f,-0.09781f,0.07602f},{0.4440f,-0.8958f,-0.0213f},{0.8740f,0.6876f},c});
    v.push_back({{-0.09003f,-0.07853f,0.09342f},{0.4440f,-0.8958f,-0.0213f},{0.8147f,0.6618f},c});
    v.push_back({{-0.08767f,-0.02940f,0.14249f},{0.4870f,0.0000f,0.8734f},{0.1709f,0.7199f},c});
    v.push_back({{-0.12556f,-0.09781f,0.16362f},{0.4870f,0.0000f,0.8734f},{0.2306f,0.8195f},c});
    v.push_back({{-0.08767f,-0.07853f,0.14249f},{0.4870f,0.0000f,0.8734f},{0.1709f,0.7936f},c});
    v.push_back({{-0.09003f,-0.07853f,0.09342f},{0.4011f,0.0000f,-0.9160f},{0.8169f,0.1760f},c});
    v.push_back({{-0.12976f,-0.01012f,0.07602f},{0.4011f,0.0000f,-0.9160f},{0.8754f,0.2756f},c});
    v.push_back({{-0.09003f,-0.02940f,0.09342f},{0.4011f,0.0000f,-0.9160f},{0.8169f,0.2497f},c});
    v.push_back({{-0.09003f,-0.02940f,0.09342f},{0.4440f,0.8958f,-0.0213f},{0.8203f,0.3226f},c});
    v.push_back({{-0.12556f,-0.01012f,0.16362f},{0.4440f,0.8958f,-0.0213f},{0.8784f,0.4222f},c});
    v.push_back({{-0.08767f,-0.02940f,0.14249f},{0.4440f,0.8958f,-0.0213f},{0.8203f,0.3964f},c});
    v.push_back({{-0.12556f,-0.01012f,0.16362f},{-0.0480f,0.0000f,-0.9988f},{0.3499f,0.5736f},c});
    v.push_back({{-0.04794f,-0.09781f,0.15989f},{-0.0480f,0.0000f,-0.9988f},{0.2332f,0.4482f},c});
    v.push_back({{-0.12556f,-0.09781f,0.16362f},{-0.0480f,0.0000f,-0.9988f},{0.3499f,0.4482f},c});
    v.push_back({{-0.12976f,-0.09781f,0.07602f},{0.0480f,0.0000f,0.9988f},{0.2332f,0.5843f},c});
    v.push_back({{-0.05214f,-0.01012f,0.07230f},{0.0480f,0.0000f,0.9988f},{0.1167f,0.4589f},c});
    v.push_back({{-0.12976f,-0.01012f,0.07602f},{0.0480f,0.0000f,0.9988f},{0.2332f,0.4589f},c});
    v.push_back({{-0.12976f,-0.01012f,0.07602f},{0.0000f,-1.0000f,-0.0000f},{0.5854f,0.5622f},c});
    v.push_back({{-0.04794f,-0.01012f,0.15989f},{0.0000f,-1.0000f,-0.0000f},{0.4689f,0.4368f},c});
    v.push_back({{-0.12556f,-0.01012f,0.16362f},{0.0000f,-1.0000f,-0.0000f},{0.5854f,0.4368f},c});
    v.push_back({{-0.12556f,-0.09781f,0.16362f},{-0.0000f,1.0000f,-0.0000f},{0.1167f,0.5843f},c});
    v.push_back({{-0.05214f,-0.09781f,0.07230f},{-0.0000f,1.0000f,-0.0000f},{-0.0000f,0.4589f},c});
    v.push_back({{-0.12976f,-0.09781f,0.07602f},{-0.0000f,1.0000f,-0.0000f},{0.1167f,0.4589f},c});
    v.push_back({{-0.04794f,-0.01012f,0.15989f},{-0.0612f,0.9981f,0.0029f},{0.1191f,0.4390f},c});
    v.push_back({{-0.11986f,-0.01429f,0.07973f},{-0.0612f,0.9981f,0.0029f},{0.2212f,0.3167f},c});
    v.push_back({{-0.11605f,-0.01429f,0.15898f},{-0.0612f,0.9981f,0.0029f},{0.2212f,0.4359f},c});
    v.push_back({{-0.04794f,-0.09781f,0.15989f},{-0.0612f,-0.9981f,0.0029f},{0.5845f,0.5622f},c});
    v.push_back({{-0.11986f,-0.09364f,0.07973f},{-0.0612f,-0.9981f,0.0029f},{0.6863f,0.6845f},c});
    v.push_back({{-0.05214f,-0.09781f,0.07230f},{-0.0612f,-0.9981f,0.0029f},{0.5845f,0.6876f},c});
    v.push_back({{-0.04794f,-0.01012f,0.15989f},{-0.0133f,0.0000f,0.9999f},{0.3524f,0.5768f},c});
    v.push_back({{-0.11605f,-0.09364f,0.15898f},{-0.0133f,0.0000f,0.9999f},{0.4541f,0.6991f},c});
    v.push_back({{-0.04794f,-0.09781f,0.15989f},{-0.0133f,0.0000f,0.9999f},{0.3524f,0.7022f},c});
    v.push_back({{-0.05214f,-0.09781f,0.07230f},{-0.1091f,0.0000f,-0.9940f},{0.4718f,0.0000f},c});
    v.push_back({{-0.11986f,-0.01429f,0.07973f},{-0.1091f,0.0000f,-0.9940f},{0.5738f,0.1223f},c});
    v.push_back({{-0.05214f,-0.01012f,0.07230f},{-0.1091f,0.0000f,-0.9940f},{0.4718f,0.1254f},c});
    v.push_back({{-0.11605f,-0.01429f,0.15898f},{-0.0887f,0.0000f,-0.9961f},{0.2359f,0.1458f},c});
    v.push_back({{-0.01922f,-0.08967f,0.15036f},{-0.0887f,0.0000f,-0.9961f},{0.3490f,0.0000f},c});
    v.push_back({{-0.11605f,-0.09364f,0.15898f},{-0.0887f,0.0000f,-0.9961f},{0.3550f,0.1458f},c});
    v.push_back({{-0.11986f,-0.09364f,0.07973f},{0.0072f,0.0000f,1.0000f},{0.0000f,0.4589f},c});
    v.push_back({{-0.02265f,-0.01826f,0.07903f},{0.0072f,0.0000f,1.0000f},{0.1132f,0.3136f},c});
    v.push_back({{-0.11986f,-0.01429f,0.07973f},{0.0072f,0.0000f,1.0000f},{0.1191f,0.4589f},c});
    v.push_back({{-0.11986f,-0.01429f,0.07973f},{-0.0408f,-0.9992f,0.0020f},{0.2332f,0.4482f},c});
    v.push_back({{-0.01922f,-0.01826f,0.15036f},{-0.0408f,-0.9992f,0.0020f},{0.3464f,0.3027f},c});
    v.push_back({{-0.11605f,-0.01429f,0.15898f},{-0.0408f,-0.9992f,0.0020f},{0.3524f,0.4482f},c});
    v.push_back({{-0.11986f,-0.09364f,0.07973f},{-0.0408f,0.9992f,0.0020f},{0.3524f,0.3027f},c});
    v.push_back({{-0.01922f,-0.08967f,0.15036f},{-0.0408f,0.9992f,0.0020f},{0.2392f,0.1568f},c});
    v.push_back({{-0.02265f,-0.08967f,0.07903f},{-0.0408f,0.9992f,0.0020f},{0.3464f,0.1568f},c});
    v.push_back({{-0.01922f,-0.01826f,0.15036f},{-0.4809f,0.0000f,0.8768f},{0.8031f,0.8100f},c});
    v.push_back({{-0.03121f,-0.08253f,0.14379f},{-0.4809f,0.0000f,0.8768f},{0.8200f,0.9065f},c});
    v.push_back({{-0.01922f,-0.08967f,0.15036f},{-0.4809f,0.0000f,0.8768f},{0.8031f,0.9173f},c});
    v.push_back({{-0.02265f,-0.08967f,0.07903f},{-0.5627f,0.0000f,-0.8267f},{0.4113f,0.8043f},c});
    v.push_back({{-0.03395f,-0.02540f,0.08672f},{-0.5627f,0.0000f,-0.8267f},{0.4287f,0.9008f},c});
    v.push_back({{-0.02265f,-0.01826f,0.07903f},{-0.5627f,0.0000f,-0.8267f},{0.4113f,0.9115f},c});
    v.push_back({{-0.02265f,-0.01826f,0.07903f},{-0.5218f,0.8527f,0.0251f},{0.6431f,0.6876f},c});
    v.push_back({{-0.03121f,-0.02540f,0.14379f},{-0.5218f,0.8527f,0.0251f},{0.6607f,0.7841f},c});
    v.push_back({{-0.01922f,-0.01826f,0.15036f},{-0.5218f,0.8527f,0.0251f},{0.6431f,0.7948f},c});
    v.push_back({{-0.02265f,-0.08967f,0.07903f},{-0.5218f,-0.8527f,0.0251f},{0.6437f,0.9020f},c});
    v.push_back({{-0.03121f,-0.08253f,0.14379f},{-0.5218f,-0.8527f,0.0251f},{0.6607f,0.8055f},c});
    v.push_back({{-0.03395f,-0.08253f,0.08672f},{-0.5218f,-0.8527f,0.0251f},{0.6607f,0.8913f},c});
    v.push_back({{-0.03395f,-0.02540f,0.08672f},{-0.0586f,-0.9983f,0.0028f},{0.8958f,0.9167f},c});
    v.push_back({{0.01717f,-0.02826f,0.13860f},{-0.0586f,-0.9983f,0.0028f},{0.8231f,0.8353f},c});
    v.push_back({{-0.03121f,-0.02540f,0.14379f},{-0.0586f,-0.9983f,0.0028f},{0.8958f,0.8310f},c});
    v.push_back({{-0.03121f,-0.08253f,0.14379f},{-0.0586f,0.9983f,0.0028f},{0.7190f,0.5613f},c});
    v.push_back({{0.01470f,-0.07967f,0.08725f},{-0.0586f,0.9983f,0.0028f},{0.6461f,0.4798f},c});
    v.push_back({{-0.03395f,-0.08253f,0.08672f},{-0.0586f,0.9983f,0.0028f},{0.7190f,0.4755f},c});
    v.push_back({{-0.03121f,-0.08253f,0.14379f},{-0.1065f,0.0000f,-0.9943f},{0.7919f,0.4755f},c});
    v.push_back({{0.01717f,-0.02826f,0.13860f},{-0.1065f,0.0000f,-0.9943f},{0.7190f,0.5570f},c});
    v.push_back({{0.01717f,-0.07967f,0.13860f},{-0.1065f,0.0000f,-0.9943f},{0.7190f,0.4798f},c});
    v.push_back({{-0.03395f,-0.08253f,0.08672f},{-0.0107f,0.0000f,0.9999f},{0.7388f,0.9260f},c});
    v.push_back({{0.01470f,-0.02826f,0.08725f},{-0.0107f,0.0000f,0.9999f},{0.6663f,0.8445f},c});
    v.push_back({{-0.03395f,-0.02540f,0.08672f},{-0.0107f,0.0000f,0.9999f},{0.7388f,0.8402f},c});
    v.push_back({{0.01470f,-0.02826f,0.08725f},{-0.2555f,0.9667f,0.0123f},{0.3116f,0.6965f},c});
    v.push_back({{-0.00248f,-0.03340f,0.13440f},{-0.2555f,0.9667f,0.0123f},{0.3409f,0.7660f},c});
    v.push_back({{0.01717f,-0.02826f,0.13860f},{-0.2555f,0.9667f,0.0123f},{0.3116f,0.7737f},c});
    v.push_back({{0.01470f,-0.07967f,0.08725f},{-0.2555f,-0.9667f,0.0123f},{0.2042f,0.6615f},c});
    v.push_back({{-0.00248f,-0.07453f,0.13440f},{-0.2555f,-0.9667f,0.0123f},{0.2331f,0.5920f},c});
    v.push_back({{-0.00446f,-0.07453f,0.09331f},{-0.2555f,-0.9667f,0.0123f},{0.2331f,0.6538f},c});
    v.push_back({{0.01717f,-0.07967f,0.13860f},{-0.2092f,0.0000f,0.9779f},{0.1173f,0.9281f},c});
    v.push_back({{-0.00248f,-0.03340f,0.13440f},{-0.2092f,0.0000f,0.9779f},{0.1460f,0.8586f},c});
    v.push_back({{-0.00248f,-0.07453f,0.13440f},{-0.2092f,0.0000f,0.9779f},{0.1460f,0.9203f},c});
    v.push_back({{0.01470f,-0.02826f,0.08725f},{-0.3019f,0.0000f,-0.9533f},{0.5389f,0.9023f},c});
    v.push_back({{-0.00446f,-0.07453f,0.09331f},{-0.3019f,0.0000f,-0.9533f},{0.5680f,0.8329f},c});
    v.push_back({{-0.00446f,-0.03340f,0.09331f},{-0.3019f,0.0000f,-0.9533f},{0.5680f,0.8946f},c});
    v.push_back({{-0.00248f,-0.03340f,0.13440f},{0.0130f,0.0000f,-0.9999f},{0.9260f,0.0803f},c});
    v.push_back({{0.04812f,-0.07762f,0.13506f},{0.0130f,0.0000f,-0.9999f},{0.9924f,0.0044f},c});
    v.push_back({{-0.00248f,-0.07453f,0.13440f},{0.0130f,0.0000f,-0.9999f},{0.9878f,0.0803f},c});
    v.push_back({{-0.00446f,-0.07453f,0.09331f},{0.1088f,0.0000f,0.9941f},{0.3570f,0.9900f},c});
    v.push_back({{0.04585f,-0.03031f,0.08781f},{0.1088f,0.0000f,0.9941f},{0.4234f,0.9141f},c});
    v.push_back({{-0.00446f,-0.03340f,0.09331f},{0.1088f,0.0000f,0.9941f},{0.4188f,0.9900f},c});
    v.push_back({{-0.00248f,-0.03340f,0.13440f},{0.0609f,-0.9981f,-0.0029f},{0.9874f,0.6382f},c});
    v.push_back({{0.04585f,-0.03031f,0.08781f},{0.0609f,-0.9981f,-0.0029f},{0.9210f,0.5622f},c});
    v.push_back({{0.04812f,-0.03031f,0.13506f},{0.0609f,-0.9981f,-0.0029f},{0.9920f,0.5622f},c});
    v.push_back({{-0.00248f,-0.07453f,0.13440f},{0.0609f,0.9981f,-0.0029f},{0.8277f,0.9925f},c});
    v.push_back({{0.04585f,-0.07762f,0.08781f},{0.0609f,0.9981f,-0.0029f},{0.8941f,0.9167f},c});
    v.push_back({{-0.00446f,-0.07453f,0.09331f},{0.0609f,0.9981f,-0.0029f},{0.8895f,0.9925f},c});
    v.push_back({{0.04812f,-0.07762f,0.13506f},{-0.2663f,0.0000f,0.9639f},{0.8958f,1.0000f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{-0.2663f,0.0000f,0.9639f},{0.9593f,0.9503f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{-0.2663f,0.0000f,0.9639f},{0.9593f,0.9787f},c});
    v.push_back({{0.04585f,-0.07762f,0.08781f},{-0.3575f,0.0000f,-0.9339f},{0.2332f,0.9215f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{-0.3575f,0.0000f,-0.9339f},{0.2979f,0.9712f},c});
    v.push_back({{0.04585f,-0.03031f,0.08781f},{-0.3575f,0.0000f,-0.9339f},{0.2332f,0.9925f},c});
    v.push_back({{0.04812f,-0.03031f,0.13506f},{-0.3119f,0.9500f,0.0150f},{0.7447f,0.9908f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{-0.3119f,0.9500f,0.0150f},{0.8097f,0.9411f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{-0.3119f,0.9500f,0.0150f},{0.8097f,0.9695f},c});
    v.push_back({{0.04812f,-0.07762f,0.13506f},{-0.3119f,-0.9500f,0.0150f},{0.6663f,0.9260f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{-0.3119f,-0.9500f,0.0150f},{0.7302f,0.9757f},c});
    v.push_back({{0.04585f,-0.07762f,0.08781f},{-0.3119f,-0.9500f,0.0150f},{0.6663f,0.9970f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{0.3184f,-0.9478f,-0.0153f},{0.5865f,0.5190f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{0.3184f,-0.9478f,-0.0153f},{0.6093f,0.5360f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{-0.3167f,0.9484f,0.0152f},{0.6459f,0.2627f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{-0.3167f,0.9484f,0.0152f},{0.6231f,0.2797f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{0.0000f,-0.0000f,-1.0000f},{0.4308f,0.8935f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{0.0000f,-0.0000f,-1.0000f},{0.4592f,0.8935f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{0.1240f,0.0000f,0.9923f},{0.9503f,0.2902f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{0.1240f,0.0000f,0.9923f},{0.9219f,0.2902f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{-0.1971f,0.0000f,-0.9804f},{0.4308f,0.8935f},c});
    v.push_back({{0.03752f,-0.06342f,0.11628f},{-0.1971f,0.0000f,-0.9804f},{0.4592f,0.8433f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{-0.1971f,0.0000f,-0.9804f},{0.4592f,0.8935f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{0.2015f,0.0000f,0.9795f},{0.9219f,0.2902f},c});
    v.push_back({{0.03207f,-0.04450f,0.09816f},{0.2015f,0.0000f,0.9795f},{0.9503f,0.2465f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{0.2015f,0.0000f,0.9795f},{0.9503f,0.2902f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{0.0000f,-1.0000f,-0.0000f},{0.5865f,0.5190f},c});
    v.push_back({{0.03752f,-0.04450f,0.11628f},{0.0000f,-1.0000f,-0.0000f},{0.6333f,0.4913f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{0.0000f,-1.0000f,-0.0000f},{0.6093f,0.5360f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{0.0000f,1.0000f,-0.0000f},{0.6459f,0.2627f},c});
    v.push_back({{0.03752f,-0.06342f,0.11628f},{0.0000f,1.0000f,-0.0000f},{0.5991f,0.2349f},c});
    v.push_back({{0.03207f,-0.06342f,0.09816f},{0.0000f,1.0000f,-0.0000f},{0.6252f,0.2239f},c});
    v.push_back({{0.03752f,-0.04450f,0.11628f},{-0.4283f,0.0000f,-0.9036f},{0.4308f,0.8433f},c});
    v.push_back({{0.06813f,-0.06342f,0.10177f},{-0.4283f,0.0000f,-0.9036f},{0.4592f,0.7964f},c});
    v.push_back({{0.03752f,-0.06342f,0.11628f},{-0.4283f,0.0000f,-0.9036f},{0.4592f,0.8433f},c});
    v.push_back({{0.03207f,-0.06342f,0.09816f},{0.4323f,0.0000f,0.9017f},{0.9219f,0.2465f},c});
    v.push_back({{0.05846f,-0.04450f,0.08551f},{0.4323f,0.0000f,0.9017f},{0.9503f,0.2051f},c});
    v.push_back({{0.03207f,-0.04450f,0.09816f},{0.4323f,0.0000f,0.9017f},{0.9503f,0.2465f},c});
    v.push_back({{0.03207f,-0.04450f,0.09816f},{0.0000f,-1.0000f,0.0000f},{0.6071f,0.4802f},c});
    v.push_back({{0.06813f,-0.04450f,0.10177f},{0.0000f,-1.0000f,0.0000f},{0.6458f,0.4420f},c});
    v.push_back({{0.06813f,-0.06342f,0.10177f},{-0.0000f,1.0000f,-0.0000f},{0.5866f,0.1857f},c});
    v.push_back({{0.05846f,-0.06342f,0.08551f},{-0.0000f,1.0000f,-0.0000f},{0.6146f,0.1813f},c});
    v.push_back({{0.05846f,-0.04450f,0.08551f},{0.0000f,-1.0000f,0.0000f},{0.6178f,0.4376f},c});
    v.push_back({{0.09432f,-0.04450f,0.08029f},{0.0000f,-1.0000f,0.0000f},{0.6461f,0.3912f},c});
    v.push_back({{0.09432f,-0.06342f,0.08029f},{-0.0000f,1.0000f,-0.0000f},{0.5863f,0.1348f},c});
    v.push_back({{0.08100f,-0.06342f,0.06685f},{-0.0000f,1.0000f,-0.0000f},{0.6146f,0.1373f},c});
    v.push_back({{0.06813f,-0.04450f,0.10177f},{-0.6341f,0.0000f,-0.7733f},{0.4308f,0.7964f},c});
    v.push_back({{0.09432f,-0.06342f,0.08029f},{-0.6341f,0.0000f,-0.7733f},{0.4592f,0.7558f},c});
    v.push_back({{0.06813f,-0.06342f,0.10177f},{-0.6341f,0.0000f,-0.7733f},{0.4592f,0.7964f},c});
    v.push_back({{0.05846f,-0.06342f,0.08551f},{0.6375f,0.0000f,0.7704f},{0.9219f,0.2051f},c});
    v.push_back({{0.08100f,-0.04450f,0.06685f},{0.6375f,0.0000f,0.7704f},{0.9503f,0.1686f},c});
    v.push_back({{0.05846f,-0.04450f,0.08551f},{0.6375f,0.0000f,0.7704f},{0.9503f,0.2051f},c});
    v.push_back({{0.09432f,-0.04450f,0.08029f},{-0.8023f,0.0000f,-0.5970f},{0.4308f,0.7558f},c});
    v.push_back({{0.11454f,-0.06342f,0.05312f},{-0.8023f,0.0000f,-0.5970f},{0.4592f,0.7238f},c});
    v.push_back({{0.09432f,-0.06342f,0.08029f},{-0.8023f,0.0000f,-0.5970f},{0.4592f,0.7558f},c});
    v.push_back({{0.08100f,-0.06342f,0.06685f},{0.8049f,0.0000f,0.5934f},{0.9219f,0.1686f},c});
    v.push_back({{0.09837f,-0.04450f,0.04330f},{0.8049f,0.0000f,0.5934f},{0.9503f,0.1390f},c});
    v.push_back({{0.08100f,-0.04450f,0.06685f},{0.8049f,0.0000f,0.5934f},{0.9503f,0.1686f},c});
    v.push_back({{0.08100f,-0.04450f,0.06685f},{0.0000f,-1.0000f,0.0000f},{0.6178f,0.3937f},c});
    v.push_back({{0.11454f,-0.04450f,0.05312f},{0.0000f,-1.0000f,0.0000f},{0.6340f,0.3418f},c});
    v.push_back({{0.11454f,-0.06342f,0.05312f},{-0.0000f,1.0000f,-0.0000f},{0.5984f,0.0854f},c});
    v.push_back({{0.09837f,-0.06342f,0.04330f},{-0.0000f,1.0000f,-0.0000f},{0.6252f,0.0947f},c});
    v.push_back({{0.09837f,-0.06342f,0.04330f},{0.9245f,0.0000f,0.3812f},{0.9219f,0.1390f},c});
    v.push_back({{0.10952f,-0.04450f,0.01625f},{0.9245f,0.0000f,0.3812f},{0.9503f,0.1182f},c});
    v.push_back({{0.09837f,-0.04450f,0.04330f},{0.9245f,0.0000f,0.3812f},{0.9503f,0.1390f},c});
    v.push_back({{0.09837f,-0.04450f,0.04330f},{0.0000f,-1.0000f,0.0000f},{0.6071f,0.3511f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{0.0000f,-1.0000f,0.0000f},{0.6103f,0.2968f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{-0.0000f,1.0000f,-0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.10952f,-0.06342f,0.01625f},{-0.0000f,1.0000f,-0.0000f},{0.6459f,0.0559f},c});
    v.push_back({{0.11454f,-0.04450f,0.05312f},{-0.9228f,0.0000f,-0.3853f},{0.4308f,0.7238f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{-0.9228f,0.0000f,-0.3853f},{0.4592f,0.7022f},c});
    v.push_back({{0.11454f,-0.06342f,0.05312f},{-0.9228f,0.0000f,-0.3853f},{0.4592f,0.7238f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{0.1346f,0.0000f,0.9909f},{0.3840f,0.9560f},c});
    v.push_back({{0.10952f,-0.06342f,0.01625f},{0.1346f,0.0000f,0.9909f},{0.9219f,0.1182f},c});
    v.push_back({{0.11304f,-0.06973f,0.01577f},{0.1346f,0.0000f,0.9909f},{0.3840f,0.8797f},c});
    v.push_back({{0.10952f,-0.04450f,0.01625f},{0.2891f,-0.2317f,-0.9289f},{0.9503f,0.1182f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{0.2891f,-0.2317f,-0.9289f},{0.3840f,0.9560f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{0.2891f,-0.2317f,-0.9289f},{0.6103f,0.2968f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{-0.3867f,-0.0000f,0.9222f},{0.4592f,0.7022f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{-0.3867f,-0.0000f,0.9222f},{0.6103f,0.2968f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{-0.3867f,-0.0000f,0.9222f},{0.3840f,0.9560f},c});
    v.push_back({{0.10952f,-0.06342f,0.01625f},{0.2891f,0.2317f,-0.9289f},{0.9219f,0.1182f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{0.2891f,0.2317f,-0.9289f},{0.6221f,0.0404f},c});
    v.push_back({{0.11304f,-0.06973f,0.01577f},{0.2891f,0.2317f,-0.9289f},{0.3840f,0.8797f},c});
    v.push_back({{0.11304f,-0.06973f,-0.01577f},{0.1148f,0.0000f,-0.9934f},{0.7432f,0.9593f},c});
    v.push_back({{0.10954f,-0.04450f,-0.01617f},{0.1148f,0.0000f,-0.9934f},{0.6962f,0.6750f},c});
    v.push_back({{0.11304f,-0.03820f,-0.01577f},{0.1148f,0.0000f,-0.9934f},{0.7432f,0.8830f},c});
    v.push_back({{0.10954f,-0.04450f,-0.01617f},{0.2893f,-0.2202f,0.9316f},{0.6962f,0.6750f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{0.2893f,-0.2202f,0.9316f},{0.5075f,0.3013f},c});
    v.push_back({{0.11304f,-0.03820f,-0.01577f},{0.2893f,-0.2202f,0.9316f},{0.7432f,0.8830f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{-0.3816f,0.0000f,-0.9243f},{0.5075f,0.3013f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{-0.3816f,0.0000f,-0.9243f},{0.5873f,0.7922f},c});
    v.push_back({{0.11304f,-0.03820f,-0.01577f},{-0.3816f,0.0000f,-0.9243f},{0.7432f,0.8830f},c});
    v.push_back({{0.10954f,-0.06342f,-0.01617f},{0.2893f,0.2202f,0.9316f},{0.7245f,0.6750f},c});
    v.push_back({{0.11304f,-0.06973f,-0.01577f},{0.2893f,0.2202f,0.9316f},{0.7432f,0.9593f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{0.2893f,0.2202f,0.9316f},{0.4936f,0.0426f},c});
    v.push_back({{0.11304f,-0.06973f,0.01577f},{-0.3972f,0.9177f,-0.0000f},{0.3840f,0.8797f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{-0.3972f,0.9177f,-0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.11304f,-0.06973f,-0.01577f},{-0.3972f,0.9177f,-0.0000f},{0.7432f,0.9593f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{0.3977f,0.9175f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{0.3977f,0.9175f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{0.3977f,0.9175f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{-0.2933f,-0.9560f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.10602f,-0.05680f,-0.00650f},{-0.2933f,-0.9560f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{-0.2933f,-0.9560f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{0.5775f,0.0000f,0.8164f},{0.6156f,0.7922f},c});
    v.push_back({{0.10602f,-0.05680f,-0.00650f},{0.5775f,0.0000f,0.8164f},{0.5873f,0.7922f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{0.5775f,0.0000f,0.8164f},{0.5873f,0.7922f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{0.2933f,-0.9560f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.10602f,-0.05113f,-0.00650f},{0.2933f,-0.9560f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{0.2933f,-0.9560f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{0.5778f,0.0000f,-0.8161f},{0.4592f,0.7022f},c});
    v.push_back({{0.10601f,-0.05113f,0.00659f},{0.5778f,0.0000f,-0.8161f},{0.4308f,0.7022f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{0.5778f,0.0000f,-0.8161f},{0.4308f,0.7022f},c});
    v.push_back({{0.10602f,-0.05680f,-0.00650f},{0.0000f,1.0000f,-0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.17077f,-0.05680f,0.00659f},{0.0000f,1.0000f,-0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.17077f,-0.05680f,-0.00650f},{0.0000f,1.0000f,-0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.10602f,-0.05113f,-0.00650f},{0.0000f,-0.0000f,-1.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.17077f,-0.05680f,-0.00650f},{0.0000f,-0.0000f,-1.0000f},{0.5873f,0.7922f},c});
    v.push_back({{0.10602f,-0.05680f,-0.00650f},{0.0000f,-0.0000f,-1.0000f},{0.5873f,0.7922f},c});
    v.push_back({{0.10602f,-0.05113f,-0.00650f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05113f,0.00659f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05113f,-0.00650f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.10601f,-0.05680f,0.00659f},{0.0000f,-0.0000f,1.0000f},{0.4592f,0.7022f},c});
    v.push_back({{0.17077f,-0.05113f,0.00659f},{0.0000f,-0.0000f,1.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.10601f,-0.05113f,0.00659f},{0.0000f,-0.0000f,1.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.17077f,-0.05680f,0.00659f},{0.3123f,-0.9500f,0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.13625f,-0.06816f,-0.03269f},{0.3123f,-0.9500f,0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.17077f,-0.05680f,-0.00650f},{0.3123f,-0.9500f,0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.17077f,-0.05680f,-0.00650f},{-0.6044f,0.0000f,0.7967f},{0.5873f,0.7922f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{-0.6044f,0.0000f,0.7967f},{0.6156f,0.7922f},c});
    v.push_back({{0.13625f,-0.06816f,-0.03269f},{-0.6044f,0.0000f,0.7967f},{0.5873f,0.7922f},c});
    v.push_back({{0.17077f,-0.05113f,0.00659f},{-0.3123f,-0.9500f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{-0.3123f,-0.9500f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05113f,-0.00650f},{-0.3123f,-0.9500f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05113f,0.00659f},{-0.6041f,0.0000f,-0.7969f},{0.4308f,0.7022f},c});
    v.push_back({{0.13621f,-0.06816f,0.03278f},{-0.6041f,0.0000f,-0.7969f},{0.4592f,0.7022f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{-0.6041f,0.0000f,-0.7969f},{0.4308f,0.7022f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{-0.4472f,-0.8944f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.11466f,-0.02898f,-0.03269f},{-0.4472f,-0.8944f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{-0.4472f,-0.8944f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{-0.8944f,0.0000f,0.4472f},{0.6156f,0.7922f},c});
    v.push_back({{0.11466f,-0.06816f,-0.07587f},{-0.8944f,0.0000f,0.4472f},{0.5873f,0.7922f},c});
    v.push_back({{0.13625f,-0.06816f,-0.03269f},{-0.8944f,0.0000f,0.4472f},{0.5873f,0.7922f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{-0.4364f,-0.8729f,0.2182f},{0.6156f,0.7922f},c});
    v.push_back({{0.11466f,-0.02898f,-0.03269f},{-0.4364f,-0.8729f,0.2182f},{0.0000f,0.0000f},c});
    v.push_back({{0.11466f,-0.03977f,-0.07587f},{-0.4364f,-0.8729f,0.2182f},{0.6156f,0.7922f},c});
    v.push_back({{0.13621f,-0.06816f,0.03278f},{-0.8944f,0.0000f,-0.4472f},{0.4592f,0.7022f},c});
    v.push_back({{0.11463f,-0.03977f,0.07595f},{-0.8944f,0.0000f,-0.4472f},{0.4308f,0.7022f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{-0.8944f,0.0000f,-0.4472f},{0.4308f,0.7022f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{0.0000f,1.0000f,-0.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.11463f,-0.02898f,0.03278f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.11463f,-0.03977f,0.07595f},{0.0000f,1.0000f,-0.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.17077f,-0.05680f,-0.00650f},{0.0000f,-1.0000f,0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.18156f,-0.05680f,0.00659f},{0.0000f,-1.0000f,0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.17077f,-0.05680f,0.00659f},{0.0000f,-1.0000f,0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.17077f,-0.05680f,-0.00650f},{0.0000f,-0.0000f,1.0000f},{0.5873f,0.7922f},c});
    v.push_back({{0.18157f,-0.05113f,-0.00650f},{0.0000f,-0.0000f,1.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.17077f,-0.05113f,-0.00650f},{0.0000f,-0.0000f,1.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.17077f,-0.05113f,-0.00650f},{0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.18156f,-0.05113f,0.00659f},{0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05113f,0.00659f},{0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05113f,0.00659f},{0.0000f,-0.0000f,-1.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.18156f,-0.05680f,0.00659f},{0.0000f,-0.0000f,-1.0000f},{0.4592f,0.7022f},c});
    v.push_back({{0.17077f,-0.05680f,0.00659f},{0.0000f,-0.0000f,-1.0000f},{0.4592f,0.7022f},c});
    v.push_back({{0.18156f,-0.05113f,0.00659f},{0.4343f,0.9008f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.14921f,-0.03552f,-0.04252f},{0.4343f,0.9008f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.14916f,-0.03552f,0.04260f},{0.4343f,0.9008f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.18156f,-0.05113f,0.00659f},{0.7434f,0.0000f,0.6688f},{0.4308f,0.7022f},c});
    v.push_back({{0.14916f,-0.07241f,0.04260f},{0.7434f,0.0000f,0.6688f},{0.4592f,0.7022f},c});
    v.push_back({{0.18156f,-0.05680f,0.00659f},{0.7434f,0.0000f,0.6688f},{0.4592f,0.7022f},c});
    v.push_back({{0.18156f,-0.05680f,0.00659f},{-0.4343f,0.9008f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.14921f,-0.07241f,-0.04252f},{-0.4343f,0.9008f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.14916f,-0.07241f,0.04260f},{-0.4343f,0.9008f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.18157f,-0.05680f,-0.00650f},{0.7438f,0.0000f,-0.6684f},{0.5873f,0.7922f},c});
    v.push_back({{0.14921f,-0.03552f,-0.04252f},{0.7438f,0.0000f,-0.6684f},{0.6156f,0.7922f},c});
    v.push_back({{0.18157f,-0.05113f,-0.00650f},{0.7438f,0.0000f,-0.6684f},{0.6156f,0.7922f},c});
    v.push_back({{0.14921f,-0.07241f,-0.04252f},{0.1685f,-0.9857f,0.0001f},{0.4936f,0.0426f},c});
    v.push_back({{0.19234f,-0.06503f,0.02558f},{0.1685f,-0.9857f,0.0001f},{0.6221f,0.0404f},c});
    v.push_back({{0.14916f,-0.07241f,0.04260f},{0.1685f,-0.9857f,0.0001f},{0.6221f,0.0404f},c});
    v.push_back({{0.14921f,-0.03552f,-0.04252f},{-0.3669f,-0.0000f,0.9303f},{0.6156f,0.7922f},c});
    v.push_back({{0.19237f,-0.06503f,-0.02549f},{-0.3669f,-0.0000f,0.9303f},{0.5873f,0.7922f},c});
    v.push_back({{0.19237f,-0.04290f,-0.02549f},{-0.3669f,-0.0000f,0.9303f},{0.6156f,0.7922f},c});
    v.push_back({{0.14921f,-0.03552f,-0.04252f},{-0.1685f,-0.9857f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.19234f,-0.04290f,0.02558f},{-0.1685f,-0.9857f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.14916f,-0.03552f,0.04260f},{-0.1685f,-0.9857f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.14916f,-0.07241f,0.04260f},{-0.3668f,0.0000f,-0.9303f},{0.4592f,0.7022f},c});
    v.push_back({{0.19234f,-0.04290f,0.02558f},{-0.3668f,0.0000f,-0.9303f},{0.4308f,0.7022f},c});
    v.push_back({{0.19234f,-0.06503f,0.02558f},{-0.3668f,0.0000f,-0.9303f},{0.4592f,0.7022f},c});
    v.push_back({{0.19234f,-0.04290f,0.02558f},{-0.3929f,0.9196f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.17941f,-0.04843f,-0.01272f},{-0.3929f,0.9196f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.17940f,-0.04843f,0.01281f},{-0.3929f,0.9196f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.19234f,-0.06503f,0.02558f},{-0.7022f,0.0000f,0.7120f},{0.4592f,0.7022f},c});
    v.push_back({{0.17940f,-0.04843f,0.01281f},{-0.7022f,0.0000f,0.7120f},{0.4308f,0.7022f},c});
    v.push_back({{0.17940f,-0.05950f,0.01281f},{-0.7022f,0.0000f,0.7120f},{0.4592f,0.7022f},c});
    v.push_back({{0.19234f,-0.06503f,0.02558f},{0.3929f,0.9196f,0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.17941f,-0.05950f,-0.01272f},{0.3929f,0.9196f,0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.17940f,-0.05950f,0.01281f},{0.3929f,0.9196f,0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.19237f,-0.04290f,-0.02549f},{-0.7018f,0.0000f,-0.7123f},{0.6156f,0.7922f},c});
    v.push_back({{0.17941f,-0.05950f,-0.01272f},{-0.7018f,0.0000f,-0.7123f},{0.5873f,0.7922f},c});
    v.push_back({{0.17941f,-0.04843f,-0.01272f},{-0.7018f,0.0000f,-0.7123f},{0.6156f,0.7922f},c});
    v.push_back({{0.17940f,-0.04843f,0.01281f},{0.0000f,-0.0000f,-1.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.21393f,-0.05950f,0.01281f},{0.0000f,-0.0000f,-1.0000f},{0.4592f,0.7022f},c});
    v.push_back({{0.17940f,-0.05950f,0.01281f},{0.0000f,-0.0000f,-1.0000f},{0.4592f,0.7022f},c});
    v.push_back({{0.17941f,-0.05950f,-0.01272f},{0.0000f,-1.0000f,0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.21393f,-0.05950f,0.01281f},{0.0000f,-1.0000f,0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.17940f,-0.05950f,0.01281f},{0.0000f,-1.0000f,0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.17941f,-0.05950f,-0.01272f},{0.0000f,-0.0000f,1.0000f},{0.5873f,0.7922f},c});
    v.push_back({{0.21395f,-0.04843f,-0.01272f},{0.0000f,-0.0000f,1.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.17941f,-0.04843f,-0.01272f},{0.0000f,-0.0000f,1.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.17941f,-0.04843f,-0.01272f},{0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.21393f,-0.04843f,0.01281f},{0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.17940f,-0.04843f,0.01281f},{0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.21393f,-0.05950f,0.01281f},{-0.3929f,0.9196f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.18806f,-0.07057f,-0.03826f},{-0.3929f,0.9196f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.18802f,-0.07057f,0.03834f},{-0.3929f,0.9196f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.21395f,-0.05950f,-0.01272f},{0.7022f,0.0000f,-0.7120f},{0.5873f,0.7922f},c});
    v.push_back({{0.18806f,-0.03736f,-0.03826f},{0.7022f,0.0000f,-0.7120f},{0.6156f,0.7922f},c});
    v.push_back({{0.21395f,-0.04843f,-0.01272f},{0.7022f,0.0000f,-0.7120f},{0.6156f,0.7922f},c});
    v.push_back({{0.21393f,-0.04843f,0.01281f},{0.3929f,0.9196f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.18806f,-0.03736f,-0.03826f},{0.3929f,0.9196f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.18802f,-0.03736f,0.03834f},{0.3929f,0.9196f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.21393f,-0.04843f,0.01281f},{0.7018f,0.0000f,0.7123f},{0.4308f,0.7022f},c});
    v.push_back({{0.18802f,-0.07057f,0.03834f},{0.7018f,0.0000f,0.7123f},{0.4592f,0.7022f},c});
    v.push_back({{0.21393f,-0.05950f,0.01281f},{0.7018f,0.0000f,0.7123f},{0.4592f,0.7022f},c});
    v.push_back({{0.18806f,-0.03736f,-0.03826f},{-0.1520f,-0.9884f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.23120f,-0.04400f,0.02302f},{-0.1520f,-0.9884f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.18802f,-0.03736f,0.03834f},{-0.1520f,-0.9884f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.18802f,-0.03736f,0.03834f},{-0.3344f,0.0000f,-0.9424f},{0.4308f,0.7022f},c});
    v.push_back({{0.23120f,-0.06393f,0.02302f},{-0.3344f,0.0000f,-0.9424f},{0.4592f,0.7022f},c});
    v.push_back({{0.18802f,-0.07057f,0.03834f},{-0.3344f,0.0000f,-0.9424f},{0.4592f,0.7022f},c});
    v.push_back({{0.18806f,-0.07057f,-0.03826f},{0.1520f,-0.9884f,0.0001f},{0.4936f,0.0426f},c});
    v.push_back({{0.23120f,-0.06393f,0.02302f},{0.1520f,-0.9884f,0.0001f},{0.6221f,0.0404f},c});
    v.push_back({{0.18802f,-0.07057f,0.03834f},{0.1520f,-0.9884f,0.0001f},{0.6221f,0.0404f},c});
    v.push_back({{0.18806f,-0.03736f,-0.03826f},{-0.3345f,-0.0000f,0.9424f},{0.6156f,0.7922f},c});
    v.push_back({{0.23122f,-0.06393f,-0.02294f},{-0.3345f,-0.0000f,0.9424f},{0.5873f,0.7922f},c});
    v.push_back({{0.23122f,-0.04400f,-0.02294f},{-0.3345f,-0.0000f,0.9424f},{0.6156f,0.7922f},c});
    v.push_back({{0.24391f,-0.05396f,-0.02235f},{0.0466f,0.0000f,-0.9989f},{0.2443f,0.7887f},c});
    v.push_back({{0.23122f,-0.06393f,-0.02294f},{0.0466f,0.0000f,-0.9989f},{0.2196f,0.8042f},c});
    v.push_back({{0.23122f,-0.04400f,-0.02294f},{0.0466f,0.0000f,-0.9989f},{0.2244f,0.7673f},c});
    v.push_back({{0.23120f,-0.04400f,0.02302f},{0.6121f,0.7867f,-0.0806f},{0.2196f,0.8517f},c});
    v.push_back({{0.24391f,-0.05396f,0.02235f},{0.6121f,0.7867f,-0.0806f},{0.1924f,0.8460f},c});
    v.push_back({{0.24097f,-0.05396f,-0.00000f},{0.6121f,0.7867f,-0.0806f},{0.1948f,0.8083f},c});
    v.push_back({{0.41366f,-0.12874f,0.04317f},{-0.0334f,-0.9234f,0.3824f},{0.3881f,0.7707f},c});
    v.push_back({{0.39680f,-0.12874f,0.04170f},{-0.0334f,-0.9234f,0.3824f},{0.3888f,0.7405f},c});
    v.push_back({{0.40498f,-0.13737f,0.02159f},{-0.0334f,-0.9234f,0.3824f},{0.4231f,0.7597f},c});
    v.push_back({{0.41366f,-0.07631f,0.08340f},{-0.0794f,-0.3817f,0.9209f},{0.0015f,0.3581f},c});
    v.push_back({{0.38070f,-0.07631f,0.08056f},{-0.0794f,-0.3817f,0.9209f},{0.0623f,0.3581f},c});
    v.push_back({{0.38419f,-0.09714f,0.07223f},{-0.0794f,-0.3817f,0.9209f},{0.0602f,0.3971f},c});
    v.push_back({{0.41366f,-0.09714f,0.07478f},{-0.0684f,-0.6076f,0.7913f},{0.0058f,0.3971f},c});
    v.push_back({{0.38419f,-0.09714f,0.07223f},{-0.0684f,-0.6076f,0.7913f},{0.0602f,0.3971f},c});
    v.push_back({{0.38969f,-0.11502f,0.05897f},{-0.0684f,-0.6076f,0.7913f},{0.0569f,0.4306f},c});
    v.push_back({{0.38969f,-0.11502f,0.05897f},{-0.0528f,-0.7924f,0.6078f},{0.3618f,0.7205f},c});
    v.push_back({{0.37951f,-0.05396f,0.08340f},{-0.0851f,-0.1302f,0.9878f},{0.0631f,0.3163f},c});
    v.push_back({{0.38969f,-0.11502f,0.05897f},{-0.1482f,-0.8044f,0.5753f},{0.3618f,0.7205f},c});
    v.push_back({{0.36602f,-0.11502f,0.05287f},{-0.1482f,-0.8044f,0.5753f},{0.3702f,0.6780f},c});
    v.push_back({{0.38038f,-0.12874f,0.03739f},{-0.1482f,-0.8044f,0.5753f},{0.3948f,0.7110f},c});
    v.push_back({{0.37951f,-0.05396f,0.08340f},{-0.2395f,-0.1366f,0.9612f},{0.0631f,0.3163f},c});
    v.push_back({{0.34489f,-0.05396f,0.07478f},{-0.2395f,-0.1366f,0.9612f},{0.1298f,0.3163f},c});
    v.push_back({{0.34741f,-0.07631f,0.07223f},{-0.2395f,-0.1366f,0.9612f},{0.1266f,0.3581f},c});
    v.push_back({{0.39680f,-0.12874f,0.04170f},{-0.0943f,-0.9285f,0.3591f},{0.3888f,0.7405f},c});
    v.push_back({{0.38038f,-0.12874f,0.03739f},{-0.0943f,-0.9285f,0.3591f},{0.3948f,0.7110f},c});
    v.push_back({{0.39664f,-0.13737f,0.01935f},{-0.0943f,-0.9285f,0.3591f},{0.4262f,0.7447f},c});
    v.push_back({{0.38070f,-0.07631f,0.08056f},{-0.2228f,-0.3973f,0.8902f},{0.0623f,0.3581f},c});
    v.push_back({{0.34741f,-0.07631f,0.07223f},{-0.2228f,-0.3973f,0.8902f},{0.1266f,0.3581f},c});
    v.push_back({{0.35469f,-0.09714f,0.06476f},{-0.2228f,-0.3973f,0.8902f},{0.1171f,0.3971f},c});
    v.push_back({{0.38419f,-0.09714f,0.07223f},{-0.1917f,-0.6245f,0.7571f},{0.0602f,0.3971f},c});
    v.push_back({{0.35469f,-0.09714f,0.06476f},{-0.1917f,-0.6245f,0.7571f},{0.1171f,0.3971f},c});
    v.push_back({{0.36602f,-0.11502f,0.05287f},{-0.1917f,-0.6245f,0.7571f},{0.1026f,0.4306f},c});
    v.push_back({{0.35469f,-0.09714f,0.06476f},{-0.2894f,-0.6653f,0.6882f},{0.1171f,0.3971f},c});
    v.push_back({{0.32643f,-0.09714f,0.05287f},{-0.2894f,-0.6653f,0.6882f},{0.1741f,0.3971f},c});
    v.push_back({{0.34447f,-0.11502f,0.04317f},{-0.2894f,-0.6653f,0.6882f},{0.1464f,0.4306f},c});
    v.push_back({{0.36602f,-0.11502f,0.05287f},{-0.2316f,-0.8257f,0.5143f},{0.3702f,0.6780f},c});
    v.push_back({{0.34447f,-0.11502f,0.04317f},{-0.2316f,-0.8257f,0.5143f},{0.3854f,0.6393f},c});
    v.push_back({{0.36531f,-0.12874f,0.03053f},{-0.2316f,-0.8257f,0.5143f},{0.4056f,0.6839f},c});
    v.push_back({{0.34489f,-0.05396f,0.07478f},{-0.3591f,-0.1534f,0.9206f},{0.1298f,0.3163f},c});
    v.push_back({{0.30971f,-0.05396f,0.06105f},{-0.3591f,-0.1534f,0.9206f},{0.2001f,0.3163f},c});
    v.push_back({{0.31393f,-0.07631f,0.05897f},{-0.3591f,-0.1534f,0.9206f},{0.1936f,0.3581f},c});
    v.push_back({{0.38038f,-0.12874f,0.03739f},{-0.1442f,-0.9375f,0.3166f},{0.3948f,0.7110f},c});
    v.push_back({{0.36531f,-0.12874f,0.03053f},{-0.1442f,-0.9375f,0.3166f},{0.4056f,0.6839f},c});
    v.push_back({{0.38906f,-0.13737f,0.01580f},{-0.1442f,-0.9375f,0.3166f},{0.4318f,0.7311f},c});
    v.push_back({{0.34741f,-0.07631f,0.07223f},{-0.3301f,-0.4424f,0.8338f},{0.1266f,0.3581f},c});
    v.push_back({{0.31393f,-0.07631f,0.05897f},{-0.3301f,-0.4424f,0.8338f},{0.1936f,0.3581f},c});
    v.push_back({{0.32643f,-0.09714f,0.05287f},{-0.3301f,-0.4424f,0.8338f},{0.1741f,0.3971f},c});
    v.push_back({{0.36531f,-0.12874f,0.03053f},{-0.2055f,-0.9490f,0.2392f},{0.4056f,0.6839f},c});
    v.push_back({{0.35490f,-0.12874f,0.02159f},{-0.2055f,-0.9490f,0.2392f},{0.4207f,0.6652f},c});
    v.push_back({{0.38261f,-0.13737f,0.01117f},{-0.2055f,-0.9490f,0.2392f},{0.4395f,0.7195f},c});
    v.push_back({{0.31393f,-0.07631f,0.05897f},{-0.3645f,-0.5486f,0.7525f},{0.1936f,0.3581f},c});
    v.push_back({{0.27826f,-0.07631f,0.04170f},{-0.3645f,-0.5486f,0.7525f},{0.2665f,0.3581f},c});
    v.push_back({{0.30070f,-0.09714f,0.03739f},{-0.3645f,-0.5486f,0.7525f},{0.2282f,0.3971f},c});
    v.push_back({{0.32643f,-0.09714f,0.05287f},{-0.3506f,-0.7333f,0.5825f},{0.3726f,0.5974f},c});
    v.push_back({{0.30070f,-0.09714f,0.03739f},{-0.3506f,-0.7333f,0.5825f},{0.3979f,0.5512f},c});
    v.push_back({{0.32671f,-0.11502f,0.03053f},{-0.3506f,-0.7333f,0.5825f},{0.4064f,0.6073f},c});
    v.push_back({{0.34447f,-0.11502f,0.04317f},{-0.2912f,-0.8648f,0.4090f},{0.3854f,0.6393f},c});
    v.push_back({{0.32671f,-0.11502f,0.03053f},{-0.2912f,-0.8648f,0.4090f},{0.4064f,0.6073f},c});
    v.push_back({{0.35490f,-0.12874f,0.02159f},{-0.2912f,-0.8648f,0.4090f},{0.4207f,0.6652f},c});
    v.push_back({{0.30971f,-0.05396f,0.06105f},{-0.3658f,-0.2592f,0.8939f},{0.2001f,0.3163f},c});
    v.push_back({{0.26602f,-0.05396f,0.04317f},{-0.3658f,-0.2592f,0.8939f},{0.2879f,0.3163f},c});
    v.push_back({{0.27826f,-0.07631f,0.04170f},{-0.3658f,-0.2592f,0.8939f},{0.2665f,0.3581f},c});
    v.push_back({{0.32671f,-0.11502f,0.03053f},{-0.3382f,-0.8970f,0.2845f},{0.4064f,0.6073f},c});
    v.push_back({{0.31432f,-0.11502f,0.01580f},{-0.3382f,-0.8970f,0.2845f},{0.4319f,0.5851f},c});
    v.push_back({{0.34682f,-0.12874f,0.01117f},{-0.3382f,-0.8970f,0.2845f},{0.4388f,0.6507f},c});
    v.push_back({{0.26602f,-0.05396f,0.04317f},{-0.6814f,-0.1113f,0.7234f},{0.2879f,0.3163f},c});
    v.push_back({{0.24391f,-0.05396f,0.02235f},{-0.6814f,-0.1113f,0.7234f},{0.3382f,0.3163f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{-0.6814f,-0.1113f,0.7234f},{0.3334f,0.3581f},c});
    v.push_back({{0.35490f,-0.12874f,0.02159f},{-0.2356f,-0.9545f,0.1829f},{0.4207f,0.6652f},c});
    v.push_back({{0.34682f,-0.12874f,0.01117f},{-0.2356f,-0.9545f,0.1829f},{0.4388f,0.6507f},c});
    v.push_back({{0.37758f,-0.13737f,0.00578f},{-0.2356f,-0.9545f,0.1829f},{0.4488f,0.7105f},c});
    v.push_back({{0.27826f,-0.07631f,0.04170f},{-0.4013f,-0.6663f,0.6286f},{0.2665f,0.3581f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{-0.4013f,-0.6663f,0.6286f},{0.3334f,0.3581f},c});
    v.push_back({{0.27783f,-0.09714f,0.01935f},{-0.4013f,-0.6663f,0.6286f},{0.2785f,0.3971f},c});
    v.push_back({{0.30070f,-0.09714f,0.03739f},{-0.3566f,-0.8175f,0.4522f},{0.3979f,0.5512f},c});
    v.push_back({{0.27783f,-0.09714f,0.01935f},{-0.3566f,-0.8175f,0.4522f},{0.4282f,0.5101f},c});
    v.push_back({{0.31432f,-0.11502f,0.01580f},{-0.3566f,-0.8175f,0.4522f},{0.4319f,0.5851f},c});
    v.push_back({{0.27783f,-0.09714f,0.01935f},{-0.3393f,-0.8885f,0.3088f},{0.4282f,0.5101f},c});
    v.push_back({{0.26022f,-0.09714f,-0.00000f},{-0.3393f,-0.8885f,0.3088f},{0.4615f,0.4785f},c});
    v.push_back({{0.30705f,-0.11502f,0.00000f},{-0.3393f,-0.8885f,0.3088f},{0.4599f,0.5720f},c});
    v.push_back({{0.31432f,-0.11502f,0.01580f},{-0.3643f,-0.9161f,0.1676f},{0.4319f,0.5851f},c});
    v.push_back({{0.30705f,-0.11502f,0.00000f},{-0.3643f,-0.9161f,0.1676f},{0.4599f,0.5720f},c});
    v.push_back({{0.34155f,-0.12874f,0.00000f},{-0.3643f,-0.9161f,0.1676f},{0.4586f,0.6412f},c});
    v.push_back({{0.24391f,-0.05396f,0.02235f},{0.0531f,0.0000f,0.9986f},{0.3382f,0.3163f},c});
    v.push_back({{0.23120f,-0.04400f,0.02302f},{0.0531f,0.0000f,0.9986f},{0.3608f,0.2977f},c});
    v.push_back({{0.23120f,-0.06393f,0.02302f},{0.0531f,0.0000f,0.9986f},{0.3608f,0.3350f},c});
    v.push_back({{0.34682f,-0.12874f,0.01117f},{-0.2536f,-0.9599f,0.1196f},{0.4388f,0.6507f},c});
    v.push_back({{0.34155f,-0.12874f,0.00000f},{-0.2536f,-0.9599f,0.1196f},{0.4586f,0.6412f},c});
    v.push_back({{0.37420f,-0.13737f,0.00000f},{-0.2536f,-0.9599f,0.1196f},{0.4590f,0.7044f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{-0.7832f,-0.6132f,0.1031f},{0.4286f,0.4433f},c});
    v.push_back({{0.24391f,-0.07631f,-0.00000f},{-0.7832f,-0.6132f,0.1031f},{0.4677f,0.4382f},c});
    v.push_back({{0.26022f,-0.09714f,-0.00000f},{-0.7832f,-0.6132f,0.1031f},{0.4615f,0.4785f},c});
    v.push_back({{0.24675f,-0.07631f,0.02159f},{-0.6162f,-0.7834f,0.0811f},{0.4286f,0.4433f},c});
    v.push_back({{0.23120f,-0.06393f,0.02302f},{-0.6162f,-0.7834f,0.0811f},{0.4291f,0.4088f},c});
    v.push_back({{0.24391f,-0.07631f,-0.00000f},{-0.6162f,-0.7834f,0.0811f},{0.4677f,0.4382f},c});
    v.push_back({{0.23120f,-0.06393f,0.02302f},{-0.6982f,-0.7159f,-0.0004f},{0.4291f,0.4088f},c});
    v.push_back({{0.23122f,-0.06393f,-0.02294f},{-0.6982f,-0.7159f,-0.0004f},{0.5130f,0.4088f},c});
    v.push_back({{0.24391f,-0.07631f,-0.00000f},{-0.6982f,-0.7159f,-0.0004f},{0.4677f,0.4382f},c});
    v.push_back({{0.37420f,-0.13737f,0.00000f},{-0.2536f,-0.9599f,-0.1196f},{0.4590f,0.7044f},c});
    v.push_back({{0.34155f,-0.12874f,0.00000f},{-0.2536f,-0.9599f,-0.1196f},{0.4586f,0.6412f},c});
    v.push_back({{0.34682f,-0.12874f,-0.01117f},{-0.2536f,-0.9599f,-0.1196f},{0.4796f,0.6507f},c});
    v.push_back({{0.26022f,-0.09714f,-0.00000f},{-0.7832f,-0.6132f,-0.1031f},{0.4615f,0.4785f},c});
    v.push_back({{0.24391f,-0.07631f,-0.00000f},{-0.7832f,-0.6132f,-0.1031f},{0.4677f,0.4382f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{-0.7832f,-0.6132f,-0.1031f},{0.5075f,0.4433f},c});
    v.push_back({{0.30705f,-0.11502f,0.00000f},{-0.3393f,-0.8885f,-0.3088f},{0.4599f,0.5720f},c});
    v.push_back({{0.26022f,-0.09714f,-0.00000f},{-0.3393f,-0.8885f,-0.3088f},{0.4615f,0.4785f},c});
    v.push_back({{0.27783f,-0.09714f,-0.01935f},{-0.3393f,-0.8885f,-0.3088f},{0.4989f,0.5101f},c});
    v.push_back({{0.34155f,-0.12874f,0.00000f},{-0.3643f,-0.9161f,-0.1676f},{0.4586f,0.6412f},c});
    v.push_back({{0.30705f,-0.11502f,0.00000f},{-0.3643f,-0.9161f,-0.1676f},{0.4599f,0.5720f},c});
    v.push_back({{0.31432f,-0.11502f,-0.01580f},{-0.3643f,-0.9161f,-0.1676f},{0.4896f,0.5851f},c});
    v.push_back({{0.34682f,-0.12874f,-0.01117f},{-0.3382f,-0.8970f,-0.2845f},{0.4796f,0.6507f},c});
    v.push_back({{0.31432f,-0.11502f,-0.01580f},{-0.3382f,-0.8970f,-0.2845f},{0.4896f,0.5851f},c});
    v.push_back({{0.32671f,-0.11502f,-0.03053f},{-0.3382f,-0.8970f,-0.2845f},{0.5180f,0.6073f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{-0.6814f,-0.1113f,-0.7234f},{0.9237f,0.2889f},c});
    v.push_back({{0.24391f,-0.05396f,-0.02235f},{-0.6814f,-0.1113f,-0.7234f},{0.9252f,0.2475f},c});
    v.push_back({{0.26602f,-0.05396f,-0.04317f},{-0.6814f,-0.1113f,-0.7234f},{0.9641f,0.2448f},c});
    v.push_back({{0.37758f,-0.13737f,-0.00578f},{-0.2356f,-0.9545f,-0.1829f},{0.4699f,0.7105f},c});
    v.push_back({{0.34682f,-0.12874f,-0.01117f},{-0.2356f,-0.9545f,-0.1829f},{0.4796f,0.6507f},c});
    v.push_back({{0.35490f,-0.12874f,-0.02159f},{-0.2356f,-0.9545f,-0.1829f},{0.4996f,0.6652f},c});
    v.push_back({{0.27783f,-0.09714f,-0.01935f},{-0.4013f,-0.6663f,-0.6286f},{0.4989f,0.5101f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{-0.4013f,-0.6663f,-0.6286f},{0.5075f,0.4433f},c});
    v.push_back({{0.27826f,-0.07631f,-0.04170f},{-0.4013f,-0.6663f,-0.6286f},{0.5479f,0.4998f},c});
    v.push_back({{0.31432f,-0.11502f,-0.01580f},{-0.3566f,-0.8175f,-0.4522f},{0.4896f,0.5851f},c});
    v.push_back({{0.27783f,-0.09714f,-0.01935f},{-0.3566f,-0.8175f,-0.4522f},{0.4989f,0.5101f},c});
    v.push_back({{0.30070f,-0.09714f,-0.03739f},{-0.3566f,-0.8175f,-0.4522f},{0.5345f,0.5511f},c});
    v.push_back({{0.30070f,-0.09714f,-0.03739f},{-0.3645f,-0.5486f,-0.7525f},{0.5345f,0.5511f},c});
    v.push_back({{0.27826f,-0.07631f,-0.04170f},{-0.3645f,-0.5486f,-0.7525f},{0.5479f,0.4998f},c});
    v.push_back({{0.31393f,-0.07631f,-0.05897f},{-0.3645f,-0.5486f,-0.7525f},{0.5836f,0.5639f},c});
    v.push_back({{0.32671f,-0.11502f,-0.03053f},{-0.3506f,-0.7333f,-0.5825f},{0.5180f,0.6073f},c});
    v.push_back({{0.30070f,-0.09714f,-0.03739f},{-0.3506f,-0.7333f,-0.5825f},{0.5345f,0.5511f},c});
    v.push_back({{0.32643f,-0.09714f,-0.05287f},{-0.3506f,-0.7333f,-0.5825f},{0.5658f,0.5973f},c});
    v.push_back({{0.30971f,0.05396f,0.06105f},{-0.3788f,0.0000f,0.9255f},{0.2001f,0.1143f},c});
    v.push_back({{0.35490f,-0.12874f,-0.02159f},{-0.2912f,-0.8648f,-0.4090f},{0.4996f,0.6652f},c});
    v.push_back({{0.32671f,-0.11502f,-0.03053f},{-0.2912f,-0.8648f,-0.4090f},{0.5180f,0.6073f},c});
    v.push_back({{0.34447f,-0.11502f,-0.04317f},{-0.2912f,-0.8648f,-0.4090f},{0.5431f,0.6392f},c});
    v.push_back({{0.27826f,-0.07631f,-0.04170f},{-0.3658f,-0.2592f,-0.8939f},{0.6295f,0.3248f},c});
    v.push_back({{0.26602f,-0.05396f,-0.04317f},{-0.3658f,-0.2592f,-0.8939f},{0.6072f,0.2829f},c});
    v.push_back({{0.30971f,-0.05396f,-0.06105f},{-0.3658f,-0.2592f,-0.8939f},{0.6706f,0.2829f},c});
    v.push_back({{0.24391f,0.05396f,-0.02235f},{-0.6856f,-0.0000f,-0.7280f},{0.9252f,0.0459f},c});
    v.push_back({{0.38261f,-0.13737f,-0.01117f},{-0.2055f,-0.9490f,-0.2392f},{0.4804f,0.7195f},c});
    v.push_back({{0.35490f,-0.12874f,-0.02159f},{-0.2055f,-0.9490f,-0.2392f},{0.4996f,0.6652f},c});
    v.push_back({{0.36531f,-0.12874f,-0.03053f},{-0.2055f,-0.9490f,-0.2392f},{0.5171f,0.6839f},c});
    v.push_back({{0.31393f,-0.07631f,-0.05897f},{-0.3591f,-0.1534f,-0.9206f},{0.6793f,0.3247f},c});
    v.push_back({{0.30971f,-0.05396f,-0.06105f},{-0.3591f,-0.1534f,-0.9206f},{0.6706f,0.2829f},c});
    v.push_back({{0.34489f,-0.05396f,-0.07478f},{-0.3591f,-0.1534f,-0.9206f},{0.7221f,0.2829f},c});
    v.push_back({{0.26602f,0.05396f,-0.04317f},{-0.3788f,-0.0000f,-0.9255f},{0.6072f,0.0809f},c});
    v.push_back({{0.38906f,-0.13737f,-0.01580f},{-0.1442f,-0.9375f,-0.3166f},{0.4896f,0.7311f},c});
    v.push_back({{0.36531f,-0.12874f,-0.03053f},{-0.1442f,-0.9375f,-0.3166f},{0.5171f,0.6839f},c});
    v.push_back({{0.38038f,-0.12874f,-0.03739f},{-0.1442f,-0.9375f,-0.3166f},{0.5314f,0.7109f},c});
    v.push_back({{0.32643f,-0.09714f,-0.05287f},{-0.3301f,-0.4424f,-0.8338f},{0.5658f,0.5973f},c});
    v.push_back({{0.31393f,-0.07631f,-0.05897f},{-0.3301f,-0.4424f,-0.8338f},{0.5836f,0.5639f},c});
    v.push_back({{0.34741f,-0.07631f,-0.07223f},{-0.3301f,-0.4424f,-0.8338f},{0.6117f,0.6240f},c});
    v.push_back({{0.30971f,-0.05396f,-0.06105f},{-0.3634f,-0.0000f,-0.9316f},{0.6706f,0.2829f},c});
    v.push_back({{0.34447f,-0.11502f,-0.04317f},{-0.2894f,-0.6653f,-0.6882f},{0.5431f,0.6392f},c});
    v.push_back({{0.32643f,-0.09714f,-0.05287f},{-0.2894f,-0.6653f,-0.6882f},{0.5658f,0.5973f},c});
    v.push_back({{0.35469f,-0.09714f,-0.06476f},{-0.2894f,-0.6653f,-0.6882f},{0.5908f,0.6481f},c});
    v.push_back({{0.34489f,0.05396f,-0.07478f},{-0.2418f,-0.0000f,-0.9703f},{0.7221f,0.0808f},c});
    v.push_back({{0.36531f,-0.12874f,-0.03053f},{-0.2316f,-0.8257f,-0.5143f},{0.5171f,0.6839f},c});
    v.push_back({{0.34447f,-0.11502f,-0.04317f},{-0.2316f,-0.8257f,-0.5143f},{0.5431f,0.6392f},c});
    v.push_back({{0.36602f,-0.11502f,-0.05287f},{-0.2316f,-0.8257f,-0.5143f},{0.5634f,0.6779f},c});
    v.push_back({{0.36602f,-0.11502f,-0.05287f},{-0.1917f,-0.6245f,-0.7571f},{0.5634f,0.6779f},c});
    v.push_back({{0.35469f,-0.09714f,-0.06476f},{-0.1917f,-0.6245f,-0.7571f},{0.5908f,0.6481f},c});
    v.push_back({{0.38419f,-0.09714f,-0.07223f},{-0.1917f,-0.6245f,-0.7571f},{0.6078f,0.7010f},c});
    v.push_back({{0.34489f,0.05396f,0.07478f},{-0.3634f,0.0000f,0.9316f},{0.1298f,0.1143f},c});
    v.push_back({{0.38038f,-0.12874f,-0.03739f},{-0.1482f,-0.8044f,-0.5753f},{0.5314f,0.7109f},c});
    v.push_back({{0.36602f,-0.11502f,-0.05287f},{-0.1482f,-0.8044f,-0.5753f},{0.5634f,0.6779f},c});
    v.push_back({{0.38969f,-0.11502f,-0.05897f},{-0.1482f,-0.8044f,-0.5753f},{0.5773f,0.7204f},c});
    v.push_back({{0.34741f,-0.07631f,-0.07223f},{-0.2395f,-0.1366f,-0.9612f},{0.7282f,0.3247f},c});
    v.push_back({{0.34489f,-0.05396f,-0.07478f},{-0.2395f,-0.1366f,-0.9612f},{0.7221f,0.2829f},c});
    v.push_back({{0.37951f,-0.05396f,-0.08340f},{-0.2395f,-0.1366f,-0.9612f},{0.7762f,0.2828f},c});
    v.push_back({{0.37951f,0.05396f,0.08340f},{-0.2418f,0.0000f,0.9703f},{0.0631f,0.1143f},c});
    v.push_back({{0.39664f,-0.13737f,-0.01935f},{-0.0943f,-0.9285f,-0.3591f},{0.4969f,0.7447f},c});
    v.push_back({{0.38038f,-0.12874f,-0.03739f},{-0.0943f,-0.9285f,-0.3591f},{0.5314f,0.7109f},c});
    v.push_back({{0.39680f,-0.12874f,-0.04170f},{-0.0943f,-0.9285f,-0.3591f},{0.5412f,0.7404f},c});
    v.push_back({{0.37951f,0.05396f,-0.08340f},{-0.0858f,-0.0000f,-0.9963f},{0.7762f,0.0808f},c});
    v.push_back({{0.35469f,-0.09714f,-0.06476f},{-0.2228f,-0.3973f,-0.8902f},{0.7461f,0.3637f},c});
    v.push_back({{0.34741f,-0.07631f,-0.07223f},{-0.2228f,-0.3973f,-0.8902f},{0.7282f,0.3247f},c});
    v.push_back({{0.38070f,-0.07631f,-0.08056f},{-0.2228f,-0.3973f,-0.8902f},{0.7802f,0.3247f},c});
    v.push_back({{0.41366f,0.05396f,0.08634f},{-0.0858f,0.0000f,0.9963f},{0.0000f,0.1143f},c});
    v.push_back({{0.38419f,-0.09714f,-0.07223f},{-0.0794f,-0.3817f,-0.9209f},{0.7921f,0.3637f},c});
    v.push_back({{0.38070f,-0.07631f,-0.08056f},{-0.0794f,-0.3817f,-0.9209f},{0.7802f,0.3247f},c});
    v.push_back({{0.41366f,-0.07631f,-0.08340f},{-0.0794f,-0.3817f,-0.9209f},{0.8355f,0.3247f},c});
    v.push_back({{0.26602f,0.05396f,0.04317f},{-0.6856f,-0.0000f,0.7280f},{0.2879f,0.1143f},c});
    v.push_back({{0.38969f,-0.11502f,-0.05897f},{-0.0684f,-0.6076f,-0.7913f},{0.5773f,0.7204f},c});
    v.push_back({{0.38419f,-0.09714f,-0.07223f},{-0.0684f,-0.6076f,-0.7913f},{0.6078f,0.7010f},c});
    v.push_back({{0.41366f,-0.09714f,-0.07478f},{-0.0684f,-0.6076f,-0.7913f},{0.6159f,0.7539f},c});
    v.push_back({{0.24097f,0.05396f,-0.00000f},{-0.9914f,-0.0000f,-0.1305f},{0.8833f,0.0463f},c});
    v.push_back({{0.38969f,-0.11502f,-0.05897f},{-0.0528f,-0.7924f,-0.6078f},{0.5773f,0.7204f},c});
    v.push_back({{0.37951f,-0.05396f,-0.08340f},{-0.0851f,-0.1302f,-0.9878f},{0.7762f,0.2828f},c});
    v.push_back({{0.24391f,0.05396f,0.02235f},{-0.9914f,-0.0000f,0.1305f},{0.8415f,0.0459f},c});
    v.push_back({{0.40498f,-0.13737f,-0.02159f},{-0.0334f,-0.9234f,-0.3824f},{0.5020f,0.7597f},c});
    v.push_back({{0.39680f,-0.12874f,-0.04170f},{-0.0334f,-0.9234f,-0.3824f},{0.5412f,0.7404f},c});
    v.push_back({{0.41366f,-0.12874f,-0.04317f},{-0.0334f,-0.9234f,-0.3824f},{0.5458f,0.7707f},c});
    v.push_back({{0.41366f,0.09714f,0.07478f},{-0.0687f,0.6073f,0.7915f},{0.0058f,0.0335f},c});
    v.push_back({{0.41366f,0.11502f,0.06105f},{-0.0687f,0.6073f,0.7915f},{0.0126f,0.0000f},c});
    v.push_back({{0.38969f,0.11502f,0.05897f},{-0.0687f,0.6073f,0.7915f},{0.0569f,0.0000f},c});
    v.push_back({{0.41366f,0.07631f,0.08340f},{-0.0796f,0.3815f,0.9209f},{0.0015f,0.0724f},c});
    v.push_back({{0.41366f,0.09714f,0.07478f},{-0.0796f,0.3815f,0.9209f},{0.0058f,0.0335f},c});
    v.push_back({{0.38419f,0.09714f,0.07223f},{-0.0796f,0.3815f,0.9209f},{0.0602f,0.0335f},c});
    v.push_back({{0.41366f,0.12874f,0.04317f},{-0.0336f,0.9234f,0.3825f},{0.0270f,0.4349f},c});
    v.push_back({{0.41366f,0.13737f,0.02235f},{-0.0336f,0.9234f,0.3825f},{0.0615f,0.4306f},c});
    v.push_back({{0.40498f,0.13737f,0.02159f},{-0.0336f,0.9234f,0.3825f},{0.0619f,0.4463f},c});
    v.push_back({{0.41366f,0.07631f,0.08340f},{-0.0852f,0.1301f,0.9878f},{0.0015f,0.0724f},c});
    v.push_back({{0.41366f,0.12874f,0.04317f},{-0.0530f,0.7922f,0.6079f},{0.0270f,0.4349f},c});
    v.push_back({{0.38969f,0.11502f,0.05897f},{-0.1513f,0.8034f,0.5760f},{0.0011f,0.4849f},c});
    v.push_back({{0.39680f,0.12874f,0.04170f},{-0.1513f,0.8034f,0.5760f},{0.0278f,0.4653f},c});
    v.push_back({{0.38038f,0.12874f,0.03739f},{-0.1513f,0.8034f,0.5760f},{0.0338f,0.4949f},c});
    v.push_back({{0.38419f,0.09714f,0.07223f},{-0.1953f,0.6221f,0.7582f},{0.0602f,0.0335f},c});
    v.push_back({{0.38969f,0.11502f,0.05897f},{-0.1953f,0.6221f,0.7582f},{0.0569f,0.0000f},c});
    v.push_back({{0.36602f,0.11502f,0.05287f},{-0.1953f,0.6221f,0.7582f},{0.1026f,-0.0000f},c});
    v.push_back({{0.38070f,0.07631f,0.08056f},{-0.2256f,0.3942f,0.8909f},{0.0623f,0.0724f},c});
    v.push_back({{0.38419f,0.09714f,0.07223f},{-0.2256f,0.3942f,0.8909f},{0.0602f,0.0335f},c});
    v.push_back({{0.35469f,0.09714f,0.06476f},{-0.2256f,0.3942f,0.8909f},{0.1172f,0.0335f},c});
    v.push_back({{0.39680f,0.12874f,0.04170f},{-0.0961f,0.9284f,0.3590f},{0.0278f,0.4653f},c});
    v.push_back({{0.40498f,0.13737f,0.02159f},{-0.0961f,0.9284f,0.3590f},{0.0619f,0.4463f},c});
    v.push_back({{0.39664f,0.13737f,0.01935f},{-0.0961f,0.9284f,0.3590f},{0.0650f,0.4613f},c});
    v.push_back({{0.37951f,0.05396f,0.08340f},{-0.2405f,0.1351f,0.9612f},{0.0631f,0.1143f},c});
    v.push_back({{0.38070f,0.07631f,0.08056f},{-0.2405f,0.1351f,0.9612f},{0.0623f,0.0724f},c});
    v.push_back({{0.34741f,0.07631f,0.07223f},{-0.2405f,0.1351f,0.9612f},{0.1266f,0.0724f},c});
    v.push_back({{0.34741f,0.07631f,0.07223f},{-0.3513f,0.4226f,0.8355f},{0.1266f,0.0724f},c});
    v.push_back({{0.35469f,0.09714f,0.06476f},{-0.3513f,0.4226f,0.8355f},{0.1172f,0.0335f},c});
    v.push_back({{0.32643f,0.09714f,0.05287f},{-0.3513f,0.4226f,0.8355f},{0.1741f,0.0335f},c});
    v.push_back({{0.38038f,0.12874f,0.03739f},{-0.1477f,0.9375f,0.3152f},{0.0338f,0.4949f},c});
    v.push_back({{0.39664f,0.13737f,0.01935f},{-0.1477f,0.9375f,0.3152f},{0.0650f,0.4613f},c});
    v.push_back({{0.38906f,0.13737f,0.01580f},{-0.1477f,0.9375f,0.3152f},{0.0707f,0.4750f},c});
    v.push_back({{0.34489f,0.05396f,0.07478f},{-0.3641f,0.1459f,0.9198f},{0.1298f,0.1143f},c});
    v.push_back({{0.34741f,0.07631f,0.07223f},{-0.3641f,0.1459f,0.9198f},{0.1266f,0.0724f},c});
    v.push_back({{0.31393f,0.07631f,0.05897f},{-0.3641f,0.1459f,0.9198f},{0.1936f,0.0724f},c});
    v.push_back({{0.36602f,0.11502f,0.05287f},{-0.2341f,0.8252f,0.5141f},{0.0096f,0.5277f},c});
    v.push_back({{0.38038f,0.12874f,0.03739f},{-0.2341f,0.8252f,0.5141f},{0.0338f,0.4949f},c});
    v.push_back({{0.36531f,0.12874f,0.03053f},{-0.2341f,0.8252f,0.5141f},{0.0447f,0.5221f},c});
    v.push_back({{0.35469f,0.09714f,0.06476f},{-0.3104f,0.6546f,0.6893f},{0.1172f,0.0335f},c});
    v.push_back({{0.36602f,0.11502f,0.05287f},{-0.3104f,0.6546f,0.6893f},{0.1026f,-0.0000f},c});
    v.push_back({{0.34447f,0.11502f,0.04317f},{-0.3104f,0.6546f,0.6893f},{0.1464f,0.0000f},c});
    v.push_back({{0.32643f,0.09714f,0.05287f},{-0.4046f,0.7165f,0.5683f},{0.1741f,0.0335f},c});
    v.push_back({{0.34447f,0.11502f,0.04317f},{-0.4046f,0.7165f,0.5683f},{0.1464f,0.0000f},c});
    v.push_back({{0.32671f,0.11502f,0.03053f},{-0.4046f,0.7165f,0.5683f},{0.1847f,0.0000f},c});
    v.push_back({{0.31393f,0.07631f,0.05897f},{-0.4498f,0.4890f,0.7473f},{0.1936f,0.0724f},c});
    v.push_back({{0.32643f,0.09714f,0.05287f},{-0.4498f,0.4890f,0.7473f},{0.1741f,0.0335f},c});
    v.push_back({{0.30070f,0.09714f,0.03739f},{-0.4498f,0.4890f,0.7473f},{0.2282f,0.0335f},c});
    v.push_back({{0.36531f,0.12874f,0.03053f},{-0.1847f,0.9484f,0.2576f},{0.0447f,0.5221f},c});
    v.push_back({{0.38906f,0.13737f,0.01580f},{-0.1847f,0.9484f,0.2576f},{0.0707f,0.4750f},c});
    v.push_back({{0.38261f,0.13737f,0.01117f},{-0.1847f,0.9484f,0.2576f},{0.0784f,0.4866f},c});
    v.push_back({{0.30971f,0.05396f,0.06105f},{-0.4300f,0.1637f,0.8879f},{0.2001f,0.1143f},c});
    v.push_back({{0.31393f,0.07631f,0.05897f},{-0.4300f,0.1637f,0.8879f},{0.1936f,0.0724f},c});
    v.push_back({{0.27826f,0.07631f,0.04170f},{-0.4300f,0.1637f,0.8879f},{0.2665f,0.0724f},c});
    v.push_back({{0.34447f,0.11502f,0.04317f},{-0.3320f,0.8605f,0.3864f},{0.0249f,0.5666f},c});
    v.push_back({{0.36531f,0.12874f,0.03053f},{-0.3320f,0.8605f,0.3864f},{0.0447f,0.5221f},c});
    v.push_back({{0.35490f,0.12874f,0.02159f},{-0.3320f,0.8605f,0.3864f},{0.0598f,0.5409f},c});
    v.push_back({{0.26602f,0.05396f,0.04317f},{-0.5078f,0.3305f,0.7955f},{0.2879f,0.1143f},c});
    v.push_back({{0.27826f,0.07631f,0.04170f},{-0.5078f,0.3305f,0.7955f},{0.2665f,0.0724f},c});
    v.push_back({{0.24675f,0.07631f,0.02159f},{-0.5078f,0.3305f,0.7955f},{0.3334f,0.0725f},c});
    v.push_back({{0.32671f,0.11502f,0.03053f},{-0.3501f,0.8964f,0.2717f},{0.0459f,0.5986f},c});
    v.push_back({{0.35490f,0.12874f,0.02159f},{-0.3501f,0.8964f,0.2717f},{0.0598f,0.5409f},c});
    v.push_back({{0.34682f,0.12874f,0.01117f},{-0.3501f,0.8964f,0.2717f},{0.0779f,0.5555f},c});
    v.push_back({{0.30070f,0.09714f,0.03739f},{-0.4534f,0.8056f,0.3814f},{0.0378f,0.6544f},c});
    v.push_back({{0.32671f,0.11502f,0.03053f},{-0.4534f,0.8056f,0.3814f},{0.0459f,0.5986f},c});
    v.push_back({{0.31432f,0.11502f,0.01580f},{-0.4534f,0.8056f,0.3814f},{0.0714f,0.6210f},c});
    v.push_back({{0.27826f,0.07631f,0.04170f},{-0.4765f,0.6386f,0.6042f},{0.2665f,0.0724f},c});
    v.push_back({{0.30070f,0.09714f,0.03739f},{-0.4765f,0.6386f,0.6042f},{0.2282f,0.0335f},c});
    v.push_back({{0.27783f,0.09714f,0.01935f},{-0.4765f,0.6386f,0.6042f},{0.2785f,0.0335f},c});
    v.push_back({{0.35490f,0.12874f,0.02159f},{-0.2199f,0.9537f,0.2051f},{0.0598f,0.5409f},c});
    v.push_back({{0.38261f,0.13737f,0.01117f},{-0.2199f,0.9537f,0.2051f},{0.0784f,0.4866f},c});
    v.push_back({{0.37758f,0.13737f,0.00578f},{-0.2199f,0.9537f,0.2051f},{0.0876f,0.4957f},c});
    v.push_back({{0.24675f,0.07631f,0.02159f},{-0.4791f,0.7618f,0.4361f},{0.0690f,0.7621f},c});
    v.push_back({{0.27783f,0.09714f,0.01935f},{-0.4791f,0.7618f,0.4361f},{0.0681f,0.6957f},c});
    v.push_back({{0.26022f,0.09714f,-0.00000f},{-0.4791f,0.7618f,0.4361f},{0.1014f,0.7275f},c});
    v.push_back({{0.34682f,0.12874f,0.01117f},{-0.2440f,0.9592f,0.1428f},{0.0779f,0.5555f},c});
    v.push_back({{0.37758f,0.13737f,0.00578f},{-0.2440f,0.9592f,0.1428f},{0.0876f,0.4957f},c});
    v.push_back({{0.37420f,0.13737f,0.00000f},{-0.2440f,0.9592f,0.1428f},{0.0978f,0.5018f},c});
    v.push_back({{0.24675f,0.07631f,0.02159f},{-0.9831f,0.1294f,0.1294f},{0.8429f,0.0039f},c});
    v.push_back({{0.31432f,0.11502f,0.01580f},{-0.3625f,0.9162f,0.1709f},{0.0714f,0.6210f},c});
    v.push_back({{0.34682f,0.12874f,0.01117f},{-0.3625f,0.9162f,0.1709f},{0.0779f,0.5555f},c});
    v.push_back({{0.34155f,0.12874f,0.00000f},{-0.3625f,0.9162f,0.1709f},{0.0977f,0.5650f},c});
    v.push_back({{0.27783f,0.09714f,0.01935f},{-0.4168f,0.8885f,0.1917f},{0.0681f,0.6957f},c});
    v.push_back({{0.31432f,0.11502f,0.01580f},{-0.4168f,0.8885f,0.1917f},{0.0714f,0.6210f},c});
    v.push_back({{0.30705f,0.11502f,0.00000f},{-0.4168f,0.8885f,0.1917f},{0.0994f,0.6341f},c});
    v.push_back({{0.30705f,0.11502f,0.00000f},{-0.4168f,0.8885f,-0.1917f},{0.0994f,0.6341f},c});
    v.push_back({{0.31432f,0.11502f,-0.01580f},{-0.4168f,0.8885f,-0.1917f},{0.1290f,0.6210f},c});
    v.push_back({{0.27783f,0.09714f,-0.01935f},{-0.4168f,0.8885f,-0.1917f},{0.1387f,0.6957f},c});
    v.push_back({{0.26022f,0.09714f,-0.00000f},{-0.4791f,0.7618f,-0.4361f},{0.1014f,0.7275f},c});
    v.push_back({{0.27783f,0.09714f,-0.01935f},{-0.4791f,0.7618f,-0.4361f},{0.1387f,0.6957f},c});
    v.push_back({{0.24675f,0.07631f,-0.02159f},{-0.4791f,0.7618f,-0.4361f},{0.1478f,0.7621f},c});
    v.push_back({{0.37420f,0.13737f,0.00000f},{-0.2440f,0.9592f,-0.1428f},{0.0978f,0.5018f},c});
    v.push_back({{0.37758f,0.13737f,-0.00578f},{-0.2440f,0.9592f,-0.1428f},{0.1087f,0.4957f},c});
    v.push_back({{0.34682f,0.12874f,-0.01117f},{-0.2440f,0.9592f,-0.1428f},{0.1186f,0.5555f},c});
    v.push_back({{0.24391f,0.07631f,-0.00000f},{-0.9831f,0.1294f,-0.1294f},{0.8833f,0.0042f},c});
    v.push_back({{0.34155f,0.12874f,0.00000f},{-0.3625f,0.9162f,-0.1709f},{0.0977f,0.5650f},c});
    v.push_back({{0.34682f,0.12874f,-0.01117f},{-0.3625f,0.9162f,-0.1709f},{0.1186f,0.5555f},c});
    v.push_back({{0.31432f,0.11502f,-0.01580f},{-0.3625f,0.9162f,-0.1709f},{0.1290f,0.6210f},c});
    v.push_back({{0.24675f,0.07631f,-0.02159f},{-0.5078f,0.3305f,-0.7955f},{0.9237f,0.0039f},c});
    v.push_back({{0.27826f,0.07631f,-0.04170f},{-0.5078f,0.3305f,-0.7955f},{0.9614f,-0.0000f},c});
    v.push_back({{0.26602f,0.05396f,-0.04317f},{-0.5078f,0.3305f,-0.7955f},{0.9641f,0.0432f},c});
    v.push_back({{0.34682f,0.12874f,-0.01117f},{-0.3501f,0.8964f,-0.2717f},{0.1186f,0.5555f},c});
    v.push_back({{0.35490f,0.12874f,-0.02159f},{-0.3501f,0.8964f,-0.2717f},{0.1385f,0.5409f},c});
    v.push_back({{0.32671f,0.11502f,-0.03053f},{-0.3501f,0.8964f,-0.2717f},{0.1573f,0.5986f},c});
    v.push_back({{0.31432f,0.11502f,-0.01580f},{-0.4534f,0.8056f,-0.3814f},{0.1290f,0.6210f},c});
    v.push_back({{0.32671f,0.11502f,-0.03053f},{-0.4534f,0.8056f,-0.3814f},{0.1573f,0.5986f},c});
    v.push_back({{0.30070f,0.09714f,-0.03739f},{-0.4534f,0.8056f,-0.3814f},{0.1741f,0.6544f},c});
    v.push_back({{0.27783f,0.09714f,-0.01935f},{-0.4765f,0.6386f,-0.6042f},{0.1387f,0.6957f},c});
    v.push_back({{0.30070f,0.09714f,-0.03739f},{-0.4765f,0.6386f,-0.6042f},{0.1741f,0.6544f},c});
    v.push_back({{0.27826f,0.07631f,-0.04170f},{-0.4765f,0.6386f,-0.6042f},{0.1880f,0.7053f},c});
    v.push_back({{0.37758f,0.13737f,-0.00578f},{-0.2199f,0.9537f,-0.2051f},{0.1087f,0.4957f},c});
    v.push_back({{0.38261f,0.13737f,-0.01117f},{-0.2199f,0.9537f,-0.2051f},{0.1191f,0.4866f},c});
    v.push_back({{0.35490f,0.12874f,-0.02159f},{-0.2199f,0.9537f,-0.2051f},{0.1385f,0.5409f},c});
    v.push_back({{0.30070f,0.09714f,-0.03739f},{-0.4498f,0.4890f,-0.7473f},{0.1741f,0.6544f},c});
    v.push_back({{0.32643f,0.09714f,-0.05287f},{-0.4498f,0.4890f,-0.7473f},{0.2053f,0.6080f},c});
    v.push_back({{0.31393f,0.07631f,-0.05897f},{-0.4498f,0.4890f,-0.7473f},{0.2235f,0.6409f},c});
    v.push_back({{0.38261f,0.13737f,-0.01117f},{-0.1847f,0.9484f,-0.2576f},{0.1191f,0.4866f},c});
    v.push_back({{0.38906f,0.13737f,-0.01580f},{-0.1847f,0.9484f,-0.2576f},{0.1283f,0.4750f},c});
    v.push_back({{0.36531f,0.12874f,-0.03053f},{-0.1847f,0.9484f,-0.2576f},{0.1560f,0.5221f},c});
    v.push_back({{0.27826f,0.07631f,-0.04170f},{-0.4300f,0.1637f,-0.8879f},{0.6295f,0.0391f},c});
    v.push_back({{0.31393f,0.07631f,-0.05897f},{-0.4300f,0.1637f,-0.8879f},{0.6793f,0.0390f},c});
    v.push_back({{0.30971f,0.05396f,-0.06105f},{-0.4300f,0.1637f,-0.8879f},{0.6706f,0.0809f},c});
    v.push_back({{0.35490f,0.12874f,-0.02159f},{-0.3320f,0.8605f,-0.3864f},{0.1385f,0.5409f},c});
    v.push_back({{0.36531f,0.12874f,-0.03053f},{-0.3320f,0.8605f,-0.3864f},{0.1560f,0.5221f},c});
    v.push_back({{0.34447f,0.11502f,-0.04317f},{-0.3320f,0.8605f,-0.3864f},{0.1823f,0.5666f},c});
    v.push_back({{0.32671f,0.11502f,-0.03053f},{-0.4046f,0.7165f,-0.5683f},{0.1573f,0.5986f},c});
    v.push_back({{0.34447f,0.11502f,-0.04317f},{-0.4046f,0.7165f,-0.5683f},{0.1823f,0.5666f},c});
    v.push_back({{0.32643f,0.09714f,-0.05287f},{-0.4046f,0.7165f,-0.5683f},{0.2053f,0.6080f},c});
    v.push_back({{0.34447f,0.11502f,-0.04317f},{-0.3104f,0.6546f,-0.6893f},{0.1823f,0.5666f},c});
    v.push_back({{0.36602f,0.11502f,-0.05287f},{-0.3104f,0.6546f,-0.6893f},{0.2024f,0.5277f},c});
    v.push_back({{0.35469f,0.09714f,-0.06476f},{-0.3104f,0.6546f,-0.6893f},{0.2301f,0.5570f},c});
    v.push_back({{0.32643f,0.09714f,-0.05287f},{-0.3513f,0.4226f,-0.8355f},{0.2053f,0.6080f},c});
    v.push_back({{0.35469f,0.09714f,-0.06476f},{-0.3513f,0.4226f,-0.8355f},{0.2301f,0.5570f},c});
    v.push_back({{0.34741f,0.07631f,-0.07223f},{-0.3513f,0.4226f,-0.8355f},{0.2514f,0.5805f},c});
    v.push_back({{0.38906f,0.13737f,-0.01580f},{-0.1477f,0.9375f,-0.3152f},{0.1283f,0.4750f},c});
    v.push_back({{0.39664f,0.13737f,-0.01935f},{-0.1477f,0.9375f,-0.3152f},{0.1356f,0.4613f},c});
    v.push_back({{0.38038f,0.12874f,-0.03739f},{-0.1477f,0.9375f,-0.3152f},{0.1702f,0.4949f},c});
    v.push_back({{0.31393f,0.07631f,-0.05897f},{-0.3641f,0.1459f,-0.9198f},{0.6793f,0.0390f},c});
    v.push_back({{0.34741f,0.07631f,-0.07223f},{-0.3641f,0.1459f,-0.9198f},{0.7282f,0.0390f},c});
    v.push_back({{0.34489f,0.05396f,-0.07478f},{-0.3641f,0.1459f,-0.9198f},{0.7221f,0.0808f},c});
    v.push_back({{0.36531f,0.12874f,-0.03053f},{-0.2341f,0.8252f,-0.5141f},{0.1560f,0.5221f},c});
    v.push_back({{0.38038f,0.12874f,-0.03739f},{-0.2341f,0.8252f,-0.5141f},{0.1702f,0.4949f},c});
    v.push_back({{0.36602f,0.11502f,-0.05287f},{-0.2341f,0.8252f,-0.5141f},{0.2024f,0.5277f},c});
    v.push_back({{0.34741f,0.07631f,-0.07223f},{-0.2405f,0.1351f,-0.9612f},{0.7282f,0.0390f},c});
    v.push_back({{0.38070f,0.07631f,-0.08056f},{-0.2405f,0.1351f,-0.9612f},{0.7802f,0.0390f},c});
    v.push_back({{0.37951f,0.05396f,-0.08340f},{-0.2405f,0.1351f,-0.9612f},{0.7762f,0.0808f},c});
    v.push_back({{0.38038f,0.12874f,-0.03739f},{-0.1513f,0.8034f,-0.5760f},{0.1702f,0.4949f},c});
    v.push_back({{0.39680f,0.12874f,-0.04170f},{-0.1513f,0.8034f,-0.5760f},{0.1799f,0.4653f},c});
    v.push_back({{0.38969f,0.11502f,-0.05897f},{-0.1513f,0.8034f,-0.5760f},{0.2162f,0.4849f},c});
    v.push_back({{0.36602f,0.11502f,-0.05287f},{-0.1953f,0.6221f,-0.7582f},{0.2024f,0.5277f},c});
    v.push_back({{0.38969f,0.11502f,-0.05897f},{-0.1953f,0.6221f,-0.7582f},{0.2162f,0.4849f},c});
    v.push_back({{0.38419f,0.09714f,-0.07223f},{-0.1953f,0.6221f,-0.7582f},{0.2470f,0.5037f},c});
    v.push_back({{0.35469f,0.09714f,-0.06476f},{-0.2256f,0.3942f,-0.8909f},{0.7461f,0.0000f},c});
    v.push_back({{0.38419f,0.09714f,-0.07223f},{-0.2256f,0.3942f,-0.8909f},{0.7921f,0.0000f},c});
    v.push_back({{0.38070f,0.07631f,-0.08056f},{-0.2256f,0.3942f,-0.8909f},{0.7802f,0.0390f},c});
    v.push_back({{0.39664f,0.13737f,-0.01935f},{-0.0961f,0.9284f,-0.3590f},{0.1356f,0.4613f},c});
    v.push_back({{0.40498f,0.13737f,-0.02159f},{-0.0961f,0.9284f,-0.3590f},{0.1406f,0.4463f},c});
    v.push_back({{0.39680f,0.12874f,-0.04170f},{-0.0961f,0.9284f,-0.3590f},{0.1799f,0.4653f},c});
    v.push_back({{0.38419f,0.09714f,-0.07223f},{-0.0796f,0.3815f,-0.9209f},{0.7921f,0.0000f},c});
    v.push_back({{0.41366f,0.09714f,-0.07478f},{-0.0796f,0.3815f,-0.9209f},{0.8415f,0.0000f},c});
    v.push_back({{0.41366f,0.07631f,-0.08340f},{-0.0796f,0.3815f,-0.9209f},{0.8355f,0.0390f},c});
    v.push_back({{0.40498f,0.13737f,-0.02159f},{-0.0336f,0.9234f,-0.3825f},{0.1406f,0.4463f},c});
    v.push_back({{0.41366f,0.13737f,-0.02235f},{-0.0336f,0.9234f,-0.3825f},{0.1430f,0.4306f},c});
    v.push_back({{0.41366f,0.12874f,-0.04317f},{-0.0336f,0.9234f,-0.3825f},{0.1845f,0.4349f},c});
    v.push_back({{0.41366f,0.07631f,-0.08340f},{-0.0852f,0.1301f,-0.9878f},{0.8355f,0.0390f},c});
    v.push_back({{0.41366f,0.12874f,-0.04317f},{-0.0530f,0.7922f,-0.6079f},{0.1845f,0.4349f},c});
    v.push_back({{0.38969f,0.11502f,-0.05897f},{-0.0687f,0.6073f,-0.7915f},{0.2162f,0.4849f},c});
    v.push_back({{0.41366f,0.11502f,-0.06105f},{-0.0687f,0.6073f,-0.7915f},{0.2227f,0.4417f},c});
    v.push_back({{0.41366f,0.09714f,-0.07478f},{-0.0687f,0.6073f,-0.7915f},{0.2550f,0.4506f},c});
    v.push_back({{0.43301f,-0.09714f,-0.07223f},{0.1039f,-0.6055f,-0.7891f},{0.8835f,0.8059f},c});
    v.push_back({{0.42946f,-0.11502f,-0.05897f},{0.0799f,-0.7908f,-0.6068f},{0.1214f,0.7734f},c});
    v.push_back({{0.43600f,-0.05396f,-0.08340f},{0.1294f,-0.1294f,-0.9831f},{0.8809f,0.7251f},c});
    v.push_back({{0.42483f,-0.12874f,-0.04170f},{0.0503f,-0.9227f,-0.3822f},{0.1149f,0.8103f},c});
    v.push_back({{0.41366f,-0.07631f,-0.08340f},{0.1207f,-0.3799f,-0.9171f},{0.8420f,0.7670f},c});
    v.push_back({{0.45683f,-0.05396f,-0.07478f},{0.3795f,-0.1294f,-0.9161f},{0.9230f,0.7251f},c});
    v.push_back({{0.43524f,-0.12874f,-0.03739f},{0.1475f,-0.9227f,-0.3562f},{0.1324f,0.8196f},c});
    v.push_back({{0.45536f,-0.07631f,-0.07223f},{0.3540f,-0.3799f,-0.8546f},{0.9223f,0.7670f},c});
    v.push_back({{0.45104f,-0.09714f,-0.06476f},{0.3046f,-0.6055f,-0.7353f},{0.9200f,0.8059f},c});
    v.push_back({{0.42946f,-0.11502f,-0.05897f},{0.2342f,-0.7908f,-0.5655f},{0.1214f,0.7734f},c});
    v.push_back({{0.45683f,-0.11502f,-0.04317f},{0.3726f,-0.7908f,-0.4856f},{0.6408f,0.6415f},c});
    v.push_back({{0.47471f,-0.05396f,-0.06105f},{0.6036f,-0.1294f,-0.7867f},{0.9637f,0.7251f},c});
    v.push_back({{0.44418f,-0.12874f,-0.03053f},{0.2347f,-0.9227f,-0.3058f},{0.1466f,0.8334f},c});
    v.push_back({{0.45536f,-0.07631f,-0.07223f},{0.5631f,-0.3799f,-0.7339f},{0.9223f,0.7670f},c});
    v.push_back({{0.46653f,-0.09714f,-0.05287f},{0.4845f,-0.6055f,-0.6314f},{0.6490f,0.6004f},c});
    v.push_back({{0.48588f,-0.07631f,-0.04170f},{0.7339f,-0.3799f,-0.5631f},{0.6983f,0.5715f},c});
    v.push_back({{0.47841f,-0.09714f,-0.03739f},{0.6314f,-0.6055f,-0.4845f},{0.6817f,0.6081f},c});
    v.push_back({{0.45683f,-0.11502f,-0.04317f},{0.4856f,-0.7908f,-0.3726f},{0.6408f,0.6415f},c});
    v.push_back({{0.48843f,-0.05396f,-0.04317f},{0.7867f,-0.1294f,-0.6036f},{1.0000f,0.7251f},c});
    v.push_back({{0.45104f,-0.12874f,-0.02159f},{0.3058f,-0.9227f,-0.2347f},{0.6567f,0.6878f},c});
    v.push_back({{0.49706f,-0.05396f,-0.02235f},{0.9161f,-0.1294f,-0.3795f},{0.7540f,0.5570f},c});
    v.push_back({{0.45104f,-0.12874f,-0.02159f},{0.3562f,-0.9227f,-0.1475f},{0.6567f,0.6878f},c});
    v.push_back({{0.49422f,-0.07631f,-0.02159f},{0.8546f,-0.3799f,-0.3540f},{0.7349f,0.5875f},c});
    v.push_back({{0.48588f,-0.09714f,-0.01935f},{0.7353f,-0.6055f,-0.3046f},{0.7145f,0.6225f},c});
    v.push_back({{0.46653f,-0.11502f,-0.03053f},{0.5655f,-0.7908f,-0.2342f},{0.6676f,0.6478f},c});
    v.push_back({{0.48843f,-0.09714f,0.00000f},{0.7891f,-0.6055f,-0.1039f},{0.7451f,0.6425f},c});
    v.push_back({{0.47263f,-0.11502f,-0.01580f},{0.6068f,-0.7908f,-0.0799f},{0.6943f,0.6595f},c});
    v.push_back({{0.50000f,-0.05396f,0.00000f},{0.9831f,-0.1294f,-0.1294f},{0.7893f,0.5801f},c});
    v.push_back({{0.45683f,-0.12874f,0.00000f},{0.3822f,-0.9227f,-0.0503f},{0.6933f,0.7076f},c});
    v.push_back({{0.49422f,-0.07631f,-0.02159f},{0.9171f,-0.3799f,-0.1207f},{0.7349f,0.5875f},c});
    v.push_back({{0.49706f,-0.07631f,0.00000f},{0.9171f,-0.3799f,0.1207f},{0.7690f,0.6098f},c});
    v.push_back({{0.48588f,-0.09714f,0.01935f},{0.7891f,-0.6055f,0.1039f},{0.7714f,0.6667f},c});
    v.push_back({{0.47263f,-0.11502f,0.01580f},{0.6068f,-0.7908f,0.0799f},{0.7408f,0.6956f},c});
    v.push_back({{0.49706f,-0.05396f,0.02235f},{0.9831f,-0.1294f,0.1294f},{0.8196f,0.6081f},c});
    v.push_back({{0.45536f,-0.12874f,0.01117f},{0.3822f,-0.9227f,0.0503f},{0.7085f,0.7216f},c});
    v.push_back({{0.46653f,-0.11502f,0.03053f},{0.5655f,-0.7908f,0.2342f},{0.7573f,0.7176f},c});
    v.push_back({{0.48843f,-0.05396f,0.04317f},{0.9161f,-0.1294f,0.3795f},{0.5318f,0.3686f},c});
    v.push_back({{0.45104f,-0.12874f,0.02159f},{0.3562f,-0.9227f,0.1475f},{0.7202f,0.7372f},c});
    v.push_back({{0.48588f,-0.07631f,0.04170f},{0.8546f,-0.3799f,0.3540f},{0.8208f,0.6669f},c});
    v.push_back({{0.47841f,-0.09714f,0.03739f},{0.7353f,-0.6055f,0.3046f},{0.7916f,0.6937f},c});
    v.push_back({{0.46653f,-0.09714f,0.05287f},{0.6314f,-0.6055f,0.4845f},{0.8044f,0.7214f},c});
    v.push_back({{0.45683f,-0.11502f,0.04317f},{0.4856f,-0.7908f,0.3726f},{0.7677f,0.7403f},c});
    v.push_back({{0.47471f,-0.05396f,0.06105f},{0.7867f,-0.1294f,0.6036f},{0.5717f,0.3597f},c});
    v.push_back({{0.44418f,-0.12874f,0.03053f},{0.3058f,-0.9227f,0.2347f},{0.7275f,0.7532f},c});
    v.push_back({{0.48588f,-0.07631f,0.04170f},{0.7339f,-0.3799f,0.5631f},{0.8208f,0.6669f},c});
    v.push_back({{0.43524f,-0.12874f,0.03739f},{0.2347f,-0.9227f,0.3058f},{0.1870f,0.7930f},c});
    v.push_back({{0.47263f,-0.07631f,0.05897f},{0.5631f,-0.3799f,0.7339f},{0.8351f,0.6978f},c});
    v.push_back({{0.46653f,-0.09714f,0.05287f},{0.4845f,-0.6055f,0.6314f},{0.8044f,0.7214f},c});
    v.push_back({{0.45683f,-0.11502f,0.04317f},{0.3726f,-0.7908f,0.4856f},{0.7677f,0.7403f},c});
    v.push_back({{0.45683f,-0.05396f,0.07478f},{0.6036f,-0.1294f,0.7867f},{0.6068f,0.3471f},c});
    v.push_back({{0.44418f,-0.11502f,0.05287f},{0.2342f,-0.7908f,0.5655f},{0.7713f,0.7620f},c});
    v.push_back({{0.43600f,-0.05396f,0.08340f},{0.3795f,-0.1294f,0.9161f},{0.2882f,0.7469f},c});
    v.push_back({{0.42483f,-0.12874f,0.04170f},{0.1475f,-0.9227f,0.3562f},{0.1769f,0.8111f},c});
    v.push_back({{0.45536f,-0.07631f,0.07223f},{0.3540f,-0.3799f,0.8546f},{0.2589f,0.7887f},c});
    v.push_back({{0.45104f,-0.09714f,0.06476f},{0.3046f,-0.6055f,0.7353f},{0.2704f,0.8277f},c});
    v.push_back({{0.41366f,-0.09714f,0.07478f},{0.1039f,-0.6055f,0.7891f},{0.3329f,0.8277f},c});
    v.push_back({{0.42946f,-0.11502f,0.05897f},{0.0799f,-0.7908f,0.6068f},{0.1924f,0.8315f},c});
    v.push_back({{0.41366f,-0.05396f,0.08634f},{0.1294f,-0.1294f,0.9831f},{0.3271f,0.7469f},c});
    v.push_back({{0.41366f,-0.12874f,0.04317f},{0.0503f,-0.9227f,0.3822f},{0.1627f,0.8258f},c});
    v.push_back({{0.43524f,-0.07631f,0.08056f},{0.1207f,-0.3799f,0.9171f},{0.2910f,0.7887f},c});
    v.push_back({{0.50000f,0.05396f,0.00000f},{0.9914f,0.0000f,0.1305f},{0.4296f,0.1901f},c});
    v.push_back({{0.45683f,0.05396f,0.07478f},{0.3827f,0.0000f,0.9239f},{0.2550f,0.5449f},c});
    v.push_back({{0.43600f,0.05396f,-0.08340f},{0.3827f,0.0000f,-0.9239f},{0.8809f,0.5231f},c});
    v.push_back({{0.43600f,-0.05396f,0.08340f},{0.1305f,0.0000f,0.9914f},{0.2882f,0.7469f},c});
    v.push_back({{0.41366f,0.05396f,-0.08634f},{0.1305f,0.0000f,-0.9914f},{0.8400f,0.5231f},c});
    v.push_back({{0.47471f,0.05396f,0.06105f},{0.6088f,0.0000f,0.7934f},{0.5525f,0.1766f},c});
    v.push_back({{0.45683f,0.05396f,-0.07478f},{0.6088f,0.0000f,-0.7934f},{0.9230f,0.5231f},c});
    v.push_back({{0.47471f,0.05396f,-0.06105f},{0.7934f,0.0000f,-0.6088f},{0.9637f,0.5231f},c});
    v.push_back({{0.48843f,0.05396f,-0.04317f},{0.9239f,0.0000f,-0.3827f},{0.8021f,0.4088f},c});
    v.push_back({{0.49706f,0.05396f,-0.02235f},{0.9914f,0.0000f,-0.1305f},{0.3921f,0.1855f},c});
    v.push_back({{0.49706f,0.05396f,0.02235f},{0.9239f,0.0000f,0.3827f},{0.4707f,0.1901f},c});
    v.push_back({{0.48843f,0.05396f,0.04317f},{0.7934f,0.0000f,0.6088f},{0.5126f,0.1855f},c});
    v.push_back({{0.41366f,0.13737f,-0.02235f},{0.0503f,0.9227f,-0.3822f},{0.0083f,0.8830f},c});
    v.push_back({{0.43524f,0.07631f,-0.08056f},{0.1294f,0.1294f,-0.9831f},{0.8815f,0.4813f},c});
    v.push_back({{0.41366f,0.12874f,-0.04317f},{0.0799f,0.7908f,-0.6068f},{0.0055f,0.9246f},c});
    v.push_back({{0.41366f,0.11502f,-0.06105f},{0.1039f,0.6055f,-0.7891f},{0.8576f,0.4088f},c});
    v.push_back({{0.41366f,0.09714f,-0.07478f},{0.1207f,0.3799f,-0.9171f},{0.8481f,0.4423f},c});
    v.push_back({{0.44418f,0.11502f,-0.05287f},{0.3046f,0.6055f,-0.7353f},{0.9164f,0.4088f},c});
    v.push_back({{0.43301f,0.09714f,-0.07223f},{0.3540f,0.3799f,-0.8546f},{0.8835f,0.4423f},c});
    v.push_back({{0.41944f,0.13737f,-0.02159f},{0.1475f,0.9227f,-0.3562f},{0.0187f,0.8819f},c});
    v.push_back({{0.45536f,0.07631f,-0.07223f},{0.3795f,0.1294f,-0.9161f},{0.9223f,0.4813f},c});
    v.push_back({{0.43524f,0.12874f,-0.03739f},{0.2342f,0.7908f,-0.5655f},{0.0441f,0.9151f},c});
    v.push_back({{0.45536f,0.07631f,-0.07223f},{0.6036f,0.1294f,-0.7867f},{0.9223f,0.4813f},c});
    v.push_back({{0.44418f,0.12874f,-0.03053f},{0.3726f,0.7908f,-0.4856f},{0.0599f,0.9031f},c});
    v.push_back({{0.45683f,0.11502f,-0.04317f},{0.4845f,0.6055f,-0.6314f},{0.9451f,0.4088f},c});
    v.push_back({{0.45104f,0.09714f,-0.06476f},{0.5631f,0.3799f,-0.7339f},{0.9200f,0.4423f},c});
    v.push_back({{0.42946f,0.13737f,-0.01580f},{0.2347f,0.9227f,-0.3058f},{0.0365f,0.8718f},c});
    v.push_back({{0.47841f,0.09714f,-0.03739f},{0.7339f,0.3799f,-0.5631f},{0.9866f,0.4423f},c});
    v.push_back({{0.43301f,0.13737f,-0.01117f},{0.3058f,0.9227f,-0.2347f},{0.0426f,0.8636f},c});
    v.push_back({{0.47263f,0.07631f,-0.05897f},{0.7867f,0.1294f,-0.6036f},{0.9615f,0.4813f},c});
    v.push_back({{0.45104f,0.12874f,-0.02159f},{0.4856f,0.7908f,-0.3726f},{0.0717f,0.8871f},c});
    v.push_back({{0.46653f,0.11502f,-0.03053f},{0.6314f,0.6055f,-0.4845f},{0.9708f,0.4089f},c});
    v.push_back({{0.46653f,0.11502f,-0.03053f},{0.7353f,0.6055f,-0.3046f},{0.3857f,0.0571f},c});
    v.push_back({{0.48588f,0.09714f,-0.01935f},{0.8546f,0.3799f,-0.3540f},{0.3966f,0.1038f},c});
    v.push_back({{0.43301f,0.13737f,-0.01117f},{0.3562f,0.9227f,-0.1475f},{0.0426f,0.8636f},c});
    v.push_back({{0.49422f,0.07631f,-0.02159f},{0.9161f,0.1294f,-0.3795f},{0.3912f,0.1454f},c});
    v.push_back({{0.45536f,0.12874f,-0.01117f},{0.5655f,0.7908f,-0.2342f},{0.4242f,0.0271f},c});
    v.push_back({{0.49706f,0.07631f,0.00000f},{0.9831f,0.1294f,-0.1294f},{0.4274f,0.1498f},c});
    v.push_back({{0.45536f,0.12874f,-0.01117f},{0.6068f,0.7908f,-0.0799f},{0.4242f,0.0271f},c});
    v.push_back({{0.47471f,0.11502f,0.00000f},{0.7891f,0.6055f,-0.1039f},{0.4343f,0.0667f},c});
    v.push_back({{0.48843f,0.09714f,0.00000f},{0.9171f,0.3799f,-0.1207f},{0.4291f,0.1077f},c});
    v.push_back({{0.43600f,0.13737f,0.00000f},{0.3822f,0.9227f,-0.0503f},{0.0473f,0.8434f},c});
    v.push_back({{0.48588f,0.09714f,0.01935f},{0.9171f,0.3799f,0.1207f},{0.4647f,0.1077f},c});
    v.push_back({{0.43600f,0.13737f,0.00000f},{0.3822f,0.9227f,0.0503f},{0.0473f,0.8434f},c});
    v.push_back({{0.49422f,0.07631f,0.02159f},{0.9831f,0.1294f,0.1294f},{0.4672f,0.1498f},c});
    v.push_back({{0.45536f,0.12874f,0.01117f},{0.6068f,0.7908f,0.0799f},{0.4635f,0.0294f},c});
    v.push_back({{0.47471f,0.11502f,0.00000f},{0.7891f,0.6055f,0.1039f},{0.4343f,0.0667f},c});
    v.push_back({{0.47263f,0.11502f,0.01580f},{0.7353f,0.6055f,0.3046f},{0.4634f,0.0667f},c});
    v.push_back({{0.48588f,0.09714f,0.01935f},{0.8546f,0.3799f,0.3540f},{0.4647f,0.1077f},c});
    v.push_back({{0.43301f,0.13737f,0.01117f},{0.3562f,0.9227f,0.1475f},{0.0412f,0.8229f},c});
    v.push_back({{0.49422f,0.07631f,0.02159f},{0.9161f,0.1294f,0.3795f},{0.4672f,0.1498f},c});
    v.push_back({{0.45536f,0.12874f,0.01117f},{0.5655f,0.7908f,0.2342f},{0.4635f,0.0294f},c});
    v.push_back({{0.43301f,0.13737f,0.01117f},{0.3058f,0.9227f,0.2347f},{0.0412f,0.8229f},c});
    v.push_back({{0.48588f,0.07631f,0.04170f},{0.7867f,0.1294f,0.6036f},{0.5077f,0.1454f},c});
    v.push_back({{0.44418f,0.12874f,0.03053f},{0.4856f,0.7908f,0.3726f},{0.5044f,0.0226f},c});
    v.push_back({{0.45683f,0.11502f,0.04317f},{0.6314f,0.6055f,0.4845f},{0.5213f,0.0571f},c});
    v.push_back({{0.46653f,0.09714f,0.05287f},{0.7339f,0.3799f,0.5631f},{0.5355f,0.0960f},c});
    v.push_back({{0.45104f,0.09714f,0.06476f},{0.5631f,0.3799f,0.7339f},{0.5660f,0.0851f},c});
    v.push_back({{0.42946f,0.13737f,0.01580f},{0.2347f,0.9227f,0.3058f},{0.0345f,0.8143f},c});
    v.push_back({{0.47263f,0.07631f,0.05897f},{0.6036f,0.1294f,0.7867f},{0.5462f,0.1368f},c});
    v.push_back({{0.43524f,0.12874f,0.03739f},{0.3726f,0.7908f,0.4856f},{0.5219f,0.0163f},c});
    v.push_back({{0.45683f,0.11502f,0.04317f},{0.4845f,0.6055f,0.6314f},{0.5213f,0.0571f},c});
    v.push_back({{0.42483f,0.12874f,0.04170f},{0.2342f,0.7908f,0.5655f},{0.5359f,0.0086f},c});
    v.push_back({{0.44418f,0.11502f,0.05287f},{0.3046f,0.6055f,0.7353f},{0.5461f,0.0482f},c});
    v.push_back({{0.43301f,0.09714f,0.07223f},{0.3540f,0.3799f,0.8546f},{0.5902f,0.0718f},c});
    v.push_back({{0.42483f,0.13737f,0.01935f},{0.1475f,0.9227f,0.3562f},{0.0259f,0.8076f},c});
    v.push_back({{0.43524f,0.07631f,0.08056f},{0.3795f,0.1294f,0.9161f},{0.2910f,0.5030f},c});
    v.push_back({{0.41944f,0.13737f,0.02159f},{0.0503f,0.9227f,0.3822f},{0.0160f,0.8032f},c});
    v.push_back({{0.43524f,0.07631f,0.08056f},{0.1294f,0.1294f,0.9831f},{0.2910f,0.5030f},c});
    v.push_back({{0.41366f,0.12874f,0.04317f},{0.0799f,0.7908f,0.6068f},{0.5454f,0.0000f},c});
    v.push_back({{0.42946f,0.11502f,0.05897f},{0.1039f,0.6055f,0.7891f},{0.3122f,0.4306f},c});
    v.push_back({{0.41366f,0.09714f,0.07478f},{0.1207f,0.3799f,0.9171f},{0.3329f,0.4641f},c});
    v.push_back({{-0.41366f,-0.01511f,-0.03885f},{0.3714f,0.9285f,0.0000f},{0.9425f,0.6041f},c});
    v.push_back({{-0.41366f,-0.01511f,0.03885f},{0.3714f,0.0000f,0.9285f},{0.9928f,0.7545f},c});
    v.push_back({{-0.41366f,-0.09282f,0.03885f},{0.3714f,-0.9285f,-0.0000f},{0.9449f,0.7545f},c});
    v.push_back({{-0.41366f,-0.09282f,-0.03885f},{0.3714f,0.0000f,-0.9285f},{1.0000f,0.1685f},c});
    v.push_back({{-0.32731f,-0.10577f,-0.05181f},{0.1483f,0.0000f,0.9889f},{0.4870f,0.4693f},c});
    v.push_back({{-0.32731f,-0.00216f,0.05181f},{0.1483f,-0.0000f,-0.9889f},{0.5071f,0.3189f},c});
    v.push_back({{-0.41366f,-0.09282f,0.03885f},{0.1483f,0.9889f,0.0000f},{0.3778f,0.1316f},c});
    v.push_back({{-0.41366f,-0.01511f,-0.03885f},{0.1483f,-0.9889f,-0.0000f},{0.3817f,0.3001f},c});
    v.push_back({{-0.36185f,-0.02806f,0.02590f},{-0.6000f,0.0000f,0.8000f},{0.3795f,0.7533f},c});
    v.push_back({{-0.32731f,-0.10577f,-0.05181f},{-0.6000f,0.0000f,-0.8000f},{0.8184f,0.6041f},c});
    v.push_back({{-0.36185f,-0.02806f,-0.02590f},{-0.6000f,0.8000f,-0.0000f},{0.2525f,0.7474f},c});
    v.push_back({{-0.36185f,-0.07987f,0.02590f},{-0.6000f,-0.8000f,0.0000f},{0.9023f,0.0376f},c});
    v.push_back({{-0.25392f,-0.02806f,0.02590f},{0.0000f,-0.0000f,-1.0000f},{0.7432f,0.6041f},c});
    v.push_back({{-0.25392f,-0.07987f,-0.02590f},{-0.0000f,-0.0000f,1.0000f},{0.1280f,0.7098f},c});
    v.push_back({{-0.25392f,-0.02806f,-0.02590f},{0.0000f,-1.0000f,-0.0000f},{0.3840f,0.6221f},c});
    v.push_back({{-0.25392f,-0.07987f,0.02590f},{0.0000f,1.0000f,-0.0000f},{0.7151f,0.0000f},c});
    v.push_back({{-0.29709f,-0.01770f,-0.03626f},{0.2334f,0.9724f,0.0000f},{0.9669f,0.4327f},c});
    v.push_back({{-0.29709f,-0.09023f,0.03626f},{0.2334f,-0.9724f,-0.0000f},{0.9653f,-0.0000f},c});
    v.push_back({{-0.29709f,-0.01770f,0.03626f},{0.2334f,0.0000f,0.9724f},{0.8702f,0.8892f},c});
    v.push_back({{-0.29709f,-0.09023f,-0.03626f},{0.2334f,0.0000f,-0.9724f},{0.7309f,0.8905f},c});
    v.push_back({{-0.21075f,-0.11199f,0.05802f},{0.2444f,0.9697f,-0.0000f},{-0.0000f,0.1685f},c});
    v.push_back({{-0.21075f,0.00406f,0.05802f},{0.2444f,-0.0000f,-0.9697f},{0.3778f,0.1685f},c});
    v.push_back({{-0.21075f,-0.11199f,-0.05802f},{0.2444f,0.0000f,0.9697f},{0.3840f,0.4874f},c});
    v.push_back({{-0.21075f,0.00406f,-0.05802f},{0.2444f,-0.9697f,-0.0000f},{0.1260f,0.1685f},c});
    v.push_back({{-0.25392f,-0.02495f,-0.02901f},{-0.5578f,0.8300f,-0.0000f},{0.8409f,0.2106f},c});
    v.push_back({{-0.25392f,-0.08298f,0.02901f},{-0.5578f,-0.8300f,0.0000f},{0.8409f,0.3791f},c});
    v.push_back({{-0.25392f,-0.02495f,0.02901f},{-0.5578f,0.0000f,0.8300f},{0.6690f,0.6462f},c});
    v.push_back({{-0.25392f,-0.08298f,-0.02901f},{-0.5578f,0.0000f,-0.8300f},{0.7432f,0.6462f},c});
    v.push_back({{-0.17837f,-0.02495f,0.02901f},{0.0000f,-0.0000f,-1.0000f},{0.5949f,0.7726f},c});
    v.push_back({{-0.17837f,-0.08298f,-0.02901f},{0.0000f,-0.0000f,1.0000f},{0.3840f,0.7788f},c});
    v.push_back({{-0.17837f,-0.02495f,-0.02901f},{0.0000f,-1.0000f,-0.0000f},{0.0000f,0.7217f},c});
    v.push_back({{-0.17837f,-0.08298f,0.02901f},{-0.0000f,1.0000f,-0.0000f},{0.4870f,0.7220f},c});
    v.push_back({{-0.22154f,-0.09458f,-0.04062f},{0.2596f,0.0000f,-0.9657f},{0.2543f,0.8602f},c});
    v.push_back({{-0.22154f,-0.01335f,-0.04062f},{0.2596f,0.9657f,0.0000f},{0.7421f,0.7726f},c});
    v.push_back({{-0.22154f,-0.09458f,0.04062f},{0.2596f,-0.9657f,-0.0000f},{0.9039f,0.4327f},c});
    v.push_back({{-0.22154f,-0.01335f,0.04062f},{0.2596f,0.0000f,0.9657f},{0.3834f,0.8765f},c});
    v.push_back({{-0.14599f,-0.02553f,-0.02843f},{-0.1592f,-0.9872f,-0.0000f},{0.1280f,0.6921f},c});
    v.push_back({{-0.14599f,-0.08240f,0.02843f},{-0.1592f,0.9872f,-0.0000f},{0.2560f,0.6980f},c});
    v.push_back({{-0.14599f,-0.02553f,0.02843f},{-0.1592f,0.0000f,-0.9872f},{0.0000f,0.7041f},c});
    v.push_back({{-0.22154f,-0.09458f,-0.04062f},{-0.1592f,0.0000f,0.9872f},{0.5949f,0.7220f},c});
    v.push_back({{-0.18916f,-0.00847f,0.04549f},{0.3675f,0.0000f,0.9300f},{0.9521f,0.1685f},c});
    v.push_back({{-0.18916f,-0.09945f,-0.04549f},{0.3675f,0.0000f,-0.9300f},{0.9521f,0.3006f},c});
    v.push_back({{-0.18916f,-0.00847f,-0.04549f},{0.3675f,0.9300f,0.0000f},{0.9041f,0.1685f},c});
    v.push_back({{-0.18916f,-0.09945f,0.04549f},{0.3675f,-0.9300f,-0.0000f},{0.9041f,0.3006f},c});
    v.push_back({{-0.10282f,-0.02667f,-0.02729f},{-0.2062f,-0.9785f,-0.0000f},{-0.0000f,0.5774f},c});
    v.push_back({{-0.10282f,-0.08126f,0.02729f},{-0.2062f,0.9785f,-0.0000f},{0.4808f,0.1057f},c});
    v.push_back({{-0.18916f,-0.00847f,0.04549f},{-0.2062f,0.0000f,-0.9785f},{0.2537f,0.5918f},c});
    v.push_back({{-0.18916f,-0.09945f,-0.04549f},{-0.2062f,0.0000f,0.9785f},{0.3817f,0.5977f},c});
    v.push_back({{-0.10282f,-0.08126f,0.02729f},{0.4446f,-0.8958f,-0.0000f},{0.8184f,0.7822f},c});
    v.push_back({{-0.10282f,-0.02667f,0.02729f},{0.4446f,0.0000f,0.8958f},{0.0782f,0.8591f},c});
    v.push_back({{-0.10282f,-0.08126f,-0.02729f},{0.4446f,-0.0000f,-0.8958f},{0.9425f,0.6318f},c});
    v.push_back({{-0.10282f,-0.02667f,-0.02729f},{0.4446f,0.8958f,-0.0000f},{0.7432f,0.7760f},c});
    v.push_back({{-0.05965f,-0.00524f,0.04872f},{0.0000f,-0.0000f,-1.0000f},{0.6024f,0.4693f},c});
    v.push_back({{-0.05965f,-0.10268f,-0.04872f},{0.0000f,-0.0000f,1.0000f},{0.7104f,0.3189f},c});
    v.push_back({{-0.05965f,-0.00524f,-0.04872f},{0.0000f,-1.0000f,-0.0000f},{0.1280f,0.4597f},c});
    v.push_back({{-0.05965f,-0.10268f,0.04872f},{-0.0000f,1.0000f,-0.0000f},{0.3817f,0.3032f},c});
    v.push_back({{-0.13520f,-0.00988f,-0.04408f},{-0.0613f,0.9981f,-0.0000f},{0.7151f,0.0034f},c});
    v.push_back({{-0.13520f,-0.09804f,0.04408f},{-0.0613f,-0.9981f,0.0000f},{0.7120f,0.4727f},c});
    v.push_back({{-0.05965f,-0.00524f,0.04872f},{-0.0613f,-0.0000f,0.9981f},{0.3840f,0.4874f},c});
    v.push_back({{-0.13520f,-0.09804f,-0.04408f},{-0.0613f,0.0000f,-0.9981f},{0.7173f,0.1718f},c});
    v.push_back({{-0.02727f,-0.01429f,0.03967f},{-0.0408f,0.0000f,-0.9992f},{0.2624f,0.3189f},c});
    v.push_back({{-0.02727f,-0.09364f,-0.03967f},{-0.0408f,-0.0000f,0.9992f},{0.0064f,0.3250f},c});
    v.push_back({{-0.02727f,-0.01429f,-0.03967f},{-0.0408f,-0.9992f,-0.0000f},{0.1344f,0.1685f},c});
    v.push_back({{-0.02727f,-0.09364f,0.03967f},{-0.0408f,0.9992f,-0.0000f},{0.0064f,0.1685f},c});
    v.push_back({{-0.02727f,-0.01429f,0.03967f},{-0.5224f,-0.0000f,0.8527f},{0.4592f,0.6221f},c});
    v.push_back({{-0.02727f,-0.09364f,-0.03967f},{-0.5224f,0.0000f,-0.8527f},{0.5712f,0.7220f},c});
    v.push_back({{-0.04022f,-0.02223f,-0.03174f},{-0.5224f,0.8527f,-0.0000f},{0.2558f,0.6033f},c});
    v.push_back({{-0.04022f,-0.08570f,0.03174f},{-0.5224f,-0.8527f,0.0000f},{0.3838f,0.6092f},c});
    v.push_back({{0.01374f,-0.02540f,-0.02856f},{-0.0587f,-0.9983f,-0.0000f},{-0.0000f,0.9190f},c});
    v.push_back({{0.01374f,-0.08253f,0.02856f},{-0.0587f,0.9983f,-0.0000f},{0.4870f,0.9192f},c});
    v.push_back({{0.01374f,-0.02540f,0.02856f},{-0.0587f,0.0000f,-0.9983f},{0.1280f,0.9415f},c});
    v.push_back({{0.01374f,-0.08253f,-0.02856f},{-0.0587f,-0.0000f,0.9983f},{0.8817f,0.9549f},c});
    v.push_back({{-0.00784f,-0.03111f,-0.02285f},{-0.2558f,0.9667f,-0.0000f},{0.9833f,0.2896f},c});
    v.push_back({{-0.00784f,-0.07682f,0.02285f},{-0.2558f,-0.9667f,0.0000f},{0.9867f,0.8756f},c});
    v.push_back({{0.01374f,-0.02540f,0.02856f},{-0.2558f,-0.0000f,0.9667f},{0.0842f,0.7217f},c});
    v.push_back({{-0.00784f,-0.07682f,-0.02285f},{-0.2558f,0.0000f,-0.9667f},{0.8113f,0.5137f},c});
    v.push_back({{-0.00784f,-0.03111f,0.02285f},{0.0610f,0.0000f,-0.9981f},{0.4570f,0.9511f},c});
    v.push_back({{0.04828f,-0.08024f,-0.02628f},{0.0610f,0.0000f,0.9981f},{0.7432f,0.9593f},c});
    v.push_back({{0.04828f,-0.02769f,-0.02628f},{0.0610f,-0.9981f,-0.0000f},{0.2560f,0.8765f},c});
    v.push_back({{0.04828f,-0.08024f,0.02628f},{0.0610f,0.9981f,-0.0000f},{0.5949f,0.8735f},c});
    v.push_back({{0.00511f,-0.06973f,0.01577f},{-0.2366f,-0.9716f,0.0000f},{0.5949f,0.8735f},c});
    v.push_back({{0.00511f,-0.03820f,0.01577f},{-0.2366f,0.0000f,0.9716f},{0.3840f,0.9560f},c});
    v.push_back({{0.00511f,-0.06973f,-0.01577f},{-0.2366f,0.0000f,-0.9716f},{0.7432f,0.9593f},c});
    v.push_back({{0.00511f,-0.03820f,-0.01577f},{-0.2366f,0.9716f,-0.0000f},{0.2560f,0.8765f},c});
    v.push_back({{0.11304f,-0.03820f,-0.01577f},{0.0000f,-1.0000f,-0.0000f},{0.2560f,0.8765f},c});
    v.push_back({{0.11304f,-0.06973f,0.01577f},{0.0000f,1.0000f,-0.0000f},{0.5949f,0.8735f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{0.0000f,-0.0000f,-1.0000f},{0.3840f,0.9560f},c});
    v.push_back({{0.11304f,-0.06973f,-0.01577f},{-0.0000f,-0.0000f,1.0000f},{0.7432f,0.9593f},c});
    v.push_back({{-0.32854f,-0.03454f,-0.14892f},{0.3710f,0.9285f,0.0178f},{0.8468f,0.8676f},c});
    v.push_back({{-0.36996f,-0.01900f,-0.09645f},{0.3264f,0.0000f,0.9452f},{0.0606f,0.8289f},c});
    v.push_back({{-0.36996f,-0.08893f,-0.09645f},{0.3710f,-0.9285f,0.0178f},{0.9476f,0.0426f},c});
    v.push_back({{-0.36660f,-0.08893f,-0.16631f},{0.4155f,0.0000f,-0.9096f},{0.1148f,0.8289f},c});
    v.push_back({{-0.28842f,-0.10059f,-0.17422f},{0.1007f,0.0000f,0.9949f},{0.2350f,0.4408f},c});
    v.push_back({{-0.29290f,-0.00734f,-0.08108f},{0.1956f,-0.0000f,-0.9807f},{0.5312f,0.4408f},c});
    v.push_back({{-0.29290f,-0.10059f,-0.08108f},{0.1482f,0.9889f,0.0071f},{0.0000f,0.4519f},c});
    v.push_back({{-0.28842f,-0.00734f,-0.17422f},{0.1482f,-0.9889f,0.0071f},{0.1163f,0.4409f},c});
    v.push_back({{-0.32283f,-0.03065f,-0.10586f},{-0.6377f,0.0000f,0.7703f},{0.2318f,0.7232f},c});
    v.push_back({{-0.32059f,-0.07728f,-0.15243f},{-0.5609f,0.0000f,-0.8279f},{0.8468f,0.2710f},c});
    v.push_back({{-0.32059f,-0.03065f,-0.15243f},{-0.5993f,0.8000f,-0.0288f},{0.1161f,0.7185f},c});
    v.push_back({{-0.29290f,-0.10059f,-0.08108f},{-0.5993f,-0.8000f,-0.0288f},{0.8854f,0.4157f},c});
    v.push_back({{-0.22580f,-0.03065f,-0.10120f},{0.0480f,-0.0000f,-0.9988f},{0.7375f,0.4157f},c});
    v.push_back({{-0.22357f,-0.07728f,-0.14777f},{-0.0480f,-0.0000f,0.9988f},{0.3525f,0.6878f},c});
    v.push_back({{-0.32059f,-0.03065f,-0.15243f},{-0.0000f,-1.0000f,-0.0000f},{0.0000f,0.8289f},c});
    v.push_back({{-0.22580f,-0.07728f,-0.10120f},{-0.0000f,1.0000f,-0.0000f},{0.6265f,0.6750f},c});
    v.push_back({{-0.26193f,-0.02133f,-0.15895f},{0.2331f,0.9724f,0.0112f},{0.9632f,0.2362f},c});
    v.push_back({{-0.22580f,-0.07728f,-0.10120f},{0.2331f,-0.9724f,0.0112f},{0.9050f,0.8583f},c});
    v.push_back({{-0.26506f,-0.02133f,-0.09375f},{0.1865f,0.0000f,0.9825f},{0.9606f,0.5657f},c});
    v.push_back({{-0.26193f,-0.08660f,-0.15895f},{0.2798f,0.0000f,-0.9601f},{0.9794f,0.4157f},c});
    v.push_back({{-0.18838f,-0.10619f,-0.07046f},{0.2441f,0.9697f,0.0117f},{-0.0000f,0.3124f},c});
    v.push_back({{-0.26506f,-0.02133f,-0.09375f},{0.2906f,0.0000f,-0.9568f},{0.4510f,0.1269f},c});
    v.push_back({{-0.18337f,-0.10619f,-0.17478f},{0.1976f,0.0000f,0.9803f},{-0.0000f,0.1562f},c});
    v.push_back({{-0.26193f,-0.02133f,-0.15895f},{0.2441f,-0.9697f,0.0117f},{0.2359f,0.1269f},c});
    v.push_back({{-0.22343f,-0.02785f,-0.15056f},{-0.5571f,0.8300f,-0.0268f},{0.8302f,0.0816f},c});
    v.push_back({{-0.18838f,-0.10619f,-0.07046f},{-0.5571f,-0.8300f,-0.0268f},{0.7858f,0.5657f},c});
    v.push_back({{-0.18838f,-0.00174f,-0.07046f},{-0.5969f,-0.0000f,0.8023f},{0.4542f,0.5628f},c});
    v.push_back({{-0.22343f,-0.08007f,-0.15056f},{-0.5173f,0.0000f,-0.8558f},{0.7141f,0.0816f},c});
    v.push_back({{-0.15802f,-0.02785f,-0.09514f},{0.0480f,-0.0000f,-0.9988f},{0.8073f,0.4157f},c});
    v.push_back({{-0.15552f,-0.08007f,-0.14730f},{-0.0480f,-0.0000f,0.9988f},{0.4445f,0.7190f},c});
    v.push_back({{-0.22343f,-0.02785f,-0.15056f},{-0.0000f,-1.0000f,-0.0000f},{0.5312f,0.7922f},c});
    v.push_back({{-0.15802f,-0.08007f,-0.09514f},{0.0000f,1.0000f,-0.0000f},{0.2350f,0.6906f},c});
    v.push_back({{-0.19382f,-0.09052f,-0.15960f},{0.3056f,0.0000f,-0.9522f},{0.8468f,0.8468f},c});
    v.push_back({{-0.15552f,-0.02785f,-0.14730f},{0.2593f,0.9657f,0.0125f},{0.6265f,0.8359f},c});
    v.push_back({{-0.19733f,-0.09052f,-0.08657f},{0.2593f,-0.9657f,0.0125f},{0.5030f,0.8199f},c});
    v.push_back({{-0.19733f,-0.01741f,-0.08657f},{0.2130f,0.0000f,0.9771f},{0.3486f,0.7923f},c});
    v.push_back({{-0.12643f,-0.02838f,-0.14538f},{-0.1590f,-0.9872f,-0.0076f},{0.6265f,0.6586f},c});
    v.push_back({{-0.12889f,-0.07955f,-0.09426f},{-0.1590f,0.9872f,-0.0076f},{0.0000f,0.6673f},c});
    v.push_back({{-0.12889f,-0.02838f,-0.09426f},{-0.1117f,0.0000f,-0.9937f},{0.6321f,0.3942f},c});
    v.push_back({{-0.12643f,-0.07955f,-0.14538f},{-0.2064f,-0.0000f,0.9785f},{0.6387f,0.5337f},c});
    v.push_back({{-0.16843f,-0.01302f,-0.08079f},{0.3224f,0.0000f,0.9466f},{0.7858f,0.7407f},c});
    v.push_back({{-0.16451f,-0.09491f,-0.16258f},{0.4117f,0.0000f,-0.9113f},{0.2889f,0.7923f},c});
    v.push_back({{-0.16451f,-0.01302f,-0.16258f},{0.3671f,0.9300f,0.0176f},{0.9050f,0.2362f},c});
    v.push_back({{-0.16843f,-0.09491f,-0.08079f},{0.3671f,-0.9300f,0.0176f},{0.9054f,0.7219f},c});
    v.push_back({{-0.16843f,-0.01302f,-0.08079f},{-0.2060f,-0.9785f,-0.0099f},{0.6455f,0.0426f},c});
    v.push_back({{-0.16451f,-0.01302f,-0.16258f},{-0.2060f,-0.9785f,-0.0099f},{0.6455f,0.1650f},c});
    v.push_back({{-0.08767f,-0.02940f,-0.14249f},{-0.2060f,-0.9785f,-0.0099f},{0.5300f,0.1406f},c});
    v.push_back({{-0.09003f,-0.07853f,-0.09342f},{-0.2060f,0.9785f,-0.0099f},{-0.0000f,0.5498f},c});
    v.push_back({{-0.16843f,-0.01302f,-0.08079f},{-0.1386f,0.0250f,-0.9900f},{0.2332f,0.5634f},c});
    v.push_back({{-0.08852f,-0.02842f,-0.09237f},{-0.1386f,0.0250f,-0.9900f},{0.1163f,0.5389f},c});
    v.push_back({{-0.09003f,-0.07853f,-0.09342f},{-0.1386f,0.0250f,-0.9900f},{0.1163f,0.4654f},c});
    v.push_back({{-0.08767f,-0.07853f,-0.14249f},{-0.2529f,-0.0000f,0.9675f},{0.5300f,0.2630f},c});
    v.push_back({{-0.09003f,-0.07853f,-0.09342f},{0.4450f,-0.8953f,0.0214f},{0.8302f,0.0683f},c});
    v.push_back({{-0.09003f,-0.07853f,-0.09342f},{0.3488f,-0.0302f,0.9367f},{0.7858f,0.8211f},c});
    v.push_back({{-0.08852f,-0.02842f,-0.09237f},{0.3488f,-0.0302f,0.9367f},{0.7858f,0.7476f},c});
    v.push_back({{-0.13014f,-0.01036f,-0.07629f},{0.3488f,-0.0302f,0.9367f},{0.8468f,0.7219f},c});
    v.push_back({{-0.12593f,-0.09806f,-0.16388f},{0.4880f,0.0000f,-0.8729f},{0.5873f,0.7922f},c});
    v.push_back({{-0.08767f,-0.02940f,-0.14249f},{0.4360f,0.8997f,0.0209f},{0.8429f,0.5914f},c});
    v.push_back({{-0.12593f,-0.01036f,-0.16388f},{0.4360f,0.8997f,0.0209f},{0.9012f,0.5657f},c});
    v.push_back({{-0.13014f,-0.01036f,-0.07629f},{0.4360f,0.8997f,0.0209f},{0.9012f,0.6906f},c});
    v.push_back({{-0.05214f,-0.01012f,-0.07230f},{0.0511f,-0.0000f,-0.9987f},{0.5312f,0.5657f},c});
    v.push_back({{-0.12593f,-0.09806f,-0.16388f},{-0.0511f,0.0000f,0.9987f},{0.3503f,0.5657f},c});
    v.push_back({{-0.12593f,-0.01036f,-0.16388f},{0.0031f,-1.0000f,0.0002f},{0.4687f,0.5628f},c});
    v.push_back({{-0.05214f,-0.09781f,-0.07230f},{-0.0031f,1.0000f,-0.0002f},{0.3537f,0.2811f},c});
    v.push_back({{-0.11605f,-0.01429f,-0.15898f},{-0.0612f,0.9981f,-0.0029f},{0.4542f,0.5660f},c});
    v.push_back({{-0.05214f,-0.09781f,-0.07230f},{-0.0612f,-0.9981f,-0.0029f},{0.1163f,0.5634f},c});
    v.push_back({{-0.11986f,-0.01429f,-0.07973f},{-0.1091f,0.0000f,0.9940f},{0.3355f,0.5688f},c});
    v.push_back({{-0.11605f,-0.09364f,-0.15898f},{-0.0133f,0.0000f,-0.9999f},{0.6265f,0.5688f},c});
    v.push_back({{-0.02265f,-0.01826f,-0.07903f},{0.0072f,-0.0000f,-1.0000f},{0.3584f,0.3013f},c});
    v.push_back({{-0.01922f,-0.08967f,-0.15036f},{-0.0887f,-0.0000f,0.9961f},{0.2419f,0.0000f},c});
    v.push_back({{-0.01922f,-0.01826f,-0.15036f},{-0.0408f,-0.9992f,-0.0020f},{0.2409f,0.1562f},c});
    v.push_back({{-0.02265f,-0.08967f,-0.07903f},{-0.0408f,0.9992f,-0.0020f},{0.1223f,0.1562f},c});
    v.push_back({{-0.03395f,-0.02540f,-0.08672f},{-0.5627f,0.0000f,0.8267f},{0.9674f,0.0511f},c});
    v.push_back({{-0.03121f,-0.08253f,-0.14379f},{-0.4809f,0.0000f,-0.8768f},{0.4424f,0.6984f},c});
    v.push_back({{-0.01922f,-0.01826f,-0.15036f},{-0.5218f,0.8527f,-0.0251f},{0.4510f,-0.0000f},c});
    v.push_back({{-0.03395f,-0.08253f,-0.08672f},{-0.5218f,-0.8527f,-0.0251f},{0.4424f,0.8052f},c});
    v.push_back({{-0.03121f,-0.02540f,-0.14379f},{-0.0586f,-0.9983f,-0.0028f},{0.1888f,0.9240f},c});
    v.push_back({{-0.03395f,-0.08253f,-0.08672f},{-0.0586f,0.9983f,-0.0028f},{0.4251f,0.9173f},c});
    v.push_back({{0.01470f,-0.02826f,-0.08725f},{-0.0107f,0.0000f,-0.9999f},{0.7253f,0.9443f},c});
    v.push_back({{0.01717f,-0.07967f,-0.13860f},{-0.1065f,-0.0000f,0.9943f},{0.6421f,0.2832f},c});
    v.push_back({{0.01717f,-0.02826f,-0.13860f},{-0.2555f,0.9667f,-0.0123f},{0.2028f,0.8278f},c});
    v.push_back({{-0.00446f,-0.07453f,-0.09331f},{-0.2555f,-0.9667f,-0.0123f},{0.7251f,0.8596f},c});
    v.push_back({{0.01470f,-0.02826f,-0.08725f},{-0.3019f,-0.0000f,0.9533f},{0.5030f,0.8199f},c});
    v.push_back({{-0.00248f,-0.07453f,-0.13440f},{-0.2092f,0.0000f,-0.9779f},{0.3429f,0.6983f},c});
    v.push_back({{0.04585f,-0.03031f,-0.08781f},{0.1088f,-0.0000f,-0.9941f},{0.2350f,0.9855f},c});
    v.push_back({{0.04812f,-0.07762f,-0.13506f},{0.0130f,0.0000f,0.9999f},{0.9054f,0.7219f},c});
    v.push_back({{0.04812f,-0.03031f,-0.13506f},{0.0609f,-0.9981f,0.0029f},{0.8889f,0.1472f},c});
    v.push_back({{-0.00446f,-0.07453f,-0.09331f},{0.0609f,0.9981f,0.0029f},{0.9096f,0.4094f},c});
    v.push_back({{0.00341f,-0.04450f,-0.10405f},{-0.3575f,0.0000f,0.9339f},{0.5058f,0.9505f},c});
    v.push_back({{0.00432f,-0.06342f,-0.12295f},{-0.2663f,0.0000f,-0.9639f},{0.5873f,0.9384f},c});
    v.push_back({{0.04812f,-0.03031f,-0.13506f},{-0.3119f,0.9500f,-0.0150f},{0.3525f,0.9173f},c});
    v.push_back({{0.04585f,-0.07762f,-0.08781f},{-0.3119f,-0.9500f,-0.0150f},{0.1163f,0.9240f},c});
    v.push_back({{0.00432f,-0.04450f,-0.12295f},{-0.0000f,-1.0000f,-0.0000f},{0.5086f,0.5398f},c});
    v.push_back({{0.02405f,-0.04450f,-0.10055f},{0.1672f,-0.0000f,-0.9859f},{0.4957f,0.0000f},c});
    v.push_back({{0.02823f,-0.06342f,-0.11901f},{-0.1628f,-0.0000f,0.9867f},{0.8302f,0.2006f},c});
    v.push_back({{0.04377f,-0.04450f,-0.09352f},{0.3359f,-0.0000f,-0.9419f},{0.6962f,0.8263f},c});
    v.push_back({{0.05109f,-0.06342f,-0.11097f},{-0.3317f,-0.0000f,0.9434f},{0.8302f,0.1675f},c});
    v.push_back({{0.06197f,-0.04450f,-0.08317f},{0.4943f,-0.0000f,-0.8693f},{0.6962f,0.7979f},c});
    v.push_back({{0.07221f,-0.06342f,-0.09908f},{-0.4905f,-0.0000f,0.8715f},{0.5873f,0.9338f},c});
    v.push_back({{0.07810f,-0.04450f,-0.06982f},{0.6378f,-0.0000f,-0.7702f},{0.6962f,0.7676f},c});
    v.push_back({{0.09094f,-0.06342f,-0.08371f},{-0.6343f,-0.0000f,0.7731f},{0.5873f,0.8981f},c});
    v.push_back({{0.09166f,-0.04450f,-0.05386f},{0.7618f,-0.0000f,-0.6478f},{0.6962f,0.7365f},c});
    v.push_back({{0.10672f,-0.06342f,-0.06532f},{-0.7589f,-0.0000f,0.6512f},{0.5873f,0.8618f},c});
    v.push_back({{0.10225f,-0.04450f,-0.03580f},{0.8627f,-0.0000f,-0.5056f},{0.6962f,0.7053f},c});
    v.push_back({{0.11907f,-0.06342f,-0.04446f},{-0.8605f,-0.0000f,0.5095f},{0.5873f,0.8262f},c});
    v.push_back({{0.10954f,-0.06342f,-0.01617f},{-0.0000f,1.0000f,0.0000f},{0.4699f,0.0580f},c});
    v.push_back({{0.10954f,-0.04450f,-0.01617f},{0.9374f,-0.0000f,-0.3481f},{0.6962f,0.6750f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{-0.9359f,-0.0000f,0.3523f},{0.5873f,0.7922f},c});
    v.push_back({{-0.36996f,-0.01900f,0.09645f},{0.3710f,0.9285f,-0.0178f},{0.9397f,0.7080f},c});
    v.push_back({{-0.36660f,-0.01900f,0.16631f},{0.4155f,0.0000f,0.9096f},{0.5284f,0.8308f},c});
    v.push_back({{-0.32854f,-0.07339f,0.14892f},{0.3710f,-0.9285f,-0.0178f},{0.8784f,0.3201f},c});
    v.push_back({{-0.36996f,-0.08893f,0.09645f},{0.3264f,0.0000f,-0.9452f},{0.9380f,0.4222f},c});
    v.push_back({{-0.29290f,-0.10059f,0.08108f},{0.1956f,0.0000f,0.9807f},{0.3524f,0.2968f},c});
    v.push_back({{-0.36660f,-0.01900f,0.16631f},{0.1007f,0.0000f,-0.9949f},{0.4718f,0.1225f},c});
    v.push_back({{-0.28842f,-0.10059f,0.17422f},{0.1482f,0.9889f,-0.0071f},{0.3524f,0.5768f},c});
    v.push_back({{-0.29290f,-0.00734f,0.08108f},{0.1482f,-0.9889f,-0.0071f},{0.3524f,0.4368f},c});
    v.push_back({{-0.28842f,-0.00734f,0.17422f},{-0.5609f,-0.0000f,0.8279f},{0.8767f,0.0044f},c});
    v.push_back({{-0.32283f,-0.07728f,0.10586f},{-0.6377f,0.0000f,-0.7703f},{0.9219f,0.1852f},c});
    v.push_back({{-0.29290f,-0.00734f,0.08108f},{-0.5993f,0.8000f,0.0288f},{0.8740f,0.5622f},c});
    v.push_back({{-0.32059f,-0.07728f,0.15243f},{-0.5993f,-0.8000f,0.0288f},{0.5841f,0.7201f},c});
    v.push_back({{-0.22357f,-0.03065f,0.14777f},{-0.0480f,0.0000f,-0.9988f},{0.7447f,0.5622f},c});
    v.push_back({{-0.22580f,-0.07728f,0.10120f},{0.0480f,0.0000f,0.9988f},{0.4689f,0.6851f},c});
    v.push_back({{-0.22580f,-0.03065f,0.10120f},{-0.0000f,-1.0000f,-0.0000f},{0.7472f,0.0044f},c});
    v.push_back({{-0.32059f,-0.07728f,0.15243f},{-0.0000f,1.0000f,0.0000f},{0.7469f,0.2960f},c});
    v.push_back({{-0.22580f,-0.03065f,0.10120f},{0.2331f,0.9724f,-0.0112f},{0.8958f,0.8450f},c});
    v.push_back({{-0.26193f,-0.08660f,0.15895f},{0.2331f,-0.9724f,-0.0112f},{0.0589f,0.8509f},c});
    v.push_back({{-0.26193f,-0.02133f,0.15895f},{0.2798f,0.0000f,0.9601f},{0.6435f,0.8444f},c});
    v.push_back({{-0.26506f,-0.08660f,0.09375f},{0.1865f,0.0000f,-0.9825f},{0.1173f,0.8509f},c});
    v.push_back({{-0.26193f,-0.08660f,0.15895f},{0.2441f,0.9697f,-0.0117f},{0.2332f,0.2842f},c});
    v.push_back({{-0.26193f,-0.02133f,0.15895f},{0.1976f,0.0000f,-0.9803f},{0.1168f,0.2842f},c});
    v.push_back({{-0.18838f,-0.10619f,0.07046f},{0.2906f,0.0000f,0.9568f},{-0.0000f,0.1568f},c});
    v.push_back({{-0.18838f,-0.00174f,0.07046f},{0.2441f,-0.9697f,-0.0117f},{0.1181f,0.1568f},c});
    v.push_back({{-0.18838f,-0.00174f,0.07046f},{-0.5571f,0.8300f,0.0268f},{0.5845f,0.6876f},c});
    v.push_back({{-0.22343f,-0.08007f,0.15056f},{-0.5571f,-0.8300f,0.0268f},{0.1148f,0.7333f},c});
    v.push_back({{-0.22343f,-0.02785f,0.15056f},{-0.5173f,0.0000f,0.8558f},{0.1709f,0.7333f},c});
    v.push_back({{-0.22594f,-0.08007f,0.09840f},{-0.5969f,0.0000f,-0.8023f},{0.0581f,0.7333f},c});
    v.push_back({{-0.15552f,-0.02785f,0.14730f},{-0.0480f,0.0000f,-0.9988f},{0.2332f,0.6965f},c});
    v.push_back({{-0.15802f,-0.08007f,0.09514f},{0.0480f,0.0000f,0.9988f},{0.6663f,0.7382f},c});
    v.push_back({{-0.15802f,-0.02785f,0.09514f},{-0.0000f,-1.0000f,-0.0000f},{0.7447f,0.7080f},c});
    v.push_back({{-0.22343f,-0.08007f,0.15056f},{0.0000f,1.0000f,-0.0000f},{0.3524f,0.8043f},c});
    v.push_back({{-0.19733f,-0.09052f,0.08657f},{0.2130f,0.0000f,-0.9771f},{0.8031f,0.8100f},c});
    v.push_back({{-0.19733f,-0.01741f,0.08657f},{0.2593f,0.9657f,-0.0125f},{0.2291f,0.8195f},c});
    v.push_back({{-0.15552f,-0.08007f,0.14730f},{0.2593f,-0.9657f,-0.0125f},{0.3524f,0.8200f},c});
    v.push_back({{-0.19382f,-0.01741f,0.15960f},{0.3056f,0.0000f,0.9522f},{0.3505f,0.7986f},c});
    v.push_back({{-0.19733f,-0.01741f,0.08657f},{-0.1590f,-0.9872f,0.0076f},{0.7472f,0.1502f},c});
    v.push_back({{-0.12643f,-0.07955f,0.14538f},{-0.1590f,0.9872f,0.0076f},{0.0000f,0.6776f},c});
    v.push_back({{-0.12643f,-0.02838f,0.14538f},{-0.2064f,0.0000f,-0.9785f},{0.1022f,0.6776f},c});
    v.push_back({{-0.12889f,-0.07955f,0.09426f},{-0.1117f,-0.0000f,0.9937f},{0.6459f,0.2435f},c});
    v.push_back({{-0.16451f,-0.01302f,0.16258f},{0.4117f,0.0000f,0.9113f},{0.8767f,0.0044f},c});
    v.push_back({{-0.16843f,-0.09491f,0.08079f},{0.3224f,0.0000f,-0.9466f},{0.8815f,0.7080f},c});
    v.push_back({{-0.16843f,-0.01302f,0.08079f},{0.3671f,0.9300f,-0.0176f},{0.2914f,0.7986f},c});
    v.push_back({{-0.12643f,-0.07955f,0.14538f},{0.3671f,-0.9300f,-0.0176f},{0.8203f,0.4453f},c});
    v.push_back({{-0.09003f,-0.02940f,0.09342f},{-0.2060f,-0.9785f,0.0099f},{0.4689f,0.6606f},c});
    v.push_back({{-0.08767f,-0.07853f,0.14249f},{-0.2060f,0.9785f,0.0099f},{0.4696f,0.3952f},c});
    v.push_back({{-0.08767f,-0.02940f,0.14249f},{-0.2529f,0.0000f,-0.9675f},{0.4698f,0.2552f},c});
    v.push_back({{-0.09003f,-0.07853f,0.09342f},{-0.1590f,-0.0000f,0.9873f},{0.2332f,0.6719f},c});
    v.push_back({{-0.12556f,-0.09781f,0.16362f},{0.4440f,-0.8958f,-0.0213f},{0.8740f,0.5622f},c});
    v.push_back({{-0.12556f,-0.01012f,0.16362f},{0.4870f,0.0000f,0.8734f},{0.2306f,0.6941f},c});
    v.push_back({{-0.12976f,-0.09781f,0.07602f},{0.4011f,0.0000f,-0.9160f},{0.8754f,0.1502f},c});
    v.push_back({{-0.12976f,-0.01012f,0.07602f},{0.4440f,0.8958f,-0.0213f},{0.8784f,0.2968f},c});
    v.push_back({{-0.04794f,-0.01012f,0.15989f},{-0.0480f,0.0000f,-0.9988f},{0.2332f,0.5736f},c});
    v.push_back({{-0.05214f,-0.09781f,0.07230f},{0.0480f,0.0000f,0.9988f},{0.1167f,0.5843f},c});
    v.push_back({{-0.05214f,-0.01012f,0.07230f},{-0.0000f,-1.0000f,-0.0000f},{0.4689f,0.5622f},c});
    v.push_back({{-0.04794f,-0.09781f,0.15989f},{0.0000f,1.0000f,-0.0000f},{-0.0000f,0.5843f},c});
    v.push_back({{-0.05214f,-0.01012f,0.07230f},{-0.0612f,0.9981f,0.0029f},{0.1191f,0.3136f},c});
    v.push_back({{-0.11605f,-0.09364f,0.15898f},{-0.0612f,-0.9981f,0.0029f},{0.6863f,0.5653f},c});
    v.push_back({{-0.11605f,-0.01429f,0.15898f},{-0.0133f,0.0000f,0.9999f},{0.4541f,0.5800f},c});
    v.push_back({{-0.11986f,-0.09364f,0.07973f},{-0.1091f,0.0000f,-0.9940f},{0.5738f,0.0031f},c});
    v.push_back({{-0.01922f,-0.01826f,0.15036f},{-0.0887f,0.0000f,-0.9961f},{0.2418f,0.0000f},c});
    v.push_back({{-0.02265f,-0.08967f,0.07903f},{0.0072f,0.0000f,1.0000f},{0.0060f,0.3136f},c});
    v.push_back({{-0.02265f,-0.01826f,0.07903f},{-0.0408f,-0.9992f,0.0020f},{0.2392f,0.3027f},c});
    v.push_back({{-0.11605f,-0.09364f,0.15898f},{-0.0408f,0.9992f,0.0020f},{0.2332f,0.3027f},c});
    v.push_back({{-0.03121f,-0.02540f,0.14379f},{-0.4809f,0.0000f,0.8768f},{0.8200f,0.8208f},c});
    v.push_back({{-0.03395f,-0.08253f,0.08672f},{-0.5627f,0.0000f,-0.8267f},{0.4287f,0.8150f},c});
    v.push_back({{-0.03395f,-0.02540f,0.08672f},{-0.5218f,0.8527f,0.0251f},{0.6607f,0.6983f},c});
    v.push_back({{-0.01922f,-0.08967f,0.15036f},{-0.5218f,-0.8527f,0.0251f},{0.6437f,0.7948f},c});
    v.push_back({{0.01470f,-0.02826f,0.08725f},{-0.0586f,-0.9983f,0.0028f},{0.8231f,0.9125f},c});
    v.push_back({{0.01717f,-0.07967f,0.13860f},{-0.0586f,0.9983f,0.0028f},{0.6461f,0.5570f},c});
    v.push_back({{-0.03121f,-0.02540f,0.14379f},{-0.1065f,0.0000f,-0.9943f},{0.7919f,0.5613f},c});
    v.push_back({{0.01470f,-0.07967f,0.08725f},{-0.0107f,-0.0000f,0.9999f},{0.6663f,0.9217f},c});
    v.push_back({{-0.00446f,-0.03340f,0.09331f},{-0.2555f,0.9667f,0.0123f},{0.3409f,0.7042f},c});
    v.push_back({{0.01717f,-0.07967f,0.13860f},{-0.2555f,-0.9667f,0.0123f},{0.2042f,0.5843f},c});
    v.push_back({{0.01717f,-0.02826f,0.13860f},{-0.2092f,-0.0000f,0.9779f},{0.1173f,0.8509f},c});
    v.push_back({{0.01470f,-0.07967f,0.08725f},{-0.3019f,0.0000f,-0.9533f},{0.5389f,0.8251f},c});
    v.push_back({{0.04812f,-0.03031f,0.13506f},{0.0130f,-0.0000f,-0.9999f},{0.9214f,0.0044f},c});
    v.push_back({{0.04585f,-0.07762f,0.08781f},{0.1088f,0.0000f,0.9941f},{0.3524f,0.9141f},c});
    v.push_back({{-0.00446f,-0.03340f,0.09331f},{0.0609f,-0.9981f,-0.0029f},{0.9256f,0.6382f},c});
    v.push_back({{0.04812f,-0.07762f,0.13506f},{0.0609f,0.9981f,-0.0029f},{0.8231f,0.9167f},c});
    v.push_back({{0.04812f,-0.03031f,0.13506f},{-0.2663f,-0.0000f,0.9639f},{0.8958f,0.9290f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{-0.3575f,0.0000f,-0.9339f},{0.2979f,0.9428f},c});
    v.push_back({{0.04585f,-0.03031f,0.08781f},{-0.3119f,0.9500f,0.0150f},{0.7447f,0.9198f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{-0.3119f,-0.9500f,0.0150f},{0.7302f,0.9473f},c});
    v.push_back({{0.00341f,-0.04450f,0.10405f},{0.3167f,-0.9484f,-0.0152f},{0.5865f,0.5190f},c});
    v.push_back({{0.00432f,-0.04450f,0.12295f},{0.3167f,-0.9484f,-0.0152f},{0.6093f,0.5360f},c});
    v.push_back({{0.00341f,-0.06342f,0.10405f},{-0.3184f,0.9478f,0.0153f},{0.6459f,0.2627f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{-0.3184f,0.9478f,0.0153f},{0.6231f,0.2797f},c});
    v.push_back({{0.03752f,-0.04450f,0.11628f},{-0.1971f,0.0000f,-0.9804f},{0.4308f,0.8433f},c});
    v.push_back({{0.03207f,-0.06342f,0.09816f},{0.2015f,0.0000f,0.9795f},{0.9219f,0.2465f},c});
    v.push_back({{0.00432f,-0.06342f,0.12295f},{0.0000f,1.0000f,-0.0000f},{0.6231f,0.2797f},c});
    v.push_back({{0.06813f,-0.04450f,0.10177f},{-0.4283f,0.0000f,-0.9036f},{0.4308f,0.7964f},c});
    v.push_back({{0.05846f,-0.06342f,0.08551f},{0.4323f,0.0000f,0.9017f},{0.9219f,0.2051f},c});
    v.push_back({{0.09432f,-0.04450f,0.08029f},{-0.6341f,0.0000f,-0.7733f},{0.4308f,0.7558f},c});
    v.push_back({{0.08100f,-0.06342f,0.06685f},{0.6375f,0.0000f,0.7704f},{0.9219f,0.1686f},c});
    v.push_back({{0.11454f,-0.04450f,0.05312f},{-0.8023f,0.0000f,-0.5970f},{0.4308f,0.7238f},c});
    v.push_back({{0.09837f,-0.06342f,0.04330f},{0.8049f,0.0000f,0.5934f},{0.9219f,0.1390f},c});
    v.push_back({{0.10952f,-0.06342f,0.01625f},{0.9245f,0.0000f,0.3812f},{0.9219f,0.1182f},c});
    v.push_back({{0.10952f,-0.04450f,0.01625f},{0.0000f,-1.0000f,0.0000f},{0.5865f,0.3123f},c});
    v.push_back({{0.12759f,-0.04450f,0.02187f},{-0.9228f,0.0000f,-0.3853f},{0.4308f,0.7022f},c});
    v.push_back({{0.10952f,-0.04450f,0.01625f},{0.1346f,0.0000f,0.9909f},{0.9503f,0.1182f},c});
    v.push_back({{0.10954f,-0.06342f,-0.01617f},{0.1148f,0.0000f,-0.9934f},{0.7245f,0.6750f},c});
    v.push_back({{0.11304f,-0.06973f,0.01577f},{-0.3977f,0.9175f,-0.0002f},{0.3840f,0.8797f},c});
    v.push_back({{0.12759f,-0.06342f,0.02187f},{-0.3977f,0.9175f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.12761f,-0.06342f,-0.02178f},{-0.3977f,0.9175f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.12761f,-0.04450f,-0.02178f},{0.3972f,0.9177f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.11304f,-0.03820f,-0.01577f},{0.3972f,0.9177f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.11304f,-0.03820f,0.01577f},{0.3972f,0.9177f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.10601f,-0.05680f,0.00659f},{-0.2933f,-0.9560f,-0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.10602f,-0.05113f,-0.00650f},{0.5775f,0.0000f,0.8164f},{0.6156f,0.7922f},c});
    v.push_back({{0.10601f,-0.05113f,0.00659f},{0.2933f,-0.9560f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.10601f,-0.05680f,0.00659f},{0.5778f,0.0000f,-0.8161f},{0.4592f,0.7022f},c});
    v.push_back({{0.10601f,-0.05680f,0.00659f},{0.0000f,1.0000f,-0.0000f},{0.6221f,0.0404f},c});
    v.push_back({{0.17077f,-0.05113f,-0.00650f},{0.0000f,-0.0000f,-1.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.10601f,-0.05113f,0.00659f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05680f,0.00659f},{-0.0000f,-0.0000f,1.0000f},{0.4592f,0.7022f},c});
    v.push_back({{0.13621f,-0.06816f,0.03278f},{0.3123f,-0.9500f,0.0002f},{0.6221f,0.0404f},c});
    v.push_back({{0.17077f,-0.05113f,-0.00650f},{-0.6044f,-0.0000f,0.7967f},{0.6156f,0.7922f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{-0.3123f,-0.9500f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.17077f,-0.05680f,0.00659f},{-0.6041f,0.0000f,-0.7969f},{0.4592f,0.7022f},c});
    v.push_back({{0.11463f,-0.02898f,0.03278f},{-0.4472f,-0.8944f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.11466f,-0.03977f,-0.07587f},{-0.8944f,0.0000f,0.4472f},{0.6156f,0.7922f},c});
    v.push_back({{0.13625f,-0.03977f,-0.03269f},{0.0000f,1.0000f,-0.0000f},{0.6156f,0.7922f},c});
    v.push_back({{0.11466f,-0.02898f,-0.03269f},{0.0000f,1.0000f,-0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.11463f,-0.06816f,0.07595f},{-0.8944f,0.0000f,-0.4472f},{0.4592f,0.7022f},c});
    v.push_back({{0.11463f,-0.03977f,0.07595f},{-0.4364f,-0.8729f,-0.2182f},{0.4308f,0.7022f},c});
    v.push_back({{0.11463f,-0.02898f,0.03278f},{-0.4364f,-0.8729f,-0.2182f},{0.0000f,0.0000f},c});
    v.push_back({{0.13621f,-0.03977f,0.03278f},{-0.4364f,-0.8729f,-0.2182f},{0.0000f,0.0000f},c});
    v.push_back({{0.18157f,-0.05680f,-0.00650f},{-0.0000f,-1.0000f,0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.18157f,-0.05680f,-0.00650f},{-0.0000f,-0.0000f,1.0000f},{0.5873f,0.7922f},c});
    v.push_back({{0.18157f,-0.05113f,-0.00650f},{-0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.18156f,-0.05113f,0.00659f},{0.0000f,-0.0000f,-1.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.18157f,-0.05113f,-0.00650f},{0.4343f,0.9008f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.14916f,-0.03552f,0.04260f},{0.7434f,0.0000f,0.6688f},{0.4308f,0.7022f},c});
    v.push_back({{0.18157f,-0.05680f,-0.00650f},{-0.4343f,0.9008f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.14921f,-0.07241f,-0.04252f},{0.7438f,0.0000f,-0.6684f},{0.5873f,0.7922f},c});
    v.push_back({{0.19237f,-0.06503f,-0.02549f},{0.1685f,-0.9857f,0.0001f},{0.4936f,0.0426f},c});
    v.push_back({{0.14921f,-0.07241f,-0.04252f},{-0.3669f,0.0000f,0.9303f},{0.5873f,0.7922f},c});
    v.push_back({{0.19237f,-0.04290f,-0.02549f},{-0.1685f,-0.9857f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.14916f,-0.03552f,0.04260f},{-0.3668f,0.0000f,-0.9303f},{0.4308f,0.7022f},c});
    v.push_back({{0.19237f,-0.04290f,-0.02549f},{-0.3929f,0.9196f,-0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.19234f,-0.04290f,0.02558f},{-0.7022f,-0.0000f,0.7120f},{0.4308f,0.7022f},c});
    v.push_back({{0.19237f,-0.06503f,-0.02549f},{0.3929f,0.9196f,0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.19237f,-0.06503f,-0.02549f},{-0.7018f,0.0000f,-0.7123f},{0.5873f,0.7922f},c});
    v.push_back({{0.21393f,-0.04843f,0.01281f},{0.0000f,-0.0000f,-1.0000f},{0.4308f,0.7022f},c});
    v.push_back({{0.21395f,-0.05950f,-0.01272f},{-0.0000f,-1.0000f,0.0000f},{0.4936f,0.0426f},c});
    v.push_back({{0.21395f,-0.05950f,-0.01272f},{-0.0000f,-0.0000f,1.0000f},{0.5873f,0.7922f},c});
    v.push_back({{0.21395f,-0.04843f,-0.01272f},{-0.0000f,-1.0000f,0.0000f},{0.0000f,0.0000f},c});
    v.push_back({{0.21395f,-0.05950f,-0.01272f},{-0.3929f,0.9196f,-0.0002f},{0.4936f,0.0426f},c});
    v.push_back({{0.18806f,-0.07057f,-0.03826f},{0.7022f,0.0000f,-0.7120f},{0.5873f,0.7922f},c});
    v.push_back({{0.21395f,-0.04843f,-0.01272f},{0.3929f,0.9196f,0.0002f},{0.0000f,0.0000f},c});
    v.push_back({{0.18802f,-0.03736f,0.03834f},{0.7018f,0.0000f,0.7123f},{0.4308f,0.7022f},c});
    v.push_back({{0.23122f,-0.04400f,-0.02294f},{-0.1520f,-0.9884f,-0.0001f},{0.0000f,0.0000f},c});
    v.push_back({{0.23120f,-0.04400f,0.02302f},{-0.3344f,0.0000f,-0.9424f},{0.4308f,0.7022f},c});
    v.push_back({{0.23122f,-0.06393f,-0.02294f},{0.1520f,-0.9884f,0.0001f},{0.4936f,0.0426f},c});
    v.push_back({{0.18806f,-0.07057f,-0.03826f},{-0.3345f,0.0000f,0.9424f},{0.5873f,0.7922f},c});
    v.push_back({{0.24391f,-0.05396f,-0.02235f},{0.0665f,-0.0255f,-0.9975f},{0.2443f,0.7887f},c});
    v.push_back({{0.24675f,-0.07631f,-0.02159f},{0.0665f,-0.0255f,-0.9975f},{0.2443f,0.8308f},c});
    v.push_back({{0.23122f,-0.06393f,-0.02294f},{0.0665f,-0.0255f,-0.9975f},{0.2196f,0.8042f},c});
    v.push_back({{0.24097f,-0.05396f,-0.00000f},{0.6132f,0.7858f,0.0807f},{0.1948f,0.8083f},c});
    v.push_back({{0.24391f,-0.05396f,-0.02235f},{0.6132f,0.7858f,0.0807f},{0.1924f,0.7673f},c});
    v.push_back({{0.23122f,-0.04400f,-0.02294f},{0.6132f,0.7858f,0.0807f},{0.2196f,0.7707f},c});
    v.push_back({{0.23122f,-0.04400f,-0.02294f},{0.7143f,0.6998f,0.0004f},{0.2196f,0.7707f},c});
    v.push_back({{0.23120f,-0.04400f,0.02302f},{0.7143f,0.6998f,0.0004f},{0.2196f,0.8517f},c});
    v.push_back({{0.24097f,-0.05396f,-0.00000f},{0.7143f,0.6998f,0.0004f},{0.1948f,0.8083f},c});
    idx.insert(idx.end(), {
        uint16_t(base+0),uint16_t(base+1),uint16_t(base+2),uint16_t(base+3),uint16_t(base+4),uint16_t(base+5),uint16_t(base+6),uint16_t(base+7),uint16_t(base+8),uint16_t(base+9),uint16_t(base+10),uint16_t(base+11),
        uint16_t(base+12),uint16_t(base+13),uint16_t(base+14),uint16_t(base+15),uint16_t(base+16),uint16_t(base+17),uint16_t(base+18),uint16_t(base+19),uint16_t(base+20),uint16_t(base+21),uint16_t(base+22),uint16_t(base+23),
        uint16_t(base+24),uint16_t(base+25),uint16_t(base+26),uint16_t(base+27),uint16_t(base+28),uint16_t(base+29),uint16_t(base+30),uint16_t(base+31),uint16_t(base+32),uint16_t(base+33),uint16_t(base+34),uint16_t(base+35),
        uint16_t(base+36),uint16_t(base+37),uint16_t(base+38),uint16_t(base+39),uint16_t(base+40),uint16_t(base+41),uint16_t(base+42),uint16_t(base+43),uint16_t(base+44),uint16_t(base+45),uint16_t(base+46),uint16_t(base+47),
        uint16_t(base+48),uint16_t(base+49),uint16_t(base+50),uint16_t(base+51),uint16_t(base+52),uint16_t(base+53),uint16_t(base+54),uint16_t(base+55),uint16_t(base+56),uint16_t(base+57),uint16_t(base+58),uint16_t(base+59),
        uint16_t(base+60),uint16_t(base+61),uint16_t(base+62),uint16_t(base+63),uint16_t(base+64),uint16_t(base+65),uint16_t(base+66),uint16_t(base+67),uint16_t(base+68),uint16_t(base+69),uint16_t(base+70),uint16_t(base+71),
        uint16_t(base+72),uint16_t(base+73),uint16_t(base+74),uint16_t(base+75),uint16_t(base+76),uint16_t(base+77),uint16_t(base+78),uint16_t(base+79),uint16_t(base+80),uint16_t(base+81),uint16_t(base+82),uint16_t(base+83),
        uint16_t(base+84),uint16_t(base+85),uint16_t(base+86),uint16_t(base+87),uint16_t(base+88),uint16_t(base+89),uint16_t(base+90),uint16_t(base+91),uint16_t(base+92),uint16_t(base+93),uint16_t(base+94),uint16_t(base+95),
        uint16_t(base+96),uint16_t(base+97),uint16_t(base+98),uint16_t(base+99),uint16_t(base+100),uint16_t(base+101),uint16_t(base+102),uint16_t(base+103),uint16_t(base+104),uint16_t(base+105),uint16_t(base+106),uint16_t(base+107),
        uint16_t(base+108),uint16_t(base+109),uint16_t(base+110),uint16_t(base+111),uint16_t(base+112),uint16_t(base+113),uint16_t(base+114),uint16_t(base+115),uint16_t(base+116),uint16_t(base+117),uint16_t(base+118),uint16_t(base+119),
        uint16_t(base+120),uint16_t(base+121),uint16_t(base+122),uint16_t(base+123),uint16_t(base+124),uint16_t(base+125),uint16_t(base+126),uint16_t(base+127),uint16_t(base+128),uint16_t(base+129),uint16_t(base+130),uint16_t(base+131),
        uint16_t(base+132),uint16_t(base+133),uint16_t(base+134),uint16_t(base+135),uint16_t(base+136),uint16_t(base+137),uint16_t(base+138),uint16_t(base+139),uint16_t(base+140),uint16_t(base+141),uint16_t(base+142),uint16_t(base+143),
        uint16_t(base+144),uint16_t(base+145),uint16_t(base+146),uint16_t(base+147),uint16_t(base+148),uint16_t(base+149),uint16_t(base+150),uint16_t(base+151),uint16_t(base+152),uint16_t(base+153),uint16_t(base+154),uint16_t(base+155),
        uint16_t(base+156),uint16_t(base+157),uint16_t(base+158),uint16_t(base+159),uint16_t(base+160),uint16_t(base+161),uint16_t(base+162),uint16_t(base+163),uint16_t(base+164),uint16_t(base+165),uint16_t(base+166),uint16_t(base+167),
        uint16_t(base+168),uint16_t(base+169),uint16_t(base+170),uint16_t(base+171),uint16_t(base+172),uint16_t(base+173),uint16_t(base+174),uint16_t(base+175),uint16_t(base+176),uint16_t(base+177),uint16_t(base+178),uint16_t(base+179),
        uint16_t(base+180),uint16_t(base+181),uint16_t(base+182),uint16_t(base+183),uint16_t(base+184),uint16_t(base+185),uint16_t(base+186),uint16_t(base+187),uint16_t(base+188),uint16_t(base+189),uint16_t(base+190),uint16_t(base+191),
        uint16_t(base+192),uint16_t(base+193),uint16_t(base+194),uint16_t(base+195),uint16_t(base+196),uint16_t(base+197),uint16_t(base+198),uint16_t(base+199),uint16_t(base+200),uint16_t(base+201),uint16_t(base+202),uint16_t(base+203),
        uint16_t(base+204),uint16_t(base+205),uint16_t(base+206),uint16_t(base+207),uint16_t(base+208),uint16_t(base+209),uint16_t(base+210),uint16_t(base+211),uint16_t(base+212),uint16_t(base+213),uint16_t(base+214),uint16_t(base+215),
        uint16_t(base+216),uint16_t(base+217),uint16_t(base+216),uint16_t(base+218),uint16_t(base+219),uint16_t(base+218),uint16_t(base+220),uint16_t(base+221),uint16_t(base+220),uint16_t(base+217),uint16_t(base+222),uint16_t(base+217),
        uint16_t(base+223),uint16_t(base+224),uint16_t(base+223),uint16_t(base+225),uint16_t(base+226),uint16_t(base+225),uint16_t(base+227),uint16_t(base+228),uint16_t(base+227),uint16_t(base+229),uint16_t(base+230),uint16_t(base+229),
        uint16_t(base+219),uint16_t(base+231),uint16_t(base+219),uint16_t(base+232),uint16_t(base+233),uint16_t(base+232),uint16_t(base+234),uint16_t(base+235),uint16_t(base+234),uint16_t(base+236),uint16_t(base+223),uint16_t(base+236),
        uint16_t(base+226),uint16_t(base+237),uint16_t(base+226),uint16_t(base+235),uint16_t(base+238),uint16_t(base+235),uint16_t(base+222),uint16_t(base+239),uint16_t(base+222),uint16_t(base+237),uint16_t(base+240),uint16_t(base+237),
        uint16_t(base+241),uint16_t(base+227),uint16_t(base+241),uint16_t(base+242),uint16_t(base+243),uint16_t(base+242),uint16_t(base+221),uint16_t(base+244),uint16_t(base+221),uint16_t(base+245),uint16_t(base+241),uint16_t(base+245),
        uint16_t(base+240),uint16_t(base+246),uint16_t(base+240),uint16_t(base+246),uint16_t(base+247),uint16_t(base+246),uint16_t(base+243),uint16_t(base+232),uint16_t(base+243),uint16_t(base+247),uint16_t(base+229),uint16_t(base+247),
        uint16_t(base+248),uint16_t(base+249),uint16_t(base+248),uint16_t(base+250),uint16_t(base+251),uint16_t(base+250),uint16_t(base+238),uint16_t(base+252),uint16_t(base+238),uint16_t(base+251),uint16_t(base+248),uint16_t(base+251),
        uint16_t(base+224),uint16_t(base+253),uint16_t(base+224),uint16_t(base+254),uint16_t(base+250),uint16_t(base+254),uint16_t(base+255),uint16_t(base+225),uint16_t(base+255),uint16_t(base+256),uint16_t(base+220),uint16_t(base+256),
        uint16_t(base+253),uint16_t(base+254),uint16_t(base+253),uint16_t(base+257),uint16_t(base+216),uint16_t(base+257),uint16_t(base+231),uint16_t(base+242),uint16_t(base+231),uint16_t(base+252),uint16_t(base+257),uint16_t(base+252),
        uint16_t(base+239),uint16_t(base+255),uint16_t(base+239),uint16_t(base+258),uint16_t(base+256),uint16_t(base+258),uint16_t(base+249),uint16_t(base+234),uint16_t(base+249),uint16_t(base+228),uint16_t(base+258),uint16_t(base+228),
        uint16_t(base+233),uint16_t(base+259),uint16_t(base+233),uint16_t(base+230),uint16_t(base+260),uint16_t(base+230),uint16_t(base+259),uint16_t(base+245),uint16_t(base+259),uint16_t(base+260),uint16_t(base+261),uint16_t(base+260),
        uint16_t(base+261),uint16_t(base+218),uint16_t(base+261),uint16_t(base+244),uint16_t(base+236),uint16_t(base+244),uint16_t(base+262),uint16_t(base+263),uint16_t(base+264),uint16_t(base+265),uint16_t(base+266),uint16_t(base+267),
        uint16_t(base+268),uint16_t(base+269),uint16_t(base+270),uint16_t(base+271),uint16_t(base+272),uint16_t(base+273),uint16_t(base+274),uint16_t(base+275),uint16_t(base+276),uint16_t(base+277),uint16_t(base+278),uint16_t(base+279),
        uint16_t(base+280),uint16_t(base+281),uint16_t(base+282),uint16_t(base+283),uint16_t(base+284),uint16_t(base+285),uint16_t(base+286),uint16_t(base+287),uint16_t(base+288),uint16_t(base+289),uint16_t(base+290),uint16_t(base+291),
        uint16_t(base+292),uint16_t(base+293),uint16_t(base+294),uint16_t(base+295),uint16_t(base+296),uint16_t(base+297),uint16_t(base+298),uint16_t(base+299),uint16_t(base+300),uint16_t(base+301),uint16_t(base+302),uint16_t(base+303),
        uint16_t(base+304),uint16_t(base+305),uint16_t(base+306),uint16_t(base+307),uint16_t(base+308),uint16_t(base+309),uint16_t(base+310),uint16_t(base+311),uint16_t(base+312),uint16_t(base+313),uint16_t(base+314),uint16_t(base+315),
        uint16_t(base+316),uint16_t(base+317),uint16_t(base+318),uint16_t(base+319),uint16_t(base+320),uint16_t(base+321),uint16_t(base+322),uint16_t(base+323),uint16_t(base+324),uint16_t(base+325),uint16_t(base+326),uint16_t(base+327),
        uint16_t(base+328),uint16_t(base+329),uint16_t(base+330),uint16_t(base+331),uint16_t(base+332),uint16_t(base+333),uint16_t(base+334),uint16_t(base+335),uint16_t(base+336),uint16_t(base+337),uint16_t(base+338),uint16_t(base+339),
        uint16_t(base+340),uint16_t(base+341),uint16_t(base+342),uint16_t(base+343),uint16_t(base+344),uint16_t(base+345),uint16_t(base+346),uint16_t(base+347),uint16_t(base+348),uint16_t(base+349),uint16_t(base+350),uint16_t(base+351),
        uint16_t(base+352),uint16_t(base+353),uint16_t(base+354),uint16_t(base+355),uint16_t(base+356),uint16_t(base+357),uint16_t(base+358),uint16_t(base+359),uint16_t(base+360),uint16_t(base+361),uint16_t(base+362),uint16_t(base+363),
        uint16_t(base+364),uint16_t(base+365),uint16_t(base+366),uint16_t(base+367),uint16_t(base+368),uint16_t(base+369),uint16_t(base+370),uint16_t(base+371),uint16_t(base+372),uint16_t(base+373),uint16_t(base+374),uint16_t(base+375),
        uint16_t(base+376),uint16_t(base+377),uint16_t(base+378),uint16_t(base+379),uint16_t(base+380),uint16_t(base+381),uint16_t(base+382),uint16_t(base+383),uint16_t(base+384),uint16_t(base+385),uint16_t(base+386),uint16_t(base+387),
        uint16_t(base+388),uint16_t(base+389),uint16_t(base+390),uint16_t(base+391),uint16_t(base+392),uint16_t(base+393),uint16_t(base+394),uint16_t(base+395),uint16_t(base+396),uint16_t(base+397),uint16_t(base+398),uint16_t(base+399),
        uint16_t(base+400),uint16_t(base+401),uint16_t(base+402),uint16_t(base+403),uint16_t(base+404),uint16_t(base+405),uint16_t(base+406),uint16_t(base+407),uint16_t(base+408),uint16_t(base+409),uint16_t(base+410),uint16_t(base+411),
        uint16_t(base+412),uint16_t(base+413),uint16_t(base+414),uint16_t(base+415),uint16_t(base+416),uint16_t(base+417),uint16_t(base+418),uint16_t(base+419),uint16_t(base+420),uint16_t(base+421),uint16_t(base+422),uint16_t(base+423),
        uint16_t(base+424),uint16_t(base+425),uint16_t(base+426),uint16_t(base+427),uint16_t(base+428),uint16_t(base+429),uint16_t(base+430),uint16_t(base+431),uint16_t(base+432),uint16_t(base+433),uint16_t(base+434),uint16_t(base+435),
        uint16_t(base+436),uint16_t(base+437),uint16_t(base+438),uint16_t(base+439),uint16_t(base+440),uint16_t(base+441),uint16_t(base+442),uint16_t(base+443),uint16_t(base+444),uint16_t(base+445),uint16_t(base+446),uint16_t(base+447),
        uint16_t(base+448),uint16_t(base+449),uint16_t(base+450),uint16_t(base+451),uint16_t(base+452),uint16_t(base+453),uint16_t(base+454),uint16_t(base+455),uint16_t(base+456),uint16_t(base+457),uint16_t(base+458),uint16_t(base+459),
        uint16_t(base+460),uint16_t(base+461),uint16_t(base+462),uint16_t(base+463),uint16_t(base+464),uint16_t(base+465),uint16_t(base+466),uint16_t(base+467),uint16_t(base+468),uint16_t(base+469),uint16_t(base+470),uint16_t(base+471),
        uint16_t(base+472),uint16_t(base+473),uint16_t(base+474),uint16_t(base+475),uint16_t(base+476),uint16_t(base+477),uint16_t(base+478),uint16_t(base+479),uint16_t(base+480),uint16_t(base+481),uint16_t(base+482),uint16_t(base+483),
        uint16_t(base+484),uint16_t(base+485),uint16_t(base+486),uint16_t(base+487),uint16_t(base+488),uint16_t(base+489),uint16_t(base+490),uint16_t(base+491),uint16_t(base+492),uint16_t(base+493),uint16_t(base+494),uint16_t(base+495),
        uint16_t(base+496),uint16_t(base+497),uint16_t(base+498),uint16_t(base+499),uint16_t(base+500),uint16_t(base+501),uint16_t(base+502),uint16_t(base+503),uint16_t(base+504),uint16_t(base+505),uint16_t(base+506),uint16_t(base+507),
        uint16_t(base+508),uint16_t(base+509),uint16_t(base+510),uint16_t(base+511),uint16_t(base+512),uint16_t(base+513),uint16_t(base+514),uint16_t(base+515),uint16_t(base+516),uint16_t(base+517),uint16_t(base+518),uint16_t(base+519),
        uint16_t(base+520),uint16_t(base+521),uint16_t(base+522),uint16_t(base+523),uint16_t(base+524),uint16_t(base+525),uint16_t(base+526),uint16_t(base+527),uint16_t(base+528),uint16_t(base+529),uint16_t(base+530),uint16_t(base+531),
        uint16_t(base+532),uint16_t(base+533),uint16_t(base+534),uint16_t(base+535),uint16_t(base+536),uint16_t(base+537),uint16_t(base+538),uint16_t(base+539),uint16_t(base+540),uint16_t(base+541),uint16_t(base+542),uint16_t(base+543),
        uint16_t(base+544),uint16_t(base+545),uint16_t(base+546),uint16_t(base+547),uint16_t(base+548),uint16_t(base+549),uint16_t(base+550),uint16_t(base+551),uint16_t(base+552),uint16_t(base+553),uint16_t(base+554),uint16_t(base+555),
        uint16_t(base+556),uint16_t(base+557),uint16_t(base+558),uint16_t(base+559),uint16_t(base+560),uint16_t(base+561),uint16_t(base+562),uint16_t(base+563),uint16_t(base+564),uint16_t(base+565),uint16_t(base+566),uint16_t(base+567),
        uint16_t(base+568),uint16_t(base+569),uint16_t(base+570),uint16_t(base+571),uint16_t(base+572),uint16_t(base+573),uint16_t(base+574),uint16_t(base+575),uint16_t(base+576),uint16_t(base+577),uint16_t(base+578),uint16_t(base+579),
        uint16_t(base+580),uint16_t(base+581),uint16_t(base+582),uint16_t(base+583),uint16_t(base+584),uint16_t(base+585),uint16_t(base+586),uint16_t(base+587),uint16_t(base+588),uint16_t(base+589),uint16_t(base+590),uint16_t(base+591),
        uint16_t(base+592),uint16_t(base+593),uint16_t(base+594),uint16_t(base+595),uint16_t(base+596),uint16_t(base+597),uint16_t(base+598),uint16_t(base+599),uint16_t(base+600),uint16_t(base+601),uint16_t(base+602),uint16_t(base+603),
        uint16_t(base+604),uint16_t(base+605),uint16_t(base+606),uint16_t(base+607),uint16_t(base+608),uint16_t(base+609),uint16_t(base+610),uint16_t(base+611),uint16_t(base+612),uint16_t(base+613),uint16_t(base+614),uint16_t(base+615),
        uint16_t(base+616),uint16_t(base+617),uint16_t(base+618),uint16_t(base+619),uint16_t(base+620),uint16_t(base+621),uint16_t(base+622),uint16_t(base+623),uint16_t(base+624),uint16_t(base+625),uint16_t(base+626),uint16_t(base+627),
        uint16_t(base+628),uint16_t(base+629),uint16_t(base+630),uint16_t(base+631),uint16_t(base+632),uint16_t(base+633),uint16_t(base+634),uint16_t(base+635),uint16_t(base+636),uint16_t(base+637),uint16_t(base+638),uint16_t(base+639),
        uint16_t(base+640),uint16_t(base+641),uint16_t(base+642),uint16_t(base+643),uint16_t(base+644),uint16_t(base+645),uint16_t(base+646),uint16_t(base+647),uint16_t(base+648),uint16_t(base+649),uint16_t(base+650),uint16_t(base+651),
        uint16_t(base+652),uint16_t(base+653),uint16_t(base+654),uint16_t(base+655),uint16_t(base+656),uint16_t(base+657),uint16_t(base+658),uint16_t(base+659),uint16_t(base+660),uint16_t(base+661),uint16_t(base+662),uint16_t(base+663),
        uint16_t(base+664),uint16_t(base+665),uint16_t(base+666),uint16_t(base+667),uint16_t(base+668),uint16_t(base+669),uint16_t(base+670),uint16_t(base+671),uint16_t(base+672),uint16_t(base+673),uint16_t(base+674),uint16_t(base+675),
        uint16_t(base+676),uint16_t(base+677),uint16_t(base+678),uint16_t(base+679),uint16_t(base+680),uint16_t(base+681),uint16_t(base+682),uint16_t(base+683),uint16_t(base+684),uint16_t(base+685),uint16_t(base+686),uint16_t(base+687),
        uint16_t(base+688),uint16_t(base+689),uint16_t(base+690),uint16_t(base+691),uint16_t(base+692),uint16_t(base+693),uint16_t(base+694),uint16_t(base+695),uint16_t(base+696),uint16_t(base+697),uint16_t(base+698),uint16_t(base+699),
        uint16_t(base+700),uint16_t(base+701),uint16_t(base+702),uint16_t(base+703),uint16_t(base+704),uint16_t(base+705),uint16_t(base+706),uint16_t(base+707),uint16_t(base+708),uint16_t(base+709),uint16_t(base+710),uint16_t(base+711),
        uint16_t(base+712),uint16_t(base+713),uint16_t(base+714),uint16_t(base+715),uint16_t(base+716),uint16_t(base+717),uint16_t(base+718),uint16_t(base+719),uint16_t(base+720),uint16_t(base+721),uint16_t(base+722),uint16_t(base+723),
        uint16_t(base+724),uint16_t(base+725),uint16_t(base+726),uint16_t(base+727),uint16_t(base+728),uint16_t(base+729),uint16_t(base+730),uint16_t(base+731),uint16_t(base+732),uint16_t(base+733),uint16_t(base+734),uint16_t(base+735),
        uint16_t(base+736),uint16_t(base+737),uint16_t(base+738),uint16_t(base+739),uint16_t(base+740),uint16_t(base+741),uint16_t(base+742),uint16_t(base+743),uint16_t(base+744),uint16_t(base+745),uint16_t(base+746),uint16_t(base+747),
        uint16_t(base+748),uint16_t(base+749),uint16_t(base+750),uint16_t(base+751),uint16_t(base+752),uint16_t(base+753),uint16_t(base+754),uint16_t(base+755),uint16_t(base+756),uint16_t(base+757),uint16_t(base+758),uint16_t(base+759),
        uint16_t(base+760),uint16_t(base+761),uint16_t(base+762),uint16_t(base+763),uint16_t(base+764),uint16_t(base+765),uint16_t(base+766),uint16_t(base+767),uint16_t(base+768),uint16_t(base+769),uint16_t(base+770),uint16_t(base+771),
        uint16_t(base+772),uint16_t(base+773),uint16_t(base+774),uint16_t(base+775),uint16_t(base+776),uint16_t(base+777),uint16_t(base+778),uint16_t(base+779),uint16_t(base+780),uint16_t(base+781),uint16_t(base+782),uint16_t(base+783),
        uint16_t(base+784),uint16_t(base+785),uint16_t(base+786),uint16_t(base+787),uint16_t(base+788),uint16_t(base+789),uint16_t(base+790),uint16_t(base+791),uint16_t(base+792),uint16_t(base+793),uint16_t(base+794),uint16_t(base+795),
        uint16_t(base+796),uint16_t(base+797),uint16_t(base+798),uint16_t(base+799),uint16_t(base+800),uint16_t(base+801),uint16_t(base+802),uint16_t(base+803),uint16_t(base+804),uint16_t(base+805),uint16_t(base+806),uint16_t(base+807),
        uint16_t(base+808),uint16_t(base+809),uint16_t(base+810),uint16_t(base+811),uint16_t(base+812),uint16_t(base+813),uint16_t(base+814),uint16_t(base+815),uint16_t(base+816),uint16_t(base+817),uint16_t(base+818),uint16_t(base+819),
        uint16_t(base+820),uint16_t(base+821),uint16_t(base+822),uint16_t(base+823),uint16_t(base+824),uint16_t(base+825),uint16_t(base+826),uint16_t(base+827),uint16_t(base+828),uint16_t(base+829),uint16_t(base+830),uint16_t(base+831),
        uint16_t(base+832),uint16_t(base+833),uint16_t(base+834),uint16_t(base+835),uint16_t(base+836),uint16_t(base+837),uint16_t(base+838),uint16_t(base+839),uint16_t(base+840),uint16_t(base+841),uint16_t(base+842),uint16_t(base+843),
        uint16_t(base+844),uint16_t(base+845),uint16_t(base+846),uint16_t(base+847),uint16_t(base+848),uint16_t(base+849),uint16_t(base+850),uint16_t(base+851),uint16_t(base+852),uint16_t(base+853),uint16_t(base+854),uint16_t(base+855),
        uint16_t(base+856),uint16_t(base+857),uint16_t(base+858),uint16_t(base+859),uint16_t(base+860),uint16_t(base+861),uint16_t(base+862),uint16_t(base+863),uint16_t(base+864),uint16_t(base+865),uint16_t(base+866),uint16_t(base+867),
        uint16_t(base+868),uint16_t(base+869),uint16_t(base+870),uint16_t(base+871),uint16_t(base+872),uint16_t(base+873),uint16_t(base+874),uint16_t(base+875),uint16_t(base+876),uint16_t(base+877),uint16_t(base+878),uint16_t(base+879),
        uint16_t(base+880),uint16_t(base+881),uint16_t(base+882),uint16_t(base+883),uint16_t(base+884),uint16_t(base+885),uint16_t(base+886),uint16_t(base+887),uint16_t(base+888),uint16_t(base+889),uint16_t(base+890),uint16_t(base+891),
        uint16_t(base+892),uint16_t(base+893),uint16_t(base+894),uint16_t(base+895),uint16_t(base+896),uint16_t(base+897),uint16_t(base+898),uint16_t(base+899),uint16_t(base+900),uint16_t(base+901),uint16_t(base+902),uint16_t(base+903),
        uint16_t(base+904),uint16_t(base+905),uint16_t(base+906),uint16_t(base+907),uint16_t(base+908),uint16_t(base+909),uint16_t(base+910),uint16_t(base+911),uint16_t(base+912),uint16_t(base+913),uint16_t(base+914),uint16_t(base+915),
        uint16_t(base+916),uint16_t(base+917),uint16_t(base+918),uint16_t(base+919),uint16_t(base+920),uint16_t(base+921),uint16_t(base+922),uint16_t(base+923),uint16_t(base+924),uint16_t(base+925),uint16_t(base+926),uint16_t(base+927),
        uint16_t(base+928),uint16_t(base+929),uint16_t(base+930),uint16_t(base+931),uint16_t(base+932),uint16_t(base+933),uint16_t(base+934),uint16_t(base+935),uint16_t(base+936),uint16_t(base+937),uint16_t(base+938),uint16_t(base+939),
        uint16_t(base+940),uint16_t(base+941),uint16_t(base+942),uint16_t(base+943),uint16_t(base+944),uint16_t(base+945),uint16_t(base+946),uint16_t(base+947),uint16_t(base+948),uint16_t(base+949),uint16_t(base+950),uint16_t(base+951),
        uint16_t(base+952),uint16_t(base+953),uint16_t(base+954),uint16_t(base+955),uint16_t(base+956),uint16_t(base+957),uint16_t(base+958),uint16_t(base+959),uint16_t(base+960),uint16_t(base+961),uint16_t(base+962),uint16_t(base+963),
        uint16_t(base+964),uint16_t(base+965),uint16_t(base+966),uint16_t(base+967),uint16_t(base+968),uint16_t(base+969),uint16_t(base+970),uint16_t(base+971),uint16_t(base+972),uint16_t(base+973),uint16_t(base+974),uint16_t(base+975),
        uint16_t(base+976),uint16_t(base+977),uint16_t(base+978),uint16_t(base+979),uint16_t(base+980),uint16_t(base+981),uint16_t(base+982),uint16_t(base+983),uint16_t(base+984),uint16_t(base+985),uint16_t(base+986),uint16_t(base+987),
        uint16_t(base+988),uint16_t(base+989),uint16_t(base+990),uint16_t(base+991),uint16_t(base+992),uint16_t(base+993),uint16_t(base+994),uint16_t(base+995),uint16_t(base+996),uint16_t(base+997),uint16_t(base+998),uint16_t(base+999),
        uint16_t(base+1000),uint16_t(base+1001),uint16_t(base+1002),uint16_t(base+1003),uint16_t(base+1004),uint16_t(base+1005),uint16_t(base+1006),uint16_t(base+1007),uint16_t(base+1008),uint16_t(base+1009),uint16_t(base+1010),uint16_t(base+1011),
        uint16_t(base+1012),uint16_t(base+1013),uint16_t(base+1014),uint16_t(base+1015),uint16_t(base+1016),uint16_t(base+1017),uint16_t(base+1018),uint16_t(base+1019),uint16_t(base+1020),uint16_t(base+1021),uint16_t(base+1022),uint16_t(base+1023),
        uint16_t(base+1024),uint16_t(base+1025),uint16_t(base+1026),uint16_t(base+1027),uint16_t(base+1028),uint16_t(base+1029),uint16_t(base+1030),uint16_t(base+1031),uint16_t(base+1032),uint16_t(base+1033),uint16_t(base+1034),uint16_t(base+1035),
        uint16_t(base+1036),uint16_t(base+1037),uint16_t(base+1038),uint16_t(base+1039),uint16_t(base+1040),uint16_t(base+1041),uint16_t(base+1042),uint16_t(base+1043),uint16_t(base+1044),uint16_t(base+1045),uint16_t(base+1046),uint16_t(base+1047),
        uint16_t(base+1048),uint16_t(base+1049),uint16_t(base+1050),uint16_t(base+1051),uint16_t(base+1052),uint16_t(base+1053),uint16_t(base+1054),uint16_t(base+1055),uint16_t(base+1056),uint16_t(base+1057),uint16_t(base+1058),uint16_t(base+1059),
        uint16_t(base+1060),uint16_t(base+1061),uint16_t(base+1062),uint16_t(base+1063),uint16_t(base+1064),uint16_t(base+1065),uint16_t(base+1066),uint16_t(base+1067),uint16_t(base+1068),uint16_t(base+1069),uint16_t(base+1070),uint16_t(base+1071),
        uint16_t(base+1072),uint16_t(base+1073),uint16_t(base+1074),uint16_t(base+1075),uint16_t(base+1076),uint16_t(base+1077),uint16_t(base+1078),uint16_t(base+1079),uint16_t(base+1080),uint16_t(base+1081),uint16_t(base+1082),uint16_t(base+1083),
        uint16_t(base+1084),uint16_t(base+1085),uint16_t(base+1086),uint16_t(base+1087),uint16_t(base+1088),uint16_t(base+1089),uint16_t(base+1090),uint16_t(base+1091),uint16_t(base+1092),uint16_t(base+1093),uint16_t(base+1094),uint16_t(base+1095),
        uint16_t(base+1096),uint16_t(base+1097),uint16_t(base+1098),uint16_t(base+1099),uint16_t(base+1100),uint16_t(base+1101),uint16_t(base+1102),uint16_t(base+1103),uint16_t(base+1104),uint16_t(base+1105),uint16_t(base+1106),uint16_t(base+1107),
        uint16_t(base+1108),uint16_t(base+1109),uint16_t(base+1110),uint16_t(base+1111),uint16_t(base+1112),uint16_t(base+1113),uint16_t(base+1114),uint16_t(base+1115),uint16_t(base+1116),uint16_t(base+1117),uint16_t(base+1118),uint16_t(base+1119),
        uint16_t(base+1120),uint16_t(base+1121),uint16_t(base+1122),uint16_t(base+1123),uint16_t(base+1124),uint16_t(base+1125),uint16_t(base+1126),uint16_t(base+1127),uint16_t(base+1128),uint16_t(base+1129),uint16_t(base+1130),uint16_t(base+1131),
        uint16_t(base+1132),uint16_t(base+1133),uint16_t(base+1134),uint16_t(base+1135),uint16_t(base+1136),uint16_t(base+1137),uint16_t(base+1138),uint16_t(base+1139),uint16_t(base+1140),uint16_t(base+1141),uint16_t(base+1142),uint16_t(base+1143),
        uint16_t(base+1144),uint16_t(base+1145),uint16_t(base+1146),uint16_t(base+1147),uint16_t(base+1148),uint16_t(base+1149),uint16_t(base+1150),uint16_t(base+1151),uint16_t(base+1152),uint16_t(base+1153),uint16_t(base+1154),uint16_t(base+1155),
        uint16_t(base+1156),uint16_t(base+1157),uint16_t(base+1158),uint16_t(base+1159),uint16_t(base+1160),uint16_t(base+1161),uint16_t(base+1162),uint16_t(base+1163),uint16_t(base+1164),uint16_t(base+1165),uint16_t(base+1166),uint16_t(base+1167),
        uint16_t(base+1168),uint16_t(base+1169),uint16_t(base+1170),uint16_t(base+1171),uint16_t(base+1172),uint16_t(base+1173),uint16_t(base+1174),uint16_t(base+1175),uint16_t(base+1176),uint16_t(base+1177),uint16_t(base+1178),uint16_t(base+1179),
        uint16_t(base+1180),uint16_t(base+1181),uint16_t(base+1182),uint16_t(base+1183),uint16_t(base+1184),uint16_t(base+1185),uint16_t(base+1186),uint16_t(base+1187),uint16_t(base+1188),uint16_t(base+1189),uint16_t(base+1190),uint16_t(base+1191),
        uint16_t(base+1192),uint16_t(base+1193),uint16_t(base+1194),uint16_t(base+1195),uint16_t(base+1196),uint16_t(base+1197),uint16_t(base+1198),uint16_t(base+1199),uint16_t(base+1200),uint16_t(base+1201),uint16_t(base+1202),uint16_t(base+1203),
        uint16_t(base+1204),uint16_t(base+1205),uint16_t(base+1206),uint16_t(base+1207),uint16_t(base+1208),uint16_t(base+1209),uint16_t(base+1210),uint16_t(base+1211),uint16_t(base+1212),uint16_t(base+1213),uint16_t(base+1214),uint16_t(base+1215),
        uint16_t(base+1216),uint16_t(base+1217),uint16_t(base+1218),uint16_t(base+1219),uint16_t(base+1220),uint16_t(base+1221),uint16_t(base+1222),uint16_t(base+1223),uint16_t(base+1224),uint16_t(base+1225),uint16_t(base+1226),uint16_t(base+1227),
        uint16_t(base+1228),uint16_t(base+1229),uint16_t(base+1230),uint16_t(base+1231),uint16_t(base+1232),uint16_t(base+1233),uint16_t(base+1234),uint16_t(base+1235),uint16_t(base+1236),uint16_t(base+1237),uint16_t(base+1238),uint16_t(base+1239),
        uint16_t(base+1240),uint16_t(base+1241),uint16_t(base+1242),uint16_t(base+1243),uint16_t(base+1244),uint16_t(base+1245),uint16_t(base+1246),uint16_t(base+1247),uint16_t(base+1248),uint16_t(base+1249),uint16_t(base+1250),uint16_t(base+1251),
        uint16_t(base+1252),uint16_t(base+1253),uint16_t(base+1254),uint16_t(base+1255),uint16_t(base+1256),uint16_t(base+1257),uint16_t(base+1258),uint16_t(base+1259),uint16_t(base+1260),uint16_t(base+1261),uint16_t(base+1262),uint16_t(base+1263),
        uint16_t(base+1264),uint16_t(base+1265),uint16_t(base+1266),uint16_t(base+1267),uint16_t(base+1268),uint16_t(base+1269),uint16_t(base+1270),uint16_t(base+1271),uint16_t(base+1272),uint16_t(base+1273),uint16_t(base+1274),uint16_t(base+1275),
        uint16_t(base+1276),uint16_t(base+1277),uint16_t(base+1278),uint16_t(base+1279),uint16_t(base+1280),uint16_t(base+1281),uint16_t(base+1282),uint16_t(base+1283),uint16_t(base+1284),uint16_t(base+1285),uint16_t(base+1286),uint16_t(base+1287),
        uint16_t(base+1288),uint16_t(base+1289),uint16_t(base+1290),uint16_t(base+1291),uint16_t(base+1292),uint16_t(base+1293),uint16_t(base+1294),uint16_t(base+1295),uint16_t(base+1296),uint16_t(base+1297),uint16_t(base+1298),uint16_t(base+1299),
        uint16_t(base+1300),uint16_t(base+1301),uint16_t(base+1302),uint16_t(base+1303),uint16_t(base+1304),uint16_t(base+1305),uint16_t(base+1306),uint16_t(base+1307),uint16_t(base+1308),uint16_t(base+1309),uint16_t(base+1310),uint16_t(base+1311),
        uint16_t(base+1312),uint16_t(base+1313),uint16_t(base+1314),uint16_t(base+1315),uint16_t(base+1316),uint16_t(base+1317),uint16_t(base+1318),uint16_t(base+1319),uint16_t(base+1320),uint16_t(base+1321),uint16_t(base+1322),uint16_t(base+1323),
        uint16_t(base+1324),uint16_t(base+1325),uint16_t(base+1326),uint16_t(base+1327),uint16_t(base+1328),uint16_t(base+1329),uint16_t(base+1330),uint16_t(base+1331),uint16_t(base+1332),uint16_t(base+1333),uint16_t(base+1334),uint16_t(base+1335),
        uint16_t(base+1336),uint16_t(base+1337),uint16_t(base+1338),uint16_t(base+1339),uint16_t(base+1340),uint16_t(base+1341),uint16_t(base+1342),uint16_t(base+1343),uint16_t(base+1344),uint16_t(base+1345),uint16_t(base+1346),uint16_t(base+1347),
        uint16_t(base+1348),uint16_t(base+1349),uint16_t(base+1350),uint16_t(base+1351),uint16_t(base+1352),uint16_t(base+1353),uint16_t(base+1354),uint16_t(base+1355),uint16_t(base+1356),uint16_t(base+1357),uint16_t(base+1358),uint16_t(base+1359),
        uint16_t(base+1360),uint16_t(base+1361),uint16_t(base+1362),uint16_t(base+1363),uint16_t(base+1364),uint16_t(base+1365),uint16_t(base+1366),uint16_t(base+1367),uint16_t(base+1368),uint16_t(base+1369),uint16_t(base+1370),uint16_t(base+1371),
        uint16_t(base+1372),uint16_t(base+1373),uint16_t(base+1374),uint16_t(base+1375),uint16_t(base+1376),uint16_t(base+1377),uint16_t(base+1378),uint16_t(base+1379),uint16_t(base+1380),uint16_t(base+1381),uint16_t(base+1382),uint16_t(base+1383),
        uint16_t(base+1384),uint16_t(base+1385),uint16_t(base+1386),uint16_t(base+1387),uint16_t(base+1388),uint16_t(base+1389),uint16_t(base+1390),uint16_t(base+1391),uint16_t(base+1382),uint16_t(base+1392),uint16_t(base+1393),uint16_t(base+1394),
        uint16_t(base+1395),uint16_t(base+1396),uint16_t(base+1397),uint16_t(base+1380),uint16_t(base+1398),uint16_t(base+1399),uint16_t(base+1400),uint16_t(base+1401),uint16_t(base+1402),uint16_t(base+1403),uint16_t(base+1404),uint16_t(base+1405),
        uint16_t(base+1399),uint16_t(base+1406),uint16_t(base+1407),uint16_t(base+1408),uint16_t(base+1409),uint16_t(base+1391),uint16_t(base+1407),uint16_t(base+1410),uint16_t(base+1411),uint16_t(base+1412),uint16_t(base+1413),uint16_t(base+1409),
        uint16_t(base+1414),uint16_t(base+1415),uint16_t(base+1416),uint16_t(base+1417),uint16_t(base+1418),uint16_t(base+1419),uint16_t(base+1420),uint16_t(base+1421),uint16_t(base+1422),uint16_t(base+1423),uint16_t(base+1424),uint16_t(base+1425),
        uint16_t(base+1411),uint16_t(base+1426),uint16_t(base+1427),uint16_t(base+1428),uint16_t(base+1429),uint16_t(base+1413),uint16_t(base+1430),uint16_t(base+1431),uint16_t(base+1432),uint16_t(base+1433),uint16_t(base+1434),uint16_t(base+1435),
        uint16_t(base+1427),uint16_t(base+1436),uint16_t(base+1437),uint16_t(base+1438),uint16_t(base+1439),uint16_t(base+1429),uint16_t(base+1437),uint16_t(base+1440),uint16_t(base+1441),uint16_t(base+1442),uint16_t(base+1443),uint16_t(base+1439),
        uint16_t(base+1444),uint16_t(base+1445),uint16_t(base+1446),uint16_t(base+1447),uint16_t(base+1448),uint16_t(base+1449),uint16_t(base+1450),uint16_t(base+1451),uint16_t(base+1452),uint16_t(base+1453),uint16_t(base+1454),uint16_t(base+1455),
        uint16_t(base+1456),uint16_t(base+1457),uint16_t(base+1458),uint16_t(base+1459),uint16_t(base+1460),uint16_t(base+1461),uint16_t(base+1462),uint16_t(base+1463),uint16_t(base+1464),uint16_t(base+1465),uint16_t(base+1466),uint16_t(base+1467),
        uint16_t(base+1468),uint16_t(base+1469),uint16_t(base+1470),uint16_t(base+1471),uint16_t(base+1472),uint16_t(base+1473),uint16_t(base+1474),uint16_t(base+1475),uint16_t(base+1476),uint16_t(base+1477),uint16_t(base+1478),uint16_t(base+1479),
        uint16_t(base+1480),uint16_t(base+1481),uint16_t(base+1482),uint16_t(base+1483),uint16_t(base+1484),uint16_t(base+1485),uint16_t(base+1486),uint16_t(base+1487),uint16_t(base+1488),uint16_t(base+1489),uint16_t(base+1490),uint16_t(base+1491),
        uint16_t(base+1492),uint16_t(base+1493),uint16_t(base+1494),uint16_t(base+1495),uint16_t(base+1496),uint16_t(base+1497),uint16_t(base+1498),uint16_t(base+1499),uint16_t(base+1500),uint16_t(base+1501),uint16_t(base+1502),uint16_t(base+1503),
        uint16_t(base+1504),uint16_t(base+1505),uint16_t(base+1506),uint16_t(base+1507),uint16_t(base+1508),uint16_t(base+1509),uint16_t(base+1510),uint16_t(base+1511),uint16_t(base+1512),uint16_t(base+1513),uint16_t(base+1514),uint16_t(base+1515),
        uint16_t(base+1516),uint16_t(base+1517),uint16_t(base+1518),uint16_t(base+1519),uint16_t(base+1520),uint16_t(base+1521),uint16_t(base+1522),uint16_t(base+1523),uint16_t(base+1524),uint16_t(base+1525),uint16_t(base+1526),uint16_t(base+1527),
        uint16_t(base+1528),uint16_t(base+1529),uint16_t(base+1530),uint16_t(base+1531),uint16_t(base+1532),uint16_t(base+1533),uint16_t(base+1534),uint16_t(base+1535),uint16_t(base+1536),uint16_t(base+1537),uint16_t(base+1538),uint16_t(base+1539),
        uint16_t(base+1540),uint16_t(base+1541),uint16_t(base+1542),uint16_t(base+1543),uint16_t(base+1544),uint16_t(base+1545),uint16_t(base+1546),uint16_t(base+1547),uint16_t(base+1548),uint16_t(base+1549),uint16_t(base+1550),uint16_t(base+1551),
        uint16_t(base+1552),uint16_t(base+1553),uint16_t(base+1554),uint16_t(base+1555),uint16_t(base+1556),uint16_t(base+1557),uint16_t(base+1558),uint16_t(base+1559),uint16_t(base+1560),uint16_t(base+1561),uint16_t(base+1562),uint16_t(base+1563),
        uint16_t(base+1564),uint16_t(base+1565),uint16_t(base+1566),uint16_t(base+1567),uint16_t(base+1568),uint16_t(base+1569),uint16_t(base+1570),uint16_t(base+1571),uint16_t(base+1572),uint16_t(base+1573),uint16_t(base+1574),uint16_t(base+1575),
        uint16_t(base+1576),uint16_t(base+1577),uint16_t(base+1578),uint16_t(base+1579),uint16_t(base+1580),uint16_t(base+1581),uint16_t(base+1582),uint16_t(base+1583),uint16_t(base+1584),uint16_t(base+1585),uint16_t(base+1586),uint16_t(base+1587),
        uint16_t(base+1588),uint16_t(base+1589),uint16_t(base+1590),uint16_t(base+1591),uint16_t(base+1592),uint16_t(base+1593),uint16_t(base+1594),uint16_t(base+1595),uint16_t(base+1596),uint16_t(base+1597),uint16_t(base+1598),uint16_t(base+1599),
        uint16_t(base+1600),uint16_t(base+1601),uint16_t(base+1602),uint16_t(base+1603),uint16_t(base+1604),uint16_t(base+1605),uint16_t(base+1606),uint16_t(base+1607),uint16_t(base+1608),uint16_t(base+1609),uint16_t(base+1610),uint16_t(base+1611),
        uint16_t(base+1612),uint16_t(base+1613),uint16_t(base+1614),uint16_t(base+1615),uint16_t(base+1616),uint16_t(base+1617),uint16_t(base+1618),uint16_t(base+1619),uint16_t(base+1620),uint16_t(base+1621),uint16_t(base+1622),uint16_t(base+1623),
        uint16_t(base+1624),uint16_t(base+1625),uint16_t(base+1626),uint16_t(base+1627),uint16_t(base+1628),uint16_t(base+1629),uint16_t(base+1630),uint16_t(base+1631),uint16_t(base+1632),uint16_t(base+1633),uint16_t(base+1634),uint16_t(base+1635),
        uint16_t(base+1636),uint16_t(base+1637),uint16_t(base+1638),uint16_t(base+1639),uint16_t(base+1640),uint16_t(base+1641),uint16_t(base+1642),uint16_t(base+1643),uint16_t(base+1644),uint16_t(base+1645),uint16_t(base+1646),uint16_t(base+1647),
        uint16_t(base+1648),uint16_t(base+1649),uint16_t(base+1650),uint16_t(base+1651),uint16_t(base+1652),uint16_t(base+1653),uint16_t(base+1654),uint16_t(base+1655),uint16_t(base+1656),uint16_t(base+1657),uint16_t(base+1658),uint16_t(base+1659),
        uint16_t(base+1660),uint16_t(base+1661),uint16_t(base+1662),uint16_t(base+1663),uint16_t(base+1664),uint16_t(base+1665),uint16_t(base+1666),uint16_t(base+1667),uint16_t(base+1668),uint16_t(base+1669),uint16_t(base+1670),uint16_t(base+1671),
        uint16_t(base+1672),uint16_t(base+1673),uint16_t(base+1674),uint16_t(base+1675),uint16_t(base+1676),uint16_t(base+1677),uint16_t(base+1678),uint16_t(base+1679),uint16_t(base+1680),uint16_t(base+1681),uint16_t(base+1682),uint16_t(base+1683),
        uint16_t(base+1684),uint16_t(base+1685),uint16_t(base+1686),uint16_t(base+1687),uint16_t(base+1688),uint16_t(base+1689),uint16_t(base+1690),uint16_t(base+1691),uint16_t(base+1692),uint16_t(base+1693),uint16_t(base+1694),uint16_t(base+1695),
        uint16_t(base+1696),uint16_t(base+1697),uint16_t(base+1698),uint16_t(base+1699),uint16_t(base+1700),uint16_t(base+1701),uint16_t(base+1702),uint16_t(base+1703),uint16_t(base+1704),uint16_t(base+1705),uint16_t(base+1706),uint16_t(base+1707),
        uint16_t(base+1708),uint16_t(base+1709),uint16_t(base+1710),uint16_t(base+1711),uint16_t(base+1712),uint16_t(base+1713),uint16_t(base+1714),uint16_t(base+1715),uint16_t(base+1715),uint16_t(base+1716),uint16_t(base+1717),uint16_t(base+1716),
        uint16_t(base+1718),uint16_t(base+1719),uint16_t(base+1719),uint16_t(base+1720),uint16_t(base+1721),uint16_t(base+1720),uint16_t(base+1722),uint16_t(base+1723),uint16_t(base+1724),uint16_t(base+1725),uint16_t(base+1726),uint16_t(base+1727),
        uint16_t(base+1728),uint16_t(base+1729),uint16_t(base+1730),uint16_t(base+1731),uint16_t(base+1732),uint16_t(base+1733),uint16_t(base+1734),uint16_t(base+1735),uint16_t(base+1736),uint16_t(base+1737),uint16_t(base+1738),uint16_t(base+1739),
        uint16_t(base+1740),uint16_t(base+1741),uint16_t(base+1729),uint16_t(base+1733),uint16_t(base+1742),uint16_t(base+1743),uint16_t(base+1744),uint16_t(base+1745),uint16_t(base+1741),uint16_t(base+1743),uint16_t(base+1746),uint16_t(base+1747),
        uint16_t(base+1748),uint16_t(base+1749),uint16_t(base+1750),uint16_t(base+1751),uint16_t(base+1752),uint16_t(base+1753),uint16_t(base+1754),uint16_t(base+1755),uint16_t(base+1756),uint16_t(base+1757),uint16_t(base+1758),uint16_t(base+1759),
        uint16_t(base+1760),uint16_t(base+1761),uint16_t(base+1745),uint16_t(base+1747),uint16_t(base+1762),uint16_t(base+1763),uint16_t(base+1764),uint16_t(base+1765),uint16_t(base+1766),uint16_t(base+1767),uint16_t(base+1768),uint16_t(base+1761),
        uint16_t(base+1763),uint16_t(base+1769),uint16_t(base+1770),uint16_t(base+1771),uint16_t(base+1772),uint16_t(base+1773),uint16_t(base+1774),uint16_t(base+1775),uint16_t(base+1776),uint16_t(base+1777),uint16_t(base+1778),uint16_t(base+1779),
        uint16_t(base+1780),uint16_t(base+1781),uint16_t(base+1782),uint16_t(base+1783),uint16_t(base+1784),uint16_t(base+1785),uint16_t(base+1786),uint16_t(base+1787),uint16_t(base+1788),uint16_t(base+1789),uint16_t(base+1790),uint16_t(base+1791),
        uint16_t(base+1792),uint16_t(base+1793),uint16_t(base+1794),uint16_t(base+1795),uint16_t(base+1796),uint16_t(base+1797),uint16_t(base+1798),uint16_t(base+1799),uint16_t(base+1800),uint16_t(base+1801),uint16_t(base+1802),uint16_t(base+1803),
        uint16_t(base+1804),uint16_t(base+1805),uint16_t(base+1806),uint16_t(base+1807),uint16_t(base+1808),uint16_t(base+1809),uint16_t(base+1810),uint16_t(base+1811),uint16_t(base+1812),uint16_t(base+1813),uint16_t(base+1814),uint16_t(base+1815),
        uint16_t(base+1816),uint16_t(base+1817),uint16_t(base+1818),uint16_t(base+1819),uint16_t(base+1820),uint16_t(base+1821),uint16_t(base+1822),uint16_t(base+1823),uint16_t(base+1824),uint16_t(base+1825),uint16_t(base+1826),uint16_t(base+1827),
        uint16_t(base+1828),uint16_t(base+1829),uint16_t(base+1830),uint16_t(base+1831),uint16_t(base+1832),uint16_t(base+1833),uint16_t(base+1834),uint16_t(base+1835),uint16_t(base+1836),uint16_t(base+1837),uint16_t(base+1838),uint16_t(base+1839),
        uint16_t(base+1840),uint16_t(base+1841),uint16_t(base+1840),uint16_t(base+1842),uint16_t(base+1843),uint16_t(base+1844),uint16_t(base+1845),uint16_t(base+1846),uint16_t(base+1847),uint16_t(base+1848),uint16_t(base+1849),uint16_t(base+1850),
        uint16_t(base+1851),uint16_t(base+1852),uint16_t(base+1853),uint16_t(base+1841),uint16_t(base+1854),uint16_t(base+1855),uint16_t(base+1856),uint16_t(base+1841),uint16_t(base+1841),uint16_t(base+1857),uint16_t(base+1858),uint16_t(base+1859),
        uint16_t(base+1860),uint16_t(base+1861),uint16_t(base+1862),uint16_t(base+1863),uint16_t(base+1864),uint16_t(base+1865),uint16_t(base+1866),uint16_t(base+1867),uint16_t(base+1868),uint16_t(base+1869),uint16_t(base+1870),uint16_t(base+1871),
        uint16_t(base+1872),uint16_t(base+1873),uint16_t(base+1874),uint16_t(base+1875),uint16_t(base+1876),uint16_t(base+1877),uint16_t(base+1878),uint16_t(base+1879),uint16_t(base+1880),uint16_t(base+1881),uint16_t(base+1882),uint16_t(base+1883),
        uint16_t(base+1884),uint16_t(base+1885),uint16_t(base+1886),uint16_t(base+1887),uint16_t(base+1888),uint16_t(base+1889),uint16_t(base+1890),uint16_t(base+1891),uint16_t(base+1892),uint16_t(base+1893),uint16_t(base+1894),uint16_t(base+1895),
        uint16_t(base+1896),uint16_t(base+1897),uint16_t(base+1898),uint16_t(base+1899),uint16_t(base+1900),uint16_t(base+1901),uint16_t(base+1902),uint16_t(base+1903),uint16_t(base+1904),uint16_t(base+1905),uint16_t(base+1906),uint16_t(base+1907),
        uint16_t(base+1908),uint16_t(base+1909),uint16_t(base+1910),uint16_t(base+1911),uint16_t(base+1912),uint16_t(base+1913),uint16_t(base+1914),uint16_t(base+1915),uint16_t(base+1916),uint16_t(base+1917),uint16_t(base+1918),uint16_t(base+1919),
        uint16_t(base+1920),uint16_t(base+1921),uint16_t(base+1922),uint16_t(base+1923),uint16_t(base+1924),uint16_t(base+1925),uint16_t(base+1926),uint16_t(base+1927),uint16_t(base+1928),uint16_t(base+1929),uint16_t(base+1930),uint16_t(base+1931),
        uint16_t(base+1932),uint16_t(base+1933),uint16_t(base+1934),uint16_t(base+1935),uint16_t(base+1936),uint16_t(base+1937),uint16_t(base+1938),uint16_t(base+1939),uint16_t(base+1940),uint16_t(base+1941),uint16_t(base+1942),uint16_t(base+1943),
        uint16_t(base+1944),uint16_t(base+1945),uint16_t(base+1946),uint16_t(base+1947),uint16_t(base+1948),uint16_t(base+1949),uint16_t(base+1950),uint16_t(base+1951),uint16_t(base+1952),uint16_t(base+1953),uint16_t(base+1954),uint16_t(base+1955),
        uint16_t(base+9),uint16_t(base+1956),uint16_t(base+10),uint16_t(base+12),uint16_t(base+1957),uint16_t(base+13),uint16_t(base+1958),uint16_t(base+1959),uint16_t(base+1960),uint16_t(base+1961),uint16_t(base+1962),uint16_t(base+1963),
        uint16_t(base+1964),uint16_t(base+1965),uint16_t(base+1966),uint16_t(base+1967),uint16_t(base+1968),uint16_t(base+1969),uint16_t(base+1970),uint16_t(base+1971),uint16_t(base+1972),uint16_t(base+1973),uint16_t(base+1974),uint16_t(base+1975),
        uint16_t(base+1976),uint16_t(base+1977),uint16_t(base+1978),uint16_t(base+1979),uint16_t(base+1980),uint16_t(base+1981),uint16_t(base+1982),uint16_t(base+1983),uint16_t(base+1984),uint16_t(base+1985),uint16_t(base+1986),uint16_t(base+1987),
        uint16_t(base+1988),uint16_t(base+1989),uint16_t(base+1990),uint16_t(base+1991),uint16_t(base+1992),uint16_t(base+1993),uint16_t(base+1994),uint16_t(base+1995),uint16_t(base+1996),uint16_t(base+1997),uint16_t(base+1998),uint16_t(base+1999),
        uint16_t(base+2000),uint16_t(base+2001),uint16_t(base+2002),uint16_t(base+2003),uint16_t(base+2004),uint16_t(base+2005),uint16_t(base+2006),uint16_t(base+2007),uint16_t(base+2008),uint16_t(base+2009),uint16_t(base+2010),uint16_t(base+2011),
        uint16_t(base+2012),uint16_t(base+2013),uint16_t(base+2014),uint16_t(base+2015),uint16_t(base+2016),uint16_t(base+2017),uint16_t(base+2018),uint16_t(base+2019),uint16_t(base+2020),uint16_t(base+2021),uint16_t(base+2022),uint16_t(base+2023),
        uint16_t(base+2024),uint16_t(base+2025),uint16_t(base+2026),uint16_t(base+2027),uint16_t(base+2028),uint16_t(base+2029),uint16_t(base+2030),uint16_t(base+2031),uint16_t(base+2032),uint16_t(base+2033),uint16_t(base+2034),uint16_t(base+2035),
        uint16_t(base+2036),uint16_t(base+2037),uint16_t(base+2038),uint16_t(base+2039),uint16_t(base+2040),uint16_t(base+2041),uint16_t(base+2042),uint16_t(base+2043),uint16_t(base+2044),uint16_t(base+2045),uint16_t(base+2046),uint16_t(base+2047),
        uint16_t(base+2048),uint16_t(base+2049),uint16_t(base+2050),uint16_t(base+2051),uint16_t(base+2052),uint16_t(base+2053),uint16_t(base+2054),uint16_t(base+2055),uint16_t(base+2056),uint16_t(base+2057),uint16_t(base+2058),uint16_t(base+2059),
        uint16_t(base+2060),uint16_t(base+2061),uint16_t(base+2062),uint16_t(base+2063),uint16_t(base+2064),uint16_t(base+2065),uint16_t(base+2066),uint16_t(base+2067),uint16_t(base+2068),uint16_t(base+2069),uint16_t(base+2070),uint16_t(base+2071),
        uint16_t(base+126),uint16_t(base+2072),uint16_t(base+127),uint16_t(base+2073),uint16_t(base+2074),uint16_t(base+2075),uint16_t(base+2076),uint16_t(base+2077),uint16_t(base+2078),uint16_t(base+135),uint16_t(base+2079),uint16_t(base+136),
        uint16_t(base+2080),uint16_t(base+2081),uint16_t(base+2082),uint16_t(base+2083),uint16_t(base+2084),uint16_t(base+2085),uint16_t(base+144),uint16_t(base+2086),uint16_t(base+145),uint16_t(base+2087),uint16_t(base+2088),uint16_t(base+2089),
        uint16_t(base+2090),uint16_t(base+2091),uint16_t(base+2092),uint16_t(base+153),uint16_t(base+2093),uint16_t(base+154),uint16_t(base+2094),uint16_t(base+2095),uint16_t(base+2096),uint16_t(base+159),uint16_t(base+2097),uint16_t(base+160),
        uint16_t(base+2098),uint16_t(base+2099),uint16_t(base+2100),uint16_t(base+2101),uint16_t(base+2102),uint16_t(base+2103),uint16_t(base+168),uint16_t(base+2104),uint16_t(base+169),uint16_t(base+2105),uint16_t(base+2106),uint16_t(base+2107),
        uint16_t(base+2108),uint16_t(base+2109),uint16_t(base+2110),uint16_t(base+177),uint16_t(base+2111),uint16_t(base+178),uint16_t(base+2112),uint16_t(base+2113),uint16_t(base+2114),uint16_t(base+183),uint16_t(base+2115),uint16_t(base+184),
        uint16_t(base+2116),uint16_t(base+2117),uint16_t(base+2118),uint16_t(base+189),uint16_t(base+2119),uint16_t(base+190),uint16_t(base+2120),uint16_t(base+2121),uint16_t(base+2122),uint16_t(base+195),uint16_t(base+2123),uint16_t(base+196),
        uint16_t(base+2124),uint16_t(base+2125),uint16_t(base+2126),uint16_t(base+201),uint16_t(base+2127),uint16_t(base+202),uint16_t(base+204),uint16_t(base+2128),uint16_t(base+205),uint16_t(base+207),uint16_t(base+2129),uint16_t(base+208),
        uint16_t(base+210),uint16_t(base+2130),uint16_t(base+211),uint16_t(base+2131),uint16_t(base+2132),uint16_t(base+2133),uint16_t(base+216),uint16_t(base+217),uint16_t(base+217),uint16_t(base+218),uint16_t(base+219),uint16_t(base+219),
        uint16_t(base+220),uint16_t(base+221),uint16_t(base+221),uint16_t(base+217),uint16_t(base+222),uint16_t(base+222),uint16_t(base+223),uint16_t(base+224),uint16_t(base+224),uint16_t(base+225),uint16_t(base+226),uint16_t(base+226),
        uint16_t(base+227),uint16_t(base+228),uint16_t(base+228),uint16_t(base+229),uint16_t(base+230),uint16_t(base+230),uint16_t(base+219),uint16_t(base+231),uint16_t(base+231),uint16_t(base+232),uint16_t(base+233),uint16_t(base+233),
        uint16_t(base+234),uint16_t(base+235),uint16_t(base+235),uint16_t(base+236),uint16_t(base+223),uint16_t(base+223),uint16_t(base+226),uint16_t(base+237),uint16_t(base+237),uint16_t(base+235),uint16_t(base+238),uint16_t(base+238),
        uint16_t(base+222),uint16_t(base+239),uint16_t(base+239),uint16_t(base+237),uint16_t(base+240),uint16_t(base+240),uint16_t(base+241),uint16_t(base+227),uint16_t(base+227),uint16_t(base+242),uint16_t(base+243),uint16_t(base+243),
        uint16_t(base+221),uint16_t(base+244),uint16_t(base+244),uint16_t(base+245),uint16_t(base+241),uint16_t(base+241),uint16_t(base+240),uint16_t(base+246),uint16_t(base+246),uint16_t(base+246),uint16_t(base+247),uint16_t(base+247),
        uint16_t(base+243),uint16_t(base+232),uint16_t(base+232),uint16_t(base+247),uint16_t(base+229),uint16_t(base+229),uint16_t(base+248),uint16_t(base+249),uint16_t(base+249),uint16_t(base+250),uint16_t(base+251),uint16_t(base+251),
        uint16_t(base+238),uint16_t(base+252),uint16_t(base+252),uint16_t(base+251),uint16_t(base+248),uint16_t(base+248),uint16_t(base+224),uint16_t(base+253),uint16_t(base+253),uint16_t(base+254),uint16_t(base+250),uint16_t(base+250),
        uint16_t(base+255),uint16_t(base+225),uint16_t(base+225),uint16_t(base+256),uint16_t(base+220),uint16_t(base+220),uint16_t(base+253),uint16_t(base+254),uint16_t(base+254),uint16_t(base+257),uint16_t(base+216),uint16_t(base+216),
        uint16_t(base+231),uint16_t(base+242),uint16_t(base+242),uint16_t(base+252),uint16_t(base+257),uint16_t(base+257),uint16_t(base+239),uint16_t(base+255),uint16_t(base+255),uint16_t(base+258),uint16_t(base+256),uint16_t(base+256),
        uint16_t(base+249),uint16_t(base+234),uint16_t(base+234),uint16_t(base+228),uint16_t(base+258),uint16_t(base+258),uint16_t(base+233),uint16_t(base+259),uint16_t(base+259),uint16_t(base+230),uint16_t(base+260),uint16_t(base+260),
        uint16_t(base+259),uint16_t(base+245),uint16_t(base+245),uint16_t(base+260),uint16_t(base+261),uint16_t(base+261),uint16_t(base+261),uint16_t(base+218),uint16_t(base+218),uint16_t(base+244),uint16_t(base+236),uint16_t(base+236),
        uint16_t(base+2134),uint16_t(base+2135),uint16_t(base+2136),uint16_t(base+2137),uint16_t(base+2138),uint16_t(base+2139),uint16_t(base+2140),uint16_t(base+2141),uint16_t(base+2142),uint16_t(base+271),uint16_t(base+2143),uint16_t(base+272),
        uint16_t(base+274),uint16_t(base+2144),uint16_t(base+275),uint16_t(base+2145),uint16_t(base+2146),uint16_t(base+2147),uint16_t(base+2148),uint16_t(base+2149),uint16_t(base+2150),uint16_t(base+2151),uint16_t(base+2152),uint16_t(base+2153),
        uint16_t(base+2154),uint16_t(base+2155),uint16_t(base+2156),uint16_t(base+2157),uint16_t(base+2158),uint16_t(base+2159),uint16_t(base+2160),uint16_t(base+2161),uint16_t(base+2162),uint16_t(base+2163),uint16_t(base+2164),uint16_t(base+2165),
        uint16_t(base+2166),uint16_t(base+2167),uint16_t(base+2168),uint16_t(base+2169),uint16_t(base+2170),uint16_t(base+2171),uint16_t(base+2172),uint16_t(base+2173),uint16_t(base+2174),uint16_t(base+2175),uint16_t(base+2176),uint16_t(base+2177),
        uint16_t(base+2178),uint16_t(base+2179),uint16_t(base+2180),uint16_t(base+2181),uint16_t(base+2182),uint16_t(base+2183),uint16_t(base+2184),uint16_t(base+2185),uint16_t(base+2186),uint16_t(base+2187),uint16_t(base+2188),uint16_t(base+2189),
        uint16_t(base+2190),uint16_t(base+2191),uint16_t(base+2192),uint16_t(base+2193),uint16_t(base+2194),uint16_t(base+2195),uint16_t(base+2196),uint16_t(base+2197),uint16_t(base+2198),uint16_t(base+2199),uint16_t(base+2200),uint16_t(base+2201),
        uint16_t(base+2202),uint16_t(base+2203),uint16_t(base+2204),uint16_t(base+2205),uint16_t(base+2206),uint16_t(base+2207),uint16_t(base+2208),uint16_t(base+2209),uint16_t(base+2210),uint16_t(base+343),uint16_t(base+2211),uint16_t(base+344),
        uint16_t(base+2212),uint16_t(base+2213),uint16_t(base+2214),uint16_t(base+2215),uint16_t(base+2216),uint16_t(base+2217),uint16_t(base+2218),uint16_t(base+2219),uint16_t(base+2220),uint16_t(base+2221),uint16_t(base+2222),uint16_t(base+2223),
        uint16_t(base+2224),uint16_t(base+2225),uint16_t(base+2226),uint16_t(base+361),uint16_t(base+2227),uint16_t(base+362),uint16_t(base+2228),uint16_t(base+2229),uint16_t(base+2230),uint16_t(base+2231),uint16_t(base+2232),uint16_t(base+2233),
        uint16_t(base+2234),uint16_t(base+2235),uint16_t(base+2236),uint16_t(base+2237),uint16_t(base+2238),uint16_t(base+2239),uint16_t(base+2240),uint16_t(base+2241),uint16_t(base+2242),uint16_t(base+2243),uint16_t(base+2244),uint16_t(base+2245),
        uint16_t(base+2246),uint16_t(base+2247),uint16_t(base+2248),uint16_t(base+2249),uint16_t(base+2250),uint16_t(base+2251),uint16_t(base+2252),uint16_t(base+2253),uint16_t(base+2254),uint16_t(base+2255),uint16_t(base+2256),uint16_t(base+2257),
        uint16_t(base+2258),uint16_t(base+2259),uint16_t(base+2260),uint16_t(base+2261),uint16_t(base+2262),uint16_t(base+2263),uint16_t(base+2264),uint16_t(base+2265),uint16_t(base+2266),uint16_t(base+2267),uint16_t(base+2268),uint16_t(base+2269),
        uint16_t(base+2270),uint16_t(base+2271),uint16_t(base+2272),uint16_t(base+2273),uint16_t(base+2274),uint16_t(base+2275),uint16_t(base+2276),uint16_t(base+2277),uint16_t(base+2278),uint16_t(base+2279),uint16_t(base+2280),uint16_t(base+2281),
        uint16_t(base+2282),uint16_t(base+2283),uint16_t(base+2284),uint16_t(base+2285),uint16_t(base+2286),uint16_t(base+2287),uint16_t(base+2288),uint16_t(base+2289),uint16_t(base+2290),uint16_t(base+2291),uint16_t(base+2292),uint16_t(base+2293),
        uint16_t(base+2294),uint16_t(base+2295),uint16_t(base+2296),uint16_t(base+433),uint16_t(base+2297),uint16_t(base+434),uint16_t(base+436),uint16_t(base+2298),uint16_t(base+437),uint16_t(base+2299),uint16_t(base+2300),uint16_t(base+2301),
        uint16_t(base+442),uint16_t(base+2302),uint16_t(base+443),uint16_t(base+445),uint16_t(base+2303),uint16_t(base+446),uint16_t(base+448),uint16_t(base+2304),uint16_t(base+449),uint16_t(base+451),uint16_t(base+2305),uint16_t(base+452),
        uint16_t(base+454),uint16_t(base+2306),uint16_t(base+455),uint16_t(base+457),uint16_t(base+2307),uint16_t(base+458),uint16_t(base+460),uint16_t(base+2308),uint16_t(base+461),uint16_t(base+463),uint16_t(base+2309),uint16_t(base+464),
        uint16_t(base+466),uint16_t(base+2310),uint16_t(base+467),uint16_t(base+469),uint16_t(base+2311),uint16_t(base+470),uint16_t(base+472),uint16_t(base+2312),uint16_t(base+473),uint16_t(base+475),uint16_t(base+2313),uint16_t(base+476),
        uint16_t(base+478),uint16_t(base+2314),uint16_t(base+479),uint16_t(base+481),uint16_t(base+2315),uint16_t(base+482),uint16_t(base+484),uint16_t(base+2316),uint16_t(base+485),uint16_t(base+487),uint16_t(base+2317),uint16_t(base+488),
        uint16_t(base+490),uint16_t(base+2318),uint16_t(base+491),uint16_t(base+493),uint16_t(base+2319),uint16_t(base+494),uint16_t(base+496),uint16_t(base+2320),uint16_t(base+497),uint16_t(base+499),uint16_t(base+2321),uint16_t(base+500),
        uint16_t(base+502),uint16_t(base+2322),uint16_t(base+503),uint16_t(base+505),uint16_t(base+2323),uint16_t(base+506),uint16_t(base+508),uint16_t(base+2324),uint16_t(base+509),uint16_t(base+511),uint16_t(base+2325),uint16_t(base+512),
        uint16_t(base+514),uint16_t(base+2326),uint16_t(base+515),uint16_t(base+517),uint16_t(base+2327),uint16_t(base+518),uint16_t(base+520),uint16_t(base+2328),uint16_t(base+521),uint16_t(base+523),uint16_t(base+2329),uint16_t(base+524),
        uint16_t(base+526),uint16_t(base+2330),uint16_t(base+527),uint16_t(base+529),uint16_t(base+2331),uint16_t(base+530),uint16_t(base+532),uint16_t(base+2332),uint16_t(base+533),uint16_t(base+535),uint16_t(base+2333),uint16_t(base+536),
        uint16_t(base+538),uint16_t(base+2334),uint16_t(base+539),uint16_t(base+541),uint16_t(base+2335),uint16_t(base+542),uint16_t(base+544),uint16_t(base+2336),uint16_t(base+545),uint16_t(base+547),uint16_t(base+2337),uint16_t(base+548),
        uint16_t(base+550),uint16_t(base+2338),uint16_t(base+551),uint16_t(base+553),uint16_t(base+2339),uint16_t(base+554),uint16_t(base+556),uint16_t(base+2340),uint16_t(base+557),uint16_t(base+559),uint16_t(base+2341),uint16_t(base+560),
        uint16_t(base+562),uint16_t(base+2342),uint16_t(base+563),uint16_t(base+565),uint16_t(base+2343),uint16_t(base+566),uint16_t(base+568),uint16_t(base+2344),uint16_t(base+569),uint16_t(base+571),uint16_t(base+2345),uint16_t(base+572),
        uint16_t(base+574),uint16_t(base+2346),uint16_t(base+575),uint16_t(base+577),uint16_t(base+2347),uint16_t(base+578),uint16_t(base+580),uint16_t(base+2348),uint16_t(base+581),uint16_t(base+583),uint16_t(base+2349),uint16_t(base+584),
        uint16_t(base+586),uint16_t(base+2350),uint16_t(base+587),uint16_t(base+589),uint16_t(base+2351),uint16_t(base+590),uint16_t(base+592),uint16_t(base+2352),uint16_t(base+593),uint16_t(base+595),uint16_t(base+2353),uint16_t(base+596),
        uint16_t(base+598),uint16_t(base+2354),uint16_t(base+599),uint16_t(base+601),uint16_t(base+2355),uint16_t(base+602),uint16_t(base+604),uint16_t(base+2356),uint16_t(base+605),uint16_t(base+607),uint16_t(base+2357),uint16_t(base+608),
        uint16_t(base+610),uint16_t(base+2358),uint16_t(base+611),uint16_t(base+613),uint16_t(base+2359),uint16_t(base+614),uint16_t(base+616),uint16_t(base+2360),uint16_t(base+617),uint16_t(base+619),uint16_t(base+2361),uint16_t(base+620),
        uint16_t(base+622),uint16_t(base+2362),uint16_t(base+623),uint16_t(base+625),uint16_t(base+2363),uint16_t(base+626),uint16_t(base+628),uint16_t(base+2364),uint16_t(base+629),uint16_t(base+631),uint16_t(base+2365),uint16_t(base+632),
        uint16_t(base+634),uint16_t(base+2366),uint16_t(base+635),uint16_t(base+637),uint16_t(base+2367),uint16_t(base+638),uint16_t(base+640),uint16_t(base+2368),uint16_t(base+641),uint16_t(base+643),uint16_t(base+2369),uint16_t(base+644),
        uint16_t(base+646),uint16_t(base+2370),uint16_t(base+647),uint16_t(base+649),uint16_t(base+2371),uint16_t(base+650),uint16_t(base+652),uint16_t(base+2372),uint16_t(base+653),uint16_t(base+655),uint16_t(base+2373),uint16_t(base+656),
        uint16_t(base+658),uint16_t(base+2374),uint16_t(base+659),uint16_t(base+661),uint16_t(base+2375),uint16_t(base+662),uint16_t(base+664),uint16_t(base+2376),uint16_t(base+665),uint16_t(base+667),uint16_t(base+2377),uint16_t(base+668),
        uint16_t(base+670),uint16_t(base+2378),uint16_t(base+671),uint16_t(base+673),uint16_t(base+2379),uint16_t(base+674),uint16_t(base+676),uint16_t(base+2380),uint16_t(base+677),uint16_t(base+679),uint16_t(base+2381),uint16_t(base+680),
        uint16_t(base+682),uint16_t(base+2382),uint16_t(base+683),uint16_t(base+685),uint16_t(base+2383),uint16_t(base+686),uint16_t(base+688),uint16_t(base+2384),uint16_t(base+689),uint16_t(base+691),uint16_t(base+2385),uint16_t(base+692),
        uint16_t(base+694),uint16_t(base+2386),uint16_t(base+695),uint16_t(base+697),uint16_t(base+2387),uint16_t(base+698),uint16_t(base+700),uint16_t(base+2388),uint16_t(base+701),uint16_t(base+703),uint16_t(base+2389),uint16_t(base+704),
        uint16_t(base+706),uint16_t(base+2390),uint16_t(base+707),uint16_t(base+709),uint16_t(base+2391),uint16_t(base+710),uint16_t(base+712),uint16_t(base+2392),uint16_t(base+713),uint16_t(base+715),uint16_t(base+2393),uint16_t(base+716),
        uint16_t(base+718),uint16_t(base+2394),uint16_t(base+719),uint16_t(base+721),uint16_t(base+2395),uint16_t(base+722),uint16_t(base+724),uint16_t(base+2396),uint16_t(base+725),uint16_t(base+727),uint16_t(base+2397),uint16_t(base+728),
        uint16_t(base+730),uint16_t(base+2398),uint16_t(base+731),uint16_t(base+733),uint16_t(base+2399),uint16_t(base+734),uint16_t(base+736),uint16_t(base+2400),uint16_t(base+737),uint16_t(base+739),uint16_t(base+2401),uint16_t(base+740),
        uint16_t(base+742),uint16_t(base+2402),uint16_t(base+743),uint16_t(base+745),uint16_t(base+2403),uint16_t(base+746),uint16_t(base+748),uint16_t(base+2404),uint16_t(base+749),uint16_t(base+751),uint16_t(base+2405),uint16_t(base+752),
        uint16_t(base+754),uint16_t(base+2406),uint16_t(base+755),uint16_t(base+757),uint16_t(base+2407),uint16_t(base+758),uint16_t(base+760),uint16_t(base+2408),uint16_t(base+761),uint16_t(base+763),uint16_t(base+2409),uint16_t(base+764),
        uint16_t(base+766),uint16_t(base+2410),uint16_t(base+767),uint16_t(base+769),uint16_t(base+2411),uint16_t(base+770),uint16_t(base+772),uint16_t(base+2412),uint16_t(base+773),uint16_t(base+775),uint16_t(base+2413),uint16_t(base+776),
        uint16_t(base+778),uint16_t(base+2414),uint16_t(base+779),uint16_t(base+781),uint16_t(base+2415),uint16_t(base+782),uint16_t(base+784),uint16_t(base+2416),uint16_t(base+785),uint16_t(base+787),uint16_t(base+2417),uint16_t(base+788),
        uint16_t(base+790),uint16_t(base+2418),uint16_t(base+791),uint16_t(base+793),uint16_t(base+2419),uint16_t(base+794),uint16_t(base+796),uint16_t(base+2420),uint16_t(base+797),uint16_t(base+799),uint16_t(base+2421),uint16_t(base+800),
        uint16_t(base+802),uint16_t(base+2422),uint16_t(base+803),uint16_t(base+805),uint16_t(base+2423),uint16_t(base+806),uint16_t(base+808),uint16_t(base+2424),uint16_t(base+809),uint16_t(base+811),uint16_t(base+2425),uint16_t(base+812),
        uint16_t(base+814),uint16_t(base+2426),uint16_t(base+815),uint16_t(base+817),uint16_t(base+2427),uint16_t(base+818),uint16_t(base+820),uint16_t(base+2428),uint16_t(base+821),uint16_t(base+823),uint16_t(base+2429),uint16_t(base+824),
        uint16_t(base+826),uint16_t(base+2430),uint16_t(base+827),uint16_t(base+829),uint16_t(base+2431),uint16_t(base+830),uint16_t(base+832),uint16_t(base+2432),uint16_t(base+833),uint16_t(base+835),uint16_t(base+2433),uint16_t(base+836),
        uint16_t(base+838),uint16_t(base+2434),uint16_t(base+839),uint16_t(base+841),uint16_t(base+2435),uint16_t(base+842),uint16_t(base+844),uint16_t(base+2436),uint16_t(base+845),uint16_t(base+847),uint16_t(base+2437),uint16_t(base+848),
        uint16_t(base+862),uint16_t(base+2438),uint16_t(base+863),uint16_t(base+865),uint16_t(base+2439),uint16_t(base+866),uint16_t(base+868),uint16_t(base+2440),uint16_t(base+869),uint16_t(base+871),uint16_t(base+2441),uint16_t(base+872),
        uint16_t(base+874),uint16_t(base+2442),uint16_t(base+875),uint16_t(base+877),uint16_t(base+2443),uint16_t(base+878),uint16_t(base+880),uint16_t(base+2444),uint16_t(base+881),uint16_t(base+883),uint16_t(base+2445),uint16_t(base+884),
        uint16_t(base+886),uint16_t(base+2446),uint16_t(base+887),uint16_t(base+889),uint16_t(base+2447),uint16_t(base+890),uint16_t(base+892),uint16_t(base+2448),uint16_t(base+893),uint16_t(base+895),uint16_t(base+2449),uint16_t(base+896),
        uint16_t(base+898),uint16_t(base+2450),uint16_t(base+899),uint16_t(base+901),uint16_t(base+2451),uint16_t(base+902),uint16_t(base+904),uint16_t(base+2452),uint16_t(base+905),uint16_t(base+907),uint16_t(base+2453),uint16_t(base+908),
        uint16_t(base+910),uint16_t(base+2454),uint16_t(base+911),uint16_t(base+913),uint16_t(base+2455),uint16_t(base+914),uint16_t(base+916),uint16_t(base+2456),uint16_t(base+917),uint16_t(base+919),uint16_t(base+2457),uint16_t(base+920),
        uint16_t(base+922),uint16_t(base+2458),uint16_t(base+923),uint16_t(base+925),uint16_t(base+2459),uint16_t(base+926),uint16_t(base+928),uint16_t(base+2460),uint16_t(base+929),uint16_t(base+931),uint16_t(base+2461),uint16_t(base+932),
        uint16_t(base+934),uint16_t(base+2462),uint16_t(base+935),uint16_t(base+937),uint16_t(base+2463),uint16_t(base+938),uint16_t(base+940),uint16_t(base+2464),uint16_t(base+941),uint16_t(base+943),uint16_t(base+2465),uint16_t(base+944),
        uint16_t(base+946),uint16_t(base+2466),uint16_t(base+947),uint16_t(base+949),uint16_t(base+2467),uint16_t(base+950),uint16_t(base+952),uint16_t(base+2468),uint16_t(base+953),uint16_t(base+955),uint16_t(base+2469),uint16_t(base+956),
        uint16_t(base+958),uint16_t(base+2470),uint16_t(base+959),uint16_t(base+961),uint16_t(base+2471),uint16_t(base+962),uint16_t(base+964),uint16_t(base+2472),uint16_t(base+965),uint16_t(base+967),uint16_t(base+2473),uint16_t(base+968),
        uint16_t(base+970),uint16_t(base+2474),uint16_t(base+971),uint16_t(base+973),uint16_t(base+2475),uint16_t(base+974),uint16_t(base+976),uint16_t(base+2476),uint16_t(base+977),uint16_t(base+979),uint16_t(base+2477),uint16_t(base+980),
        uint16_t(base+982),uint16_t(base+2478),uint16_t(base+983),uint16_t(base+985),uint16_t(base+2479),uint16_t(base+986),uint16_t(base+988),uint16_t(base+2480),uint16_t(base+989),uint16_t(base+991),uint16_t(base+2481),uint16_t(base+992),
        uint16_t(base+994),uint16_t(base+2482),uint16_t(base+995),uint16_t(base+997),uint16_t(base+2483),uint16_t(base+998),uint16_t(base+1000),uint16_t(base+2484),uint16_t(base+1001),uint16_t(base+1003),uint16_t(base+2485),uint16_t(base+1004),
        uint16_t(base+1006),uint16_t(base+2486),uint16_t(base+1007),uint16_t(base+1009),uint16_t(base+2487),uint16_t(base+1010),uint16_t(base+1012),uint16_t(base+2488),uint16_t(base+1013),uint16_t(base+1015),uint16_t(base+2489),uint16_t(base+1016),
        uint16_t(base+1018),uint16_t(base+2490),uint16_t(base+1019),uint16_t(base+1021),uint16_t(base+2491),uint16_t(base+1022),uint16_t(base+1024),uint16_t(base+2492),uint16_t(base+1025),uint16_t(base+1027),uint16_t(base+2493),uint16_t(base+1028),
        uint16_t(base+1030),uint16_t(base+2494),uint16_t(base+1031),uint16_t(base+1033),uint16_t(base+2495),uint16_t(base+1034),uint16_t(base+1036),uint16_t(base+2496),uint16_t(base+1037),uint16_t(base+1039),uint16_t(base+2497),uint16_t(base+1040),
        uint16_t(base+1042),uint16_t(base+2498),uint16_t(base+1043),uint16_t(base+1045),uint16_t(base+2499),uint16_t(base+1046),uint16_t(base+1048),uint16_t(base+2500),uint16_t(base+1049),uint16_t(base+1051),uint16_t(base+2501),uint16_t(base+1052),
        uint16_t(base+1054),uint16_t(base+2502),uint16_t(base+1055),uint16_t(base+1057),uint16_t(base+2503),uint16_t(base+1058),uint16_t(base+1060),uint16_t(base+2504),uint16_t(base+1061),uint16_t(base+1063),uint16_t(base+2505),uint16_t(base+1064),
        uint16_t(base+1066),uint16_t(base+2506),uint16_t(base+1067),uint16_t(base+1069),uint16_t(base+2507),uint16_t(base+1070),uint16_t(base+1072),uint16_t(base+2508),uint16_t(base+1073),uint16_t(base+1075),uint16_t(base+2509),uint16_t(base+1076),
        uint16_t(base+1078),uint16_t(base+2510),uint16_t(base+1079),uint16_t(base+1081),uint16_t(base+2511),uint16_t(base+1082),uint16_t(base+1084),uint16_t(base+2512),uint16_t(base+1085),uint16_t(base+1087),uint16_t(base+2513),uint16_t(base+1088),
        uint16_t(base+1090),uint16_t(base+2514),uint16_t(base+1091),uint16_t(base+1093),uint16_t(base+2515),uint16_t(base+1094),uint16_t(base+1096),uint16_t(base+2516),uint16_t(base+1097),uint16_t(base+1099),uint16_t(base+2517),uint16_t(base+1100),
        uint16_t(base+1102),uint16_t(base+2518),uint16_t(base+1103),uint16_t(base+1105),uint16_t(base+2519),uint16_t(base+1106),uint16_t(base+1108),uint16_t(base+2520),uint16_t(base+1109),uint16_t(base+1111),uint16_t(base+2521),uint16_t(base+1112),
        uint16_t(base+1114),uint16_t(base+2522),uint16_t(base+1115),uint16_t(base+1117),uint16_t(base+2523),uint16_t(base+1118),uint16_t(base+1120),uint16_t(base+2524),uint16_t(base+1121),uint16_t(base+1123),uint16_t(base+2525),uint16_t(base+1124),
        uint16_t(base+1138),uint16_t(base+2526),uint16_t(base+1139),uint16_t(base+1141),uint16_t(base+2527),uint16_t(base+1142),uint16_t(base+1144),uint16_t(base+2528),uint16_t(base+1145),uint16_t(base+1147),uint16_t(base+2529),uint16_t(base+1148),
        uint16_t(base+1150),uint16_t(base+2530),uint16_t(base+1151),uint16_t(base+1153),uint16_t(base+2531),uint16_t(base+1154),uint16_t(base+1156),uint16_t(base+2532),uint16_t(base+1157),uint16_t(base+1159),uint16_t(base+2533),uint16_t(base+1160),
        uint16_t(base+1162),uint16_t(base+2534),uint16_t(base+1163),uint16_t(base+1165),uint16_t(base+2535),uint16_t(base+1166),uint16_t(base+1168),uint16_t(base+2536),uint16_t(base+1169),uint16_t(base+1171),uint16_t(base+2537),uint16_t(base+1172),
        uint16_t(base+1174),uint16_t(base+2538),uint16_t(base+1175),uint16_t(base+1177),uint16_t(base+2539),uint16_t(base+1178),uint16_t(base+1180),uint16_t(base+2540),uint16_t(base+1181),uint16_t(base+1183),uint16_t(base+2541),uint16_t(base+1184),
        uint16_t(base+1186),uint16_t(base+2542),uint16_t(base+1187),uint16_t(base+1189),uint16_t(base+2543),uint16_t(base+1190),uint16_t(base+1192),uint16_t(base+2544),uint16_t(base+1193),uint16_t(base+1195),uint16_t(base+2545),uint16_t(base+1196),
        uint16_t(base+1198),uint16_t(base+2546),uint16_t(base+1199),uint16_t(base+1201),uint16_t(base+2547),uint16_t(base+1202),uint16_t(base+1204),uint16_t(base+2548),uint16_t(base+1205),uint16_t(base+1207),uint16_t(base+2549),uint16_t(base+1208),
        uint16_t(base+1210),uint16_t(base+2550),uint16_t(base+1211),uint16_t(base+1213),uint16_t(base+2551),uint16_t(base+1214),uint16_t(base+1216),uint16_t(base+2552),uint16_t(base+1217),uint16_t(base+1219),uint16_t(base+2553),uint16_t(base+1220),
        uint16_t(base+1222),uint16_t(base+2554),uint16_t(base+1223),uint16_t(base+1225),uint16_t(base+2555),uint16_t(base+1226),uint16_t(base+1228),uint16_t(base+2556),uint16_t(base+1229),uint16_t(base+1231),uint16_t(base+2557),uint16_t(base+1232),
        uint16_t(base+1234),uint16_t(base+2558),uint16_t(base+1235),uint16_t(base+1237),uint16_t(base+2559),uint16_t(base+1238),uint16_t(base+1240),uint16_t(base+2560),uint16_t(base+1241),uint16_t(base+1243),uint16_t(base+2561),uint16_t(base+1244),
        uint16_t(base+1246),uint16_t(base+2562),uint16_t(base+1247),uint16_t(base+1249),uint16_t(base+2563),uint16_t(base+1250),uint16_t(base+1252),uint16_t(base+2564),uint16_t(base+1253),uint16_t(base+1255),uint16_t(base+2565),uint16_t(base+1256),
        uint16_t(base+2566),uint16_t(base+2567),uint16_t(base+2568),uint16_t(base+1261),uint16_t(base+2569),uint16_t(base+1262),uint16_t(base+2570),uint16_t(base+2571),uint16_t(base+2572),uint16_t(base+1267),uint16_t(base+2573),uint16_t(base+1268),
        uint16_t(base+1270),uint16_t(base+2574),uint16_t(base+1271),uint16_t(base+2575),uint16_t(base+2576),uint16_t(base+2577),uint16_t(base+1276),uint16_t(base+2578),uint16_t(base+1277),uint16_t(base+2579),uint16_t(base+2580),uint16_t(base+2581),
        uint16_t(base+1282),uint16_t(base+2582),uint16_t(base+1283),uint16_t(base+1285),uint16_t(base+2583),uint16_t(base+1286),uint16_t(base+1288),uint16_t(base+2584),uint16_t(base+1289),uint16_t(base+1291),uint16_t(base+2585),uint16_t(base+1292),
        uint16_t(base+1294),uint16_t(base+2586),uint16_t(base+1295),uint16_t(base+1297),uint16_t(base+2587),uint16_t(base+1298),uint16_t(base+1300),uint16_t(base+2588),uint16_t(base+1301),uint16_t(base+1303),uint16_t(base+2589),uint16_t(base+1304),
        uint16_t(base+1306),uint16_t(base+2590),uint16_t(base+1307),uint16_t(base+1309),uint16_t(base+2591),uint16_t(base+1310),uint16_t(base+1312),uint16_t(base+2592),uint16_t(base+1313),uint16_t(base+1315),uint16_t(base+2593),uint16_t(base+1316),
        uint16_t(base+1318),uint16_t(base+2594),uint16_t(base+1319),uint16_t(base+1321),uint16_t(base+2595),uint16_t(base+1322),uint16_t(base+1324),uint16_t(base+2596),uint16_t(base+1325),uint16_t(base+1327),uint16_t(base+2597),uint16_t(base+1328),
        uint16_t(base+1330),uint16_t(base+2598),uint16_t(base+1331),uint16_t(base+1333),uint16_t(base+2599),uint16_t(base+1334),uint16_t(base+1336),uint16_t(base+2600),uint16_t(base+1337),uint16_t(base+1339),uint16_t(base+2601),uint16_t(base+1340),
        uint16_t(base+1342),uint16_t(base+2602),uint16_t(base+1343),uint16_t(base+1345),uint16_t(base+2603),uint16_t(base+1346),uint16_t(base+1348),uint16_t(base+2604),uint16_t(base+1349),uint16_t(base+1351),uint16_t(base+2605),uint16_t(base+1352),
        uint16_t(base+1354),uint16_t(base+2606),uint16_t(base+1355),uint16_t(base+1357),uint16_t(base+2607),uint16_t(base+1358),uint16_t(base+1360),uint16_t(base+2608),uint16_t(base+1361),uint16_t(base+1363),uint16_t(base+2609),uint16_t(base+1364),
        uint16_t(base+1366),uint16_t(base+2610),uint16_t(base+1367),uint16_t(base+1369),uint16_t(base+2611),uint16_t(base+1370),uint16_t(base+1372),uint16_t(base+2612),uint16_t(base+1373),uint16_t(base+1375),uint16_t(base+2613),uint16_t(base+1376),
        uint16_t(base+1378),uint16_t(base+2614),uint16_t(base+1379),uint16_t(base+1381),uint16_t(base+1390),uint16_t(base+1382),uint16_t(base+1384),uint16_t(base+2615),uint16_t(base+1385),uint16_t(base+1387),uint16_t(base+2616),uint16_t(base+1388),
        uint16_t(base+1390),uint16_t(base+1408),uint16_t(base+1391),uint16_t(base+1392),uint16_t(base+2617),uint16_t(base+1393),uint16_t(base+1395),uint16_t(base+2618),uint16_t(base+1396),uint16_t(base+1380),uint16_t(base+1379),uint16_t(base+1398),
        uint16_t(base+1400),uint16_t(base+2619),uint16_t(base+1401),uint16_t(base+1403),uint16_t(base+2620),uint16_t(base+1404),uint16_t(base+1399),uint16_t(base+1398),uint16_t(base+1406),uint16_t(base+1408),uint16_t(base+1412),uint16_t(base+1409),
        uint16_t(base+1407),uint16_t(base+1406),uint16_t(base+1410),uint16_t(base+1412),uint16_t(base+1428),uint16_t(base+1413),uint16_t(base+1414),uint16_t(base+2621),uint16_t(base+1415),uint16_t(base+1417),uint16_t(base+2622),uint16_t(base+1418),
        uint16_t(base+1420),uint16_t(base+2623),uint16_t(base+1421),uint16_t(base+1423),uint16_t(base+2624),uint16_t(base+1424),uint16_t(base+1411),uint16_t(base+1410),uint16_t(base+1426),uint16_t(base+1428),uint16_t(base+1438),uint16_t(base+1429),
        uint16_t(base+1430),uint16_t(base+2625),uint16_t(base+1431),uint16_t(base+1433),uint16_t(base+2626),uint16_t(base+1434),uint16_t(base+1427),uint16_t(base+1426),uint16_t(base+1436),uint16_t(base+1438),uint16_t(base+1442),uint16_t(base+1439),
        uint16_t(base+1437),uint16_t(base+1436),uint16_t(base+1440),uint16_t(base+1442),uint16_t(base+2627),uint16_t(base+1443),uint16_t(base+1444),uint16_t(base+2628),uint16_t(base+1445),uint16_t(base+1447),uint16_t(base+2629),uint16_t(base+1448),
        uint16_t(base+1450),uint16_t(base+2630),uint16_t(base+1451),uint16_t(base+1453),uint16_t(base+2631),uint16_t(base+1454),uint16_t(base+1456),uint16_t(base+2632),uint16_t(base+1457),uint16_t(base+1459),uint16_t(base+2633),uint16_t(base+1460),
        uint16_t(base+1474),uint16_t(base+2634),uint16_t(base+1475),uint16_t(base+1477),uint16_t(base+2635),uint16_t(base+1478),uint16_t(base+1480),uint16_t(base+2636),uint16_t(base+1481),uint16_t(base+1483),uint16_t(base+2637),uint16_t(base+1484),
        uint16_t(base+1486),uint16_t(base+2638),uint16_t(base+1487),uint16_t(base+1489),uint16_t(base+2639),uint16_t(base+1490),uint16_t(base+1492),uint16_t(base+2640),uint16_t(base+1493),uint16_t(base+1495),uint16_t(base+2641),uint16_t(base+1496),
        uint16_t(base+1498),uint16_t(base+2642),uint16_t(base+1499),uint16_t(base+1501),uint16_t(base+2643),uint16_t(base+1502),uint16_t(base+1504),uint16_t(base+2644),uint16_t(base+1505),uint16_t(base+1507),uint16_t(base+2645),uint16_t(base+1508),
        uint16_t(base+1510),uint16_t(base+2646),uint16_t(base+1511),uint16_t(base+1513),uint16_t(base+2647),uint16_t(base+1514),uint16_t(base+1516),uint16_t(base+2648),uint16_t(base+1517),uint16_t(base+1519),uint16_t(base+2649),uint16_t(base+1520),
        uint16_t(base+1522),uint16_t(base+2650),uint16_t(base+1523),uint16_t(base+1525),uint16_t(base+2651),uint16_t(base+1526),uint16_t(base+1528),uint16_t(base+2652),uint16_t(base+1529),uint16_t(base+1531),uint16_t(base+2653),uint16_t(base+1532),
        uint16_t(base+1534),uint16_t(base+2654),uint16_t(base+1535),uint16_t(base+1537),uint16_t(base+2655),uint16_t(base+1538),uint16_t(base+1540),uint16_t(base+2656),uint16_t(base+1541),uint16_t(base+1543),uint16_t(base+2657),uint16_t(base+1544),
        uint16_t(base+1546),uint16_t(base+2658),uint16_t(base+1547),uint16_t(base+1549),uint16_t(base+2659),uint16_t(base+1550),uint16_t(base+1552),uint16_t(base+2660),uint16_t(base+1553),uint16_t(base+1555),uint16_t(base+2661),uint16_t(base+1556),
        uint16_t(base+1558),uint16_t(base+2662),uint16_t(base+1559),uint16_t(base+1561),uint16_t(base+2663),uint16_t(base+1562),uint16_t(base+1564),uint16_t(base+2664),uint16_t(base+1565),uint16_t(base+1567),uint16_t(base+2665),uint16_t(base+1568),
        uint16_t(base+1570),uint16_t(base+2666),uint16_t(base+1571),uint16_t(base+1573),uint16_t(base+2667),uint16_t(base+1574),uint16_t(base+1576),uint16_t(base+2668),uint16_t(base+1577),uint16_t(base+1579),uint16_t(base+2669),uint16_t(base+1580),
        uint16_t(base+1582),uint16_t(base+2670),uint16_t(base+1583),uint16_t(base+1585),uint16_t(base+2671),uint16_t(base+1586),uint16_t(base+1588),uint16_t(base+2672),uint16_t(base+1589),uint16_t(base+1591),uint16_t(base+2673),uint16_t(base+1592),
        uint16_t(base+1594),uint16_t(base+2674),uint16_t(base+1595),uint16_t(base+1597),uint16_t(base+2675),uint16_t(base+1598),uint16_t(base+1600),uint16_t(base+2676),uint16_t(base+1601),uint16_t(base+1603),uint16_t(base+2677),uint16_t(base+1604),
        uint16_t(base+1606),uint16_t(base+2678),uint16_t(base+1607),uint16_t(base+1609),uint16_t(base+2679),uint16_t(base+1610),uint16_t(base+1612),uint16_t(base+2680),uint16_t(base+1613),uint16_t(base+1615),uint16_t(base+2681),uint16_t(base+1616),
        uint16_t(base+1618),uint16_t(base+2682),uint16_t(base+1619),uint16_t(base+1621),uint16_t(base+2683),uint16_t(base+1622),uint16_t(base+1624),uint16_t(base+2684),uint16_t(base+1625),uint16_t(base+1627),uint16_t(base+2685),uint16_t(base+1628),
        uint16_t(base+1630),uint16_t(base+2686),uint16_t(base+1631),uint16_t(base+1633),uint16_t(base+2687),uint16_t(base+1634),uint16_t(base+1636),uint16_t(base+2688),uint16_t(base+1637),uint16_t(base+1639),uint16_t(base+2689),uint16_t(base+1640),
        uint16_t(base+1642),uint16_t(base+2690),uint16_t(base+1643),uint16_t(base+1645),uint16_t(base+2691),uint16_t(base+1646),uint16_t(base+1648),uint16_t(base+2692),uint16_t(base+1649),uint16_t(base+1651),uint16_t(base+2693),uint16_t(base+1652),
        uint16_t(base+1654),uint16_t(base+2694),uint16_t(base+1655),uint16_t(base+1657),uint16_t(base+2695),uint16_t(base+1658),uint16_t(base+1660),uint16_t(base+2696),uint16_t(base+1661),uint16_t(base+1663),uint16_t(base+2697),uint16_t(base+1664),
        uint16_t(base+1666),uint16_t(base+2698),uint16_t(base+1667),uint16_t(base+1669),uint16_t(base+2699),uint16_t(base+1670),uint16_t(base+1672),uint16_t(base+2700),uint16_t(base+1673),uint16_t(base+1675),uint16_t(base+2701),uint16_t(base+1676),
        uint16_t(base+1678),uint16_t(base+2702),uint16_t(base+1679),uint16_t(base+1681),uint16_t(base+2703),uint16_t(base+1682),uint16_t(base+1684),uint16_t(base+2704),uint16_t(base+1685),uint16_t(base+1687),uint16_t(base+2705),uint16_t(base+1688),
        uint16_t(base+1690),uint16_t(base+2706),uint16_t(base+1691),uint16_t(base+1693),uint16_t(base+2707),uint16_t(base+1694),uint16_t(base+1696),uint16_t(base+2708),uint16_t(base+1697),uint16_t(base+1699),uint16_t(base+2709),uint16_t(base+1700),
        uint16_t(base+1702),uint16_t(base+2710),uint16_t(base+1703),uint16_t(base+1705),uint16_t(base+2711),uint16_t(base+1706),uint16_t(base+1708),uint16_t(base+2712),uint16_t(base+1709),uint16_t(base+1711),uint16_t(base+2713),uint16_t(base+1712),
        uint16_t(base+2714),uint16_t(base+2714),uint16_t(base+2715),uint16_t(base+2716),uint16_t(base+2717),uint16_t(base+2717),uint16_t(base+1718),uint16_t(base+1718),uint16_t(base+1719),uint16_t(base+1720),uint16_t(base+1721),uint16_t(base+1721),
        uint16_t(base+1722),uint16_t(base+2718),uint16_t(base+1723),uint16_t(base+1725),uint16_t(base+2719),uint16_t(base+1726),uint16_t(base+1728),uint16_t(base+1740),uint16_t(base+1729),uint16_t(base+1731),uint16_t(base+2720),uint16_t(base+1732),
        uint16_t(base+1734),uint16_t(base+2721),uint16_t(base+1735),uint16_t(base+1737),uint16_t(base+2722),uint16_t(base+1738),uint16_t(base+1740),uint16_t(base+1744),uint16_t(base+1741),uint16_t(base+1733),uint16_t(base+1732),uint16_t(base+1742),
        uint16_t(base+1744),uint16_t(base+1760),uint16_t(base+1745),uint16_t(base+1743),uint16_t(base+1742),uint16_t(base+1746),uint16_t(base+1748),uint16_t(base+2723),uint16_t(base+1749),uint16_t(base+1751),uint16_t(base+2724),uint16_t(base+1752),
        uint16_t(base+1754),uint16_t(base+2725),uint16_t(base+1755),uint16_t(base+1757),uint16_t(base+2726),uint16_t(base+1758),uint16_t(base+1760),uint16_t(base+1767),uint16_t(base+1761),uint16_t(base+1747),uint16_t(base+1746),uint16_t(base+1762),
        uint16_t(base+1764),uint16_t(base+2727),uint16_t(base+1765),uint16_t(base+1767),uint16_t(base+2728),uint16_t(base+1768),uint16_t(base+1763),uint16_t(base+1762),uint16_t(base+1769),uint16_t(base+1771),uint16_t(base+2729),uint16_t(base+1772),
        uint16_t(base+1774),uint16_t(base+2730),uint16_t(base+1775),uint16_t(base+1786),uint16_t(base+2731),uint16_t(base+1787),uint16_t(base+2732),uint16_t(base+2733),uint16_t(base+2734),uint16_t(base+2735),uint16_t(base+2736),uint16_t(base+2737),
        uint16_t(base+1804),uint16_t(base+2738),uint16_t(base+1805),uint16_t(base+1807),uint16_t(base+2739),uint16_t(base+1808),uint16_t(base+1810),uint16_t(base+2740),uint16_t(base+1811),uint16_t(base+1813),uint16_t(base+2741),uint16_t(base+1814),
        uint16_t(base+1816),uint16_t(base+2742),uint16_t(base+1817),uint16_t(base+1819),uint16_t(base+2743),uint16_t(base+1820),uint16_t(base+1822),uint16_t(base+2744),uint16_t(base+1823),uint16_t(base+1825),uint16_t(base+2745),uint16_t(base+1826),
        uint16_t(base+1828),uint16_t(base+2746),uint16_t(base+1829),uint16_t(base+1831),uint16_t(base+2747),uint16_t(base+1832),uint16_t(base+1834),uint16_t(base+2748),uint16_t(base+1835),uint16_t(base+1837),uint16_t(base+2749),uint16_t(base+1838),
        uint16_t(base+1840),uint16_t(base+1841),uint16_t(base+1841),uint16_t(base+1842),uint16_t(base+2750),uint16_t(base+1843),uint16_t(base+1845),uint16_t(base+2751),uint16_t(base+1846),uint16_t(base+2752),uint16_t(base+1840),uint16_t(base+2753),
        uint16_t(base+1851),uint16_t(base+2754),uint16_t(base+1852),uint16_t(base+1841),uint16_t(base+1856),uint16_t(base+1854),uint16_t(base+2755),uint16_t(base+2756),uint16_t(base+2757),uint16_t(base+1857),uint16_t(base+2758),uint16_t(base+1858),
        uint16_t(base+1860),uint16_t(base+2759),uint16_t(base+1861),uint16_t(base+1863),uint16_t(base+2760),uint16_t(base+1864),uint16_t(base+1866),uint16_t(base+2761),uint16_t(base+1867),uint16_t(base+1869),uint16_t(base+2762),uint16_t(base+1870),
        uint16_t(base+1872),uint16_t(base+2763),uint16_t(base+1873),uint16_t(base+1875),uint16_t(base+2764),uint16_t(base+1876),uint16_t(base+1878),uint16_t(base+2765),uint16_t(base+1879),uint16_t(base+1881),uint16_t(base+2766),uint16_t(base+1882),
        uint16_t(base+1884),uint16_t(base+2767),uint16_t(base+1885),uint16_t(base+1887),uint16_t(base+2768),uint16_t(base+1888),uint16_t(base+1890),uint16_t(base+2769),uint16_t(base+1891),uint16_t(base+1893),uint16_t(base+2770),uint16_t(base+1894),
        uint16_t(base+1896),uint16_t(base+2771),uint16_t(base+1897),uint16_t(base+1899),uint16_t(base+2772),uint16_t(base+1900),uint16_t(base+1902),uint16_t(base+2773),uint16_t(base+1903),uint16_t(base+1905),uint16_t(base+2774),uint16_t(base+1906),
        uint16_t(base+1908),uint16_t(base+2775),uint16_t(base+1909),uint16_t(base+1911),uint16_t(base+2776),uint16_t(base+1912),uint16_t(base+1914),uint16_t(base+2777),uint16_t(base+1915),uint16_t(base+1917),uint16_t(base+2778),uint16_t(base+1918),
        uint16_t(base+1920),uint16_t(base+2779),uint16_t(base+1921),uint16_t(base+1923),uint16_t(base+2780),uint16_t(base+1924),uint16_t(base+1926),uint16_t(base+2781),uint16_t(base+1927),uint16_t(base+1929),uint16_t(base+2782),uint16_t(base+1930),
        uint16_t(base+1932),uint16_t(base+2783),uint16_t(base+1933),uint16_t(base+1935),uint16_t(base+2784),uint16_t(base+1936),uint16_t(base+1938),uint16_t(base+2785),uint16_t(base+1939),uint16_t(base+2786),uint16_t(base+2787),uint16_t(base+2788),
        uint16_t(base+2789),uint16_t(base+2790),uint16_t(base+2791),uint16_t(base+2792),uint16_t(base+2793),uint16_t(base+2794),
    });
}

}
