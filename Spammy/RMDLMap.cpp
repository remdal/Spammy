//
//  RMDLMap.cpp
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#include "RMDLMap.hpp"

TerrainGeneratorLisse::TerrainGeneratorLisse(MTL::Device* device, const TerrainConfigLisse& config)
    : m_device(device)
    , m_config(config)
    , m_terrainPipeline(nullptr)
    , m_normalsPipeline(nullptr)
    , m_vertexBuffer(nullptr)
    , m_indexBuffer(nullptr)
    , m_indexCount(0)
{
    m_configBuffer = m_device->newBuffer(sizeof(TerrainConfigLisse), MTL::ResourceStorageModeShared);
    memcpy(m_configBuffer->contents(), &m_config, sizeof(TerrainConfigLisse));
    
    buildComputePipeline();
    buildNormalsPipeline();
}

TerrainGeneratorLisse::~TerrainGeneratorLisse() {
    if (m_terrainPipeline) m_terrainPipeline->release();
    if (m_normalsPipeline) m_normalsPipeline->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    if (m_configBuffer) m_configBuffer->release();
}

void TerrainGeneratorLisse::buildComputePipeline() {
    NS::Error* error = nullptr;
    auto library = m_device->newDefaultLibrary();
    
    auto terrainFunc = library->newFunction(NS::String::string("terrainGenerateKernel", NS::UTF8StringEncoding));
    m_terrainPipeline = m_device->newComputePipelineState(terrainFunc, &error);
    terrainFunc->release();
    library->release();
}

void TerrainGeneratorLisse::buildNormalsPipeline() {
    NS::Error* error = nullptr;
    auto library = m_device->newDefaultLibrary();
    
    auto normalsFunc = library->newFunction(NS::String::string("computeNormalsKernel", NS::UTF8StringEncoding));
    m_normalsPipeline = m_device->newComputePipelineState(normalsFunc, &error);
    normalsFunc->release();
    library->release();
}

