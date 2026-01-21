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


void BlockRegistry::registerDefaults() {
    // === STRUCTURE BLOCKS ===
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
                    .category = BlockCategory::Cosmetic,
                    .name = "Icosphere",
                    .description = "Blender import",
                    .mass = 1.0f, .health = 100.0f, .cost = 50.0f,
                    .size = {1, 1, 1},
                    .baseColor = {0.6f, 0.62f, 0.65f, 1.0f}, });
}

BlockRenderer::BlockRenderer(MTL::Device* device, MTL::PixelFormat colorFormat,
                             MTL::PixelFormat depthFormat, MTL::Library* library)
    : m_device(device), m_library(library) {
    createPipeline(colorFormat, depthFormat);
    buildMeshes();
}

BlockRenderer::~BlockRenderer() {
    if (m_pipeline) m_pipeline->release();
    if (m_depthState) m_depthState->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    if (m_instanceBuffer) m_instanceBuffer->release();
}

void BlockRenderer::createPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat) {
    NS::Error* err = nullptr;
    
    auto vertFn = m_library->newFunction(MTLSTR("blockVertex"));
    auto fragFn = m_library->newFunction(MTLSTR("blockFragment"));
    
    auto desc = MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vertFn);
    desc->setFragmentFunction(fragFn);
    desc->colorAttachments()->object(0)->setPixelFormat(colorFormat);
    desc->setDepthAttachmentPixelFormat(depthFormat);
    
    // Vertex descriptor
    auto vertDesc = MTL::VertexDescriptor::alloc()->init();
    // Position
    vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertDesc->attributes()->object(0)->setOffset(0);
    vertDesc->attributes()->object(0)->setBufferIndex(0);
    // Normal
    vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertDesc->attributes()->object(1)->setOffset(sizeof(simd::float3));
    vertDesc->attributes()->object(1)->setBufferIndex(0);
    // UV
    vertDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertDesc->attributes()->object(2)->setOffset(sizeof(simd::float3) * 2);
    vertDesc->attributes()->object(2)->setBufferIndex(0);
    // Color
    vertDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat4);
    vertDesc->attributes()->object(3)->setOffset(sizeof(simd::float3) * 2 + sizeof(simd::float2));
    vertDesc->attributes()->object(3)->setBufferIndex(0);
    
    vertDesc->layouts()->object(0)->setStride(sizeof(BlockVertex));
    vertDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    desc->setVertexDescriptor(vertDesc);
    
    m_pipeline = m_device->newRenderPipelineState(desc, &err);
    
    // Depth state
    auto depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    m_depthState = m_device->newDepthStencilState(depthDesc);
    
    vertFn->release();
    fragFn->release();
    desc->release();
    vertDesc->release();
    depthDesc->release();
}

