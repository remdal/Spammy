//
//  RMDLPetitPrince.hpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#ifndef RMDLPetitPrince_hpp
#define RMDLPetitPrince_hpp

#include <Metal/Metal.hpp>

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <simd/simd.h>
#include <unordered_map>

namespace NASAAtTheHelm {

class Vehicle;
class VehicleBlock;
class CommandBlock;
class Inventory;
class DragDropSystem;

enum class BlockType : uint32_t {
    Commander   = 0,
    Wheel       = 1,
    Thruster    = 2,
    Weapon      = 3,
    Armor       = 4,
    Generator   = 5,
    Storage     = 6,
    Sensor      = 7,
    Connector   = 8,
    COUNT
};

struct AttachmentPoint
{
    simd_float3 localPosition;
    simd_float3 normal;             // Direction de l'attache (sortante)
    bool occupied = false;          // Déjà connecté?
    uint32_t connectedBlockID = 0;  // ID du bloc connecté
    
    enum Face : uint8_t {
        PosX = 0, NegX, PosY, NegY, PosZ, NegZ, COUNT
    };
    Face face;
};

struct BlockStats
{
    float mass;                 // kg
    float health;
    float energyConsumption;    // énergie/seconde
    float energyProduction;
    float drag;                 // Résistance aérodynamique
    simd_float3 size;           // max 1.0 pour tenir dans 1∛ float
};

class VehicleBlock
{
public:
    uint32_t uniqueID;
    BlockType type;
    BlockStats stats;
    
    simd_float3 localPosition;
    simd_quatf localRotation;                   // Rotation locale (multiples de 90°)
    
    std::vector<AttachmentPoint> attachPoints;
    
    // Rendering
    uint32_t meshID;                            // ID du mesh dans le MeshLibrary
    uint32_t materialID;                        // ID du matériau
    
    float currentHealth;
    bool isDestroyed = false;
    
    VehicleBlock(BlockType t, uint32_t id);
    
    virtual ~VehicleBlock() = default;
    
    // Méthodes virtuelles pour comportement spécifique
    virtual void update(float deltaTime, Vehicle& vehicle) {}
    virtual void onAttach(Vehicle& vehicle) {}
    virtual void onDetach(Vehicle& vehicle) {}
    virtual void onDamage(float damage, Vehicle& vehicle) {
        currentHealth -= damage;
        if (currentHealth <= 0) {
            isDestroyed = true;
        }
    }
    
    void initializeAttachmentPoints();
    
    simd_float3 getWorldPosition(const simd_float3& vehiclePos, const simd_quatf& vehicleRot) const;
    
    simd_float4x4 getModelMatrix(const simd_float3& vehiclePos, const simd_quatf& vehicleRot) const;
};


class CommandBlock : public VehicleBlock
{
public:
    float energyCapacity = 100.0f;
    float currentEnergy = 100.0f;
    
    simd_float3 cameraOffset = {0.0f, 2.0f, -5.0f};  // Offset 3ème personne
    float cameraDistance = 5.0f;
    float cameraPitch = 0.2f;
    float cameraYaw = 0.0f;
    
    CommandBlock(uint32_t id);
    
    void update(float deltaTime, Vehicle& vehicle) override
    {
        // Consommation d'énergie de base
        currentEnergy -= stats.energyConsumption * deltaTime;
        currentEnergy = simd_clamp(currentEnergy, 0.0f, energyCapacity);
    }
    
    simd_float3 getCameraWorldPosition(const simd_float3& vehiclePos, const simd_quatf& vehicleRot) const;
    simd_float3 getCameraTarget(const simd_float3& vehiclePos) const;
};


class WheelBlock : public VehicleBlock
{
public:
    float wheelRadius = 0.4f;
    float motorTorque = 100.0f;         // Force motrice
    float brakeTorque = 200.0f;
    float currentRotation = 0.0f;       // Rotation visuelle
    float angularVelocity = 0.0f;
    bool isPowered = true;              // Roue motrice?
    bool isSteering = false;            // Roue directrice?
    float steerAngle = 0.0f;
    
