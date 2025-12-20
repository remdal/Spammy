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
#include "stdio.h"

#include "RMDLUtils.hpp"

struct VertexBlender
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texCoord;
    simd::int4   joints;
    simd::float4 weights;
};

struct AnimatedSpriteBlender
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewProjectionMatrix;
    simd::float4x4 boneMatrices[3];
    float time;
};

class RMDLBlender
{
public:
    RMDLBlender(MTL::Device* pDevice, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat, const std::string& resourcesPath, MTL::Library* pShaderLibrary);
    ~RMDLBlender();

    bool doTheImportThing(const std::string& resourcesPath);
    bool loadGlb(const std::string& resourcesPath);
    void createPipelineBlender(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat);
    void updateBlender(float deltaTime);
    void drawBlender(MTL::RenderCommandEncoder* pEncoder, const simd::float4x4& viewProjectionMatrix, const simd::float4x4& model);
private:
    MTL::Device*                _pDevice;
    MTL::Buffer*                _pVertexBufferBlender;
    MTL::Buffer*                _pIndexBufferBlender;
    MTL::Buffer*                _pUniformBufferBlender;
    MTL::Texture*               _pDiffuseTexture = nullptr;
    MTL::Texture*               _pNormalTexture = nullptr;
    MTL::Texture*               _pRoughnessTexture = nullptr;
    MTL::Texture*               _pMetallicTexture = nullptr;
    MTL::PixelFormat            _pPixelFormat;
    MTL::PixelFormat            _pDepthPixelFormat;
    MTL::SamplerState*          _pSampler = nullptr;
    MTL::DepthStencilState*     _pDepthState;
    MTL::RenderPipelineState*   _pPipelineStateBlender;
    std::vector<VertexBlender>  _pVertices;
    std::vector<uint32_t>       _pIndices;
    float                       _pCurrentTime;
    float                       _pAnimationDuration;
    simd::float4x4              boneMatrix;

    bool loadMesh(const aiScene* scene);
    bool loadTextures(const std::string& resourcesPath, const aiScene* scene);
    MTL::Texture* loadTexture(const std::string& resourcesPath, const char* path, const aiScene* scene, bool sRGB);
    MTL::Texture* loadEmbeddedTexture(aiTexture* aiTexture, bool sRGB);
    void loadAnimation(const aiScene* scene);
    void createBuffers();
    void createSampler();
};

#endif /* RMDLBLENDER_HPP */
