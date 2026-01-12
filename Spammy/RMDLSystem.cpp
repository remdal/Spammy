//
//  RMDLSystem.cpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include "RMDLSystem.hpp"

BiomeGen::BiomeGen(uint32_t seed)
    : m_rng(seed)
    , m_cellBuffer(nullptr)
{
    
}

BiomeType BiomeGen::determineBiomeType(float distanceFromSpawn, float randomValue)
{
    // Nearby spawn: gentler biomes
    if (distanceFromSpawn < 200.0f) {
        if (randomValue < 0.6f) return BiomeType::VoronoiPlains;
        else if (randomValue < 0.9f) return BiomeType::VoronoiTerrace;
        else return BiomeType::VoronoiEroded;
    }
    
    // Mid-range: more variety
    if (distanceFromSpawn < 500.0f) {
        if (randomValue < 0.2f) return BiomeType::VoronoiIslands;
        else if (randomValue < 0.4f) return BiomeType::VoronoiEroded;
        else if (randomValue < 0.6f) return BiomeType::CrystalCaves;
        else if (randomValue < 0.8f) return BiomeType::VoronoiTerrace;
        else return BiomeType::Ocean;
    }
    
    // Far away: extreme biomes
    if (randomValue < 0.15f) return BiomeType::Planet;
    else if (randomValue < 0.3f) return BiomeType::Moon;
    else if (randomValue < 0.45f) return BiomeType::Ocean;
    else if (randomValue < 0.6f) return BiomeType::FloatingIslands;
    else if (randomValue < 0.75f) return BiomeType::LavaFields;
    else if (randomValue < 0.9f) return BiomeType::IceShelf;
    else return BiomeType::CrystalCaves;
}

void BiomeGen::generate(float worldSize, int numCells) {
    m_cells.clear();
    m_cells.reserve(numCells);
    
    std::uniform_real_distribution<float> posDist(-worldSize/2, worldSize/2);
    std::uniform_real_distribution<float> radiusDist(50.0f, 150.0f);
    std::uniform_int_distribution<int> biomeDist(0, (int)BiomeType::COUNT - 1);
    
    // Always create spawn crater at center
    VoronoiCell spawnCell;
    spawnCell.center = {0.0f, 0.0f};
    spawnCell.biomeType = BiomeType::SpawnCrater;
    spawnCell.radius = 100.0f;
    spawnCell.color = {0.6f, 0.55f, 0.4f, 1.0f}; // Sandy/rocky color
    m_cells.push_back(spawnCell);
    
    // Generate other cells
    for (int i = 1; i < numCells; i++) {
        VoronoiCell cell;
        cell.center = {posDist(m_rng), posDist(m_rng)};
        
        float distFromSpawn = simd::length(cell.center);
        
        // Determine biome type based on distance from spawn
        cell.biomeType = determineBiomeType(distFromSpawn, (float)biomeDist(m_rng) / (float)BiomeType::COUNT);
        cell.radius = radiusDist(m_rng);
        
        // Assign color based on biome
        switch (cell.biomeType) {
            case BiomeType::VoronoiPlains:
                cell.color = {0.4f, 0.7f, 0.3f, 1.0f}; // Green
                break;
            case BiomeType::VoronoiIslands:
                cell.color = {0.2f, 0.5f, 0.8f, 1.0f}; // Blue-green
                break;
            case BiomeType::VoronoiTerrace:
                cell.color = {0.7f, 0.6f, 0.4f, 1.0f}; // Brown
                break;
            case BiomeType::VoronoiEroded:
                cell.color = {0.6f, 0.4f, 0.3f, 1.0f}; // Red-brown
                break;
            case BiomeType::Planet:
                cell.color = {0.5f, 0.3f, 0.2f, 1.0f}; // Mars-like
                break;
            case BiomeType::Moon:
                cell.color = {0.7f, 0.7f, 0.7f, 1.0f}; // Gray
                break;
            case BiomeType::Ocean:
                cell.color = {0.1f, 0.3f, 0.6f, 1.0f}; // Deep blue
                break;
            case BiomeType::CrystalCaves:
                cell.color = {0.5f, 0.7f, 0.9f, 1.0f}; // Crystal blue
                break;
            case BiomeType::FloatingIslands:
                cell.color = {0.3f, 0.6f, 0.4f, 1.0f}; // Green with altitude
                break;
            case BiomeType::LavaFields:
                cell.color = {0.9f, 0.3f, 0.1f, 1.0f}; // Orange-red
                break;
            case BiomeType::IceShelf:
                cell.color = {0.8f, 0.9f, 1.0f, 1.0f}; // Icy white
                break;
            default:
                cell.color = {0.5f, 0.5f, 0.5f, 1.0f};
        }
        
        m_cells.push_back(cell);
    }
}

