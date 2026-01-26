/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLRendererSpammy.cpp            +++    +++   */
/*                                        +       +          */
/*      By: Laboitederemdal                +       +         */
/*                                       +           +       */
/*      Created: 27/10/2025 18:44:15      + + + + + +        */
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define MTLFX_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#include <MetalFX/MetalFX.hpp>
#include <MetalKit/MetalKit.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include <simd/simd.h>
#include <utility>
#include <variant>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <thread>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>

#include "RMDLRendererSpammy.hpp"
#include "RMDLUtilities.h"

#define kMaxFramesInFlight 3

#define NUM_ELEMS(arr) (sizeof(arr) / sizeof(arr[0]))

static constexpr uint32_t kGridWidth = 256;
static constexpr uint32_t kGridHeight = 256;
static constexpr uint32_t kCellSize = 4;

const simd_float4 red = { 1.0, 0.0, 0.0, 1.0 };
const simd_float4 green = { 0.0, 1.0, 0.0, 1.0 };
const simd_float4 blue = { 0.0, 0.0, 1.0, 1.0 };

void triangleRedGreenBlue(float radius, float rotationInDegrees, TriangleData *triangleData)
{
    const float angle0 = (float)rotationInDegrees * M_PI / 180.0f;
    const float angle1 = angle0 + (2.0f * M_PI  / 3.0f);
    const float angle2 = angle0 + (4.0f * M_PI  / 3.0f);

    simd_float2 position0 = {
        30 * cosf(angle0),
        radius * sinf(angle0)
    };

    simd_float2 position1 = {
        radius * cosf(angle1),
        radius * sinf(angle1)
    };

    simd_float2 position2 = {
        radius * cosf(angle2),
        radius * sinf(angle2)
    };

    triangleData->vertex0.color = red;
    triangleData->vertex0.position = position0;

    triangleData->vertex1.color = green;
    triangleData->vertex1.position = position1;

    triangleData->vertex2.color = blue;
    triangleData->vertex2.position = position2;
}

void configureVertexDataForBuffer(long rotationInDegrees, void *bufferContents)
{
    const short radius = 350;
    const short angle = rotationInDegrees % 360;

    TriangleData triangleData;
    triangleRedGreenBlue(radius, (float)angle, &triangleData);

    ft_memcpy(bufferContents, &triangleData, sizeof(TriangleData));
}

float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

GameCoordinator::GameCoordinator(MTL::Device* device,
                                 MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat,
                                 NS::UInteger width, NS::UInteger height,
                                 const std::string& resourcePath)
