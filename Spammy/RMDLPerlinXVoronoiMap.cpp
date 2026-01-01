//
//  RMDLPerlinXVoronoiMap.cpp
//  Spammy
//
//  Created by Rémy on 01/01/2026.
//

#include "RMDLPerlinXVoronoiMap.hpp"

#include <iostream>

TerrainGenerator::TerrainGenerator(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger height, MTL::Library* shaderLibrary)
{
    compileShaders(shaderLibrary, device);

    size_t blockBufferSize = width * height * sizeof(TerrainBlocks);
    m_blocksBuffer = device->newBuffer(blockBufferSize, MTL::ResourceStorageModeShared);
    m_paramsBuffer = device->newBuffer(sizeof(Generation), MTL::ResourceStorageModeShared);
}

TerrainGenerator::~TerrainGenerator()
{
    m_blocksBuffer->release();
    m_paramsBuffer->release();
    m_generatePipeline->release();
    m_renderHeightmapPipeline->release();
    m_renderMetallicPipeline->release();
    m_renderRoughnessPipeline->release();
}

void TerrainGenerator::compileShaders(MTL::Library* library, MTL::Device* device)
{
    NS::Error* error = nullptr;
    
    NS::String* generateName = NS::String::string("generateTerrain", NS::UTF8StringEncoding);
    MTL::Function* generateFunc = library->newFunction(generateName);
    m_generatePipeline = device->newComputePipelineState(generateFunc, &error);
    generateFunc->release();
    
    NS::String* heightmapName = NS::String::string("renderHeightmap", NS::UTF8StringEncoding);
    MTL::Function* heightmapFunc = library->newFunction(heightmapName);
    m_renderHeightmapPipeline = device->newComputePipelineState(heightmapFunc, &error);
    heightmapFunc->release();
    
    NS::String* metallicName = NS::String::string("renderMetallicMap", NS::UTF8StringEncoding);
    MTL::Function* metallicFunc = library->newFunction(metallicName);
    m_renderMetallicPipeline = device->newComputePipelineState(metallicFunc, &error);
    metallicFunc->release();
    
    NS::String* roughnessName = NS::String::string("renderRoughnessMap", NS::UTF8StringEncoding);
    MTL::Function* roughnessFunc = library->newFunction(roughnessName);
    m_renderRoughnessPipeline = device->newComputePipelineState(roughnessFunc, &error);
    roughnessFunc->release();
}

void TerrainGenerator::generate(MTL::CommandQueue* commandQueue, float scale, int octaves, float persistence, float lacunarity, uint32_t seed, int voronoiCount, NS::UInteger width, NS::UInteger height)
{
    Generation params = { scale, octaves, persistence, lacunarity, seed, voronoiCount };
    
    memcpy(m_paramsBuffer->contents(), &params, sizeof(Generation));
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    
    encoder->setComputePipelineState(m_generatePipeline);
    encoder->setBuffer(m_blocksBuffer, 0, 0);
    encoder->setBuffer(m_paramsBuffer, 0, 1);
    
    MTL::Size gridSize = MTL::Size(width, height, 1);
    MTL::Size threadGroupSize = MTL::Size(16, 16, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

void TerrainGenerator::renderToTexture(MTL::CommandQueue* commandQueue, MTL::Texture* texture, RenderMode mode, NS::UInteger width, NS::UInteger height)
{
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    
    MTL::ComputePipelineState* pipeline = nullptr;
    switch(mode) {
        case RenderMode::Heightmap: pipeline = m_renderHeightmapPipeline; break;
        case RenderMode::Metallic: pipeline = m_renderMetallicPipeline; break;
        case RenderMode::Roughness: pipeline = m_renderRoughnessPipeline; break;
    }
    
    encoder->setComputePipelineState(pipeline);
    encoder->setBuffer(m_blocksBuffer, 0, 0);
    encoder->setTexture(texture, 0);
    encoder->setBuffer(m_paramsBuffer, 0, 1);
    
    MTL::Size gridSize = MTL::Size(width, height, 1);
    MTL::Size threadGroupSize = MTL::Size(16, 16, 1);
    
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

std::vector<TerrainBlocks> TerrainGenerator::getBlocks(NS::UInteger width, NS::UInteger height) const
{
    std::vector<TerrainBlocks> blocks(width * height);
    memcpy(blocks.data(), m_blocksBuffer->contents(), blocks.size() * sizeof(TerrainBlocks));
    return blocks;
}

TerrainBlocks TerrainGenerator::getBlock(NS::UInteger x, NS::UInteger y, NS::UInteger width, NS::UInteger height) const
{
    TerrainBlocks* blocks = static_cast<TerrainBlocks*>(m_blocksBuffer->contents());
    return blocks[y * width + x];
}

std::vector<float> TerrainGenerator::getHeightMap(NS::UInteger width, NS::UInteger height) const
{
    auto blocks = getBlocks(width, height);
    std::vector<float> heights(blocks.size());
    for(size_t i = 0; i < blocks.size(); i++) {
        heights[i] = blocks[i].height;
    }
    return heights;
}

std::vector<float> TerrainGenerator::getMetallicMap(NS::UInteger width, NS::UInteger height) const
{
    auto blocks = getBlocks(width, height);
    std::vector<float> metallic(blocks.size());
    for(size_t i = 0; i < blocks.size(); i++) {
        metallic[i] = blocks[i].metallic;
    }
    return metallic;
}

std::vector<float> TerrainGenerator::getRoughnessMap(NS::UInteger width, NS::UInteger height) const
{
    auto blocks = getBlocks(width, height);
    std::vector<float> roughness(blocks.size());
    for(size_t i = 0; i < blocks.size(); i++) {
        roughness[i] = blocks[i].roughness;
    }
    return roughness;
}
