//
//  RMDLPNJ.hpp
//  Spammy
//
//  Created by Rémy on 02/01/2026.
//

#ifndef RMDLPNJ_hpp
#define RMDLPNJ_hpp

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>

#include "RMDLMainRenderer_shared.h"
#include "RMDLMathUtils.hpp"

struct InstanceData
{
    simd::float4x4 modelMatrix;
    simd::float4x4 normalMatrix;
};

struct MaterialUniforms
{
    simd::float3 albedo;
    float metallic;
    float roughness;
    float ao;
    simd::float3 emissive;
    float emissiveStrength;
};

struct LightUniforms
{
    simd::float3 position;
    simd::float3 color;
    float intensity;
    simd::float3 ambientColor;
    float ambientIntensity;
};

struct BoneTransform
{
    simd::float4x4 transform;
};

enum class AnimationState {
    IDLE,
    WALKING
//    RUNNING,
//    JUMPING,
//    ATTACKING,
//    ROLLING,
//    VICTORY
};

struct Bone
{
    std::string name;
    int id;
    int parentId;
    simd::float4x4 bindPose;
    simd::float4x4 currentTransform;
    simd::float3 position;
    simd::quatf rotation;
    simd::float3 scale;
};

struct KeyFrame
{
    float time;
    simd::float3 position;
    simd::quatf rotation;
    simd::float3 scale;
};

class Animation
{
public:
    Animation(const std::string& name, float duration, bool loop = true);
    
    void addKeyFrame(int boneId, const KeyFrame& keyframe);
    
    void update(float deltaTime);
    
    simd::float4x4 getBoneTransform(int boneId) const;
    
    void reset() { m_currentTime = 0.0f; }
    bool isFinished() const { return !m_loop && m_currentTime >= m_duration; }
    std::string getName() const { return m_name; }

private:
    std::string m_name;
    float       m_duration;
    float       m_currentTime;
    bool        m_loop;
    std::map<int, std::vector<KeyFrame>> m_boneKeyFrames;
};

class MetalPlayer
{
public:
    MetalPlayer(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~MetalPlayer();
    
    void initializeMetal(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device);
    void initializeSkeleton(MTL::Device* device);
    void initializeAnimations();
    void createGeometry();
    void loadTextures();
    void update(float deltaTime);
    void updateBoneTransforms();
    void render(MTL::RenderCommandEncoder* renderEncoder);
    void moveForward();
    void moveBackward();
    void strafeLeft();
    void strafeRight();
    void run();
    void stopMoving();
    void jump();
    void attack();
    void printStatus() const;
    
    simd::float3 getPosition() const { return position; }
    int getHealth() const { return health; }
    AnimationState getState() const { return currentState; }

    void setMaterialColor(const simd::float3& color);
    void setMetallic(float value);
    void setRoughness(float value);
    void setEmissive(const simd::float3& color, float strength);

private:
    MTL::RenderPipelineState*   m_renderPipelineState;
    MTL::DepthStencilState*     m_depthStencilState;

    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    MTL::Buffer* instanceBuffer;
    MTL::Buffer* cameraUniformsBuffer;
    MTL::Buffer* materialUniformsBuffer;
    MTL::Buffer* lightUniformsBuffer;
    MTL::Buffer*                m_boneTransformBuffer;

    MTL::Texture* albedoTexture;
    MTL::Texture* normalTexture;
    MTL::Texture* metallicTexture;
    MTL::Texture* roughnessTexture;
    MTL::Texture* aoTexture;
    MTL::Texture* emissiveTexture;

    MTL::SamplerState* samplerState;

    std::vector<Bone> skeleton;
    std::map<AnimationState, Animation> animations;
    AnimationState currentState;
    
    simd::float3 position;
    simd::float3 velocity;
    simd::quatf rotation;
    float speed;
    int health;
    bool isGrounded;
    
private:
    std::string getStateName(AnimationState state) const;
};

#endif /* RMDLPNJ_hpp */