BiomeType BiomeGen::getBiomeAt(const simd::float2& position) const {
    float minDist = FLT_MAX;
    BiomeType closestBiome = BiomeType::SpawnCrater;
    
    for (const auto& cell : m_cells) {
        float dist = simd::length(position - cell.center);
        if (dist < minDist) {
            minDist = dist;
            closestBiome = cell.biomeType;
        }
    }
    
    return closestBiome;
}

simd::float2 BiomeGen::getNearestCellCenter(const simd::float2& position) const {
    float minDist = FLT_MAX;
    simd::float2 nearest = {0, 0};
    
    for (const auto& cell : m_cells) {
        float dist = simd::length(position - cell.center);
        if (dist < minDist) {
            minDist = dist;
            nearest = cell.center;
        }
    }
    
    return nearest;
}

float BiomeGen::getDistanceToNearestEdge(const simd::float2& position) const {
    float minDist1 = FLT_MAX;
    float minDist2 = FLT_MAX;
    
    for (const auto& cell : m_cells) {
        float dist = simd::length(position - cell.center);
        
        if (dist < minDist1) {
            minDist2 = minDist1;
            minDist1 = dist;
        } else if (dist < minDist2) {
            minDist2 = dist;
        }
    }
    
    return minDist2 - minDist1;
}

void BiomeGen::uploadToGPU(MTL::Device* device) {
    if (m_cellBuffer) {
        m_cellBuffer->release();
    }
    
    m_cellBuffer = device->newBuffer(m_cells.data(),
                                     m_cells.size() * sizeof(VoronoiCell),
                                     MTL::ResourceStorageModeShared);
}

// ============================================================================
// TerrainSystem Implementation
// ============================================================================

TerrainSystem::TerrainSystem(MTL::Device* device,
                             MTL::PixelFormat colorFormat,
                             MTL::PixelFormat depthFormat,
                             MTL::Library* library)
    : m_device(device->retain())
    , m_library(library->retain())
    , m_seed(89)
    , m_heightScale(1.0f)
    , m_maxRenderDistance(500.0f)
    , m_lodEnabled(true)
    , m_wireframe(false)
    , m_time(0.0f)
    , m_octaves(6)
    , m_persistence(0.5f)
    , m_lacunarity(2.0f)
    , m_noiseScale(0.01f)
{
    m_uniformsBuffer = m_device->newBuffer(sizeof(TerrainUniforms),
                                          MTL::ResourceStorageModeShared);
    
    createPipelines(colorFormat, depthFormat);
    createTextures();
    
    initialize(m_seed);
}

TerrainSystem::~TerrainSystem() {
    if (m_terrainPipeline) m_terrainPipeline->release();
    if (m_depthStencilState) m_depthStencilState->release();
    if (m_uniformsBuffer) m_uniformsBuffer->release();
    
    if (m_voronoiGeneratorPipeline) m_voronoiGeneratorPipeline->release();
    if (m_heightmapGeneratorPipeline) m_heightmapGeneratorPipeline->release();
    if (m_normalCalculatorPipeline) m_normalCalculatorPipeline->release();
    if (m_erosionPipeline) m_erosionPipeline->release();
    
    if (m_heightmapTexture) m_heightmapTexture->release();
    if (m_normalTexture) m_normalTexture->release();
    if (m_biomeTexture) m_biomeTexture->release();
    if (m_voronoiTexture) m_voronoiTexture->release();
    
    if (m_albedoArray) m_albedoArray->release();
    if (m_normalMapArray) m_normalMapArray->release();
    if (m_roughnessArray) m_roughnessArray->release();
    if (m_metallicArray) m_metallicArray->release();
    if (m_aoArray) m_aoArray->release();
    
    m_library->release();
    m_device->release();
}