: m_device(device->retain()), m_shaderLibrary(m_device->newDefaultLibrary()), m_commandQueue(m_device->newCommandQueue()),
m_pixelFormat(layerPixelFormat), m_depthPixelFormat(depthPixelFormat),
_rotationAngle(0.0f), m_frame(0),
m_editMode(false), m_buildMode(true),
blender(m_device, layerPixelFormat, m_depthPixelFormat, resourcePath, m_shaderLibrary),
skybox(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
world(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
ui(m_device, layerPixelFormat, m_depthPixelFormat, width, height, m_shaderLibrary),
colorsFlash(device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
mouseAndCursor(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
grid(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
blackHole(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
gridCommandant(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
m_terraVehicle(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
vertexBuffer(nullptr), indexBuffer(nullptr), transformBuffer(nullptr), m_gamePlayMode(GamePlayMode::Building),
m_inventoryPanel(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary),
planet(1e12f, 1000.0f, simd::float3{0, -1000, 0}),
moon(1e10f, 100.0f, simd::float3{500, 200, 0}),
sea(2000.0f, simd::float3{0, 0, 0}, -5.0f),
terrainLisse(m_device, 89),
blocs(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary, resourcePath, m_commandQueue)
{
    m_viewportSizeBuffer = m_device->newBuffer(sizeof(m_viewportSize), MTL::ResourceStorageModeShared);
    AAPL_PRINT("NS::UIntegerMax = " + std::to_string(NS::UIntegerMax));
    _pAudioEngine = std::make_unique<PhaseAudio>(resourcePath);
    loadGameSounds(resourcePath, _pAudioEngine.get());
    cursorPosition = simd::make_float2(0, 0);
    resizeMtkViewAndUpdateViewportWindow(width, height);
    m_camera.initPerspectiveWithPosition({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 1.8f, 1.0f, 0.6f, 50000.0f);
    m_cameraPNJ.initPerspectiveWithPosition({0.0f, 89.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 1.8f, 1.0f, 1.0f, 250.0f);
    m_cameraOrtho.initParallelWithPosition({20000.0f, 8900.0f, 0.f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 1.8f, 1.0f, 1.0f, 250.0f);
    printf("%lu\n%lu\n%lu\n", sizeof(simd_float2), sizeof(simd_uint2), sizeof(simd::float4x4));
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(layerPixelFormat, height, width, false);
    texDesc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    m_terrainTexture = m_device->newTexture(texDesc);
    
    size_t character = blender.loadModel(resourcePath + "/rosée.glb", "player");
    size_t tree = blender.loadModel(resourcePath + "/all.glb", "plane");
    blender.getModel("player")->transform = math::makeTranslate({-100, 60, 20});
    blender.getModel("plane")->transform = math::makeTranslate({5, 20, 0});
    
    
    world.setBiomeGenerator(std::make_unique<BiomeGenerator>(89));
    
    printf("%f\n%f\n%f\n", fade(0.1), fade(1.1), fade(2.7));
    
    uint64_t seed = 89;
//    _renderSystem = std::make_unique<RenderSystem>(m_device, layerPixelFormat, depthPixelFormat, width, height, resourcePath);
//    _terrainManager = std::make_unique<TerrainManager>(m_device, seed);
//    _physicsSystem = std::make_unique<PhysicsSystem>(_terrainManager.get());
    
    NS::Date* date = NS::Date::dateWithTimeIntervalSinceNow(0);
    m_uniforms.frameTime = 0.f;
    makeTexture(layerPixelFormat, depthPixelFormat);
    for (uint8_t i = 0; i < kMaxFramesInFlight; i++)
    {
        m_gpuUniforms[i] = m_device->newBuffer(sizeof(m_uniforms), MTL::ResourceStorageModeShared);
    }
    createBuffers();
    
    
    grid.setGridSize(16);           // 16x16 cellules
    grid.setCellSize(1.0f);         // 1 unité par cellule
    grid.setEdgeColor({0.3f, 0.8f, 1.0f, 0.8f}); // Bleu cyan
    grid.setEdgeThickness(0.02f);   // Épaisseur des lignes
    grid.setFadeDistance(50.0f);    // Distance de fade
//    grid.setVisible(false);
    
    blackHole.setPosition({0.f, 80.f, -150.f});
    blackHole.setRadius(10.f);
    blackHole.setAccretionDiskRadii(15.f, 50.f);
    
    gridCommandant.setCellSize(1.0f);      // 1 unité = 1 bloc
    gridCommandant.setGridExtent(3);       // 7x7x7 zone de construction
    gridCommandant.setLineThickness(0.025f);
    gridCommandant.setFadeDistance(30.f);
    
    // Couleurs par axe (RGB = XYZ)
    gridCommandant.setColorXY({0.3f, 0.5f, 1.0f, 0.7f}); // Bleu - avant/arrière
    gridCommandant.setColorXZ({0.3f, 1.0f, 0.5f, 0.7f}); // Vert - haut/bas
    gridCommandant.setColorYZ({1.0f, 0.5f, 0.3f, 0.7f}); // Rouge - gauche/droite
    
    gridCommandant.setVisible(true);
    
    m_spaceAudio = std::make_unique<SpaceshipAudioEngine>();
    m_spaceAudio->start();
    
    // Test : ajoute un moteur de base
    int engineVoice = m_spaceAudio->synth().addVoice(BlockPresets::Engine());
    m_spaceAudio->synth().triggerVoice(engineVoice, 1.0f);

//    vehicleManager.initialize(m_device, layerPixelFormat, depthPixelFormat, m_shaderLibrary);
//    inventoryUI.initialize(m_device);
//    if (vehicleManager.activeVehicle)
//        vehicleManager.activeVehicle->position = {0.0f, 5.0f, 0.0f};
    
//    m_inventoryPanel.setSlotData(1, 2, 20, {0.5f, 0.5f, 0.55f, 1.f});
//    m_inventoryPanel.setSlotData(2, 3, 4, {0.8f, 0.4f, 0.2f, 1.f});
    
    configLisse.seed = seed;
    configLisse.flatRadius = 50.0f;      // 50 unités de zone plate
    configLisse.maxHeight = 30.0f;       // max 30 unités de hauteur
    configLisse.flatness = 0.7f;         // assez plat globalement
    configLisse.center = simd::float2{0, 0};
    
    terrainLisse.setViewDistance(10);
    terrainLisse.setFlatRadius(100.0f);

    moon.setOrbit(500.0f, 0.1f);
    
    
    blocs.addBlock(cube::BlockType::CubeBasic, {0, 5, 0});
    blocs.addBlock(cube::BlockType::Blender, {1, 5, 0});
//    blocs.addBlock(cube::BlockType::CubeBasic, {1, 2, 0});
    blocs.addBlock(cube::BlockType::RobotHead, {0, 6, 0});
//    blocs.addBlock(cube::BlockType::WTF, {1, 5, 0});
    blocs.addBlock(cube::BlockType::WheelMedium, {-1, 5, 0}, 8);  // Rotation latérale
    blocs.addBlock(cube::BlockType::WheelMedium, {2, 5, 0}, 8);
    blocs.addBlock(cube::BlockType::Cockpit, {0, 5, -1});
    blocs.addBlock(cube::BlockType::ThrusterSmall, {0, 5, 1});
//    blocs.setBlockColor(1, {1.0f, 0.3f, 0.3f, 1.0f});
    auto* block = blocs.getBlockAt({1, 2, 0});
    if (block) {
        printf("Block OK: type=%d\n", (int)block->type);
    } else {
        printf("Block NOT FOUND!\n");
    }
}

GameCoordinator::~GameCoordinator()
{
    m_shaderLibrary->release();

    if (vertexBuffer) vertexBuffer->release();
    if (indexBuffer) indexBuffer->release();
    if (transformBuffer) transformBuffer->release();


    m_terraVehicle.cleanup();
    
    m_shaderLibrary->release();
    m_viewportSizeBuffer->release();
    m_renderPipelineState->release();
    m_depthStencilState->release();
    m_commandQueue->release();
    m_device->release();
}

void GameCoordinator::createBuffers()
{
    m_mouseBuffer = m_device->newBuffer(sizeof(VertexCursor), MTL::ResourceStorageModeShared);
}

void GameCoordinator::makeTexture(MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat)
{
    static const NS::UInteger width = 1024;
    MTL::TextureDescriptor* shadow = MTL::TextureDescriptor::texture2DDescriptor(depthPixelFormat, width, width, false);
    shadow->setTextureType(MTL::TextureType2DArray);
    shadow->setArrayLength(3);
    shadow->setStorageMode(MTL::StorageModePrivate);
    shadow->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    m_shadow = m_device->newTexture(shadow);
    m_shadow->setLabel(MTLSTR("ShadowMap"));

    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStateDesc = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStateDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthStateDesc->setDepthWriteEnabled(true);

    _shadowPassDesc = NS::TransferPtr(MTL::RenderPassDescriptor::alloc()->init());
    _shadowPassDesc->depthAttachment()->setTexture(m_shadow);
    _shadowPassDesc->depthAttachment()->setClearDepth(1.f);
    _shadowPassDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    _shadowPassDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);

    _shadowDepthState = m_device->newDepthStencilState(depthStateDesc.get());

    _gBufferPassDesc = NS::TransferPtr(MTL::RenderPassDescriptor::alloc()->init());
    _gBufferPassDesc->depthAttachment()->setClearDepth(1.0f);
    _gBufferPassDesc->depthAttachment()->setLevel(0);
    _gBufferPassDesc->depthAttachment()->setSlice(0);
    _gBufferPassDesc->depthAttachment()->setTexture(m_shadow);
    _gBufferPassDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    _gBufferPassDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);

    _gBufferPassDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionDontCare);
    _gBufferPassDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    _gBufferPassDesc->colorAttachments()->object(1)->setLoadAction(MTL::LoadActionDontCare);
    _gBufferPassDesc->colorAttachments()->object(1)->setStoreAction(MTL::StoreActionStore);

    depthStateDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthStateDesc->setDepthWriteEnabled(true);

    _gBufferDepthState = m_device->newDepthStencilState(depthStateDesc.get());

    _gBufferWithLoadPassDesc = NS::TransferPtr(MTL::RenderPassDescriptor::alloc()->init());
    _gBufferWithLoadPassDesc->depthAttachment()->setLoadAction(MTL::LoadActionLoad);
    _gBufferWithLoadPassDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    _gBufferWithLoadPassDesc->colorAttachments()->object(1)->setLoadAction(MTL::LoadActionLoad);

    _lightingPassDesc = NS::TransferPtr(MTL::RenderPassDescriptor::alloc()->init());
    _lightingPassDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    _lightingPassDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    _lightingPassDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.1, 0.15, 0.2, 1.0));

    depthStateDesc->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthStateDesc->setDepthWriteEnabled(false);
    _lightingDepthState = m_device->newDepthStencilState(depthStateDesc.get());
    
    NS::SharedPtr<MTL::RenderPipelineDescriptor> pRenderPipDesc = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pRenderPipDesc->setLabel(MTLSTR("Lighting"));
    
    NS::SharedPtr<MTL::Function> vertexFunction = NS::TransferPtr(m_shaderLibrary->newFunction(MTLSTR("LightingVs")));
    NS::SharedPtr<MTL::Function> fragmentFunction = NS::TransferPtr(m_shaderLibrary->newFunction(MTLSTR("LightingPs")));

    pRenderPipDesc->setVertexFunction(vertexFunction.get());
    pRenderPipDesc->setFragmentFunction(fragmentFunction.get());
    
//    pRenderPipDesc->setRasterSampleCount(1);
    pRenderPipDesc->colorAttachments()->object(0)->setPixelFormat(layerPixelFormat);
//    pRenderPipDesc->setSampleCount(1);

    NS::Error* error = nullptr;
//    NS::SharedPtr<MTL::VertexDescriptor> pVertexDesc = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
//    pVertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat4);
//    pVertexDesc->attributes()->object(0)->setOffset(0);
//    pVertexDesc->attributes()->object(0)->setBufferIndex(0);
//    
//    pVertexDesc->layouts()->object(0)->setStride(sizeof(16));
//    pVertexDesc->layouts()->object(0)->setStepRate(1);
//    pVertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);

//    pRenderPipDesc->setVertexDescriptor(pVertexDesc.get());
    pRenderPipDesc->setDepthAttachmentPixelFormat(depthPixelFormat);
    m_lightingPpl = m_device->newRenderPipelineState(pRenderPipDesc.get(), &error);
    m_depthStencilState = m_device->newDepthStencilState(depthStencilDescriptor.get());
    
    simd::float4 initialMouseWorldPos = simd::float4{ 0.f, 0.f, 0.f, 0.f };
//    m_mouseBuffer = m_device->newBuffer(&initialMouseWorldPos, sizeof(initialMouseWorldPos), MTL::ResourceStorageModeManaged);
    // Store 1 byte buffer for the 3D mouse position after depth read and resolve
    NS::SharedPtr<MTL::ComputePipelineDescriptor> pPipStateDesc = NS::TransferPtr(MTL::ComputePipelineDescriptor::alloc()->init());
    pPipStateDesc->setThreadGroupSizeIsMultipleOfThreadExecutionWidth(true);

    NS::SharedPtr<MTL::Function> computeFunction = NS::TransferPtr(m_shaderLibrary->newFunction(MTLSTR("mousePositionUpdate")));
    m_mousePositionComputeKernel = m_device->newComputePipelineState(computeFunction.get(), 0, nullptr, &error);
    
    m_textureBuffer = m_device->newBuffer(sizeof(uint), MTL::ResourceStorageModeManaged);

}

void GameCoordinator::loadGameSounds(const std::string &resourcePath, PhaseAudio *pAudioEngine)
{
//    pAudioEngine->loadMonoSound(resourcesPath, "Test.mp3");
    pAudioEngine->loadStereoSound(resourcePath, "success.mp3");
    pAudioEngine->loadStereoSound(resourcePath, "Test.m4a");
}

void GameCoordinator::playSoundTestY()
{
    _pAudioEngine->playSoundEvent("Test.m4a");
}

void GameCoordinator::handleKeyPress(int key)
{
    if (key == 15) // R
    {
    }
}

void GameCoordinator::inventory(bool visible)
{
    grid.setVisible(visible);
}

void GameCoordinator::jump()
{
    float _angle = 0;
    const float scl = 0.2f;
    _angle += 0.002f;
    simd::float3 objectPosition = { 0.f, 10.f, 0.f };
    simd::float4x4 scale = math::makeScale( (simd::float3){ scl, scl, scl } );
    simd::float4x4 rt = math::makeTranslate( objectPosition );
    simd::float4x4 rr1 = math::makeYRotate( -_angle );
    simd::float4x4 rr0 = math::makeXRotate( _angle * 0.5 );
    simd::float4x4 rtInv = math::makeTranslate( { -objectPosition.x, -objectPosition.y, -objectPosition.z } );
    simd::float4x4 zrot = math::makeZRotate( _angle * sinf(1.0));
    simd::float4x4 yrot = math::makeYRotate( _angle * cosf(1.0));
    simd::float4x4 fullObjectRot = rt * rr1 * rr0 * rtInv;
    
    simd::float3 jump = m_camera.position() + objectPosition;
    simd::float3 jumpback = m_camera.position() - objectPosition;
    m_camera.setPosition(jump);
//    m_camera.setPosition(jumpback);
//    setPosition(math::makeTranslate(jump));
}

void GameCoordinator::moveCamera(simd::float3 translation)
{
    newPosition = m_camera.position() + translation; // * cursorPosition.y
    m_camera.setPosition(newPosition);
}

void GameCoordinator::rotateCamera(float deltaYaw, float deltaPitch)
{
    m_camera.rotateOnAxis({0.0f, 1.0f, 0.0f}, deltaYaw);
    m_camera.rotateOnAxis(m_camera.right(), deltaPitch);
}

void GameCoordinator::setInventory()
{
    m_inventoryPanel.setVisible(!m_inventoryPanel.isVisible());
    if (testTransitionCamera == 0)
        m_camera.transitionTo(m_cameraPNJ, 1.5f, RMDLCameraEase::EaseInOutCubic);
    if (testTransitionCamera == 1)
        m_camera.transitionTo(m_cameraOrtho, 1.5f, RMDLCameraEase::Linear);
    if (testTransitionCamera == 2)
        m_camera.transitionTo(m_cameraPNJ, 1.5f, RMDLCameraEase::SmoothStep);
    if (testTransitionCamera == 3)
        m_camera.transitionTo(m_cameraPNJ, 15.f, RMDLCameraEase::SmootherStep);
    if (testTransitionCamera == 4)
        m_camera.transitionTo(m_cameraOrtho, 15.f, RMDLCameraEase::EaseInQuad);
    if (testTransitionCamera == 5)
        m_camera.transitionTo(m_cameraPNJ, 1.5f, RMDLCameraEase::EaseOutQuad);
    if (testTransitionCamera == 6)
        m_camera.transitionTo(m_cameraPNJ, 5.f, RMDLCameraEase::EaseInOutQuad);
    if (testTransitionCamera == 7)
        m_camera.transitionTo(m_cameraPNJ, 120.f, RMDLCameraEase::EaseInCubic);
    if (testTransitionCamera == 8)
        m_camera.transitionTo(m_cameraPNJ, 15.f, RMDLCameraEase::EaseOutCubic);
    if (testTransitionCamera == 9)
        m_camera.transitionTo(m_cameraPNJ, 5.5f, RMDLCameraEase::EaseInOutBack);
    testTransitionCamera++;
    if (testTransitionCamera == 10)
    {
        m_camera.transitionTo(cinematicView, 15.0f, RMDLCameraEase::SmootherStep);
        testTransitionCamera = 0;
        m_cameraOrtho.initPerspectiveWithPosition({-300.0f, 89.0f, 45.0f}, {10.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 1.8f, 1.0f, 1.0f, 250.0f);
    }
}

void GameCoordinator::setGamePlayMode(GamePlayMode mode)
{
    m_gamePlayMode = mode;
    
    if (mode == GamePlayMode::Building)
        m_terraVehicle.toggleBuildMode();
    else if (m_terraVehicle.isBuildMode())
        m_terraVehicle.toggleBuildMode();
}

void GameCoordinator::toggleVehicleBuildMode()
{
    if (m_gamePlayMode == GamePlayMode::Building)
        m_gamePlayMode = GamePlayMode::Driving;
    else
        m_gamePlayMode = GamePlayMode::Building;
    m_terraVehicle.toggleBuildMode();
}

void GameCoordinator::selectVehicleSlot(int slot)
{
    m_terraVehicle.selectInventorySlot(slot);
}

void GameCoordinator::rotateVehicleGhost()
{
    m_terraVehicle.rotateGhostBlock();
}

void GameCoordinator::vehicleMouseDown(bool rightClick)
{
    simd::float2 normPos = {
        cursorPosition.x / (float)m_viewport.width,
        1.f - (cursorPosition.y / (float)m_viewport.height)
    };
    simd::float2 screenSize = {(float)m_viewport.width, (float)m_viewport.height};
    m_terraVehicle.onMouseDown(normPos, screenSize, rightClick);
}

void GameCoordinator::vehicleMouseUp()
{
    simd::float2 normPos = {
        cursorPosition.x / (float)m_viewport.width,
        1.f - (cursorPosition.y / (float)m_viewport.height)
    };
    simd::float2 screenSize = {(float)m_viewport.width, (float)m_viewport.height};
    m_terraVehicle.onMouseUp(normPos, screenSize);
}

void GameCoordinator::handleMouseDown(bool rightClick)
{
    m_inventoryPanel.onMouseDown(cursorPosition, simd::make_float2(m_viewport.width, m_viewport.height));
}

void GameCoordinator::handleMouseUp()
{
    m_inventoryPanel.onMouseUp(cursorPosition, simd::make_float2(m_viewport.width, m_viewport.height));
}

void GameCoordinator::handleScroll(float deltaY)
{
    if (m_gamePlayMode == GamePlayMode::Driving || m_gamePlayMode == GamePlayMode::Building || m_gamePlayMode == GamePlayMode::DEV)
        m_terraVehicle.zoomCamera(deltaY * 0.5f);
}

void GameCoordinator::setMousePosition(float x, float y)
{
    cursorPosition = simd::make_float2(x, y);
    mouseAndCursor.setMousePosition(x, y);
    m_inventoryPanel.onMouseMoved(simd::make_float2(cursorPosition.x, cursorPosition.y), simd::make_float2(m_viewport.width, m_viewport.height));
}

void GameCoordinator::createCursor(float mouseX, float mouseY, float width, float height, float size, std::vector<VertexCursor> &outVertices)
{
    float x = (mouseX / width) * 2.0f - 1.0f;
    float y = 1.0f - (mouseY / height) * 2.0f;

    float sX = size / width * 2.0f;
    float sY = size / height * 2.0f;

    outVertices = {
        {{x - sX, y}}, {{x + sX, y}},   // ligne horizontale
        {{x, y - sY}}, {{x, y + sY}}    // ligne verticale
    };
}

void GameCoordinator::update(float dt, const InputState& input)
{
//    constexpr float baseMoveSpeed = 5.0f;  // unités/seconde
//    constexpr float lookSensitivity = 0.003f;
//    
//    // Mouvement caméra (frame-independent)
//    float speed = baseMoveSpeed * input.speedMultiplier * dt;
//    simd::float3 movement = input.moveDirection * speed;
//    
//    // Transformer en espace caméra
//    simd::float3 forward = m_camera.forward();
//    simd::float3 right = m_camera.right();
//    simd::float3 up = {0, 1, 0};
//    
//    simd::float3 worldMove = right * movement.x + up * movement.y + forward * movement.z;
//    
//    m_camera.setPosition(worldMove);
//    
//    // Rotation caméra
//    m_camera.rotateOnAxis(-input.lookDelta.x * lookSensitivity, -input.lookDelta.y * lookSensitivity);
    switch (m_gamePlayMode)
    {
        case GamePlayMode::FreeCam:
        {
            // Ton code existant de caméra libre
            constexpr float baseMoveSpeed = 5.0f;
            constexpr float lookSensitivity = 0.003f;
            
            float speed = baseMoveSpeed * input.speedMultiplier * dt;
            simd::float3 movement = input.moveDirection * speed;
            
            simd::float3 forward = m_camera.forward();
            simd::float3 right = m_camera.right();
            simd::float3 up = {0, 1, 0};
            
            simd::float3 worldMove = right * movement.x + up * movement.y + forward * movement.z;
            m_camera.setPosition(m_camera.position() + worldMove);
            m_camera.rotateOnAxis({0,1,0}, -input.lookDelta.x * lookSensitivity);
            m_camera.rotateOnAxis(m_camera.right(), -input.lookDelta.y * lookSensitivity);
            break;
        }
            
        case GamePlayMode::Driving:
        {
            // Contrôle du véhicule
            m_terraVehicle.setThrottle(-input.moveDirection.z);  // Z avant, S arrière
            m_terraVehicle.setSteering(-input.moveDirection.x);  // Q gauche, D droite
            m_terraVehicle.setBrake(input.moveDirection.y < 0 ? 1.f : 0.f);
            
            // Orbite caméra avec souris
            m_terraVehicle.orbitCamera(-input.lookDelta.x * 0.005f,
                                        -input.lookDelta.y * 0.005f);
            break;
        }
            
        case GamePlayMode::Building:
        {
            // Mode construction - orbite seulement
            m_terraVehicle.orbitCamera(-input.lookDelta.x * 0.005f,
                                        -input.lookDelta.y * 0.005f);
            
            // Ray pour placement de bloc (depuis la caméra vers le curseur)
            simd::float3 camPos = m_terraVehicle.getCameraPosition();
            simd::float3 camTarget = m_terraVehicle.getCameraTarget();
            simd::float3 rayDir = simd::normalize(camTarget - camPos);
            
            simd::float2 normMouse = {
                cursorPosition.x / (float)m_viewport.width,
                1.f - (cursorPosition.y / (float)m_viewport.height)
            };
            simd::float2 screenSize = {(float)m_viewport.width, (float)m_viewport.height};
            
            m_terraVehicle.onMouseMove(normMouse, screenSize, camPos, rayDir);
            break;
        }
    }

    m_terraVehicle.update(dt);
    float throttle = simd::length(input.moveDirection);
    m_spaceAudio->setEngineThrottle(throttle);
    
    moon.updateOrbit(dt);
    
    m_camera.updateTransition(dt);
//    if (!m_camera.isTransitioning())
//        m_camera.applyShake(9.0f, 13.f);
//        m_camera.applyShake(5.f, 8.f, 250.f);
        // Contrôles joueur actifs seulement hors transition
//        m_camera.rotateYawPitch(snappedPos.x, snappedPos.y);
    
    
//    planet->applyGravityTo(m_vehicleRigidBody);
//    if (moon->isOnSurface(m_vehicleRigidBody.position, 50.0f)) {
//        moon->applyGravityTo(m_vehicleRigidBody);
//    }
    blocs.update(dt);
}

void GameCoordinator::addBlockToVehicle(int blockId, BlockType type)
{
//    BlockSoundProfile profile;
//    switch (type) {
//        case BlockType::Engine:    profile = BlockPresets::Engine(); break;
//        case BlockType::Thruster:  profile = BlockPresets::Thruster(); break;
//        case BlockType::Generator: profile = BlockPresets::Generator(); break;
//        case BlockType::Shield:    profile = BlockPresets::Shield(); break;
//        case BlockType::Weapon:    profile = BlockPresets::Weapon(); break;
//        case BlockType::Cockpit:   profile = BlockPresets::Cockpit(); break;
//    }
    
//    int voiceId = m_spaceAudio->synth().addVoice(profile);
//    m_spaceAudio->synth().triggerVoice(voiceId, 0.8f);
//    m_blockVoices[blockId] = voiceId;
}

void GameCoordinator::removeBlockFromVehicle(int blockId)
{
    if (m_blockVoices.count(blockId)) {
        m_spaceAudio->synth().releaseVoice(m_blockVoices[blockId]);
        m_blockVoices.erase(blockId);
    }
}

void GameCoordinator::updateUniforms()
{
    m_cameraUniforms = m_camera.uniforms();
    
    m_uniforms.cameraUniforms = m_camera.uniforms();
    m_uniforms.mouseState = simd::float3{ cursorPosition.x, cursorPosition.y, float(1.0f) };
    m_uniforms.invScreenSize = simd::float2{ 1.0f / m_depth->width(), 1.0f / m_depth->width() };
    m_uniforms.projectionYScale = 1.73205066;
    m_uniforms.brushSize = 10.0f;

    // Set up shadows
    //  - to cover as much of our frustum with shadow volumes, we create a simplified 1D model along the frustum center axis.
    //  Our frustum is reduced to a gradient that coincides with our tan(view angle/2). The three shadow volumes are modeled as a three circles
    //  that are packed under the gradient, so that all space is overlapped at least once. We then project the three circles back to 3D spheres
    //  and wrap them in 3 shadow volumes (parallel camera volumes) by constructing an oriented box around them
    {
//        const float3 sunDirection       = normalize ((float3) {1,-0.7,0.5});

        // Extend view angle to the angle of the corners of the frustum to get the cone angle; we use half-angles in the math
        float tan_half_angle = tanf(m_camera.viewAngle() * .5f) * sqrtf(2.0);
        float half_angle = atanf(tan_half_angle);
        float sine_half_angle = sinf(half_angle);

        // Define three bounding spheres that cover/fill the view cone. These can be optimized by angle, etc., for nice coverage without over spending shadow map space
        float cascade_sizes[3] = {400.0f, 1600.0f, 6400.0f };

        // Now the centers of the cone in distance to camera can be calulated
        float cascade_distances[3];
        cascade_distances[0] = 2 * cascade_sizes[0] * (1.0f - sine_half_angle * sine_half_angle);
        cascade_distances[1] = sqrtf(cascade_sizes[1]*cascade_sizes[1] - cascade_distances[0]*cascade_distances[0]*tan_half_angle*tan_half_angle) + cascade_distances[0];
        cascade_distances[2] = sqrtf(cascade_sizes[2]*cascade_sizes[2] - cascade_distances[1]*cascade_distances[1]*tan_half_angle*tan_half_angle) + cascade_distances[1];

        for (uint c = 0; c < 3; c++)
        {
            // Center of sun cascade back-plane
            simd::float3 center = m_camera.position() + m_camera.direction() * cascade_distances[c];
            float size = cascade_sizes[c];

            // Stepsize is some multiple of the texel size
            float stepsize = size/64.0f;
//            m_cameraShadow.initPerspectiveWithPosition({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 1.8f, 1.0f, 0.1f, 250.0f);
//          initParallelWithPosition:center-sunDirection*size direction:sunDirection up:(float3){ 0, 1, 0} width:size*2.0f height:size*2.0f nearPlane:0.0f farPlane:size*2];
//            m_cameraShadow.position -= simd::fract(simd::dot(center, m_cameraShadow.up()) / stepsize) * m_cameraShadow.up() * stepsize;
//            shadow_cam.position -= fract(dot(center, shadow_cam.right) /stepsize) * shadow_cam.right * stepsize;

//            _uniforms_cpu.shadowCameraUniforms[c] = shadow_cam.uniforms;
        }
    }
}

void GameCoordinator::draw(MTK::View* view)
{
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer* commandBuffer = m_commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* passDesc = view->currentRenderPassDescriptor();
    passDesc->depthAttachment()->setTexture(m_depth.get());
    passDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    passDesc->depthAttachment()->setClearDepth(1.0f);
    passDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    m_frame += 1;
    float dt = 0.016f;
    const uint32_t frameIndex = m_frame % kMaxFramesInFlight;
//    passDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.1, 0.15, 0.2, 1.0));
    
    
    terrainLisse.update(m_camera.position(), commandBuffer);
    
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(passDesc);
    renderCommandEncoder->setViewport(m_viewport);
    
    simd::float3 vehicleCamPos = m_terraVehicle.getCameraPosition();
    simd::float3 vehicleCamTarget = m_terraVehicle.getCameraTarget();

    _rotationAngle += 0.0002f;
    if (_rotationAngle > 2 * M_PI)
    {
        _rotationAngle -= 2 * M_PI;
    }
    simd::float4x4 modelMatrixRot = {
        simd::float4{ cosf(_rotationAngle), 0.0f, sinf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 1.0f, 0.0f, 0.0f },
        simd::float4{ -sinf(_rotationAngle), 0.0f, cosf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 60.0f, 5.0f, 1.0f }
    };
    
    updateUniforms();
    update(dt, m_input);
    
    renderCommandEncoder->setVertexBytes(&m_cameraUniforms, sizeof(RMDLCameraUniforms), 1);
    renderCommandEncoder->setFragmentBytes(&m_cameraUniforms, sizeof(RMDLCameraUniforms), 1);
    
//    blender.drawBlender(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix * modelMatrixRot, modelMatrixRot); // matrix_identity_float4x4
    blender.draw(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix);
    blender.updateBlender(dt);

    skybox.render(renderCommandEncoder, math::makeIdentity(), m_cameraUniforms.viewProjectionMatrix * math::makeIdentity(), m_camera.position()); //{0,0,0});
//    snow.render(enc, modelMatrix2, {0,0,0});
//    skybox.updateUniforms(modelMatrix, cameraUniforms.viewProjectionMatrix * modelMatrix, {0,0,0});
    blackHole.update(0.016f);
    blackHole.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix, m_cameraUniforms.invViewProjectionMatrix, m_camera.position());
    
    renderCommandEncoder->setVertexBytes(&m_cameraUniforms, sizeof(RMDLCameraUniforms), 1);
    renderCommandEncoder->setFragmentBytes(&m_cameraUniforms, sizeof(RMDLCameraUniforms), 1);


//    world.update(dt, m_camera.position(), m_device);
//    world.updateTime(dt);
//    world.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix);

    gridCommandant.update(0.016f);
    gridCommandant.setBlockPosition(m_currentVehicleBlockPos);
    gridCommandant.setBlockRotation(m_currentVehicleRotation);
    gridCommandant.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix, m_camera.position());
    
//    grid.setGridCenter({0.0f, 0.0f, 0.0f});
    grid.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix, m_camera.position());
    
//    m_terraVehicle.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix, m_camera.position());

    m_cameraUniforms.position = m_camera.position();
    
    
    dt += 0.016f;
    
    
    
    ui.beginFrame(m_viewport.width, m_viewport.height);
    ui.drawText("Hello 89 ! Make sense", 550, 50, 0.5);
//    colorsFlash.renderPostProcess(renderCommandEncoder);
    
//    m_terrainSystem->update(dt, m_camera.position());

//    m_terrainSystem->render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix);


    ui.endFrame(renderCommandEncoder);


//    _lightingPassDesc->colorAttachments()->object(0)->setTexture(passDesc->colorAttachments()->object(0)->texture());
//    renderCommandEncoder->endEncoding();
//    renderCommandEncoder = commandBuffer->renderCommandEncoder(_lightingPassDesc.get());
//    renderCommandEncoder->setRenderPipelineState(m_lightingPpl);
//    renderCommandEncoder->setDepthStencilState(_lightingDepthState);
//    renderCommandEncoder->setFragmentTexture(m_gBuffer0, 0);
//    renderCommandEncoder->setFragmentTexture(m_gBuffer1, 1);
//    renderCommandEncoder->setFragmentTexture(m_depth, 2);
//    renderCommandEncoder->setFragmentTexture(m_shadow, 3);
//    renderCommandEncoder->setFragmentBuffer(m_gpuUniforms[frameIndex], 0, 0);
//    renderCommandEncoder->setFragmentBuffer(m_mouseBuffer, 0, 1);
//    renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, 0UL, 3UL); // NS::UInt

    m_inventoryPanel.render(renderCommandEncoder, screenSz);


    terrainLisse.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix);
    
    blocs.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix, m_camera.position(), dt);
    
    
    renderCommandEncoder->endEncoding();

    
    mouseAndCursor.setInverseViewProjection(m_cameraUniforms.invViewProjectionMatrix);
    
    mouseAndCursor.pick(commandBuffer, m_depth.get());
    
//    uint* ptr = reinterpret_cast<uint*>(m_textureBuffer->contents());
//    m_textureBuffer->didModifyRange(NS::Range::Make(0, sizeof(uint)));
//    MTL::ComputeCommandEncoder* computeCommandEncoder = commandBuffer->computeCommandEncoder();
//    computeCommandEncoder->setComputePipelineState(m_mousePositionComputeKernel);
//    computeCommandEncoder->setTexture(m_depth, 0);
//    computeCommandEncoder->setBuffer(m_gpuUniforms[frameIndex], 0, 0);
//    computeCommandEncoder->setBuffer(m_mouseBuffer, 0, 1);
//    computeCommandEncoder->dispatchThreadgroups({ 1, 1, 1 }, { 64, 1, 1 });
//    computeCommandEncoder->endEncoding();
    
    
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
    MousePickResult result = mouseAndCursor.getResult();
    snappedPos.x = roundf(result.worldPosition.x);
    snappedPos.y = roundf(result.worldPosition.y);//0.0f;
    snappedPos.z = roundf(result.worldPosition.z);
    grid.setGridCenter(snappedPos);
    if (result.depth > 0.9)
    {
//        printf("%f\n", result.depth);
        grid.setFadeDistance(150.0f);
        grid.setEdgeThickness(0.07f);
    } else {
        grid.setFadeDistance(50.0f);
        grid.setEdgeThickness(0.02f);
    }
    if (frameIndex == 2)
    {
    }
    if (m_frame > 1000 || m_frame % 1000 == 0)
    {
//        printf("%llu\n", m_frame);
    }
    autoreleasePool->release();
}

void GameCoordinator::resizeMtkViewAndUpdateViewportWindow(NS::UInteger width, NS::UInteger height)
{
    setViewportWindow(width, height);
    
    screenSz = {(float)m_viewport.width, (float)m_viewport.height};

    m_depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(m_depthPixelFormat, width, height, false);
//    m_depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
    m_depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
//    m_depthTexture = m_device->newTexture(m_depthTextureDescriptor);
    MTL::TextureDescriptor* m_gTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(m_pixelFormat, width, height, false); // pas de ns shared
    m_gTextureDescriptor->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageRenderTarget);
    m_gTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
    m_gBuffer0 = m_device->newTexture(m_gTextureDescriptor);
    m_gBuffer1 = m_device->newTexture(m_gTextureDescriptor);
    m_camera.setAspectRatio(width / height);
    mouseAndCursor.setScreenSize((float)width, (float)height);
    m_depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    m_depth = NS::TransferPtr(m_device->newTexture(m_depthTextureDescriptor));
}

void GameCoordinator::setViewportWindow(NS::UInteger width, NS::UInteger height)
{
    m_viewport.originX = 0.0;
    m_viewport.originY = 0.0;
    m_viewport.width   = (double)width;
    m_viewport.height  = (double)height;
    m_viewport.znear   = 0.0;
    m_viewport.zfar    = 1.0;
    m_viewportSize.x = (float)width;
    m_viewportSize.y = (float)height;
    memcpy(m_viewportSizeBuffer->contents(), &m_viewportSize, sizeof(m_viewportSize));
}



















RMDLRendererSpammy::RMDLRendererSpammy(MTL::Device* pDevice, MTL::PixelFormat pixelFormat, NS::UInteger width, NS::UInteger height, const std::string& resourcePath )
: _pDevice(pDevice->retain()), _pixelFormat(pixelFormat), _frame(0)
{
    _pCommandQueue = _pDevice->newMTL4CommandQueue();
    _pShaderLibrary = _pDevice->newDefaultLibrary();
    _pSharedEvent = _pDevice->newSharedEvent();
    _pSharedEvent->setSignaledValue(_currentFrameIndex);

    _semaphore = dispatch_semaphore_create( kMaxFramesInFlight );
}

RMDLRendererSpammy::~RMDLRendererSpammy()
{
    _pTexture->release();
    _pDepthTextureDesc->release();
    _pPSO->release();
    _pShaderLibrary->release();
    _pCommandQueue->release();
    _pResidencySet->release();
    _pArgumentTable->release();
    _pCommandQueue->release();
}

void RMDLRendererSpammy::loadPngAndFont( const std::string& resourcesPath )
{
//    auto pCommandQueue = NS::TransferPtr(_pDevice->newMTL4CommandQueue());
//    auto pCommandBuffer = _pDevice->newCommandBuffer();

    std::vector<std::string> sprite2dPng
    {
        
    };

    _textureAssets["jspencore.png"] = NS::TransferPtr(newTextureFromFile(resourcesPath + "/jspencore.png", _pDevice));
//    _fontAtlas = newFontAtlas(_pDevice);
//    _textureAssets["fontAtlas"] = _fontAtlas.texture;
//    assert(_textureAssets["fontAtlas"]);
}

void RMDLRendererSpammy::makeArgumentTable()
{
    NS::Error* pError = nullptr;

    NS::SharedPtr<MTL4::ArgumentTableDescriptor> argumentTableDescriptor = NS::TransferPtr( MTL4::ArgumentTableDescriptor::alloc()->init() );
//    argumentTableDescriptor->setMaxBufferBindCount(2);
    _pArgumentTable = _pDevice->newArgumentTable(argumentTableDescriptor.get(), &pError);
}

void RMDLRendererSpammy::draw( MTK::View* view )
{
    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

    _currentFrameIndex += 1;

    const uint32_t frameIndex = _currentFrameIndex % kMaxFramesInFlight;
    std::string label = "Frame: " + std::to_string(_currentFrameIndex);
    
    if (_currentFrameIndex > kMaxFramesInFlight)
    {
        uint64_t const timeStampToWait = _currentFrameIndex - kMaxFramesInFlight;
        _pSharedEvent->waitUntilSignaledValue(timeStampToWait, DISPATCH_TIME_FOREVER);
    }

    pPool->release();
}