void BlockRenderer::buildMeshes() {
    std::vector<BlockVertex> allVerts;
    std::vector<uint16_t> allIndices;
    
    auto& registry = BlockRegistry::instance();
    
    for (auto& [type, def] : registry.all()) {
        MeshRange range;
        range.vertexOffset = (uint32_t)allVerts.size();
        range.indexOffset = (uint32_t)allIndices.size();
        
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
    
    m_vertexBuffer = m_device->newBuffer(allVerts.data(), allVerts.size() * sizeof(BlockVertex),
                                         MTL::ResourceStorageModeShared);
    m_indexBuffer = m_device->newBuffer(allIndices.data(), allIndices.size() * sizeof(uint16_t),
                                        MTL::ResourceStorageModeShared);
    
    // Pre-allocate instance buffer (max 4096 blocks)
    m_instanceBuffer = m_device->newBuffer(4096 * sizeof(BlockGPUInstance), MTL::ResourceStorageModeShared);
}

void BlockRenderer::updateInstances(const std::vector<BlockInstance>& blocks, float time) {
    m_gpuInstances.clear();
    m_gpuInstances.reserve(blocks.size() + 1);
    
    for (const auto& block : blocks) {
        BlockGPUInstance gpu;
        gpu.modelMatrix = block.getModelMatrix();
        gpu.color = block.tintColor;
        gpu.params = {block.damage, block.powered ? 1.0f : 0.0f, time, (float)block.type};
        m_gpuInstances.push_back(gpu);
    }
    
    // Ghost block
    if (m_hasGhost) {
        BlockInstance ghost;
        ghost.gridPos = m_ghostPos;
        ghost.rotation = m_ghostRot;
        ghost.type = m_ghostType;
        ghost.tintColor = {1, 1, 1, 0.4f};  // Semi-transparent
        
        BlockGPUInstance gpu;
        gpu.modelMatrix = ghost.getModelMatrix();
        gpu.color = ghost.tintColor;
        gpu.params = {0, 0, time, (float)m_ghostType};
        m_gpuInstances.push_back(gpu);
    }
    
    m_instanceCount = (uint32_t)m_gpuInstances.size();
    
    if (m_instanceCount > 0) {
        memcpy(m_instanceBuffer->contents(), m_gpuInstances.data(),
               m_instanceCount * sizeof(BlockGPUInstance));
    }
}

void BlockRenderer::render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos) {
    if (m_instanceCount == 0) return;
    
    enc->setRenderPipelineState(m_pipeline);
    enc->setDepthStencilState(m_depthState);
    enc->setCullMode(MTL::CullModeBack);
    
    enc->setVertexBuffer(m_vertexBuffer, 0, 0);
    enc->setVertexBuffer(m_instanceBuffer, 0, 1);
    enc->setVertexBytes(&viewProj, sizeof(viewProj), 2);
    enc->setVertexBytes(&camPos, sizeof(camPos), 3);
    
    enc->setFragmentBytes(&camPos, sizeof(camPos), 0);
    
    // Group instances by block type for batched drawing
    std::unordered_map<BlockType, std::vector<uint32_t>> typeInstances;
    for (uint32_t i = 0; i < m_instanceCount; i++) {
        BlockType t = (BlockType)(int)m_gpuInstances[i].params.w;
        typeInstances[t].push_back(i);
    }
    
    for (auto& [type, instances] : typeInstances) {
        auto it = m_meshRanges.find(type);
        if (it == m_meshRanges.end()) continue;
        
        const auto& range = it->second;
        
        for (uint32_t instIdx : instances) {
            enc->setVertexBufferOffset(instIdx * sizeof(BlockGPUInstance), 1);
            enc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                       range.indexCount, MTL::IndexTypeUInt16,
                                       m_indexBuffer, range.indexOffset * sizeof(uint16_t),
                                       1);
        }
    }
}

void BlockRenderer::setGhostBlock(BlockType type, simd::int3 pos, uint8_t rot) {
    m_hasGhost = true;
    m_ghostType = type;
    m_ghostPos = pos;
    m_ghostRot = rot;
}

// ============================================================================
// BLOCK SYSTEM
// ============================================================================

BlockSystem::BlockSystem(MTL::Device* device, MTL::PixelFormat colorFormat,
                         MTL::PixelFormat depthFormat, MTL::Library* library) {
    m_renderer = std::make_unique<BlockRenderer>(device, colorFormat, depthFormat, library);
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
//    return m_gridMap.find(hashPos(pos)) == m_gridMap.end(); //;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Check if position is occupied
    if (m_gridMap.find(hashPos(pos)) != m_gridMap.end()) return false;
    
    // First block can be placed anywhere
    if (m_blocks.empty()) return true;
    
    // Must have at least one neighbor
    return hasNeighbor(pos);
}

