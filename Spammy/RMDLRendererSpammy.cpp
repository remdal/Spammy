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
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); // 1,7 - > 9.03992
}

GameCoordinator::GameCoordinator(MTL::Device* device,
                                 MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat,
                                 NS::UInteger width, NS::UInteger height,
                                 const std::string& resourcePath)
: m_device(device->retain()), m_shaderLibrary(m_device->newDefaultLibrary()),
m_pixelFormat(layerPixelFormat), m_depthPixelFormat(depthPixelFormat),
_rotationAngle(0.0f), m_frame(0),
blender(m_device, layerPixelFormat, m_depthPixelFormat, resourcePath, m_shaderLibrary),
skybox(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
snow(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
world(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
ui(m_device, layerPixelFormat, m_depthPixelFormat, width, height, m_shaderLibrary),
colorsFlash(device, layerPixelFormat, depthPixelFormat, m_shaderLibrary)
{
    m_viewportSize.x = (float)width;
    m_viewportSize.x = (float)height;
    m_viewportSizeBuffer = m_device->newBuffer(sizeof(m_viewportSize), MTL::ResourceStorageModeShared);
    ft_memcpy(m_viewportSizeBuffer->contents(), &m_viewportSize, sizeof(m_viewportSize));
    AAPL_PRINT("NS::UIntegerMax = " + std::to_string(NS::UIntegerMax));
    m_commandQueue = m_device->newCommandQueue();
    _pAudioEngine = std::make_unique<PhaseAudio>(resourcePath);
    loadGameSounds(resourcePath, _pAudioEngine.get());
    cursorPos = simd::make_float2(0, 0);
    resizeMtkViewAndUpdateViewportWindow(width, height);
    m_camera.initPerspectiveWithPosition({0.0f, 30.0f, 5.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 3.0f, 1.0f, 0.1f, 150.0f);
//    m_cameraPNJ.initPerspectiveWithPosition(<#simd::float3 position#>, <#simd::float3 direction#>, <#simd::float3 up#>, <#float viewAngle#>, <#float aspectRatio#>, <#float nearPlane#>, <#float farPlane#>)
    printf("%lu\n%lu\n%lu\n", sizeof(simd_float2), sizeof(simd_uint2), sizeof(simd::float4x4));
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(layerPixelFormat, height, width, false);
    texDesc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    m_terrainTexture = m_device->newTexture(texDesc);
    world.setBiomeGenerator(std::make_unique<BiomeGenerator>(89));
    
    printf("%f\n%f\n%f\n", fade(0.1), fade(1.1), fade(2.7));
}

GameCoordinator::~GameCoordinator()
{
    m_shaderLibrary->release();
    

    vertexBuffer->release();
    indexBuffer->release();
    transformBuffer->release();


    
    m_shaderLibrary->release();
    m_viewportSizeBuffer->release();
    m_renderPipelineState->release();
    m_depthStencilState->release();
    m_commandQueue->release();
    m_device->release();
}

void GameCoordinator::moveCamera(simd::float3 translation)
{
    simd::float3 newPosition = m_camera.position() + translation;
    m_camera.setPosition(newPosition);
}

void GameCoordinator::rotateCamera(float deltaYaw, float deltaPitch)
{
    m_camera.rotateOnAxis({0.0f, 1.0f, 0.0f}, deltaYaw);
    m_camera.rotateOnAxis(m_camera.right(), deltaPitch);
}

void GameCoordinator::handleMouseDown(bool rightClick)
{
}

void GameCoordinator::handleMouseUp()
{
}

void GameCoordinator::handleScroll(float deltaY)
{
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

void GameCoordinator::draw(MTK::View* view)
{
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer* commandBuffer = m_commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* passDesc = view->currentRenderPassDescriptor();
//    passDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.1, 0.15, 0.2, 1.0));
    
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(passDesc);
    renderCommandEncoder->setViewport(m_viewport);

    _rotationAngle += 0.0002f;
    if (_rotationAngle > 2 * M_PI)
    {
        _rotationAngle -= 2 * M_PI;
    }
    simd::float4x4 modelMatrixRot = {
        simd::float4{ cosf(_rotationAngle), 0.0f, sinf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 1.0f, 0.0f, 0.0f },
        simd::float4{ -sinf(_rotationAngle), 0.0f, cosf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    m_cameraUniforms = m_camera.uniforms();
    blender.drawBlender(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix * modelMatrixRot, modelMatrixRot); // matrix_identity_float4x4
    blender.updateBlender(0.0f);

    skybox.render(renderCommandEncoder, math::makeIdentity(), m_cameraUniforms.viewProjectionMatrix * math::makeIdentity(), m_camera.position()); //{0,0,0});
//    snow.render(enc, modelMatrix2, {0,0,0});
//    skybox.updateUniforms(modelMatrix, cameraUniforms.viewProjectionMatrix * modelMatrix, {0,0,0});

    float dt = 0.016f;
    world.update(dt, m_camera.position(), m_device);
    world.updateTime(dt);
    m_cameraUniforms.position = m_camera.position();
    renderCommandEncoder->setVertexBytes(&m_cameraUniforms, sizeof(m_cameraUniforms), 1);
    renderCommandEncoder->setFragmentBytes(&m_cameraUniforms, sizeof(m_cameraUniforms), 1);
    world.render(renderCommandEncoder, m_cameraUniforms.viewProjectionMatrix);
    dt += 0.016f;
    
    ui.beginFrame(m_viewport.width, m_viewport.height);
    ui.drawText("Hello 89 ! Make sense", 50, 50, 0.5);
    ui.endFrame(renderCommandEncoder);

//    colorsFlash.renderPostProcess(renderCommandEncoder);
    
    

    renderCommandEncoder->endEncoding();
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    pool->release();
}

void GameCoordinator::resizeMtkViewAndUpdateViewportWindow(NS::UInteger width, NS::UInteger height)
{
    setViewportWindow(width, height);

    m_depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(m_depthPixelFormat, width, height, false);
    m_depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
    m_depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
    m_depthTexture = m_device->newTexture(m_depthTextureDescriptor);
}

void GameCoordinator::setViewportWindow(NS::UInteger width, NS::UInteger height)
{
    m_viewport.originX = 0.0;
    m_viewport.originY = 0.0;
    m_viewport.width   = width;
    m_viewport.height  = height;
    m_viewport.znear   = 0.0;
    m_viewport.zfar    = 1.0;
    m_viewportSize.x = (float)width;
    m_viewportSize.y = (float)height;
    ft_memcpy(m_viewportSizeBuffer->contents(), &m_viewportSize, sizeof(m_viewportSize));
}




RMDLRendererSpammy::RMDLRendererSpammy(MTL::Device* pDevice, MTL::PixelFormat pixelFormat, NS::UInteger width, NS::UInteger height, const std::string& resourcePath )
: _pDevice(pDevice->retain()), _pixelFormat(pixelFormat), _frame(0)
{
    _pCommandQueue = _pDevice->newMTL4CommandQueue();
    _pShaderLibrary = _pDevice->newDefaultLibrary();
    _pSharedEvent = _pDevice->newSharedEvent();
    _pSharedEvent->setSignaledValue(_currentFrameIndex);

    _semaphore = dispatch_semaphore_create( kMaxFramesInFlight );

    NS::Error* pError = nullptr;

    _pViewportSize.x = (float)width;
    _pViewportSize.x = (float)height;
    _pViewportSizeBuffer = pDevice->newBuffer(sizeof(_pViewportSize), MTL::ResourceStorageModeShared);
    ft_memcpy(_pViewportSizeBuffer->contents(), &_pViewportSize, sizeof(_pViewportSize));

    setViewportWindow(width, height);

    loadPngAndFont(resourcePath);
    _pAudioEngine = std::make_unique<PhaseAudio>(resourcePath);
    loadSoundMp3(resourcePath, _pAudioEngine.get());

    buildDepthStencilStates(width, height);

    for (uint8_t i = 0; i < kMaxFramesInFlight; i++)
    {
    }
    
    resizeMtkViewAndUpdateViewportWindow(width, height);
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

void RMDLRendererSpammy::setViewportWindow(NS::UInteger width, NS::UInteger height)
{
    _pViewportSize.x = (float)width;
    _pViewportSize.y = (float)height;
    ft_memcpy(_pViewportSizeBuffer->contents(), &_pViewportSize, sizeof(_pViewportSize));
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

void RMDLRendererSpammy::loadSoundMp3( const std::string &resourcesPath, PhaseAudio *pAudioEngine )
{
    pAudioEngine->loadStereoSound(resourcesPath, "Test.mp3");
    pAudioEngine->loadMonoSound(resourcesPath, "Test.m4a");
}

void RMDLRendererSpammy::makeArgumentTable()
{
    NS::Error* pError = nullptr;

    NS::SharedPtr<MTL4::ArgumentTableDescriptor> argumentTableDescriptor = NS::TransferPtr( MTL4::ArgumentTableDescriptor::alloc()->init() );
//    argumentTableDescriptor->setMaxBufferBindCount(2);
    _pArgumentTable = _pDevice->newArgumentTable(argumentTableDescriptor.get(), &pError);
}

void RMDLRendererSpammy::buildDepthStencilStates( NS::UInteger width, NS::UInteger height )
{
    NS::SharedPtr<MTL::DepthStencilDescriptor> pDepthStencilDesc = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    pDepthStencilDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionAlways);
    pDepthStencilDesc->setDepthWriteEnabled(false);
    _pDepthStencilState = _pDevice->newDepthStencilState(pDepthStencilDesc.get());

    MTL::TextureDescriptor* pTextureDesc = MTL::TextureDescriptor::textureCubeDescriptor(m_depthPixelFormat, 3.3f, false);
    pTextureDesc->setUsage(MTL::TextureUsageRenderTarget);
    pTextureDesc->setStorageMode(MTL::StorageModePrivate);
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

void RMDLRendererSpammy::resizeMtkViewAndUpdateViewportWindow(NS::UInteger width, NS::UInteger height)
{
    setViewportWindow(width, height);
    _pDepthTextureDesc = MTL::TextureDescriptor::texture2DDescriptor(m_depthPixelFormat, width, height, false);
    _pDepthTextureDesc->setUsage(MTL::TextureUsageRenderTarget);
    _pDepthTextureDesc->setStorageMode(MTL::StorageModePrivate);
    _pTexture = _pDevice->newTexture(_pDepthTextureDesc);
}