void TerrainSystem::createPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat) {
    // Terrain rendering pipeline
    MTL::RenderPipelineDescriptor* terrainDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    MTL::Function* terrainVertFunc = m_library->newFunction(
        NS::String::string("terrainVertex", NS::UTF8StringEncoding));
    MTL::Function* terrainFragFunc = m_library->newFunction(
        NS::String::string("terrainFragment", NS::UTF8StringEncoding));
    
    MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();

    // 0 : position
    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(0)->setOffset(0);
    vertexDesc->attributes()->object(0)->setBufferIndex(0);

    // 1 : normal
    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(1)->setOffset(12);
    vertexDesc->attributes()->object(1)->setBufferIndex(0);

    // 2 : tangent
    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(2)->setOffset(24);
    vertexDesc->attributes()->object(2)->setBufferIndex(0);

    // 3 : texCoord
    vertexDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(3)->setOffset(36);
    vertexDesc->attributes()->object(3)->setBufferIndex(0);

    // 4 : color
    vertexDesc->attributes()->object(4)->setFormat(MTL::VertexFormatFloat4);
    vertexDesc->attributes()->object(4)->setOffset(44);
    vertexDesc->attributes()->object(4)->setBufferIndex(0);

    // 5 : height
    vertexDesc->attributes()->object(5)->setFormat(MTL::VertexFormatFloat);
    vertexDesc->attributes()->object(5)->setOffset(60);
    vertexDesc->attributes()->object(5)->setBufferIndex(0);

    // 6 : biomeId
    vertexDesc->attributes()->object(6)->setFormat(MTL::VertexFormatFloat);
    vertexDesc->attributes()->object(6)->setOffset(64);
    vertexDesc->attributes()->object(6)->setBufferIndex(0);

    // layout
    vertexDesc->layouts()->object(0)->setStride(72);
    vertexDesc->layouts()->object(0)->setStepRate(1);
    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    if (terrainVertFunc && terrainFragFunc) {
        terrainDesc->setVertexDescriptor(vertexDesc);
        terrainDesc->setVertexFunction(terrainVertFunc);
        terrainDesc->setFragmentFunction(terrainFragFunc);
        terrainDesc->colorAttachments()->object(0)->setPixelFormat(colorFormat);
        terrainDesc->setDepthAttachmentPixelFormat(depthFormat);
        
        NS::Error* error = nullptr;
        m_terrainPipeline = m_device->newRenderPipelineState(terrainDesc, &error);
        if (error) {
            printf("Terrain pipeline error: %s\n",
                   error->localizedDescription()->utf8String());
        }
        
        terrainVertFunc->release();
        terrainFragFunc->release();
    }
    terrainDesc->release();
    
    // Depth stencil state
    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    m_depthStencilState = m_device->newDepthStencilState(depthDesc);
    depthDesc->release();
    
    // Compute pipelines
    NS::Error* error = nullptr;
    
    MTL::Function* voronoiFunc = m_library->newFunction(
        NS::String::string("advancedVoronoiGenerator", NS::UTF8StringEncoding));
    if (voronoiFunc) {
        m_voronoiGeneratorPipeline = m_device->newComputePipelineState(voronoiFunc, &error);
        voronoiFunc->release();
    }
    
    MTL::Function* heightFunc = m_library->newFunction(
        NS::String::string("voronoiHeightmapGenerator", NS::UTF8StringEncoding));
    if (heightFunc) {
        m_heightmapGeneratorPipeline = m_device->newComputePipelineState(heightFunc, &error);
        heightFunc->release();
    }
    
    MTL::Function* normalFunc = m_library->newFunction(
        NS::String::string("calculateNormals", NS::UTF8StringEncoding));
    if (normalFunc) {
        m_normalCalculatorPipeline = m_device->newComputePipelineState(normalFunc, &error);
        normalFunc->release();
    }
    
    MTL::Function* erosionFunc = m_library->newFunction(
        NS::String::string("thermalErosion", NS::UTF8StringEncoding));
    if (erosionFunc) {
        m_erosionPipeline = m_device->newComputePipelineState(erosionFunc, &error);
        erosionFunc->release();
    }
}

