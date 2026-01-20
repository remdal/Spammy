//
//  RMDLManager.cpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#include "RMDLManager.hpp"

#include <cmath>
#include <cfloat>
#include <algorithm>

namespace TerraVehicle {

// ============================================================================
// BLOCK DEFINITION
// ============================================================================
BlockDefinition::BlockDefinition()
    : typeID(0), name("Unknown"), description(""), category(BlockCategory::Structure),
      mass(10.f), maxHealth(100.f), energyUse(0.f), energyGen(0.f),
      size{1,1,1}, baseColor{0.5f,0.5f,0.5f,1.f}, meshID(0)
{}

BlockDefinition::BlockDefinition(uint32_t id, const std::string& n, BlockCategory cat,
                                 float m, float hp, simd::float4 col)
    : typeID(id), name(n), description(""), category(cat),
      mass(m), maxHealth(hp), energyUse(0.f), energyGen(0.f),
      size{1,1,1}, baseColor(col), meshID(0)
{}

// ============================================================================
// ATTACH POINT
// ============================================================================
AttachPoint::AttachPoint()
    : localOffset{0,0,0}, normal{0,1,0}, face(AttachFace::PosY),
      occupied(false), connectedID(0)
{}

AttachPoint::AttachPoint(simd::float3 off, simd::float3 n, AttachFace f)
    : localOffset(off), normal(n), face(f), occupied(false), connectedID(0)
{}

// ============================================================================
// BLOCK INSTANCE
// ============================================================================
BlockInstance::BlockInstance()
    : instanceID(0), definitionID(0), localPosition{0,0,0},
      localRotation(matrix_identity_float4x4), currentHealth(100.f), destroyed(false)
{
    initAttachPoints();
}

BlockInstance::BlockInstance(uint32_t instID, uint32_t defID)
    : instanceID(instID), definitionID(defID), localPosition{0,0,0},
      localRotation(matrix_identity_float4x4), currentHealth(100.f), destroyed(false)
{
    initAttachPoints();
}

void BlockInstance::initAttachPoints()
{
    float h = BLOCK_UNIT * 0.5f;
    attachPoints[0] = AttachPoint(simd::float3{+h, 0, 0}, simd::float3{+1, 0, 0}, AttachFace::PosX);
    attachPoints[1] = AttachPoint(simd::float3{-h, 0, 0}, simd::float3{-1, 0, 0}, AttachFace::NegX);
    attachPoints[2] = AttachPoint(simd::float3{0, +h, 0}, simd::float3{0, +1, 0}, AttachFace::PosY);
    attachPoints[3] = AttachPoint(simd::float3{0, -h, 0}, simd::float3{0, -1, 0}, AttachFace::NegY);
    attachPoints[4] = AttachPoint(simd::float3{0, 0, +h}, simd::float3{0, 0, +1}, AttachFace::PosZ);
    attachPoints[5] = AttachPoint(simd::float3{0, 0, -h}, simd::float3{0, 0, -1}, AttachFace::NegZ);
}

simd::float4x4 BlockInstance::computeWorldMatrix(simd::float3 vehiclePos, simd::float4x4 vehicleRot) const
{
    simd::float4 localPos4 = {localPosition.x, localPosition.y, localPosition.z, 1.f};
    simd::float4 worldPos4 = simd_mul(vehicleRot, localPos4);
    simd::float3 worldPos = {worldPos4.x + vehiclePos.x, worldPos4.y + vehiclePos.y, worldPos4.z + vehiclePos.z};
    
    simd::float4x4 T = math::makeTranslate(worldPos);
    simd::float4x4 R = simd_mul(vehicleRot, localRotation);
    return simd_mul(T, R);
}

// ============================================================================
// COMMANDER BLOCK
// ============================================================================
CommanderBlock::CommanderBlock() : BlockInstance(0, 0)
{
    definitionID = 0; // Commander type
    energyCapacity = 100.f;
    currentEnergy = 100.f;
    camOrbitYaw = 0.f;
    camOrbitPitch = 0.3f;
    camOrbitDist = 10.f;
    currentHealth = 200.f;
}

CommanderBlock::CommanderBlock(uint32_t instID) : BlockInstance(instID, 0)
{
    definitionID = 0;
    energyCapacity = 100.f;
    currentEnergy = 100.f;
    camOrbitYaw = 0.f;
    camOrbitPitch = 0.3f;
    camOrbitDist = 10.f;
    currentHealth = 200.f;
}

simd::float3 CommanderBlock::computeCameraPosition(simd::float3 vehiclePos, simd::float4x4 vehicleRot) const
{
    float x = camOrbitDist * cosf(camOrbitPitch) * sinf(camOrbitYaw);
    float y = camOrbitDist * sinf(camOrbitPitch) + 2.5f;
    float z = camOrbitDist * cosf(camOrbitPitch) * cosf(camOrbitYaw);
    
    simd::float4 offset = {x, y, z, 0.f};
    simd::float4 rotatedOffset = simd_mul(vehicleRot, offset);
    
    return {vehiclePos.x + rotatedOffset.x, vehiclePos.y + rotatedOffset.y, vehiclePos.z + rotatedOffset.z};
}

simd::float3 CommanderBlock::computeCameraTarget(simd::float3 vehiclePos) const
{
    return {vehiclePos.x, vehiclePos.y + 1.5f, vehiclePos.z};
}

// ============================================================================
// WHEEL BLOCK
// ============================================================================
WheelBlock::WheelBlock() : BlockInstance(0, 1)
{
    definitionID = 1; // Wheel type
    radius = 0.45f;
    torque = 180.f;
    spinAngle = 0.f;
    steerAngle = 0.f;
    isPowered = true;
    canSteer = false;
    currentHealth = 60.f;
}

WheelBlock::WheelBlock(uint32_t instID) : BlockInstance(instID, 1)
{
    definitionID = 1;
    radius = 0.45f;
    torque = 180.f;
    spinAngle = 0.f;
    steerAngle = 0.f;
    isPowered = true;
    canSteer = false;
    currentHealth = 60.f;
}

simd::float3 WheelBlock::computeDriveForce(float throttle, simd::float4x4 vehicleRot) const
{
    if (!isPowered || destroyed) return {0, 0, 0};
    
    simd::float4 fwd = {0, 0, 1, 0};
    simd::float4 worldFwd = simd_mul(vehicleRot, fwd);
    
    return {worldFwd.x * throttle * torque,
            worldFwd.y * throttle * torque,
            worldFwd.z * throttle * torque};
}

void WheelBlock::updateSpin(float vehicleSpeed, float dt)
{
    if (radius > 0.01f) {
        spinAngle += (vehicleSpeed / radius) * dt;
        if (spinAngle > M_PI * 2.f) spinAngle -= M_PI * 2.f;
    }
}

// ============================================================================
// VEHICLE
// ============================================================================
Vehicle::Vehicle() : vehicleID(0) { initialize(); }
Vehicle::Vehicle(uint32_t id) : vehicleID(id) { initialize(); }

void Vehicle::initialize()
{
    name = "Vehicle";
    nextBlockID = 1;
    position = {0, 3, 0};
    rotationMatrix = matrix_identity_float4x4;
    velocity = {0, 0, 0};
    angularVelocity = {0, 0, 0};
    totalMass = 0.f;
    centerOfMass = {0, 0, 0};
    inputThrottle = 0.f;
    inputSteering = 0.f;
    inputBrake = 0.f;
    isGrounded = false;
    
    commander = std::make_unique<CommanderBlock>(0);
    recalculateMass();
}

void Vehicle::recalculateMass()
{
    totalMass = 50.f; // Commander base mass
    simd::float3 weighted = {0, 0, 0};
    
    if (commander) {
        weighted = commander->localPosition * 50.f;
    }
    
    for (const auto& [id, block] : blocks) {
        if (block && !block->destroyed) {
            float blockMass = 15.f; // Default block mass
            totalMass += blockMass;
            weighted.x += block->localPosition.x * blockMass;
            weighted.y += block->localPosition.y * blockMass;
            weighted.z += block->localPosition.z * blockMass;
        }
    }
    
    if (totalMass > 0.f) {
        centerOfMass = {weighted.x / totalMass, weighted.y / totalMass, weighted.z / totalMass};
    }
}

float Vehicle::computeLowestPoint() const
{
    float lowest = commander ? (commander->localPosition.y - 0.5f) : 0.f;
    
    for (const auto& [id, block] : blocks) {
        if (block) {
            float y = block->localPosition.y - 0.5f;
            if (y < lowest) lowest = y;
        }
    }
    return lowest;
}

void Vehicle::updatePhysics(float dt)
{
    if (!commander) return;
    
    // Update energy
    commander->currentEnergy -= 1.f * dt;
    if (commander->currentEnergy < 0.f) commander->currentEnergy = 0.f;
    
    // Forces
    simd::float3 force = {0, -9.81f * totalMass, 0}; // Gravity
    
    // Drive force from wheels
    for (const auto& [id, block] : blocks) {
        if (block && block->definitionID == 1) { // Wheel
            WheelBlock* wheel = static_cast<WheelBlock*>(block.get());
            simd::float3 driveF = wheel->computeDriveForce(inputThrottle, rotationMatrix);
            force.x += driveF.x;
            force.y += driveF.y;
            force.z += driveF.z;
            
            // Update wheel spin
            float speed = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
            wheel->updateSpin(speed, dt);
        }
    }
    
    // Air drag
    float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
    if (speed > 0.01f) {
        float drag = speed * speed * 0.5f;
        force.x -= (velocity.x / speed) * drag;
        force.y -= (velocity.y / speed) * drag;
        force.z -= (velocity.z / speed) * drag;
    }
    
    // Ground check
    float lowest = computeLowestPoint();
    isGrounded = (position.y + lowest) <= 0.1f;
    
    // Brake friction
    if (isGrounded && inputBrake > 0.f) {
        force.x -= velocity.x * 8.f * inputBrake;
        force.z -= velocity.z * 8.f * inputBrake;
    }
    
    // Integration
    if (totalMass > 0.f) {
        velocity.x += (force.x / totalMass) * dt;
        velocity.y += (force.y / totalMass) * dt;
        velocity.z += (force.z / totalMass) * dt;
    }
    
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;
    
    // Ground collision
    if (position.y + lowest < 0.f) {
        position.y = -lowest;
        if (velocity.y < 0.f) velocity.y = 0.f;
    }
    
    // Steering rotation
    if (isGrounded && fabsf(inputSteering) > 0.01f && speed > 0.5f) {
        float turnRate = inputSteering * 2.5f * dt;
        simd::float4x4 turnMatrix = math::makeYRotate(turnRate);
        rotationMatrix = simd_mul(turnMatrix, rotationMatrix);
        
        // Rotate velocity
        simd::float4 vel4 = {velocity.x, velocity.y, velocity.z, 0.f};
        simd::float4 rotVel = simd_mul(turnMatrix, vel4);
        velocity = {rotVel.x, rotVel.y, rotVel.z};
    }
}

bool Vehicle::attachBlock(std::unique_ptr<BlockInstance> block, uint32_t parentID,
                          AttachFace parentFace, AttachFace childFace)
{
    if (!block) return false;
    
    BlockInstance* parent = nullptr;
    if (parentID == 0) {
        parent = commander.get();
    } else {
        auto it = blocks.find(parentID);
        if (it != blocks.end()) parent = it->second.get();
    }
    
    if (!parent) return false;
    
    uint8_t pf = static_cast<uint8_t>(parentFace);
    uint8_t cf = static_cast<uint8_t>(childFace);
    
    if (pf >= 6 || cf >= 6) return false;
    if (parent->attachPoints[pf].occupied) return false;
    if (block->attachPoints[cf].occupied) return false;
    
    // Compute position
    simd::float3 parentOff = parent->attachPoints[pf].localOffset;
    simd::float3 childOff = block->attachPoints[cf].localOffset;
    
    block->localPosition = {
        parent->localPosition.x + parentOff.x - childOff.x,
        parent->localPosition.y + parentOff.y - childOff.y,
        parent->localPosition.z + parentOff.z - childOff.z
    };
    
    block->instanceID = nextBlockID++;
    
    // Link
    parent->attachPoints[pf].occupied = true;
    parent->attachPoints[pf].connectedID = block->instanceID;
    block->attachPoints[cf].occupied = true;
    block->attachPoints[cf].connectedID = parentID;
    
    blocks[block->instanceID] = std::move(block);
    recalculateMass();
    
    return true;
}

bool Vehicle::detachBlock(uint32_t blockID)
{
    if (blockID == 0) return false;
    
    auto it = blocks.find(blockID);
    if (it == blocks.end()) return false;
    
    BlockInstance* block = it->second.get();
    
    // Unlink from connected blocks
    for (int i = 0; i < 6; i++) {
        if (block->attachPoints[i].occupied) {
            uint32_t linkedID = block->attachPoints[i].connectedID;
            BlockInstance* linked = nullptr;
            
            if (linkedID == 0) {
                linked = commander.get();
            } else {
                auto lit = blocks.find(linkedID);
                if (lit != blocks.end()) linked = lit->second.get();
            }
            
            if (linked) {
                for (int j = 0; j < 6; j++) {
                    if (linked->attachPoints[j].connectedID == blockID) {
                        linked->attachPoints[j].occupied = false;
                        linked->attachPoints[j].connectedID = 0;
                    }
                }
            }
        }
    }
    
    blocks.erase(it);
    recalculateMass();
    return true;
}

simd::float3 Vehicle::getCameraPosition() const
{
    if (!commander) return {position.x, position.y + 8.f, position.z - 12.f};
    return commander->computeCameraPosition(position, rotationMatrix);
}

simd::float3 Vehicle::getCameraTarget() const
{
    if (!commander) return position;
    return commander->computeCameraTarget(position);
}

void Vehicle::orbitCamera(float dYaw, float dPitch)
{
    if (!commander) return;
    commander->camOrbitYaw += dYaw;
    commander->camOrbitPitch += dPitch;
    
    if (commander->camOrbitPitch < -0.2f) commander->camOrbitPitch = -0.2f;
    if (commander->camOrbitPitch > 1.4f) commander->camOrbitPitch = 1.4f;
}

void Vehicle::zoomCamera(float delta)
{
    if (!commander) return;
    commander->camOrbitDist += delta;
    
    if (commander->camOrbitDist < 4.f) commander->camOrbitDist = 4.f;
    if (commander->camOrbitDist > 30.f) commander->camOrbitDist = 30.f;
}

// ============================================================================
// INVENTORY SLOT
// ============================================================================
InventorySlot::InventorySlot() : definitionID(0), quantity(0) {}
bool InventorySlot::isEmpty() const { return quantity == 0; }

// ============================================================================
// INVENTORY
// ============================================================================
Inventory::Inventory()
    : selectedSlot(0), hoveredSlot(-1), isOpen(false),
      panelPosition{0.5f, 0.5f}, isDraggingPanel(false), dragOffset{0,0}
{
    for (uint32_t i = 0; i < INVENTORY_SIZE; i++) {
        slots[i] = InventorySlot();
    }
}

void Inventory::setDefaultItems()
{
    // Wheels
    slots[0].definitionID = 1;
    slots[0].quantity = 8;
    
    // Armor
    slots[1].definitionID = 2;
    slots[1].quantity = 20;
    
    // Thruster
    slots[2].definitionID = 3;
    slots[2].quantity = 4;
    
    // Generator
    slots[3].definitionID = 4;
    slots[3].quantity = 2;
}

bool Inventory::consumeSlot(int32_t slot)
{
    if (slot < 0 || slot >= (int32_t)INVENTORY_SIZE) return false;
    if (slots[slot].quantity == 0) return false;
    slots[slot].quantity--;
    return true;
}

bool Inventory::addItem(uint32_t defID, uint32_t count)
{
    // Try existing slot first
    for (uint32_t i = 0; i < INVENTORY_SIZE; i++) {
        if (slots[i].definitionID == defID && slots[i].quantity > 0) {
            slots[i].quantity += count;
            return true;
        }
    }
    // Find empty slot
    for (uint32_t i = 0; i < INVENTORY_SIZE; i++) {
        if (slots[i].quantity == 0) {
            slots[i].definitionID = defID;
            slots[i].quantity = count;
            return true;
        }
    }
    return false;
}

simd::float2 Inventory::getSlotScreenPos(int32_t slot, simd::float2 screenSize) const
{
    if (slot < 0 || slot >= (int32_t)INVENTORY_SIZE) return {0, 0};
    
    int32_t col = slot % INVENTORY_COLS;
    int32_t row = slot / INVENTORY_COLS;
    
    float slotW = 50.f / screenSize.x;
    float slotH = 50.f / screenSize.y;
    float padding = 6.f / screenSize.x;
    
    float panelW = INVENTORY_COLS * (slotW + padding);
    float startX = panelPosition.x - panelW * 0.5f;
    float startY = panelPosition.y + 0.05f; // Below title bar
    
    return {startX + col * (slotW + padding), startY + row * (slotH + padding)};
}

int32_t Inventory::hitTestSlot(simd::float2 mouseNorm, simd::float2 screenSize) const
{
    if (!isOpen) return -1;
    
    float slotW = 50.f / screenSize.x;
    float slotH = 50.f / screenSize.y;
    
    for (int32_t i = 0; i < (int32_t)INVENTORY_SIZE; i++) {
        simd::float2 pos = getSlotScreenPos(i, screenSize);
        
        if (mouseNorm.x >= pos.x && mouseNorm.x <= pos.x + slotW &&
            mouseNorm.y >= pos.y && mouseNorm.y <= pos.y + slotH) {
            return i;
        }
    }
    return -1;
}

bool Inventory::hitTestPanel(simd::float2 mouseNorm, simd::float2 screenSize) const
{
    if (!isOpen) return false;
    
    float slotW = 50.f / screenSize.x;
    float padding = 6.f / screenSize.x;
    float panelW = INVENTORY_COLS * (slotW + padding) + padding * 2.f;
    float panelH = INVENTORY_ROWS * (slotW + padding) + 0.08f; // +title bar
    
    float left = panelPosition.x - panelW * 0.5f;
    float top = panelPosition.y - 0.03f;
    
    return mouseNorm.x >= left && mouseNorm.x <= left + panelW &&
           mouseNorm.y >= top && mouseNorm.y <= top + panelH;
}

// ============================================================================
// BUILD DRAG DROP
// ============================================================================
BuildDragDrop::BuildDragDrop()
    : mode(Mode::Idle), sourceSlot(-1), draggedDefID(0),
      ghostLocalPos{0,0,0}, ghostRotation(matrix_identity_float4x4),
      targetBlockID(0), targetFace(AttachFace::PosY), ghostFace(AttachFace::NegY),
      validPlacement(false)
{}

void BuildDragDrop::startDrag(Inventory& inv, int32_t slot)
{
    if (slot < 0 || slot >= (int32_t)INVENTORY_SIZE) return;
    if (inv.slots[slot].quantity == 0) return;
    
    mode = Mode::FromInventory;
    sourceSlot = slot;
    draggedDefID = inv.slots[slot].definitionID;
    inv.selectedSlot = slot;
}

bool BuildDragDrop::finishDrag(Vehicle& vehicle, Inventory& inv, const std::vector<BlockDefinition>& defs)
{
    if (mode == Mode::Idle) return false;
    
    bool success = false;
    
    if (mode == Mode::Placing && validPlacement && draggedDefID > 0) {
        // Create block based on definition
        std::unique_ptr<BlockInstance> newBlock;
        
        if (draggedDefID == 1) {
            newBlock = std::make_unique<WheelBlock>(vehicle.nextBlockID);
        } else {
            newBlock = std::make_unique<BlockInstance>(vehicle.nextBlockID, draggedDefID);
        }
        
        newBlock->localRotation = ghostRotation;
        
        if (vehicle.attachBlock(std::move(newBlock), targetBlockID, targetFace, ghostFace)) {
            inv.consumeSlot(sourceSlot);
            success = true;
        }
    }
    
    cancelDrag();
    return success;
}

void BuildDragDrop::cancelDrag()
{
    mode = Mode::Idle;
    sourceSlot = -1;
    draggedDefID = 0;
    validPlacement = false;
}

void BuildDragDrop::cycleGhostRotation()
{
    ghostFace = static_cast<AttachFace>((static_cast<uint8_t>(ghostFace) + 1) % 6);
}

void BuildDragDrop::updateGhostPlacement(Vehicle& vehicle, simd::float3 rayOrigin, simd::float3 rayDir)
{
    if (mode == Mode::Idle) return;
    mode = Mode::Placing;
    validPlacement = false;
    
    float bestT = FLT_MAX;
    
    auto testBlock = [&](BlockInstance* blk) {
        simd::float4x4 worldMat = blk->computeWorldMatrix(vehicle.position, vehicle.rotationMatrix);
        simd::float3 wpos = {worldMat.columns[3].x, worldMat.columns[3].y, worldMat.columns[3].z};
        
        simd::float3 toBlock = {wpos.x - rayOrigin.x, wpos.y - rayOrigin.y, wpos.z - rayOrigin.z};
        float t = toBlock.x * rayDir.x + toBlock.y * rayDir.y + toBlock.z * rayDir.z;
        if (t < 0) return;
        
        simd::float3 closest = {rayOrigin.x + rayDir.x * t, rayOrigin.y + rayDir.y * t, rayOrigin.z + rayDir.z * t};
        simd::float3 diff = {closest.x - wpos.x, closest.y - wpos.y, closest.z - wpos.z};
        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        if (dist < 1.5f && t < bestT) {
            // Find best face
            AttachFace bestFace = AttachFace::PosY;
            float maxDot = -FLT_MAX;
            
            for (int i = 0; i < 6; i++) {
                if (blk->attachPoints[i].occupied) continue;
                simd::float3 n = blk->attachPoints[i].normal;
                float d = diff.x * n.x + diff.y * n.y + diff.z * n.z;
                if (d > maxDot) {
                    maxDot = d;
                    bestFace = static_cast<AttachFace>(i);
                }
            }
            
            if (!blk->attachPoints[static_cast<uint8_t>(bestFace)].occupied) {
                bestT = t;
                targetBlockID = blk->instanceID;
                targetFace = bestFace;
                
                simd::float3 off = blk->attachPoints[static_cast<uint8_t>(bestFace)].localOffset;
                ghostLocalPos = {blk->localPosition.x + off.x * 2.f,
                                 blk->localPosition.y + off.y * 2.f,
                                 blk->localPosition.z + off.z * 2.f};
                validPlacement = true;
            }
        }
    };
    
    if (vehicle.commander) testBlock(vehicle.commander.get());
    for (auto& [id, blk] : vehicle.blocks) {
        if (blk) testBlock(blk.get());
    }
}

// ============================================================================
// BLOCK REGISTRY
// ============================================================================
BlockRegistry::BlockRegistry() { registerDefaults(); }

void BlockRegistry::registerDefaults()
{
    definitions.clear();
    
    // 0: Commander (special - not placeable)
    definitions.push_back(BlockDefinition(0, "Commander", BlockCategory::Core, 50.f, 200.f, simd::float4{0.2f, 0.4f, 0.8f, 1.f}));
    
    // 1: Wheel
    definitions.push_back(BlockDefinition(1, "Wheel", BlockCategory::Mobility, 15.f, 60.f, simd::float4{0.3f, 0.3f, 0.35f, 1.f}));
    
    // 2: Armor
    definitions.push_back(BlockDefinition(2, "Armor Block", BlockCategory::Structure, 20.f, 150.f, simd::float4{0.5f, 0.5f, 0.55f, 1.f}));
    
    // 3: Thruster
    definitions.push_back(BlockDefinition(3, "Thruster", BlockCategory::Mobility, 8.f, 40.f, simd::float4{0.8f, 0.4f, 0.2f, 1.f}));
    
    // 4: Generator
    definitions.push_back(BlockDefinition(4, "Generator", BlockCategory::Utility, 30.f, 80.f, simd::float4{0.9f, 0.8f, 0.2f, 1.f}));
}

const BlockDefinition* BlockRegistry::getDefinition(uint32_t typeID) const
{
    for (const auto& def : definitions) {
        if (def.typeID == typeID) return &def;
    }
    return nullptr;
}

std::unique_ptr<BlockInstance> BlockRegistry::createInstance(uint32_t defID, uint32_t instanceID) const
{
    if (defID == 1) {
        return std::make_unique<WheelBlock>(instanceID);
    }
    return std::make_unique<BlockInstance>(instanceID, defID);
}

// ============================================================================
// VEHICLE RENDERER
// ============================================================================
VehicleRenderer::VehicleRenderer(MTL::Device* device, MTL::PixelFormat colorFmt,
                                 MTL::PixelFormat depthFmt, MTL::Library* library)
    : m_device(device), m_solidPipeline(nullptr), m_ghostPipeline(nullptr),
      m_depthState(nullptr), m_vertexBuffer(nullptr), m_indexBuffer(nullptr),
      m_instanceBuffer(nullptr), m_uniformBuffer(nullptr), m_indexCount(0)
{
    buildPipeline(colorFmt, depthFmt, library);
    buildBlockMesh();
    
    m_instanceBuffer = device->newBuffer(sizeof(BlockGPUInstance) * MAX_VEHICLE_BLOCKS, MTL::ResourceStorageModeShared);
    m_uniformBuffer = device->newBuffer(sizeof(VehicleGPUUniforms), MTL::ResourceStorageModeShared);
}

VehicleRenderer::~VehicleRenderer()
{
    if (m_solidPipeline) m_solidPipeline->release();
    if (m_ghostPipeline) m_ghostPipeline->release();
    if (m_depthState) m_depthState->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    if (m_instanceBuffer) m_instanceBuffer->release();
    if (m_uniformBuffer) m_uniformBuffer->release();
}

void VehicleRenderer::buildPipeline(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library)
{
    NS::Error* error = nullptr;
    
    MTL::Function* vsFunc = library->newFunction(MTLSTR("terraBlockVS"));
    MTL::Function* fsFunc = library->newFunction(MTLSTR("terraBlockFS"));
    MTL::Function* ghostFS = library->newFunction(MTLSTR("terraGhostFS"));
    
    if (!vsFunc || !fsFunc) {
        printf("TerraVehicle: Shader functions not found!\n");
        return;
    }
    
    NS::SharedPtr<MTL::VertexDescriptor> vd = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
    vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vd->attributes()->object(0)->setOffset(0);
    vd->attributes()->object(0)->setBufferIndex(0);
    vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vd->attributes()->object(1)->setOffset(sizeof(simd::float3));
    vd->attributes()->object(1)->setBufferIndex(0);
    vd->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vd->attributes()->object(2)->setOffset(sizeof(simd::float3) * 2);
    vd->attributes()->object(2)->setBufferIndex(0);
    vd->attributes()->object(3)->setFormat(MTL::VertexFormatFloat4);
    vd->attributes()->object(3)->setOffset(sizeof(simd::float3) * 2 + sizeof(simd::float2));
    vd->attributes()->object(3)->setBufferIndex(0);
    vd->layouts()->object(0)->setStride(sizeof(BlockGPUVertex));
    
    NS::SharedPtr<MTL::RenderPipelineDescriptor> pd = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pd->setVertexFunction(vsFunc);
    pd->setFragmentFunction(fsFunc);
    pd->setVertexDescriptor(vd.get());
    pd->colorAttachments()->object(0)->setPixelFormat(colorFmt);
    pd->setDepthAttachmentPixelFormat(depthFmt);
    
    m_solidPipeline = m_device->newRenderPipelineState(pd.get(), &error);
    
    if (ghostFS) {
        pd->setFragmentFunction(ghostFS);
        pd->colorAttachments()->object(0)->setBlendingEnabled(true);
        pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        m_ghostPipeline = m_device->newRenderPipelineState(pd.get(), &error);
        ghostFS->release();
    }
    
    NS::SharedPtr<MTL::DepthStencilDescriptor> dsd = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    dsd->setDepthCompareFunction(MTL::CompareFunctionLess);
    dsd->setDepthWriteEnabled(true);
    m_depthState = m_device->newDepthStencilState(dsd.get());
    
    vsFunc->release();
    fsFunc->release();
}

void VehicleRenderer::buildBlockMesh()
{
    std::vector<BlockGPUVertex> verts;
    std::vector<uint32_t> idxs;
    
    auto addQuad = [&](simd::float3 p0, simd::float3 p1, simd::float3 p2, simd::float3 p3,
                       simd::float3 n, simd::float4 c) {
        uint32_t base = (uint32_t)verts.size();
        verts.push_back({p0, n, {0,0}, c});
        verts.push_back({p1, n, {1,0}, c});
        verts.push_back({p2, n, {1,1}, c});
        verts.push_back({p3, n, {0,1}, c});
        idxs.insert(idxs.end(), {base, base+1, base+2, base, base+2, base+3});
    };
    
    float h = 0.48f;
    simd::float4 col = {1.f, 1.f, 1.f, 1.f};
    
    addQuad(simd::float3{-h,+h,-h}, simd::float3{+h,+h,-h}, simd::float3{+h,+h,+h}, simd::float3{-h,+h,+h}, simd::float3{0,+1,0}, col); // +Y
    addQuad(simd::float3{-h,-h,+h}, simd::float3{+h,-h,+h}, simd::float3{+h,-h,-h}, simd::float3{-h,-h,-h}, simd::float3{0,-1,0}, col); // -Y
    addQuad(simd::float3{+h,-h,-h}, simd::float3{+h,-h,+h}, simd::float3{+h,+h,+h}, simd::float3{+h,+h,-h}, simd::float3{+1,0,0}, col); // +X
    addQuad(simd::float3{-h,-h,+h}, simd::float3{-h,-h,-h}, simd::float3{-h,+h,-h}, simd::float3{-h,+h,+h}, simd::float3{-1,0,0}, col); // -X
    addQuad(simd::float3{+h,-h,+h}, simd::float3{-h,-h,+h}, simd::float3{-h,+h,+h}, simd::float3{+h,+h,+h}, simd::float3{0,0,+1}, col); // +Z
    addQuad(simd::float3{-h,-h,-h}, simd::float3{+h,-h,-h}, simd::float3{+h,+h,-h}, simd::float3{-h,+h,-h}, simd::float3{0,0,-1}, col); // -Z
    
    m_indexCount = (uint32_t)idxs.size();
    
    m_vertexBuffer = m_device->newBuffer(verts.data(), verts.size() * sizeof(BlockGPUVertex), MTL::ResourceStorageModeShared);
    m_indexBuffer = m_device->newBuffer(idxs.data(), idxs.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
}

void VehicleRenderer::render(MTL::RenderCommandEncoder* enc, Vehicle& vehicle, BuildDragDrop& drag,
                             const BlockRegistry& registry, simd::float4x4 vpMatrix,
                             simd::float3 camPos, float time)
{
    if (!m_solidPipeline || !vehicle.commander) return;
    
    VehicleGPUUniforms* u = (VehicleGPUUniforms*)m_uniformBuffer->contents();
    u->viewProjection = vpMatrix;
    u->cameraPos = camPos;
    u->time = time;
    u->sunDir = simd::normalize(simd::float3{0.5f, 1.f, 0.3f});
    
    std::vector<BlockGPUInstance> instances;
    
    // Commander
    const BlockDefinition* cmdDef = registry.getDefinition(0);
    BlockGPUInstance ci;
    ci.modelMatrix = vehicle.commander->computeWorldMatrix(vehicle.position, vehicle.rotationMatrix);
    ci.tint = cmdDef ? cmdDef->baseColor : simd::float4{0.3f, 0.5f, 0.8f, 1.f};
    ci.typeID = 0;
    ci.state = 0;
    ci.healthRatio = vehicle.commander->currentHealth / 200.f;
    instances.push_back(ci);
    
    // Other blocks
    for (const auto& [id, blk] : vehicle.blocks) {
        if (!blk || blk->destroyed) continue;
        
        const BlockDefinition* def = registry.getDefinition(blk->definitionID);
        BlockGPUInstance bi;
        bi.modelMatrix = blk->computeWorldMatrix(vehicle.position, vehicle.rotationMatrix);
        bi.tint = def ? def->baseColor : simd::float4{0.5f, 0.5f, 0.5f, 1.f};
        bi.typeID = blk->definitionID;
        bi.state = 0;
        bi.healthRatio = blk->currentHealth / (def ? def->maxHealth : 100.f);
        instances.push_back(bi);
    }
    
    if (instances.empty()) return;
    
    memcpy(m_instanceBuffer->contents(), instances.data(), instances.size() * sizeof(BlockGPUInstance));
    
    enc->setRenderPipelineState(m_solidPipeline);
    enc->setDepthStencilState(m_depthState);
    enc->setVertexBuffer(m_vertexBuffer, 0, 0);
    enc->setVertexBuffer(m_uniformBuffer, 0, 1);
    enc->setVertexBuffer(m_instanceBuffer, 0, 2);
    enc->setFragmentBuffer(m_uniformBuffer, 0, 1);
    
    enc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)m_indexCount,
                               MTL::IndexTypeUInt32, m_indexBuffer, 0, (NS::UInteger)instances.size());
    
