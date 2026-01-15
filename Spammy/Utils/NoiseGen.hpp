//
//  NoiseGen.hpp
//  Spammy
//
//  Created by Rémy on 14/01/2026.
//

#ifndef NoiseGen_hpp
#define NoiseGen_hpp

#include <Metal/Metal.hpp>

#include <simd/simd.h>
#include <random>
#include <vector>

class NoiseGenerator
{
public:
    NoiseGenerator(MTL::Device* device, uint64_t seed);
    ~NoiseGenerator();
    
    void generateHeightmap(MTL::CommandBuffer* cmd,
                          MTL::Texture* output,
                          simd::float2 worldOffset,
                          float scale,
                          uint32_t biomeId);
    
    void generateNormalMap(MTL::CommandBuffer* cmd,
                          MTL::Texture* heightmap,
                          MTL::Texture* output,
                          float heightScale);
    
    void generateBiomeMap(MTL::CommandBuffer* cmd,
                         MTL::Texture* output,
                         simd::float2 worldOffset,
                         float scale);
    
    MTL::Buffer* getPermutationBuffer() const { return _permutationBuffer; }
    MTL::Buffer* getGradientBuffer() const { return _gradientBuffer; }
    
    uint64_t getSeed() const { return _seed; }
    void setSeed(uint64_t seed);

private:
    void createPipelines();
    void generatePermutationTable();
    void generateGradients();
    
    MTL::Device* _device;
    uint64_t _seed;
    
    MTL::ComputePipelineState* _heightmapPipeline;
    MTL::ComputePipelineState* _normalMapPipeline;
    MTL::ComputePipelineState* _biomePipeline;
    
    MTL::Buffer* _permutationBuffer;
    MTL::Buffer* _gradientBuffer;
    MTL::Buffer* _noiseParamsBuffer;
    
    std::mt19937_64 _rng;
};

#endif /* NoiseGen_hpp */