void TerrainSystem::createTextures()
{
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA16Float, 2048, 2048, false);
    texDesc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    texDesc->setStorageMode(MTL::StorageModePrivate);
    
    m_heightmapTexture = m_device->newTexture(texDesc);
    m_normalTexture = m_device->newTexture(texDesc);
    m_biomeTexture = m_device->newTexture(texDesc);
    m_voronoiTexture = m_device->newTexture(texDesc);
    
    // TODO: Create texture arrays for materials
    // For now, create placeholder textures
    MTL::TextureDescriptor* arrayDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA16Float, 512, 512, false);
    arrayDesc->setTextureType(MTL::TextureType2DArray);
    arrayDesc->setArrayLength((int)BiomeType::COUNT);
    arrayDesc->setUsage(MTL::TextureUsageShaderRead);
    
    m_albedoArray = m_device->newTexture(arrayDesc);
    m_normalMapArray = m_device->newTexture(arrayDesc);
    m_roughnessArray = m_device->newTexture(arrayDesc);
    m_metallicArray = m_device->newTexture(arrayDesc);
    m_aoArray = m_device->newTexture(arrayDesc);
}

void TerrainSystem::initialize(uint32_t seed)
{
    m_seed = seed;
    
    // Generate biome map
    m_biomeGenerator = std::make_unique<BiomeGen>(seed);
    m_biomeGenerator->generate(2048.0f, 50); // 50 Voronoi cells
    m_biomeGenerator->uploadToGPU(m_device);
    
    // Generate heightmap on GPU
    generateHeightmapGPU();
    
    // Calculate normals on GPU
    generateNormalsGPU();
    
    // Apply erosion
    applyErosionGPU(5);
    
    // Generate initial chunks
    generateChunkMesh(getOrCreateChunk(0, 0));
}

void TerrainSystem::generateHeightmapGPU() {
    if (!m_voronoiGeneratorPipeline) return;
    
    MTL::CommandQueue* queue = m_device->newCommandQueue();
    MTL::CommandBuffer* cmd = queue->commandBuffer();
    MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
    
    enc->setComputePipelineState(m_voronoiGeneratorPipeline);
    enc->setTexture(m_heightmapTexture, 0);
    enc->setTexture(m_biomeTexture, 1);
    enc->setTexture(m_voronoiTexture, 2);
    enc->setBuffer(m_biomeGenerator->getCellBuffer(), 0, 0);
    
    uint32_t numCells = (uint32_t)m_biomeGenerator->getCells().size();
    enc->setBytes(&numCells, sizeof(uint32_t), 1);
    enc->setBytes(&m_seed, sizeof(uint32_t), 2);
    
    float worldSize = 2048.0f;
    enc->setBytes(&worldSize, sizeof(float), 3);
    
    MTL::Size gridSize = MTL::Size::Make(2048, 2048, 1);
    MTL::Size threadgroupSize = MTL::Size::Make(16, 16, 1);
    
    enc->dispatchThreads(gridSize, threadgroupSize);
    enc->endEncoding();
    
    cmd->commit();
    cmd->waitUntilCompleted();
    
    queue->release();
}

void TerrainSystem::generateNormalsGPU() {
    if (!m_normalCalculatorPipeline) return;
    
    MTL::CommandQueue* queue = m_device->newCommandQueue();
    MTL::CommandBuffer* cmd = queue->commandBuffer();
    MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
    
    enc->setComputePipelineState(m_normalCalculatorPipeline);
    enc->setTexture(m_heightmapTexture, 0);
    enc->setTexture(m_normalTexture, 1);
    enc->setBytes(&m_heightScale, sizeof(float), 0);
    
    MTL::Size gridSize = MTL::Size::Make(2048, 2048, 1);
    MTL::Size threadgroupSize = MTL::Size::Make(16, 16, 1);
    
    enc->dispatchThreads(gridSize, threadgroupSize);
    enc->endEncoding();
    
    cmd->commit();
    cmd->waitUntilCompleted();
    
    queue->release();
}

