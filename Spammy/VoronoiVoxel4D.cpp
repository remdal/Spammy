//
//  VoronoiVoxel4D.cpp
//  Spammy
//
//  Created by Rémy on 25/12/2025.
//

#include "VoronoiVoxel4D.hpp"

VoronoiVoxel4D::VoronoiVoxel4D(uint32_t seed)
    : rng(seed), timeOffset(0.0f)
{
}

VoronoiVoxel4D::~VoronoiVoxel4D()
{
}

uint32_t VoronoiVoxel4D::hash(int x, int y, int z)
{
    uint32_t h = 2166136261u;
    h = (h ^ x) * 16777619u;
    h = (h ^ y) * 16777619u;
    h = (h ^ z) * 16777619u;
    return h;
}

void VoronoiVoxel4D::generateSitesForRegion(int chunkX, int chunkZ, float time)
{
    sites.clear();
    
    // Génère 50-80 sites par région autour du chunk
    const int regionSize = 3; // 3x3 chunks
    int numSites = 50 + (hash(chunkX, chunkZ, (int)time) % 30);
    
    for (int i = 0; i < numSites; ++i)
    {
        uint32_t h = hash(chunkX * 1000 + i, chunkZ * 1000, (int)(time * 100));
        std::mt19937 localRng(h);
        std::uniform_real_distribution<float> dist(-regionSize * CHUNK_SIZE * 0.5f, regionSize * CHUNK_SIZE * 1.5f);
        std::uniform_real_distribution<float> distTime(-10.0f, 10.0f);
        std::uniform_real_distribution<float> distInfluence(8.0f, 25.0f);
        
        VoronoiSite4D site;
        site.position = { chunkX * CHUNK_SIZE + dist(localRng), dist(localRng) * 0.3f + 40.0f, chunkZ * CHUNK_SIZE + dist(localRng), time + distTime(localRng) }; // Centré 40 / Y
        
        // Type de bloc basé sur position 4D
        float typeNoise = sinf(site.position.x * 0.1f) * cosf(site.position.w * 0.2f);
        if (typeNoise > 0.6f) site.blockType = BlockType::CRYSTAL_RED;
        else if (typeNoise > 0.3f) site.blockType = BlockType::CRYSTAL_BLUE;
        else if (typeNoise > 0.0f) site.blockType = BlockType::CRYSTAL_GREEN;
        else if (typeNoise > -0.3f) site.blockType = BlockType::ORGANIC;
        else if (typeNoise > -0.6f) site.blockType = BlockType::METAL;
        else site.blockType = BlockType::STONE;
        
        site.influence = distInfluence(localRng);
        sites.push_back(site);
    }
}

float VoronoiVoxel4D::distance4D(simd::float4 a, simd::float4 b)
{
    simd::float4 diff = a - b;
    return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z + diff.w * diff.w);
}

VoronoiSite4D* VoronoiVoxel4D::findClosestSite(simd::float3 worldPos, float time)
{
    simd::float4 pos4D = {worldPos.x, worldPos.y, worldPos.z, time};
    
    VoronoiSite4D* closest = nullptr;
    float minDist = std::numeric_limits<float>::max();
    
    for (auto& site : sites) {
        float dist = distance4D(pos4D, site.position);
        if (dist < minDist) {
            minDist = dist;
            closest = &site;
        }
    }
    
    return closest;
}

float VoronoiVoxel4D::worleyNoise(simd::float3 pos, float time)
{
    VoronoiSite4D* site = findClosestSite(pos, time);
    if (!site) return 1.0f;
    
    simd::float4 pos4D = {pos.x, pos.y, pos.z, time};
    float dist = distance4D(pos4D, site->position);
    
    return fminf(1.0f, dist / site->influence);
}