    // Ghost block
    if (m_ghostPipeline && drag.mode == BuildDragDrop::Mode::Placing) {
        simd::float4 ghostPos4 = simd_mul(vehicle.rotationMatrix, simd::float4{drag.ghostLocalPos.x, drag.ghostLocalPos.y, drag.ghostLocalPos.z, 1.f});
        simd::float3 gWorld = {vehicle.position.x + ghostPos4.x, vehicle.position.y + ghostPos4.y, vehicle.position.z + ghostPos4.z};
        
        BlockGPUInstance gi;
        gi.modelMatrix = simd_mul(math::makeTranslate(gWorld), simd_mul(vehicle.rotationMatrix, drag.ghostRotation));
        gi.tint = drag.validPlacement ? simd::float4{0.3f, 0.9f, 0.3f, 0.6f} : simd::float4{0.9f, 0.3f, 0.3f, 0.6f};
        gi.typeID = drag.draggedDefID;
        gi.state = 1;
        gi.healthRatio = 1.f;
        
        memcpy(m_instanceBuffer->contents(), &gi, sizeof(BlockGPUInstance));
        
        enc->setRenderPipelineState(m_ghostPipeline);
        enc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)m_indexCount,
                                   MTL::IndexTypeUInt32, m_indexBuffer, 0, 1);
    }
}

