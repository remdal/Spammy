//
//  RMDLBlender.cpp
//  Spammy
//
//  Created by Rémy on 18/12/2025.
//

#include "RMDLBlender.hpp"

static simd::float4x4 aiToSimd(const aiMatrix4x4& m)
{
    return simd::float4x4{simd::float4{m.a1, m.b1, m.c1, m.d1},
                          simd::float4{m.a2, m.b2, m.c2, m.d2},
                          simd::float4{m.a3, m.b3, m.c3, m.d3},
                          simd::float4{m.a4, m.b4, m.c4, m.d4}};
}

static simd::float4x4 makeTRS(simd::float3 t, simd::quatf r, simd::float3 s)
{
    simd::float4x4 T{simd::float4{ 1, 0, 0, 0 },
                     simd::float4{ 0, 1, 0, 0 },
                     simd::float4{ 0, 0, 1, 0 }, simd::float4{ t.x, t.y, t.z, 1 }};

    simd::float4x4 S{simd::float4{ s.x, 0,   0,   0 },
                     simd::float4{ 0,   s.y, 0,   0 },
                     simd::float4{ 0,   0,   s.z, 0 }, simd::float4{ 0, 0, 0, 1 }};

    return T * simd::float4x4(r) * S;
}

RMDLBlender::RMDLBlender(MTL::Device* pDevice, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, const std::string& resourcesPath, MTL::Library* pShaderLibrary)
: m_device(pDevice->retain()),
m_frame(0), _pCurrentTime(0.0f), _pAnimationDuration(12.0f)
{
    m_boneMatrices.resize(28, math::makeIdentity());
    doTheImportThing(resourcesPath);
    createPipelineBlender(pShaderLibrary, pixelFormat, depthPixelFormat);
    createSampler();
}

RMDLBlender::~RMDLBlender()
{
    for (auto& model : m_models) model.release();
    if (_pPipelineStateBlender) _pPipelineStateBlender->release();
    if (_pPipelineStateBlenderFull) _pPipelineStateBlenderFull->release();
    if (_pDepthState) _pDepthState->release();
    if (_pSampler) _pSampler->release();
    if (m_device) m_device->release();
}

bool RMDLBlender::doTheImportThing(const std::string& resourcesPath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(resourcesPath + "/rosée.glb", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (scene == nullptr)
    {
        printf("false");
        return false;
    }
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("Assimp Error: %s\n", importer.GetErrorString());
        return false;
    }

    printf("✓ GLB loaded:\n");
    printf("  - Meshes: %d\n", scene->mNumMeshes);
    printf("  - Materials: %d\n", scene->mNumMaterials);
    printf("  - Textures embedded: %d\n", scene->mNumTextures);
    printf("  - Animations: %d\n", scene->mNumAnimations);

    return true;
}

size_t RMDLBlender::loadModel(const std::string& resourcesPath, const std::string& name)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(resourcesPath,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
        aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights);

    Blender model;
    model.name = name.empty() ? resourcesPath : name;
    model.rootNode = copyNodeHierarchy(scene->mRootNode);
    
    bool hasBones = modelHasBones(scene->mRootNode, scene);
    
    if (hasBones && scene->mNumAnimations > 0)
    {
        model.hasAnimation = true;
        processNodeSkinned(scene->mRootNode, scene, model);
        model.boneMatrices.resize(model.boneCount, matrix_identity_float4x4);
        model.uniformBuffer = m_device->newBuffer(sizeof(BlenderUniformsFull), MTL::ResourceStorageModeShared);
        loadAnimations(scene, model);
        
        model.vertexBuffer = m_device->newBuffer(model.verticesFull.data(), model.verticesFull.size() * sizeof(VertexBlenderFull), MTL::ResourceStorageModeShared);
    }
    else
    {
        model.hasAnimation = false;
        processNodeStatic(scene->mRootNode, scene, model);
        model.uniformBuffer = m_device->newBuffer(sizeof(BlenderUniforms), MTL::ResourceStorageModeShared);
        
        model.vertexBuffer = m_device->newBuffer(model.vertices.data(), model.vertices.size() * sizeof(VertexBlender), MTL::ResourceStorageModeShared);
    }
    
    std::string path = resourcesPath.substr(0, resourcesPath.find_last_of('/'));
    loadTextures(scene, model, resourcesPath);
    
    model.indexBuffer = m_device->newBuffer(model.indices.data(), model.indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    
    m_models.push_back(std::move(model));
    return m_models.size() - 1;
}

