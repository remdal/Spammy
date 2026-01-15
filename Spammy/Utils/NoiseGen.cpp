//
//  NoiseGen.cpp
//  Spammy
//
//  Created by Rémy on 14/01/2026.
//

#include "NoiseGen.hpp"

NoiseGenerator::NoiseGenerator(MTL::Device* device, uint64_t seed)
    : _device(device)
    , _seed(seed)
    , _rng(seed)
    , _heightmapPipeline(nullptr)
    , _normalMapPipeline(nullptr)
    , _biomePipeline(nullptr)
    , _permutationBuffer(nullptr)
    , _gradientBuffer(nullptr)
    , _noiseParamsBuffer(nullptr)
{
    generatePermutationTable();
    generateGradients();
    createPipelines();
}

NoiseGenerator::~NoiseGenerator() {
    if (_heightmapPipeline) _heightmapPipeline->release();
    if (_normalMapPipeline) _normalMapPipeline->release();
    if (_biomePipeline) _biomePipeline->release();
    if (_permutationBuffer) _permutationBuffer->release();
    if (_gradientBuffer) _gradientBuffer->release();
    if (_noiseParamsBuffer) _noiseParamsBuffer->release();
}

void NoiseGenerator::setSeed(uint64_t seed) {
    _seed = seed;
    _rng.seed(seed);
    generatePermutationTable();
    generateGradients();
}

void NoiseGenerator::generatePermutationTable() {
    std::vector<uint32_t> perm(512);
    for (int i = 0; i < 256; i++) perm[i] = i;
    
    std::shuffle(perm.begin(), perm.begin() + 256, _rng);
    
    for (int i = 0; i < 256; i++) perm[256 + i] = perm[i];
    
    if (_permutationBuffer) _permutationBuffer->release();
    _permutationBuffer = _device->newBuffer(perm.data(), perm.size() * sizeof(uint32_t),
                                            MTL::ResourceStorageModeShared);
}

void NoiseGenerator::generateGradients() {
    std::vector<simd::float4> gradients(256);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    for (int i = 0; i < 256; i++) {
        simd::float3 g = simd::normalize(simd::make_float3(dist(_rng), dist(_rng), dist(_rng)));
        gradients[i] = simd::make_float4(g.x, g.y, g.z, 0.0f);
    }
    
    if (_gradientBuffer) _gradientBuffer->release();
    _gradientBuffer = _device->newBuffer(gradients.data(), gradients.size() * sizeof(simd::float4),
                                         MTL::ResourceStorageModeShared);
}

void NoiseGenerator::createPipelines() {
    // Les pipelines seront créés avec la library compilée
    // Voir ShaderLibrary pour l'initialisation
}

void NoiseGenerator::generateHeightmap(MTL::CommandBuffer* cmd,
                                       MTL::Texture* output,
                                       simd::float2 worldOffset,
                                       float scale,
                                       uint32_t biomeId) {
    if (!_heightmapPipeline) return;
    
    struct NoiseParams {
        simd::float2 worldOffset;
        float scale;
        uint32_t biomeId;
        uint64_t seed;
    } params = { worldOffset, scale, biomeId, _seed };
    
    MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
    encoder->setComputePipelineState(_heightmapPipeline);
    encoder->setTexture(output, 0);
    encoder->setBuffer(_permutationBuffer, 0, 0);
    encoder->setBuffer(_gradientBuffer, 0, 1);
    encoder->setBytes(&params, sizeof(params), 2);
    
    MTL::Size threadgroups(output->width() / 16, output->height() / 16, 1);
    MTL::Size threadsPerGroup(16, 16, 1);
    encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
    encoder->endEncoding();
}

void NoiseGenerator::generateNormalMap(MTL::CommandBuffer* cmd,
                                       MTL::Texture* heightmap,
                                       MTL::Texture* output,
                                       float heightScale) {
    if (!_normalMapPipeline) return;
    
    MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
    encoder->setComputePipelineState(_normalMapPipeline);
    encoder->setTexture(heightmap, 0);
    encoder->setTexture(output, 1);
    encoder->setBytes(&heightScale, sizeof(float), 0);
    
    MTL::Size threadgroups(output->width() / 16, output->height() / 16, 1);
    MTL::Size threadsPerGroup(16, 16, 1);
    encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
    encoder->endEncoding();
}

void NoiseGenerator::generateBiomeMap(MTL::CommandBuffer* cmd,
                                      MTL::Texture* output,
                                      simd::float2 worldOffset,
                                      float scale) {
    if (!_biomePipeline) return;
    
    struct BiomeParams {
        simd::float2 worldOffset;
        float scale;
        uint64_t seed;
    } params = { worldOffset, scale, _seed };
    
    MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
    encoder->setComputePipelineState(_biomePipeline);
    encoder->setTexture(output, 0);
    encoder->setBuffer(_permutationBuffer, 0, 0);
    encoder->setBytes(&params, sizeof(params), 1);
    
    MTL::Size threadgroups(output->width() / 16, output->height() / 16, 1);
    MTL::Size threadsPerGroup(16, 16, 1);
    encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
    encoder->endEncoding();
}