void TerrainSystem::applyErosionGPU(int iterations) {
    if (!m_erosionPipeline) return;
    
    MTL::CommandQueue* queue = m_device->newCommandQueue();
    
    // Create temp texture for ping-pong
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA16Float, 2048, 2048, false);
    texDesc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    texDesc->setStorageMode(MTL::StorageModePrivate);
    MTL::Texture* tempTexture = m_device->newTexture(texDesc);
    
    float talusAngle = 0.7f; // ~40 degrees
    float erosionRate = 0.1f;
    
    for (int i = 0; i < iterations; i++) {
        MTL::CommandBuffer* cmd = queue->commandBuffer();
        MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
        
        enc->setComputePipelineState(m_erosionPipeline);
        enc->setTexture((i % 2 == 0) ? m_heightmapTexture : tempTexture, 0);
        enc->setTexture((i % 2 == 0) ? tempTexture : m_heightmapTexture, 1);
        enc->setBytes(&talusAngle, sizeof(float), 0);
        enc->setBytes(&erosionRate, sizeof(float), 1);
        
        MTL::Size gridSize = MTL::Size::Make(2048, 2048, 1);
        MTL::Size threadgroupSize = MTL::Size::Make(16, 16, 1);
        
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
        
        cmd->commit();
        cmd->waitUntilCompleted();
    }
    
    tempTexture->release();
    queue->release();
    
    // Recalculate normals after erosion
    generateNormalsGPU();
}

void TerrainSystem::update(float deltaTime, const simd::float3& cameraPosition) {
    m_time += deltaTime;
    
    // Update chunks (streaming, LOD)
    streamChunks(cameraPosition);
    unloadDistantChunks(cameraPosition);
    
    // Update uniforms
    m_uniforms.cameraPosition = cameraPosition;
    m_uniforms.time = m_time;
    m_uniforms.seed = m_seed;
    m_uniforms.heightScale = m_heightScale;
    m_uniforms.maxRenderDistance = m_maxRenderDistance;
    m_uniforms.fogColor = {0.6f, 0.7f, 0.8f, 1.0f};
    m_uniforms.fogDensity = 0.001f;
    
    memcpy(m_uniformsBuffer->contents(), &m_uniforms, sizeof(TerrainUniforms));
}

void TerrainSystem::render(MTL::RenderCommandEncoder* encoder,
                          const simd::float4x4& viewProjection)
{
    if (!m_terrainPipeline)
        return;

    encoder->setRenderPipelineState(m_terrainPipeline);
    encoder->setDepthStencilState(m_depthStencilState);
    
    m_uniforms.viewProjectionMatrix = viewProjection;
    m_uniforms.modelMatrix = math::makeIdentity();
    memcpy(m_uniformsBuffer->contents(), &m_uniforms, sizeof(TerrainUniforms));
    
    encoder->setVertexBuffer(m_uniformsBuffer, 0, 1);
    encoder->setFragmentBuffer(m_uniformsBuffer, 0, 0);
    
    // Set textures
    encoder->setFragmentTexture(m_albedoArray, 0);
    encoder->setFragmentTexture(m_normalMapArray, 1);
    encoder->setFragmentTexture(m_roughnessArray, 2);
    encoder->setFragmentTexture(m_metallicArray, 3);
    encoder->setFragmentTexture(m_aoArray, 4);
    
    // Render all visible chunks
    for (const auto& chunk : m_chunks) {
        if (!chunk->visible || chunk->indexCount == 0) continue;
        
        encoder->setVertexBuffer(chunk->vertexBuffer, 0, 0);
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                      chunk->indexCount,
                                      MTL::IndexTypeUInt32,
                                      chunk->indexBuffer,
                                      0);
    }
}

