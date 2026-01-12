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

#include "RMDLGrid.hpp"
#include "RMDLSystem.hpp"
#include "RMDLInventory.hpp"
#include "RMDLPhysics.hpp"

#define kMaxBuffersInFlight 3

static const uint32_t NumLights = 256;

struct TriangleData
{
    VertexData vertex0;
    VertexData vertex1;
    VertexData vertex2;
};

class GameCoordinatorExtensions {
public:
    // À ajouter dans le constructeur de GameCoordinator
    static void initializeSystems(
        MTL::Device* device,
        MTL::PixelFormat colorFormat,
        MTL::PixelFormat depthFormat,
        MTL::Library* library,
        float screenWidth,
        float screenHeight,
        std::unique_ptr<RMDLGrid>& voxelGrid,
        std::unique_ptr<TerrainSystem>& terrainSystem,
        std::unique_ptr<PhysicsSystem>& physicsSystem,
        std::unique_ptr<InventorySystem>& inventorySystem
    ) {
        // Initialize voxel system
        voxelGrid = std::make_unique<RMDLGrid>(device, colorFormat,depthFormat,screenWidth, screenHeight, library);
        
        // Initialize terrain with seed
        terrainSystem = std::make_unique<TerrainSystem>(device, colorFormat, depthFormat, library);
        terrainSystem->initialize(89); // Votre seed préféré
        terrainSystem->setHeightScale(1.0f);
        terrainSystem->setRenderDistance(500.0f);
        
        // Initialize physics
        physicsSystem = std::make_unique<PhysicsSystem>(device, library);
        physicsSystem->setGravity({0.0f, -20.0f, 0.0f});
        physicsSystem->setTerrainSystem(terrainSystem.get());
        physicsSystem->setVoxelGrid(voxelGrid.get());
        
        // Initialize inventory
        inventorySystem = std::make_unique<InventorySystem>(device, library,
                                                           screenWidth, screenHeight);
    }
    
    // À appeler dans update()
    static void updateSystems(
        float deltaTime,
        const simd::float3& cameraPosition,
        MTL::Device* device,
        RMDLGrid* voxelGrid,
        TerrainSystem* terrainSystem,
        PhysicsSystem* physicsSystem,
        InventorySystem* inventorySystem
    ) {
        // Update terrain (streaming, LOD)
        terrainSystem->update(deltaTime, cameraPosition);
        
        // Update physics
        physicsSystem->update(deltaTime);
        // ou physicsSystem->updateGPU(deltaTime); pour la version GPU
        
        // Update inventory UI
        inventorySystem->updateUI(deltaTime);
        
        // Update voxel chunks around camera
        voxelGrid->updateChunks(cameraPosition);
    }
    
    // À appeler dans draw()
    static void renderSystems(
        MTL::RenderCommandEncoder* encoder,
        const simd::float4x4& viewProjection,
        const simd::float3& cameraPosition,
        const simd::float2& screenSize,
        bool editMode,
        RMDLGrid* voxelGrid,
        TerrainSystem* terrainSystem,
        PhysicsSystem* physicsSystem,
        InventorySystem* inventorySystem
    ) {
        // Render terrain first (opaque)
        terrainSystem->render(encoder, viewProjection);
        
        // Render voxel grid
        voxelGrid->renderChunks(encoder, viewProjection);
        
        // Render grid edges in edit mode
        if (editMode) {
            voxelGrid->setEditMode(true);
            voxelGrid->renderGridEdges(encoder, viewProjection);
        }
        
        // Render vehicle if in build mode
        if (inventorySystem->isVehicleBuildMode()) {
            auto& vehicle = physicsSystem->getVehicle();
            inventorySystem->renderVehicleGrid(encoder, viewProjection,
                                              vehicle.entity.position);
        }
        
        // Render UI last (with alpha blending)
        inventorySystem->renderUI(encoder, screenSize);
    }
    
