//
//  RMDLPNJ.cpp
//  Spammy
//
//  Created by Rémy on 02/01/2026.
//

#include "RMDLPNJ.hpp"

Animation::Animation(const std::string& name, float duration, bool loop)
: m_currentTime(0.0f)
{
    
}

//Animation::~Animation()
//{
//}

void Animation::addKeyFrame(int boneId, const KeyFrame &keyframe)
{
    m_boneKeyFrames[boneId].push_back(keyframe);
}

void Animation::update(float deltaTime)
{
    m_currentTime += deltaTime;
    if (m_loop && m_currentTime > m_duration)
        m_currentTime = fmod(m_currentTime, m_duration);
}

simd::float4x4 Animation::getBoneTransform(int boneId) const
{
    auto it = m_boneKeyFrames.find(boneId);
    if (it == m_boneKeyFrames.end() || it->second.empty())
        return simd::float4x4(1.0f);

    const KeyFrame& kf = it->second[0];
    
//    float3 objectPosition = { 0.f, 0.f, -10.f };
//    float4x4 rt = math::makeTranslate( objectPosition );

    simd::float4x4 translation = simd::float4x4(1.0f);
    translation.columns[3] = simd::float4{kf.position.x, kf.position.y, kf.position.z, 1.0f};
    
    simd::float4x4 rotation = simd::float4x4(kf.rotation);

//    float4x4 rr1 = math::makeYRotate( -_angle );
//    float4x4 rr0 = math::makeXRotate( _angle * 0.5 );

    simd::float4x4 scale = simd::float4x4(1.0f);
    scale.columns[0][0] = kf.scale.x;
    scale.columns[1][1] = kf.scale.y;
    scale.columns[2][2] = kf.scale.z;
    
    return translation * rotation * scale;
}

MetalPlayer::MetalPlayer(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
: position(simd::make_float3(0, 0, 0)), velocity(simd::make_float3(0, 0, 0)), rotation(simd::quatf(0, 0, 0, 1)),
speed(5.0f), health(100), isGrounded(true), currentState(AnimationState::IDLE)
{
//    initializeMetal();
//    initializeSkeleton();
//    initializeAnimations();
//    createGeometry();
//    loadTextures();
}
    
MetalPlayer::~MetalPlayer()
{
    if (vertexBuffer) vertexBuffer->release();
    if (indexBuffer) indexBuffer->release();
    if (instanceBuffer) instanceBuffer->release();
    if (cameraUniformsBuffer) cameraUniformsBuffer->release();
    if (materialUniformsBuffer) materialUniformsBuffer->release();
    if (lightUniformsBuffer) lightUniformsBuffer->release();

    m_boneTransformBuffer->release();
    m_renderPipelineState->release();
    m_depthStencilState->release();

    if (samplerState) samplerState->release();
}

void MetalPlayer::initializeMetal(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device)
{
    NS::SharedPtr<MTL::VertexDescriptor> vertexDescriptor = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());

    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(0)->setOffset(offsetof(VertexPNJ, position));
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(1)->setOffset(offsetof(VertexPNJ, normal));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(2)->setOffset(offsetof(VertexPNJ, texCoord));
    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

    vertexDescriptor->attributes()->object(3)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(3)->setOffset(offsetof(VertexPNJ, tangent));
    vertexDescriptor->attributes()->object(3)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(4)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(4)->setOffset(offsetof(VertexPNJ, bitangent));
    vertexDescriptor->attributes()->object(4)->setBufferIndex(0);

    vertexDescriptor->attributes()->object(4)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(4)->setOffset(offsetof(VertexPNJ, boneWeights));
    vertexDescriptor->attributes()->object(4)->setBufferIndex(0);

    vertexDescriptor->attributes()->object(4)->setFormat(MTL::VertexFormatUInt4);
    vertexDescriptor->attributes()->object(4)->setOffset(offsetof(VertexPNJ, boneIndices));
    vertexDescriptor->attributes()->object(4)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(sizeof(VertexPNJ));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    NS::SharedPtr<MTL::RenderPipelineDescriptor> renderPipelineDescriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    renderPipelineDescriptor->setVertexFunction(shaderLibrary->newFunction(MTLSTR("vertex_main_pnj")));
    renderPipelineDescriptor->setFragmentFunction(shaderLibrary->newFunction(NS::String::string("fragment_main_pnj", NS::UTF8StringEncoding)));
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor.get());
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(depthPixelFormat);

    NS::Error* error = nullptr;
    m_renderPipelineState = device->newRenderPipelineState(renderPipelineDescriptor.get(), &error);

    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    m_depthStencilState = device->newDepthStencilState(depthStencilDescriptor.get());
    //        auto samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
    //        samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
    //        samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
    //        samplerDescriptor->setMipFilter(MTL::SamplerMipFilterLinear);
    //        samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeRepeat);
    //        samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeRepeat);
    //        samplerState = device->newSamplerState(samplerDescriptor);
    //        samplerDescriptor->release();
}

