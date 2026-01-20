//
//  RMDLManager.hpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#ifndef RMDLManager_hpp
#define RMDLManager_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <memory>
#include <vector>

//#include "RMDLNeedNasa.hpp"
//#include "RMDLPetitPrince.hpp"

#include "RMDLMathUtils.hpp"

#include <unordered_map>
#include <string>
#include <functional>

namespace TerraVehicle {

constexpr uint32_t MAX_VEHICLE_BLOCKS = 128;
constexpr uint32_t INVENTORY_ROWS = 4;
constexpr uint32_t INVENTORY_COLS = 10;
constexpr uint32_t INVENTORY_SIZE = INVENTORY_ROWS * INVENTORY_COLS;
constexpr float BLOCK_UNIT = 1.0f;

enum class BlockCategory : uint8_t {
    Core = 0,       // Commandant
    Mobility,       // Roues, chenilles, thrusters
    Combat,         // Armes
    Utility,        // Générateurs, stockage
    Structure,      // Armure, connecteurs
    COUNT
};

enum class AttachFace : uint8_t {
    PosX = 0, NegX, PosY, NegY, PosZ, NegZ, FACE_COUNT
};

struct BlockGPUVertex
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 uv;
    simd::float4 color;
};

struct BlockGPUInstance
{
    simd::float4x4 modelMatrix;
    simd::float4 tint;
    uint32_t typeID;
    uint32_t state;      // 0=normal, 1=ghost, 2=damaged
    float healthRatio;
    float _pad;
};

struct VehicleGPUUniforms
{
    simd::float4x4 viewProjection;
    simd::float3 cameraPos;
    float time;
    simd::float3 sunDir;
    float _pad;
};

struct InventoryGPUUniforms
{
    simd::float2 screenSize;
    simd::float2 position;
    simd::float2 panelSize;
    simd::float2 slotSize;
    float slotPadding;
    float time;
    int32_t hoveredSlot;
    int32_t selectedSlot;
};

struct InventorySlotGPU
{
    simd::float4 iconColor;
    uint32_t count;
    uint32_t typeID;
    uint32_t flags;      // 1=empty, 2=selected, 4=hovered
    float _pad;
};

struct BlockDefinition
{
    uint32_t typeID;
    std::string name;
    std::string description;
    BlockCategory category;
    float mass;
    float maxHealth;
    float energyUse;
    float energyGen;
    simd::float3 size;
    simd::float4 baseColor;
    uint32_t meshID;
    
    BlockDefinition();
    BlockDefinition(uint32_t id, const std::string& n, BlockCategory cat,
                    float m, float hp, simd::float4 col);
};


struct AttachPoint
{
    simd::float3 localOffset;
    simd::float3 normal;
    AttachFace face;
    bool occupied;
    uint32_t connectedID;
    
    AttachPoint();
    AttachPoint(simd::float3 off, simd::float3 n, AttachFace f);
};

class BlockInstance
{
public:
    uint32_t instanceID;
    uint32_t definitionID;
    simd::float3 localPosition;
    simd::float4x4 localRotation;
    float currentHealth;
    bool destroyed;
    AttachPoint attachPoints[6];
    
    BlockInstance();
    BlockInstance(uint32_t instID, uint32_t defID);
    
    void initAttachPoints();
    simd::float4x4 computeWorldMatrix(simd::float3 vehiclePos, simd::float4x4 vehicleRot) const;
};

class CommanderBlock : public BlockInstance
{
public:
    float energyCapacity;
    float currentEnergy;
    
    // Caméra 3ème personne
    float camOrbitYaw;
    float camOrbitPitch;
    float camOrbitDist;
    
    CommanderBlock();
    CommanderBlock(uint32_t instID);
    
    simd::float3 computeCameraPosition(simd::float3 vehiclePos, simd::float4x4 vehicleRot) const;
    simd::float3 computeCameraTarget(simd::float3 vehiclePos) const;
};

// ============================================================================
// WHEEL BLOCK
// ============================================================================
class WheelBlock : public BlockInstance {
public:
    float radius;
    float torque;
    float spinAngle;
    float steerAngle;
    bool isPowered;
    bool canSteer;
    
    WheelBlock();
    WheelBlock(uint32_t instID);
    
    simd::float3 computeDriveForce(float throttle, simd::float4x4 vehicleRot) const;
    void updateSpin(float vehicleSpeed, float dt);
};

// ============================================================================
// VEHICLE
// ============================================================================
class Vehicle {
public:
    uint32_t vehicleID;
    std::string name;
    
    // Blocs
    std::unique_ptr<CommanderBlock> commander;
    std::unordered_map<uint32_t, std::unique_ptr<BlockInstance>> blocks;
    uint32_t nextBlockID;
    
    // Physique
    simd::float3 position;
    simd::float4x4 rotationMatrix;
    simd::float3 velocity;
    simd::float3 angularVelocity;
    float totalMass;
    simd::float3 centerOfMass;
    
    // Inputs
    float inputThrottle;
    float inputSteering;
    float inputBrake;
    bool isGrounded;
    