BlockType VoronoiVoxel4D::getBlockAtPosition(int worldX, int worldY, int worldZ, float time)
{
    simd::float3 pos = {(float)worldX, (float)worldY, (float)worldZ};
    
    VoronoiSite4D* site = findClosestSite(pos, time);
    if (!site) return BlockType::STONE;
    
    simd::float4 pos4D = {pos.x, pos.y, pos.z, time};
    float dist = distance4D(pos4D, site->position);
    
    // Distance au site détermine le type
    if (dist < site->influence * 0.3f) {
        return site->blockType; // Coeur du site
    } else if (dist < site->influence * 0.6f) {
        // Zone de transition
        return (site->blockType == BlockType::CRYSTAL_RED) ? BlockType::STONE : site->blockType;
    } else if (dist < site->influence) {
        return BlockType::STONE;
    }
    
    return BlockType::AIR;
}


Chunk::Chunk(int x, int z) : chunkX(x), chunkZ(z), vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0), needsRebuild(true)
{
    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_SIZE; ++z)
                blocks[x][y][z] = BlockType::AIR;
        }
    }
}

Chunk::~Chunk()
{
    vertexBuffer->release();
    indexBuffer->release();
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
        return BlockType::AIR;
    return blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, BlockType type)
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
        return;
    blocks[x][y][z] = type;
    needsRebuild = true;
}

bool Chunk::isBlockSolid(int x, int y, int z) const
{
    BlockType type = getBlock(x, y, z);
    return type != BlockType::AIR;
}

void Chunk::addCubeFace(std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices, simd::float3 pos, BlockType type, int face)
{
    simd::float4 color = BLOCK_COLORS[(int)type];
    uint32_t baseIndex = (uint32_t)vertices.size();

    simd::float3 normals[6] = { {0, 1, 0}, {0, -1, 0}, {0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {-1, 0, 0} };

    simd::float3 faceVertices[6][4] =
    {
        {{0,1,0}, {1,1,0}, {1,1,1}, {0,1,1}},
        {{0,0,0}, {0,0,1}, {1,0,1}, {1,0,0}},
        {{0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}},
        {{1,0,1}, {0,0,1}, {0,1,1}, {1,1,1}},
        {{1,0,0}, {1,0,1}, {1,1,1}, {1,1,0}},
        {{0,0,1}, {0,0,0}, {0,1,0}, {0,1,1}}
    };

    // Variation de couleur selon la face (éclairage ambiant)
    float brightness[6] = {1.0f, 0.5f, 0.7f, 0.7f, 0.9f, 0.6f};
    simd::float4 shadedColor = color * brightness[face];
    shadedColor.w = color.w;

    for (int i = 0; i < 4; ++i)
    {
        VoxelVertex v;
        v.position = pos + faceVertices[face][i] * VOXEL_SIZE;
        v.color = shadedColor;
        v.normal = normals[face];
        vertices.push_back(v);
    }

    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

void Chunk::rebuildMesh(MTL::Device* device)
{
    if (!needsRebuild)
        return;
    
    std::vector<VoxelVertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * 128 * 4);
    indices.reserve(CHUNK_SIZE * CHUNK_SIZE * 128 * 6);
    
    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_SIZE; ++z)
            {
                BlockType type = blocks[x][y][z];
                if (type == BlockType::AIR)
                    continue;
                
                simd::float3 worldPos = { (float)(chunkX * CHUNK_SIZE + x), (float)y, (float)(chunkZ * CHUNK_SIZE + z) };
                
                if (!isBlockSolid(x, y + 1, z))
                    addCubeFace(vertices, indices, worldPos, type, 0);
                if (!isBlockSolid(x, y - 1, z))
                    addCubeFace(vertices, indices, worldPos, type, 1);
                if (!isBlockSolid(x, y, z - 1))
                    addCubeFace(vertices, indices, worldPos, type, 2);
                if (!isBlockSolid(x, y, z + 1))
                    addCubeFace(vertices, indices, worldPos, type, 3);
                if (!isBlockSolid(x + 1, y, z))
                    addCubeFace(vertices, indices, worldPos, type, 4);
                if (!isBlockSolid(x - 1, y, z))
                    addCubeFace(vertices, indices, worldPos, type, 5);
            }
        }
    }

    if (vertexBuffer)
        vertexBuffer->release();
    if (indexBuffer)
        indexBuffer->release();

    if (vertices.empty())
    {
        vertexBuffer = nullptr;
        indexBuffer = nullptr;
        indexCount = 0;
    }
    else
    {
        vertexBuffer = device->newBuffer(vertices.data(), vertices.size() * sizeof(VoxelVertex), MTL::ResourceStorageModeShared);
        indexBuffer = device->newBuffer(indices.data(), indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
        indexCount = (uint32_t)indices.size();
    }
    needsRebuild = false;
}