    // Gestion des inputs - à appeler depuis handleKeyPress
    static void handleGameInput(
        int key,
        bool editMode,
        RMDLGrid* voxelGrid,
        PhysicsSystem* physicsSystem,
        InventorySystem* inventorySystem,
        bool& editModeOut,
        bool& buildModeOut
    ) {
        switch (key) {
            case 0: // A - Move left
            case 1: // S - Move backward
            case 2: // D - Move right
            case 13: // W - Move forward
            {
                // Calculate movement direction from keys
                simd::float3 moveDir = {0, 0, 0};
                // ... calculer moveDir selon les touches
                
                bool run = false; // Détecté par Shift
                bool jump = false; // Barre espace
                bool crouch = false; // Ctrl
                
                physicsSystem->applyPlayerInput(moveDir, jump, crouch, run);
                break;
            }
            
            case 14: // E - Toggle edit mode
                editModeOut = !editMode;
                voxelGrid->setEditMode(editModeOut);
                break;
                
            case 17: // T - Toggle Tab inventory
                inventorySystem->toggleInventory();
                break;
                
            case 11: // B - Toggle build mode
                buildModeOut = !buildModeOut;
                inventorySystem->setVehicleBuildMode(buildModeOut);
                break;
        }
    }
    
    // Gestion du placement de blocs
    static void handleBlockPlacement(
        const simd::float3& cameraPosition,
        const simd::float3& cameraDirection,
        bool placeBlock, // true = place, false = remove
        RMDLGrid* voxelGrid,
        InventorySystem* inventorySystem,
        PhysicsSystem* physicsSystem
    ) {
        // Raycast depuis la caméra
        auto rayResult = physicsSystem->raycast(cameraPosition, cameraDirection, 10.0f);
        
        if (rayResult.hit) {
            // Get selected item from inventory
            auto* selectedItem = inventorySystem->getSelectedItem();
            
            if (placeBlock && selectedItem && !selectedItem->isEmpty()) {
                // Place block at raycast hit
                auto voxelHit = voxelGrid->raycast(cameraPosition, cameraDirection, 10.0f);
                if (voxelHit.hit) {
                    Voxel newVoxel;
                    newVoxel.setActive(true);
                    newVoxel.setVisible(true);
                    newVoxel.blockType = (uint16_t)selectedItem->type;
                    
                    // Place adjacent to hit voxel
                    voxelGrid->setVoxel(voxelHit.adjacentPos.x,
                                       voxelHit.adjacentPos.y,
                                       voxelHit.adjacentPos.z,
                                       newVoxel);
                    
                    // Remove from inventory
                    inventorySystem->removeItem(selectedItem->type, 1);
                    
                    // Regenerate mesh
                    voxelGrid->generateMesh();
                    voxelGrid->uploadToGPU();
                }
            } else if (!placeBlock) {
                // Remove block
                auto voxelHit = voxelGrid->raycast(cameraPosition, cameraDirection, 10.0f);
                if (voxelHit.hit) {
                    Voxel* voxel = voxelGrid->getVoxel(voxelHit.voxelPos.x,
                                                       voxelHit.voxelPos.y,
                                                       voxelHit.voxelPos.z);
                    if (voxel && voxel->isActive()) {
                        // Add to inventory
                        inventorySystem->addItem((ItemType)voxel->blockType, 1);
                        
                        // Remove voxel
                        Voxel emptyVoxel;
                        voxelGrid->setVoxel(voxelHit.voxelPos.x,
                                          voxelHit.voxelPos.y,
                                          voxelHit.voxelPos.z,
                                          emptyVoxel);
                        
                        voxelGrid->generateMesh();
                        voxelGrid->uploadToGPU();
                    }
                }
            }
        }
    }
    