void MetalPlayer::initializeSkeleton(MTL::Device* device)
{
    skeleton = {
        {"Root", 0, -1, simd::float4x4(1.0f), simd::float4x4(1.0f), {0, 0, 0}, simd::quatf(0,0,0,1), {1,1,1}},
        {"Hips", 1, 0, simd::float4x4(1.0f), simd::float4x4(1.0f), {0, 1, 0}, simd::quatf(0,0,0,1), {1,1,1}},
        {"Spine", 2, 1, simd::float4x4(1.0f), simd::float4x4(1.0f), {0, 0.3f, 0}, simd::quatf(0,0,0,1), {1,1,1}},
        {"Chest", 3, 2, simd::float4x4(1.0f), simd::float4x4(1.0f), {0, 0.3f, 0}, simd::quatf(0,0,0,1), {1,1,1}},
        {"Neck", 4, 3, simd::float4x4(1.0f), simd::float4x4(1.0f), {0, 0.3f, 0}, simd::quatf(0,0,0,1), {1,1,1}},
        {"Head", 5, 4, simd::float4x4(1.0f), simd::float4x4(1.0f), {0, 0.2f, 0}, simd::quatf(0,0,0,1), {1,1,1}},
    };

    m_boneTransformBuffer = device->newBuffer(skeleton.size() * sizeof(BoneTransform), MTL::ResourceStorageModeShared);
}