void TerrainGeneratorLisse::generate(MTL::CommandBuffer* cmd, simd::float2 chunkOrigin, uint32_t chunkSize) {
    uint32_t vertexCount = (chunkSize + 1) * (chunkSize + 1);
    uint32_t quadCount = chunkSize * chunkSize;
    m_indexCount = quadCount * 6;
    
    // Allocate buffers
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    
    m_vertexBuffer = m_device->newBuffer(vertexCount * sizeof(TerrainVertexLisse), MTL::ResourceStorageModeShared);
    m_indexBuffer = m_device->newBuffer(m_indexCount * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    
    // Generate indices (CPU side, simple grid)
    uint32_t* indices = static_cast<uint32_t*>(m_indexBuffer->contents());
    uint32_t idx = 0;
    for (uint32_t z = 0; z < chunkSize; z++) {
        for (uint32_t x = 0; x < chunkSize; x++) {
            uint32_t topLeft = z * (chunkSize + 1) + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (z + 1) * (chunkSize + 1) + x;
            uint32_t bottomRight = bottomLeft + 1;
            
            indices[idx++] = topLeft;
            indices[idx++] = bottomLeft;
            indices[idx++] = topRight;
            indices[idx++] = topRight;
            indices[idx++] = bottomLeft;
            indices[idx++] = bottomRight;
        }
    }
    
    // Chunk params
    struct ChunkParams {
        simd::float2 origin;
        uint32_t size;
        uint32_t padding;
    } chunkParams = { chunkOrigin, chunkSize, 0 };
    
    auto paramsBuffer = m_device->newBuffer(sizeof(ChunkParams), MTL::ResourceStorageModeShared);
    memcpy(paramsBuffer->contents(), &chunkParams, sizeof(ChunkParams));
    
    // Pass 1: Generate heightmap + biomes
    auto encoder = cmd->computeCommandEncoder();
    encoder->setComputePipelineState(m_terrainPipeline);
    encoder->setBuffer(m_vertexBuffer, 0, 0);
    encoder->setBuffer(m_configBuffer, 0, 1);
    encoder->setBuffer(paramsBuffer, 0, 2);
    
    MTL::Size gridSize = MTL::Size(chunkSize + 1, chunkSize + 1, 1);
    MTL::Size threadGroupSize = MTL::Size(16, 16, 1);
    encoder->dispatchThreads(gridSize, threadGroupSize);
    
    // Pass 2: Compute normals
    encoder->setComputePipelineState(m_normalsPipeline);
    encoder->setBuffer(m_vertexBuffer, 0, 0);
    encoder->setBuffer(paramsBuffer, 0, 1);
    encoder->dispatchThreads(gridSize, threadGroupSize);
    
    encoder->endEncoding();
    paramsBuffer->release();
}

InfiniteTerrainManager::InfiniteTerrainManager(MTL::Device* device, uint32_t seed)
    : m_device(device)
{
    m_config.seed = seed;
    m_config.flatRadius = 50.0f;
    m_config.maxHeight = 60.0f;
    m_config.flatness = 0.7f;
    m_config.center = simd::float2{0, 0};
    
    m_configBuffer = m_device->newBuffer(sizeof(TerrainConfigLisse), MTL::ResourceStorageModeShared);
    memcpy(m_configBuffer->contents(), &m_config, sizeof(TerrainConfigLisse));
    
    // Build pipelines
    NS::Error* error = nullptr;
    auto library = m_device->newDefaultLibrary();
    
    auto terrainFunc = library->newFunction(NS::String::string("terrainGenerateKernel", NS::UTF8StringEncoding));
    m_terrainPipeline = m_device->newComputePipelineState(terrainFunc, &error);
    terrainFunc->release();
    
    auto normalsFunc = library->newFunction(NS::String::string("computeNormalsKernel", NS::UTF8StringEncoding));
    m_normalsPipeline = m_device->newComputePipelineState(normalsFunc, &error);
    normalsFunc->release();
    
    library->release();
    buildRenderPipeline(MTL::PixelFormatRGBA16Float, MTL::PixelFormatDepth32Float);
}

InfiniteTerrainManager::~InfiniteTerrainManager()
{
    for (auto& [coord, chunk] : m_chunks)
    {
        if (chunk.vertexBuffer) chunk.vertexBuffer->release();
        if (chunk.indexBuffer) chunk.indexBuffer->release();
    }
    m_terrainPipeline->release();
    m_normalsPipeline->release();
    m_configBuffer->release();
}

void InfiniteTerrainManager::buildRenderPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat) {
    NS::Error* error = nullptr;
    auto library = m_device->newDefaultLibrary();
    
    auto vertexFunc = library->newFunction(NS::String::string("terrainVertexShader", NS::UTF8StringEncoding));
    auto fragmentFunc = library->newFunction(NS::String::string("terrainFragmentShader", NS::UTF8StringEncoding));
    
    // Vertex descriptor pour TerrainVertexLisse
    auto vertexDesc = MTL::VertexDescriptor::alloc()->init();
    
    // Position: float3 at offset 0
    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(0)->setOffset(0);
    vertexDesc->attributes()->object(0)->setBufferIndex(0);
    
    // Normal: float3 at offset 12
    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(1)->setOffset(sizeof(simd::float3));
    vertexDesc->attributes()->object(1)->setBufferIndex(0);
    
    // UV: float2 at offset 24
    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(2)->setOffset(sizeof(simd::float3) * 2);
    vertexDesc->attributes()->object(2)->setBufferIndex(0);
    
    // BiomeID: uint at offset 32
    vertexDesc->attributes()->object(3)->setFormat(MTL::VertexFormatUInt);
    vertexDesc->attributes()->object(3)->setOffset(sizeof(simd::float3) * 2 + sizeof(simd::float2));
    vertexDesc->attributes()->object(3)->setBufferIndex(0);
    
    // Layout
    vertexDesc->layouts()->object(0)->setStride(sizeof(TerrainVertexLisse));
    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    // Pipeline descriptor
    auto pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setVertexFunction(vertexFunc);
    pipelineDesc->setFragmentFunction(fragmentFunc);
    pipelineDesc->setVertexDescriptor(vertexDesc);
    pipelineDesc->colorAttachments()->object(0)->setPixelFormat(colorFormat);
    pipelineDesc->setDepthAttachmentPixelFormat(depthFormat);
    
    m_renderPipeline = m_device->newRenderPipelineState(pipelineDesc, &error);
    if (error) {
        printf("Terrain pipeline error: %s\n", error->localizedDescription()->utf8String());
    }
    
    // Depth state
    auto depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    m_depthState = m_device->newDepthStencilState(depthDesc);
    
    vertexFunc->release();
    fragmentFunc->release();
    vertexDesc->release();
    pipelineDesc->release();
    depthDesc->release();
    library->release();
}

ChunkCoord InfiniteTerrainManager::worldToChunk(simd::float3 pos) const {
    return ChunkCoord{
        int32_t(floorf(pos.x / m_chunkWorldSize)),
        int32_t(floorf(pos.z / m_chunkWorldSize))
    };
}

void InfiniteTerrainManager::update(simd::float3 playerPos, MTL::CommandBuffer* cmd) {
    ChunkCoord playerChunk = worldToChunk(playerPos);
    
    // Générer chunks dans le rayon de vue
    for (int32_t dz = -int32_t(m_viewDistance); dz <= int32_t(m_viewDistance); dz++) {
        for (int32_t dx = -int32_t(m_viewDistance); dx <= int32_t(m_viewDistance); dx++) {
            ChunkCoord coord{playerChunk.x + dx, playerChunk.z + dz};
            
            // Check distance circulaire
            if (dx * dx + dz * dz > int32_t(m_viewDistance * m_viewDistance)) continue;
            
            // Chunk pas encore généré ?
            if (m_chunks.find(coord) == m_chunks.end()) {
                generateChunk(coord, cmd);
            }
        }
    }
    
    unloadDistantChunks(playerChunk);
}