// ============================================================================
// INVENTORY RENDERER (simplifié - tu peux étendre)
// ============================================================================
InventoryRenderer::InventoryRenderer(MTL::Device* device, MTL::PixelFormat colorFmt,
                                     MTL::PixelFormat depthFmt, MTL::Library* library)
    : m_device(device), m_pipeline(nullptr), m_vertexBuffer(nullptr),
      m_uniformBuffer(nullptr), m_slotBuffer(nullptr)
{
    buildPipeline(colorFmt, depthFmt, library);
    buildQuadMesh();
    
    m_uniformBuffer = device->newBuffer(sizeof(InventoryGPUUniforms), MTL::ResourceStorageModeShared);
    m_slotBuffer = device->newBuffer(sizeof(InventorySlotGPU) * INVENTORY_SIZE, MTL::ResourceStorageModeShared);
}

InventoryRenderer::~InventoryRenderer()
{
    if (m_pipeline) m_pipeline->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_uniformBuffer) m_uniformBuffer->release();
    if (m_slotBuffer) m_slotBuffer->release();
}

void InventoryRenderer::buildPipeline(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* library)
{
    // Tu peux utiliser ton UI existant ou créer des shaders dédiés
    // Pour l'instant on skip le rendu UI complexe
}

void InventoryRenderer::buildQuadMesh()
{
    // Quad simple pour les slots
}