Blender* RMDLBlender::getModel(size_t index)
{
    return index < m_models.size() ? &m_models[index] : nullptr;
}

Blender* RMDLBlender::getModel(const std::string& name)
{
    for (auto& m : m_models)
        if (m.name == name) return &m;
    return nullptr;
}

NodeData RMDLBlender::copyNodeHierarchy(aiNode* node)
{
    NodeData data;
    data.name = node->mName.C_Str();
    data.transform = aiToSimd(node->mTransformation);
    data.children.reserve(node->mNumChildren);
    for (unsigned i = 0; i < node->mNumChildren; i++)
        data.children.push_back(copyNodeHierarchy(node->mChildren[i]));
    return data;
}

bool RMDLBlender::modelHasBones(aiNode* pNode, const aiScene* pScene)
{
    for (unsigned i = 0; i < pNode->mNumMeshes; i++)
        if (pScene->mMeshes[pNode->mMeshes[i]]->mNumBones > 0) return true;
    for (unsigned i = 0; i < pNode->mNumChildren; i++)
        if (modelHasBones(pNode->mChildren[i], pScene)) return true;
    return false;
}

void RMDLBlender::processNodeStatic(aiNode* pNode, const aiScene* pScene, Blender& model)
{
    for (unsigned i = 0; i < pNode->mNumMeshes; i++)
        processMeshStatic(pScene->mMeshes[pNode->mMeshes[i]], model);
    for (unsigned i = 0; i < pNode->mNumChildren; i++)
        processNodeStatic(pNode->mChildren[i], pScene, model);
}

void RMDLBlender::processNodeSkinned(aiNode* pNode, const aiScene* pScene, Blender& model)
{
    for (unsigned i = 0; i < pNode->mNumMeshes; i++)
        processMeshSkinned(pScene->mMeshes[pNode->mMeshes[i]], model);
    for (unsigned i = 0; i < pNode->mNumChildren; i++)
        processNodeSkinned(pNode->mChildren[i], pScene, model);
}

void RMDLBlender::processMeshStatic(aiMesh* pMesh, Blender& model)
{
    uint32_t baseVertex = static_cast<uint32_t>(model.vertices.size());
    
    for (unsigned i = 0; i < pMesh->mNumVertices; i++)
    {
        VertexBlender v;
        v.position = {pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z};
        v.normal = pMesh->HasNormals()
            ? simd::float3{pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z}
            : simd::float3{0, 1, 0};
        v.texCoord = pMesh->mTextureCoords[0]
            ? simd::float2{pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y}
            : simd::float2{0, 0};
        model.vertices.push_back(v);
    }
    
    for (unsigned i = 0; i < pMesh->mNumFaces; i++)
    {
        aiFace& face = pMesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++)
            model.indices.push_back(baseVertex + face.mIndices[j]);
    }
}

void RMDLBlender::processMeshSkinned(aiMesh* pMesh, Blender& model)
{
    uint32_t baseVertex = static_cast<uint32_t>(model.verticesFull.size());
    
    for (unsigned i = 0; i < pMesh->mNumVertices; i++)
    {
        VertexBlenderFull v;
        v.position = {pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z};
        v.normal = pMesh->HasNormals()
            ? simd::float3{pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z}
            : simd::float3{0, 1, 0};
        v.texCoord = pMesh->mTextureCoords[0]
            ? simd::float2{pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y}
            : simd::float2{0, 0};
        v.joints = {-1, -1, -1, -1};
        v.weights = {0, 0, 0, 0};
        model.verticesFull.push_back(v);
    }
    
    if (pMesh->mNumBones > 0)
    {
        loadBones(pMesh, model, baseVertex);

        for (unsigned i = baseVertex; i < model.verticesFull.size(); i++)
        {
            auto& v = model.verticesFull[i];
            float sum = v.weights.x + v.weights.y + v.weights.z + v.weights.w;
            
            if (sum > 0.0f)
                v.weights /= sum;
            else
                v.joints.x = 0; v.weights.x = 1.0f;
        }
    }
    else
    {
        for (unsigned i = baseVertex; i < model.verticesFull.size(); i++)
        {
            model.verticesFull[i].joints = {0, 0, 0, 0};
            model.verticesFull[i].weights = {1, 0, 0, 0};
        }
    }
    
    for (unsigned i = 0; i < pMesh->mNumFaces; i++)
    {
        aiFace& face = pMesh->mFaces[i];

        for (unsigned j = 0; j < face.mNumIndices; j++)
            model.indices.push_back(baseVertex + face.mIndices[j]);
    }
}