VoxelWorld::VoxelWorld(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, MTL::Library* pShaderLibrary)
    : _pDevice(pDevice->retain()), voronoiGen(12345), currentTime(0.0f)
{
    createPipeline(pShaderLibrary, pPixelFormat, pDepthPixelFormat);
}

VoxelWorld::~VoxelWorld()
{
    for (auto& [key, chunk] : chunks)
        delete chunk;
    _pPipelineState->release();
    _pDevice->release();
}

void VoxelWorld::worldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ, int& localX, int& localZ)
{
    chunkX = worldX >> 5; // Division par 32
    chunkZ = worldZ >> 5;
    localX = worldX & 31; // Modulo 32
    localZ = worldZ & 31;
    
    if (worldX < 0 && localX != 0) {
        chunkX--;
        localX = CHUNK_SIZE + localX;
    }
    if (worldZ < 0 && localZ != 0) {
        chunkZ--;
        localZ = CHUNK_SIZE + localZ;
    }
}

Chunk* VoxelWorld::getChunk(int chunkX, int chunkZ)
{
    uint64_t key = chunkKey(chunkX, chunkZ);
    auto it = chunks.find(key);

    if (it != chunks.end())
        return it->second;

    Chunk* chunk = new Chunk(chunkX, chunkZ);
    chunks[key] = chunk;
    generateTerrainVoronoi(chunkX, chunkZ);
    return chunk;
}

void VoxelWorld::generateTerrainVoronoi(int chunkX, int chunkZ)
{
    Chunk* chunk = getChunk(chunkX, chunkZ);

    voronoiGen.generateSitesForRegion(chunkX, chunkZ, currentTime);

    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE; ++z)
        {
            int worldX = chunkX * CHUNK_SIZE + x;
            int worldZ = chunkZ * CHUNK_SIZE + z;

            for (int y = 0; y < CHUNK_HEIGHT; ++y)
            {
                BlockType type = voronoiGen.getBlockAtPosition(worldX, y, worldZ, currentTime);

                // Ajoute du vide en bas et en haut
                if (y < 5 || y > 100)
                    type = BlockType::AIR;

                chunk->setBlock(x, y, z, type);
            }
        }
    }
}

BlockType VoxelWorld::getBlock(int worldX, int worldY, int worldZ)
{
    if (worldY < 0 || worldY >= CHUNK_HEIGHT)
        return BlockType::AIR;

    int chunkX, chunkZ, localX, localZ;
    worldToChunk(worldX, worldZ, chunkX, chunkZ, localX, localZ);

    Chunk* chunk = getChunk(chunkX, chunkZ);
    return chunk->getBlock(localX, worldY, localZ);
}

void VoxelWorld::setBlock(int worldX, int worldY, int worldZ, BlockType type)
{
    if (worldY < 0 || worldY >= CHUNK_HEIGHT)
        return;

    int chunkX, chunkZ, localX, localZ;
    worldToChunk(worldX, worldZ, chunkX, chunkZ, localX, localZ);

    Chunk* chunk = getChunk(chunkX, chunkZ);
    chunk->setBlock(localX, worldY, localZ, type);
}

void VoxelWorld::removeBlock(int worldX, int worldY, int worldZ)
{
    setBlock(worldX, worldY, worldZ, BlockType::AIR);
}

