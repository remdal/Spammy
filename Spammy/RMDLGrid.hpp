//
//  RMDLGrid.hpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#ifndef RMDLGrid_hpp
#define RMDLGrid_hpp

#include "Metal/Metal.hpp"

//#include <cmath>
//#include <algorithm>
#include <simd/simd.h>
#include <vector>
//#include <string>
//#include <fstream>
//#include <memory>

#include "RMDLMainRenderer_shared.h"

#include "RMDLMathUtils.hpp"

class BuildGrid
{
public:
    BuildGrid(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~BuildGrid();
    
    void render(MTL::RenderCommandEncoder* encoder, simd::float4x4 viewProjectionMatrix, simd::float3 cameraPosition);
    
    void setGridCenter(simd::float3 center);
    void setGridSize(int size);
    void setCellSize(float size);
    void setEdgeColor(simd::float4 color);
    void setEdgeThickness(float thickness);
    void setFadeDistance(float distance);
    void setVisible(bool visible);
    
    bool isVisible() const { return m_visible; }
    
private:
    void buildPipeline(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    void buildBuffers(MTL::Device* device);
    void generateGridMesh(MTL::Device* device);
    
    MTL::RenderPipelineState*   m_pipelineState;
    MTL::DepthStencilState*     m_depthStencilState;
    MTL::Buffer*                m_vertexBuffer;
    MTL::Buffer*                m_uniformBuffer;
    
    GridUniforms                m_uniforms;
    uint32_t                    m_vertexCount;
    bool                        m_visible;
    int                         m_gridSize;
};


namespace Fabience {

struct have
{
    int score;
    
    
    
    uint8_t item;
    void reset();
};

class NotSoClassy
{
public:
    NotSoClassy(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~NotSoClassy();

    void render(MTL::RenderCommandEncoder* renderCommandEncoder, simd::float2 screenSize, NS::UInteger width, NS::UInteger height);
    const have* update(float deltaTime);
    void drawUI(MTL::RenderCommandEncoder* renderCommandEncoder);
    
    void onMouseDown(simd::float2 screenPosition, simd::float2 screenSize);
    void onMouseUp(simd::float2 screenPosition, simd::float2 screenSize);
    void onMouseMoved(simd::float2 screenPosition, simd::float2 screenSize);
    void onMouseScroll(float deltaY);
    
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    
    void setCellSize(float size) { m_cellSize = size; }
    
    void setColorXY(simd::float4 color) { m_colorXY = color; }
    void setColorXZ(simd::float4 color) { m_colorXZ = color; }
    void setColorYZ(simd::float4 color) { m_colorYZ = color; }
    void setAllColors(simd::float4 color) { m_colorXY = m_colorXZ = m_colorYZ = color; }
    
private:
    MTL::RenderPipelineState*   m_renderPipelineState;
    MTL::DepthStencilState*     m_depthStencilState;
    MTL::Buffer*                m_vertexBuffer;
    MTL::Buffer*                m_indexBuffer;
    MTL::Buffer*                m_uniformBuffer;
    
    have                        m_have;
    bool                        m_showXY = true;
    bool                        m_showXZ = true;
    bool                        m_showYZ = true;
    bool                        m_visible = true;
    float                       m_cellSize = 1.0f;
    simd::float4                m_colorXY = {0.2f, 0.6f, 1.0f, 0.6f};
    simd::float4                m_colorXZ = {0.2f, 1.0f, 0.4f, 0.6f};
    simd::float4                m_colorYZ = {1.0f, 0.4f, 0.2f, 0.6f};
    
    void buildPipeline(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    
};

}

namespace GridCommandant {

class VehicleBuildGrid
{
public:
    VehicleBuildGrid(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~VehicleBuildGrid();
    
    void render(MTL::RenderCommandEncoder* renderCommandEncoder, const simd::float4x4& viewProjectionMatrix, const simd::float3& cameraPosition);
    
    void update(float delta);
    
    void setBlockPosition(simd::float3 pos) { m_blockPosition = pos; }
    simd::float3 blockPosition() const { return m_blockPosition; }
    
    void setBlockRotation(simd::float4x4 rot) { m_blockRotation = rot; }
    
    void setCellSize(float size) { m_cellSize = size; }
    void setGridExtent(int32_t extent) { m_gridExtent = extent; rebuildMesh(); }
    void setLineThickness(float t) { m_lineThickness = t; }
    void setFadeDistance(float d) { m_fadeDistance = d; }
    
    void setColorXY(simd::float4 c) { m_colorXY = c; }
    void setColorXZ(simd::float4 c) { m_colorXZ = c; }
    void setColorYZ(simd::float4 c) { m_colorYZ = c; }
    void setAllColors(simd::float4 c) { m_colorXY = m_colorXZ = m_colorYZ = c; }

    void setPlaneXYVisible(bool v) { m_showXY = v; rebuildMesh(); }
    void setPlaneXZVisible(bool v) { m_showXZ = v; rebuildMesh(); }
    void setPlaneYZVisible(bool v) { m_showYZ = v; rebuildMesh(); }
    
    void setVisible(bool v) { m_visible = v; }
    bool isVisible() const { return m_visible; }

    void setPulseEnabled(bool e) { m_pulseEnabled = e; }
    void setPulseSpeed(float s) { m_pulseSpeed = s; }

private:
    void buildPipeline(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    void rebuildMesh();
    void generatePlaneGrid(std::vector<GridVertex3D>& vertices,
                           std::vector<uint32_t>& indices,
                           uint8_t planeIndex, simd::float3 normal,
                           simd::float3 tangent, simd::float3 bitangent);

    MTL::Device*                m_device;
    MTL::RenderPipelineState*   m_renderPipelineState;
    MTL::DepthStencilState*     m_depthStencilState;
    MTL::Buffer*                m_vertexBuffer;
    MTL::Buffer*                m_indexBuffer;
    MTL::Buffer*                m_uniformBuffer;
    
    simd::float3   m_blockPosition  = {0.f, 0.f, 0.f};
    simd::float4x4 m_blockRotation  = matrix_identity_float4x4;
    float          m_cellSize       = 1.0f;
    int32_t        m_gridExtent     = 5; // -5 à +5 = 11 cellules par axe
    float          m_lineThickness  = 0.03f;
    float          m_fadeDistance   = 20.f;
    
    simd::float4   m_colorXY = {0.2f, 0.6f, 1.0f, 0.6f};
    simd::float4   m_colorXZ = {0.2f, 1.0f, 0.4f, 0.6f};
    simd::float4   m_colorYZ = {1.0f, 0.4f, 0.2f, 0.6f};
    
    bool m_showXY = true;
    bool m_showXZ = true;
    bool m_showYZ = true;
    bool m_visible = true;
    
    bool  m_pulseEnabled = true;
    float m_pulseSpeed   = 2.0f;
    float m_time         = 0.f;
    
    uint32_t m_indexCount = 0;
};
}






















//static constexpr int GRID_SIZE_X = 128;
//static constexpr int GRID_SIZE_Y = 128;
//static constexpr int GRID_SIZE_Z = 128;
//static constexpr float VOXEL_SIZE = 1.0f;
//
//// Voxel data structure - optimized for GPU
//struct Voxel
//{
//    uint16_t blockType;      // Type de bloc (0 = air, 1+ = différents matériaux)
//    uint8_t  flags;          // Active, visible, dirty, etc.
//    uint8_t  lightLevel;     // Niveau de lumière 0-15
//    
//    Voxel() : blockType(0), flags(0), lightLevel(0) {}
//    
//    bool isActive() const { return (flags & 0x01) != 0; }
//    void setActive(bool active)
//    {
//        if (active)
//            flags |= 0x01;
//        else
//            flags &= ~0x01;
//    }
//    
//    bool isVisible() const
//    {
//        return (flags & 0x02) != 0;
//    }
//    
//    void setVisible(bool visible)
//    {
//        if (visible)
//            flags |= 0x02;
//        else
//            flags &= ~0x02;
//    }
//};
//
//// Grid metadata for GPU
//struct GridUniforms
//{
//    simd::uint3 dimensions;      // 128, 128, 128
//    uint32_t    totalVoxels;
//    simd::float3 voxelSize;      // Taille d'un voxel en unités monde
//    float       padding;
//    simd::float3 gridOrigin;     // Position (0, 0, 0) dans le monde
//    float       padding2;
//};
//
//// Chunk pour optimisation du culling et du streaming
//struct VoxelChunk
//{
//    static constexpr int CHUNK_SIZE = 16;
//    simd::int3 chunkCoord;
//    bool dirty;
//    bool visible;
//    std::vector<Voxel> voxels;
//    MTL::Buffer* gpuBuffer;
//    
//    VoxelChunk() : chunkCoord{0,0,0}, dirty(true), visible(true), gpuBuffer(nullptr) {}
//};
//
//class RMDLGrid
//{
//public:
//    RMDLGrid(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary);
//    ~RMDLGrid();
//    
//    Voxel* getVoxel(int x, int y, int z);
//    const Voxel* getVoxel(int x, int y, int z) const;
//    void setVoxel(int x, int y, int z, const Voxel& voxel);
//    
//    // Coordonnées monde <-> grille
//    simd::int3 worldToGrid(const simd::float3& worldPos) const;
//    simd::float3 gridToWorld(const simd::int3& gridPos) const;
//    
//    // Edition mode
//    void setEditMode(bool enabled) { m_editMode = enabled; }
//    bool isEditMode() const { return m_editMode; }
//    void renderGridEdges(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection);
//    
//    // GPU sync
//    void uploadToGPU();
//    void downloadFromGPU();
//    MTL::Buffer* getGPUBuffer() const { return m_voxelBuffer; }
//    MTL::Buffer* getUniformsBuffer() const { return m_uniformsBuffer; }
//    
//    // Chunk management
//    void updateChunks(const simd::float3& cameraPos);
//    void renderChunks(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection);
//    
//    // Mesh generation from voxels (greedy meshing)
//    void generateMesh();
//    MTL::Buffer* getVertexBuffer() const { return m_meshVertexBuffer; }
//    MTL::Buffer* getIndexBuffer() const { return m_meshIndexBuffer; }
//    size_t getIndexCount() const { return m_indexCount; }
//    
//    // Sauvegarde/Chargement
//    bool saveToFile(const std::string& filepath);
//    bool loadFromFile(const std::string& filepath);
//    
//    // Raycasting pour placement de blocs
//    struct RaycastHit {
//        bool hit;
//        simd::int3 voxelPos;
//        simd::int3 adjacentPos;  // Position du voxel adjacent (pour placement)
//        simd::float3 hitPoint;
//        int faceIndex;           // 0-5 pour les 6 faces
//    };
//    RaycastHit raycast(const simd::float3& origin, const simd::float3& direction, float maxDistance) const;
//    
//    // Collision detection
//    bool checkCollision(const simd::float3& position, float radius) const;
//    simd::float3 resolveCollision(const simd::float3& position, const simd::float3& velocity, float radius) const;
//    
//    // Debug
//    size_t getActiveVoxelCount() const;
//    void clear();
//    
//private:
//    MTL::Device* m_device;
//    MTL::Library* m_library;
//    
//    // Data
//    std::vector<Voxel> m_voxels;
//    std::vector<VoxelChunk> m_chunks;
//    GridUniforms m_uniforms;
//    
//    // GPU resources
//    MTL::Buffer* m_voxelBuffer;
//    MTL::Buffer* m_uniformsBuffer;
//    MTL::Buffer* m_meshVertexBuffer;
//    MTL::Buffer* m_meshIndexBuffer;
//    size_t m_indexCount;
//    
//    // Edit mode rendering
//    bool m_editMode;
//    MTL::Buffer* m_gridEdgeVertexBuffer;
//    MTL::Buffer* m_gridEdgeIndexBuffer;
//    size_t m_gridEdgeIndexCount;
//    MTL::RenderPipelineState* m_gridEdgePipeline;
//    MTL::DepthStencilState* m_gridEdgeDepthState;
//    
//    // Voxel rendering pipeline
//    MTL::RenderPipelineState* m_voxelPipeline;
//    MTL::DepthStencilState* m_voxelDepthState;
//    
//    // Compute pipelines
//    MTL::ComputePipelineState* m_meshGenPipeline;
//    MTL::ComputePipelineState* m_cullingPipeline;
//    
//    // Helpers
//    int getIndex(int x, int y, int z) const {
//        return x + y * GRID_SIZE_X + z * GRID_SIZE_X * GRID_SIZE_Y;
//    }
//    
//    bool isValidCoord(int x, int y, int z) const {
//        return x >= 0 && x < GRID_SIZE_X &&
//               y >= 0 && y < GRID_SIZE_Y &&
//               z >= 0 && z < GRID_SIZE_Z;
//    }
//    
//    void initializeChunks();
//    void generateGridEdges();
//    void createPipelines();
//    
//    NS::SharedPtr<MTL::RenderPipelineState> m_textPipeline;
//    NS::SharedPtr<MTL::RenderPipelineState> m_shapePipeline;
//    NS::SharedPtr<MTL::DepthStencilState> m_depthState;
//
//
//    std::vector<uint16_t> m_indices;
//    NS::SharedPtr<MTL::Buffer> m_vertexBuffer;
//    NS::SharedPtr<MTL::Buffer> m_indexBuffer;
//    NS::SharedPtr<MTL::Buffer> m_frameDataBuffer;
//    size_t m_maxVertices;
//    size_t m_maxIndices;
//
//    NS::SharedPtr<MTL::SamplerState> m_sampler;
//
//    MTL::Buffer*                        m_rectangleDataBuffer;
//    MTL4::ArgumentTable*                m_argumentTable;
//    MTL::ResidencySet*                  m_residencySet;
//    MTL::Buffer*                        m_viewportSizeBuffer;
////    simd_uint2                          m_viewportSize;
//    MTL::RenderPipelineState*           m_psoUi;
//};
//
//// Vertex pour le mesh de voxels
//struct Vertex {
//    simd::float3 position;
//    simd::float3 normal;
//    simd::float2 texCoord;
//    uint32_t blockType;
//};

#endif /* RMDLGrid_hpp */

