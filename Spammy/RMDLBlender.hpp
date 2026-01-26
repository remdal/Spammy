//
//  RMDLBlender.hpp
//  Spammy
//
//  Created by Rémy on 18/12/2025.
//

#ifndef RMDLBLENDER_HPP
#define RMDLBLENDER_HPP

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include <MetalKit/MetalKit.hpp>

#include <simd/simd.h>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include "stdio.h"

#include "RMDLUtils.hpp"
#include "RMDLMathUtils.hpp"

struct VertexBlender
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texCoord;
};

struct VertexBlenderFull
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texCoord;
    simd::int4   joints;
    simd::float4 weights;
};

struct BlenderUniforms
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjectionMatrix;
};

struct BlenderUniformsFull
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjectionMatrix;
    simd::float4x4 boneMatrices[28];
};

struct BoneInfo
{
    int id;
    simd::float4x4 offset; // inverse bind pose
};

template<typename T>
struct KeyFrame
{
    float time;
    T value;
};

struct BoneAnimation
{
    std::string boneName; // Bone -> Bone.026
    std::vector<KeyFrame<simd::float3>> positions;
    std::vector<KeyFrame<simd::quatf>>  rotations;
    std::vector<KeyFrame<simd::float3>> scales;
};

struct Animation
{
    std::string name;
    float duration;
    float ticksPerSec;
    std::vector<BoneAnimation> channels;
};

struct NodeData
{
    std::string name;
    simd::float4x4 transform;
    std::vector<NodeData> children;
};

struct Blender
{
    std::string name;
    
    std::vector<VertexBlender> vertices;
    std::vector<VertexBlenderFull> verticesFull;
    std::vector<uint32_t> indices;
    MTL::Buffer* vertexBuffer = nullptr;
    MTL::Buffer* indexBuffer = nullptr;
    
    bool hasAnimation = false;
    std::unordered_map<std::string, BoneInfo> boneMap;
    std::vector<simd::float4x4> boneMatrices;
    std::vector<Animation> animations;
    NodeData rootNode;
    int boneCount = 0;
    size_t currentAnimation = 0;
    float currentTime = 0.0f;
    MTL::Buffer* uniformBuffer = nullptr;
    
    MTL::Texture* diffuseTexture = nullptr;
    MTL::Texture* normalTexture = nullptr;
    MTL::Texture* roughnessTexture = nullptr;
    MTL::Texture* metallicTexture = nullptr;
    
    simd::float4x4 transform = matrix_identity_float4x4;
//    simd::float3 position = {};
    
    size_t getVertexCount() const {
        return hasAnimation ? verticesFull.size() : vertices.size();
    }
    
    void release()
    {
        if (vertexBuffer) vertexBuffer->release();
        if (indexBuffer) indexBuffer->release();
        if (uniformBuffer) uniformBuffer->release();
        if (diffuseTexture) diffuseTexture->release();
        if (normalTexture) normalTexture->release();
        if (roughnessTexture) roughnessTexture->release();
        if (metallicTexture) metallicTexture->release();
    }
};

class RMDLBlender
{
public:
    RMDLBlender(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, const std::string& resourcesPath, MTL::Library* shaderLibrary);
    ~RMDLBlender();

    bool doTheImportThing(const std::string& resourcesPath);
    size_t loadModel(const std::string& resourcesPath, const std::string& name = "");
    void printMemoryStats() const;

    Blender* getModel(size_t index);
    Blender* getModel(const std::string& name);
    size_t getModelCount() const { return m_models.size(); }
        
    void createPipelineBlender(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat);
    void updateBlender(float deltaTime);
    void draw(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProj);
    void drawBlender(MTL::RenderCommandEncoder* pEncoder, size_t index, const simd::float4x4& viewProjectionMatrix, const simd::float4x4& model);
    
private:
    MTL::Device*                m_device;
    MTL::SamplerState*          _pSampler = nullptr;
    MTL::DepthStencilState*     _pDepthState;
    MTL::RenderPipelineState*   _pPipelineStateBlender;
    MTL::RenderPipelineState*   _pPipelineStateBlenderFull;
    float                       _pCurrentTime;
    float                       _pAnimationDuration;
    
    simd::float4x4              boneMatrix;
    float                       m_currentTime = 0.f;
    int                         m_frame;
    int                         m_currentAnimation = 0;
    
    std::vector<Animation>      mv_animations;
    int                         _boneCount = 0;
    std::map<std::string, BoneInfo> m_boneMap;
    std::vector<uint32_t>           m_indices;
    std::vector<simd::float4x4>     m_boneMatrices;
    std::vector<VertexBlender>    m_vertices;
    

    
    std::vector<Blender> m_models;
    
    
    NodeData _rootNode;

    bool modelHasBones(aiNode* node, const aiScene* scene);
    void loadMesh(const aiScene* scene);
    MTL::Texture* loadTexture(const std::string& resourcesPath, const char* path, const aiScene* scene, bool sRGB);
    MTL::Texture* loadEmbeddedTexture(aiTexture* aiTexture, bool sRGB);

    void createSampler();
    
    void processNodeStatic(aiNode* node, const aiScene* scene, Blender& model);
    void processNodeSkinned(aiNode* node, const aiScene* scene, Blender& model);
    void processMeshStatic(aiMesh* mesh, Blender& model);
    void processMeshSkinned(aiMesh* mesh, Blender& model);
    void loadBones(aiMesh* mesh, Blender& model, uint32_t baseVertex);
    void loadAnimations(const aiScene* scene, Blender& model);
    void loadTextures(const aiScene* scene, Blender& model, const std::string& resourcesPath);
    
    NodeData copyNodeHierarchy(aiNode* node);
    void computeBoneTransforms(float time, const NodeData& node, const simd::float4x4& parentTf, Blender& model);
    simd::float3 interpolatePosition(float time, const BoneAnimation& anim);
    simd::quatf interpolateRotation(float time, const BoneAnimation& anim);
    simd::float3 interpolateScale(float time, const BoneAnimation& anim);
};

#endif /* RMDLBLENDER_HPP */
