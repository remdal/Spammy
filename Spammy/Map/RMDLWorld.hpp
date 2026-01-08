//
//  RMDLWorld.hpp
//  Spammy
//
//  Created by Rémy on 05/01/2026.
//

#ifndef RMDLWorld_hpp
#define RMDLWorld_hpp

#include <Metal/Metal.hpp>

#include <cmath>
#include <random>
#include <memory>
#include <simd/simd.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include "VoronoiVoxel4D.hpp"

enum class CellShape : uint8_t {
    EMPTY = 0,
    CUBE,
    ICOSPHERE,
    TRIANGULAR_PRISM
};

struct GridCell {
    CellShape shape;
    simd::float4 color;
    float rotation;
    uint8_t variant;
};

struct GeometricVertex {
    simd::float3 position;
    simd::float4 color;
    simd::float3 normal;
    simd::float2 uv;
};

class GeometricGrid {
public:
    GeometricGrid(int width, int height, int depth);
    ~GeometricGrid();
    
    void setCell(int x, int y, int z, CellShape shape, simd::float4 color = simd::make_float4(1,1,1,1));
    GridCell getCell(int x, int y, int z) const;
    bool isEmpty(int x, int y, int z) const;
    
    void generatePerlinTerrain(float scale, float threshold);
    void generateVoronoiStructure(int numSeeds);
    void generateWave(float amplitude, float frequency);
    void generateSphere(simd::float3 center, float radius, CellShape shape);
    
    void buildMesh(MTL::Device* device);
    void render(MTL::RenderCommandEncoder* encoder);
    
    void clear();
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getDepth() const { return depth; }
    
private:
    int width, height, depth;
    std::vector<GridCell> cells;
    
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    uint32_t indexCount;
    bool needsRebuild;
    
    int index(int x, int y, int z) const;
    bool isValidCoord(int x, int y, int z) const;
    
    void addCube(int x, int y, int z, const GridCell& cell,
                 std::vector<GeometricVertex>& vertices,
                 std::vector<uint32_t>& indices);
    
    void addIcosphere(int x, int y, int z, const GridCell& cell,
                      std::vector<GeometricVertex>& vertices,
                      std::vector<uint32_t>& indices);
    
    void addPrism(int x, int y, int z, const GridCell& cell,
                  std::vector<GeometricVertex>& vertices,
                  std::vector<uint32_t>& indices);
    
    simd::float3 gridToWorld(int x, int y, int z) const;
    
    float perlinNoise3D(float x, float y, float z);
    simd::float3 voronoiCell(simd::float3 p, int seed);
};

class ShapeLibrary {
public:
    static void generateCubeFaces(
        simd::float3 center,
        float size,
        simd::float4 color,
        const bool faces[6],
        std::vector<GeometricVertex>& vertices,
        std::vector<uint32_t>& indices);
    
    static void generateIcosphere(
        simd::float3 center,
        float radius,
        simd::float4 color,
        int subdivisions,
        std::vector<GeometricVertex>& vertices,
        std::vector<uint32_t>& indices);
    
    static void generateTriangularPrism(
        simd::float3 center,
        float size,
        float rotation,
        simd::float4 color,
        std::vector<GeometricVertex>& vertices,
        std::vector<uint32_t>& indices);
};

class ModularWorld {
public:
    ModularWorld(MTL::Device* device,
                 MTL::PixelFormat pixelFormat,
                 MTL::PixelFormat depthPixelFormat,
                 MTL::Library* library);
    ~ModularWorld();
    
    void update(float dt, simd::float3 cameraPos);
    void render(MTL::RenderCommandEncoder* encoder, simd::float4x4 viewProjectionMatrix);
    
    GeometricGrid* getGrid() { return grid.get(); }
    
    void regenerateWorld();
    void setWorldSize(int width, int height, int depth);
    
    void editCell(simd::float3 worldPos, CellShape shape, simd::float4 color);
    void removeCell(simd::float3 worldPos);
    
private:
    MTL::Device* device;
    MTL::RenderPipelineState* pipelineState;
    MTL::DepthStencilState* depthStencilState;
    
    std::unique_ptr<GeometricGrid> grid;
    
    void createPipeline(MTL::Library* library,
                       MTL::PixelFormat pixelFormat,
                       MTL::PixelFormat depthPixelFormat);
};

#endif /* RMDLWorld_hpp */
