//
//  RMDLBlender.cpp
//  Spammy
//
//  Created by Rémy on 18/12/2025.
//

#include "RMDLBlender.hpp"

static simd::float4x4 aiToSimd(const aiMatrix4x4& m)
{
    return simd::float4x4{
        simd::float4{m.a1, m.b1, m.c1, m.d1},
        simd::float4{m.a2, m.b2, m.c2, m.d2},
        simd::float4{m.a3, m.b3, m.c3, m.d3},
        simd::float4{m.a4, m.b4, m.c4, m.d4}
    };
}

RMDLBlender::RMDLBlender(MTL::Device* pDevice, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, const std::string& resourcesPath, MTL::Library* pShaderLibrary) :
m_device(pDevice->retain()), m_frame(0),
_pCurrentTime(0.0f), _pAnimationDuration(12.0f)
{
    m_boneMatrices.resize(28, math::makeIdentity());
    _pUniformBufferBlender = m_device->newBuffer(sizeof(BlenderUniforms), MTL::ResourceStorageModeShared);
    doTheImportThing(resourcesPath);
    loadGlb(resourcesPath);
    createPipelineBlender(pShaderLibrary, pixelFormat, depthPixelFormat);
    printf("   Pipeline: %p\n", _pPipelineStateBlender);
    printf("   Textures: D=%p N=%p R=%p M=%p\n", _pDiffuseTexture, _pNormalTexture, _pRoughnessTexture, _pMetallicTexture);
    printf("✓ Device: %p\n", m_device);
    printf("✓ Shader library: %p\n", pShaderLibrary);
    printf("✓ GLB path exists: %s\n", resourcesPath.c_str());
    createSampler();
}

RMDLBlender::~RMDLBlender()
{
    
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

void RMDLBlender::loadGlb(const std::string &resourcePath)
{
    Assimp::Importer    importer;

    const aiScene* scene = importer.ReadFile(resourcePath + "/rosée.glb", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights);
//    _rootNode = scene->mRootNode;
//    loadMesh(scene);
    _rootNode = copyNodeHierarchy(scene->mRootNode);
    processNode(scene->mRootNode, scene);
    loadAnimation(scene);
    loadTextures(resourcePath, scene);
    createBuffers();
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

void RMDLBlender::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
        processMesh(scene->mMeshes[node->mMeshes[i]], scene);
    for (unsigned i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

void RMDLBlender::processMesh(aiMesh* mesh, const aiScene* scene)
{
    uint32_t baseVertex = (uint32_t)m_vertices.size();
    
    for (unsigned i = 0; i < mesh->mNumVertices; i++)
    {
        VertexBlender v{};
        v.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        v.normal = mesh->HasNormals() ?
            simd::float3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z} :
            simd::float3{0, 1, 0};
        v.texCoord = mesh->mTextureCoords[0] ?
            simd::float2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y} :
            simd::float2{0, 0};
        v.joints = {-1, -1, -1, -1};  // -1 = pas de bone
        v.weights = {0, 0, 0, 0};
        m_vertices.push_back(v);
    }
    
    loadBones(mesh);
    
    // Normalize weights pour les vertices de CE mesh
    for (unsigned i = baseVertex; i < m_vertices.size(); i++)
    {
        float sum = m_vertices[i].weights.x + m_vertices[i].weights.y + m_vertices[i].weights.z + m_vertices[i].weights.w;
        if (sum > 0.0f)
            m_vertices[i].weights /= sum;
        else {
            // Vertex sans bone → bind à l'identité (bone 0 fictif)
            m_vertices[i].joints.x = 0;
            m_vertices[i].weights.x = 1.0f;
        }
    }
    
    for (unsigned i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++)
            m_indices.push_back(baseVertex + face.mIndices[j]);
    }
}

