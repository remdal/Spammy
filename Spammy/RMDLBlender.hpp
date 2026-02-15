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
#include <queue>

#include "stdio.h"

#include "RMDLUtils.hpp"
#include "RMDLMathUtils.hpp"

#include "RMDLMainRenderer_shared.h"

enum class AnimationState
{
    Idle,
    Chenille,
    Walk,
    Run,
    Jump,
    Automate,
    Custom
};

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

struct AnimationLayer
{
    size_t animationIndex = 0;
    float weight = 1.0f;
    float currentTime = 0.0f;
    bool isPlaying = false;
    bool loop = true;
    float speedMultiplier = 1.0f;
};

struct AnimationController
{
    bool isPlaying = false;
    float speedMultiplier = 1.0f;
    bool loop = true;
    bool isTransitioning = false;
    size_t targetAnimation = 0;
    float transitionDuration = 0.3f;
    float transitionTime = 0.0f;
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
    std::string parent;
    
    std::vector<VertexBlender> vertices;
    std::vector<VertexBlenderFull> verticesFull;
    std::vector<uint32_t> indices;
    MTL::Buffer* vertexBuffer = nullptr;
    MTL::Buffer* indexBuffer = nullptr;
    
    bool hasAnimation = false;
    bool shouldAnimate = true;
    std::unordered_map<std::string, BoneInfo> boneMap;
    std::vector<simd::float4x4> boneMatrices;
    std::vector<Animation> animations;
    std::unordered_map<std::string, size_t> animationMap;
    AnimationController animController;
    std::vector<AnimationLayer> animationLayers;
    bool useLayeredAnimation = false;
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
    simd::float3 position = {};
    
    bool hasAnimations(const std::string& name) const
    {
        return animationMap.find(name) != animationMap.end();
    }
    
    size_t getAnimationIndex(const std::string& name) const
    {
        auto it = animationMap.find(name);
        return it != animationMap.end() ? it->second : 0;
    }
    
    size_t getVertexCount() const
    {
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
    void printAnimations(size_t modelIndex) const;
    void debugBones(size_t modelIndex) const;

    Blender* getModel(size_t index);
    Blender* getModel(const std::string& name);
    size_t getModelCount() const { return m_models.size(); }
    
    void addAnimationLayer(size_t modelIndex, const std::string& animName, float weight);
    void clearAnimationLayers(size_t modelIndex);
    
    void playAnimation(size_t modelIndex, const std::string& animName, bool loop = true);
    void playAnimation(const std::string& modelName, const std::string& animName, bool loop = true);
    void transitionToAnimation(size_t modelIndex, const std::string& animName, float duration = 0.3f);
    void stopAnimation(size_t modelIndex);
    void setAnimationSpeed(size_t modelIndex, float speed);
        
    void createPipelineBlender(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat);
    void updateBlender(float deltaTime);
    void draw(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProj, const RMDLUniforms &uniforms);
    void drawBlender(MTL::RenderCommandEncoder* pEncoder, size_t index, const simd::float4x4& viewProjectionMatrix, const simd::float4x4& model, const RMDLUniforms &uniforms);
    
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

class AnimationStateMachine
{
public:
    void addState(const std::string& name, const std::string& animName)
    {
        states[name] = animName;
    }
    
    void addTransition(const std::string& from, const std::string& to,
                      std::function<bool()> condition)
    {
        transitions[from].push_back({to, condition});
    }
    
    void update(RMDLBlender& blender, size_t modelIndex)
    {
        for (auto& trans : transitions[currentState])
        {
            if (trans.condition())
            {
                setState(blender, modelIndex, trans.targetState);
                break;
            }
        }
    }
    
    void setState(RMDLBlender& blender, size_t modelIndex, const std::string& state)
    {
        if (states.find(state) == states.end() || state == currentState)
            return;
        
        currentState = state;
        blender.playAnimation(modelIndex, states[state]);
    }
    
    std::string getCurrentState() const { return currentState; }
    
private:
    struct Transition
    {
        std::string targetState;
        std::function<bool()> condition;
    };
    
    std::unordered_map<std::string, std::string> states;
    std::unordered_map<std::string, std::vector<Transition>> transitions;
    std::string currentState;
};

#endif /* RMDLBLENDER_HPP */
