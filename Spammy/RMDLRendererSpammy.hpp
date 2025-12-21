/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLRendererSpammy.hpp           +++     +++  **/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 27/10/2025 15:45:19      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RMDLRENDERERSPAMMY_HPP
#define RMDLRENDERERSPAMMY_HPP

#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <string>
#include <unordered_map>
#include <simd/simd.h>
#include <vector>
#include <stdio.h>

#include "RMDLSkybox.hpp"
#include "RMDLMainRenderer_shared.h"
#include "RMDLCamera.hpp"
#include "RMDLUtils.hpp"
#include "RMDLFontLoader.h"
#include "RMDLMeshUtils.hpp"
#include "RMDLPhaseAudio.hpp"
//#include "BumpAllocator.hpp"
#include "RMDLMathUtils.hpp"
#include "RMDLBlender.hpp"

#define kMaxBuffersInFlight 3

static const uint32_t NumLights = 256;

struct Vertex {
    simd::float3 pos;
    simd::float4 color;
};

struct Transform {
    simd::float4x4 mvp;
};

enum BlockType {
    BLOCK_CORE,
    BLOCK_ARMOR,
    BLOCK_WHEEL,
    BLOCK_WEAPON
};

struct Block {
    BlockType type;
    simd::float2 gridPos;
    simd::float4 color;
    bool placed;
};


struct TriangleData
{
    VertexData vertex0;
    VertexData vertex1;
    VertexData vertex2;
};

struct GameConfig
{
    uint8_t                                 enemyRows;
    uint32_t                                screenWidth;
    uint32_t                                screenHeight;
    NS::SharedPtr<MTL::Texture>             playerTexture;
    NS::SharedPtr<MTL::Texture>             fontAtlasTexture;
    NS::SharedPtr<MTL::RenderPipelineState> spritePso;
    float                                   playerSpeed;
    PhaseAudio*                             pAudioEngine;
};

struct RenderData
{
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxBuffersInFlight> frameDataBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxBuffersInFlight> enemyPositionBuf;
    std::array<NS::SharedPtr<MTL::Buffer>, kMaxBuffersInFlight> playerPositionBuf;
    
    NS::SharedPtr<MTL::Buffer>  textureTable;
    NS::SharedPtr<MTL::Buffer>  samplerTable;
    
    NS::SharedPtr<MTL::SamplerState> sampler;
    IndexedMesh                      spriteMesh;
    IndexedMesh                      backgroundMesh;
    
//    std::array<std::unique_ptr<BumpAllocator>, kMaxFramesInFlight> bufferAllocator;
    std::array<NS::SharedPtr<MTL::Heap>, kMaxBuffersInFlight>       resourceHeaps;
    
    NS::SharedPtr<MTL::ResidencySet> residencySet;
};

class RMDLRendererSpammy
{
public:
    RMDLRendererSpammy( MTL::Device* pDevice, MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger Heigth, const std::string& resourcePath );
    ~RMDLRendererSpammy();

    void loadPngAndFont( const std::string& resourcesPath );
    void loadSoundMp3( const std::string& resourcesPath, PhaseAudio* pAudioEngine );
    void makeArgumentTable();
    void buildDepthStencilStates( NS::UInteger width, NS::UInteger height );
    void setViewportWindow(NS::UInteger width, NS::UInteger height);
    
    void draw( MTK::View* view );
    void resizeMtkViewAndUpdateViewportWindow(NS::UInteger width, NS::UInteger height);
private:
    MTL::Device*                        _pDevice;
    MTL::Buffer*                        _pViewportSizeBuffer;
    MTL::Buffer*                        _pABuffer[kMaxBuffersInFlight];
    MTL::Buffer*                        _pBuffer[kMaxBuffersInFlight];
    MTL::Buffer*                        _pCBuffer[kMaxBuffersInFlight];
    MTL::Texture*                       _pTexture;
    MTL::Library*                       _pShaderLibrary;
    MTL::Viewport                       _pViewport;
    MTL::PixelFormat                    _pPixelFormat;
    MTL::PixelFormat                    _pDepthPixelFormat;
    MTL::SharedEvent*                   _pSharedEvent;
    MTL::ResidencySet*                  _pResidencySet;
    MTL::TextureDescriptor*             _pDepthTextureDesc;
    MTL::DepthStencilState*             _pDepthStencilState;
    MTL::RenderPipelineState*           _pPSO;
    MTL::ComputePipelineState*          _pipelineStateDescriptor;
    MTL::ComputePipelineState*          _mousePositionComputeKnl;
    MTL4::CommandQueue*                 _pCommandQueue;
    MTL4::ArgumentTable*                _pArgumentTable;
    MTL4::CommandBuffer*                _pCommandBuffer[5];
    MTL4::CommandAllocator*             _pCommandAllocator[kMaxBuffersInFlight];
    MTL4::RenderPassDescriptor*         _gBufferRenderPassDesc;
    MTL4::RenderPassDescriptor*         _zBufferRenderPassDesc;
    NS::SharedPtr<MTL::SharedEvent>     _pPacingEvent;
    std::unordered_map<std::string, NS::SharedPtr<MTL::Texture>> _textureAssets;
    std::unique_ptr<PhaseAudio>         _pAudioEngine;
    NS::SharedPtr<MTL::DepthStencilDescriptor> pDsDesc;
    NS::SharedPtr<MTL::TextureDescriptor> depthDesc;
    int                                 _frame;
    uint8_t                             _uniformBufferIndex;
    uint64_t                            _currentFrameIndex;
    simd_uint2                          _pViewportSize;
    dispatch_semaphore_t                _semaphore;
};
    

