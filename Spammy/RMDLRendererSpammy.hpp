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
#include "RMDLSnow.hpp"
#include "RMDLMainRenderer_shared.h"
#include "RMDLCamera.hpp"
#include "RMDLUtils.hpp"
#include "RMDLFontLoader.h"
#include "RMDLMeshUtils.hpp"
#include "RMDLPhaseAudio.hpp"
//#include "BumpAllocator.hpp"
#include "RMDLMathUtils.hpp"
#include "RMDLBlender.hpp"
#include "RMDLUi.hpp"
#include "VoronoiVoxel4D.hpp"
#include "Utils/NonCopyable.h"
#include "RMDLColors.hpp"

#define kMaxBuffersInFlight 3

static const uint32_t NumLights = 256;

struct Vertex {
    simd::float3 pos;
    simd::float4 color;
};

struct TriangleData
{
    VertexData vertex0;
    VertexData vertex1;
    VertexData vertex2;
};

class GameCoordinator : NonCopyable
{
public:
    GameCoordinator(MTL::Device* device, MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, const std::string& resourcePath);
    ~GameCoordinator();
    
    void setViewSize(int width, int height);
    void setViewportWindow(NS::UInteger width, NS::UInteger height);
    void makeArgumentTable();
    void buildDepthStencilStates(NS::UInteger width, NS::UInteger height);

    void handleMouseMove(float x, float y);
    void handleMouseDown(bool rightClick);
    void handleMouseUp();
    void handleScroll(float deltaY);
    void handleKeyPress(int key);

    void playSoundTestY();
    void loadGameSounds(const std::string& resourcePath, PhaseAudio* audioEngine);
    void loadPngAndFont(const std::string& resourcePath);
    void moveCamera(simd::float3 translation);
    void rotateCamera(float deltaYaw, float deltaPitch);
    void draw(MTK::View* view);
    void resizeMtkViewAndUpdateViewportWindow(NS::UInteger width, NS::UInteger height);

private:
    MTL::Device*                m_device;
    MTL::CommandQueue*          m_commandQueue;
    MTL::RenderPipelineState*   m_renderPipelineState;
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    MTL::Buffer* transformBuffer;
    MTL::Buffer*                m_viewportSizeBuffer;
    MTL::Library*               m_shaderLibrary;
    MTL::Viewport               m_viewport;

    simd::float2 cursorPos;
    simd_uint2                          m_viewportSize;
    float                       _rotationAngle;

    uint64_t                            m_frame;
    RMDLCamera                          m_camera;
    RMDLCamera                          m_cameraPNJ;
    MTL::PixelFormat                    m_pixelFormat;
    MTL::PixelFormat                    m_depthPixelFormat;
    MTL::DepthStencilState*             m_depthStencilState;
    PhaseAudio*                             pAudioEngine;
    std::unique_ptr<PhaseAudio> _pAudioEngine;
    RMDLCameraUniforms                  m_cameraUniforms;
    bool DoTheImportThing(const std::string& resourcePath);
    RMDLBlender blender;
    sky::RMDLSkybox skybox;
    snow::RMDLSnow snow;
    VoxelWorld world;
    MetalUIManager ui;
    MTL::Texture* m_terrainTexture;
    VibrantColorRenderer    colorsFlash;
    MTL::TextureDescriptor*             m_depthTextureDescriptor;
    MTL::Texture*                       m_depthTexture;
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

class RMDLRendererSpammy : NonCopyable
{
public:
    RMDLRendererSpammy(MTL::Device* pDevice, MTL::PixelFormat pixelFormat, NS::UInteger width, NS::UInteger heigth, const std::string& resourcePath);
    ~RMDLRendererSpammy();

    void loadPngAndFont(const std::string& resourcePath);
    void loadSoundMp3(const std::string& resourcePath, PhaseAudio* audioEngine);
    void makeArgumentTable();
    void buildDepthStencilStates(NS::UInteger width, NS::UInteger height);
    void setViewportWindow(NS::UInteger width, NS::UInteger height);
    
    void draw(MTK::View* pView);
    void resizeMtkViewAndUpdateViewportWindow(NS::UInteger width, NS::UInteger height);
private:
    MTL::Device*                        _pDevice;
    MTL::Buffer*                        _pViewportSizeBuffer;
    MTL::Buffer*                        _pABuffer[kMaxBuffersInFlight];
    MTL::Buffer*                        _pBuffer[kMaxBuffersInFlight];
    MTL::Buffer*                        _pCBuffer[kMaxBuffersInFlight];
    MTL::Texture*                       _pTexture;
    MTL::Texture*                       _pTextureNormalShadow_GBuffer;
    MTL::Texture*                       _pTextureAlbedoSpectacular_GBuffer;
    MTL::Library*                       _pShaderLibrary;
    MTL::Viewport                       _viewport;
    MTL::PixelFormat                    _pixelFormat;
    MTL::PixelFormat                    m_depthPixelFormat{MTL::PixelFormatDepth32Float};
    MTL::PixelFormat                    _normalShadowPixelFormat_GBuffer;
    MTL::PixelFormat                    _albedoSpectacularPixelFormat_GBuffer;
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

#endif /* RMDLRENDERERSPAMMY_HPP */
