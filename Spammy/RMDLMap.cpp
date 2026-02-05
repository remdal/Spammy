//
//  RMDLMap.cpp
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#include "RMDLMap.hpp"

InfiniteTerrainManager::InfiniteTerrainManager(MTL::Device* device, uint32_t seed)
: m_device(device)
{
    m_config.seed = seed;
    m_config.flatRadius = 150.0f;
    m_config.maxHeight = 160.0f;
    m_config.flatness = 0.7f;
    m_config.center = simd::float2{0, 0};
    
    m_configBuffer = m_device->newBuffer(sizeof(TerrainConfigLisse), MTL::ResourceStorageModeShared);
    memcpy(m_configBuffer->contents(), &m_config, sizeof(TerrainConfigLisse));
    
    NS::Error* error = nullptr;
    auto library = m_device->newDefaultLibrary();
    
    auto terrainFunc = library->newFunction(NS::String::string("terrainGenerateKernel", NS::UTF8StringEncoding));
    m_terrainPipeline = m_device->newComputePipelineState(terrainFunc, &error);
    terrainFunc->release();
    
    auto normalsFunc = library->newFunction(NS::String::string("computeNormalsKernel", NS::UTF8StringEncoding));
    m_normalsPipeline = m_device->newComputePipelineState(normalsFunc, &error);
    normalsFunc->release();
    
    // AJOUT : Height sample pipeline
    auto heightFunc = library->newFunction(NS::String::string("sampleTerrainPhysics", NS::UTF8StringEncoding));
    m_heightSamplePipeline = m_device->newComputePipelineState(heightFunc, &error);
    heightFunc->release();
    
    for (int i = 0; i < 2; i++)
    {
        m_queryBuffer[i] = m_device->newBuffer(sizeof(simd::float2), MTL::ResourceStorageModeShared);
        m_resultBuffer[i] = m_device->newBuffer(sizeof(PhysicsSample), MTL::ResourceStorageModeShared);
    }
    
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
    vertexDesc->attributes()->object(3)->setOffset(offsetof(TerrainVertexLisse, biomeID));//sizeof(simd::float3) * 2 + sizeof(simd::float2));
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

ChunkCoord InfiniteTerrainManager::worldToChunk(simd::float3 pos) const
{
    return ChunkCoord{ int32_t(floorf(pos.x / m_chunkWorldSize)), int32_t(floorf(pos.z / m_chunkWorldSize)) };
}

void InfiniteTerrainManager::update(simd::float3 playerPos, MTL::CommandBuffer* cmd)
{
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
    dayTime += 0.0002f;//= fmod(totalTime * 0.11f, 1.0f); // ~100 sec -> 0.01f
    unloadDistantChunks(playerChunk);
    if (dayTime > 1.5f)
        dayTime = 0.f;
}

void InfiniteTerrainManager::generateChunk(ChunkCoord coord, MTL::CommandBuffer* cmd)
{
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
    encoder->setBuffer(chunk.vertexBuffer, 0, 0);
    encoder->setBuffer(paramsBuffer, 0, 1);
    encoder->setBuffer(m_configBuffer, 0, 2);  // AJOUT
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
}

void InfiniteTerrainManager::unloadDistantChunks(ChunkCoord playerChunk)
{
    uint32_t unloadDist = m_viewDistance + 15;
    
    std::vector<ChunkCoord> toRemove;
    for (auto& [coord, chunk] : m_chunks)
    {
        int32_t dx = coord.x - playerChunk.x;
        int32_t dz = coord.z - playerChunk.z;
        
        if (dx * dx + dz * dz > int32_t(unloadDist * unloadDist))
            toRemove.push_back(coord);
    }
    
    for (auto& coord : toRemove)
    {
        auto& chunk = m_chunks[coord];
        if (chunk.vertexBuffer) chunk.vertexBuffer->release();
        if (chunk.indexBuffer) chunk.indexBuffer->release();
        m_chunks.erase(coord);
    }
}

void InfiniteTerrainManager::requestHeightAt(float x, float z, MTL::CommandBuffer* cmd)
{
    if (m_heightReady)
    {
        uint32_t readBuffer = 1 - m_currentBuffer;
        PhysicsSample* result = static_cast<PhysicsSample*>(m_resultBuffer[readBuffer]->contents());
        m_lastHeight = result->height;
    }
    
    // Écrire la nouvelle query dans le buffer COURANT
    simd::float2* query = static_cast<simd::float2*>(m_queryBuffer[m_currentBuffer]->contents());
    *query = simd::float2{x, z};
    
    // Dispatch
    auto encoder = cmd->computeCommandEncoder();
    encoder->setComputePipelineState(m_heightSamplePipeline);
    encoder->setBuffer(m_queryBuffer[m_currentBuffer], 0, 0);
    encoder->setBuffer(m_resultBuffer[m_currentBuffer], 0, 1);
    encoder->setBuffer(m_configBuffer, 0, 2);
    encoder->dispatchThreads(MTL::Size(1, 1, 1), MTL::Size(1, 1, 1));
    encoder->endEncoding();
    
    // Swap buffers
    m_currentBuffer = 1 - m_currentBuffer;
    m_heightReady = true;
}

float InfiniteTerrainManager::getHeightAt(float x, float z, MTL::CommandBuffer* cmd)
{
//    // Lazy init des buffers
//    if (!m_singleQueryBuffer) {
//        m_singleQueryBuffer = m_device->newBuffer(sizeof(simd::float2), MTL::ResourceStorageModeShared);
//        m_singleResultBuffer = m_device->newBuffer(sizeof(PhysicsSample), MTL::ResourceStorageModeShared);
//        
//        // Build pipeline si pas fait
//        NS::Error* error = nullptr;
//        auto library = m_device->newDefaultLibrary();
//        auto func = library->newFunction(NS::String::string("sampleTerrainPhysics", NS::UTF8StringEncoding));
//        m_heightSamplePipeline = m_device->newComputePipelineState(func, &error);
//        if (error) {
//            printf("Height sample pipeline error: %s\n", error->localizedDescription()->utf8String());
//        }
//        func->release();
//        library->release();
//    }
//    
//    // Écrire la query
//    simd::float2* query = static_cast<simd::float2*>(m_singleQueryBuffer->contents());
//    *query = simd::float2{x, z};
//    
//    // Dispatch
//    auto encoder = cmd->computeCommandEncoder();
//    encoder->setComputePipelineState(m_heightSamplePipeline);
//    encoder->setBuffer(m_singleQueryBuffer, 0, 0);
//    encoder->setBuffer(m_singleResultBuffer, 0, 1);
//    encoder->setBuffer(m_configBuffer, 0, 2);
//    encoder->dispatchThreads(MTL::Size(1, 1, 1), MTL::Size(1, 1, 1));
//    encoder->endEncoding();
//    
//    // Lire le résultat (après commit + waitUntilCompleted)
//    cmd->commit();
//    cmd->waitUntilCompleted();
//    
//    PhysicsSample* result = static_cast<PhysicsSample*>(m_singleResultBuffer->contents());
//    return result->height;
    return 0.0f;
}

void InfiniteTerrainManager::render(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection, const simd::float3& cameraPosition)
{
    if (m_chunks.empty()) return;
        
    encoder->setRenderPipelineState(m_renderPipeline);
    encoder->setDepthStencilState(m_depthState);
    encoder->setVertexBytes(&viewProjection, sizeof(simd::float4x4), 1);
    encoder->setFragmentBytes(&cameraPosition, sizeof(simd::float3), 0);
    encoder->setFragmentBytes(&dayTime, sizeof(float), 1);
    
    for (auto& [coord, chunk] : m_chunks)
    {
        if (!chunk.ready) continue;
        
        encoder->setVertexBuffer(chunk.vertexBuffer, 0, 0);
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, chunk.indexCount, MTL::IndexTypeUInt32, chunk.indexBuffer, 0);
    }
}

