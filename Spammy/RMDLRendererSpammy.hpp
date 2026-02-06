/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLRendererSpammy.hpp           +++     +++   */
/*                                        +       +          */
/*      By: Laboitederemdal                +       +         */
/*                                       +           +       */
/*      Created: 27/10/2025 15:45:19      + + + + + +        */
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
#include "RMDLMathUtils.hpp"
#include "RMDLBlender.hpp"
#include "RMDLUi.hpp"
#include "VoronoiVoxel4D.hpp"
#include "Utils/NonCopyable.h"
#include "RMDLColors.hpp"

#include "RMDLGrid.hpp"
#include "RMDLSystem.hpp"

#include "RMDLMouseAndCursor.hpp"
#include "RMDLAVFAudio.hpp"
#include "RMDLManager.hpp"
#include "RMDLInventory.hpp"

#include "RMDLMap.hpp"
//#include "RMDLPhysics.hpp"

#include "RMDLMotherCube.hpp"

#include "RMDLFab3DUI.hpp"

#define kMaxBuffersInFlight 3

static const uint32_t NumLights = 256;

enum class GamePlayMode {
    FreeCam, Driving, Building, DEV, Flight, FAB
};

struct VertexCursor
{
    vector_float2 cursorPositionCompare;
};

struct TriangleData
{
    VertexData vertex0;
    VertexData vertex1;
    VertexData vertex2;
};

struct InputState
{
    // Mouvement (valeurs normalisées -1 à 1)
    simd::float3 moveDirection {0, 0, 0};
    simd::float2 lookDelta {0, 0};
    
    float speedMultiplier = 2.0f;
    
    // Actions ponctuelles (consommées après lecture)
    bool jump = false;
    bool interact = false;
    bool inventory = false;
    
    void reset()
    {
        moveDirection = {0, 0, 0};
        lookDelta = {0, 0};
        speedMultiplier = 1.0f;
    }
    
    void consumeActions()
    {
        jump = false;
        interact = false;
        inventory = false;
    }
};

struct RenderContext {
    MTL::Device*      device; // 8 bytes
    MTL::Library*     library; // 8 si déjà chargée
    MTL::PixelFormat  colorFormat; // 4
    MTL::PixelFormat  depthFormat;
};

//BlackHole::BlackHole(const RenderContext& ctx)
//    : m_device(ctx.device->retain())
//{
//    buildPipeline(ctx);
//}

class GameCoordinator : NonCopyable
{
public:
    GameCoordinator(MTL::Device* device, MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, const std::string& resourcePath);
    ~GameCoordinator();
    
    void setViewSize(int width, int height);
    void setViewportWindow(NS::UInteger width, NS::UInteger height);
    void makeArgumentTable();
    void buildDepthStencilStates(NS::UInteger width, NS::UInteger height);
    
    GamePlayMode m_gamePlayMode;
    InputState  m_input;
    
    void update(float deltaTime, const InputState& input, MTL::CommandBuffer* commandBuffer);

    void handleMouseMove(float x, float y);
    void handleMouseDown(bool rightClick);
    void handleMouseUp();
    void handleScroll(float deltaY);
    void handleKeyPress(int key);
    
    void setMousePosition(float x, float y);
    
    void toggleEditMode() { m_editMode = !m_editMode; }
    void toggleBuildMode() { m_buildMode = !m_buildMode; }
    
    void makeTexture(MTL::PixelFormat layerPixelFormat, MTL::PixelFormat depthPixelFormat);
    
    void createCursor(float mouseX, float mouseY, float width, float height, float size, std::vector<VertexCursor>& outVertices);
    void createBuffers();
    
    void inventory(bool visible);
    void jump();
    
    void addBlockToVehicle(int blockId, BlockType type);
    void removeBlockFromVehicle(int blockId);

//    void setGameMode(GameMode mode);
////    void toggleBuildMode();
//    void placeBlock();
//    void removeBlock();
//    simd_float3 screenToWorldRay(float screenX, float screenY);
    
    void setGamePlayMode(GamePlayMode mode);
    void toggleVehicleBuildMode();
    void selectVehicleSlot(int slot);
    void rotateVehicleGhost();
    void vehicleMouseDown(bool rightClick);
    void vehicleMouseUp();
    
    void setInventory();
    void setInventoryBest();
    
    void onMouseDragged(simd::float2 screenPos, simd::float2 screenSize, simd::float2 delta, int button);
    
    void playCinematic();
    
    
    void fabPanelKey(uint16_t keyCode) {
        if (m_fabPanel && m_fabPanel->visible) {
            m_fabPanel->handleKey(keyCode);
        }
    }

    void fabPanelDrag(float dx, float dy) {
        if (m_fabPanel && m_fabPanel->visible) {
            m_fabPanel->cameraYaw += dx;
            m_fabPanel->cameraPitch = std::clamp(m_fabPanel->cameraPitch + dy, -1.5f, 1.5f);
        }
    }

    void fabPanelScroll(float delta) {
        if (m_fabPanel && m_fabPanel->visible) {
            m_fabPanel->cameraZoom = std::clamp(m_fabPanel->cameraZoom + delta * 0.1f, 0.3f, 3.0f);
        }
    }

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
    MTL::RenderPipelineState*           m_renderPipelineState = nullptr;
    MTL::Buffer*                        m_mouseBuffer = nullptr;
    MTL::Buffer*                        vertexBuffer = nullptr;
    MTL::Buffer* indexBuffer;
    MTL::Buffer* transformBuffer;
    MTL::Buffer*                        m_viewportSizeBuffer;
    MTL::Library*                       m_shaderLibrary;
    MTL::Viewport                       m_viewport;
    MTL::ComputePipelineState*          m_mousePositionComputeKernel;
    