void RMDLBlender::loadBones(aiMesh* pMesh, Blender& model, uint32_t baseVertex)
{
    for (unsigned i = 0; i < pMesh->mNumBones; i++)
    {
        aiBone* pBone = pMesh->mBones[i];
        std::string name = pBone->mName.C_Str();
        int boneID;
        
        auto it = model.boneMap.find(name);
        if (it == model.boneMap.end())
        {
            boneID = model.boneCount++;
            model.boneMap[name] = {boneID, aiToSimd(pBone->mOffsetMatrix)};
        }
        else
            boneID = it->second.id;
        
        for (unsigned j = 0; j < pBone->mNumWeights; j++)
        {
            unsigned vertID = baseVertex + pBone->mWeights[j].mVertexId;
            float weight = pBone->mWeights[j].mWeight;
            auto& v = model.verticesFull[vertID];
            
            for (int k = 0; k < 4; k++)
            {
                if (v.joints[k] < 0)
                {
                    v.joints[k] = boneID;
                    v.weights[k] = weight;
                    break;
                }
            }
        }
    }
}

void RMDLBlender::createPipelineBlender(MTL::Library *pShaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat)
{
    MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();

    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(0)->setOffset(offsetof(VertexBlender, position));
    vertexDesc->attributes()->object(0)->setBufferIndex(0);

    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(1)->setOffset(offsetof(VertexBlender, normal));
    vertexDesc->attributes()->object(1)->setBufferIndex(0);
    
    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(2)->setOffset(offsetof(VertexBlender, texCoord));
    vertexDesc->attributes()->object(2)->setBufferIndex(0);
    
    vertexDesc->layouts()->object(0)->setStride(sizeof(VertexBlender));
    
    MTL::VertexDescriptor* vertexDescFull = MTL::VertexDescriptor::alloc()->init();

    vertexDescFull->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDescFull->attributes()->object(0)->setOffset(offsetof(VertexBlenderFull, position));
    vertexDescFull->attributes()->object(0)->setBufferIndex(0);

    vertexDescFull->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDescFull->attributes()->object(1)->setOffset(offsetof(VertexBlenderFull, normal));
    vertexDescFull->attributes()->object(1)->setBufferIndex(0);
    
    vertexDescFull->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertexDescFull->attributes()->object(2)->setOffset(offsetof(VertexBlenderFull, texCoord));
    vertexDescFull->attributes()->object(2)->setBufferIndex(0);
    
    vertexDescFull->attributes()->object(3)->setFormat(MTL::VertexFormatInt4);
    vertexDescFull->attributes()->object(3)->setOffset(offsetof(VertexBlenderFull, joints));
    vertexDescFull->attributes()->object(3)->setBufferIndex(0);
    
    vertexDescFull->attributes()->object(4)->setFormat(MTL::VertexFormatFloat4);
    vertexDescFull->attributes()->object(4)->setOffset(offsetof(VertexBlenderFull, weights));
    vertexDescFull->attributes()->object(4)->setBufferIndex(0);
    
    vertexDescFull->layouts()->object(0)->setStride(sizeof(VertexBlenderFull));
//    vertexDescFull->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    MTL::RenderPipelineDescriptor* renderPipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipelineDesc->setVertexFunction(pShaderLibrary->newFunction(NS::String::string("vertexmain", NS::UTF8StringEncoding)));
    renderPipelineDesc->setFragmentFunction(pShaderLibrary->newFunction(NS::String::string("fragmentmain", NS::UTF8StringEncoding)));
    renderPipelineDesc->setVertexDescriptor(vertexDesc);
    renderPipelineDesc->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    renderPipelineDesc->setDepthAttachmentPixelFormat(depthPixelFormat);

    NS::Error* pError = nullptr;
    _pPipelineStateBlender = m_device->newRenderPipelineState(renderPipelineDesc, &pError);
    
    renderPipelineDesc->setVertexFunction(pShaderLibrary->newFunction(MTLSTR("vertex_full")));
    renderPipelineDesc->setVertexDescriptor(vertexDescFull);
    _pPipelineStateBlenderFull = m_device->newRenderPipelineState(renderPipelineDesc, &pError);

    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    _pDepthState = m_device->newDepthStencilState(depthDesc);

    renderPipelineDesc->release();
    vertexDesc->release();
    vertexDescFull->release();
    depthDesc->release();
    printf("✓ Pipeline created\n");
}

