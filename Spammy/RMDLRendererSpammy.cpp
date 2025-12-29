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

GameCoordinator::GameCoordinator(MTL::Device* device,
                                 MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat,
                                 NS::UInteger width, NS::UInteger height,
                                 const std::string& ressourcePath)
: m_device(device->retain()), m_shaderLibrary(m_device->newDefaultLibrary()),
m_pixelFormat(layerPixelFormat), m_depthPixelFormat(depthPixelFormat),
blender(m_device, layerPixelFormat, m_depthPixelFormat, ressourcePath, m_shaderLibrary), _rotationAngle(0.0f),
skybox(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
snow(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
world(m_device, layerPixelFormat, m_depthPixelFormat, m_shaderLibrary),
ui(m_device, layerPixelFormat, depthPixelFormat, width, height, m_shaderLibrary)
{
    AAPL_PRINT("NS::UIntegerMax = " + std::to_string(NS::UIntegerMax));
    cmdQueue = m_device->newCommandQueue();
    _pAudioEngine = std::make_unique<PhaseAudio>(ressourcePath);
    loadGameSounds(ressourcePath, _pAudioEngine.get());
    cursorPos = simd::make_float2(0, 0);
    resizeMtkView(width, height);
    _camera.initPerspectiveWithPosition({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, M_PI / 3.0f, 1.0f, 0.1f, 100.0f);
}

GameCoordinator::~GameCoordinator()
{
    m_shaderLibrary->release();

    vertexBuffer->release();
    indexBuffer->release();
    transformBuffer->release();
    pipelineState->release();
    cmdQueue->release();
}

void GameCoordinator::moveCamera(simd::float3 translation)
{
    simd::float3 newPosition = _camera.position() + translation;
    _camera.setPosition(newPosition);
}

void GameCoordinator::rotateCamera(float deltaYaw, float deltaPitch)
{
    _camera.rotateOnAxis({0.0f, 1.0f, 0.0f}, deltaYaw);
    _camera.rotateOnAxis(_camera.right(), deltaPitch);
}

void GameCoordinator::handleMouseDown(bool rightClick)
{
}

void GameCoordinator::handleMouseUp()
{
    isDragging = false;
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

void GameCoordinator::draw( MTK::View* view )
{
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer* cmdBuf = cmdQueue->commandBuffer();
    MTL::RenderPassDescriptor* passDesc = view->currentRenderPassDescriptor();
//    passDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.1, 0.15, 0.2, 1.0));
    
    MTL::RenderCommandEncoder* enc = cmdBuf->renderCommandEncoder(passDesc);
    MTL::Viewport viewport;
    viewport.originX = 0.0;
    viewport.originY = 0.0;
    viewport.width   = view->drawableSize().width;
    viewport.height  = view->drawableSize().height;
    viewport.znear   = 0.0;
    viewport.zfar    = 1.0;
    enc->setViewport(viewport);

    _rotationAngle += 0.0004f;
    if (_rotationAngle > 2 * M_PI)
    {
        _rotationAngle -= 2 * M_PI;
    }
    simd::float4x4 modelMatrix =
    {
        simd::float4{ cosf(_rotationAngle), 0.0f, sinf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 1.0f, 0.0f, 0.0f },
        simd::float4{ -sinf(_rotationAngle), 0.0f, cosf(_rotationAngle), 0.0f },
        simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    simd::float4x4 modelMatrix2 = math::makeIdentity();
    RMDLCameraUniforms cameraUniforms = _camera.uniforms();
    blender.drawBlender(enc, cameraUniforms.viewProjectionMatrix * modelMatrix, modelMatrix); // matrix_identity_float4x4
    blender.updateBlender(0.0f);

    skybox.render(enc, modelMatrix2, cameraUniforms.viewProjectionMatrix * modelMatrix2, _camera.position()); //{0,0,0});
//    snow.render(enc, modelMatrix2, {0,0,0});
//    skybox.updateUniforms(modelMatrix, cameraUniforms.viewProjectionMatrix * modelMatrix, {0,0,0});

    
    
    float dt = 0.016f;
    world.update(dt, _camera.position());
    world.updateTime(dt);
    cameraUniforms.position = _camera.position();
    enc->setVertexBytes(&cameraUniforms, sizeof(cameraUniforms), 1);
    enc->setFragmentBytes(&cameraUniforms, sizeof(cameraUniforms), 1);
    world.render(enc, cameraUniforms.viewProjectionMatrix);
    
    ui.beginFrame(enc, viewport.width, viewport.height);
    ui.drawText("Hello 89 ! Make sense", 50, 50, 0.5);
    ui.endFrame(enc);

    enc->endEncoding();
    cmdBuf->presentDrawable(view->currentDrawable());
    cmdBuf->commit();
    pool->release();
}

void GameCoordinator::resizeMtkView( NS::UInteger width, NS::UInteger height )
{
    
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
    NS::SharedPtr<MTL::DepthStencilDescriptor> pDepthStencilDesc = NS::TransferPtr( MTL::DepthStencilDescriptor::alloc()->init() );
    pDepthStencilDesc->setDepthCompareFunction( MTL::CompareFunction::CompareFunctionAlways );
    pDepthStencilDesc->setDepthWriteEnabled(false);
    _pDepthStencilState = _pDevice->newDepthStencilState(pDepthStencilDesc.get());

    MTL::TextureDescriptor* pTextureDesc = MTL::TextureDescriptor::textureCubeDescriptor(m_depthPixelFormat, 3.3f, false);
    pTextureDesc->setUsage( MTL::TextureUsageRenderTarget );
    pTextureDesc->setStorageMode( MTL::StorageModePrivate );
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


//GameCoordinator::GameCoordinator(MTL::Device* pDevice,
//                                 MTL::PixelFormat layerPixelFormat,
//                                 NS::UInteger width,
//                                 NS::UInteger height,
//                                 NS::UInteger gameUICanvasSize,
//                                 const std::string& assetSearchPath)
//    : _pPixelFormat(layerPixelFormat)
//    , _pCommandQueue(nullptr)
//    , _pCommandBuffer{nullptr}
//    , _pArgumentTable(nullptr)
//    , _pArgumentTableJDLV(nullptr)
//    , _pResidencySet(nullptr)
//    , _sharedEvent(nullptr)
//    , _semaphore(nullptr)
//    , _pDevice(pDevice->retain())
//    , _pPSO(nullptr)
//    , _pDepthStencilState(nullptr)
//    , _pTexture(nullptr)
//    , _uniformBufferIndex(0)
//    , _currentFrameIndex(0)
//    , _pShaderLibrary(nullptr)
//    , _frameNumber(0)
//    , _pViewportSizeBuffer(nullptr)
//    , _pJDLVRenderPSO(nullptr)
//    , _pJDLVComputePSO(nullptr)
//    , _useBufferAAsSource(true)
//    , _pDepthStencilStateJDLV(nullptr)
//{
//    printf("GameCoordinator constructor called\n");
//
//    unsigned int numThreads = std::thread::hardware_concurrency();
//    std::cout << "std::thread::hardware_concurrency : " << numThreads << std::endl;
//
//    std::cout << "size of uint64_t : "<< sizeof(uint64_t) << " bits" << std::endl;
//
//    if (!_pDevice->supportsFamily(MTL::GPUFamily::GPUFamilyMetal4))
//        std::cerr << "Metal features required by this app are not supported on this device (GPUFamily::GPUFamilyMetal4 check failed)." << std::endl;
//
//#pragma mark init
//    _pCommandQueue = _pDevice->newMTL4CommandQueue();
//    _pShaderLibrary = _pDevice->newDefaultLibrary(); // MTL::Library* MTL::Device::newDefaultLibrary(const NS::Bundle*, NS::Error**)
//
//    size_t gridSize = kGridWidth * kGridHeight * sizeof(uint32_t);
//
//    for (uint8_t i = 0; i < kMaxFramesInFlight; i++)
//    {
//        _pTriangleDataBuffer[i] = _pDevice->newBuffer( sizeof(TriangleData), MTL::ResourceStorageModeShared );
//        std::string name = "_pTriangleDataBuffer[" + std::to_string(i) + "]";
//        _pTriangleDataBuffer[i]->setLabel( NS::String::string( name.c_str(), NS::ASCIIStringEncoding ) );
//
//        _pCommandAllocator[i] = _pDevice->newCommandAllocator();
//
//        _pJDLVStateBuffer[i] = _pDevice->newBuffer( sizeof(JDLVState), MTL::ResourceStorageModeManaged );
//        _pGridBuffer_A[i] = _pDevice->newBuffer( gridSize, MTL::ResourceStorageModeManaged );
//        _pGridBuffer_B[i] = _pDevice->newBuffer( gridSize, MTL::ResourceStorageModeManaged );
//        ft_memset(_pGridBuffer_A[i]->contents(), 0, gridSize);
//        ft_memset(_pGridBuffer_B[i]->contents(), 0, gridSize);
//        _pGridBuffer_A[i]->didModifyRange( NS::Range(0, gridSize) );
//        _pGridBuffer_B[i]->didModifyRange( NS::Range(0, gridSize) );
//
//        _pTextBuffer[i] = _pDevice->newBuffer(sizeof(TextVertex), MTL::ResourceStorageModeShared);
//        //    auto vbuf = _pDevice->newBuffer(textVertices.data(), textVertices.size() * sizeof(TextVertex),
//        //    ft_memcpy(_pTextVertexBuffer->contents(), textVertices.data(), textVertices.size() * sizeof(TextVertex));
//    }
//    font = newFontAtlas(_pDevice);
//    _textureAssets["fontAtlas"] = font.texture;
//
//    initGrid();
//    buildJDLVPipelines();
//    buildDepthStencilStates( width, height );
//
//    const NS::UInteger nativeWidth = (NS::UInteger)(width);
//    const NS::UInteger nativeHeight = (NS::UInteger)(height);
//    _pViewportSize.x = (float)nativeWidth;
//    _pViewportSize.y = (float)nativeHeight;
//    _pViewportSizeBuffer = _pDevice->newBuffer(sizeof(_pViewportSize), MTL::ResourceStorageModeShared);
//    ft_memcpy(_pViewportSizeBuffer->contents(), &_pViewportSize, sizeof(_pViewportSize));
//
//    makeArgumentTable();
//    makeResidencySet();
//
//    compileRenderPipeline(_pPixelFormat);
//    createTextPipeline();
//
//    _sharedEvent = _pDevice->newSharedEvent();
//    _sharedEvent->setSignaledValue(_currentFrameIndex);
//
//    _semaphore = dispatch_semaphore_create( kMaxFramesInFlight );
//
//    setupCamera();
//}
//
//GameCoordinator::~GameCoordinator()
//{
//    for (uint8_t i = 0; i < kMaxFramesInFlight; ++i)
//    {
//        _pTriangleDataBuffer[i]->release();
//        _pCommandAllocator[i]->release();
//        _pInstanceDataBuffer[i]->release();
//        _pJDLVStateBuffer[i]->release();
//        _pGridBuffer_A[i]->release();
//        _pGridBuffer_B[i]->release();
//    }
//    _pJDLVComputePSO->release();
//    _pJDLVRenderPSO->release();
//    _pTexture->release();
//    _pDepthStencilState->release();
//    _pDepthStencilStateJDLV->release();
//    _pPSO->release();
//    _pShaderLibrary->release();
////    _pCommandBuffer->release();
//    mesh_utils::releaseMesh(&_currentScoreMesh);
//    mesh_utils::releaseMesh(&_timeMesh);
//    _pCommandQueue->release();
//    _pResidencySet->release();
//    _pArgumentTable->release();
//    _pArgumentTableJDLV->release();
//    _sharedEvent->release();
//    _pViewportSizeBuffer->release();
//    dispatch_release(_semaphore);
//    _pDevice->release();
//}
//
//void GameCoordinator::resizeMtkView( NS::UInteger width, NS::UInteger height )
//{
//    const NS::UInteger nativeWidth = (NS::UInteger)(width/1.2);
//    const NS::UInteger nativeHeight = (NS::UInteger)(height/1.2);
//    _pViewportSize.x = (float)nativeWidth;
//    _pViewportSize.y = (float)nativeHeight;
//
//    _pViewportSizeBuffer = _pDevice->newBuffer(sizeof(_pViewportSize), MTL::ResourceStorageModeShared);
//    ft_memcpy(_pViewportSizeBuffer->contents(), &_pViewportSize, sizeof(_pViewportSize));
//    MTL::TextureDescriptor* depthDesc =
//        MTL::TextureDescriptor::texture2DDescriptor(
//            MTL::PixelFormatDepth32Float,
//            width,
//            height,
//            false );
//    depthDesc->setUsage( MTL::TextureUsageRenderTarget );
//    depthDesc->setStorageMode( MTL::StorageModePrivate );
//    _pTexture = _pDevice->newTexture(depthDesc);
//    depthDesc->release();
//}
//
//void GameCoordinator::buildDepthStencilStates( NS::UInteger width, NS::UInteger height )
//{
//    MTL::DepthStencilDescriptor* pDsDesc = MTL::DepthStencilDescriptor::alloc()->init();
//    MTL::DepthStencilDescriptor* pDsDescTriangle = MTL::DepthStencilDescriptor::alloc()->init();
//    pDsDesc->setDepthCompareFunction( MTL::CompareFunction::CompareFunctionAlways );
//    pDsDescTriangle->setDepthCompareFunction( MTL::CompareFunction::CompareFunctionLess );
//    pDsDesc->setDepthWriteEnabled( false );
//    pDsDescTriangle->setDepthWriteEnabled( true );
//
//    _pDepthStencilStateJDLV = _pDevice->newDepthStencilState( pDsDesc );
//    _pDepthStencilState = _pDevice->newDepthStencilState( pDsDescTriangle );
//
//    pDsDesc->release();
//    pDsDescTriangle->release();
//    MTL::TextureDescriptor* depthDesc =
//        MTL::TextureDescriptor::texture2DDescriptor(
//            MTL::PixelFormatDepth32Float,
//                                                    width,
//                                                    height,
////                                        1024, 1024,
////            (NS::UInteger)_pViewportSize.x,
////            (NS::UInteger)_pViewportSize.y,
//            false );
//    depthDesc->setUsage( MTL::TextureUsageRenderTarget );
//    depthDesc->setStorageMode( MTL::StorageModePrivate );
//    _pTexture = _pDevice->newTexture(depthDesc);
////    depthDesc->release(); // why crash?
//}
//
//void GameCoordinator::initGrid()
//{
//    uint32_t* gridData = static_cast<uint32_t*>(_pGridBuffer_A[0]->contents());
//
//    int gunPattern[36][9] = {
//            {1,0,1,0,1,0,1,1,1},
//            {1,0,1,0,0,0,1,0,0},
//            {1,0,1,0,1,0,0,1,0},
//            {0,1,0,0,1,1,1,0,0},
//            {1,0,0,0,0,0,1,1,0},
//            {1,1,0,0,0,1,1,0,1},
//            {0,1,1,0,0,0,0,1,0},
//            {1,0,0,1,1,1,1,1,1},
//            {0,1,1,1,1,1,1,1,1},
//            {0,0,1,1,1,0,0,1,1},
//            {0,0,0,1,1,1,0,1,0},
//            {0,0,0,1,0,1,1,1,0},
//            {0,0,0,1,0,1,1,0,1},
//            {0,0,0,1,1,1,1,1,0},
//            {0,0,0,0,0,1,1,1,0},
//            {1,1,0,1,0,1,1,1,0},
//            {1,1,0,1,1,1,1,0,1} };
//
//    int startX = kGridWidth / 2 - 4;
//    int startY = kGridHeight / 2 - 8;
//    
//    for (int y = 0; y < 17; y++)
//    {
//        for (int x = 0; x < 9; x++)
//        {
//            int gridX = startX + x;
//            int gridY = startY + y;
//            if (gridX >= 0 && gridX < kGridWidth && gridY >= 0 && gridY < kGridHeight)
//            {
//                gridData[gridY * kGridWidth + gridX] = gunPattern[y][x];
//            }
//        }
//    }
//    _pGridBuffer_A[0]->didModifyRange( NS::Range(0, kGridWidth * kGridHeight * sizeof(uint32_t)) );
//}
//
//void GameCoordinator::createTextPipeline()
//{
//    NS::Error* pError = nullptr;
//
//    NS::SharedPtr<MTL4::RenderPipelineDescriptor> renderDescriptor = NS::TransferPtr( MTL4::RenderPipelineDescriptor::alloc()->init() );
//    renderDescriptor->setLabel(MTLSTR("Text Pipeline"));
//    renderDescriptor->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatRGBA16Float );
//
//    NS::SharedPtr<MTL4::LibraryFunctionDescriptor> vertexFunction = NS::TransferPtr( MTL4::LibraryFunctionDescriptor::alloc()->init() );
//    vertexFunction->setName(MTLSTR("textVSc"));
//    vertexFunction->setLibrary(_pShaderLibrary);
//    renderDescriptor->setVertexFunctionDescriptor(vertexFunction.get());
//
//    NS::SharedPtr<MTL4::LibraryFunctionDescriptor> fragmentFunction = NS::TransferPtr( MTL4::LibraryFunctionDescriptor::alloc()->init() );
//    fragmentFunction->setName(MTLSTR("textFSc"));
//    fragmentFunction->setLibrary(_pShaderLibrary);
//    renderDescriptor->setFragmentFunctionDescriptor(fragmentFunction.get());
//
//    renderDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
//    renderDescriptor->colorAttachments()->object(0)->setBlendingState(MTL4::BlendStateEnabled);
//    renderDescriptor->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);
//    renderDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
//    renderDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//    renderDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//
//    NS::SharedPtr<MTL4::Compiler> compiler = NS::TransferPtr(_pDevice->newCompiler( MTL4::CompilerDescriptor::alloc()->init(), &pError ));
//    _pTextPSO = compiler->newRenderPipelineState(renderDescriptor.get(), nullptr, &pError);
//
//    NS::SharedPtr<MTL4::ArgumentTableDescriptor> computeArgumentTable = NS::TransferPtr( MTL4::ArgumentTableDescriptor::alloc()->init() );
//    computeArgumentTable->setMaxBufferBindCount(1);
//    computeArgumentTable->setMaxTextureBindCount(1);
//    computeArgumentTable->setLabel( NS::String::string( "text argument table descriptor", NS::ASCIIStringEncoding ) );
//
//    _pArgumentTableText = _pDevice->newArgumentTable(computeArgumentTable.get(), &pError);
//
//    NS::SharedPtr<MTL::ResidencySetDescriptor> residencySetDescriptor = NS::TransferPtr( MTL::ResidencySetDescriptor::alloc()->init() );
//    residencySetDescriptor->setLabel( NS::String::string( "text residency set", NS::ASCIIStringEncoding ) );
//
//    _pResidencySet = _pDevice->newResidencySet(residencySetDescriptor.get(), &pError);
//    _pResidencySet->requestResidency();
//
//    for (uint8_t i = 0u; i < kMaxFramesInFlight; ++i)
//    {
//        _pResidencySet->addAllocation(_pTextBuffer[i]);
//    }
////    _pResidencySet->addAllocation(_pFontTexture);
//    _pResidencySet->addAllocation(font.texture.get());
//
//    _pResidencySet->commit();
//
//    _pCommandQueue->addResidencySet(_pResidencySet);
//}
//
//
//void GameCoordinator::buildJDLVPipelines()
//{
//    NS::Error* pError = nullptr;
//
//    MTL::Function* computeFunction = _pShaderLibrary->newFunction(MTLSTR("JDLVCompute"));
//    _pJDLVComputePSO = _pDevice->newComputePipelineState(computeFunction, &pError);
//    computeFunction->release();
////    MTL4::ComputePipelineDescriptor* computeDescriptor = MTL4::ComputePipelineDescriptor::alloc()->init();
////    computeDescriptor->setComputeFunctionDescriptor(computeFunction);
//    MTL4::RenderPipelineDescriptor* renderDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
//    renderDescriptor->setLabel(MTLSTR("JDLV Pipeline"));
//    renderDescriptor->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatRGBA16Float );
//
//    renderDescriptor->colorAttachments()->object(0)->setBlendingState(MTL4::BlendStateEnabled);
//    renderDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//    renderDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//    renderDescriptor->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);
//    renderDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
//    renderDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//    renderDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
//    
//    MTL4::LibraryFunctionDescriptor* vertexFunction = MTL4::LibraryFunctionDescriptor::alloc()->init();
//    vertexFunction->setName(MTLSTR("JDLVVertex"));
//    vertexFunction->setLibrary(_pShaderLibrary);
//    renderDescriptor->setVertexFunctionDescriptor(vertexFunction);
//
//    MTL4::LibraryFunctionDescriptor* fragmentFunction = MTL4::LibraryFunctionDescriptor::alloc()->init();
//    fragmentFunction->setName(MTLSTR("JDLVFragment"));
//    fragmentFunction->setLibrary(_pShaderLibrary);
//    renderDescriptor->setFragmentFunctionDescriptor(fragmentFunction);
//
//    MTL4::Compiler* compiler = _pDevice->newCompiler( MTL4::CompilerDescriptor::alloc()->init(), &pError );
//    _pJDLVRenderPSO = compiler->newRenderPipelineState(renderDescriptor, nullptr, &pError);
//
//    MTL4::ArgumentTableDescriptor* computeArgumentTable = MTL4::ArgumentTableDescriptor::alloc()->init();
//    computeArgumentTable->setMaxBufferBindCount(3);
//    computeArgumentTable->setLabel( NS::String::string( "p argument table descriptor JDLV", NS::ASCIIStringEncoding ) );
//
//    _pArgumentTableJDLV = _pDevice->newArgumentTable(computeArgumentTable, &pError);
//
//    MTL::ResidencySetDescriptor* residencySetDescriptor = MTL::ResidencySetDescriptor::alloc()->init();
//    residencySetDescriptor->setLabel( NS::String::string( "p residency set JDLV", NS::ASCIIStringEncoding ) );
//
//    _pResidencySet = _pDevice->newResidencySet(residencySetDescriptor, &pError);
//    _pResidencySet->requestResidency();
//
//    for (uint8_t i = 0u; i < kMaxFramesInFlight; ++i)
//    {
//        _pResidencySet->addAllocation(_pJDLVStateBuffer[i]);
//        _pResidencySet->addAllocation(_pGridBuffer_A[i]);
//        _pResidencySet->addAllocation(_pGridBuffer_B[i]);
//    }
//    _pResidencySet->commit();
//
//    _pCommandQueue->addResidencySet(_pResidencySet); // .get() is for struct
//
//    residencySetDescriptor->release();
//    computeArgumentTable->release();
//    fragmentFunction->release();
//    vertexFunction->release();
//    renderDescriptor->release();
////    computeDescriptor->release();
////    compiler->release();
//}
//
//void GameCoordinator::makeArgumentTable()
//{
//    NS::Error* pError = nullptr;
//
//    MTL4::ArgumentTableDescriptor* argumentTableDescriptor = MTL4::ArgumentTableDescriptor::alloc()->init();
//    argumentTableDescriptor->setMaxBufferBindCount(2);
//    argumentTableDescriptor->setLabel( NS::String::string( "p argument table descriptor triangle", NS::ASCIIStringEncoding ) );
//
//    _pArgumentTable = _pDevice->newArgumentTable(argumentTableDescriptor, &pError);
//    argumentTableDescriptor->release();
//}
//
//void GameCoordinator::makeResidencySet()
//{
//    NS::Error* pError = nullptr;
//
//    MTL::ResidencySetDescriptor* residencySetDescriptor = MTL::ResidencySetDescriptor::alloc()->init();
//    residencySetDescriptor->setLabel( NS::String::string( "p residency set", NS::ASCIIStringEncoding ) );
//
//    _pResidencySet = _pDevice->newResidencySet(residencySetDescriptor, &pError);
//    _pResidencySet->requestResidency();
//
//    for (uint8_t i = 0u; i < kMaxFramesInFlight; ++i)
//    {
//        _pResidencySet->addAllocation(_pTriangleDataBuffer[i]);
//    }
//    _pResidencySet->addAllocation(_pViewportSizeBuffer);
//
//    _pResidencySet->commit();
//    _pCommandQueue->addResidencySet(_pResidencySet); // .get() is for struct
////    _pCommandQueue->addResidencySet((CA::MetalLayer *)layer)
//    residencySetDescriptor->release();
//}
//
//void GameCoordinator::compileRenderPipeline( MTL::PixelFormat _layerPixelFormat )
//{
//    NS::Error* pError = nullptr;
//
//    MTL4::Compiler* compiler = _pDevice->newCompiler( MTL4::CompilerDescriptor::alloc()->init(), &pError );
//
//    MTL4::RenderPipelineDescriptor* descriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
//    descriptor->setLabel( NS::String::string( "Metal 4 render pipeline", NS::ASCIIStringEncoding ) );
//    descriptor->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatRGBA16Float );
//
//    MTL4::LibraryFunctionDescriptor* vertexFunction = MTL4::LibraryFunctionDescriptor::alloc()->init();
//    vertexFunction->setName(MTLSTR("vertexShaderTriangle"));
//    vertexFunction->setLibrary(_pShaderLibrary);
//
//    MTL4::LibraryFunctionDescriptor* fragmentFunction = MTL4::LibraryFunctionDescriptor::alloc()->init();
//    fragmentFunction->setName( NS::String::string( "fragmentShaderTriangle", NS::ASCIIStringEncoding ) );
//    fragmentFunction->setLibrary(_pShaderLibrary);
//
//    descriptor->setVertexFunctionDescriptor(vertexFunction);
//    descriptor->setFragmentFunctionDescriptor(fragmentFunction);
//
////    NS::URL* url = NS::URL::fileURLWithPath( NS::String::string( assetSearchPath.c_str(), NS::ASCIIStringEncoding ) );
////    MTL4::Archive* archivesArray = _pDevice->newArchive(url, &pError);
//    MTL4::CompilerTaskOptions* compilerTaskOptions = MTL4::CompilerTaskOptions::alloc()->init();
//    //compilerTaskOptions->setLookupArchives(archivesArray);
//
//    _pPSO = compiler->newRenderPipelineState(descriptor, nullptr, &pError);
//    compilerTaskOptions->release();
//    fragmentFunction->release();
//    vertexFunction->release();
//    descriptor->release();
//    compiler->release();
//}
//
//void GameCoordinator::setupCamera()
//{
//}
//
//void GameCoordinator::buildCubeBuffers()
//{
//}
//
//void GameCoordinator::buildRenderPipelines( const std::string& shaderSearchPath )
//{
//}
//
//void GameCoordinator::buildComputePipelines( const std::string& shaderSearchPath )
//{
//    // Build any compute pipelines
//}
//
//void GameCoordinator::buildRenderTextures(NS::UInteger nativeWidth, NS::UInteger nativeHeight,
//                                          NS::UInteger presentWidth, NS::UInteger presentHeight)
//{
//}
//
//void GameCoordinator::moveCamera( simd::float3 translation )
//{
//}
//
//void GameCoordinator::rotateCamera(float deltaYaw, float deltaPitch)
//{
//}
//
//void GameCoordinator::setCameraAspectRatio(float aspectRatio)
//{
//}
//
//void GameCoordinator::draw( MTK::View* _pView )
//{
//    NS::AutoreleasePool *pPool = NS::AutoreleasePool::alloc()->init();
//
//    _currentFrameIndex += 1;
//
//    const uint32_t frameIndex = _currentFrameIndex % kMaxFramesInFlight;
//    std::string label = "Frame: " + std::to_string(_currentFrameIndex);
//    
//    if (_currentFrameIndex > kMaxFramesInFlight)
//    {
//        uint64_t const timeStampToWait = _currentFrameIndex - kMaxFramesInFlight;
//        _sharedEvent->waitUntilSignaledValue(timeStampToWait, DISPATCH_TIME_FOREVER);
//    }
//
//    MTL::Viewport viewPort;
//    viewPort.originX = 0.0;
//    viewPort.originY = 0.0;
//    viewPort.znear = 0.0;
//    viewPort.zfar = 1.0;
//    viewPort.width = (double)_pViewportSize.x;
//    viewPort.height = (double)_pViewportSize.y;
//
//    MTL::Viewport viewPortJDLV;
//    viewPortJDLV.originX = 200.0;
//    viewPortJDLV.originY = 200.0;
//    viewPortJDLV.znear = 0.0;
//    viewPortJDLV.zfar = 1.0;
//    viewPortJDLV.width = (double)_pViewportSize.x;
//    viewPortJDLV.height = (double)_pViewportSize.y;
//
//    _pCommandAllocator[frameIndex]->reset();
//
//    _pCommandBuffer[0] = _pDevice->newCommandBuffer();
//    _pCommandBuffer[0]->beginCommandBuffer(_pCommandAllocator[frameIndex]);
//    _pCommandBuffer[0]->setLabel( NS::String::string( label.c_str(), NS::ASCIIStringEncoding ) );
//
//    MTL4::RenderPassDescriptor* pRenderPassDescriptor = _pView->currentMTL4RenderPassDescriptor();
//    MTL::RenderPassColorAttachmentDescriptor* color0 = pRenderPassDescriptor->colorAttachments()->object(0);
//    color0->setLoadAction( MTL::LoadActionClear );
//    color0->setStoreAction( MTL::StoreActionStore );
//    color0->setClearColor( MTL::ClearColor(0.1, 0.1, 0.1, 1.0) );
//
//    pRenderPassDescriptor->depthAttachment()->setTexture(_pTexture);
//    pRenderPassDescriptor->depthAttachment()->setClearDepth(1.0f);
//    pRenderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
//    pRenderPassDescriptor->depthAttachment()->setStoreAction(MTL::StoreActionStore);
//
//    MTL4::RenderCommandEncoder* renderPassEncoder = _pCommandBuffer[0]->renderCommandEncoder(pRenderPassDescriptor);
//    renderPassEncoder->setLabel(NS::String::string(label.c_str(), NS::ASCIIStringEncoding));
//    renderPassEncoder->setRenderPipelineState(_pPSO);
//    renderPassEncoder->setDepthStencilState( _pDepthStencilState );
//    renderPassEncoder->setViewport(viewPort);
//
//    configureVertexDataForBuffer(_currentFrameIndex, _pTriangleDataBuffer[frameIndex]->contents());
//
//    _pArgumentTable->setAddress(_pTriangleDataBuffer[frameIndex]->gpuAddress(), 0);
//    _pArgumentTable->setAddress(_pViewportSizeBuffer->gpuAddress(), 1);
//
//    renderPassEncoder->setArgumentTable(_pArgumentTable, MTL::RenderStageVertex);
//    renderPassEncoder->drawPrimitives( MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3) );
//    renderPassEncoder->endEncoding();
//    _pCommandBuffer[0]->endCommandBuffer();
//
//    _pCommandBuffer[1] = _pDevice->newCommandBuffer();
//    _pCommandBuffer[1]->beginCommandBuffer(_pCommandAllocator[frameIndex]);
//
//    JDLVState* jdlvState = static_cast<JDLVState*>(_pJDLVStateBuffer[frameIndex]->contents());
//    jdlvState->width = kGridWidth;
//    jdlvState->height = kGridHeight;
//    _pJDLVStateBuffer[frameIndex]->didModifyRange( NS::Range(0, sizeof(JDLVState)) );
//    MTL::Buffer* sourceGrid = _useBufferAAsSource ? _pGridBuffer_A[frameIndex] : _pGridBuffer_B[frameIndex];
//    MTL::Buffer* destGrid = _useBufferAAsSource ? _pGridBuffer_B[frameIndex] : _pGridBuffer_A[frameIndex];
//
//    _pArgumentTableJDLV->setAddress(sourceGrid->gpuAddress(), 0);
//    _pArgumentTableJDLV->setAddress(destGrid->gpuAddress(), 1);
//    _pArgumentTableJDLV->setAddress(_pJDLVStateBuffer[frameIndex]->gpuAddress(), 2);
////    renderPassEncoder->setArgumentTable(_pArgumentTableJDLV, MTL::RenderStageVertex);
//
//    MTL4::ComputeCommandEncoder* computeEncoder = _pCommandBuffer[1]->computeCommandEncoder();
//    computeEncoder->setLabel(MTLSTR("JDLV Compute"));
//
//    computeEncoder->setComputePipelineState(_pJDLVComputePSO);
//
//    computeEncoder->setArgumentTable(_pArgumentTableJDLV);
//
//    MTL::Size gridSize = MTL::Size(kGridWidth, kGridHeight, 1);
//    MTL::Size threadgroupSize = MTL::Size(16, 16, 1);
//    MTL::Size threadgroups = MTL::Size((gridSize.width + threadgroupSize.width - 1) / threadgroupSize.width, (gridSize.height + threadgroupSize.height - 1) / threadgroupSize.height, 1);
//
//    computeEncoder->dispatchThreadgroups(threadgroups, threadgroupSize);
//    computeEncoder->endEncoding();
//
//    _pCommandBuffer[1]->endCommandBuffer();
//    _pCommandBuffer[2] = _pDevice->newCommandBuffer();
//    _pCommandBuffer[2]->beginCommandBuffer(_pCommandAllocator[frameIndex]);
//
//    color0->setLoadAction(MTL::LoadActionLoad);
//    pRenderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionLoad);
//
//    MTL4::RenderCommandEncoder* gridRenderPassEncoder = _pCommandBuffer[2]->renderCommandEncoder(pRenderPassDescriptor);
//
//    gridRenderPassEncoder->setRenderPipelineState(_pJDLVRenderPSO);
//    gridRenderPassEncoder->setDepthStencilState( _pDepthStencilStateJDLV );
//    gridRenderPassEncoder->setViewport(viewPortJDLV);
//
//    _pArgumentTableJDLV->setAddress(destGrid->gpuAddress(), 0);
//    _pArgumentTableJDLV->setAddress(_pJDLVStateBuffer[frameIndex]->gpuAddress(), 1);
//    gridRenderPassEncoder->setArgumentTable( _pArgumentTableJDLV, MTL::RenderStageVertex );
//    gridRenderPassEncoder->setArgumentTable( _pArgumentTableJDLV, MTL::RenderStageFragment );
//
//    gridRenderPassEncoder->drawPrimitives( MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6) );
//    gridRenderPassEncoder->endEncoding();
//
//    _pCommandBuffer[2]->endCommandBuffer();
//
//    _useBufferAAsSource = !_useBufferAAsSource;
//
//    _pCommandBuffer[3] = _pDevice->newCommandBuffer();
//    _pCommandBuffer[3]->beginCommandBuffer(_pCommandAllocator[frameIndex]);
//
//    color0->setLoadAction(MTL::LoadActionLoad);
//    pRenderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionLoad);
//
//    MTL4::RenderCommandEncoder* textRenderPassEncoder = _pCommandBuffer[3]->renderCommandEncoder(pRenderPassDescriptor);
//    textRenderPassEncoder->setRenderPipelineState(_pTextPSO);
//    textRenderPassEncoder->setViewport(viewPort);
//
//    
//    std::vector<TextVertex> textVerts;
//
//    buildTextVerticesC("SCORE : 00000000",
//                          font,
//                          -0.95f,
//                          0.85f,
//                          0.05f,
//                          textVerts);
//
//    MTL::Buffer* textVertexBuffer = _pDevice->newBuffer(
//            textVerts.data(),
//            textVerts.size() * sizeof(TextVertex),
//            MTL::ResourceStorageModeShared
//        );
//    _pArgumentTableText->setAddress(textVertexBuffer->gpuAddress(), 0);
//    _pArgumentTableText->setTexture(font.texture->gpuResourceID(), 0);
////    _pArgumentTableText->setTexture(_pFontTexture->gpuResourceID(), 0);
//
//    textRenderPassEncoder->setArgumentTable( _pArgumentTableText, MTL::RenderStageVertex );
//    textRenderPassEncoder->setArgumentTable( _pArgumentTableText, MTL::RenderStageFragment );
//
//    textRenderPassEncoder->drawPrimitives( MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(textVerts.size()) );
//    textRenderPassEncoder->endEncoding();
//    _pCommandBuffer[3]->endCommandBuffer();
//
//    CA::MetalDrawable* currentDrawable = _pView->currentDrawable();
//    _pCommandQueue->wait(currentDrawable);
//    _pCommandQueue->commit(_pCommandBuffer, 3);
//    _pCommandQueue->signalDrawable(currentDrawable);
//    _pCommandQueue->signalEvent(_sharedEvent, _currentFrameIndex);
//    currentDrawable->present();
//    pPool->release();
//}