TerrainChunk* TerrainSystem::getOrCreateChunk(int chunkX, int chunkZ) {
    // Check if chunk exists
    for (auto& chunk : m_chunks) {
        if (chunk->chunkCoord.x == chunkX && chunk->chunkCoord.y == chunkZ) {
            return chunk.get();
        }
    }
    
    // Create new chunk
    auto chunk = std::make_unique<TerrainChunk>();
    chunk->chunkCoord = {chunkX, chunkZ};
    chunk->needsUpdate = true;
    
    TerrainChunk* ptr = chunk.get();
    m_chunks.push_back(std::move(chunk));
    
    return ptr;
}

void TerrainSystem::generateChunkMesh(TerrainChunk* chunk) {
    // Generate mesh for this chunk
    // Simplified version - full implementation would sample heightmap texture
    
    std::vector<TerrainVertex> vertices;
    std::vector<uint32_t> indices;
    
    int resolution = TerrainChunk::VERTEX_RESOLUTION;
    float chunkWorldSize = TerrainChunk::CHUNK_SIZE;
    float vertexSpacing = chunkWorldSize / (resolution - 1);
    
    float offsetX = chunk->chunkCoord.x * chunkWorldSize;
    float offsetZ = chunk->chunkCoord.y * chunkWorldSize;
    
    // Generate grid vertices
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            TerrainVertex vertex;
            vertex.position = {
                offsetX + x * vertexSpacing,
                0.0f, // Will be set from heightmap
                offsetZ + z * vertexSpacing
            };
            
            // Sample heightmap (simplified - would read from GPU texture)
            float height = sampleHeightmap(vertex.position.x, vertex.position.z);
            vertex.position.y = height;
            vertex.height = height;
            
            // Calculate UVs
            vertex.texCoord = {
                (float)x / (resolution - 1),
                (float)z / (resolution - 1)
            };
            
            // Normal and tangent would be calculated or read from texture
            vertex.normal = {0, 1, 0};
            vertex.tangent = {1, 0, 0};
            
            // Biome color
            vertex.biomeId = (float)m_biomeGenerator->getBiomeAt({vertex.position.x, vertex.position.z});
            
            vertices.push_back(vertex);
        }
    }
    
    // Generate indices
    for (int z = 0; z < resolution - 1; z++) {
        for (int x = 0; x < resolution - 1; x++) {
            int baseIndex = z * resolution + x;
            
            // Triangle 1
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + resolution);
            indices.push_back(baseIndex + 1);
            
            // Triangle 2
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + resolution);
            indices.push_back(baseIndex + resolution + 1);
        }
    }
    
    chunk->indexCount = indices.size();
    
    // Create GPU buffers
    if (chunk->vertexBuffer) chunk->vertexBuffer->release();
    if (chunk->indexBuffer) chunk->indexBuffer->release();
    
    chunk->vertexBuffer = m_device->newBuffer(vertices.data(),
                                             vertices.size() * sizeof(TerrainVertex),
                                             MTL::ResourceStorageModeShared);
    chunk->indexBuffer = m_device->newBuffer(indices.data(),
                                            indices.size() * sizeof(uint32_t),
                                            MTL::ResourceStorageModeShared);
    
    chunk->needsUpdate = false;
}