void RMDLBlender::updateBlender(float deltaTime)
{
//    float bounce = sin(t * M_PI * 2.0f) * 0.5f;
//    boneMatrix = simd::float4x4{
//        simd::make_float4(1, 0, 0, 0),
//        simd::make_float4(0, 1, 0, 0),
//        simd::make_float4(0, 0, 1, 0),
//        simd::make_float4(0, bounce, 0, 1)
//    };
    m_frame++;
    const uint32_t frameIndex = m_frame % 3;

    for (auto& model : m_models)
    {
        if (!model.hasAnimation || model.animations.empty())
            continue;
        
        Animation& anim = model.animations[model.currentAnimation];
        model.currentTime += deltaTime * anim.ticksPerSec;
        if (model.currentTime > anim.duration)
            model.currentTime = fmod(model.currentTime, anim.duration);
        
        for (auto& m : m_boneMatrices) m = math::makeIdentity();
        computeBoneTransforms(model.currentTime, model.rootNode, math::makeIdentity(), model);
    }
}

MTL::Texture* RMDLBlender::loadEmbeddedTexture(aiTexture *aiTexture, bool sRGB)
{
    int width, height, channels;
    unsigned char* data = nullptr;
    if (aiTexture->mHeight == 0) // compressed PNG && JPG
        data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexture->pcData), aiTexture->mWidth, &width, &height, &channels, 4);
    else {
        width = aiTexture->mWidth;
        height = aiTexture->mHeight;
        data = reinterpret_cast<unsigned char*>(aiTexture->pcData);
    }

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setPixelFormat(sRGB ? MTL::PixelFormatRGBA8Unorm_sRGB : MTL::PixelFormatRGBA8Unorm);
    desc->setWidth(width);
    desc->setHeight(height);
    desc->setUsage(MTL::TextureUsageShaderRead);

    MTL::Texture* texture = m_device->newTexture(desc);
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
    texture->replaceRegion(region, 0, data, width * 4);

    if (aiTexture->mHeight == 0)
        stbi_image_free(data);
    desc->release();

    return texture;
}

void RMDLBlender::loadTextures(const aiScene* scene, Blender& model, const std::string& resourcesPath)
{
    if (scene->mNumMaterials == 0) return;
    aiMaterial* mat = scene->mMaterials[0];
    aiString path;
    
    auto loadTex = [&](aiTextureType type, bool sRGB) -> MTL::Texture*
    {
        if (mat->GetTexture(type, 0, &path) != AI_SUCCESS) return nullptr;
        if (path.data[0] == '*')
        {
            int idx = atoi(path.data + 1);
            if (idx < (int)scene->mNumTextures)
                return loadEmbeddedTexture(scene->mTextures[idx], sRGB);
        }
        return nullptr; // External texture loading omitted for brevity
    };
    
    model.diffuseTexture = loadTex(aiTextureType_DIFFUSE, true);
    model.normalTexture = loadTex(aiTextureType_NORMALS, false);
    model.roughnessTexture = loadTex(aiTextureType_DIFFUSE_ROUGHNESS, false);
    model.metallicTexture = loadTex(aiTextureType_METALNESS, false);
}