class GameCoordinator {
private:
    MTL::Device* device;
    MTL::CommandQueue* cmdQueue;
    MTL::RenderPipelineState* pipelineState;
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    MTL::Buffer* transformBuffer;
    MTL::Library*                       _pShaderLibrary;
    
    std::vector<Block> blocks;
    simd::float2 cursorPos;
    float cameraZoom;
    simd::float2 cameraPos;
    bool isDragging;
    simd::float2 dragStart;
    int viewWidth;
    int viewHeight;
    float                       _rotationAngle;

    RMDLCamera _camera;
    MTL::PixelFormat                    _pPixelFormat;
    MTL::PixelFormat                    _pDepthPixelFormat;
    MTL::DepthStencilState*             _pDepthStencilState;
    PhaseAudio*                             pAudioEngine;
    std::unique_ptr<PhaseAudio> _pAudioEngine;
    
    void createPipeline();
    void createBuffers();
    simd::float4x4 makeTransform(simd::float2 pos, float scale);
    simd::float4x4 makeCamera();
    simd::float4 getBlockColor(BlockType type);
    bool DoTheImportThing(const std::string& pFile);
    RMDLBlender blender;
    sky::RMDLSkybox skybox;

public:
    GameCoordinator( MTL::Device* dev, MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger Heigth, const std::string& resourcePath );
    ~GameCoordinator();
    
    void setViewSize(int width, int height);
    void handleMouseMove(float x, float y);
    void handleMouseDown(bool rightClick);
    void handleMouseUp();
    void handleScroll(float deltaY);
    void handleKeyPress(int key);
    void playSoundTestY();
    void loadGameSounds(const std::string& resourcePath, PhaseAudio* pAudioEngine);
    void moveCamera(simd::float3 translation);
    void rotateCamera(float deltaYaw, float deltaPitch);
    void draw(MTK::View* view);
    void resizeMtkView( NS::UInteger width, NS::UInteger height );
};

