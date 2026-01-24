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
#include "stdio.h"

#include "RMDLUtils.hpp"
#include "RMDLMathUtils.hpp"

struct VertexBlender
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
    simd::float4x4 boneMatrices[28];
};

struct BoneInfo
{
    int id;
    simd::float4x4 offset; // inverse bind pose
};

struct VectorKey
{
    float time;
    simd::float3 value;
};

struct QuatKey
{
    float time;
    simd::quatf value;
    // float3 scale
};

struct BoneAnimation
{
    std::string boneName; // Bone -> Bone.026
    std::vector<VectorKey> positions;
    std::vector<QuatKey> rotations;
    std::vector<VectorKey> scales;
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

class RMDLBlender
{
public:
    RMDLBlender(MTL::Device* pDevice, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, const std::string& resourcesPath, MTL::Library* pShaderLibrary);
    ~RMDLBlender();

    bool doTheImportThing(const std::string& resourcesPath);
    void loadGlb(const std::string& resourcesPath);
    void createPipelineBlender(MTL::Library* pShaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat);
    void updateBlender(float deltaTime);
    void drawBlender(MTL::RenderCommandEncoder* pEncoder, const simd::float4x4& viewProjectionMatrix, const simd::float4x4& model);
    
private:
    MTL::Device*                m_device;
    MTL::Buffer*                _pVertexBufferBlender;
    MTL::Buffer*                _pIndexBufferBlender;
    MTL::Buffer*                _pUniformBufferBlender;
    MTL::Texture*               _pDiffuseTexture = nullptr;
    MTL::Texture*               _pNormalTexture = nullptr;
    MTL::Texture*               _pRoughnessTexture = nullptr;
    MTL::Texture*               _pMetallicTexture = nullptr;
    MTL::SamplerState*          _pSampler = nullptr;
    MTL::DepthStencilState*     _pDepthState;
    MTL::RenderPipelineState*   _pPipelineStateBlender;
    std::vector<VertexBlender>  _pVertices;
    std::vector<uint32_t>       _pIndices;
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
    
    
    NodeData _rootNode;

    void loadMesh(const aiScene* scene);
    void loadTextures(const std::string& resourcesPath, const aiScene* scene);
    MTL::Texture* loadTexture(const std::string& resourcesPath, const char* path, const aiScene* scene, bool sRGB);
    MTL::Texture* loadEmbeddedTexture(aiTexture* aiTexture, bool sRGB);
    void loadAnimation(const aiScene* scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);
    void processNode(aiNode* node, const aiScene* scene);
    void loadBones(aiMesh* mesh);
    void createBuffers();
    void createSampler();
    
    NodeData copyNodeHierarchy(aiNode* node);
    void computeBoneTransforms(float time, const NodeData& node, const simd::float4x4& parentTf);
    simd::float3 interpolatePosition(float time, const BoneAnimation& anim);
    simd::quatf interpolateRotation(float time, const BoneAnimation& anim);
    simd::float3 interpolateScale(float time, const BoneAnimation& anim);
};

#endif /* RMDLBLENDER_HPP */