void InventoryRenderer::render(MTL::RenderCommandEncoder* enc, Inventory& inv,
                               const BlockRegistry& registry, simd::float2 screenSize, float time)
{
    // Render via ton système UI existant
    // L'inventaire est juste des données, le rendu peut utiliser ton UIRenderer
}

// ============================================================================
// VEHICLE MANAGER
// ============================================================================
VehicleManager::VehicleManager(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
    : m_buildMode(false), m_time(0.f), m_initialized(false)
{
    m_vehicle = std::make_unique<Vehicle>(1);
    m_vehicleRenderer = std::make_unique<VehicleRenderer>(device, pixelFormat, depthPixelFormat, shaderLibrary);
    m_inventoryRenderer = std::make_unique<InventoryRenderer>(device, pixelFormat, depthPixelFormat, shaderLibrary);
    
    m_inventory.setDefaultItems();
    m_initialized = true;
}

VehicleManager::~VehicleManager()
{
    cleanup();
}


void VehicleManager::cleanup()
{
    m_vehicleRenderer.reset();
    m_inventoryRenderer.reset();
    m_vehicle.reset();
    m_initialized = false;
}

void VehicleManager::update(float dt)
{
    m_time += dt;
    if (m_vehicle) {
        m_vehicle->updatePhysics(dt);
    }
}

void VehicleManager::render(MTL::RenderCommandEncoder* enc, simd::float4x4 vpMatrix, simd::float3 camPos)
{
    if (!m_initialized || !m_vehicle || !m_vehicleRenderer) return;
    m_vehicleRenderer->render(enc, *m_vehicle, m_dragDrop, m_registry, vpMatrix, camPos, m_time);
}

void VehicleManager::renderUI(MTL::RenderCommandEncoder* enc, simd::float2 screenSize)
{
    if (!m_initialized || !m_inventoryRenderer) return;
    m_inventoryRenderer->render(enc, m_inventory, m_registry, screenSize, m_time);
}

void VehicleManager::setThrottle(float val) { if (m_vehicle) m_vehicle->inputThrottle = val; }
void VehicleManager::setSteering(float val) { if (m_vehicle) m_vehicle->inputSteering = val; }
void VehicleManager::setBrake(float val) { if (m_vehicle) m_vehicle->inputBrake = val; }

simd::float3 VehicleManager::getCameraPosition() const {
    return m_vehicle ? m_vehicle->getCameraPosition() : simd::float3{0, 10, -15};
}

simd::float3 VehicleManager::getCameraTarget() const {
    return m_vehicle ? m_vehicle->getCameraTarget() : simd::float3{0, 0, 0};
}

void VehicleManager::orbitCamera(float dYaw, float dPitch) {
    if (m_vehicle) m_vehicle->orbitCamera(dYaw, dPitch);
}

void VehicleManager::zoomCamera(float delta) {
    if (m_vehicle) m_vehicle->zoomCamera(delta);
}

void VehicleManager::toggleBuildMode() {
    m_buildMode = !m_buildMode;
    m_inventory.isOpen = m_buildMode;
    if (!m_buildMode) m_dragDrop.cancelDrag();
}

bool VehicleManager::isBuildMode() const { return m_buildMode; }

void VehicleManager::rotateGhostBlock() { m_dragDrop.cycleGhostRotation(); }

void VehicleManager::selectInventorySlot(int32_t slot) {
    if (slot >= 0 && slot < (int32_t)INVENTORY_SIZE) {
        m_inventory.selectedSlot = slot;
    }
}

void VehicleManager::onMouseDown(simd::float2 normPos, simd::float2 screenSize, bool rightClick)
{
    if (rightClick) {
        m_dragDrop.cancelDrag();
        return;
    }
    
    if (m_buildMode) {
        // Check inventory hit
        int32_t slot = m_inventory.hitTestSlot(normPos, screenSize);
        if (slot >= 0) {
            m_dragDrop.startDrag(m_inventory, slot);
        }
        // Check panel drag
        else if (m_inventory.hitTestPanel(normPos, screenSize)) {
            m_inventory.isDraggingPanel = true;
            m_inventory.dragOffset = {normPos.x - m_inventory.panelPosition.x,
                                       normPos.y - m_inventory.panelPosition.y};
        }
    }
}

void VehicleManager::onMouseUp(simd::float2 normPos, simd::float2 screenSize)
{
    if (m_inventory.isDraggingPanel) {
        m_inventory.isDraggingPanel = false;
    }
    
    if (m_vehicle && m_dragDrop.mode != BuildDragDrop::Mode::Idle) {
        m_dragDrop.finishDrag(*m_vehicle, m_inventory, m_registry.definitions);
    }
}

void VehicleManager::onMouseMove(simd::float2 normPos, simd::float2 screenSize,
                                 simd::float3 rayOrigin, simd::float3 rayDir)
{
    // Update hovered slot
    m_inventory.hoveredSlot = m_inventory.hitTestSlot(normPos, screenSize);
    
    // Drag panel
    if (m_inventory.isDraggingPanel) {
        m_inventory.panelPosition = {normPos.x - m_inventory.dragOffset.x,
                                      normPos.y - m_inventory.dragOffset.y};
    }
    
    // Update ghost placement
    if (m_vehicle && m_dragDrop.mode != BuildDragDrop::Mode::Idle) {
        m_dragDrop.updateGhostPlacement(*m_vehicle, rayOrigin, rayDir);
    }
}

} // namespace TerraVehicle