    WheelBlock(uint32_t id) : VehicleBlock(BlockType::Wheel, id) {
        stats = {
            .mass = 15.0f,
            .health = 50.0f,
            .energyConsumption = 2.0f,  // Quand motorisée
            .energyProduction = 0.0f,
            .drag = 0.1f,
            .size = {0.3f, 0.8f, 0.8f}  // Fine, ronde
        };
        currentHealth = stats.health;
    }
    
    void update(float deltaTime, Vehicle& vehicle) override;  // Défini plus bas
    
    // Force appliquée au véhicule
    simd_float3 calculateDriveForce(float throttle, const simd_quatf& vehicleRot) const {
        if (!isPowered || isDestroyed) return {0, 0, 0};
        
        simd_float3 forward = simd_act(vehicleRot, simd_make_float3(0, 0, 1));
        return forward * throttle * motorTorque;
    }
};

class Vehicle
{
public:
    uint32_t vehicleID;
    std::string name = "New Vehicle";
    std::unique_ptr<CommandBlock> commander;
    std::unordered_map<uint32_t, std::unique_ptr<VehicleBlock>> blocks;
    uint32_t nextBlockID = 1;

    simd_float3 position = {0, 0, 0};
    simd_quatf rotation = simd_quaternion(0.0f, simd_make_float3(0, 1, 0)); // simd::quatf
    simd_float3 velocity = {0, 0, 0};
    simd_float3 angularVelocity = {0, 0, 0};
    
    float totalMass = 0.0f;
    simd_float3 centerOfMass = {0, 0, 0};   // Relative au commandant
    float totalDrag = 0.0f;
    
    float throttle = 0.0f;      // -1 à 1
    float steering = 0.0f;      // -1 à 1
    float brake = 0.0f;         // 0 à 1
    bool isGrounded = false;
    
    static constexpr float GRAVITY = -9.81f;
    static constexpr float GROUND_LEVEL = 0.0f;  // Temporaire
    static constexpr float GROUND_FRICTION = 0.8f;
    static constexpr float AIR_RESISTANCE = 0.02f;
    
