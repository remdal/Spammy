//
//  RMDLPerlinXVoronoiMap.hpp
//  Spammy
//
//  Created by Rémy on 01/01/2026.
//

#ifndef RMDLPerlinXVoronoiMap_hpp
#define RMDLPerlinXVoronoiMap_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <vector>
#include <string>
#include <memory>

struct TerrainBlocks
{
    float height;
    float metallic;
    float roughness;
    uint32_t blocks; // 0=air, 1=terre, 2=pierre, 3=neige, 4=minerai
};

struct Generation
{
    float scale;
    int32_t octaves;
    float persistence;
    float lacunarity;
    uint32_t seed;
    int32_t voronoiCount;
};

class TerrainGenerator
{
private:
    MTL::ComputePipelineState* m_generatePipeline;
    MTL::ComputePipelineState* m_renderHeightmapPipeline;
    MTL::ComputePipelineState* m_renderMetallicPipeline;
    MTL::ComputePipelineState* m_renderRoughnessPipeline;
    
    MTL::Buffer* m_blocksBuffer;
    MTL::Buffer* m_paramsBuffer;
    
    void compileShaders(MTL::Library* library, MTL::Device* device);
    
public:
    TerrainGenerator(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary);
    ~TerrainGenerator();

    enum class RenderMode {
        Heightmap,
        Metallic,
        Roughness
    };

    void generate(MTL::CommandQueue* commandQueue, float scale = 80.0f, int octaves = 6, float persistence = 0.5f, float lacunarity = 2.0f, uint32_t seed = 89, int voronoiCount = 60, NS::UInteger width = 10, NS::UInteger height = 10);

    void renderToTexture(MTL::CommandQueue* commandQueue, MTL::Texture* texture, RenderMode mode = RenderMode::Heightmap, NS::UInteger width = 255, NS::UInteger height = 255);

    std::vector<TerrainBlocks> getBlocks(NS::UInteger width, NS::UInteger height) const;
    TerrainBlocks getBlock(NS::UInteger x, NS::UInteger y, NS::UInteger width, NS::UInteger height) const;

    std::vector<float> getHeightMap(NS::UInteger width, NS::UInteger height) const;
    std::vector<float> getMetallicMap(NS::UInteger width, NS::UInteger height) const;
    std::vector<float> getRoughnessMap(NS::UInteger width, NS::UInteger height) const;
};

#endif /* RMDLPerlinXVoronoiMap_hpp */