    Vehicle();
    Vehicle(uint32_t id);
    
    void initialize();
    void updatePhysics(float dt);
    void recalculateMass();
    
    bool attachBlock(std::unique_ptr<BlockInstance> block, uint32_t parentID,
                     AttachFace parentFace, AttachFace childFace);
    bool detachBlock(uint32_t blockID);
    
    // Caméra
    simd::float3 getCameraPosition() const;
    simd::float3 getCameraTarget() const;
    void orbitCamera(float dYaw, float dPitch);
    void zoomCamera(float delta);
    
private:
    float computeLowestPoint() const;
};

// ============================================================================
// INVENTORY SLOT
// ============================================================================
struct InventorySlot {
    uint32_t definitionID;
    uint32_t quantity;
    
    InventorySlot();
    bool isEmpty() const;
};

// ============================================================================
// INVENTORY
// ============================================================================
class Inventory {
public:
    InventorySlot slots[INVENTORY_SIZE];
    int32_t selectedSlot;
    int32_t hoveredSlot;
    bool isOpen;
    
    // Position du panneau (draggable)
    simd::float2 panelPosition;
    bool isDraggingPanel;
    simd::float2 dragOffset;
    
    Inventory();
    
    void setDefaultItems();
    bool consumeSlot(int32_t slot);
    bool addItem(uint32_t defID, uint32_t count);
    int32_t hitTestSlot(simd::float2 mouseNorm, simd::float2 screenSize) const;
    bool hitTestPanel(simd::float2 mouseNorm, simd::float2 screenSize) const;
    
    simd::float2 getSlotScreenPos(int32_t slot, simd::float2 screenSize) const;
};

// ============================================================================
// DRAG & DROP SYSTEM
// ============================================================================
class BuildDragDrop {
public:
    enum class Mode { Idle, FromInventory, Placing };
    
    Mode mode;
    int32_t sourceSlot;
    uint32_t draggedDefID;
    
    // Ghost placement
    simd::float3 ghostLocalPos;
    simd::float4x4 ghostRotation;
    uint32_t targetBlockID;
    AttachFace targetFace;
    AttachFace ghostFace;
    bool validPlacement;
    
    BuildDragDrop();
    
    void startDrag(Inventory& inv, int32_t slot);
    bool finishDrag(Vehicle& vehicle, Inventory& inv, const std::vector<BlockDefinition>& defs);
    void cancelDrag();
    void cycleGhostRotation();
    void updateGhostPlacement(Vehicle& vehicle, simd::float3 rayOrigin, simd::float3 rayDir);
};

// ============================================================================
// BLOCK REGISTRY (définitions de tous les types de blocs)
// ============================================================================
class BlockRegistry {
public:
    std::vector<BlockDefinition> definitions;
    
    BlockRegistry();
    
    void registerDefaults();
    const BlockDefinition* getDefinition(uint32_t typeID) const;
    std::unique_ptr<BlockInstance> createInstance(uint32_t defID, uint32_t instanceID) const;
};

// ============================================================================
// VEHICLE RENDERER
// ============================================================================
class VehicleRenderer {
public:
    VehicleRenderer(MTL::Device* device, MTL::PixelFormat colorFmt,
                    MTL::PixelFormat depthFmt, MTL::Library* library);
    ~VehicleRenderer();
    
    void render(MTL::RenderCommandEncoder* enc, Vehicle& vehicle, BuildDragDrop& drag,
                const BlockRegistry& registry, simd::float4x4 vpMatrix,
                simd::float3 camPos, float time);
    
private:
    MTL::Device* m_device;
    MTL::RenderPipelineState* m_solidPipeline;
    MTL::RenderPipelineState* m_ghostPipeline;
    MTL::DepthStencilState* m_depthState;
    MTL::Buffer* m_vertexBuffer;
    MTL::Buffer* m_indexBuffer;
    MTL::Buffer* m_instanceBuffer;
    MTL::Buffer* m_uniformBuffer;
    uint32_t m_indexCount;
    
    void buildPipeline(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library);
    void buildBlockMesh();
};

// ============================================================================
// INVENTORY RENDERER
// ============================================================================
class InventoryRenderer {
public:
    InventoryRenderer(MTL::Device* device, MTL::PixelFormat colorFmt,
                      MTL::PixelFormat depthFmt, MTL::Library* library);
    ~InventoryRenderer();
    
    void render(MTL::RenderCommandEncoder* enc, Inventory& inv,
                const BlockRegistry& registry, simd::float2 screenSize, float time);
    
private:
    MTL::Device* m_device;
    MTL::RenderPipelineState* m_pipeline;
    MTL::Buffer* m_vertexBuffer;
    MTL::Buffer* m_uniformBuffer;
    MTL::Buffer* m_slotBuffer;
    
    void buildPipeline(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library);
    void buildQuadMesh();
};


