//
//  RMDLNeedNasa.hpp
//  Spammy
//
//  Created by Rémy on 19/01/2026.
//

#ifndef RMDLNeedNasa_hpp
#define RMDLNeedNasa_hpp

#include <simd/simd.h>
#include <Metal/Metal.hpp>
#include <vector>
#include <cmath>

namespace NASAAtTheHelm {

struct BlockVertex
{
    simd_float3 position;
    simd_float3 normal;
    simd_float2 uv;
    simd_float4 color;
};

class CommandBlockMeshGenerator
{
public:
    std::vector<BlockVertex>    vertices;
    std::vector<uint32_t>       indices;

    simd_float4 primaryColor   = {0.15f, 0.35f, 0.65f, 1.0f};   // Bleu foncé
    simd_float4 secondaryColor = {0.25f, 0.55f, 0.85f, 1.0f};   // Bleu clair
    simd_float4 accentColor    = {0.95f, 0.65f, 0.15f, 1.0f};   // Orange/doré
    simd_float4 glowColor      = {0.4f, 0.8f, 1.0f, 1.0f};      // Cyan lumineux

    simd_float4 otherColor   = {0.12f, 0.32f, 0.60f, 1.0f};
    simd_float4 paletteColor = {0.22f, 0.50f, 0.82f, 1.0f};
    simd_float4 ofColor      = {0.90f, 0.60f, 0.18f, 1.0f};
    simd_float4 hdrColor     = {0.45f, 0.85f, 1.2f,  1.0f};
    
    struct MetalBuffers
    {
        MTL::Buffer* vertexBuffer = nullptr;
        MTL::Buffer* indexBuffer = nullptr;
        uint32_t indexCount = 0;
    };
    
    MetalBuffers createMetalBuffers(MTL::Device* device)
    {
        generate();
        
        MetalBuffers buffers;
        buffers.indexCount = (uint32_t)indices.size();
        buffers.vertexBuffer = device->newBuffer(vertices.data(), vertices.size() * sizeof(BlockVertex), MTL::ResourceStorageModeShared);
        buffers.indexBuffer = device->newBuffer(indices.data(), indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
        return buffers;
    }
    
    void generate();

private:
    void generateBeveledBox(float hx, float hy, float hz, float bevel, simd_float4 color);
    void generateEdgeBevels(float bx, float by, float bz, float hx, float hy, float hz, float bevel, simd_float4 color);
    void generateRing(simd_float3 center, float outerR, float innerR, float height, int segments, simd_float4 color);
    void generateCockpit(simd_float3 center, float width, float height, simd_float4 color);
    void generateAntenna(simd_float3 base, simd_float4 color);
    void generateAttachmentConnector(simd_float3 position, simd_float3 normal);
    void generateTechLines();
    void generateVentGrilles();
    void generatePowerCore(simd_float3 center, float radius);
    void addQuad(BlockVertex v0, BlockVertex v1, BlockVertex v2, BlockVertex v3);
    void generateSphere(simd_float3 center, float radius, int detail, simd_float4 color);
};
}

#endif /* RMDLNeedNasa_hpp */