    Vehicle(uint32_t id) : vehicleID(id) {
        // Créer le commandant obligatoire
        commander = std::make_unique<CommandBlock>(0);
        recalculatePhysicsProperties();
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // GESTION DES BLOCS
    // ═══════════════════════════════════════════════════════════════════════
    
    // Attache un nouveau bloc au véhicule
    bool attachBlock(std::unique_ptr<VehicleBlock> block,
                     uint32_t parentBlockID,
                     AttachmentPoint::Face parentFace,
                     AttachmentPoint::Face childFace) {
        
        // Trouver le bloc parent
        VehicleBlock* parent = nullptr;
        if (parentBlockID == 0) {
            parent = commander.get();
        } else {
            auto it = blocks.find(parentBlockID);
            if (it == blocks.end()) return false;
            parent = it->second.get();
        }
        
        // Vérifier que le point d'attache est libre
        if (parent->attachPoints[parentFace].occupied) return false;
        if (block->attachPoints[childFace].occupied) return false;
        
        // Calculer la position du nouveau bloc
        simd_float3 attachOffset = parent->attachPoints[parentFace].localPosition;
        simd_float3 childOffset = block->attachPoints[childFace].localPosition;
        
        block->localPosition = parent->localPosition + attachOffset - childOffset;
        block->uniqueID = nextBlockID++;
        
        // Marquer les points comme occupés
        parent->attachPoints[parentFace].occupied = true;
        parent->attachPoints[parentFace].connectedBlockID = block->uniqueID;
        block->attachPoints[childFace].occupied = true;
        block->attachPoints[childFace].connectedBlockID = parentBlockID;
        
        // Callback
        block->onAttach(*this);
        
        // Ajouter au véhicule
        blocks[block->uniqueID] = std::move(block);
        
        // Recalculer physique
        recalculatePhysicsProperties();
        
        return true;
    }
    
    // Détache un bloc (et tous ses enfants!)
    bool detachBlock(uint32_t blockID) {
        if (blockID == 0) return false;  // Impossible de détacher le commandant
        
        auto it = blocks.find(blockID);
        if (it == blocks.end()) return false;
        
        VehicleBlock* block = it->second.get();
        
        // Libérer les points d'attache connectés
        for (auto& ap : block->attachPoints) {
            if (ap.occupied) {
                // Trouver le bloc connecté et libérer son point
                VehicleBlock* connected = nullptr;
                if (ap.connectedBlockID == 0) {
                    connected = commander.get();
                } else {
                    auto cit = blocks.find(ap.connectedBlockID);
                    if (cit != blocks.end()) connected = cit->second.get();
                }
                
                if (connected) {
                    for (auto& cap : connected->attachPoints) {
                        if (cap.connectedBlockID == blockID) {
                            cap.occupied = false;
                            cap.connectedBlockID = 0;
                        }
                    }
                }
            }
        }
        
        block->onDetach(*this);
        
        // TODO: Détacher récursivement les enfants
        // Pour l'instant on supprime juste ce bloc
        blocks.erase(it);
        
        recalculatePhysicsProperties();
        return true;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // PHYSIQUE
    // ═══════════════════════════════════════════════════════════════════════
    
    void recalculatePhysicsProperties() {
        totalMass = commander->stats.mass;
        totalDrag = commander->stats.drag;
        simd_float3 weightedPos = commander->localPosition * commander->stats.mass;
        
        for (const auto& [id, block] : blocks) {
            if (!block->isDestroyed) {
                totalMass += block->stats.mass;
                totalDrag += block->stats.drag;
                weightedPos += block->localPosition * block->stats.mass;
            }
        }
        
        centerOfMass = weightedPos / totalMass;
    }
    
    void update(float deltaTime) {
        // ─────────────────────────────────────────────────────────────────
        // 1. Mise à jour des blocs
        commander->update(deltaTime, *this);
        for (auto& [id, block] : blocks) {
            block->update(deltaTime, *this);
        }
        
        // ─────────────────────────────────────────────────────────────────
        // 2. Calculer les forces
        simd_float3 totalForce = {0, 0, 0};
        
        // Gravité
        totalForce.y += GRAVITY * totalMass;
        
        // Force des roues
        for (auto& [id, block] : blocks) {
            if (block->type == BlockType::Wheel) {
                WheelBlock* wheel = static_cast<WheelBlock*>(block.get());
                totalForce += wheel->calculateDriveForce(throttle, rotation);
            }
        }
        
        // Résistance de l'air
        float speed = simd_length(velocity);
        if (speed > 0.001f) {
            simd_float3 dragForce = -simd_normalize(velocity) * speed * speed *
                                     totalDrag * AIR_RESISTANCE;
            totalForce += dragForce;
        }
        
        // ─────────────────────────────────────────────────────────────────
        // 3. Détection sol (simplifiée)
        float lowestPoint = getLowestPoint();
        isGrounded = (position.y + lowestPoint) <= GROUND_LEVEL + 0.1f;
        
        // Friction au sol
        if (isGrounded && brake > 0.0f) {
            simd_float3 frictionForce = -velocity * GROUND_FRICTION * brake * totalMass;
            frictionForce.y = 0;
            totalForce += frictionForce;
        }
        
        // ─────────────────────────────────────────────────────────────────
        // 4. Intégration (Euler semi-implicite)
        simd_float3 acceleration = totalForce / totalMass;
        velocity += acceleration * deltaTime;
        position += velocity * deltaTime;
        
        // ─────────────────────────────────────────────────────────────────
        // 5. Collision sol
        if (position.y + lowestPoint < GROUND_LEVEL) {
            position.y = GROUND_LEVEL - lowestPoint;
            velocity.y = 0;
        }
        
        // ─────────────────────────────────────────────────────────────────
        // 6. Rotation (steering)
        if (isGrounded && fabsf(steering) > 0.01f && speed > 0.5f) {
            float turnRate = steering * 2.0f * deltaTime;  // Vitesse de rotation
            simd_quatf turnQuat = simd_quaternion(turnRate, simd_make_float3(0, 1, 0));
            rotation = simd_mul(turnQuat, rotation);
            rotation = simd_normalize(rotation);
            
            // Tourner aussi la vélocité
            velocity = simd_act(turnQuat, velocity);
        }
    }
    
    float getLowestPoint() const {
        float lowest = 0.0f;
        for (const auto& [id, block] : blocks) {
            float blockBottom = block->localPosition.y - block->stats.size.y * 0.5f;
            lowest = fminf(lowest, blockBottom);
        }
        // Commander aussi
        float cmdBottom = commander->localPosition.y - commander->stats.size.y * 0.5f;
        return fminf(lowest, cmdBottom);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // CAMÉRA
    // ═══════════════════════════════════════════════════════════════════════
    
    simd_float3 getCameraPosition() const {
        return commander->getCameraWorldPosition(position, rotation);
    }
    
    simd_float3 getCameraTarget() const {
        return commander->getCameraTarget(position);
    }
    
    void rotateCameraOrbit(float deltaYaw, float deltaPitch) {
        commander->cameraYaw += deltaYaw;
        commander->cameraPitch += deltaPitch;
        commander->cameraPitch = simd_clamp(commander->cameraPitch, -0.5f, 1.2f);
    }
    
    void zoomCamera(float delta) {
        commander->cameraDistance += delta;
        commander->cameraDistance = simd_clamp(commander->cameraDistance, 2.0f, 20.0f);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// INVENTORY ITEM - Un item dans l'inventaire
// ═══════════════════════════════════════════════════════════════════════════
struct InventoryItem {
    BlockType type;
    uint32_t count;
    uint32_t iconTextureID;     // Texture de l'icône
    std::string displayName;
    std::string description;
    
    // Factory function pour créer le bloc
    std::function<std::unique_ptr<VehicleBlock>(uint32_t id)> createBlock;
};

// ═══════════════════════════════════════════════════════════════════════════
// INVENTORY - Système d'inventaire
// ═══════════════════════════════════════════════════════════════════════════
class Inventory {
public:
    static constexpr uint32_t SLOTS_PER_ROW = 10;
    static constexpr uint32_t MAX_ROWS = 5;
    static constexpr uint32_t MAX_SLOTS = SLOTS_PER_ROW * MAX_ROWS;
    
    std::vector<InventoryItem> slots;
    int32_t selectedSlot = -1;
    bool isVisible = false;
    
    // Position UI (normalisée 0-1)
    simd_float2 position = {0.5f, 0.9f};  // Bas centre
    simd_float2 slotSize = {0.06f, 0.08f};
    float padding = 0.01f;
    
    Inventory() {
        slots.resize(MAX_SLOTS);
        initializeDefaultItems();
    }
    
    void initializeDefaultItems() {
        // Slot 0: Roues
        slots[0] = {
            .type = BlockType::Wheel,
            .count = 10,
            .iconTextureID = 1,
            .displayName = "Standard Wheel",
            .description = "Basic motorized wheel for ground vehicles.",
            .createBlock = [](uint32_t id) { return std::make_unique<WheelBlock>(id); }
        };
        
        // Ajouter d'autres items par défaut...
    }
    
    // Récupère l'item sélectionné
    InventoryItem* getSelectedItem() {
        if (selectedSlot < 0 || selectedSlot >= slots.size()) return nullptr;
        if (slots[selectedSlot].count == 0) return nullptr;
        return &slots[selectedSlot];
    }
    
    // Consomme un item (après placement)
    bool consumeItem(int32_t slotIndex) {
        if (slotIndex < 0 || slotIndex >= slots.size()) return false;
        if (slots[slotIndex].count == 0) return false;
        slots[slotIndex].count--;
        return true;
    }
    
    // Ajoute un item (drop ou pickup)
    bool addItem(BlockType type, uint32_t count = 1) {
        // Cherche un slot existant du même type
        for (auto& slot : slots) {
            if (slot.type == type && slot.count > 0) {
                slot.count += count;
                return true;
            }
        }
        // Cherche un slot vide
        for (auto& slot : slots) {
            if (slot.count == 0) {
                // TODO: Copier les infos du type
                slot.type = type;
                slot.count = count;
                return true;
            }
        }
        return false;  // Inventaire plein
    }
    
    // Hit test pour savoir si un clic est sur l'inventaire
    int32_t hitTest(simd_float2 mousePos) const {
        if (!isVisible) return -1;
        
        float startX = position.x - (SLOTS_PER_ROW * (slotSize.x + padding)) * 0.5f;
        float startY = position.y;
        
        for (uint32_t row = 0; row < MAX_ROWS; row++) {
            for (uint32_t col = 0; col < SLOTS_PER_ROW; col++) {
                float x = startX + col * (slotSize.x + padding);
                float y = startY - row * (slotSize.y + padding);
                
                if (mousePos.x >= x && mousePos.x <= x + slotSize.x &&
                    mousePos.y >= y - slotSize.y && mousePos.y <= y) {
                    return row * SLOTS_PER_ROW + col;
                }
            }
        }
        return -1;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// DRAG & DROP SYSTEM
// ═══════════════════════════════════════════════════════════════════════════
class DragDropSystem {
public:
    enum class State {
        Idle,
        DraggingFromInventory,
        PlacingBlock,
        DraggingOnVehicle
    };
    
    State state = State::Idle;
    
    // Drag depuis l'inventaire
    int32_t dragSourceSlot = -1;
    InventoryItem* draggedItem = nullptr;
    
    // Placement de bloc
    simd_float3 ghostPosition;          // Position du bloc fantôme
    simd_quatf ghostRotation;           // Rotation du bloc fantôme
    bool canPlace = false;              // Placement valide?
    uint32_t targetBlockID = 0;         // Bloc cible pour l'attachement
    AttachmentPoint::Face targetFace;   // Face du bloc cible
    AttachmentPoint::Face sourceFace = AttachmentPoint::NegY;  // Face du bloc à placer
    
    // Visuels
    simd_float4 ghostColor = {0.3f, 0.8f, 0.3f, 0.5f};  // Vert transparent
    simd_float4 invalidColor = {0.8f, 0.3f, 0.3f, 0.5f}; // Rouge transparent
    
    // ─────────────────────────────────────────────────────────────────────
    // Démarre un drag depuis l'inventaire
    void beginDragFromInventory(Inventory& inventory, int32_t slotIndex) {
        if (slotIndex < 0) return;
        
        InventoryItem* item = &inventory.slots[slotIndex];
        if (item->count == 0) return;
        
        state = State::DraggingFromInventory;
        dragSourceSlot = slotIndex;
        draggedItem = item;
        inventory.selectedSlot = slotIndex;
    }
    
    // Met à jour pendant le drag
    void updateDrag(Vehicle& vehicle,
                    simd_float3 rayOrigin,
                    simd_float3 rayDirection,
                    Inventory& inventory) {
        
        if (state == State::Idle) return;
        
        if (state == State::DraggingFromInventory) {
            // Transition vers placement quand on quitte l'inventaire
            int32_t hitSlot = inventory.hitTest({rayOrigin.x, rayOrigin.y});
            if (hitSlot < 0) {
                state = State::PlacingBlock;
            }
        }
        
        if (state == State::PlacingBlock) {
            // Raycast contre les blocs du véhicule pour trouver un point d'attache
            canPlace = findAttachmentPoint(vehicle, rayOrigin, rayDirection);
            
            if (canPlace) {
                ghostColor = {0.3f, 0.8f, 0.3f, 0.5f};
            } else {
                ghostColor = {0.8f, 0.3f, 0.3f, 0.5f};
            }
        }
    }
    
    // Termine le drag
    bool endDrag(Vehicle& vehicle, Inventory& inventory) {
        if (state == State::Idle) return false;
        
        bool success = false;
        
        if (state == State::PlacingBlock && canPlace && draggedItem) {
            // Créer et attacher le bloc
            auto newBlock = draggedItem->createBlock(vehicle.nextBlockID);
            newBlock->localRotation = ghostRotation;
            
            if (vehicle.attachBlock(std::move(newBlock), targetBlockID,
                                    targetFace, sourceFace)) {
                inventory.consumeItem(dragSourceSlot);
                success = true;
            }
        }
        
        // Reset
        state = State::Idle;
        dragSourceSlot = -1;
        draggedItem = nullptr;
        canPlace = false;
        
        return success;
    }
    
    // Annule le drag
    void cancelDrag() {
        state = State::Idle;
        dragSourceSlot = -1;
        draggedItem = nullptr;
        canPlace = false;
    }
    
    // Tourne le bloc fantôme (R key)
    void rotateGhost() {
        // Cycle à travers les faces source
        sourceFace = static_cast<AttachmentPoint::Face>(
            (static_cast<int>(sourceFace) + 1) % AttachmentPoint::Face::COUNT
        );
    }
    
private:
    // Trouve le meilleur point d'attache
    bool findAttachmentPoint(Vehicle& vehicle,
                             simd_float3 rayOrigin,
                             simd_float3 rayDir) {
        float closestDist = FLT_MAX;
        bool found = false;
        
        // Fonction helper pour tester un bloc
        auto testBlock = [&](VehicleBlock* block) {
            simd_float3 blockWorldPos = block->getWorldPosition(vehicle.position,
                                                                 vehicle.rotation);
            
            // Simple sphere test pour la démo
            float radius = 0.7f;
            simd_float3 oc = rayOrigin - blockWorldPos;
            float b = simd_dot(oc, rayDir);
            float c = simd_dot(oc, oc) - radius * radius;
            float discriminant = b * b - c;
            
            if (discriminant > 0) {
                float t = -b - sqrtf(discriminant);
                if (t > 0 && t < closestDist) {
                    // Trouver la face la plus proche
                    simd_float3 hitPoint = rayOrigin + rayDir * t;
                    simd_float3 localHit = hitPoint - blockWorldPos;
                    
                    // Déterminer la face basée sur la direction
                    AttachmentPoint::Face bestFace = AttachmentPoint::PosY;
                    float maxDot = -FLT_MAX;
                    
                    for (int i = 0; i < 6; i++) {
                        auto& ap = block->attachPoints[i];
                        if (ap.occupied) continue;
                        
                        float d = simd_dot(localHit, ap.normal);
                        if (d > maxDot) {
                            maxDot = d;
                            bestFace = ap.face;
                        }
                    }
                    
                    // Vérifier si cette face est libre
                    if (!block->attachPoints[bestFace].occupied) {
                        closestDist = t;
                        targetBlockID = block->uniqueID;
                        targetFace = bestFace;
                        
                        // Calculer position fantôme
                        simd_float3 attachOffset = block->attachPoints[bestFace].localPosition;
                        ghostPosition = block->localPosition + attachOffset * 2.0f;
                        
                        found = true;
                    }
                }
            }
        };
        
        // Tester le commandant
        testBlock(vehicle.commander.get());
        
        // Tester tous les blocs
        for (auto& [id, block] : vehicle.blocks) {
            testBlock(block.get());
        }
        
        return found;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// WHEEL BLOCK UPDATE (implémentation)
// ═══════════════════════════════════════════════════════════════════════════
inline void WheelBlock::update(float deltaTime, Vehicle& vehicle) {
    if (isDestroyed) return;
    
    // Rotation visuelle basée sur la vitesse
    float speed = simd_length(vehicle.velocity);
    angularVelocity = speed / wheelRadius;
    currentRotation += angularVelocity * deltaTime;
    
    // Steering
    if (isSteering) {
        steerAngle = vehicle.steering * 0.5f;  // Max 0.5 rad (~30°)
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// VEHICLE FACTORY - Pour créer des véhicules préconfigurés
// ═══════════════════════════════════════════════════════════════════════════
class VehicleFactory {
public:
    static std::unique_ptr<Vehicle> createBasicCar(uint32_t vehicleID) {
        auto vehicle = std::make_unique<Vehicle>(vehicleID);
        vehicle->name = "Basic Car";
        
        // 4 roues
        auto createWheel = [](bool powered, bool steering) {
            auto wheel = std::make_unique<WheelBlock>(0);
            wheel->isPowered = powered;
            wheel->isSteering = steering;
            return wheel;
        };
        
        // Roue avant gauche
        vehicle->attachBlock(createWheel(false, true), 0, AttachmentPoint::NegX, AttachmentPoint::PosX);
        // Roue avant droite
        vehicle->attachBlock(createWheel(false, true), 0,
                            AttachmentPoint::PosX, AttachmentPoint::NegX);
        // Roue arrière gauche
        vehicle->attachBlock(createWheel(true, false), 0,
                            AttachmentPoint::NegZ, AttachmentPoint::PosZ);
        // Roue arrière droite
        vehicle->attachBlock(createWheel(true, false), 0,
                            AttachmentPoint::PosZ, AttachmentPoint::NegZ);
        
        return vehicle;
    }
};
}

#endif /* RMDLPetitPrince_hpp */