bool BlockSystem::hasNeighbor(simd::int3 pos) const {
    static const simd::int3 offsets[6] = {
        {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
    };
    for (const auto& off : offsets) {
        simd::int3 neighbor = pos + off;
        if (m_gridMap.find(hashPos(neighbor)) != m_gridMap.end()) return true;
    }
    return false;
}

void BlockSystem::setBlockColor(uint32_t id, simd::float4 color) {
    if (auto* block = getBlock(id)) block->tintColor = color;
}

void BlockSystem::update(float dt) {
    m_time += dt;
    m_renderer->updateInstances(m_blocks, m_time);
}

void BlockSystem::render(MTL::RenderCommandEncoder* enc, simd::float4x4 viewProj, simd::float3 camPos) {
    m_renderer->render(enc, viewProj, camPos);
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

simd::float3 BlockSystem::centerOfMass() const {
    simd::float3 com = {0, 0, 0};
    float totalM = 0;
    for (const auto& b : m_blocks) {
        if (auto* def = BlockRegistry::instance().get(b.type)) {
            simd::float3 pos = {(float)b.gridPos.x, (float)b.gridPos.y, (float)b.gridPos.z};
            com += pos * def->mass;
            totalM += def->mass;
        }
    }
    return totalM > 0 ? com / totalM : simd::float3{0, 0, 0};
}

simd::float4x4 BlockInstance::getRotationMatrix() const {
    // 24 orientations: 6 faces × 4 rotations autour de la normale
    static const simd::float4x4 rotations[24] = {
        // Face +Y (up) - 4 rotations
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

// ============================================================================
// MESH GENERATION HELPERS
// ============================================================================

void BlockMeshGenerator::addQuad(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                 simd::float3 p0, simd::float3 p1, simd::float3 p2, simd::float3 p3,
                                 simd::float3 n, simd::float4 c) {
    uint16_t base = (uint16_t)v.size();
    v.push_back({p0, n, {0,0}, c});
    v.push_back({p1, n, {1,0}, c});
    v.push_back({p2, n, {1,1}, c});
    v.push_back({p3, n, {0,1}, c});
    idx.insert(idx.end(), {base, uint16_t(base+1), uint16_t(base+2),
                           base, uint16_t(base+2), uint16_t(base+3)});
}

void BlockMeshGenerator::addTriangle(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                     simd::float3 p0, simd::float3 p1, simd::float3 p2,
                                     simd::float3 n, simd::float4 c) {
    uint16_t base = (uint16_t)v.size();
    v.push_back({p0, n, {0,0}, c});
    v.push_back({p1, n, {1,0}, c});
    v.push_back({p2, n, {0.5f,1}, c});
    idx.insert(idx.end(), {base, uint16_t(base+1), uint16_t(base+2)});
}

// ============================================================================
// CUBE GENERATION
// ============================================================================

void BlockMeshGenerator::generateCube(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                      simd::float3 sz, simd::float4 c) {
    float x = sz.x * 0.5f, y = sz.y * 0.5f, z = sz.z * 0.5f;
    
    // +Y (top)
    addQuad(v, idx, {-x,y,-z}, {x,y,-z}, {x,y,z}, {-x,y,z}, {0,1,0}, c);
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

// ============================================================================
// WEDGE/SLOPE GENERATION
// ============================================================================

void BlockMeshGenerator::generateWedge(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                       simd::float3 sz, simd::float4 c) {
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

// ============================================================================
// WHEEL GENERATION - Cylindre avec jante détaillée
// ============================================================================

void BlockMeshGenerator::generateWheel(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                       float radius, float width, int seg, simd::float4 c) {
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

// ============================================================================
// COCKPIT/COMMAND SEAT - Forme aérodynamique vitrée
// ============================================================================

void BlockMeshGenerator::generateCockpit(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                         simd::float3 sz, simd::float4 c) {
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

// ============================================================================
// ROBOT HEAD - Tête style mecha/AAA
// ============================================================================

void BlockMeshGenerator::generateRobotHead(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                           simd::float4 c) {
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

// ============================================================================
// THRUSTER GENERATION
// ============================================================================

void BlockMeshGenerator::generateThruster(std::vector<BlockVertex>& v, std::vector<uint16_t>& idx,
                                          float radius, float length, simd::float4 c) {
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
    for (int i = 0; i < seg; i++) {
        float a0 = (float)i / seg * M_PI * 2.0f;
        float a1 = (float)(i+1) / seg * M_PI * 2.0f;
        addTriangle(v, idx, {0,0,0},
            {radius*0.8f*cosf(a1), radius*0.8f*sinf(a1), 0},
            {radius*0.8f*cosf(a0), radius*0.8f*sinf(a0), 0},
            {0,0,1}, c * 0.9f);
    }
}

// Icosphere: 234 verts, 78 tris
// Scale: fits 1x1x1, centered, Y-up

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

}