void InfiniteTerrainManager::generateChunk(ChunkCoord coord, MTL::CommandBuffer* cmd) {
    TerrainChunkLisse chunk;
    chunk.worldOrigin = simd::float3{
        float(coord.x) * m_chunkWorldSize,
        0.0f,
        float(coord.z) * m_chunkWorldSize
    };
    chunk.ready = false;
    
    uint32_t vertexCount = (m_chunkSize + 1) * (m_chunkSize + 1);
    uint32_t quadCount = m_chunkSize * m_chunkSize;
    chunk.indexCount = quadCount * 6;
    
    chunk.vertexBuffer = m_device->newBuffer(vertexCount * sizeof(TerrainVertexLisse), MTL::ResourceStorageModeShared);
    chunk.indexBuffer = m_device->newBuffer(chunk.indexCount * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    
    // Generate indices CPU-side
    uint32_t* indices = static_cast<uint32_t*>(chunk.indexBuffer->contents());
    uint32_t idx = 0;
    for (uint32_t z = 0; z < m_chunkSize; z++) {
        for (uint32_t x = 0; x < m_chunkSize; x++) {
            uint32_t tl = z * (m_chunkSize + 1) + x;
            uint32_t tr = tl + 1;
            uint32_t bl = (z + 1) * (m_chunkSize + 1) + x;
            uint32_t br = bl + 1;
            
            indices[idx++] = tl;
            indices[idx++] = bl;
            indices[idx++] = tr;
            indices[idx++] = tr;
            indices[idx++] = bl;
            indices[idx++] = br;
        }
    }
    
    // Chunk params pour GPU
    struct ChunkParams {
        simd::float2 origin;
        uint32_t size;
        uint32_t padding;
    } params = {
        simd::float2{chunk.worldOrigin.x, chunk.worldOrigin.z},
        m_chunkSize,
        0
    };
    
    auto paramsBuffer = m_device->newBuffer(sizeof(ChunkParams), MTL::ResourceStorageModeShared);
    memcpy(paramsBuffer->contents(), &params, sizeof(ChunkParams));
    
    // Compute pass
    auto encoder = cmd->computeCommandEncoder();
    
    // Pass 1: heightmap + biomes
    encoder->setComputePipelineState(m_terrainPipeline);
    encoder->setBuffer(chunk.vertexBuffer, 0, 0);
    encoder->setBuffer(m_configBuffer, 0, 1);
    encoder->setBuffer(paramsBuffer, 0, 2);
    
    MTL::Size gridSize = MTL::Size(m_chunkSize + 1, m_chunkSize + 1, 1);
    MTL::Size threadGroup = MTL::Size(16, 16, 1);
    encoder->dispatchThreads(gridSize, threadGroup);
    
    // Pass 2: normals
    encoder->setComputePipelineState(m_normalsPipeline);
    encoder->dispatchThreads(gridSize, threadGroup);
    
    encoder->endEncoding();
    
    chunk.ready = true;
    m_chunks[coord] = chunk;
    // Cleanup callback
    cmd->addCompletedHandler([paramsBuffer, coord, this](MTL::CommandBuffer*) {
        paramsBuffer->release();
        if (m_chunks.find(coord) != m_chunks.end()) {
            m_chunks[coord].ready = true;
        }
    });
//    cmd->addCompletedHandler([paramsBuffer](MTL::CommandBuffer*) {
//            paramsBuffer->release();
//        });
    
//    m_chunks[coord] = chunk;
}

void InfiniteTerrainManager::unloadDistantChunks(ChunkCoord playerChunk) {
    uint32_t unloadDist = m_viewDistance + 2;
    
    std::vector<ChunkCoord> toRemove;
    for (auto& [coord, chunk] : m_chunks) {
        int32_t dx = coord.x - playerChunk.x;
        int32_t dz = coord.z - playerChunk.z;
        
        if (dx * dx + dz * dz > int32_t(unloadDist * unloadDist)) {
            toRemove.push_back(coord);
        }
    }
    
    for (auto& coord : toRemove) {
        auto& chunk = m_chunks[coord];
        if (chunk.vertexBuffer) chunk.vertexBuffer->release();
        if (chunk.indexBuffer) chunk.indexBuffer->release();
        m_chunks.erase(coord);
    }
}

void InfiniteTerrainManager::render(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection)
{
    if (m_chunks.empty()) return;
        
        encoder->setRenderPipelineState(m_renderPipeline);
        encoder->setDepthStencilState(m_depthState);
        encoder->setVertexBytes(&viewProjection, sizeof(simd::float4x4), 1);
        
        for (auto& [coord, chunk] : m_chunks)
        {
            if (!chunk.ready) continue;
            
            encoder->setVertexBuffer(chunk.vertexBuffer, 0, 0);
            encoder->drawIndexedPrimitives(
                MTL::PrimitiveTypeTriangle,
                chunk.indexCount,
                MTL::IndexTypeUInt32,
                chunk.indexBuffer,
                0
            );
        }
}