//class GameCoordinator
//{
//public:
//    GameCoordinator(MTL::Device* pDevice,
//                    MTL::PixelFormat layerPixelFormat,
//                    NS::UInteger width,
//                    NS::UInteger height,
//                    NS::UInteger gameUICanvasSize,
//                    const std::string& assetSearchPath);
//    ~GameCoordinator();
//
//    void resizeMtkView( NS::UInteger nWidth, NS::UInteger wHeight );
//    void buildRenderPipelines( const std::string& shaderSearchPath );
//    void buildComputePipelines( const std::string& shaderSearchPath );
//    void buildRenderTextures(NS::UInteger nativeWidth, NS::UInteger nativeHeight,
//                             NS::UInteger presentWidth, NS::UInteger presentHeight);
//    void loadGameTextures( const std::string& textureSearchPath );
//    //void loadGameSounds( const std::string& assetSearchPath, PhaseAudio* pAudioEngine );
//    void buildSamplers();
//    void buildMetalFXUpscaler(NS::UInteger inputWidth, NS::UInteger inputHeight,
//                              NS::UInteger outputWidth, NS::UInteger outputHeight);
//
//    void presentTexture( MTL::RenderCommandEncoder* pRenderEnc, MTL::Texture* pTexture );
//
//    void setMaxEDRValue(float value)     { _maxEDRValue = value; }
//    void setBrightness(float brightness) { _brightness = brightness; }
//    void setEDRBias(float edrBias)       { _edrBias = edrBias; }
//
//    enum class HighScoreSource {
//        Local,
//        Cloud
//    };
//    
//    void setHighScore(int highScore, HighScoreSource scoreSource);
////    int highScore() const                { return _highScore; }
//
//    void setupCamera();
//    void moveCamera( simd::float3 translation );
//    void rotateCamera(float deltaYaw, float deltaPitch);
//    void setCameraAspectRatio(float aspectRatio);
//    
//    float _rotationAngle;
//    void buildCubeBuffers();
//
//    void buildShaders();
//    void buildComputePipeline();
//    void createTextPipeline();
//    void buildDepthStencilStates( NS::UInteger width, NS::UInteger height );
//    void buildTextures();
//    void buildBuffers();
//    void draw( MTK::View* _pView );
//    void buildShadersMap();
//    void buildBuffersMap();
//    
//    void setupPipelineCamera();
//    
//    //MTL::Buffer** buildTriangleDataBuffer(NS::UInteger count);
//    void makeArgumentTable();
//    void makeResidencySet();
//    void compileRenderPipeline( MTL::PixelFormat );
//
//private:
//    MTL::PixelFormat                    _pPixelFormat;
//    MTL4::CommandQueue*                 _pCommandQueue;
//    MTL4::CommandBuffer*                _pCommandBuffer[4];
//    MTL4::CommandAllocator*             _pCommandAllocator[kMaxBuffersInFlight];
//    MTL4::ArgumentTable*                _pArgumentTable;
//    MTL::ResidencySet*                  _pResidencySet;
//    MTL::SharedEvent*                   _sharedEvent;
//    dispatch_semaphore_t                _semaphore;
//    MTL::Buffer*                        _pInstanceDataBuffer[kMaxBuffersInFlight];
//    MTL::Buffer*                        _pTriangleDataBuffer[kMaxBuffersInFlight];
//    MTL::Buffer*                        _pViewportSizeBuffer;
//    MTL::Device*                        _pDevice;
//    MTL::RenderPipelineState*           _pPSO;
//    MTL::DepthStencilState*             _pDepthStencilState;
//    MTL::DepthStencilState*             _pDepthStencilStateJDLV;
//    MTL::Texture*                       _pTexture;
//    MTL::TextureDescriptor*             _pDepthTextureDesc;
//    uint8_t                             _uniformBufferIndex;
//    uint64_t                            _currentFrameIndex;
//    simd_uint2                          _pViewportSize;
//    MTL::Library*                       _pShaderLibrary;
//    int                                 _frameNumber;
//    NS::SharedPtr<MTL::SharedEvent>     _pPacingEvent;
//    FontAtlas font;
//    std::unordered_map<std::string, NS::SharedPtr<MTL::Texture>> _textureAssets;
//
//    MTL::Buffer*                        _pTextDataBuffer[kMaxBuffersInFlight];
//
//    IndexedMesh                         _timeMesh;
//    IndexedMesh                         _currentScoreMesh;
//    simd::float4                        _timePosition;
//    simd::float4                        _currentScorePosition;
////    UIRenderData _renderData;
//
//    MTL::Buffer* _pJDLVStateBuffer[kMaxBuffersInFlight];
//    MTL::Buffer* _pGridBuffer_A[kMaxBuffersInFlight];
//    MTL::Buffer*            _pGridBuffer_B[kMaxBuffersInFlight];
//    MTL::Buffer*            _pTextBuffer[kMaxBuffersInFlight];
//    MTL::ComputePipelineState*  _pJDLVComputePSO;
//    MTL::RenderPipelineState*   _pJDLVRenderPSO;
//    MTL::RenderPipelineState*   _pTextPSO;
//    MTL4::ArgumentTable*                _pArgumentTableJDLV;
//    MTL4::ArgumentTable*                _pArgumentTableText;
//    bool _useBufferAAsSource;
//    MTL4::RenderPassDescriptor*         _gBufferPassDesc;
//    MTL4::RenderPassDescriptor*         _shadowPassDesc;
//    MTL::Texture* _pFontTexture;
//    void initGrid();
//    void buildJDLVPipelines();
//
////    simd::float4x4                      _presentOrtho;
////    MTL::Buffer*                        _pUniformBuffer;
////    NS::SharedPtr<MTL::Texture>         _pBackbuffer;
////    NS::SharedPtr<MTL::Texture>         _pUpscaledbuffer;
////    NS::SharedPtr<MTL::Texture>         _pBackbufferAdapter;
////    NS::SharedPtr<MTL::Texture>         _pUpscaledbufferAdapter;
////    int            _highScore;
////    int            _prevScore;
//    float          _maxEDRValue;
//    float          _brightness;
//    float          _edrBias;
////    uint64_t                            _pacingTimeStampIndex;
////    MTL::SamplerState*                  _pSampler;
////    std::unordered_map<std::string, NS::SharedPtr<MTL::Texture>> _textureAssets;
////    MTL::RenderPipelineState*           _pPresentPipeline;
////    MTL::RenderPipelineState*           _pInstancedSpritePipeline;
////    NS::SharedPtr<MTLFX::SpatialScaler> _pSpatialScaler;
////    MTL::PixelFormat                    _layerPixelFormat;
////    MTL::RenderPipelineState*           _pMapPSO;
////    MTL::RenderPipelineState*           _pCameraPSO;
////    MTL::ComputePipelineState*          _pComputePSO;
////    MTL::Buffer*                        _pVertexDataBuffer;
//    //MTL::Buffer*                _pCameraDataBuffer[kMaxFramesInFlight];
////    MTL::Buffer*                _pIndexBuffer;
////    MTL::Buffer*                _pTextureAnimationBuffer;
////    float                       _angle;
////    int                         _frameP;
////    uint                        _animationIndex;
////    NS::SharedPtr<MTL::Texture>         _pUpscaledbufferAdapterP;
////    MTL::Buffer* _pVertexDataBufferMap;
////    MTL::Buffer* _pIndexBufferMap;
//};

#endif /* RMDLRENDERERSPAMMY_HPP */