void VoxelWorld::createPipeline(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat)
{
    NS::Error* error = nullptr;

    NS::SharedPtr<MTL::Function> pVertexFunction = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("voxel_vertex", NS::UTF8StringEncoding)));
    NS::SharedPtr<MTL::Function> pFragmentFunction = NS::TransferPtr(pShaderLibrary->newFunction(NS::String::string("voxel_fragment", NS::UTF8StringEncoding)));

    NS::SharedPtr<MTL::RenderPipelineDescriptor> pRenderDescriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pRenderDescriptor->setVertexFunction(pVertexFunction.get());
    pRenderDescriptor->setFragmentFunction(pFragmentFunction.get());
    pRenderDescriptor->colorAttachments()->object(0)->setPixelFormat(pPixelFormat);
    pRenderDescriptor->setDepthAttachmentPixelFormat(pDepthPixelFormat);
    
    NS::SharedPtr<MTL::VertexDescriptor> pVertexDesc = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
    pVertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(0)->setOffset(0);
    pVertexDesc->attributes()->object(0)->setBufferIndex(0);

    pVertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
    pVertexDesc->attributes()->object(1)->setOffset(sizeof(simd::float3));
    pVertexDesc->attributes()->object(1)->setBufferIndex(0);

    pVertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(2)->setOffset(sizeof(simd::float3) + sizeof(simd::float4));
    pVertexDesc->attributes()->object(2)->setBufferIndex(0);

    pVertexDesc->layouts()->object(0)->setStride(sizeof(VoxelVertex));
    pVertexDesc->layouts()->object(0)->setStepRate(1);
    pVertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    NS::SharedPtr<MTL::DepthStencilDescriptor> pDepthStencilDesc = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    pDepthStencilDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    pDepthStencilDesc->setDepthWriteEnabled(true);

    pRenderDescriptor->setVertexDescriptor(pVertexDesc.get());
    _pPipelineState = _pDevice->newRenderPipelineState(pRenderDescriptor.get(), &error);
    _pDepthStencilState = _pDevice->newDepthStencilState(pDepthStencilDesc.get());
}

void VoxelWorld::update(float dt, simd::float3 cameraPos)
{
    int camChunkX = (int)floorf(cameraPos.x / CHUNK_SIZE);
    int camChunkZ = (int)floorf(cameraPos.z / CHUNK_SIZE);

    for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; ++x)
    {
        for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; ++z)
        {
            int chunkX = camChunkX + x;
            int chunkZ = camChunkZ + z;
            Chunk* chunk = getChunk(chunkX, chunkZ);

            if (chunk->needsRebuild)
                chunk->rebuildMesh(_pDevice);
        }
    }
    for (auto it = chunks.begin(); it != chunks.end();)
    {
        Chunk* chunk = it->second;
        int dx = abs(chunk->chunkX - camChunkX);
        int dz = abs(chunk->chunkZ - camChunkZ);

        if (dx > RENDER_DISTANCE + 3 || dz > RENDER_DISTANCE + 3)
        {
            delete chunk;
            it = chunks.erase(it);
        }
        else
            ++it;
    }
}

void VoxelWorld::render(MTL::RenderCommandEncoder* pEncoder, simd::float4x4 viewProjectionMatrix)
{
    pEncoder->setRenderPipelineState(_pPipelineState);
    pEncoder->setDepthStencilState(_pDepthStencilState);
    pEncoder->setCullMode(MTL::CullModeBack);

    for (auto& [key, chunk] : chunks)
    {
        if (chunk->indexCount == 0)
            continue;

        pEncoder->setVertexBuffer(chunk->vertexBuffer, 0, 0);
        pEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, chunk->indexCount, MTL::IndexTypeUInt32, chunk->indexBuffer, 0);
    }
}

bool VoxelWorld::raycast(simd::float3 origin, simd::float3 direction, float maxDistance, simd::int3& hitBlock, simd::int3& adjacentBlock)
{
    simd::float3 pos = origin;
    simd::float3 step = simd::normalize(direction) * 0.1f;
    simd::float3 prevPos = pos;

    for (float dist = 0; dist < maxDistance; dist += 0.1f)
    {
        int x = (int)floorf(pos.x);
        int y = (int)floorf(pos.y);
        int z = (int)floorf(pos.z);

        if (getBlock(x, y, z) != BlockType::AIR)
        {
            hitBlock = {x, y, z};
            adjacentBlock = { (int)floorf(prevPos.x), (int)floorf(prevPos.y), (int)floorf(prevPos.z) };
            return true;
        }

        prevPos = pos;
        pos += step;
    }
    return false;
}