float TerrainSystem::sampleHeightmap(float x, float z) const
{
    // Simplified CPU version - in real implementation would read from GPU texture
    // For now, use biome generator
    
    BiomeType biome = m_biomeGenerator->getBiomeAt({x, z});
    // Noise multi-octave simple
    auto simpleNoise = [](float x, float z, float freq) -> float {
        return sin(x * freq) * cos(z * freq);
    };

    float height = 0.0f;
    float amplitude = 20.0f;
    float frequency = 0.005f;
    
    // FBM simple
    for (int i = 0; i < 6; i++)
    {
        height += simpleNoise(x, z, frequency) * amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    // Base height par biome
    float baseHeight = 4.0f;
    float distFromCenter = sqrt(x*x + z*z);
    switch (biome)
    {
        case BiomeType::SpawnCrater:
            // Cratère plat au centre
            baseHeight = std::max(0.0f, 20.0f - distFromCenter * 0.1f);
            break;
        case BiomeType::VoronoiPlains:
            baseHeight = 15.0f;
            break;
            
        case BiomeType::VoronoiIslands:
            baseHeight = (height > 5.0f) ? height : -10.0f;
            break;
            
        case BiomeType::VoronoiTerrace:
            baseHeight = floor(height / 10.0f) * 10.0f;
            break;
            
        case BiomeType::Ocean:
            baseHeight = -15.0f;
            break;
            
        case BiomeType::Moon:
            baseHeight = 12.0f + height * 0.3f;
            break;
            
        default:
            baseHeight = 10.0f;
    }
    return (baseHeight + height * 0.5f) * m_heightScale;
    
    // Simple noise-based height
    // In real version, this would read from the GPU-generated heightmap texture
    
    return height * m_heightScale;
}

float TerrainSystem::getHeightAt(float x, float z) const {
    return sampleHeightmap(x, z);
}

simd::float3 TerrainSystem::getNormalAt(float x, float z) const {
    float h = getHeightAt(x, z);
    float hL = getHeightAt(x - 1.0f, z);
    float hR = getHeightAt(x + 1.0f, z);
    float hD = getHeightAt(x, z - 1.0f);
    float hU = getHeightAt(x, z + 1.0f);
    
    simd::float3 normal;
    normal.x = hL - hR;
    normal.y = 2.0f;
    normal.z = hD - hU;
    
    return simd::normalize(normal);
}

BiomeType TerrainSystem::getBiomeAt(float x, float z) const {
    return m_biomeGenerator->getBiomeAt({x, z});
}

void TerrainSystem::streamChunks(const simd::float3& cameraPosition) {
    // Load chunks around camera
    int cameraChunkX = (int)floorf(cameraPosition.x / TerrainChunk::CHUNK_SIZE);
    int cameraChunkZ = (int)floorf(cameraPosition.z / TerrainChunk::CHUNK_SIZE);
    
    int loadRadius = 8; // Load 8 chunks in each direction
    
    for (int z = -loadRadius; z <= loadRadius; z++) {
        for (int x = -loadRadius; x <= loadRadius; x++) {
            int chunkX = cameraChunkX + x;
            int chunkZ = cameraChunkZ + z;
            
            TerrainChunk* chunk = getOrCreateChunk(chunkX, chunkZ);
            if (chunk->needsUpdate) {
                generateChunkMesh(chunk);
            }
        }
    }
}

void TerrainSystem::unloadDistantChunks(const simd::float3& cameraPosition) {
    // Remove chunks that are too far
    m_chunks.erase(
        std::remove_if(m_chunks.begin(), m_chunks.end(),
            [&](const std::unique_ptr<TerrainChunk>& chunk) {
                simd::float2 chunkCenter = {
                    chunk->chunkCoord.x * TerrainChunk::CHUNK_SIZE + TerrainChunk::CHUNK_SIZE * 0.5f,
                    chunk->chunkCoord.y * TerrainChunk::CHUNK_SIZE + TerrainChunk::CHUNK_SIZE * 0.5f
                };
                
                float distance = simd::length(simd::float2{cameraPosition.x, cameraPosition.z} - chunkCenter);
                
                if (distance > m_maxRenderDistance * 1.5f) {
                    if (chunk->vertexBuffer) chunk->vertexBuffer->release();
                    if (chunk->indexBuffer) chunk->indexBuffer->release();
                    return true;
                }
                
                chunk->visible = distance < m_maxRenderDistance;
                return false;
            }
        ),
        m_chunks.end()
    );
}

bool TerrainSystem::checkTerrainCollision(const simd::float3& position, float radius) const {
    float terrainHeight = getHeightAt(position.x, position.z);
    return position.y - radius < terrainHeight;
}

simd::float3 TerrainSystem::resolveTerrainCollision(const simd::float3& position,
                                                    const simd::float3& velocity,
                                                    float radius) const {
    float terrainHeight = getHeightAt(position.x, position.z);
    simd::float3 resolved = position;
    
    if (position.y - radius < terrainHeight) {
        resolved.y = terrainHeight + radius;
    }
    
    return resolved;
}
