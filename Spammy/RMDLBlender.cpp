//
//  RMDLBlender.cpp
//  Spammy
//
//  Created by Rémy on 18/12/2025.
//

#include "RMDLBlender.hpp"

RMDLBlender::RMDLBlender(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, const std::string& resourcesPath, MTL::Library* pShaderLibrary) :
_pDevice(pDevice->retain()),
_pPixelFormat(pPixelFormat), _pDepthPixelFormat(pDepthPixelFormat),
_pCurrentTime(0.0f), _pAnimationDuration(12.0f)
{
    _pUniformBufferBlender = _pDevice->newBuffer(sizeof(AnimatedSpriteBlender), MTL::ResourceStorageModeShared);
    doTheImportThing(resourcesPath);
    loadGlb(resourcesPath);
    createPipelineBlender(pShaderLibrary, pPixelFormat, pDepthPixelFormat);
    printf("   Pipeline: %p\n", _pPipelineStateBlender);
    printf("   Textures: D=%p N=%p R=%p M=%p\n", _pDiffuseTexture, _pNormalTexture, _pRoughnessTexture, _pMetallicTexture);
    printf("✓ Device: %p\n", _pDevice);
    printf("✓ Shader library: %p\n", pShaderLibrary);
    printf("✓ GLB path exists: %s\n", resourcesPath.c_str());
    createSampler();
}

RMDLBlender::~RMDLBlender()
{
}

bool RMDLBlender::doTheImportThing(const std::string& resourcesPath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(resourcesPath + "/Test.glb", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

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

bool RMDLBlender::loadGlb(const std::string &resourcesPath)
{
    Assimp::Importer    importer;

    const aiScene* scene = importer.ReadFile(resourcesPath + "/Test.glb", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    loadMesh(scene);
    loadTextures(resourcesPath, scene);
    loadAnimation(scene);
    createBuffers();
    return true;
}

void RMDLBlender::createPipelineBlender(MTL::Library *pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat)
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
    renderPipelineDesc->colorAttachments()->object(0)->setPixelFormat(pPixelFormat);
    renderPipelineDesc->setDepthAttachmentPixelFormat(pDepthPixelFormat);

    NS::Error* pError = nullptr;
    _pPipelineStateBlender = _pDevice->newRenderPipelineState(renderPipelineDesc, &pError);

    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    _pDepthState = _pDevice->newDepthStencilState(depthDesc);

    renderPipelineDesc->release();
    vertexDesc->release();
    depthDesc->release();
    printf("✓ Pipeline created\n");
}

void RMDLBlender::updateBlender(float deltaTime)
{
    _pCurrentTime += deltaTime;
    if (_pCurrentTime > _pAnimationDuration)
        _pCurrentTime = fmod(_pCurrentTime, _pAnimationDuration);

    float t = _pCurrentTime / _pAnimationDuration;
    float bounce = sin(t * M_PI * 2.0f) * 0.5f;

    boneMatrix = simd::float4x4{
        simd::make_float4(1, 0, 0, 0),
        simd::make_float4(0, 1, 0, 0),
        simd::make_float4(0, 0, 1, 0),
        simd::make_float4(0, bounce, 0, 1)
    };
}

bool RMDLBlender::loadMesh(const aiScene *scene)
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
    printf("✓ Mesh loaded: %d vertices, %zu indices\n", mesh->mNumVertices, _pIndices.size());
    return true;
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

    MTL::Texture* texture = _pDevice->newTexture(desc);
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
    texture->replaceRegion(region, 0, data, width * 4);

    if (aiTexture->mHeight == 0)
        stbi_image_free(data);
    desc->release();

    return texture;
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

    MTL::Texture* texture = _pDevice->newTexture(desc);
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
    texture->replaceRegion(region, 0, data, width * 4);

    stbi_image_free(data);
    desc->release();

    return texture;
}

bool RMDLBlender::loadTextures(const std::string& resourcesPath, const aiScene *scene)
{
    aiMaterial* material = scene->mMaterials[0];
    aiString texPath;
    material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
    _pDiffuseTexture = loadTexture(resourcesPath, texPath.C_Str(), scene, true);
    material->GetTexture(aiTextureType_NORMALS, 0, &texPath);
    _pNormalTexture = loadTexture(resourcesPath, texPath.C_Str(), scene, false);
    material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath);
    _pRoughnessTexture = loadTexture(resourcesPath, texPath.C_Str(), scene, false);
    material->GetTexture(aiTextureType_METALNESS, 0, &texPath);
    return true;
}

void RMDLBlender::loadAnimation(const aiScene *scene)
{
    aiAnimation* anim = scene->mAnimations[0];
    _pAnimationDuration = anim->mDuration / anim->mTicksPerSecond;
    printf("✓ Animation loaded: %.2fs\n", _pAnimationDuration);
}

void RMDLBlender::createBuffers()
{
    _pVertexBufferBlender = _pDevice->newBuffer(_pVertices.data(), _pVertices.size() * sizeof(VertexBlender), MTL::ResourceStorageModeShared);
    _pIndexBufferBlender = _pDevice->newBuffer(_pIndices.data(), _pIndices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
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
    _pSampler = _pDevice->newSamplerState(samplerDesc);
    samplerDesc->release();
}

void RMDLBlender::drawBlender(MTL::RenderCommandEncoder *pEncoder, const simd::float4x4 &viewProjectionMatrix, const simd::float4x4 &model)
{
    AnimatedSpriteBlender* uniforms = (AnimatedSpriteBlender *)_pUniformBufferBlender->contents();
    uniforms->modelMatrix = model;
    uniforms->viewProjectionMatrix = viewProjectionMatrix;
    uniforms->boneMatrices[0] = boneMatrix;
    uniforms->time = _pCurrentTime;

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
//    pEncoder->setFragmentTexture(_pMetallicTexture, 3);
    pEncoder->setFragmentSamplerState(_pSampler, 0);
    pEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, _pIndices.size(), MTL::IndexTypeUInt32, _pIndexBufferBlender, 0);
}