void RMDLBlender::loadBones(aiMesh* mesh)
{
    uint32_t baseVertex = (uint32_t)m_vertices.size() - mesh->mNumVertices;
    
    for (unsigned i = 0; i < mesh->mNumBones; i++)
    {
        aiBone* bone = mesh->mBones[i];
        std::string name = bone->mName.C_Str();
        int boneID;
        
        if (m_boneMap.find(name) == m_boneMap.end())
        {
            boneID = _boneCount++;
            BoneInfo info{boneID, aiToSimd(bone->mOffsetMatrix)};
            m_boneMap[name] = info;
        } else
            boneID = m_boneMap[name].id;
        
        // Assign weights to vertices
        for (unsigned j = 0; j < bone->mNumWeights; j++)
        {
            unsigned vertID = baseVertex + bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            
            // Find free slot
            for (int k = 0; k < 4; k++) // MAX_BONE_INFLUENCE
            {
                if (m_vertices[vertID].joints[k] < 0)
                {
                    m_vertices[vertID].joints[k] = boneID;
                    m_vertices[vertID].weights[k] = weight;
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
    
    vertexDesc->attributes()->object(3)->setFormat(MTL::VertexFormatInt4);
    vertexDesc->attributes()->object(3)->setOffset(offsetof(VertexBlender, joints));
    vertexDesc->attributes()->object(3)->setBufferIndex(0);
    
    vertexDesc->attributes()->object(4)->setFormat(MTL::VertexFormatFloat4);
    vertexDesc->attributes()->object(4)->setOffset(offsetof(VertexBlender, weights));
    vertexDesc->attributes()->object(4)->setBufferIndex(0);
    
    vertexDesc->layouts()->object(0)->setStride(sizeof(VertexBlender));
    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    MTL::RenderPipelineDescriptor* renderPipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipelineDesc->setVertexFunction(pShaderLibrary->newFunction(NS::String::string("vertexmain", NS::UTF8StringEncoding)));
    renderPipelineDesc->setFragmentFunction(pShaderLibrary->newFunction(NS::String::string("fragmentmain", NS::UTF8StringEncoding)));
    renderPipelineDesc->setVertexDescriptor(vertexDesc);
    renderPipelineDesc->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    renderPipelineDesc->setDepthAttachmentPixelFormat(depthPixelFormat);

    NS::Error* pError = nullptr;
    _pPipelineStateBlender = m_device->newRenderPipelineState(renderPipelineDesc, &pError);

    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    _pDepthState = m_device->newDepthStencilState(depthDesc);

    renderPipelineDesc->release();
    vertexDesc->release();
    depthDesc->release();
    printf("✓ Pipeline created\n");
}

void RMDLBlender::updateBlender(float deltaTime)
{
//    float t = _pCurrentTime / _pAnimationDuration;
//    float bounce = sin(t * M_PI * 2.0f) * 0.5f;

//    boneMatrix = simd::float4x4{
//        simd::make_float4(1, 0, 0, 0),
//        simd::make_float4(0, 1, 0, 0),
//        simd::make_float4(0, 0, 1, 0),
//        simd::make_float4(0, bounce, 0, 1)
//    };
    if (mv_animations.empty()) return;
    
    m_frame++;
    const uint32_t frameIndex = m_frame % 3;
    Animation& anim = mv_animations[m_currentAnimation];
    m_currentTime += deltaTime * anim.ticksPerSec;
    if (m_currentTime > anim.duration)
        m_currentTime = fmod(m_currentTime, anim.duration);
    
    // Reset matrices
    for (auto& m : m_boneMatrices) m = math::makeIdentity();
    
    // Compute transforms from root
    computeBoneTransforms(m_currentTime, _rootNode, math::makeIdentity());
}

void RMDLBlender::loadMesh(const aiScene *scene)
{
    aiMesh* mesh = scene->mMeshes[0];
    _pVertices.resize(mesh->mNumVertices);

    for (unsigned i = 0; i < mesh->mNumVertices; i++)
    {
        _pVertices[i].position = simd::make_float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if (mesh->HasNormals())
            _pVertices[i].normal = simd::make_float3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->mTextureCoords[0])
            _pVertices[i].texCoord = simd::make_float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else
            _pVertices[i].texCoord = simd::make_float2(0, 0);
                    
        // Default bone weights (pas de skinning complexe pour l'instant)
        _pVertices[i].joints = simd::make_int4(0, 0, 0, 0);
        _pVertices[i].weights = simd::make_float4(1, 0, 0, 0);
    }

    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone* bone = mesh->mBones[i];
        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
            unsigned int vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            // Add to first available slot
            for (int k = 0; k < 4; k++)
            {
                if (_pVertices[vertexID].weights[k] == 0.0f)
                {
                    _pVertices[vertexID].joints[k] = i;
                    _pVertices[vertexID].weights[k] = weight;
                    break;
                }
            }
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            _pIndices.push_back(face.mIndices[j]);
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

    MTL::PixelFormat format = sRGB ? MTL::PixelFormatRGBA8Unorm_sRGB : MTL::PixelFormatRGBA8Unorm;

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setPixelFormat(format);
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

void RMDLBlender::loadTextures(const std::string& resourcesPath, const aiScene* scene)
{
    if (scene->mNumMaterials == 0) return;
    aiMaterial* mat = scene->mMaterials[0];
    aiString path;
    
    auto loadTex = [&](aiTextureType type, bool sRGB) -> MTL::Texture*
    {
        if (mat->GetTexture(type, 0, &path) != AI_SUCCESS) return nullptr;
        if (path.data[0] == '*') {
            int idx = atoi(path.data + 1);
            if (idx < (int)scene->mNumTextures)
                return loadEmbeddedTexture(scene->mTextures[idx], sRGB);
        }
        return nullptr; // External texture loading omitted for brevity
    };
    
    _pDiffuseTexture = loadTex(aiTextureType_DIFFUSE, true);
    _pNormalTexture = loadTex(aiTextureType_NORMALS, false);
    _pRoughnessTexture = loadTex(aiTextureType_DIFFUSE_ROUGHNESS, false);
    _pMetallicTexture = loadTex(aiTextureType_METALNESS, false);
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

void RMDLBlender::loadAnimation(const aiScene *scene)
{
//    aiAnimation* anim = scene->mAnimations[0];
//    _pAnimationDuration = anim->mDuration / anim->mTicksPerSecond;
//    printf("✓ Animation loaded: %.2fs\n", _pAnimationDuration);
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
                ba.positions.push_back({(float)ch->mPositionKeys[p].mTime, {ch->mPositionKeys[p].mValue.x, ch->mPositionKeys[p].mValue.y, ch->mPositionKeys[p].mValue.z}});
            
            for (unsigned r = 0; r < ch->mNumRotationKeys; r++)
            {
                auto& q = ch->mRotationKeys[r].mValue;
                ba.rotations.push_back({(float)ch->mRotationKeys[r].mTime,
                    simd_quaternion(q.x, q.y, q.z, q.w)});
            }
            
            for (unsigned s = 0; s < ch->mNumScalingKeys; s++)
                ba.scales.push_back({(float)ch->mScalingKeys[s].mTime, {ch->mScalingKeys[s].mValue.x, ch->mScalingKeys[s].mValue.y, ch->mScalingKeys[s].mValue.z}});
            
            a.channels.push_back(ba);
        }
        mv_animations.push_back(a);
    }
}

void RMDLBlender::createBuffers()
{
    _pVertexBufferBlender = m_device->newBuffer(m_vertices.data(), m_vertices.size() * sizeof(VertexBlender), MTL::ResourceStorageModeShared);
    _pIndexBufferBlender = m_device->newBuffer(m_indices.data(), m_indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
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

static simd::float4x4 makeTRS(simd::float3 t, simd::quatf r, simd::float3 s)
{
    simd::float4x4 T{simd::float4{1,0,0,0}, simd::float4{0,1,0,0},
                     simd::float4{0,0,1,0}, simd::float4{t.x,t.y,t.z,1}};
    simd::float4x4 R = simd::float4x4(r);
    simd::float4x4 S{simd::float4{s.x,0,0,0}, simd::float4{0,s.y,0,0},
                     simd::float4{0,0,s.z,0}, simd::float4{0,0,0,1}};
    return T * R * S;
}

void RMDLBlender::computeBoneTransforms(float time, const NodeData& node, const simd::float4x4& parentTf)
{
    simd::float4x4 localTf = node.transform;
//    std::string name = node->mName.C_Str();
    // Check if this node has animation
    if (!mv_animations.empty())
    {
        Animation& anim = mv_animations[m_currentAnimation];
        for (auto& ch : anim.channels)
        {
            if (ch.boneName == node.name)
            {
                simd::float3 pos = interpolatePosition(time, ch);
                simd::quatf rot = interpolateRotation(time, ch);
                simd::float3 scl = interpolateScale(time, ch);
                localTf = makeTRS(pos, rot, scl);
                break;
            }
        }
    }
    simd::float4x4 globalTf = parentTf * localTf;

    // If this is a bone, compute final matrix
    auto it = m_boneMap.find(node.name);
    if (it != m_boneMap.end())
    {
        int id = it->second.id;
        m_boneMatrices[id] = globalTf * it->second.offset;
    }
    
    // Recurse sur les enfants (maintenant safe, on possède les données)
    for (const auto& child : node.children)
        computeBoneTransforms(time, child, globalTf);
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

void RMDLBlender::drawBlender(MTL::RenderCommandEncoder *pEncoder, const simd::float4x4 &viewProjectionMatrix, const simd::float4x4 &model)
{
    BlenderUniforms* uniforms = (BlenderUniforms *)_pUniformBufferBlender->contents();
    uniforms->modelMatrix = model;
    uniforms->viewProjectionMatrix = viewProjectionMatrix;
    memcpy(uniforms->boneMatrices, m_boneMatrices.data(), _boneCount * sizeof(simd::float4x4));

    pEncoder->setRenderPipelineState(_pPipelineStateBlender);
    pEncoder->setDepthStencilState(_pDepthState);
    pEncoder->setCullMode(MTL::CullModeFront);
    pEncoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    pEncoder->setVertexBuffer(_pVertexBufferBlender, 0, 0);
    pEncoder->setVertexBuffer(_pUniformBufferBlender, 0, 1);
    pEncoder->setFragmentBuffer(_pUniformBufferBlender, 0, 0);
    pEncoder->setFragmentTexture(_pDiffuseTexture, 0);
    pEncoder->setFragmentTexture(_pNormalTexture, 1);
    pEncoder->setFragmentTexture(_pRoughnessTexture, 2);
    pEncoder->setFragmentTexture(_pMetallicTexture, 3);
    pEncoder->setFragmentSamplerState(_pSampler, 0);
    pEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, m_indices.size(), MTL::IndexTypeUInt32, _pIndexBufferBlender, 0);
}