    // Gestion de la construction du véhicule
    static void handleVehicleConstruction(
        const simd::float3& cameraPosition,
        const simd::float3& cameraDirection,
        bool placeBlock,
        InventorySystem* inventorySystem,
        PhysicsSystem* physicsSystem
    ) {
        if (!inventorySystem->isVehicleBuildMode()) return;
        
        // TODO: Raycast dans la grille du véhicule
        // Placer/retirer des blocs dans VehicleGrid
        
        auto* selectedItem = inventorySystem->getSelectedItem();
        if (placeBlock && selectedItem && !selectedItem->isEmpty()) {
            // Place block on vehicle grid at raycasted position
            simd::int3 gridPos = {8, 8, 8}; // Example position
            inventorySystem->placeBlockOnVehicle(gridPos, selectedItem->type);
        } else if (!placeBlock) {
            // Remove block from vehicle
            simd::int3 gridPos = {8, 8, 8};
            inventorySystem->removeBlockFromVehicle(gridPos);
        }
    }
    
    // Sauvegarde complète
    static bool saveGame(
        const std::string& savePath,
        RMDLGrid* voxelGrid,
        InventorySystem* inventorySystem,
        PhysicsSystem* physicsSystem
    ) {
        // Save voxel grid
        if (!voxelGrid->saveToFile(savePath + "/voxels.dat")) {
            return false;
        }
        
        // Save inventory and vehicle
        if (!inventorySystem->saveToFile(savePath + "/inventory.dat")) {
            return false;
        }
        
        // Save player position
        std::ofstream playerFile(savePath + "/player.dat", std::ios::binary);
        if (!playerFile.is_open()) return false;
        
        auto& player = physicsSystem->getPlayer();
        playerFile.write(reinterpret_cast<const char*>(&player.entity.position),
                        sizeof(simd::float3));
        playerFile.write(reinterpret_cast<const char*>(&player.entity.velocity),
                        sizeof(simd::float3));
        playerFile.close();
        
        return true;
    }
    
    // Chargement complet
    static bool loadGame(
        const std::string& savePath,
        RMDLGrid* voxelGrid,
        InventorySystem* inventorySystem,
        PhysicsSystem* physicsSystem
    ) {
        // Load voxel grid
        if (!voxelGrid->loadFromFile(savePath + "/voxels.dat")) {
            return false;
        }
        
        // Load inventory
        if (!inventorySystem->loadFromFile(savePath + "/inventory.dat")) {
            return false;
        }
        
        // Load player
        std::ifstream playerFile(savePath + "/player.dat", std::ios::binary);
        if (!playerFile.is_open()) return false;
        
        auto& player = physicsSystem->getPlayer();
        playerFile.read(reinterpret_cast<char*>(&player.entity.position),
                       sizeof(simd::float3));
        playerFile.read(reinterpret_cast<char*>(&player.entity.velocity),
                       sizeof(simd::float3));
        playerFile.close();
        
        return true;
    }
};
/*
 *     GameCoordinatorExtensions::updateSystems(
 *         dt, m_camera.position(), m_device,
 *         m_voxelGrid.get(), m_terrainSystem.get(),
 *         m_physicsSystem.get(), m_inventorySystem.get()
 *     );
 *
 *     // Update camera from player physics
 *     auto& player = m_physicsSystem->getPlayer();
 *     m_camera.setPosition(player.entity.position + simd::float3{0, 1.6f, 0}); // Eye height
 *
 *     // Draw everything
 *     GameCoordinatorExtensions::renderSystems(
 *         enc, m_cameraUniforms.viewProjectionMatrix,
 *         m_camera.position(),
 *         {(float)m_viewport.width, (float)m_viewport.height},
 *         m_editMode,
 *         m_voxelGrid.get(), m_terrainSystem.get(), m_inventorySystem.get()
 *     );
 */

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
    
    void toggleEditMode() { m_editMode = !m_editMode; }
    void toggleBuildMode() { m_buildMode = !m_buildMode; }

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
    
    std::unique_ptr<RMDLGrid> m_voxelGrid;
    std::unique_ptr<TerrainSystem> m_terrainSystem;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<InventorySystem> m_inventorySystem;
    bool m_editMode;
    bool m_buildMode;
    simd::float3 m_raycastHitPoint;
    simd::int3 m_selectedVoxel;
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
