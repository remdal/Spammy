//
//  RMDLGrid.hpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#ifndef RMDLGrid_hpp
#define RMDLGrid_hpp

#include "Metal/Metal.hpp"

#include <cmath>
#include <algorithm>
#include <simd/simd.h>
#include <vector>
#include <string>
#include <fstream>
#include <memory>

static constexpr int GRID_SIZE_X = 128;
static constexpr int GRID_SIZE_Y = 128;
static constexpr int GRID_SIZE_Z = 128;
static constexpr float VOXEL_SIZE = 1.0f;

// Voxel data structure - optimized for GPU
struct Voxel
{
    uint16_t blockType;      // Type de bloc (0 = air, 1+ = différents matériaux)
    uint8_t  flags;          // Active, visible, dirty, etc.
    uint8_t  lightLevel;     // Niveau de lumière 0-15
    
    Voxel() : blockType(0), flags(0), lightLevel(0) {}
    
    bool isActive() const { return (flags & 0x01) != 0; }
    void setActive(bool active)
    {
        if (active)
            flags |= 0x01;
        else
            flags &= ~0x01;
    }
    
    bool isVisible() const
    {
        return (flags & 0x02) != 0;
    }
    
    void setVisible(bool visible)
    {
        if (visible)
            flags |= 0x02;
        else
            flags &= ~0x02;
    }
};

// Grid metadata for GPU
struct GridUniforms
{
    simd::uint3 dimensions;      // 128, 128, 128
    uint32_t    totalVoxels;
    simd::float3 voxelSize;      // Taille d'un voxel en unités monde
    float       padding;
    simd::float3 gridOrigin;     // Position (0, 0, 0) dans le monde
    float       padding2;
};

// Chunk pour optimisation du culling et du streaming
struct VoxelChunk
{
    static constexpr int CHUNK_SIZE = 16;
    simd::int3 chunkCoord;
    bool dirty;
    bool visible;
    std::vector<Voxel> voxels;
    MTL::Buffer* gpuBuffer;
    
    VoxelChunk() : chunkCoord{0,0,0}, dirty(true), visible(true), gpuBuffer(nullptr) {}
};

class RMDLGrid
{
public:
    RMDLGrid(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary);
    ~RMDLGrid();
    
    Voxel* getVoxel(int x, int y, int z);
    const Voxel* getVoxel(int x, int y, int z) const;
    void setVoxel(int x, int y, int z, const Voxel& voxel);
    
    // Coordonnées monde <-> grille
    simd::int3 worldToGrid(const simd::float3& worldPos) const;
    simd::float3 gridToWorld(const simd::int3& gridPos) const;
    
    // Edition mode
    void setEditMode(bool enabled) { m_editMode = enabled; }
    bool isEditMode() const { return m_editMode; }
    void renderGridEdges(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection);
    
    // GPU sync
    void uploadToGPU();
    void downloadFromGPU();
    MTL::Buffer* getGPUBuffer() const { return m_voxelBuffer; }
    MTL::Buffer* getUniformsBuffer() const { return m_uniformsBuffer; }
    
    // Chunk management
    void updateChunks(const simd::float3& cameraPos);
    void renderChunks(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProjection);
    
    // Mesh generation from voxels (greedy meshing)
    void generateMesh();
    MTL::Buffer* getVertexBuffer() const { return m_meshVertexBuffer; }
    MTL::Buffer* getIndexBuffer() const { return m_meshIndexBuffer; }
    size_t getIndexCount() const { return m_indexCount; }
    
    // Sauvegarde/Chargement
    bool saveToFile(const std::string& filepath);
    bool loadFromFile(const std::string& filepath);
    
    // Raycasting pour placement de blocs
    struct RaycastHit {
        bool hit;
        simd::int3 voxelPos;
        simd::int3 adjacentPos;  // Position du voxel adjacent (pour placement)
        simd::float3 hitPoint;
        int faceIndex;           // 0-5 pour les 6 faces
    };
    RaycastHit raycast(const simd::float3& origin, const simd::float3& direction, float maxDistance) const;
    
    // Collision detection
    bool checkCollision(const simd::float3& position, float radius) const;
    simd::float3 resolveCollision(const simd::float3& position, const simd::float3& velocity, float radius) const;
    
    // Debug
    size_t getActiveVoxelCount() const;
    void clear();
    
private:
    MTL::Device* m_device;
    MTL::Library* m_library;
    
    // Data
    std::vector<Voxel> m_voxels;
    std::vector<VoxelChunk> m_chunks;
    GridUniforms m_uniforms;
    
    // GPU resources
    MTL::Buffer* m_voxelBuffer;
    MTL::Buffer* m_uniformsBuffer;
    MTL::Buffer* m_meshVertexBuffer;
    MTL::Buffer* m_meshIndexBuffer;
    size_t m_indexCount;
    
    // Edit mode rendering
    bool m_editMode;
    MTL::Buffer* m_gridEdgeVertexBuffer;
    MTL::Buffer* m_gridEdgeIndexBuffer;
    size_t m_gridEdgeIndexCount;
    MTL::RenderPipelineState* m_gridEdgePipeline;
    MTL::DepthStencilState* m_gridEdgeDepthState;
    
    // Voxel rendering pipeline
    MTL::RenderPipelineState* m_voxelPipeline;
    MTL::DepthStencilState* m_voxelDepthState;
    
    // Compute pipelines
    MTL::ComputePipelineState* m_meshGenPipeline;
    MTL::ComputePipelineState* m_cullingPipeline;
    
    // Helpers
    int getIndex(int x, int y, int z) const {
        return x + y * GRID_SIZE_X + z * GRID_SIZE_X * GRID_SIZE_Y;
    }
    
    bool isValidCoord(int x, int y, int z) const {
        return x >= 0 && x < GRID_SIZE_X &&
               y >= 0 && y < GRID_SIZE_Y &&
               z >= 0 && z < GRID_SIZE_Z;
    }
    
    void initializeChunks();
    void generateGridEdges();
    void createPipelines();
    
    NS::SharedPtr<MTL::RenderPipelineState> m_textPipeline;
    NS::SharedPtr<MTL::RenderPipelineState> m_shapePipeline;
    NS::SharedPtr<MTL::DepthStencilState> m_depthState;


    std::vector<uint16_t> m_indices;
    NS::SharedPtr<MTL::Buffer> m_vertexBuffer;
    NS::SharedPtr<MTL::Buffer> m_indexBuffer;
    NS::SharedPtr<MTL::Buffer> m_frameDataBuffer;
    size_t m_maxVertices;
    size_t m_maxIndices;

    NS::SharedPtr<MTL::SamplerState> m_sampler;

    MTL::Buffer*                        m_rectangleDataBuffer;
    MTL4::ArgumentTable*                m_argumentTable;
    MTL::ResidencySet*                  m_residencySet;
    MTL::Buffer*                        m_viewportSizeBuffer;
//    simd_uint2                          m_viewportSize;
    MTL::RenderPipelineState*           m_psoUi;
};

// Vertex pour le mesh de voxels
struct Vertex {
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texCoord;
    uint32_t blockType;
};

#endif /* RMDLGrid_hpp */