void MetalPlayer::initializeAnimations()
{
//    Animation idle("idle", 3.0f, true);
//    KeyFrame idleFrame1 = {0.0f, {0, 0, 0}, math::makeZRotate(0.05f)};//, simd::float3{1, 0, 0}), {1, 1, 1}};
//    KeyFrame idleFrame2 = {1.5f, {0, 0.02f, 0}, math::makeZRotate(-0.05f)};//simd::quatf::make_rotate(-0.05f, simd::float3{1, 0, 0}), {1, 1, 1}};
//    idle.addKeyFrame(3, idleFrame1); // Chest
//    idle.addKeyFrame(3, idleFrame2);
//    animations[AnimationState::IDLE] = idle;
//
//    Animation walk("walk", 1.2f, true);
//    KeyFrame walkFrame1 = {0.0f, {0, 0, 0}, math::makeXRotate(0.6f)};//simd::quatf::make_rotate(0.6f, simd::float3{1, 0, 0}), {1, 1, 1}};
//    KeyFrame walkFrame2 = {0.6f, {0, 0, 0}, math::makeXRotate(-0.6f)};//simd::quatf::make_rotate(-0.6f, simd::float3{1, 0, 0}), {1, 1, 1}};
//    walk.addKeyFrame(1, walkFrame1); // Hips animation
//    walk.addKeyFrame(1, walkFrame2);
//    animations[AnimationState::WALKING] = walk;
}
    //
    //        // Animation RUNNING - plus rapide et dynamique
    //        Animation run("run", 0.7f, true);
    //        KeyFrame runFrame1 = {0.0f, {0, 0, 0}, simd::quatf::make_rotate(1.0f, simd::float3{1, 0, 0}), {1, 1, 1}};
    //        KeyFrame runFrame2 = {0.35f, {0, 0.1f, 0}, simd::quatf::make_rotate(-1.0f, simd::float3{1, 0, 0}), {1, 1, 1}};
    //        run.addKeyFrame(3, runFrame1);
    //        run.addKeyFrame(3, runFrame2);
    //        animations[AnimationState::RUNNING] = run;
    //
    //        // Animation JUMPING
    //        Animation jump("jump", 0.8f, false);
    //        KeyFrame jumpFrame1 = {0.0f, {0, 0, 0}, simd::quatf::make_rotate(-0.3f, simd::float3{1, 0, 0}), {1, 1, 1}};
    //        KeyFrame jumpFrame2 = {0.4f, {0, 1.5f, 0}, simd::quatf::make_rotate(0.2f, simd::float3{1, 0, 0}), {1, 1, 1}};
    //        jump.addKeyFrame(1, jumpFrame1);
    //        jump.addKeyFrame(1, jumpFrame2);
    //        animations[AnimationState::JUMPING] = jump;
    //
    //        // Animation ATTACKING - coup puissant
    //        Animation attack("attack", 0.6f, false);
    //        KeyFrame attackFrame1 = {0.0f, {0, 0, 0}, simd::quatf::make_rotate(-1.8f, simd::float3{1, 0, 0}), {1, 1, 1}};
    //        KeyFrame attackFrame2 = {0.3f, {0, 0, 0.5f}, simd::quatf::make_rotate(0.8f, simd::float3{1, 0, 0}), {1, 1, 1}};
    //        attack.addKeyFrame(3, attackFrame1);
    //        attack.addKeyFrame(3, attackFrame2);
    //        animations[AnimationState::ATTACKING] = attack;
    //    }
    //
    //    void createGeometry() {
    //        // Création d'un mesh humanoïde simplifié
    //        std::vector<Vertex> vertices;
    //        std::vector<uint16_t> indices;
    //
    //        // Corps principal (très simplifié pour l'exemple)
    //        // En production, vous chargeriez un modèle 3D complet avec Assimp ou ModelIO
    //
    //        vertexBuffer = device->newBuffer(vertices.data(), vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
    //        indexBuffer = device->newBuffer(indices.data(), indices.size() * sizeof(uint16_t), MTL::ResourceStorageModeShared);
    //    }
    //
    //    void loadTextures() {
    //        // Chargement des textures PBR
    //        // En production: MTKTextureLoader pour charger depuis des fichiers
    //        std::cout << "[Metal] Loading PBR textures..." << std::endl;
    //        std::cout << "  - Albedo: character_albedo.png" << std::endl;
    //        std::cout << "  - Normal: character_normal.png" << std::endl;
    //        std::cout << "  - Metallic: character_metallic.png" << std::endl;
    //        std::cout << "  - Roughness: character_roughness.png" << std::endl;
    //        std::cout << "  - AO: character_ao.png" << std::endl;
    //        std::cout << "  - Emissive: character_emissive.png" << std::endl;
    //    }
    //
    //    void update(float deltaTime) {
    //        // Mise à jour physique
    //        if (!isGrounded) {
    //            velocity.y -= 9.81f * deltaTime;
    //        }
    //
    //        position += velocity * deltaTime;
    //
    //        if (position.y >= 0) {
    //            position.y = 0;
    //            velocity.y = 0;
    //            isGrounded = true;
    //        } else {
    //            isGrounded = false;
    //        }
    //
    //        // Mise à jour animation
    //        animations[currentState].update(deltaTime);
    //
    //        // Mise à jour des transformations d'os
    //        updateBoneTransforms();
    //    }
    //
    //    void updateBoneTransforms() {
    //        BoneTransform* transforms = (BoneTransform*)boneTransformBuffer->contents();
    //
    //        for (size_t i = 0; i < skeleton.size(); ++i) {
    //            transforms[i].transform = animations[currentState].getBoneTransform(i);
    //        }
    //    }
    //
    //    void render(MTL::RenderCommandEncoder* renderEncoder) {
    //        // Configuration du pipeline
    //        renderEncoder->setRenderPipelineState(pipelineState);
    //        renderEncoder->setDepthStencilState(depthState);
    //        renderEncoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    //        renderEncoder->setCullMode(MTL::CullModeBack);
    //
    //        // Buffers
    //        renderEncoder->setVertexBuffer(vertexBuffer, 0, 0);
    //        renderEncoder->setVertexBuffer(instanceBuffer, 0, 1);
    //        renderEncoder->setVertexBuffer(cameraUniformsBuffer, 0, 2);
    //        renderEncoder->setVertexBuffer(boneTransformBuffer, 0, 3);
    //
    //        renderEncoder->setFragmentBuffer(materialUniformsBuffer, 0, 0);
    //        renderEncoder->setFragmentBuffer(lightUniformsBuffer, 0, 1);
    //        renderEncoder->setFragmentBuffer(cameraUniformsBuffer, 0, 2);
    //
    //        // Textures
    //        renderEncoder->setFragmentTexture(albedoTexture, 0);
    //        renderEncoder->setFragmentTexture(normalTexture, 1);
    //        renderEncoder->setFragmentTexture(metallicTexture, 2);
    //        renderEncoder->setFragmentTexture(roughnessTexture, 3);
    //        renderEncoder->setFragmentTexture(aoTexture, 4);
    //        renderEncoder->setFragmentTexture(emissiveTexture, 5);
    //        renderEncoder->setFragmentSamplerState(samplerState, 0);
    //
    //        // Draw call
    //        // renderEncoder->drawIndexedPrimitives(...);
    //
    //        std::cout << "[Metal] Rendering character at ("
    //                  << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    //        std::cout << "[Metal] Animation: " << animations[currentState].getName() << std::endl;
    //    }
    //
    //    // Contrôles
    //    void moveForward() {
    //        velocity.z = -speed;
    //        if (isGrounded) currentState = AnimationState::WALKING;
    //    }
    //
    //    void moveBackward() {
    //        velocity.z = speed;
    //        if (isGrounded) currentState = AnimationState::WALKING;
    //    }
    //
    //    void strafeLeft() {
    //        velocity.x = -speed;
    //        if (isGrounded) currentState = AnimationState::WALKING;
    //    }
    //
    //    void strafeRight() {
    //        velocity.x = speed;
    //        if (isGrounded) currentState = AnimationState::WALKING;
    //    }
    //
    //    void run() {
    //        velocity.z = -speed * 2.0f;
    //        if (isGrounded) currentState = AnimationState::RUNNING;
    //    }
    //
    //    void stopMoving() {
    //        velocity.x = 0;
    //        velocity.z = 0;
    //        if (isGrounded) currentState = AnimationState::IDLE;
    //    }
    //
    //    void jump() {
    //        if (isGrounded) {
    //            velocity.y = 10.0f;
    //            isGrounded = false;
    //            currentState = AnimationState::JUMPING;
    //        }
    //    }
    //
    //    void attack() {
    //        if (isGrounded) {
    //            currentState = AnimationState::ATTACKING;
    //            velocity = simd::make_float3(0, 0, 0);
    //        }
    //    }
    //
    //    void printStatus() const {
    //        std::cout << "\n=== METAL PLAYER STATUS ===" << std::endl;
    //        std::cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    //        std::cout << "Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
    //        std::cout << "Health: " << health << "/100" << std::endl;
    //        std::cout << "State: " << getStateName(currentState) << std::endl;
    //        std::cout << "Grounded: " << (isGrounded ? "Yes" : "No") << std::endl;
    //        std::cout << "Animation: " << animations.at(currentState).getName() << std::endl;
    //        std::cout << "Bone count: " << skeleton.size() << std::endl;
    //        std::cout << "==========================" << std::endl;
    //    }
    //
    //    // Getters
    //    simd::float3 getPosition() const { return position; }
    //    int getHealth() const { return health; }
    //    AnimationState getState() const { return currentState; }
    //
    //    void setMaterialColor(const simd::float3& color) {
    //        MaterialUniforms* material = (MaterialUniforms*)materialUniformsBuffer->contents();
    //        material->albedo = color;
    //    }
    //
    //    void setMetallic(float value) {
    //        MaterialUniforms* material = (MaterialUniforms*)materialUniformsBuffer->contents();
    //        material->metallic = value;
    //    }
    //
    //    void setRoughness(float value) {
    //        MaterialUniforms* material = (MaterialUniforms*)materialUniformsBuffer->contents();
    //        material->roughness = value;
    //    }
    //
    //    void setEmissive(const simd::float3& color, float strength) {
    //        MaterialUniforms* material = (MaterialUniforms*)materialUniformsBuffer->contents();
    //        material->emissive = color;
    //        material->emissiveStrength = strength;
    //    }
    //
    //private:
    //    std::string getStateName(AnimationState state) const {
    //        switch(state) {
    //            case AnimationState::IDLE: return "IDLE";
    //            case AnimationState::WALKING: return "WALKING";
    //            case AnimationState::RUNNING: return "RUNNING";
    //            case AnimationState::JUMPING: return "JUMPING";
    //            case AnimationState::ATTACKING: return "ATTACKING";
    //            case AnimationState::ROLLING: return "ROLLING";
    //            case AnimationState::VICTORY: return "VICTORY";
    //            default: return "UNKNOWN";
    //        }
    //    }
    //};