MTL::Texture* RMDLBlender::loadTexture(const std::string& resourcesPath, const char* path, const aiScene *scene, bool sRGB)
{
    int texIndex = ft_atoi(path + 1);
    if (texIndex < scene->mNumTextures)
    {
        aiTexture* embeddedTex = scene->mTextures[texIndex];
        return loadEmbeddedTexture(embeddedTex, sRGB);
    }
    std::string fullPath = resourcesPath + "/" + std::string(path);
    int width, height, channels;
    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &channels, 4);
    MTL::PixelFormat format = sRGB ? MTL::PixelFormatRGBA8Unorm_sRGB : MTL::PixelFormatRGBA8Unorm;

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setTextureType(MTL::TextureType2D);
    desc->setPixelFormat(format);
    desc->setWidth(width);
    desc->setHeight(height);
    desc->setUsage(MTL::TextureUsageShaderRead);

    MTL::Texture* texture = m_device->newTexture(desc);
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
    texture->replaceRegion(region, 0, data, width * 4);

    stbi_image_free(data);
    desc->release();

    return texture;
}

//void RMDLBlender::loadTextures(const std::string& resourcesPath, const aiScene *scene)
//{
//    aiMaterial* material = scene->mMaterials[0];
//    aiString texPath;
//    material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
//    _pDiffuseTexture = loadTexture(resourcesPath, texPath.C_Str(), scene, true);
//    material->GetTexture(aiTextureType_NORMALS, 0, &texPath);
//    _pNormalTexture = loadTexture(resourcesPath, texPath.C_Str(), scene, false);
//    material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath);
//    _pRoughnessTexture = loadTexture(resourcesPath, texPath.C_Str(), scene, false);
//    material->GetTexture(aiTextureType_METALNESS, 0, &texPath);
//}

void RMDLBlender::loadAnimations(const aiScene *scene, Blender& model)
{
    for (unsigned i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation* anim = scene->mAnimations[i];
        Animation a;
        a.name = anim->mName.C_Str();
        a.duration = (float)anim->mDuration;
        a.ticksPerSec = anim->mTicksPerSecond > 0 ? (float)anim->mTicksPerSecond : 25.0f;
        
        for (unsigned c = 0; c < anim->mNumChannels; c++)
        {
            aiNodeAnim* ch = anim->mChannels[c];
            BoneAnimation ba;
            ba.boneName = ch->mNodeName.C_Str();
            
            for (unsigned p = 0; p < ch->mNumPositionKeys; p++)
            {
                auto& key = ch->mPositionKeys[p];
                ba.positions.push_back({(float)(key.mTime), {key.mValue.x, key.mValue.y, key.mValue.z}}); //ch->mPositionKeys[p].mTime, {ch->mPositionKeys[p].mValue.x, ch->mPositionKeys[p].mValue.y, ch->mPositionKeys[p].mValue.z}});
            }
            
            for (unsigned r = 0; r < ch->mNumRotationKeys; r++)
            {
                auto& key = ch->mRotationKeys[r];
                ba.rotations.push_back({(float)(key.mTime), simd_quaternion(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w)});//ch->mRotationKeys[r].mTime, simd_quaternion(q.x, q.y, q.z, q.w)});
            }
            
            for (unsigned s = 0; s < ch->mNumScalingKeys; s++)
            {
                auto& key = ch->mScalingKeys[s];
                ba.scales.push_back({(float)(key.mTime), {key.mValue.x, key.mValue.y, key.mValue.z}});
            }
            
            a.channels.push_back(ba);
        }
        model.animations.push_back(a);
    }
}

void RMDLBlender::createSampler()
{
    MTL::SamplerDescriptor* samplerDesc = MTL::SamplerDescriptor::alloc()->init();
    samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMipFilter(MTL::SamplerMipFilterLinear);
    samplerDesc->setSAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setTAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setMaxAnisotropy(16);
    _pSampler = m_device->newSamplerState(samplerDesc);
    samplerDesc->release();
}