//#include <cmath>
//#include <cfloat>
//
//namespace Vehicle {
//
//// ============================================================================
//// BLOCK STATS
//// ============================================================================
//BlockStats::BlockStats()
//    : mass(10.0f), maxHealth(100.0f), energyUse(0.0f), energyGen(0.0f), size{1,1,1} {}
//
//BlockStats::BlockStats(float m, float hp, float eUse, float eGen, simd_float3 sz)
//    : mass(m), maxHealth(hp), energyUse(eUse), energyGen(eGen), size(sz) {}
//
//// ============================================================================
//// ATTACH POINT
//// ============================================================================
//AttachPoint::AttachPoint()
//    : offset{0,0,0}, normal{0,1,0}, face(FacePosY), used(false), linkedBlockID(0) {}
//
//AttachPoint::AttachPoint(simd_float3 off, simd_float3 norm, AttachFace f)
//    : offset(off), normal(norm), face(f), used(false), linkedBlockID(0) {}
//
//// ============================================================================
//// BLOCK
//// ============================================================================
//Block::Block()
//    : id(0), type(BlockType::Armor), localPos{0,0,0},
//      localRot(simd_quaternion(0.f, simd_make_float3(0,1,0))),
//      currentHealth(100.f), destroyed(false)
//{
//    stats = BlockStats();
//    initAttachPoints();
//}
//
//Block::Block(uint32_t blockID, BlockType blockType)
//    : id(blockID), type(blockType), localPos{0,0,0},
//      localRot(simd_quaternion(0.f, simd_make_float3(0,1,0))),
//      currentHealth(100.f), destroyed(false)
//{
//    stats = BlockStats();
//    initAttachPoints();
//}
//
//void Block::initAttachPoints() {
//    float h = BLOCK_SIZE * 0.5f;
//    attachPoints[FacePosX] = AttachPoint({+h,0,0}, {+1,0,0}, FacePosX);
//    attachPoints[FaceNegX] = AttachPoint({-h,0,0}, {-1,0,0}, FaceNegX);
//    attachPoints[FacePosY] = AttachPoint({0,+h,0}, {0,+1,0}, FacePosY);
//    attachPoints[FaceNegY] = AttachPoint({0,-h,0}, {0,-1,0}, FaceNegY);
//    attachPoints[FacePosZ] = AttachPoint({0,0,+h}, {0,0,+1}, FacePosZ);
//    attachPoints[FaceNegZ] = AttachPoint({0,0,-h}, {0,0,-1}, FaceNegZ);
//}
//
//simd_float4x4 Block::buildModelMatrix(simd_float3 vehiclePos, simd_quatf vehicleRot) const {
//    simd_float3 worldPos = vehiclePos + simd_act(vehicleRot, localPos);
//    simd_quatf worldRot = simd_mul(vehicleRot, localRot);
//    
//    simd_float4x4 T = matrix_identity_float4x4;
//    T.columns[3] = simd_make_float4(worldPos.x, worldPos.y, worldPos.z, 1.0f);
//    simd_float4x4 R = simd_matrix4x4(worldRot);
//    return simd_mul(T, R);
//}
//
//// ============================================================================
//// COMMANDER BLOCK
//// ============================================================================
//CommanderBlock::CommanderBlock() : Block(0, BlockType::Commander) {
//    stats = BlockStats(50.f, 200.f, 1.f, 0.f, {1,1,1});
//    currentHealth = stats.maxHealth;
//    energyCapacity = 100.f;
//    currentEnergy = 100.f;
//    camDistance = 8.f;
//    camPitch = 0.3f;
//    camYaw = 0.f;
//}
//
//CommanderBlock::CommanderBlock(uint32_t blockID) : Block(blockID, BlockType::Commander) {
//    stats = BlockStats(50.f, 200.f, 1.f, 0.f, {1,1,1});
//    currentHealth = stats.maxHealth;
//    energyCapacity = 100.f;
//    currentEnergy = 100.f;
//    camDistance = 8.f;
//    camPitch = 0.3f;
//    camYaw = 0.f;
//}
//
//void CommanderBlock::update(float dt) {
//    currentEnergy -= stats.energyUse * dt;
//    if (currentEnergy < 0.f) currentEnergy = 0.f;
//}
//
//simd_float3 CommanderBlock::getCameraPosition(simd_float3 vehiclePos, simd_quatf vehicleRot) const {
//    float x = camDistance * cosf(camPitch) * sinf(camYaw);
//    float y = camDistance * sinf(camPitch) + 2.f;
//    float z = camDistance * cosf(camPitch) * cosf(camYaw);
//    return vehiclePos + simd_act(vehicleRot, simd_make_float3(x, y, z));
//}
//
//simd_float3 CommanderBlock::getCameraTarget(simd_float3 vehiclePos) const {
//    return vehiclePos + simd_make_float3(0, 1.5f, 0);
//}
//
//// ============================================================================
//// WHEEL BLOCK
//// ============================================================================
//WheelBlock::WheelBlock() : Block(0, BlockType::Wheel) {
//    stats = BlockStats(15.f, 50.f, 2.f, 0.f, {0.4f, 0.8f, 0.8f});
//    currentHealth = stats.maxHealth;
//    radius = 0.4f;
//    torque = 150.f;
//    spinAngle = 0.f;
//    steerAngle = 0.f;
//    powered = true;
//    steering = false;
//}
//
//WheelBlock::WheelBlock(uint32_t blockID) : Block(blockID, BlockType::Wheel) {
//    stats = BlockStats(15.f, 50.f, 2.f, 0.f, {0.4f, 0.8f, 0.8f});
//    currentHealth = stats.maxHealth;
//    radius = 0.4f;
//    torque = 150.f;
//    spinAngle = 0.f;
//    steerAngle = 0.f;
//    powered = true;
//    steering = false;
//}
//
//void WheelBlock::update(float dt) {
//    // Spin animation handled externally based on velocity
//}
//
//simd_float3 WheelBlock::getDriveForce(float throttle, simd_quatf vehicleRot) const {
//    if (!powered || destroyed) return {0,0,0};
//    simd_float3 fwd = simd_act(vehicleRot, simd_make_float3(0, 0, 1));
//    return fwd * throttle * torque;
//}
//
//// ============================================================================
//// VEHICLE ENTITY
//// ============================================================================
//VehicleEntity::VehicleEntity() : vehicleID(0) { init(); }
//VehicleEntity::VehicleEntity(uint32_t id) : vehicleID(id) { init(); }
//
//void VehicleEntity::init() {
//    name = "Vehicle";
//    nextBlockID = 1;
//    position = {0, 2, 0};
//    rotation = simd_quaternion(0.f, simd_make_float3(0,1,0));
//    velocity = {0,0,0};
//    angularVel = {0,0,0};
//    totalMass = 0.f;
//    centerOfMass = {0,0,0};
//    throttleInput = 0.f;
//    steerInput = 0.f;
//    brakeInput = 0.f;
//    onGround = false;
//    
//    commander = std::make_unique<CommanderBlock>(0);
//    recalculateMass();
//}
//
//void VehicleEntity::recalculateMass() {
//    totalMass = commander ? commander->stats.mass : 0.f;
//    simd_float3 weighted = commander ? commander->localPos * commander->stats.mass : simd_float3{0,0,0};
//    
//    for (auto& [id, blk] : blocks) {
//        if (blk && !blk->destroyed) {
//            totalMass += blk->stats.mass;
//            weighted += blk->localPos * blk->stats.mass;
//        }
//    }
//    centerOfMass = (totalMass > 0.f) ? weighted / totalMass : simd_float3{0,0,0};
//}
//
//void VehicleEntity::updatePhysics(float dt) {
//    if (!commander) return;
//    
//    commander->update(dt);
//    for (auto& [id, blk] : blocks) {
//        if (blk) blk->update(dt);
//    }
//    
//    // Forces
//    simd_float3 force = {0, -9.81f * totalMass, 0};
//    
//    // Drive force from wheels
//    for (auto& [id, blk] : blocks) {
//        if (blk && blk->type == BlockType::Wheel) {
//            WheelBlock* w = static_cast<WheelBlock*>(blk.get());
//            force += w->getDriveForce(throttleInput, rotation);
//        }
//    }
//    
//    // Air drag
//    float spd = simd_length(velocity);
//    if (spd > 0.01f) {
//        force -= simd_normalize(velocity) * spd * spd * 0.5f;
//    }
//    
//    // Ground check
//    float lowest = getLowestY();
//    onGround = (position.y + lowest) <= 0.1f;
//    
//    // Friction/brake
//    if (onGround && brakeInput > 0.f) {
//        simd_float3 fric = -velocity * 5.f * brakeInput;
//        fric.y = 0;
//        force += fric;
//    }
//    
//    // Integration
//    if (totalMass > 0.f) {
//        velocity += (force / totalMass) * dt;
//    }
//    position += velocity * dt;
//    
//    // Ground collision
//    if (position.y + lowest < 0.f) {
//        position.y = -lowest;
//        if (velocity.y < 0) velocity.y = 0;
//    }
//    
//    // Steering
//    if (onGround && fabsf(steerInput) > 0.01f && spd > 0.3f) {
//        float turn = steerInput * 2.5f * dt;
//        simd_quatf q = simd_quaternion(turn, simd_make_float3(0,1,0));
//        rotation = simd_normalize(simd_mul(q, rotation));
//        velocity = simd_act(q, velocity);
//    }
//}
//
//float VehicleEntity::getLowestY() const {
//    float low = commander ? (commander->localPos.y - 0.5f) : 0.f;
//    for (auto& [id, blk] : blocks) {
//        if (blk) {
//            float y = blk->localPos.y - blk->stats.size.y * 0.5f;
//            if (y < low) low = y;
//        }
//    }
//    return low;
//}
//
//bool VehicleEntity::addBlock(std::unique_ptr<Block> block, uint32_t parentID,
//                              AttachFace parentFace, AttachFace blockFace) {
//    if (!block) return false;
//    
//    Block* parent = (parentID == 0) ? commander.get() : nullptr;
//    if (parentID != 0) {
//        auto it = blocks.find(parentID);
//        if (it != blocks.end()) parent = it->second.get();
//    }
//    if (!parent) return false;
//    
//    if (parent->attachPoints[parentFace].used) return false;
//    if (block->attachPoints[blockFace].used) return false;
//    
//    simd_float3 off = parent->attachPoints[parentFace].offset - block->attachPoints[blockFace].offset;
//    block->localPos = parent->localPos + off;
//    block->id = nextBlockID++;
//    
//    parent->attachPoints[parentFace].used = true;
//    parent->attachPoints[parentFace].linkedBlockID = block->id;
//    block->attachPoints[blockFace].used = true;
//    block->attachPoints[blockFace].linkedBlockID = parentID;
//    
//    block->onAttached();
//    blocks[block->id] = std::move(block);
//    recalculateMass();
//    return true;
//}
//
//bool VehicleEntity::removeBlock(uint32_t blockID) {
//    if (blockID == 0) return false;
//    auto it = blocks.find(blockID);
//    if (it == blocks.end()) return false;
//    
//    Block* blk = it->second.get();
//    for (int i = 0; i < FaceCount; i++) {
//        if (blk->attachPoints[i].used) {
//            uint32_t linked = blk->attachPoints[i].linkedBlockID;
//            Block* other = (linked == 0) ? commander.get() : nullptr;
//            if (linked != 0) {
//                auto oit = blocks.find(linked);
//                if (oit != blocks.end()) other = oit->second.get();
//            }
//            if (other) {
//                for (int j = 0; j < FaceCount; j++) {
//                    if (other->attachPoints[j].linkedBlockID == blockID) {
//                        other->attachPoints[j].used = false;
//                        other->attachPoints[j].linkedBlockID = 0;
//                    }
//                }
//            }
//        }
//    }
//    
//    blk->onDetached();
//    blocks.erase(it);
//    recalculateMass();
//    return true;
//}
//
//simd_float3 VehicleEntity::getCameraPos() const {
//    if (!commander) return position + simd_make_float3(0, 5, -10);
//    return commander->getCameraPosition(position, rotation);
//}
//
//simd_float3 VehicleEntity::getCameraTarget() const {
//    if (!commander) return position;
//    return commander->getCameraTarget(position);
//}
//
//void VehicleEntity::orbitCamera(float dYaw, float dPitch) {
//    if (!commander) return;
//    commander->camYaw += dYaw;
//    commander->camPitch += dPitch;
//    if (commander->camPitch < -0.3f) commander->camPitch = -0.3f;
//    if (commander->camPitch > 1.4f) commander->camPitch = 1.4f;
//}
//
//void VehicleEntity::zoomCamera(float delta) {
//    if (!commander) return;
//    commander->camDistance += delta;
//    if (commander->camDistance < 3.f) commander->camDistance = 3.f;
//    if (commander->camDistance > 25.f) commander->camDistance = 25.f;
//}
//
//// ============================================================================
//// INVENTORY ITEM
//// ============================================================================
//InvItem::InvItem() : type(BlockType::Armor), count(0), name(""), factory(nullptr) {}
//
//InvItem::InvItem(BlockType t, uint32_t c, const std::string& n,
//                 std::function<std::unique_ptr<Block>(uint32_t)> f)
//    : type(t), count(c), name(n), factory(f) {}
//
//// ============================================================================
//// INVENTORY
//// ============================================================================
//Inventory::Inventory() : selected(0), visible(false) {
//    slots.resize(INVENTORY_SLOTS);
//}
//
//void Inventory::initDefaults() {
//    slots[0] = InvItem(BlockType::Wheel, 8, "Wheel",
//        [](uint32_t id) { return std::make_unique<WheelBlock>(id); });
//    
//    slots[1] = InvItem(BlockType::Armor, 20, "Armor",
//        [](uint32_t id) { return std::make_unique<Block>(id, BlockType::Armor); });
//    
//    slots[2] = InvItem(BlockType::Thruster, 4, "Thruster",
//        [](uint32_t id) {
//            auto b = std::make_unique<Block>(id, BlockType::Thruster);
//            b->stats = BlockStats(8.f, 40.f, 5.f, 0.f, {0.5f, 0.5f, 1.f});
//            return b;
//        });
//    
//    slots[3] = InvItem(BlockType::Generator, 2, "Generator",
//        [](uint32_t id) {
//            auto b = std::make_unique<Block>(id, BlockType::Generator);
//            b->stats = BlockStats(25.f, 80.f, 0.f, 10.f, {1,1,1});
//            return b;
//        });
//}
//
//InvItem* Inventory::getSelected() {
//    if (selected < 0 || selected >= (int32_t)slots.size()) return nullptr;
//    if (slots[selected].count == 0) return nullptr;
//    return &slots[selected];
//}
//
//bool Inventory::consume(int32_t slot) {
//    if (slot < 0 || slot >= (int32_t)slots.size()) return false;
//    if (slots[slot].count == 0) return false;
//    slots[slot].count--;
//    return true;
//}
//
//int32_t Inventory::hitTest(simd_float2 normPos, simd_float2 screenSize) const {
//    if (!visible) return -1;
//    
//    float slotW = 60.f / screenSize.x;
//    float slotH = 60.f / screenSize.y;
//    float pad = 8.f / screenSize.x;
//    float startX = 0.5f - (5.f * (slotW + pad)) + pad * 0.5f;
//    float startY = 0.1f;
//    
//    for (int row = 0; row < 5; row++) {
//        for (int col = 0; col < 10; col++) {
//            int idx = row * 10 + col;
//            float x = startX + col * (slotW + pad);
//            float y = startY + row * (slotH + pad);
//            
//            if (normPos.x >= x && normPos.x <= x + slotW &&
//                normPos.y >= y && normPos.y <= y + slotH) {
//                return idx;
//            }
//        }
//    }
//    return -1;
//}
//
//// ============================================================================
//// DRAG DROP
//// ============================================================================
//DragDrop::DragDrop()
//    : state(State::Idle), sourceSlot(-1), draggedItem(nullptr),
//      ghostPos{0,0,0}, ghostRot(simd_quaternion(0.f, simd_make_float3(0,1,0))),
//      targetBlockID(0), targetFace(FacePosY), sourceFace(FaceNegY), canPlace(false) {}
//
//void DragDrop::beginDrag(Inventory& inv, int32_t slot) {
//    if (slot < 0 || slot >= (int32_t)inv.slots.size()) return;
//    if (inv.slots[slot].count == 0) return;
//    
//    state = State::Dragging;
//    sourceSlot = slot;
//    draggedItem = &inv.slots[slot];
//    inv.selected = slot;
//}
//
//bool DragDrop::endDrag(VehicleEntity& vehicle, Inventory& inv) {
//    if (state == State::Idle) return false;
//    
//    bool success = false;
//    if (state == State::Placing && canPlace && draggedItem && draggedItem->factory) {
//        auto newBlock = draggedItem->factory(vehicle.nextBlockID);
//        newBlock->localRot = ghostRot;
//        
//        if (vehicle.addBlock(std::move(newBlock), targetBlockID, targetFace, sourceFace)) {
//            inv.consume(sourceSlot);
//            success = true;
//        }
//    }
//    
//    cancel();
//    return success;
//}
//
//void DragDrop::cancel() {
//    state = State::Idle;
//    sourceSlot = -1;
//    draggedItem = nullptr;
//    canPlace = false;
//}
//
//void DragDrop::rotatePart() {
//    sourceFace = (AttachFace)((sourceFace + 1) % FaceCount);
//}
//
//void DragDrop::updatePlacement(VehicleEntity& vehicle, simd_float3 rayOrigin, simd_float3 rayDir) {
//    if (state == State::Idle) return;
//    state = State::Placing;
//    canPlace = false;
//    
//    float bestDist = FLT_MAX;
//    
//    auto testBlock = [&](Block* blk) {
//        simd_float3 wpos = vehicle.position + simd_act(vehicle.rotation, blk->localPos);
//        simd_float3 toBlock = wpos - rayOrigin;
//        float t = simd_dot(toBlock, rayDir);
//        if (t < 0) return;
//        
//        simd_float3 closest = rayOrigin + rayDir * t;
//        float dist = simd_length(closest - wpos);
//        
//        if (dist < 1.5f && t < bestDist) {
//            simd_float3 localHit = closest - wpos;
//            
//            AttachFace bestFace = FacePosY;
//            float maxDot = -FLT_MAX;
//            for (int i = 0; i < FaceCount; i++) {
//                if (blk->attachPoints[i].used) continue;
//                float d = simd_dot(localHit, blk->attachPoints[i].normal);
//                if (d > maxDot) {
//                    maxDot = d;
//                    bestFace = (AttachFace)i;
//                }
//            }
//            
//            if (!blk->attachPoints[bestFace].used) {
//                bestDist = t;
//                targetBlockID = blk->id;
//                targetFace = bestFace;
//                ghostPos = blk->localPos + blk->attachPoints[bestFace].offset * 2.f;
//                canPlace = true;
//            }
//        }
//    };
//    
//    if (vehicle.commander) testBlock(vehicle.commander.get());
//    for (auto& [id, blk] : vehicle.blocks) {
//        if (blk) testBlock(blk.get());
//    }
//}
//
//// ============================================================================
//// VEHICLE RENDERER
//// ============================================================================
//VehicleRenderer::VehicleRenderer()
//    : device(nullptr), solidPipeline(nullptr), ghostPipeline(nullptr),
//      depthState(nullptr), vertexBuffer(nullptr), indexBuffer(nullptr),
//      instanceBuffer(nullptr), uniformBuffer(nullptr), indexCount(0) {}
//
//VehicleRenderer::~VehicleRenderer() { cleanup(); }
//
//void VehicleRenderer::cleanup() {
//    if (solidPipeline) { solidPipeline->release(); solidPipeline = nullptr; }
//    if (ghostPipeline) { ghostPipeline->release(); ghostPipeline = nullptr; }
//    if (depthState) { depthState->release(); depthState = nullptr; }
//    if (vertexBuffer) { vertexBuffer->release(); vertexBuffer = nullptr; }
//    if (indexBuffer) { indexBuffer->release(); indexBuffer = nullptr; }
//    if (instanceBuffer) { instanceBuffer->release(); instanceBuffer = nullptr; }
//    if (uniformBuffer) { uniformBuffer->release(); uniformBuffer = nullptr; }
//}
//
//void VehicleRenderer::init(MTL::Device* dev, MTL::PixelFormat colorFmt,
//                           MTL::PixelFormat depthFmt, MTL::Library* library) {
//    device = dev;
//    createPipelines(colorFmt, depthFmt, library);
//    buildCommanderMesh();
//    
//    instanceBuffer = device->newBuffer(sizeof(BlockInstance) * MAX_BLOCKS_PER_VEHICLE,
//                                        MTL::ResourceStorageModeShared);
//    uniformBuffer = device->newBuffer(sizeof(VehicleUniforms), MTL::ResourceStorageModeShared);
//}
//
//void VehicleRenderer::addQuad(std::vector<BlockVertex>& verts, std::vector<uint32_t>& idxs,
//                               simd_float3 p0, simd_float3 p1, simd_float3 p2, simd_float3 p3,
//                               simd_float3 normal, simd_float4 color) {
//    uint32_t base = (uint32_t)verts.size();
//    verts.push_back({p0, normal, {0,0}, color});
//    verts.push_back({p1, normal, {1,0}, color});
//    verts.push_back({p2, normal, {1,1}, color});
//    verts.push_back({p3, normal, {0,1}, color});
//    idxs.push_back(base); idxs.push_back(base+1); idxs.push_back(base+2);
//    idxs.push_back(base); idxs.push_back(base+2); idxs.push_back(base+3);
//}
//
//void VehicleRenderer::buildCommanderMesh() {
//    std::vector<BlockVertex> verts;
//    std::vector<uint32_t> idxs;
//    
//    float h = 0.45f;
//    simd_float4 col = {0.2f, 0.4f, 0.7f, 1.f};
//    
//    // +Y
//    addQuad(verts, idxs, {-h,h,-h}, {h,h,-h}, {h,h,h}, {-h,h,h}, {0,1,0}, col);
//    // -Y
//    addQuad(verts, idxs, {-h,-h,h}, {h,-h,h}, {h,-h,-h}, {-h,-h,-h}, {0,-1,0}, col);
//    // +X
//    addQuad(verts, idxs, {h,-h,-h}, {h,-h,h}, {h,h,h}, {h,h,-h}, {1,0,0}, col);
//    // -X
//    addQuad(verts, idxs, {-h,-h,h}, {-h,-h,-h}, {-h,h,-h}, {-h,h,h}, {-1,0,0}, col);
//    // +Z
//    addQuad(verts, idxs, {h,-h,h}, {-h,-h,h}, {-h,h,h}, {h,h,h}, {0,0,1}, col);
//    // -Z
//    addQuad(verts, idxs, {-h,-h,-h}, {h,-h,-h}, {h,h,-h}, {-h,h,-h}, {0,0,-1}, col);
//    
//    // Cockpit dome
//    simd_float4 glowCol = {0.4f, 0.8f, 1.f, 1.f};
//    float dh = 0.2f;
//    addQuad(verts, idxs, {-0.2f,h,-0.1f}, {0.2f,h,-0.1f}, {0.15f,h+dh,0.05f}, {-0.15f,h+dh,0.05f},
//            {0,1,0}, glowCol);
//    
//    indexCount = (uint32_t)idxs.size();
//    
//    vertexBuffer = device->newBuffer(verts.data(), verts.size() * sizeof(BlockVertex),
//                                      MTL::ResourceStorageModeShared);
//    indexBuffer = device->newBuffer(idxs.data(), idxs.size() * sizeof(uint32_t),
//                                     MTL::ResourceStorageModeShared);
//}
//
//void VehicleRenderer::createPipelines(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt,
//                                       MTL::Library* library) {
//    NS::Error* err = nullptr;
//    
//    MTL::Function* vsFunc = library->newFunction(MTLSTR("vehicleBlockVS"));
//    MTL::Function* fsFunc = library->newFunction(MTLSTR("vehicleBlockFS"));
//    MTL::Function* ghostFS = library->newFunction(MTLSTR("vehicleGhostFS"));
//    
//    if (!vsFunc || !fsFunc) {
//        printf("Vehicle shader functions not found in library!\n");
//        return;
//    }
//    
//    MTL::VertexDescriptor* vd = MTL::VertexDescriptor::alloc()->init();
//    vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//    vd->attributes()->object(0)->setOffset(0);
//    vd->attributes()->object(0)->setBufferIndex(0);
//    vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//    vd->attributes()->object(1)->setOffset(12);
//    vd->attributes()->object(1)->setBufferIndex(0);
//    vd->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
//    vd->attributes()->object(2)->setOffset(24);
//    vd->attributes()->object(2)->setBufferIndex(0);
//    vd->attributes()->object(3)->setFormat(MTL::VertexFormatFloat4);
//    vd->attributes()->object(3)->setOffset(32);
//    vd->attributes()->object(3)->setBufferIndex(0);
//    vd->layouts()->object(0)->setStride(sizeof(BlockVertex));
//    
//    MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
//    pd->setVertexFunction(vsFunc);
//    pd->setFragmentFunction(fsFunc);
//    pd->setVertexDescriptor(vd);
//    pd->colorAttachments()->object(0)->setPixelFormat(colorFmt);
//    pd->setDepthAttachmentPixelFormat(depthFmt);
//    
//    solidPipeline = device->newRenderPipelineState(pd, &err);
//    if (err) printf("Solid pipeline error\n");
//    
//    if (ghostFS) {
//        pd->setFragmentFunction(ghostFS);
//        pd->colorAttachments()->object(0)->setBlendingEnabled(true);
//        pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        ghostPipeline = device->newRenderPipelineState(pd, &err);
//    }
//    
//    MTL::DepthStencilDescriptor* dsd = MTL::DepthStencilDescriptor::alloc()->init();
//    dsd->setDepthCompareFunction(MTL::CompareFunctionLess);
//    dsd->setDepthWriteEnabled(true);
//    depthState = device->newDepthStencilState(dsd);
//    
//    vsFunc->release();
//    fsFunc->release();
//    if (ghostFS) ghostFS->release();
//    vd->release();
//    pd->release();
//    dsd->release();
//}
//
//void VehicleRenderer::render(MTL::RenderCommandEncoder* enc, VehicleEntity& vehicle,
//                              DragDrop& drag, simd_float4x4 vpMatrix,
//                              simd_float3 camPos, float time) {
//    if (!solidPipeline || !vehicle.commander) return;
//    
//    VehicleUniforms* u = (VehicleUniforms*)uniformBuffer->contents();
//    u->viewProjectionMatrix = vpMatrix;
//    u->cameraPosition = camPos;
//    u->time = time;
//    u->sunDirection = simd_normalize(simd_make_float3(0.5f, 1.f, 0.3f));
//    
//    std::vector<BlockInstance> instances;
//    
//    // Commander
//    BlockInstance ci;
//    ci.modelMatrix = vehicle.commander->buildModelMatrix(vehicle.position, vehicle.rotation);
//    ci.tintColor = {1,1,1,1};
//    ci.blockType = (uint32_t)BlockType::Commander;
//    ci.flags = 0;
//    ci.health = vehicle.commander->currentHealth / vehicle.commander->stats.maxHealth;
//    instances.push_back(ci);
//    
//    // Other blocks
//    for (auto& [id, blk] : vehicle.blocks) {
//        if (!blk || blk->destroyed) continue;
//        BlockInstance bi;
//        bi.modelMatrix = blk->buildModelMatrix(vehicle.position, vehicle.rotation);
//        bi.tintColor = {1,1,1,1};
//        bi.blockType = (uint32_t)blk->type;
//        bi.flags = 0;
//        bi.health = blk->currentHealth / blk->stats.maxHealth;
//        instances.push_back(bi);
//    }
//    
//    if (instances.empty()) return;
//    
//    memcpy(instanceBuffer->contents(), instances.data(), instances.size() * sizeof(BlockInstance));
//    
//    enc->setRenderPipelineState(solidPipeline);
//    enc->setDepthStencilState(depthState);
//    enc->setVertexBuffer(vertexBuffer, 0, 0);
//    enc->setVertexBuffer(uniformBuffer, 0, 1);
//    enc->setVertexBuffer(instanceBuffer, 0, 2);
//    enc->setFragmentBuffer(uniformBuffer, 0, 1);
//    
//    enc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)indexCount,
//                               MTL::IndexTypeUInt32, indexBuffer, 0, (NS::UInteger)instances.size());
//    
//    // Ghost block
//    if (ghostPipeline && drag.state == DragDrop::State::Placing) {
//        BlockInstance gi;
//        simd_float3 gWorld = vehicle.position + simd_act(vehicle.rotation, drag.ghostPos);
//        simd_float4x4 T = matrix_identity_float4x4;
//        T.columns[3] = simd_make_float4(gWorld.x, gWorld.y, gWorld.z, 1.f);
//        simd_float4x4 R = simd_matrix4x4(simd_mul(vehicle.rotation, drag.ghostRot));
//        gi.modelMatrix = simd_mul(T, R);
//        gi.tintColor = drag.canPlace ? simd_make_float4(0.3f,0.9f,0.3f,0.6f)
//                                     : simd_make_float4(0.9f,0.3f,0.3f,0.6f);
//        gi.blockType = drag.draggedItem ? (uint32_t)drag.draggedItem->type : 0;
//        gi.flags = 1;
//        gi.health = 1.f;
//        
//        memcpy(instanceBuffer->contents(), &gi, sizeof(BlockInstance));
//        
//        enc->setRenderPipelineState(ghostPipeline);
//        enc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)indexCount,
//                                   MTL::IndexTypeUInt32, indexBuffer, 0, 1);
//    }
//}
//
//// ============================================================================
//// VEHICLE MANAGER
//// ============================================================================
//VehicleManager::VehicleManager() : buildMode(false), time(0.f) {}
//VehicleManager::~VehicleManager() { cleanup(); }
//
//void VehicleManager::init(MTL::Device* dev, MTL::PixelFormat colorFmt,
//                          MTL::PixelFormat depthFmt, MTL::Library* library) {
//    vehicle = std::make_unique<VehicleEntity>(1);
//    inventory.initDefaults();
//    renderer.init(dev, colorFmt, depthFmt, library);
//}
//
//void VehicleManager::cleanup() {
//    renderer.cleanup();
//    vehicle.reset();
//}
//
//void VehicleManager::update(float dt) {
//    time += dt;
//    if (vehicle) vehicle->updatePhysics(dt);
//}
//
//void VehicleManager::render(MTL::RenderCommandEncoder* enc, simd_float4x4 vpMatrix, simd_float3 camPos) {
//    if (vehicle) {
//        renderer.render(enc, *vehicle, dragDrop, vpMatrix, camPos, time);
//    }
//}
//
//void VehicleManager::setThrottle(float val) { if (vehicle) vehicle->throttleInput = val; }
//void VehicleManager::setSteering(float val) { if (vehicle) vehicle->steerInput = val; }
//void VehicleManager::setBrake(float val) { if (vehicle) vehicle->brakeInput = val; }
//
//simd_float3 VehicleManager::getCameraPosition() const {
//    return vehicle ? vehicle->getCameraPos() : simd_float3{0,5,-10};
//}
//simd_float3 VehicleManager::getCameraTarget() const {
//    return vehicle ? vehicle->getCameraTarget() : simd_float3{0,0,0};
//}
//void VehicleManager::orbitCamera(float dYaw, float dPitch) {
//    if (vehicle) vehicle->orbitCamera(dYaw, dPitch);
//}
//void VehicleManager::zoomCamera(float delta) {
//    if (vehicle) vehicle->zoomCamera(delta);
//}
//
//void VehicleManager::toggleBuildMode() {
//    buildMode = !buildMode;
//    inventory.visible = buildMode;
//    if (!buildMode) dragDrop.cancel();
//}
//
//void VehicleManager::handleClick(simd_float2 normPos, simd_float2 screenSize, bool rightClick) {
//    if (rightClick) {
//        dragDrop.cancel();
//        return;
//    }
//    
//    if (buildMode) {
//        int32_t slot = inventory.hitTest(normPos, screenSize);
//        if (slot >= 0) {
//            dragDrop.beginDrag(inventory, slot);
//        }
//    }
//}
//
//void VehicleManager::handleRelease() {
//    if (vehicle && dragDrop.state != DragDrop::State::Idle) {
//        dragDrop.endDrag(*vehicle, inventory);
//    }
//}
//
//void VehicleManager::handleMouseMove(simd_float3 rayOrigin, simd_float3 rayDir) {
//    if (vehicle && dragDrop.state != DragDrop::State::Idle) {
//        dragDrop.updatePlacement(*vehicle, rayOrigin, rayDir);
//    }
//}
//
//void VehicleManager::rotateGhost() { dragDrop.rotatePart(); }
//void VehicleManager::selectSlot(int32_t slot) { inventory.selected = slot; }
//
//} // namespace Vehicle