class VehicleManager
{
public:
    VehicleManager(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~VehicleManager();

    void cleanup();
    
    void update(float dt);
    void render(MTL::RenderCommandEncoder* renderCommandEncoder, simd::float4x4 viewProjectionMatrix, simd::float3 cameraPosition);
    void renderUI(MTL::RenderCommandEncoder* enc, simd::float2 screenSize);
    
    // Contrôles véhicule
    void setThrottle(float val);
    void setSteering(float val);
    void setBrake(float val);
    
    // Caméra
    simd::float3 getCameraPosition() const;
    simd::float3 getCameraTarget() const;
    void orbitCamera(float dYaw, float dPitch);
    void zoomCamera(float delta);
    
    // Mode construction
    void toggleBuildMode();
    bool isBuildMode() const;
    void rotateGhostBlock();
    void selectInventorySlot(int32_t slot);
    
    // Input
    void onMouseDown(simd::float2 normPos, simd::float2 screenSize, bool rightClick);
    void onMouseUp(simd::float2 normPos, simd::float2 screenSize);
    void onMouseMove(simd::float2 normPos, simd::float2 screenSize,
                     simd::float3 rayOrigin, simd::float3 rayDir);
    
private:
    std::unique_ptr<Vehicle> m_vehicle;
    std::unique_ptr<VehicleRenderer> m_vehicleRenderer;
    std::unique_ptr<InventoryRenderer> m_inventoryRenderer;
    BlockRegistry m_registry;
    Inventory m_inventory;
    BuildDragDrop m_dragDrop;
    
    bool m_buildMode;
    float m_time;
    bool m_initialized;
};

} // namespace TerraVehicle