void RMDLBlender::computeBoneTransforms(float time, const NodeData& node, const simd::float4x4& parentTf, Blender& model)
{
    simd::float4x4 localTf = node.transform;

    if (!model.animations.empty())
    {
        Animation& anim = model.animations[model.currentAnimation];
        for (auto& ch : anim.channels)
        {
            if (ch.boneName == node.name)
            {
                localTf = makeTRS(interpolatePosition(time, ch), interpolateRotation(time, ch), interpolateScale(time, ch));
                break;
            }
        }
    }
    simd::float4x4 globalTf = parentTf * localTf;

    auto it = model.boneMap.find(node.name);
    
    if (it != model.boneMap.end())
        model.boneMatrices[it->second.id] = globalTf * it->second.offset;
    
    for (const auto& child : node.children)
        computeBoneTransforms(time, child, globalTf, model);
}

simd::float3 RMDLBlender::interpolatePosition(float time, const BoneAnimation& anim)
{
    if (anim.positions.size() == 1) return anim.positions[0].value;
    
    size_t i = 0;
    while (i < anim.positions.size() - 1 && anim.positions[i + 1].time < time) i++;
    size_t j = (i + 1) % anim.positions.size();
    
    float dt = anim.positions[j].time - anim.positions[i].time;
    float t = (dt > 0) ? (time - anim.positions[i].time) / dt : 0;
    return simd_mix(anim.positions[i].value, anim.positions[j].value, t);
}

simd::quatf RMDLBlender::interpolateRotation(float time, const BoneAnimation& anim)
{
    if (anim.rotations.size() == 1) return anim.rotations[0].value;
    
    size_t i = 0;
    while (i < anim.rotations.size() - 1 && anim.rotations[i + 1].time < time) i++;
    size_t j = (i + 1) % anim.rotations.size();
    
    float dt = anim.rotations[j].time - anim.rotations[i].time;
    float t = (dt > 0) ? (time - anim.rotations[i].time) / dt : 0;
    return simd::slerp(anim.rotations[i].value, anim.rotations[j].value, t);
}

simd::float3 RMDLBlender::interpolateScale(float time, const BoneAnimation& anim)
{
    if (anim.scales.size() == 1) return anim.scales[0].value;
    
    size_t i = 0;
    while (i < anim.scales.size() - 1 && anim.scales[i + 1].time < time) i++;
    size_t j = (i + 1) % anim.scales.size();
    
    float dt = anim.scales[j].time - anim.scales[i].time;
    float t = (dt > 0) ? (time - anim.scales[i].time) / dt : 0;
    return simd_mix(anim.scales[i].value, anim.scales[j].value, t);
}

void RMDLBlender::printMemoryStats() const
{
    size_t totalVertexMem = 0, totalIndexMem = 0, totalUniformMem = 0;
    size_t staticCount = 0, skinnedCount = 0;
    
    printf("\n=== RMDLBlender Memory Stats ===\n");
    for (const auto& m : m_models)
{
        size_t vertMem, unifMem;
        if (m.hasAnimation) {
            vertMem = m.verticesFull.size() * sizeof(VertexBlenderFull);
            unifMem = sizeof(BlenderUniformsFull);
            skinnedCount++;
        } else {
            vertMem = m.vertices.size() * sizeof(VertexBlender);
            unifMem = sizeof(BlenderUniforms);
            staticCount++;
        }
        size_t idxMem = m.indices.size() * sizeof(uint32_t);
        
        printf("  %-20s %s  V:%.1fKB  I:%.1fKB  U:%.1fKB\n",
               m.name.c_str(), m.hasAnimation ? "[SKIN]" : "[STAT]",
               vertMem / 1024.f, idxMem / 1024.f, unifMem / 1024.f);
        
        totalVertexMem += vertMem;
        totalIndexMem += idxMem;
        totalUniformMem += unifMem;
    }
    printf("--------------------------------\n");
    printf("  Models: %zu static, %zu skinned\n", staticCount, skinnedCount);
    printf("  Total: %.1f KB (V:%.1f + I:%.1f + U:%.1f)\n",
           (totalVertexMem + totalIndexMem + totalUniformMem) / 1024.f,
           totalVertexMem / 1024.f, totalIndexMem / 1024.f, totalUniformMem / 1024.f);
    printf("================================\n\n");
}