    uint64_t                            m_frame;
    simd::float2                        cursorPosition;
    
    simd_uint2                          m_viewportSize;
    float                       _rotationAngle;

    
    MTL::Texture*                        m_gBuffer0;
    MTL::Texture*                        m_gBuffer1;
    MTL::Texture*                        m_shadow;
    NS::SharedPtr<MTL::Texture>                        m_depth;

    MTL::Buffer*                        m_textureBuffer;
    
    MTL::RenderPipelineState*          m_lightingPpl;
    NS::SharedPtr<MTL::RenderPassDescriptor>        _gBufferPassDesc;
    NS::SharedPtr<MTL::RenderPassDescriptor>         _shadowPassDesc;
    NS::SharedPtr<MTL::RenderPassDescriptor>         _lightingPassDesc;
    NS::SharedPtr<MTL::RenderPassDescriptor>         _gBufferWithLoadPassDesc;
    
    MTL::Buffer*                            m_gpuUniforms[kMaxBuffersInFlight];
    int j = 1;
    
    MTL::DepthStencilState*             _shadowDepthState;
    MTL::DepthStencilState*             _gBufferDepthState;
    MTL::DepthStencilState*             _lightingDepthState;
    MTL::ComputePipelineState*          _pipelineStateDescriptor;
    

//    simd::float3        m_lastValidGridCenter;
    
    std::unique_ptr<SpaceshipAudioEngine> m_spaceAudio;
    std::unordered_map<int, int> m_blockVoices;  // blockId -> voiceId

    
    RMDLCamera                          m_camera;
    RMDLCamera                          m_cameraOrtho;
    RMDLCamera                          m_cameraPNJ;
    RMDLCamera                          m_cameraShadow;
    RMDLCameraSnapshot cinematicView = {
        .position = {30, 1000, 30},
        .direction = simd::normalize(simd::float3{-1, -0.2f, -1}),
        .up = {0, 1, 0},
        .viewAngle = M_PI / 5.0f,
        .nearPlane = 0.1f,
        .farPlane = 500.0f
    };
    MTL::PixelFormat                    m_pixelFormat;
    MTL::PixelFormat                    m_depthPixelFormat;
    MTL::DepthStencilState*             m_depthStencilState;
    PhaseAudio*                             pAudioEngine;
    std::unique_ptr<PhaseAudio> _pAudioEngine;
    RMDLCameraUniforms                  m_cameraUniforms;
    RMDLUniforms                        m_uniforms;
    bool DoTheImportThing(const std::string& resourcePath);
    RMDLBlender blender;
    sky::RMDLSkybox skybox;
    VoxelWorld world;
    MetalUIManager ui;
    MTL::Texture* m_terrainTexture;
    VibrantColorRenderer    colorsFlash;
    MTL::TextureDescriptor*             m_depthTextureDescriptor;
    MTL::Texture*                       m_depthTexture;
    MouseDepthPicker    mouseAndCursor;
    BuildGrid           grid;
    int testTransitionCamera = 0;
    
    skybox::BlackHole   blackHole;
    
    simd::float3 snappedPos;
    simd::float2 screenSz;
    
    GridCommandant::VehicleBuildGrid    gridCommandant;
    
    TerraVehicle::VehicleManager m_terraVehicle;
    simd::float3 m_currentVehicleBlockPos = {0.f, 0.f, 0.f};
    simd::float4x4 m_currentVehicleRotation = matrix_identity_float4x4;
    
    std::unique_ptr<RenderSystem>     _renderSystem;
    std::unique_ptr<TerrainManager>     _terrainManager;
    std::unique_ptr<PhysicsSystem>     _physicsSystem;
    

    simd::float3 newPosition;
    bool m_editMode;
    bool m_buildMode;
    simd::float3 m_raycastHitPoint;
    simd::int3 m_selectedVoxel;
    dispatch_semaphore_t                _semaphore;
    
    void updateUniforms();
    
    TerrainConfigLisse  configLisse;
    InfiniteTerrainManager   terrainLisse;
    
    
    inventoryWindow::texture2d standardPNG();
    std::unordered_map<std::string, NS::SharedPtr<MTL::Texture>> _textureAssets;

    
    cube::BlockSystem   blocs;
    
    
    inventoryWindow::InventoryPanel m_inventoryPanel;
    
//    FabUI  fabUI;
    inventoryWindow::FabPanel3D* m_fabPanel = nullptr;
//    NASAAtTheHelm::VehicleManager vehicleManager;
//    NASAAtTheHelm::InventoryUIRenderer inventoryUI;
//    GameMode m_gameMode = GameMode::FreeCam;
//    // Position du bloc en cours de placement
//    simd_float3 m_buildPreviewPos = {0, 0, 0};
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
    
    MTL4::CommandQueue*                 _pCommandQueue;
    MTL4::ArgumentTable*                _pArgumentTable;
    MTL4::CommandBuffer*                _pCommandBuffer[5];
    MTL4::CommandAllocator*             _pCommandAllocator[kMaxBuffersInFlight];
    MTL4::RenderPassDescriptor*         _gBufferRenderPassDesc;
    MTL4::RenderPassDescriptor*         _zBufferRenderPassDesc;
    NS::SharedPtr<MTL::SharedEvent>     _pPacingEvent;
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