//namespace Vehicle {
//
//
//enum class BlockType : uint32_t {
//    Commander = 0,
//    Wheel,
//    Thruster,
//    Weapon,
//    Armor,
//    Generator,
//    COUNT
//};
//
//constexpr uint32_t MAX_BLOCKS_PER_VEHICLE = 128;
//constexpr uint32_t INVENTORY_SLOTS = 50;
//constexpr float BLOCK_SIZE = 1.0f;
//
//struct BlockVertex
//{
//    simd_float3 position;
//    simd_float3 normal;
//    simd_float2 uv;
//    simd_float4 color;
//};
//
//struct BlockInstance {
//    simd_float4x4 modelMatrix;
//    simd_float4 tintColor;
//    uint32_t blockType;
//    uint32_t flags;
//    float health;
//    float padding;
//};
//
//struct VehicleUniforms {
//    simd_float4x4 viewProjectionMatrix;
//    simd_float3 cameraPosition;
//    float time;
//    simd_float3 sunDirection;
//    float padding;
//};
//
//// ============================================================================
//// BLOCK STATS
//// ============================================================================
//struct BlockStats {
//    float mass;
//    float maxHealth;
//    float energyUse;
//    float energyGen;
//    simd_float3 size;
//    
//    BlockStats();
//    BlockStats(float m, float hp, float eUse, float eGen, simd_float3 sz);
//};
//
//// ============================================================================
//// ATTACHMENT FACE
//// ============================================================================
//enum AttachFace : uint8_t {
//    FacePosX = 0, FaceNegX,
//    FacePosY, FaceNegY,
//    FacePosZ, FaceNegZ,
//    FaceCount
//};
//
//struct AttachPoint {
//    simd_float3 offset;
//    simd_float3 normal;
//    AttachFace face;
//    bool used;
//    uint32_t linkedBlockID;
//    
//    AttachPoint();
//    AttachPoint(simd_float3 off, simd_float3 norm, AttachFace f);
//};
//
//// ============================================================================
//// BLOCK BASE CLASS
//// ============================================================================
//class Block {
//public:
//    uint32_t id;
//    BlockType type;
//    BlockStats stats;
//    simd_float3 localPos;
//    simd_quatf localRot;
//    float currentHealth;
//    bool destroyed;
//    AttachPoint attachPoints[FaceCount];
//    
//    Block();
//    Block(uint32_t blockID, BlockType blockType);
//    virtual ~Block() = default;
//    
//    void initAttachPoints();
//    simd_float4x4 buildModelMatrix(simd_float3 vehiclePos, simd_quatf vehicleRot) const;
//    
//    virtual void update(float dt) {}
//    virtual void onAttached() {}
//    virtual void onDetached() {}
//};
//
//// ============================================================================
//// COMMANDER BLOCK (coeur du véhicule)
//// ============================================================================
//class CommanderBlock : public Block {
//public:
//    float energyCapacity;
//    float currentEnergy;
//    float camDistance;
//    float camPitch;
//    float camYaw;
//    
//    CommanderBlock();
//    CommanderBlock(uint32_t blockID);
//    
//    void update(float dt) override;
//    simd_float3 getCameraPosition(simd_float3 vehiclePos, simd_quatf vehicleRot) const;
//    simd_float3 getCameraTarget(simd_float3 vehiclePos) const;
//};
//
//// ============================================================================
//// WHEEL BLOCK
//// ============================================================================
//class WheelBlock : public Block {
//public:
//    float radius;
//    float torque;
//    float spinAngle;
//    float steerAngle;
//    bool powered;
//    bool steering;
//    
//    WheelBlock();
//    WheelBlock(uint32_t blockID);
//    
//    void update(float dt) override;
//    simd_float3 getDriveForce(float throttle, simd_quatf vehicleRot) const;
//};
//
//// ============================================================================
//// VEHICLE CLASS
//// ============================================================================
//class VehicleEntity {
//public:
//    uint32_t vehicleID;
//    std::string name;
//    
//    std::unique_ptr<CommanderBlock> commander;
//    std::unordered_map<uint32_t, std::unique_ptr<Block>> blocks;
//    uint32_t nextBlockID;
//    
//    simd_float3 position;
//    simd_quatf rotation;
//    simd_float3 velocity;
//    simd_float3 angularVel;
//    
//    float totalMass;
//    simd_float3 centerOfMass;
//    
//    float throttleInput;
//    float steerInput;
//    float brakeInput;
//    bool onGround;
//    
//    VehicleEntity();
//    VehicleEntity(uint32_t id);
//    
//    void init();
//    void updatePhysics(float dt);
//    void recalculateMass();
//    
//    bool addBlock(std::unique_ptr<Block> block, uint32_t parentID, AttachFace parentFace, AttachFace blockFace);
//    bool removeBlock(uint32_t blockID);
//    
//    simd_float3 getCameraPos() const;
//    simd_float3 getCameraTarget() const;
//    void orbitCamera(float dYaw, float dPitch);
//    void zoomCamera(float delta);
//    
//    float getLowestY() const;
//};
//
//// ============================================================================
//// INVENTORY ITEM
//// ============================================================================
//struct InvItem {
//    BlockType type;
//    uint32_t count;
//    std::string name;
//    std::function<std::unique_ptr<Block>(uint32_t)> factory;
//    
//    InvItem();
//    InvItem(BlockType t, uint32_t c, const std::string& n, std::function<std::unique_ptr<Block>(uint32_t)> f);
//};
//
//// ============================================================================
//// INVENTORY
//// ============================================================================
//class Inventory {
//public:
//    std::vector<InvItem> slots;
//    int32_t selected;
//    bool visible;
//    
//    Inventory();
//    void initDefaults();
//    InvItem* getSelected();
//    bool consume(int32_t slot);
//    int32_t hitTest(simd_float2 normPos, simd_float2 screenSize) const;
//};
//
//// ============================================================================
//// DRAG DROP STATE
//// ============================================================================
//class DragDrop {
//public:
//    enum class State { Idle, Dragging, Placing };
//    
//    State state;
//    int32_t sourceSlot;
//    InvItem* draggedItem;
//    simd_float3 ghostPos;
//    simd_quatf ghostRot;
//    uint32_t targetBlockID;
//    AttachFace targetFace;
//    AttachFace sourceFace;
//    bool canPlace;
//    
//    DragDrop();
//    void beginDrag(Inventory& inv, int32_t slot);
//    bool endDrag(VehicleEntity& vehicle, Inventory& inv);
//    void cancel();
//    void rotatePart();
//    void updatePlacement(VehicleEntity& vehicle, simd_float3 rayOrigin, simd_float3 rayDir);
//};
//
//// ============================================================================
//// VEHICLE RENDERER
//// ============================================================================
//class VehicleRenderer {
//public:
//    MTL::Device* device;
//    MTL::RenderPipelineState* solidPipeline;
//    MTL::RenderPipelineState* ghostPipeline;
//    MTL::DepthStencilState* depthState;
//    MTL::Buffer* vertexBuffer;
//    MTL::Buffer* indexBuffer;
//    MTL::Buffer* instanceBuffer;
//    MTL::Buffer* uniformBuffer;
//    uint32_t indexCount;
//    
//    VehicleRenderer();
//    ~VehicleRenderer();
//    
//    void init(MTL::Device* dev, MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library);
//    void buildCommanderMesh();
//    void render(MTL::RenderCommandEncoder* enc, VehicleEntity& vehicle, DragDrop& drag,
//                simd_float4x4 vpMatrix, simd_float3 camPos, float time);
//    void cleanup();
//    
//private:
//    void createPipelines(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library);
//    void addQuad(std::vector<BlockVertex>& verts, std::vector<uint32_t>& idxs,
//                 simd_float3 p0, simd_float3 p1, simd_float3 p2, simd_float3 p3,
//                 simd_float3 normal, simd_float4 color);
//};
//
//// ============================================================================
//// VEHICLE MANAGER (point d'entrée principal)
//// ============================================================================
//class VehicleManager {
//public:
//    std::unique_ptr<VehicleEntity> vehicle;
//    Inventory inventory;
//    DragDrop dragDrop;
//    VehicleRenderer renderer;
//    bool buildMode;
//    float time;
//    
//    VehicleManager();
//    ~VehicleManager();
//    
//    void init(MTL::Device* dev, MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library);
//    void update(float dt);
//    void render(MTL::RenderCommandEncoder* enc, simd_float4x4 vpMatrix, simd_float3 camPos);
//    
//    void setThrottle(float val);
//    void setSteering(float val);
//    void setBrake(float val);
//    
//    simd_float3 getCameraPosition() const;
//    simd_float3 getCameraTarget() const;
//    void orbitCamera(float dYaw, float dPitch);
//    void zoomCamera(float delta);
//    
//    void toggleBuildMode();
//    void handleClick(simd_float2 normPos, simd_float2 screenSize, bool rightClick);
//    void handleRelease();
//    void handleMouseMove(simd_float3 rayOrigin, simd_float3 rayDir);
//    void rotateGhost();
//    void selectSlot(int32_t slot);
//    
//    void cleanup();
//};
//
//} // namespace Vehicle