//// Interpoler entre keyframes
//simd::float4x4 RMDLBlender::sampleChannel(const AnimationChannel& channel, float time)
//{
//    // Trouver les deux keyframes encadrant 'time'
//    size_t i = 0;
//    while (i < channel.keyframes.size() - 1 && channel.keyframes[i + 1].time < time)
//        ++i;
//    
//    const auto& k0 = channel.keyframes[i];
//    const auto& k1 = channel.keyframes[std::min(i + 1, channel.keyframes.size() - 1)];
//    
//    float t = (k1.time > k0.time) ? (time - k0.time) / (k1.time - k0.time) : 0.0f;
//    
//    // Interpolation
//    simd::float3 pos = simd_mix(k0.position, k1.position, t);
//    simd::quatf rot = simd::slerp(k0.rotation, k1.rotation, t);
//    simd::float3 scl = simd_mix(k0.scale, k1.scale, t);
//    
//    return simd::float4x4(math::makeTranslate(pos) * simd::float4x4(rot) * simd::float4x4(math::makeScale(scl)));
//}

//void RMDLBlender::update(float deltaTime)
//{
//    m_currentTime = fmod(m_currentTime + deltaTime * ticksPerSecond, duration);
//    
//    for (auto& channel : currentAnimation->channels) {
//        bones[channel.boneIndex].localTransform = sampleChannel(channel, currentTime);
//    }
//    
//    // Propager dans la hiérarchie
//    for (size_t i = 0; i < bones.size(); ++i) {
//        if (bones[i].parentIndex >= 0)
//            bones[i].globalTransform = bones[bones[i].parentIndex].globalTransform * bones[i].localTransform;
//        else
//            bones[i].globalTransform = bones[i].localTransform;
//    }
//    
//    // Matrices finales pour le GPU
//    for (size_t i = 0; i < bones.size(); ++i)
//        boneMatrices[i] = bones[i].globalTransform * bones[i].offsetMatrix;
//}

void RMDLBlender::draw(MTL::RenderCommandEncoder* pEncoder, const simd::float4x4& viewProj)
{
    for (size_t i = 0; i < m_models.size(); i++)
        drawBlender(pEncoder, i, viewProj, m_models[i].transform);
}

void RMDLBlender::drawBlender(MTL::RenderCommandEncoder *pEncoder, size_t index, const simd::float4x4 &viewProjectionMatrix, const simd::float4x4 &modelMatrix)
{
    if (index >= m_models.size()) return;
    Blender& model = m_models[index];
    
    if (model.hasAnimation)
    {
        BlenderUniformsFull* uniforms = (BlenderUniformsFull *)model.uniformBuffer->contents();
        uniforms->modelMatrix = modelMatrix;
        uniforms->viewProjectionMatrix = viewProjectionMatrix;
        memcpy(uniforms->boneMatrices, model.boneMatrices.data(), model.boneCount * sizeof(simd::float4x4));
        pEncoder->setRenderPipelineState(_pPipelineStateBlenderFull);
    }
    else
    {
        BlenderUniforms* uniforms = (BlenderUniforms *)model.uniformBuffer->contents();
        uniforms->modelMatrix = modelMatrix;
        uniforms->viewProjectionMatrix = viewProjectionMatrix;
        pEncoder->setRenderPipelineState(_pPipelineStateBlender);
    }
    pEncoder->setDepthStencilState(_pDepthState);
    pEncoder->setCullMode(MTL::CullModeFront);
    pEncoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    pEncoder->setVertexBuffer(model.vertexBuffer, 0, 0);
    pEncoder->setVertexBuffer(model.uniformBuffer, 0, 1);
    pEncoder->setFragmentBuffer(model.uniformBuffer, 0, 0);
    pEncoder->setFragmentTexture(model.diffuseTexture, 0);
    pEncoder->setFragmentTexture(model.normalTexture, 1);
    pEncoder->setFragmentTexture(model.roughnessTexture, 2);
    pEncoder->setFragmentTexture(model.metallicTexture, 3);
    pEncoder->setFragmentSamplerState(_pSampler, 0);
    pEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, model.indices.size(), MTL::IndexTypeUInt32, model.indexBuffer, 0);
}
