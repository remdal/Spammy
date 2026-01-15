//
//  RMDLGrid.cpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

//#include "RMDLGrid.hpp"
//
//RMDLGrid::RMDLGrid(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary)
//: m_indexCount(0), m_gridEdgeIndexCount(0), m_editMode(true)
//{
//    // Init
//    m_voxels.resize(GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z);
//    
//    // Setup uniforms
//    m_uniforms.dimensions = {GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z};
//    m_uniforms.totalVoxels = GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z;
//    m_uniforms.voxelSize = {VOXEL_SIZE, VOXEL_SIZE, VOXEL_SIZE};
//    m_uniforms.gridOrigin = {0.0f, 0.0f, 0.0f};
//    
//    // Create GPU buffers
//    m_voxelBuffer = device->newBuffer(m_voxels.size() * sizeof(Voxel), MTL::ResourceStorageModeShared);
//    m_uniformsBuffer = device->newBuffer(sizeof(GridUniforms), MTL::ResourceStorageModeShared);
//    
//    m_meshVertexBuffer = device->newBuffer(1024 * 1024 * sizeof(Vertex), MTL::ResourceStorageModeShared);
//    m_meshIndexBuffer = device->newBuffer(1024 * 1024 * sizeof(uint32_t), MTL::ResourceStorageModeShared);
//    
//    memcpy(m_uniformsBuffer->contents(), &m_uniforms, sizeof(GridUniforms));
//    
//    initializeChunks();
//    generateGridEdges();
//    createPipelines();
//}
//
//RMDLGrid::~RMDLGrid()
//{
//    m_voxelBuffer->release();
//    m_uniformsBuffer->release();
//    m_meshVertexBuffer->release();
//    m_meshIndexBuffer->release();
//    
//    if (m_gridEdgeVertexBuffer) m_gridEdgeVertexBuffer->release();
//    if (m_gridEdgeIndexBuffer) m_gridEdgeIndexBuffer->release();
//    if (m_gridEdgePipeline) m_gridEdgePipeline->release();
//    if (m_gridEdgeDepthState) m_gridEdgeDepthState->release();
//    if (m_voxelPipeline) m_voxelPipeline->release();
//    if (m_voxelDepthState) m_voxelDepthState->release();
//    if (m_meshGenPipeline) m_meshGenPipeline->release();
//    if (m_cullingPipeline) m_cullingPipeline->release();
//    
//    m_library->release();
//    m_device->release();
//}
//
//void RMDLGrid::createPipelines()
//{
//    MTL::RenderPipelineDescriptor* gridDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//    MTL::Function* gridVertFunc = m_library->newFunction(NS::String::string("voxelGridEdgeVertex", NS::UTF8StringEncoding));
//    MTL::Function* gridFragFunc = m_library->newFunction(NS::String::string("voxelGridEdgeFragment", NS::UTF8StringEncoding));
//    
//    gridDesc->setVertexFunction(gridVertFunc);
//    gridDesc->setFragmentFunction(gridFragFunc);
//    gridDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
//    gridDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
//    
//    NS::Error* error = nullptr;
//    m_gridEdgePipeline = m_device->newRenderPipelineState(gridDesc, &error);
//    
//    gridVertFunc->release();
//    gridFragFunc->release();
//    gridDesc->release();
//    
//    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
//    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
//    depthDesc->setDepthWriteEnabled(false);
//    m_gridEdgeDepthState = m_device->newDepthStencilState(depthDesc);
//    depthDesc->release();
//    
//    MTL::RenderPipelineDescriptor* voxelDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//    MTL::Function* voxelVertFunc = m_library->newFunction(NS::String::string("voxelMeshVertex", NS::UTF8StringEncoding));
//    MTL::Function* voxelFragFunc = m_library->newFunction(NS::String::string("voxelMeshFragment", NS::UTF8StringEncoding));
//    
//    voxelDesc->setVertexFunction(voxelVertFunc);
//    voxelDesc->setFragmentFunction(voxelFragFunc);
//    voxelDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
//    voxelDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
//    
//    m_voxelPipeline = m_device->newRenderPipelineState(voxelDesc, &error);
//    
//    voxelVertFunc->release();
//    voxelFragFunc->release();
//    voxelDesc->release();
//    
//    // Depth state for voxels
//    MTL::DepthStencilDescriptor* voxelDepthDesc = MTL::DepthStencilDescriptor::alloc()->init();
//    voxelDepthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
//    voxelDepthDesc->setDepthWriteEnabled(true);
//    m_voxelDepthState = m_device->newDepthStencilState(voxelDepthDesc);
//    voxelDepthDesc->release();
//    
//    // Compute pipelines
//    MTL::Function* meshGenFunc = m_library->newFunction(NS::String::string("greedyMeshGeneration", NS::UTF8StringEncoding));
//    if (meshGenFunc) {
//        m_meshGenPipeline = m_device->newComputePipelineState(meshGenFunc, &error);
//        meshGenFunc->release();
//    }
//    
//    MTL::Function* cullingFunc = m_library->newFunction(NS::String::string("voxelCulling", NS::UTF8StringEncoding));
//    if (cullingFunc) {
//        m_cullingPipeline = m_device->newComputePipelineState(cullingFunc, &error);
//        cullingFunc->release();
//    }
//}
//
//Voxel* RMDLGrid::getVoxel(int x, int y, int z)
//{
//    if (!isValidCoord(x, y, z))
//        return nullptr;
//    return &m_voxels[getIndex(x, y, z)];
//}
//
//const Voxel* RMDLGrid::getVoxel(int x, int y, int z) const {
//    if (!isValidCoord(x, y, z)) return nullptr;
//    return &m_voxels[getIndex(x, y, z)];
//}
//
//void RMDLGrid::setVoxel(int x, int y, int z, const Voxel& voxel) {
//    if (!isValidCoord(x, y, z)) return;
//    m_voxels[getIndex(x, y, z)] = voxel;
//    
//    // Mark corresponding chunk as dirty
//    int chunkX = x / VoxelChunk::CHUNK_SIZE;
//    int chunkY = y / VoxelChunk::CHUNK_SIZE;
//    int chunkZ = z / VoxelChunk::CHUNK_SIZE;
//    
//    for (auto& chunk : m_chunks) {
//        if (chunk.chunkCoord.x == chunkX &&
//            chunk.chunkCoord.y == chunkY &&
//            chunk.chunkCoord.z == chunkZ) {
//            chunk.dirty = true;
//            break;
//        }
//    }
//}
//
//simd::int3 RMDLGrid::worldToGrid(const simd::float3& worldPos) const {
//    simd::float3 relative = worldPos - m_uniforms.gridOrigin;
//    return {
//        (int)floorf(relative.x / VOXEL_SIZE),
//        (int)floorf(relative.y / VOXEL_SIZE),
//        (int)floorf(relative.z / VOXEL_SIZE)
//    };
//}
//
//simd::float3 RMDLGrid::gridToWorld(const simd::int3& gridPos) const {
//    return m_uniforms.gridOrigin + simd::float3{
//        gridPos.x * VOXEL_SIZE + VOXEL_SIZE * 0.5f,
//        gridPos.y * VOXEL_SIZE + VOXEL_SIZE * 0.5f,
//        gridPos.z * VOXEL_SIZE + VOXEL_SIZE * 0.5f
//    };
//}
//
//void RMDLGrid::initializeChunks() {
//    int chunksX = (GRID_SIZE_X + VoxelChunk::CHUNK_SIZE - 1) / VoxelChunk::CHUNK_SIZE;
//    int chunksY = (GRID_SIZE_Y + VoxelChunk::CHUNK_SIZE - 1) / VoxelChunk::CHUNK_SIZE;
//    int chunksZ = (GRID_SIZE_Z + VoxelChunk::CHUNK_SIZE - 1) / VoxelChunk::CHUNK_SIZE;
//    
//    for (int z = 0; z < chunksZ; z++) {
//        for (int y = 0; y < chunksY; y++) {
//            for (int x = 0; x < chunksX; x++) {
//                VoxelChunk chunk;
//                chunk.chunkCoord = {x, y, z};
//                chunk.dirty = true;
//                chunk.visible = true;
//                m_chunks.push_back(chunk);
//            }
//        }
//    }
//}
//
//void RMDLGrid::generateGridEdges() {
//    std::vector<simd::float3> vertices;
//    std::vector<uint32_t> indices;
//    
//    // Generate edges for the grid bounds
//    float sizeX = GRID_SIZE_X * VOXEL_SIZE;
//    float sizeY = GRID_SIZE_Y * VOXEL_SIZE;
//    float sizeZ = GRID_SIZE_Z * VOXEL_SIZE;
//    
//    // 8 corners
//    simd::float3 corners[8] = {
//        {0, 0, 0}, {sizeX, 0, 0}, {sizeX, 0, sizeZ}, {0, 0, sizeZ},
//        {0, sizeY, 0}, {sizeX, sizeY, 0}, {sizeX, sizeY, sizeZ}, {0, sizeY, sizeZ}
//    };
//    
//    for (int i = 0; i < 8; i++) {
//        vertices.push_back(corners[i]);
//    }
//    
//    // 12 edges
//    uint32_t edgeIndices[] = {
//        0,1, 1,2, 2,3, 3,0,  // Bottom
//        4,5, 5,6, 6,7, 7,4,  // Top
//        0,4, 1,5, 2,6, 3,7   // Vertical
//    };
//    
//    for (uint32_t idx : edgeIndices) {
//        indices.push_back(idx);
//    }
//    
//    m_gridEdgeIndexCount = indices.size();
//    
//    m_gridEdgeVertexBuffer = m_device->newBuffer(vertices.data(),
//                                                 vertices.size() * sizeof(simd::float3),
//                                                 MTL::ResourceStorageModeShared);
//    m_gridEdgeIndexBuffer = m_device->newBuffer(indices.data(),
//                                               indices.size() * sizeof(uint32_t),
//                                               MTL::ResourceStorageModeShared);
//}
//
//void RMDLGrid::renderGridEdges(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection) {
//    if (!m_editMode || !m_gridEdgePipeline) return;
//    
//    encoder->setRenderPipelineState(m_gridEdgePipeline);
//    encoder->setDepthStencilState(m_gridEdgeDepthState);
//    
//    encoder->setVertexBuffer(m_gridEdgeVertexBuffer, 0, 0);
//    encoder->setVertexBytes(&viewProjection, sizeof(simd::float4x4), 1);
//    
//    simd::float4 gridColor = {0.1f, 1.0f, 1.0f, 0.9f};  // Cyan semi-transparent
//    encoder->setFragmentBytes(&gridColor, sizeof(simd::float4), 0);
//    
//    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeLine,
//                                  m_gridEdgeIndexCount,
//                                  MTL::IndexTypeUInt32,
//                                  m_gridEdgeIndexBuffer,
//                                  0);
//}
//
//void RMDLGrid::uploadToGPU() {
//    memcpy(m_voxelBuffer->contents(), m_voxels.data(), m_voxels.size() * sizeof(Voxel));
//}
//
//void RMDLGrid::downloadFromGPU() {
//    memcpy(m_voxels.data(), m_voxelBuffer->contents(), m_voxels.size() * sizeof(Voxel));
//}
//
//void RMDLGrid::generateMesh() {
//    // Greedy meshing algorithm - CPU version for now
//    // TODO: Move to GPU compute shader for better performance
//    
//    std::vector<Vertex> vertices;
//    std::vector<uint32_t> indices;
//    
//    // ... Greedy meshing implementation
//    // For now, simple cube per voxel
//    
//    for (int z = 0; z < GRID_SIZE_Z; z++) {
//        for (int y = 0; y < GRID_SIZE_Y; y++) {
//            for (int x = 0; x < GRID_SIZE_X; x++) {
//                const Voxel* voxel = getVoxel(x, y, z);
//                if (!voxel || !voxel->isActive()) continue;
//                
//                simd::float3 pos = gridToWorld({x, y, z});
//                
//                // Check each face for occlusion
//                // Only generate faces that are visible
//                
//                // ... Generate cube faces
//            }
//        }
//    }
//    
//    m_indexCount = indices.size();
//    
//    if (!vertices.empty()) {
//        memcpy(m_meshVertexBuffer->contents(), vertices.data(),
//               vertices.size() * sizeof(Vertex));
//        memcpy(m_meshIndexBuffer->contents(), indices.data(),
//               indices.size() * sizeof(uint32_t));
//    }
//}
//
//RMDLGrid::RaycastHit RMDLGrid::raycast(const simd::float3& origin,
//                                         const simd::float3& direction,
//                                         float maxDistance) const {
//    RaycastHit result;
//    result.hit = false;
//    
//    // DDA raycast algorithm
//    simd::float3 rayPos = origin;
//    simd::float3 rayDir = simd::normalize(direction);
//    
//    simd::int3 mapPos = worldToGrid(rayPos);
//    simd::int3 step = {
//        rayDir.x > 0 ? 1 : -1,
//        rayDir.y > 0 ? 1 : -1,
//        rayDir.z > 0 ? 1 : -1
//    };
//    
//    simd::float3 deltaDist = {
//        fabsf(1.0f / rayDir.x),
//        fabsf(1.0f / rayDir.y),
//        fabsf(1.0f / rayDir.z)
//    };
//    
//    simd::float3 sideDist;
//    simd::float3 voxelBound = gridToWorld(mapPos);
//    
//    sideDist.x = (step.x > 0) ? (voxelBound.x + VOXEL_SIZE - rayPos.x) : (rayPos.x - voxelBound.x);
//    sideDist.y = (step.y > 0) ? (voxelBound.y + VOXEL_SIZE - rayPos.y) : (rayPos.y - voxelBound.y);
//    sideDist.z = (step.z > 0) ? (voxelBound.z + VOXEL_SIZE - rayPos.z) : (rayPos.z - voxelBound.z);
//    
//    sideDist.x *= deltaDist.x;
//    sideDist.y *= deltaDist.y;
//    sideDist.z *= deltaDist.z;
//    
//    int side = 0;
//    float distance = 0.0f;
//    
//    while (distance < maxDistance) {
//        const Voxel* voxel = getVoxel(mapPos.x, mapPos.y, mapPos.z);
//        
//        if (voxel && voxel->isActive()) {
//            result.hit = true;
//            result.voxelPos = mapPos;
//            result.hitPoint = rayPos + rayDir * distance;
//            result.faceIndex = side;
//            
//            // Calculate adjacent position for block placement
//            result.adjacentPos = mapPos;
//            if (side == 0) result.adjacentPos.x -= step.x;
//            else if (side == 1) result.adjacentPos.y -= step.y;
//            else if (side == 2) result.adjacentPos.z -= step.z;
//            
//            return result;
//        }
//        
//        // Step to next voxel
//        if (sideDist.x < sideDist.y) {
//            if (sideDist.x < sideDist.z) {
//                sideDist.x += deltaDist.x;
//                mapPos.x += step.x;
//                side = 0;
//                distance = sideDist.x;
//            } else {
//                sideDist.z += deltaDist.z;
//                mapPos.z += step.z;
//                side = 2;
//                distance = sideDist.z;
//            }
//        } else {
//            if (sideDist.y < sideDist.z) {
//                sideDist.y += deltaDist.y;
//                mapPos.y += step.y;
//                side = 1;
//                distance = sideDist.y;
//            } else {
//                sideDist.z += deltaDist.z;
//                mapPos.z += step.z;
//                side = 2;
//                distance = sideDist.z;
//            }
//        }
//    }
//    
//    return result;
//}
//
//bool RMDLGrid::checkCollision(const simd::float3& position, float radius) const {
//    simd::int3 gridPos = worldToGrid(position);
//    int radiusVoxels = (int)ceilf(radius / VOXEL_SIZE);
//    
//    for (int z = -radiusVoxels; z <= radiusVoxels; z++) {
//        for (int y = -radiusVoxels; y <= radiusVoxels; y++) {
//            for (int x = -radiusVoxels; x <= radiusVoxels; x++) {
//                const Voxel* voxel = getVoxel(gridPos.x + x, gridPos.y + y, gridPos.z + z);
//                if (voxel && voxel->isActive()) {
//                    simd::float3 voxelCenter = gridToWorld({gridPos.x + x, gridPos.y + y, gridPos.z + z});
//                    float dist = simd::length(position - voxelCenter);
//                    if (dist < radius + VOXEL_SIZE * 0.866f) {  // sqrt(3)/2 for cube diagonal
//                        return true;
//                    }
//                }
//            }
//        }
//    }
//    return false;
//}
//
//simd::float3 RMDLGrid::resolveCollision(const simd::float3& position,
//                                         const simd::float3& velocity,
//                                         float radius) const {
//    // Simple collision response - push out of solid voxels
//    if (!checkCollision(position, radius)) {
//        return position;
//    }
//    
//    simd::float3 resolved = position;
//    simd::int3 gridPos = worldToGrid(position);
//    int radiusVoxels = (int)ceilf(radius / VOXEL_SIZE);
//    
//    simd::float3 pushDir = {0, 0, 0};
//    int pushCount = 0;
//    
//    for (int z = -radiusVoxels; z <= radiusVoxels; z++) {
//        for (int y = -radiusVoxels; y <= radiusVoxels; y++) {
//            for (int x = -radiusVoxels; x <= radiusVoxels; x++) {
//                const Voxel* voxel = getVoxel(gridPos.x + x, gridPos.y + y, gridPos.z + z);
//                if (voxel && voxel->isActive()) {
//                    simd::float3 voxelCenter = gridToWorld({gridPos.x + x, gridPos.y + y, gridPos.z + z});
//                    simd::float3 diff = position - voxelCenter;
//                    float dist = simd::length(diff);
//                    float minDist = radius + VOXEL_SIZE * 0.866f;
//                    
//                    if (dist < minDist && dist > 0.001f) {
//                        pushDir += simd::normalize(diff) * (minDist - dist);
//                        pushCount++;
//                    }
//                }
//            }
//        }
//    }
//    
//    if (pushCount > 0) {
//        resolved += pushDir / (float)pushCount;
//    }
//    
//    return resolved;
//}
//
//bool RMDLGrid::saveToFile(const std::string& filepath) {
//    std::ofstream file(filepath, std::ios::binary);
//    if (!file.is_open()) return false;
//    
//    // Header
//    uint32_t version = 1;
//    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
//    file.write(reinterpret_cast<const char*>(&m_uniforms), sizeof(GridUniforms));
//    
//    // Compressed voxel data - only save active voxels
//    uint32_t activeCount = 0;
//    for (const auto& voxel : m_voxels) {
//        if (voxel.isActive()) activeCount++;
//    }
//    
//    file.write(reinterpret_cast<const char*>(&activeCount), sizeof(activeCount));
//    
//    for (int i = 0; i < m_voxels.size(); i++) {
//        if (m_voxels[i].isActive()) {
//            int x = i % GRID_SIZE_X;
//            int y = (i / GRID_SIZE_X) % GRID_SIZE_Y;
//            int z = i / (GRID_SIZE_X * GRID_SIZE_Y);
//            
//            file.write(reinterpret_cast<const char*>(&x), sizeof(int));
//            file.write(reinterpret_cast<const char*>(&y), sizeof(int));
//            file.write(reinterpret_cast<const char*>(&z), sizeof(int));
//            file.write(reinterpret_cast<const char*>(&m_voxels[i]), sizeof(Voxel));
//        }
//    }
//    
//    file.close();
//    return true;
//}
//
//bool RMDLGrid::loadFromFile(const std::string& filepath) {
//    std::ifstream file(filepath, std::ios::binary);
//    if (!file.is_open()) return false;
//    
//    // Clear current data
//    clear();
//    
//    // Read header
//    uint32_t version;
//    file.read(reinterpret_cast<char*>(&version), sizeof(version));
//    if (version != 1) return false;
//    
//    file.read(reinterpret_cast<char*>(&m_uniforms), sizeof(GridUniforms));
//    
//    // Read voxel data
//    uint32_t activeCount;
//    file.read(reinterpret_cast<char*>(&activeCount), sizeof(activeCount));
//    
//    for (uint32_t i = 0; i < activeCount; i++) {
//        int x, y, z;
//        Voxel voxel;
//        
//        file.read(reinterpret_cast<char*>(&x), sizeof(int));
//        file.read(reinterpret_cast<char*>(&y), sizeof(int));
//        file.read(reinterpret_cast<char*>(&z), sizeof(int));
//        file.read(reinterpret_cast<char*>(&voxel), sizeof(Voxel));
//        
//        setVoxel(x, y, z, voxel);
//    }
//    
//    file.close();
//    uploadToGPU();
//    generateMesh();
//    
//    return true;
//}
//
//void RMDLGrid::clear() {
//    for (auto& voxel : m_voxels) {
//        voxel = Voxel();
//    }
//    uploadToGPU();
//}
//
//size_t RMDLGrid::getActiveVoxelCount() const {
//    size_t count = 0;
//    for (const auto& voxel : m_voxels) {
//        if (voxel.isActive()) count++;
//    }
//    return count;
//}
//
//void RMDLGrid::updateChunks(const simd::float3& cameraPos) {
//    // Frustum culling and LOD based on camera distance
//    for (auto& chunk : m_chunks) {
//        simd::float3 chunkCenter = gridToWorld({
//            chunk.chunkCoord.x * VoxelChunk::CHUNK_SIZE + VoxelChunk::CHUNK_SIZE / 2,
//            chunk.chunkCoord.y * VoxelChunk::CHUNK_SIZE + VoxelChunk::CHUNK_SIZE / 2,
//            chunk.chunkCoord.z * VoxelChunk::CHUNK_SIZE + VoxelChunk::CHUNK_SIZE / 2
//        });
//        
//        float distance = simd::length(cameraPos - chunkCenter);
//        chunk.visible = distance < 200.0f;  // Render distance
//    }
//}
//
//void RMDLGrid::renderChunks(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection)
//{
//    if (!m_voxelPipeline || m_indexCount == 0)
//        return;
//    
//    encoder->setRenderPipelineState(m_voxelPipeline);
//    encoder->setDepthStencilState(m_voxelDepthState);
//    
//    encoder->setVertexBuffer(m_meshVertexBuffer, 0, 0);
//    encoder->setVertexBytes(&viewProjection, sizeof(simd::float4x4), 1);
//    encoder->setVertexBuffer(m_uniformsBuffer, 0, 2);
//    
//    encoder->setFragmentBuffer(m_uniformsBuffer, 0, 0);
//    
//    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, m_indexCount, MTL::IndexTypeUInt32, m_meshIndexBuffer, 0);
//}