//namespace NASAAtTheHelm {
//
//struct BlockInstanceData
//{
//    simd_float4x4 modelMatrix;
//    simd_float4 color;
//    uint32_t blockType;
//    uint32_t flags;         // isGhost, isSelected, etc.
//    float health;           // Pour effets visuels de dommages
//    float padding;
//};
//
//struct VehicleRenderUniforms
//{
//    simd_float4x4 viewProjectionMatrix;
//    simd_float3 cameraPosition;
//    float time;
//    simd_float3 lightDirection;
//    float padding;
//};
//
//class VehicleManager
//{
//public:
//    std::unique_ptr<Vehicle> activeVehicle;
//    Inventory inventory;
//    DragDropSystem dragDrop;
//    
//    MTL::Device* device = nullptr;
//    MTL::RenderPipelineState* blockPipeline = nullptr;
//    MTL::RenderPipelineState* ghostPipeline = nullptr;
//    MTL::DepthStencilState* depthState = nullptr;
//    MTL::Buffer* uniformBuffer = nullptr;
//    MTL::Buffer* instanceBuffer = nullptr;
//
//    CommandBlockMeshGenerator commandMeshGen;
//    CommandBlockMeshGenerator::MetalBuffers commandMeshBuffers;
//    
//    bool buildModeActive = false;
//    float time = 0.0f;
//    
//    simd_float3 lastRayOrigin;
//    simd_float3 lastRayDirection;
//    
//    void initialize(MTL::Device* dev, MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* library)
//    {
//        device = dev;
//        
//        // Créer véhicule de base
//        activeVehicle = VehicleFactory::createBasicCar(1);
//        activeVehicle->position = {0, 2, 0};
//        
//        // Initialiser inventaire avec items de départ
//        inventory.initializeDefaultItems();
//        
//        // Créer pipelines
//        createPipelines(colorFormat, depthFormat, library);
//        
//        // Créer buffers
//        uniformBuffer = device->newBuffer(sizeof(VehicleRenderUniforms),
//                                          MTL::ResourceStorageModeShared);
//        instanceBuffer = device->newBuffer(sizeof(BlockInstanceData) * 256,
//                                           MTL::ResourceStorageModeShared);
//        
//        // Générer meshes
//        commandMeshBuffers = commandMeshGen.createMetalBuffers(device);
//    }
//    
//    void createPipelines(MTL::PixelFormat colorFormat,
//                         MTL::PixelFormat depthFormat,
//                         MTL::Library* library) {
//        
//        // Shader pour les blocs
//        const char* shaderSrc = R"(
//            #include <metal_stdlib>
//            using namespace metal;
//            
//            struct BlockVertex {
//                float3 position [[attribute(0)]];
//                float3 normal   [[attribute(1)]];
//                float2 uv       [[attribute(2)]];
//                float4 color    [[attribute(3)]];
//            };
//            
//            struct BlockInstance {
//                float4x4 modelMatrix;
//                float4 color;
//                uint blockType;
//                uint flags;
//                float health;
//                float padding;
//            };
//            
//            struct VehicleUniforms {
//                float4x4 viewProjectionMatrix;
//                float3 cameraPosition;
//                float time;
//                float3 lightDirection;
//                float padding;
//            };
//            
//            struct BlockVarying {
//                float4 position [[position]];
//                float3 worldPos;
//                float3 normal;
//                float2 uv;
//                float4 color;
//                float health;
//                uint flags;
//            };
//            
//            vertex BlockVarying blockVertexShader(
//                BlockVertex in [[stage_in]],
//                constant VehicleUniforms& uniforms [[buffer(1)]],
//                constant BlockInstance* instances [[buffer(2)]],
//                uint instanceID [[instance_id]]
//            ) {
//                BlockVarying out;
//                
//                BlockInstance inst = instances[instanceID];
//                
//                float4 worldPos = inst.modelMatrix * float4(in.position, 1.0);
//                out.position = uniforms.viewProjectionMatrix * worldPos;
//                out.worldPos = worldPos.xyz;
//                
//                // Transformer normale
//                float3x3 normalMatrix = float3x3(inst.modelMatrix[0].xyz,
//                                                  inst.modelMatrix[1].xyz,
//                                                  inst.modelMatrix[2].xyz);
//                out.normal = normalize(normalMatrix * in.normal);
//                
//                out.uv = in.uv;
//                out.color = in.color * inst.color;
//                out.health = inst.health;
//                out.flags = inst.flags;
//                
//                return out;
//            }
//            
//            fragment float4 blockFragmentShader(
//                BlockVarying in [[stage_in]],
//                constant VehicleUniforms& uniforms [[buffer(1)]]
//            ) {
//                // Direction lumière
//                float3 L = normalize(uniforms.lightDirection);
//                float3 N = normalize(in.normal);
//                float3 V = normalize(uniforms.cameraPosition - in.worldPos);
//                float3 H = normalize(L + V);
//                
//                // Diffuse
//                float diff = max(dot(N, L), 0.0) * 0.7 + 0.3;
//                
//                // Specular
//                float spec = pow(max(dot(N, H), 0.0), 32.0) * 0.5;
//                
//                // Couleur de base
//                float3 baseColor = in.color.rgb;
//                
//                // Effet de dégâts
//                if (in.health < 1.0) {
//                    float damage = 1.0 - in.health;
//                    baseColor = mix(baseColor, float3(0.2, 0.1, 0.1), damage * 0.5);
//                }
//                
//                // Assemblage final
//                float3 finalColor = baseColor * diff + float3(1.0) * spec;
//                
//                // Fresnel rim light
//                float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);
//                finalColor += float3(0.3, 0.5, 0.8) * fresnel * 0.3;
//                
//                return float4(finalColor, in.color.a);
//            }
//            
//            // Shader pour bloc fantôme (placement)
//            fragment float4 ghostFragmentShader(
//                BlockVarying in [[stage_in]],
//                constant VehicleUniforms& uniforms [[buffer(1)]]
//            ) {
//                // Effet pulsant
//                float pulse = sin(uniforms.time * 4.0) * 0.15 + 0.85;
//                
//                // Grille holographique
//                float2 grid = fract(in.uv * 8.0);
//                float gridLine = step(0.9, grid.x) + step(0.9, grid.y);
//                
//                float3 color = in.color.rgb * pulse;
//                color += gridLine * 0.2;
//                
//                // Alpha variable
//                float alpha = in.color.a * (0.4 + gridLine * 0.3);
//                
//                return float4(color, alpha);
//            }
//        )";
//        
//        NS::Error* error = nullptr;
//        MTL::Library* shaderLib = device->newLibrary(
//            NS::String::string(shaderSrc, NS::UTF8StringEncoding),
//            nullptr, &error
//        );
//        
//        if (!shaderLib) {
//            printf("Failed to compile vehicle shaders\n");
//            return;
//        }
//        
//        MTL::Function* vertFunc = shaderLib->newFunction(
//            NS::String::string("blockVertexShader", NS::UTF8StringEncoding));
//        MTL::Function* fragFunc = shaderLib->newFunction(
//            NS::String::string("blockFragmentShader", NS::UTF8StringEncoding));
//        MTL::Function* ghostFrag = shaderLib->newFunction(
//            NS::String::string("ghostFragmentShader", NS::UTF8StringEncoding));
//        
//        // Vertex descriptor
//        MTL::VertexDescriptor* vertDesc = MTL::VertexDescriptor::alloc()->init();
//        
//        // Position
//        vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//        vertDesc->attributes()->object(0)->setOffset(0);
//        vertDesc->attributes()->object(0)->setBufferIndex(0);
//        
//        // Normal
//        vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//        vertDesc->attributes()->object(1)->setOffset(sizeof(simd_float3));
//        vertDesc->attributes()->object(1)->setBufferIndex(0);
//        
//        // UV
//        vertDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
//        vertDesc->attributes()->object(2)->setOffset(sizeof(simd_float3) * 2);
//        vertDesc->attributes()->object(2)->setBufferIndex(0);
//        
//        // Color
//        vertDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat4);
//        vertDesc->attributes()->object(3)->setOffset(sizeof(simd_float3) * 2 + sizeof(simd_float2));
//        vertDesc->attributes()->object(3)->setBufferIndex(0);
//        
//        vertDesc->layouts()->object(0)->setStride(sizeof(BlockVertex));
//        
//        // Pipeline principal
//        MTL::RenderPipelineDescriptor* pipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//        pipeDesc->setVertexFunction(vertFunc);
//        pipeDesc->setFragmentFunction(fragFunc);
//        pipeDesc->setVertexDescriptor(vertDesc);
//        pipeDesc->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//        pipeDesc->setDepthAttachmentPixelFormat(depthFormat);
//        
//        blockPipeline = device->newRenderPipelineState(pipeDesc, &error);
//        
//        // Pipeline fantôme (transparent)
//        pipeDesc->setFragmentFunction(ghostFrag);
//        pipeDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
//        pipeDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        pipeDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        
//        ghostPipeline = device->newRenderPipelineState(pipeDesc, &error);
//        
//        // Depth state
//        MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
//        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
//        depthDesc->setDepthWriteEnabled(true);
//        depthState = device->newDepthStencilState(depthDesc);
//        
//        // Cleanup
//        vertFunc->release();
//        fragFunc->release();
//        ghostFrag->release();
//        pipeDesc->release();
//        vertDesc->release();
//        depthDesc->release();
//        shaderLib->release();
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // MISE À JOUR
//    // ═══════════════════════════════════════════════════════════════════════
//    void update(float deltaTime, const simd_float3& cameraPos) {
//        time += deltaTime;
//        
//        if (!activeVehicle) return;
//        
//        // Mettre à jour physique du véhicule
//        activeVehicle->update(deltaTime);
//        
//        // Mettre à jour drag & drop si actif
//        if (dragDrop.state != DragDropSystem::State::Idle) {
//            dragDrop.updateDrag(*activeVehicle, lastRayOrigin, lastRayDirection, inventory);
//        }
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // CONTRÔLES VÉHICULE
//    // ═══════════════════════════════════════════════════════════════════════
//    void setVehicleThrottle(float value) {
//        if (activeVehicle) activeVehicle->throttle = value;
//    }
//    
//    void setVehicleSteering(float value) {
//        if (activeVehicle) activeVehicle->steering = value;
//    }
//    
//    void setVehicleBrake(float value) {
//        if (activeVehicle) activeVehicle->brake = value;
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // CAMÉRA
//    // ═══════════════════════════════════════════════════════════════════════
//    simd_float3 getVehicleCameraPosition() const {
//        if (!activeVehicle) return {0, 5, -10};
//        return activeVehicle->getCameraPosition();
//    }
//    
//    simd_float3 getVehicleCameraTarget() const {
//        if (!activeVehicle) return {0, 0, 0};
//        return activeVehicle->getCameraTarget();
//    }
//    
//    void rotateCameraOrbit(float deltaYaw, float deltaPitch) {
//        if (activeVehicle) {
//            activeVehicle->rotateCameraOrbit(deltaYaw, deltaPitch);
//        }
//    }
//    
//    void zoomCamera(float delta) {
//        if (activeVehicle) {
//            activeVehicle->zoomCamera(delta);
//        }
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // CONSTRUCTION (BUILD MODE)
//    // ═══════════════════════════════════════════════════════════════════════
//    void setBuildMode(bool active) {
//        buildModeActive = active;
//        if (!active) {
//            dragDrop.cancelDrag();
//        }
//    }
//    
//    void updateBuildRay(simd_float3 rayOrigin, simd_float3 rayDirection) {
//        lastRayOrigin = rayOrigin;
//        lastRayDirection = rayDirection;
//    }
//    
//    void rotateGhostBlock() {
//        dragDrop.rotateGhost();
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // RENDU
//    // ═══════════════════════════════════════════════════════════════════════
//    void render(MTL::RenderCommandEncoder* encoder,
//                const simd_float4x4& viewProjectionMatrix,
//                const simd_float3& cameraPosition) {
//        
//        if (!activeVehicle || !blockPipeline) return;
//        
//        // Mettre à jour uniforms
//        VehicleRenderUniforms* uniforms = (VehicleRenderUniforms*)uniformBuffer->contents();
//        uniforms->viewProjectionMatrix = viewProjectionMatrix;
//        uniforms->cameraPosition = cameraPosition;
//        uniforms->time = time;
//        uniforms->lightDirection = simd_normalize(simd_make_float3(0.5f, 1.0f, 0.3f));
//        
//        // Préparer instances
//        std::vector<BlockInstanceData> instances;
//        
//        // Commandant
//        {
//            BlockInstanceData inst;
//            inst.modelMatrix = activeVehicle->commander->getModelMatrix(
//                activeVehicle->position, activeVehicle->rotation);
//            inst.color = {1.0f, 1.0f, 1.0f, 1.0f};
//            inst.blockType = (uint32_t)BlockType::Commander;
//            inst.flags = 0;
//            inst.health = activeVehicle->commander->currentHealth /
//                         activeVehicle->commander->stats.health;
//            instances.push_back(inst);
//        }
//        
//        // Autres blocs
//        for (const auto& [id, block] : activeVehicle->blocks) {
//            if (block->isDestroyed) continue;
//            
//            BlockInstanceData inst;
//            inst.modelMatrix = block->getModelMatrix(
//                activeVehicle->position, activeVehicle->rotation);
//            inst.color = {1.0f, 1.0f, 1.0f, 1.0f};
//            inst.blockType = (uint32_t)block->type;
//            inst.flags = 0;
//            inst.health = block->currentHealth / block->stats.health;
//            instances.push_back(inst);
//        }
//        
//        if (instances.empty()) return;
//        
//        // Uploader instances
//        memcpy(instanceBuffer->contents(), instances.data(),
//               instances.size() * sizeof(BlockInstanceData));
//        
//        // Dessiner blocs opaques
//        encoder->setRenderPipelineState(blockPipeline);
//        encoder->setDepthStencilState(depthState);
//        
//        encoder->setVertexBuffer(commandMeshBuffers.vertexBuffer, 0, 0);
//        encoder->setVertexBuffer(uniformBuffer, 0, 1);
//        encoder->setVertexBuffer(instanceBuffer, 0, 2);
//        encoder->setFragmentBuffer(uniformBuffer, 0, 1);
//        
//        encoder->drawIndexedPrimitives(
//            MTL::PrimitiveTypeTriangle,
//            commandMeshBuffers.indexCount,
//            MTL::IndexTypeUInt32,
//            commandMeshBuffers.indexBuffer,
//            0,
//            instances.size()
//        );
//        
//        // Dessiner bloc fantôme si en mode construction
//        if (buildModeActive && dragDrop.state == DragDropSystem::State::PlacingBlock) {
//            renderGhostBlock(encoder, viewProjectionMatrix);
//        }
//    }
//    
//    void renderGhostBlock(MTL::RenderCommandEncoder* encoder,
//                          const simd_float4x4& viewProjectionMatrix) {
//        
//        // Instance unique pour le fantôme
//        BlockInstanceData ghostInst;
//        
//        // Construire matrice du fantôme
//        simd_float4x4 T = matrix_identity_float4x4;
//        simd_float3 ghostWorldPos = activeVehicle->position +
//            simd_act(activeVehicle->rotation, dragDrop.ghostPosition);
//        T.columns[3] = simd_make_float4(ghostWorldPos.x, ghostWorldPos.y, ghostWorldPos.z, 1.0f);
//        
//        simd_float4x4 R = simd_matrix4x4(simd_mul(activeVehicle->rotation, dragDrop.ghostRotation));
//        
//        ghostInst.modelMatrix = simd_mul(T, R);
//        ghostInst.color = dragDrop.canPlace ?
//            simd_make_float4(0.3f, 0.8f, 0.3f, 0.6f) :
//            simd_make_float4(0.8f, 0.3f, 0.3f, 0.6f);
//        ghostInst.blockType = dragDrop.draggedItem ? (uint32_t)dragDrop.draggedItem->type : 0;
//        ghostInst.flags = 1;  // isGhost
//        ghostInst.health = 1.0f;
//        
//        // Upload
//        memcpy(instanceBuffer->contents(), &ghostInst, sizeof(BlockInstanceData));
//        
//        // Dessiner avec pipeline transparent
//        encoder->setRenderPipelineState(ghostPipeline);
//        
//        encoder->drawIndexedPrimitives(
//            MTL::PrimitiveTypeTriangle,
//            commandMeshBuffers.indexCount,
//            MTL::IndexTypeUInt32,
//            commandMeshBuffers.indexBuffer,
//            0,
//            1
//        );
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // INPUT HANDLERS
//    // ═══════════════════════════════════════════════════════════════════════
//    void handleMouseDown(simd_float2 normalizedPos, bool rightClick) {
//        if (buildModeActive && !rightClick) {
//            int32_t slot = inventory.hitTest(normalizedPos);
//            if (slot >= 0) {
//                dragDrop.beginDragFromInventory(inventory, slot);
//            }
//        }
//    }
//    
//    void handleMouseUp(simd_float2 normalizedPos, bool rightClick) {
//        if (buildModeActive && dragDrop.state != DragDropSystem::State::Idle) {
//            bool placed = dragDrop.endDrag(*activeVehicle, inventory);
//            if (placed) {
//                // Feedback visuel/audio
//            }
//        }
//    }
//    
//    void handleMouseMove(simd_float2 normalizedPos) {
//        if (buildModeActive) {
//            // Mettre à jour le survol
//        }
//    }
//    
//    void handleKeyPress(int keyCode) {
//        switch (keyCode) {
//            case 0x0F: // R - Rotation du bloc fantôme
//                if (buildModeActive) {
//                    rotateGhostBlock();
//                }
//                break;
//                
//            case 0x0E: // E - Toggle inventaire
//                inventory.isVisible = !inventory.isVisible;
//                break;
//                
//            case 0x0B: // B - Toggle mode construction
//                setBuildMode(!buildModeActive);
//                break;
//        }
//        
//        // Hotbar 1-9, 0
//        if (keyCode >= 0x12 && keyCode <= 0x1D) {
//            int slot = keyCode - 0x12;
//            if (slot < 10) inventory.selectedSlot = slot;
//        }
//    }
//    
//    void handleScroll(float deltaY) {
//        if (buildModeActive) {
//            // Rien en mode construction
//        } else {
//            zoomCamera(deltaY * 0.5f);
//        }
//    }
//    
//    // ═══════════════════════════════════════════════════════════════════════
//    // CLEANUP
//    // ═══════════════════════════════════════════════════════════════════════
//    void cleanup() {
//        if (blockPipeline) blockPipeline->release();
//        if (ghostPipeline) ghostPipeline->release();
//        if (depthState) depthState->release();
//        if (uniformBuffer) uniformBuffer->release();
//        if (instanceBuffer) instanceBuffer->release();
//        if (commandMeshBuffers.vertexBuffer) commandMeshBuffers.vertexBuffer->release();
//        if (commandMeshBuffers.indexBuffer) commandMeshBuffers.indexBuffer->release();
//    }
//};
//
//} // namespace Metal4

#endif /* RMDLManager_hpp */
