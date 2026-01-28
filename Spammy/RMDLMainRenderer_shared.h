//
//  Untitled.hpp
//  Spammy
//
//  Created by Rémy on 13/12/2025.
//

#ifndef RMDLMAINRENDERER_SHARED_H
#define RMDLMAINRENDERER_SHARED_H

#include <simd/simd.h>

#define GAME_TIME 1.1f

typedef struct
{
    simd_float2 position;
    simd_float4 color;
}   VertexData;

struct RMDLCameraUniforms
{
    simd::float4x4      viewMatrix;
    simd::float4x4      projectionMatrix;
    simd::float4x4      viewProjectionMatrix;
    simd::float4x4      invOrientationProjectionMatrix;
    simd::float4x4      invViewProjectionMatrix;
    simd::float4x4      invProjectionMatrix;
    simd::float4x4      invViewMatrix;
    simd::float4        frustumPlanes[6];
    simd::float3        position;
    float               padding;
};

struct RMDLUniforms
{
    RMDLCameraUniforms  cameraUniforms;
//    RMDLCameraUniforms  shadowCameraUniforms[3];
    simd::float3        mouseState;
    simd::float2        invScreenSize;
    float               projectionYScale;
    float               brushSize;

    float               ambientOcclusionContrast;
    float               ambientOcclusionScale;
    float               ambientLightScale;
    float               frameTime;
};

struct Plane
{
    simd::float3        normal = { 0.f, 1.f, 0.f };
    float               distance = 0.f;
};

struct Frustum
{
    Plane               topFace;
    Plane               bottomFace;
    Plane               rightFace;
    Plane               leftFace;
    Plane               farFace;
    Plane               nearFace;
};

struct RMDLObjVertex
{
    simd::float3    position;
    simd::float3    normal;
    simd::float3    color;
};

typedef enum VertexAttributes
{
    VertexAttributePosition  = 0,
    VertexAttributeTexcoord  = 1,
    VertexAttributeNormal    = 2,
    VertexAttributeTangent   = 3,
    VertexAttributeBitangent = 4
}   VertexAttributes;

typedef enum BufferIndex
{
    BufferIndexMeshPositions     = 0,
    BufferIndexMeshGenerics      = 1,
    BufferIndexFrameData         = 2,
    BufferIndexLightsData        = 3,
    BufferIndexLightsPosition    = 4,
    BufferIndexFlatColor         = 0,
    BufferIndexDepthRange        = 0,
}   BufferIndex;

typedef enum TextureIndex
{
    TextureIndexBaseColor = 0,
    TextureIndexSpecular  = 1,
    TextureIndexNormal    = 2,
    TextureIndexShadow    = 3,
    TextureIndexAlpha     = 4,
    NumMeshTextures = TextureIndexNormal + 1
}   TextureIndex;

typedef enum RenderTargetIndex
{
    RenderTargetLighting  = 0,
    RenderTargetAlbedo    = 1,
    RenderTargetNormal    = 2,
    RenderTargetDepth     = 3
}   RenderTargetIndex;

struct RMDLSun
{
    simd::float3 sunDirection;
};

struct RMDLSkyboxUniforms
{
    simd::float4x4 invViewProjection;
    simd::float3 cameraPos;
    float _pad0;
    RMDLSun rmdlSun;
    float sunIntensity;
    simd::float3 rayleighCoeff;
    float rayleighHeight;
    float mieCoeff;
    float mieHeight;
    float mieG;
    float planetRadius;
    float atmosphereRadius;
    float exposure;
    float timeOfDay;
    float _pad1;
};

namespace skybox {
struct BlackHoleUniforms
{
    simd::float4x4 viewProjectionMatrix;
    simd::float4x4 invViewProjectionMatrix;
    simd::float3   cameraPosition;
    float    time;
    simd::float3   blackHolePosition;
    float    blackHoleRadius;
    float    accretionDiskInnerRadius;
    float    accretionDiskOuterRadius;
    float    gravitationalStrength;
    float    rotationSpeed;
};

struct BlackHoleVertexIn
{
    simd_float3 position;
};
}

struct RMDLSnowUniforms
{
    float deltaTime;
    float time;
    simd::float3 cameraPos;
    float spawnRadius;
    float spawnHeight;
    float fallSpeed;
    simd::float3 windDirection;
    float windStrength;
    float turbulence;
    float respawnThreshold;
    float intensity;
    float _pad;
};

struct VertexRectangle
{
    simd::float2 position;
    simd::float4 color;
};

#pragma mark PNJ

struct VertexPNJ
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texCoord;
    simd::float3 tangent;
    simd::float3 bitangent;
    simd::float4 boneWeights;
    simd::uint4 boneIndices;
};

struct GridUniforms
{
    simd::float4x4 viewProjectionMatrix;
    simd::float3 cameraPosition;
    float gridSize;
    simd::float3 gridCenter;
    float cellSize;
    simd::float4 edgeColor;
    float edgeThickness;
    float fadeDistance;
    float padding[2];
};

namespace GridCommandant {
struct VehicleGridUniforms
{
    simd::float4x4 viewProjectionMatrix;
    simd::float4x4 modelMatrix;
    simd::float3   cameraPosition;
    float          time;
    simd::float3   gridCenter;
    float          cellSize;
    simd::float4   gridColorXY;
    simd::float4   gridColorXZ;
    simd::float4   gridColorYZ;
    float          lineThickness;
    float          fadeDistance;
    float          pulseIntensity;
    int32_t        gridExtent;
};

struct GridVertex3D {
    simd::float3 position;
    simd::float3 normal;
    simd::float2 uv;
    uint8_t      planeIndex; // 0=XY, 1=XZ, 2=YZ
    uint8_t      padding[3];
};
}

#endif /* RMDLMAINRENDERER_SHARED_H */
