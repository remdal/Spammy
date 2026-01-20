//
//  RMDLCmd.hpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

//#ifndef RMDLCmd_hpp
//#define RMDLCmd_hpp
//
//#include <Metal/Metal.hpp>
//#include <simd/simd.h>
//#include <vector>
//#include <unordered_map>
//#include <memory>
//#include <optional>
//#include <string>
//#include <cmath>
//#include <random>
//#include <algorithm>
//
//namespace Terratech {
//
//// ============================================================================
//// TYPES DE BASE
//// ============================================================================
//
//using float2 = simd::float2;
//using float3 = simd::float3;
//using float4 = simd::float4;
//using float4x4 = simd::float4x4;
//using int3 = simd::int3;
//using uint = uint32_t;
//
//// ============================================================================
//// ÉNUMÉRATIONS DES BLOCS
//// ============================================================================
//
//enum class BlockType : uint8_t {
//    None = 0,
//    // Structure
//    Chassis,            // Bloc de base
//    ArmorLight,         // Armure légère
//    ArmorHeavy,         // Armure lourde
//    ArmorSloped,        // Armure inclinée
//    // Mobilité
//    WheelSmall,
//    WheelMedium,
//    WheelLarge,
//    WheelTank,          // Chenilles
//    Hover,              // Hoverpads
//    ThrusterSmall,
//    ThrusterLarge,
//    PropellerAir,
//    WingSmall,
//    WingLarge,
//    // Armes
//    MachineGun,
//    Cannon,
//    Laser,
//    MissileLauncher,
//    PlasmaGun,
//    Flamethrower,
//    // Utilitaires
//    Battery,
//    Generator,
//    SolarPanel,
//    Shield,
//    RepairBubble,
//    Radar,
//    AnchorPoint,        // Point d'ancrage au sol
//    // Ressources
//    ResourceCollector,
//    StorageCrate,
//    Refinery,
//    // Contrôle
//    CabSmall,           // Cabine pilote (obligatoire)
//    CabMedium,
//    CabLarge,
//    AIModule,
//    
//    COUNT
//};
//
//enum class BlockCategory : uint8_t {
//    Structure,
//    Wheels,
//    Flight,
//    Weapons,
//    Power,
//    Utility,
//    Production,
//    Control
//};
//
//enum class Faction : uint8_t {
//    GSO,        // GeoCorp Standard Order
//    Venture,    // Exploration/vitesse
//    Hawkeye,    // Militaire
//    GeoCorp,    // Industrie lourde
//    BetterFuture, // High-tech
//    Reticule,   // Alien
//    Player
//};
//
//// ============================================================================
//// DONNÉES DE BLOCS
//// ============================================================================
//
//struct BlockDefinition {
//    BlockType type;
//    std::string name;
//    BlockCategory category;
//    Faction faction;
//    int3 size;              // Taille en unités de grille
//    float mass;
//    float health;
//    float energyConsumption;
//    float energyGeneration;
//    float4 color;           // Couleur de base
//    
//    // Stats spécifiques
//    float thrust;           // Pour propulseurs
//    float damage;           // Pour armes
//    float fireRate;         // Pour armes
//    float shieldStrength;   // Pour boucliers
//    float storageCapacity;  // Pour stockage
//};
//
//static const std::unordered_map<BlockType, BlockDefinition> BLOCK_DEFINITIONS = {
//    {BlockType::Chassis, {"Chassis", "Chassis Block", BlockCategory::Structure, Faction::GSO, {1,1,1}, 10.f, 100.f, 0.f, 0.f, {0.6f, 0.6f, 0.65f, 1.f}, 0,0,0,0,0}},
//    {BlockType::ArmorLight, {"ArmorLight", "Light Armor", BlockCategory::Structure, Faction::GSO, {1,1,1}, 15.f, 200.f, 0.f, 0.f, {0.5f, 0.55f, 0.6f, 1.f}, 0,0,0,0,0}},
//    {BlockType::ArmorHeavy, {"ArmorHeavy", "Heavy Armor", BlockCategory::Structure, Faction::Hawkeye, {1,1,1}, 40.f, 500.f, 0.f, 0.f, {0.3f, 0.35f, 0.4f, 1.f}, 0,0,0,0,0}},
//    {BlockType::ArmorSloped, {"ArmorSloped", "Sloped Armor", BlockCategory::Structure, Faction::GSO, {1,1,1}, 12.f, 180.f, 0.f, 0.f, {0.55f, 0.58f, 0.62f, 1.f}, 0,0,0,0,0}},
//    
//    {BlockType::WheelSmall, {"WheelSmall", "Small Wheel", BlockCategory::Wheels, Faction::GSO, {1,1,1}, 8.f, 50.f, 1.f, 0.f, {0.2f, 0.2f, 0.2f, 1.f}, 100.f,0,0,0,0}},
//    {BlockType::WheelMedium, {"WheelMedium", "Medium Wheel", BlockCategory::Wheels, Faction::GSO, {1,2,1}, 15.f, 80.f, 2.f, 0.f, {0.15f, 0.15f, 0.15f, 1.f}, 200.f,0,0,0,0}},
//    {BlockType::WheelLarge, {"WheelLarge", "Large Wheel", BlockCategory::Wheels, Faction::GeoCorp, {2,3,2}, 35.f, 150.f, 4.f, 0.f, {0.1f, 0.1f, 0.1f, 1.f}, 400.f,0,0,0,0}},
//    {BlockType::WheelTank, {"WheelTank", "Tank Tracks", BlockCategory::Wheels, Faction::Hawkeye, {1,2,3}, 50.f, 200.f, 5.f, 0.f, {0.25f, 0.22f, 0.18f, 1.f}, 350.f,0,0,0,0}},
//    {BlockType::Hover, {"Hover", "Hover Pad", BlockCategory::Flight, Faction::Venture, {1,1,1}, 12.f, 60.f, 8.f, 0.f, {0.1f, 0.6f, 0.8f, 1.f}, 150.f,0,0,0,0}},
//    
//    {BlockType::ThrusterSmall, {"ThrusterSmall", "Small Thruster", BlockCategory::Flight, Faction::Venture, {1,1,2}, 10.f, 40.f, 10.f, 0.f, {0.9f, 0.5f, 0.1f, 1.f}, 500.f,0,0,0,0}},
//    {BlockType::ThrusterLarge, {"ThrusterLarge", "Large Thruster", BlockCategory::Flight, Faction::Venture, {2,2,3}, 30.f, 100.f, 25.f, 0.f, {0.95f, 0.4f, 0.05f, 1.f}, 1500.f,0,0,0,0}},
//    {BlockType::PropellerAir, {"PropellerAir", "Air Propeller", BlockCategory::Flight, Faction::Venture, {1,1,2}, 8.f, 30.f, 6.f, 0.f, {0.7f, 0.7f, 0.75f, 1.f}, 300.f,0,0,0,0}},
//    {BlockType::WingSmall, {"WingSmall", "Small Wing", BlockCategory::Flight, Faction::Venture, {3,1,2}, 5.f, 40.f, 0.f, 0.f, {0.8f, 0.8f, 0.85f, 1.f}, 0,0,0,0,0}},
//    {BlockType::WingLarge, {"WingLarge", "Large Wing", BlockCategory::Flight, Faction::Hawkeye, {5,1,3}, 12.f, 80.f, 0.f, 0.f, {0.4f, 0.45f, 0.5f, 1.f}, 0,0,0,0,0}},
//    
//    {BlockType::MachineGun, {"MachineGun", "Machine Gun", BlockCategory::Weapons, Faction::GSO, {1,1,2}, 12.f, 60.f, 3.f, 0.f, {0.35f, 0.38f, 0.4f, 1.f}, 0, 15.f, 10.f, 0,0}},
//    {BlockType::Cannon, {"Cannon", "Battle Cannon", BlockCategory::Weapons, Faction::Hawkeye, {2,2,3}, 45.f, 120.f, 8.f, 0.f, {0.3f, 0.32f, 0.28f, 1.f}, 0, 80.f, 1.f, 0,0}},
//    {BlockType::Laser, {"Laser", "Laser Beam", BlockCategory::Weapons, Faction::BetterFuture, {1,1,2}, 15.f, 50.f, 15.f, 0.f, {0.2f, 0.8f, 0.9f, 1.f}, 0, 25.f, 0.f, 0,0}},
//    {BlockType::MissileLauncher, {"MissileLauncher", "Missile Pod", BlockCategory::Weapons, Faction::Hawkeye, {2,2,2}, 35.f, 80.f, 12.f, 0.f, {0.5f, 0.3f, 0.25f, 1.f}, 0, 120.f, 0.5f, 0,0}},
//    {BlockType::PlasmaGun, {"PlasmaGun", "Plasma Cutter", BlockCategory::Weapons, Faction::GeoCorp, {1,1,3}, 20.f, 70.f, 20.f, 0.f, {0.9f, 0.2f, 0.8f, 1.f}, 0, 40.f, 2.f, 0,0}},
//    {BlockType::Flamethrower, {"Flamethrower", "Flamethrower", BlockCategory::Weapons, Faction::GSO, {1,1,2}, 18.f, 55.f, 8.f, 0.f, {0.95f, 0.3f, 0.1f, 1.f}, 0, 10.f, 20.f, 0,0}},
//    
//    {BlockType::Battery, {"Battery", "Battery Pack", BlockCategory::Power, Faction::GSO, {1,1,1}, 20.f, 40.f, 0.f, 0.f, {0.2f, 0.5f, 0.2f, 1.f}, 0,0,0,0, 500.f}},
//    {BlockType::Generator, {"Generator", "Fuel Generator", BlockCategory::Power, Faction::GSO, {2,1,2}, 40.f, 80.f, 0.f, 30.f, {0.6f, 0.55f, 0.3f, 1.f}, 0,0,0,0,0}},
//    {BlockType::SolarPanel, {"SolarPanel", "Solar Panel", BlockCategory::Power, Faction::BetterFuture, {2,1,1}, 8.f, 20.f, 0.f, 10.f, {0.1f, 0.15f, 0.4f, 1.f}, 0,0,0,0,0}},
//    {BlockType::Shield, {"Shield", "Bubble Shield", BlockCategory::Utility, Faction::BetterFuture, {2,2,2}, 25.f, 60.f, 25.f, 0.f, {0.3f, 0.7f, 0.9f, 0.5f}, 0,0,0, 300.f, 0}},
//    {BlockType::RepairBubble, {"RepairBubble", "Repair Bubble", BlockCategory::Utility, Faction::GSO, {1,1,1}, 15.f, 50.f, 10.f, 0.f, {0.2f, 0.9f, 0.3f, 1.f}, 0,0,0,0,0}},
//    {BlockType::Radar, {"Radar", "Radar Dish", BlockCategory::Utility, Faction::Venture, {1,2,1}, 10.f, 30.f, 5.f, 0.f, {0.7f, 0.7f, 0.75f, 1.f}, 0,0,0,0,0}},
//    
//    {BlockType::ResourceCollector, {"ResourceCollector", "Resource Collector", BlockCategory::Production, Faction::GSO, {2,1,2}, 30.f, 70.f, 5.f, 0.f, {0.8f, 0.6f, 0.2f, 1.f}, 0,0,0,0, 100.f}},
//    {BlockType::StorageCrate, {"StorageCrate", "Storage Crate", BlockCategory::Production, Faction::GSO, {2,2,2}, 25.f, 100.f, 0.f, 0.f, {0.7f, 0.5f, 0.3f, 1.f}, 0,0,0,0, 200.f}},
//    {BlockType::Refinery, {"Refinery", "Refinery", BlockCategory::Production, Faction::GeoCorp, {3,2,3}, 80.f, 150.f, 15.f, 0.f, {0.5f, 0.45f, 0.35f, 1.f}, 0,0,0,0,0}},
//    
//    {BlockType::CabSmall, {"CabSmall", "Small Cab", BlockCategory::Control, Faction::GSO, {1,1,1}, 20.f, 150.f, 0.f, 5.f, {0.4f, 0.6f, 0.8f, 1.f}, 0,0,0,0,0}},
//    {BlockType::CabMedium, {"CabMedium", "Medium Cab", BlockCategory::Control, Faction::Venture, {2,2,2}, 40.f, 300.f, 0.f, 10.f, {0.5f, 0.7f, 0.3f, 1.f}, 0,0,0,0,0}},
//    {BlockType::CabLarge, {"CabLarge", "Command Cab", BlockCategory::Control, Faction::Hawkeye, {3,2,3}, 80.f, 500.f, 0.f, 15.f, {0.6f, 0.3f, 0.3f, 1.f}, 0,0,0,0,0}},
//    {BlockType::AIModule, {"AIModule", "AI Control Module", BlockCategory::Control, Faction::BetterFuture, {1,1,1}, 15.f, 100.f, 8.f, 0.f, {0.8f, 0.2f, 0.9f, 1.f}, 0,0,0,0,0}},
//};
//
//// ============================================================================
//// STRUCTURE D'UN BLOC PLACÉ
//// ============================================================================
//
//struct PlacedBlock {
//    BlockType type = BlockType::None;
//    int3 gridPosition;      // Position dans la grille du véhicule
//    uint8_t rotation;       // 0-23 pour les 24 rotations possibles
//    float currentHealth;
//    bool isActive = true;
//    uint32_t blockId;       // ID unique
//    
//    // État dynamique
//    float weaponCooldown = 0.f;
//    float thrusterOutput = 0.f;
//    float wheelRotation = 0.f;
//};
//
//// ============================================================================
//// STRUCTURE D'UN TECH (VÉHICULE)
//// ============================================================================
//
//struct Tech {
//    uint32_t techId;
//    std::string name;
//    Faction faction;
//    bool isPlayer;
//    bool isAI;
//    
//    // Blocs
//    std::vector<PlacedBlock> blocks;
//    std::unordered_map<uint32_t, size_t> blockIdToIndex;
//    
//    // Grille d'occupation (64x64x64)
//    static constexpr int GRID_SIZE = 64;
//    std::array<uint32_t, GRID_SIZE * GRID_SIZE * GRID_SIZE> occupancyGrid;
//    
//    // Transform
//    float3 position;
//    simd::quatf rotation;
//    float3 velocity;
//    float3 angularVelocity;
//    
//    // Stats calculées
//    float totalMass = 0.f;
//    float3 centerOfMass;
//    float maxEnergy = 0.f;
//    float currentEnergy = 0.f;
//    float energyGeneration = 0.f;
//    float energyConsumption = 0.f;
//    float totalThrust = 0.f;
//    float maxSpeed = 0.f;
//    float totalHealth = 0.f;
//    float currentHealth = 0.f;
//    float shieldStrength = 0.f;
//    
//    // État
//    bool isGrounded = false;
//    bool isFlying = false;
//    bool shieldActive = false;
//    float3 inputDirection;
//    bool isFiring = false;
//    
//    // Cabine (obligatoire)
//    std::optional<uint32_t> cabBlockId;
//    
//    // Méthodes
//    bool hasCab() const { return cabBlockId.has_value(); }
//    bool isValid() const { return hasCab() && !blocks.empty(); }
//    
//    int gridIndex(int x, int y, int z) const {
//        return x + y * GRID_SIZE + z * GRID_SIZE * GRID_SIZE;
//    }
//    
//    bool isOccupied(int3 pos) const {
//        if (pos.x < 0 || pos.x >= GRID_SIZE ||
//            pos.y < 0 || pos.y >= GRID_SIZE ||
//            pos.z < 0 || pos.z >= GRID_SIZE) return true;
//        return occupancyGrid[gridIndex(pos.x, pos.y, pos.z)] != 0;
//    }
//    
//    void recalculateStats() {
//        totalMass = 0.f;
//        float3 massSum = {0, 0, 0};
//        maxEnergy = 100.f; // Base
//        energyGeneration = 0.f;
//        energyConsumption = 0.f;
//        totalThrust = 0.f;
//        totalHealth = 0.f;
//        currentHealth = 0.f;
//        shieldStrength = 0.f;
//        
//        for (const auto& block : blocks) {
//            if (block.type == BlockType::None) continue;
//            auto it = BLOCK_DEFINITIONS.find(block.type);
//            if (it == BLOCK_DEFINITIONS.end()) continue;
//            
//            const auto& def = it->second;
//            totalMass += def.mass;
//            float3 blockPos = float3{(float)block.gridPosition.x,
//                                     (float)block.gridPosition.y,
//                                     (float)block.gridPosition.z};
//            massSum += blockPos * def.mass;
//            
//            energyGeneration += def.energyGeneration;
//            energyConsumption += def.energyConsumption;
//            totalThrust += def.thrust;
//            totalHealth += def.health;
//            currentHealth += block.currentHealth;
//            shieldStrength += def.shieldStrength;
//            
//            if (def.category == BlockCategory::Power && def.storageCapacity > 0) {
//                maxEnergy += def.storageCapacity;
//            }
//        }
//        
//        if (totalMass > 0.f) {
//            centerOfMass = massSum / totalMass;
//        }
//        maxSpeed = (totalMass > 0.f) ? (totalThrust / totalMass) * 10.f : 0.f;
//    }
//};
//
//// ============================================================================
//// RESSOURCES DU MONDE
//// ============================================================================
//
//enum class ResourceType : uint8_t {
//    Celestite,      // Bleu - commun
//    Erudite,        // Vert - commun
//    Ignite,         // Rouge - peu commun
//    Carbite,        // Gris - peu commun
//    Luxite,         // Jaune - rare
//    Plumbite,       // Violet - rare
//    Rodite,         // Orange - très rare
//    Fibrite,        // Rose - épique
//    Oleite,         // Noir - épique
//    COUNT
//};
//
//struct ResourceChunk {
//    ResourceType type;
//    float3 position;
//    float amount;
//    float maxAmount;
//    bool isCollected = false;
//};
//
//// ============================================================================
//// PROJECTILES
//// ============================================================================
//
//struct Projectile {
//    uint32_t id;
//    uint32_t ownerTechId;
//    BlockType weaponType;
//    float3 position;
//    float3 velocity;
//    float damage;
//    float lifetime;
//    float maxLifetime;
//    float radius;
//    bool isHoming = false;
//    uint32_t targetTechId = 0;
//};
//
//// ============================================================================
//// DONNÉES GPU
//// ============================================================================
//
//struct Vertex {
//    float3 position;
//    float3 normal;
//    float2 texCoord;
//    float4 color;
//};
//
//struct InstanceData {
//    float4x4 modelMatrix;
//    float4 color;
//    float4 emissive;
//    float metallic;
//    float roughness;
//    float health;       // Pour effets de dégâts
//    uint blockType;
//};
//
//struct CameraUniforms {
//    float4x4 viewMatrix;
//    float4x4 projectionMatrix;
//    float4x4 viewProjectionMatrix;
//    float3 cameraPosition;
//    float time;
//};
//
//struct LightData {
//    float3 direction;
//    float intensity;
//    float3 color;
//    float ambientIntensity;
//};
//
//struct TerrainChunk {
//    float3 position;
//    std::vector<Vertex> vertices;
//    std::vector<uint32_t> indices;
//    MTL::Buffer* vertexBuffer = nullptr;
//    MTL::Buffer* indexBuffer = nullptr;
//    bool isDirty = true;
//};
//
//// ============================================================================
//// CLASSE PRINCIPALE DU RENDERER
//// ============================================================================
//
//class TerratechRenderer {
//public:
//    // ========================================================================
//    // INITIALISATION
//    // ========================================================================
//    
//    TerratechRenderer(MTL::Device* device, MTL::CommandQueue* queue, float2 viewportSize)
//        : m_device(device)
//        , m_commandQueue(queue)
//        , m_viewportSize(viewportSize)
//    {
//        initializePipelines();
//        initializeBuffers();
//        initializeBlockMeshes();
//        initializeTerrain();
//        initializeDefaultTech();
//    }
//    
//    ~TerratechRenderer() {
//        cleanup();
//    }
//    
//    // ========================================================================
//    // BOUCLE PRINCIPALE
//    // ========================================================================
//    
//    void update(float deltaTime) {
//        m_time += deltaTime;
//        
//        updateInput();
//        updatePhysics(deltaTime);
//        updateTechs(deltaTime);
//        updateProjectiles(deltaTime);
//        updateResources(deltaTime);
//        updateCamera(deltaTime);
//    }
//    
//    void render(MTL::RenderCommandEncoder* encoder) {
//        updateUniforms();
//        
//        // 1. Rendu du terrain
//        renderTerrain(encoder);
//        
//        // 2. Rendu des ressources
//        renderResources(encoder);
//        
//        // 3. Rendu des techs (véhicules)
//        renderTechs(encoder);
//        
//        // 4. Rendu des projectiles
//        renderProjectiles(encoder);
//        
//        // 5. Rendu des effets (boucliers, particles, etc.)
//        renderEffects(encoder);
//        
//        // 6. UI en mode construction
//        if (m_buildMode) {
//            renderBuildUI(encoder);
//        }
//    }
//    
//    // ========================================================================
//    // GESTION DES TECHS
//    // ========================================================================
//    
//    uint32_t createTech(const std::string& name, Faction faction, float3 position, bool isPlayer = false) {
//        Tech tech;
//        tech.techId = m_nextTechId++;
//        tech.name = name;
//        tech.faction = faction;
//        tech.position = position;
//        tech.rotation = simd::quatf{0, 0, 0, 1};
//        tech.velocity = {0, 0, 0};
//        tech.angularVelocity = {0, 0, 0};
//        tech.isPlayer = isPlayer;
//        tech.isAI = !isPlayer;
//        std::fill(tech.occupancyGrid.begin(), tech.occupancyGrid.end(), 0);
//        
//        m_techs.push_back(std::move(tech));
//        
//        if (isPlayer) {
//            m_playerTechId = tech.techId;
//        }
//        
//        return tech.techId;
//    }
//    
//    bool addBlockToTech(uint32_t techId, BlockType type, int3 gridPos, uint8_t rotation = 0) {
//        Tech* tech = getTech(techId);
//        if (!tech) return false;
//        
//        auto it = BLOCK_DEFINITIONS.find(type);
//        if (it == BLOCK_DEFINITIONS.end()) return false;
//        
//        const auto& def = it->second;
//        
//        // Vérifier si l'espace est libre
//        for (int x = 0; x < def.size.x; ++x) {
//            for (int y = 0; y < def.size.y; ++y) {
//                for (int z = 0; z < def.size.z; ++z) {
//                    int3 checkPos = {gridPos.x + x, gridPos.y + y, gridPos.z + z};
//                    if (tech->isOccupied(checkPos)) {
//                        return false;
//                    }
//                }
//            }
//        }
//        
//        // Vérifier la connectivité (sauf premier bloc)
//        if (!tech->blocks.empty() && !hasAdjacentBlock(*tech, gridPos, def.size)) {
//            return false;
//        }
//        
//        // Créer le bloc
//        PlacedBlock block;
//        block.type = type;
//        block.gridPosition = gridPos;
//        block.rotation = rotation;
//        block.currentHealth = def.health;
//        block.blockId = m_nextBlockId++;
//        
//        // Marquer l'occupation
//        for (int x = 0; x < def.size.x; ++x) {
//            for (int y = 0; y < def.size.y; ++y) {
//                for (int z = 0; z < def.size.z; ++z) {
//                    int3 occPos = {gridPos.x + x, gridPos.y + y, gridPos.z + z};
//                    tech->occupancyGrid[tech->gridIndex(occPos.x, occPos.y, occPos.z)] = block.blockId;
//                }
//            }
//        }
//        
//        tech->blockIdToIndex[block.blockId] = tech->blocks.size();
//        tech->blocks.push_back(block);
//        
//        // Si c'est une cabine, l'enregistrer
//        if (def.category == BlockCategory::Control &&
//            (type == BlockType::CabSmall || type == BlockType::CabMedium || type == BlockType::CabLarge)) {
//            if (!tech->cabBlockId.has_value()) {
//                tech->cabBlockId = block.blockId;
//            }
//        }
//        
//        tech->recalculateStats();
//        m_instancesDirty = true;
//        
//        return true;
//    }
//    
//    bool removeBlockFromTech(uint32_t techId, uint32_t blockId) {
//        Tech* tech = getTech(techId);
//        if (!tech) return false;
//        
//        auto indexIt = tech->blockIdToIndex.find(blockId);
//        if (indexIt == tech->blockIdToIndex.end()) return false;
//        
//        size_t index = indexIt->second;
//        const PlacedBlock& block = tech->blocks[index];
//        
//        // Ne pas supprimer la cabine si c'est la seule
//        if (tech->cabBlockId == blockId && tech->blocks.size() > 1) {
//            // Vérifier s'il y a une autre cabine
//            bool hasOtherCab = false;
//            for (const auto& b : tech->blocks) {
//                if (b.blockId != blockId) {
//                    auto it = BLOCK_DEFINITIONS.find(b.type);
//                    if (it != BLOCK_DEFINITIONS.end() && it->second.category == BlockCategory::Control) {
//                        hasOtherCab = true;
//                        break;
//                    }
//                }
//            }
//            if (!hasOtherCab) return false;
//        }
//        
//        auto defIt = BLOCK_DEFINITIONS.find(block.type);
//        if (defIt != BLOCK_DEFINITIONS.end()) {
//            const auto& def = defIt->second;
//            // Libérer l'occupation
//            for (int x = 0; x < def.size.x; ++x) {
//                for (int y = 0; y < def.size.y; ++y) {
//                    for (int z = 0; z < def.size.z; ++z) {
//                        int3 occPos = {block.gridPosition.x + x,
//                                       block.gridPosition.y + y,
//                                       block.gridPosition.z + z};
//                        tech->occupancyGrid[tech->gridIndex(occPos.x, occPos.y, occPos.z)] = 0;
//                    }
//                }
//            }
//        }
//        
//        // Supprimer et mettre à jour les index
//        tech->blocks.erase(tech->blocks.begin() + index);
//        tech->blockIdToIndex.erase(blockId);
//        
//        // Reconstruire la map d'index
//        for (size_t i = 0; i < tech->blocks.size(); ++i) {
//            tech->blockIdToIndex[tech->blocks[i].blockId] = i;
//        }
//        
//        if (tech->cabBlockId == blockId) {
//            tech->cabBlockId.reset();
//            // Chercher une nouvelle cabine
//            for (const auto& b : tech->blocks) {
//                auto it = BLOCK_DEFINITIONS.find(b.type);
//                if (it != BLOCK_DEFINITIONS.end() && it->second.category == BlockCategory::Control) {
//                    tech->cabBlockId = b.blockId;
//                    break;
//                }
//            }
//        }
//        
//        tech->recalculateStats();
//        m_instancesDirty = true;
//        
//        return true;
//    }
//    
//    void damageBlock(uint32_t techId, uint32_t blockId, float damage) {
//        Tech* tech = getTech(techId);
//        if (!tech) return;
//        
//        auto indexIt = tech->blockIdToIndex.find(blockId);
//        if (indexIt == tech->blockIdToIndex.end()) return;
//        
//        PlacedBlock& block = tech->blocks[indexIt->second];
//        block.currentHealth -= damage;
//        
//        if (block.currentHealth <= 0.f) {
//            // Bloc détruit
//            spawnBlockDebris(tech->position + gridToWorld(block.gridPosition), block.type);
//            removeBlockFromTech(techId, blockId);
//            
//            // Vérifier si le tech est détruit
//            if (!tech->isValid()) {
//                destroyTech(techId);
//            }
//        }
//        
//        tech->recalculateStats();
//    }
//    
//    // ========================================================================
//    // SYSTÈME DE COMBAT
//    // ========================================================================
//    
//    void fireWeapon(uint32_t techId, uint32_t blockId) {
//        Tech* tech = getTech(techId);
//        if (!tech) return;
//        
//        auto indexIt = tech->blockIdToIndex.find(blockId);
//        if (indexIt == tech->blockIdToIndex.end()) return;
//        
//        PlacedBlock& block = tech->blocks[indexIt->second];
//        auto defIt = BLOCK_DEFINITIONS.find(block.type);
//        if (defIt == BLOCK_DEFINITIONS.end()) return;
//        
//        const auto& def = defIt->second;
//        if (def.category != BlockCategory::Weapons) return;
//        if (block.weaponCooldown > 0.f) return;
//        if (tech->currentEnergy < def.energyConsumption) return;
//        
//        // Créer le projectile
//        Projectile proj;
//        proj.id = m_nextProjectileId++;
//        proj.ownerTechId = techId;
//        proj.weaponType = block.type;
//        proj.position = tech->position + gridToWorld(block.gridPosition);
//        
//        // Direction basée sur la rotation du bloc et du tech
//        float3 forward = getBlockForward(block.rotation, tech->rotation);
//        proj.velocity = forward * getProjectileSpeed(block.type);
//        proj.damage = def.damage;
//        proj.lifetime = 0.f;
//        proj.maxLifetime = getProjectileLifetime(block.type);
//        proj.radius = getProjectileRadius(block.type);
//        proj.isHoming = (block.type == BlockType::MissileLauncher);
//        
//        m_projectiles.push_back(proj);
//        
//        // Cooldown et énergie
//        block.weaponCooldown = 1.f / std::max(def.fireRate, 0.1f);
//        tech->currentEnergy -= def.energyConsumption;
//    }
//    
//    // ========================================================================
//    // MODE CONSTRUCTION
//    // ========================================================================
//    
//    void enterBuildMode() {
//        m_buildMode = true;
//        m_selectedBlockType = BlockType::Chassis;
//        m_buildRotation = 0;
//    }
//    
//    void exitBuildMode() {
//        m_buildMode = false;
//    }
//    
//    void selectBlockType(BlockType type) {
//        m_selectedBlockType = type;
//    }
//    
//    void rotateBuildBlock() {
//        m_buildRotation = (m_buildRotation + 1) % 24;
//    }
//    
//    void placeBuildBlock() {
//        if (!m_buildMode) return;
//        
//        Tech* playerTech = getTech(m_playerTechId);
//        if (!playerTech) return;
//        
//        addBlockToTech(m_playerTechId, m_selectedBlockType, m_buildCursor, m_buildRotation);
//    }
//    
//    void removeBuildBlock() {
//        if (!m_buildMode) return;
//        
//        Tech* playerTech = getTech(m_playerTechId);
//        if (!playerTech) return;
//        
//        uint32_t blockId = playerTech->occupancyGrid[playerTech->gridIndex(
//            m_buildCursor.x, m_buildCursor.y, m_buildCursor.z)];
//        
//        if (blockId != 0) {
//            removeBlockFromTech(m_playerTechId, blockId);
//        }
//    }
//    
//    void moveBuildCursor(int3 delta) {
//        m_buildCursor.x = std::clamp(m_buildCursor.x + delta.x, 0, Tech::GRID_SIZE - 1);
//        m_buildCursor.y = std::clamp(m_buildCursor.y + delta.y, 0, Tech::GRID_SIZE - 1);
//        m_buildCursor.z = std::clamp(m_buildCursor.z + delta.z, 0, Tech::GRID_SIZE - 1);
//    }
//    
//    // ========================================================================
//    // INPUT
//    // ========================================================================
//    
//    void setInput(float3 moveDir, bool fire, bool jump, bool boost) {
//        m_inputMove = moveDir;
//        m_inputFire = fire;
//        m_inputJump = jump;
//        m_inputBoost = boost;
//    }
//    
//    void setMousePosition(float2 pos) {
//        m_mousePosition = pos;
//    }
//    
//private:
//    // ========================================================================
//    // MEMBRES
//    // ========================================================================
//    
//    MTL::Device* m_device;
//    MTL::CommandQueue* m_commandQueue;
//    float2 m_viewportSize;
//    float m_time = 0.f;
//    
//    // Pipelines
//    MTL::RenderPipelineState* m_blockPipeline = nullptr;
//    MTL::RenderPipelineState* m_terrainPipeline = nullptr;
//    MTL::RenderPipelineState* m_projectilePipeline = nullptr;
//    MTL::RenderPipelineState* m_effectPipeline = nullptr;
//    MTL::RenderPipelineState* m_uiPipeline = nullptr;
//    MTL::DepthStencilState* m_depthState = nullptr;
//    
//    // Buffers
//    MTL::Buffer* m_cameraBuffer = nullptr;
//    MTL::Buffer* m_lightBuffer = nullptr;
//    MTL::Buffer* m_instanceBuffer = nullptr;
//    size_t m_maxInstances = 10000;
//    bool m_instancesDirty = true;
//    
//    // Meshes des blocs
//    struct BlockMesh {
//        MTL::Buffer* vertexBuffer = nullptr;
//        MTL::Buffer* indexBuffer = nullptr;
//        uint32_t indexCount = 0;
//    };
//    std::unordered_map<BlockType, BlockMesh> m_blockMeshes;
//    
//    // Données de jeu
//    std::vector<Tech> m_techs;
//    std::vector<Projectile> m_projectiles;
//    std::vector<ResourceChunk> m_resources;
//    std::vector<TerrainChunk> m_terrainChunks;
//    
//    uint32_t m_nextTechId = 1;
//    uint32_t m_nextBlockId = 1;
//    uint32_t m_nextProjectileId = 1;
//    uint32_t m_playerTechId = 0;
//    
//    // Mode construction
//    bool m_buildMode = false;
//    BlockType m_selectedBlockType = BlockType::Chassis;
//    uint8_t m_buildRotation = 0;
//    int3 m_buildCursor = {32, 32, 32}; // Centre de la grille
//    
//    // Input
//    float3 m_inputMove = {0, 0, 0};
//    bool m_inputFire = false;
//    bool m_inputJump = false;
//    bool m_inputBoost = false;
//    float2 m_mousePosition = {0, 0};
//    
//    // Caméra
//    float3 m_cameraPosition = {0, 50, -100};
//    float3 m_cameraTarget = {0, 0, 0};
//    float m_cameraDistance = 50.f;
//    float m_cameraPitch = 0.3f;
//    float m_cameraYaw = 0.f;
//    
//    // Constantes physiques
//    static constexpr float GRAVITY = 9.81f;
//    static constexpr float BLOCK_SIZE = 1.0f;
//    static constexpr float GROUND_LEVEL = 0.f;
//    
//    // ========================================================================
//    // INITIALISATION PRIVÉE
//    // ========================================================================
//    
//    void initializePipelines() {
//        // Shaders inline pour simplifier
//        const char* shaderSource = R"(
//            #include <metal_stdlib>
//            using namespace metal;
//            
//            struct Vertex {
//                float3 position [[attribute(0)]];
//                float3 normal [[attribute(1)]];
//                float2 texCoord [[attribute(2)]];
//                float4 color [[attribute(3)]];
//            };
//            
//            struct InstanceData {
//                float4x4 modelMatrix;
//                float4 color;
//                float4 emissive;
//                float metallic;
//                float roughness;
//                float health;
//                uint blockType;
//            };
//            
//            struct CameraUniforms {
//                float4x4 viewMatrix;
//                float4x4 projectionMatrix;
//                float4x4 viewProjectionMatrix;
//                float3 cameraPosition;
//                float time;
//            };
//            
//            struct LightData {
//                float3 direction;
//                float intensity;
//                float3 color;
//                float ambientIntensity;
//            };
//            
//            struct VertexOut {
//                float4 position [[position]];
//                float3 worldPosition;
//                float3 normal;
//                float2 texCoord;
//                float4 color;
//                float4 emissive;
//                float metallic;
//                float roughness;
//                float health;
//            };
//            
//            vertex VertexOut blockVertex(
//                Vertex in [[stage_in]],
//                constant CameraUniforms& camera [[buffer(1)]],
//                constant InstanceData* instances [[buffer(2)]],
//                uint instanceId [[instance_id]]
//            ) {
//                InstanceData inst = instances[instanceId];
//                float4 worldPos = inst.modelMatrix * float4(in.position, 1.0);
//                
//                VertexOut out;
//                out.position = camera.viewProjectionMatrix * worldPos;
//                out.worldPosition = worldPos.xyz;
//                out.normal = normalize((inst.modelMatrix * float4(in.normal, 0.0)).xyz);
//                out.texCoord = in.texCoord;
//                out.color = in.color * inst.color;
//                out.emissive = inst.emissive;
//                out.metallic = inst.metallic;
//                out.roughness = inst.roughness;
//                out.health = inst.health;
//                return out;
//            }
//            
//            fragment float4 blockFragment(
//                VertexOut in [[stage_in]],
//                constant LightData& light [[buffer(0)]],
//                constant CameraUniforms& camera [[buffer(1)]]
//            ) {
//                // PBR simplifié
//                float3 N = normalize(in.normal);
//                float3 L = normalize(-light.direction);
//                float3 V = normalize(camera.cameraPosition - in.worldPosition);
//                float3 H = normalize(L + V);
//                
//                float NdotL = max(dot(N, L), 0.0);
//                float NdotH = max(dot(N, H), 0.0);
//                float NdotV = max(dot(N, V), 0.0);
//                
//                // Fresnel
//                float3 F0 = mix(float3(0.04), in.color.rgb, in.metallic);
//                float3 F = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
//                
//                // Distribution GGX
//                float a = in.roughness * in.roughness;
//                float a2 = a * a;
//                float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
//                float D = a2 / (3.14159 * denom * denom);
//                
//                // Diffuse
//                float3 diffuse = in.color.rgb * (1.0 - in.metallic);
//                
//                // Specular
//                float3 specular = F * D * 0.25;
//                
//                // Combinaison
//                float3 ambient = in.color.rgb * light.ambientIntensity;
//                float3 direct = (diffuse + specular) * light.color * light.intensity * NdotL;
//                
//                float3 finalColor = ambient + direct + in.emissive.rgb;
//                
//                // Effet de dégâts
//                float damageEffect = 1.0 - in.health;
//                finalColor = mix(finalColor, float3(0.1, 0.05, 0.0), damageEffect * 0.5);
//                
//                return float4(finalColor, in.color.a);
//            }
//            
//            // Terrain vertex/fragment
//            vertex VertexOut terrainVertex(
//                Vertex in [[stage_in]],
//                constant CameraUniforms& camera [[buffer(1)]]
//            ) {
//                VertexOut out;
//                out.position = camera.viewProjectionMatrix * float4(in.position, 1.0);
//                out.worldPosition = in.position;
//                out.normal = in.normal;
//                out.texCoord = in.texCoord;
//                out.color = in.color;
//                out.emissive = float4(0);
//                out.metallic = 0.0;
//                out.roughness = 0.8;
//                out.health = 1.0;
//                return out;
//            }
//        )";
//        
//        NS::Error* error = nullptr;
//        MTL::Library* library = m_device->newLibrary(
//            NS::String::string(shaderSource, NS::UTF8StringEncoding),
//            nullptr,
//            &error
//        );
//        
//        if (!library) {
//            // Fallback: créer une bibliothèque vide
//            return;
//        }
//        
//        // Créer le vertex descriptor
//        MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();
//        vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//        vertexDesc->attributes()->object(0)->setOffset(0);
//        vertexDesc->attributes()->object(0)->setBufferIndex(0);
//        
//        vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//        vertexDesc->attributes()->object(1)->setOffset(sizeof(float3));
//        vertexDesc->attributes()->object(1)->setBufferIndex(0);
//        
//        vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
//        vertexDesc->attributes()->object(2)->setOffset(sizeof(float3) * 2);
//        vertexDesc->attributes()->object(2)->setBufferIndex(0);
//        
//        vertexDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat4);
//        vertexDesc->attributes()->object(3)->setOffset(sizeof(float3) * 2 + sizeof(float2));
//        vertexDesc->attributes()->object(3)->setBufferIndex(0);
//        
//        vertexDesc->layouts()->object(0)->setStride(sizeof(Vertex));
//        
//        // Pipeline blocks
//        MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//        pipelineDesc->setVertexFunction(library->newFunction(NS::String::string("blockVertex", NS::UTF8StringEncoding)));
//        pipelineDesc->setFragmentFunction(library->newFunction(NS::String::string("blockFragment", NS::UTF8StringEncoding)));
//        pipelineDesc->setVertexDescriptor(vertexDesc);
//        pipelineDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
//        pipelineDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
//        
//        m_blockPipeline = m_device->newRenderPipelineState(pipelineDesc, &error);
//        
//        // Pipeline terrain
//        pipelineDesc->setVertexFunction(library->newFunction(NS::String::string("terrainVertex", NS::UTF8StringEncoding)));
//        m_terrainPipeline = m_device->newRenderPipelineState(pipelineDesc, &error);
//        
//        // Depth state
//        MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
//        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
//        depthDesc->setDepthWriteEnabled(true);
//        m_depthState = m_device->newDepthStencilState(depthDesc);
//        
//        library->release();
//        pipelineDesc->release();
//        vertexDesc->release();
//        depthDesc->release();
//    }
//    
//    void initializeBuffers() {
//        m_cameraBuffer = m_device->newBuffer(sizeof(CameraUniforms), MTL::ResourceStorageModeShared);
//        m_lightBuffer = m_device->newBuffer(sizeof(LightData), MTL::ResourceStorageModeShared);
//        m_instanceBuffer = m_device->newBuffer(sizeof(InstanceData) * m_maxInstances, MTL::ResourceStorageModeShared);
//        
//        // Configuration lumière par défaut
//        LightData* light = static_cast<LightData*>(m_lightBuffer->contents());
//        light->direction = simd::normalize(float3{0.5f, -1.f, 0.3f});
//        light->intensity = 1.2f;
//        light->color = {1.f, 0.98f, 0.95f};
//        light->ambientIntensity = 0.3f;
//    }
//    
//    void initializeBlockMeshes() {
//        // Générer les meshes pour chaque type de bloc
//        for (const auto& [type, def] : BLOCK_DEFINITIONS) {
//            generateBlockMesh(type, def);
//        }
//    }
//    
//    void generateBlockMesh(BlockType type, const BlockDefinition& def) {
//        std::vector<Vertex> vertices;
//        std::vector<uint32_t> indices;
//        
//        float3 size = {def.size.x * BLOCK_SIZE, def.size.y * BLOCK_SIZE, def.size.z * BLOCK_SIZE};
//        float3 halfSize = size * 0.5f;
//        
//        // Génération selon le type
//        switch (type) {
//            case BlockType::WheelSmall:
//            case BlockType::WheelMedium:
//            case BlockType::WheelLarge:
//                generateCylinderMesh(vertices, indices, halfSize.y, halfSize.x, 16, def.color);
//                break;
//                
//            case BlockType::ArmorSloped:
//                generateSlopedMesh(vertices, indices, halfSize, def.color);
//                break;
//                
//            case BlockType::ThrusterSmall:
//            case BlockType::ThrusterLarge:
//                generateThrusterMesh(vertices, indices, halfSize, def.color);
//                break;
//                
//            case BlockType::WingSmall:
//            case BlockType::WingLarge:
//                generateWingMesh(vertices, indices, halfSize, def.color);
//                break;
//                
//            case BlockType::Shield:
//                generateSphereMesh(vertices, indices, halfSize.x, 16, 16, def.color);
//                break;
//                
//            default:
//                generateCubeMesh(vertices, indices, halfSize, def.color);
//                break;
//        }
//        
//        BlockMesh mesh;
//        mesh.vertexBuffer = m_device->newBuffer(vertices.data(), vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
//        mesh.indexBuffer = m_device->newBuffer(indices.data(), indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
//        mesh.indexCount = static_cast<uint32_t>(indices.size());
//        
//        m_blockMeshes[type] = mesh;
//    }
//    
//    void generateCubeMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                          float3 halfSize, float4 color) {
//        // 6 faces, 4 vertices par face
//        float3 normals[6] = {
//            {0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}
//        };
//        
//        float3 positions[8] = {
//            {-halfSize.x, -halfSize.y, -halfSize.z},
//            { halfSize.x, -halfSize.y, -halfSize.z},
//            { halfSize.x,  halfSize.y, -halfSize.z},
//            {-halfSize.x,  halfSize.y, -halfSize.z},
//            {-halfSize.x, -halfSize.y,  halfSize.z},
//            { halfSize.x, -halfSize.y,  halfSize.z},
//            { halfSize.x,  halfSize.y,  halfSize.z},
//            {-halfSize.x,  halfSize.y,  halfSize.z}
//        };
//        
//        int faceIndices[6][4] = {
//            {4, 5, 6, 7}, // Front
//            {1, 0, 3, 2}, // Back
//            {5, 1, 2, 6}, // Right
//            {0, 4, 7, 3}, // Left
//            {3, 7, 6, 2}, // Top
//            {0, 1, 5, 4}  // Bottom
//        };
//        
//        float2 uvs[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
//        
//        for (int f = 0; f < 6; ++f) {
//            uint32_t baseIndex = static_cast<uint32_t>(vertices.size());
//            
//            for (int v = 0; v < 4; ++v) {
//                Vertex vert;
//                vert.position = positions[faceIndices[f][v]];
//                vert.normal = normals[f];
//                vert.texCoord = uvs[v];
//                vert.color = color;
//                vertices.push_back(vert);
//            }
//            
//            indices.push_back(baseIndex + 0);
//            indices.push_back(baseIndex + 1);
//            indices.push_back(baseIndex + 2);
//            indices.push_back(baseIndex + 0);
//            indices.push_back(baseIndex + 2);
//            indices.push_back(baseIndex + 3);
//        }
//    }
//    
//    void generateCylinderMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                              float radius, float height, int segments, float4 color) {
//        float halfHeight = height * 0.5f;
//        
//        // Centre du haut et du bas
//        uint32_t topCenter = static_cast<uint32_t>(vertices.size());
//        vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0.5f}, color});
//        
//        uint32_t bottomCenter = static_cast<uint32_t>(vertices.size());
//        vertices.push_back({{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}, color});
//        
//        for (int i = 0; i <= segments; ++i) {
//            float angle = (float)i / segments * 2.f * M_PI;
//            float x = cos(angle) * radius;
//            float z = sin(angle) * radius;
//            float u = (float)i / segments;
//            
//            // Top
//            vertices.push_back({{x, halfHeight, z}, {0, 1, 0}, {x/radius*0.5f+0.5f, z/radius*0.5f+0.5f}, color});
//            // Bottom
//            vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {x/radius*0.5f+0.5f, z/radius*0.5f+0.5f}, color});
//            // Side top
//            float3 sideNormal = simd::normalize(float3{x, 0, z});
//            vertices.push_back({{x, halfHeight, z}, sideNormal, {u, 0}, color});
//            // Side bottom
//            vertices.push_back({{x, -halfHeight, z}, sideNormal, {u, 1}, color});
//        }
//        
//        // Indices
//        for (int i = 0; i < segments; ++i) {
//            uint32_t base = 2 + i * 4;
//            
//            // Top face
//            indices.push_back(topCenter);
//            indices.push_back(base);
//            indices.push_back(base + 4);
//            
//            // Bottom face
//            indices.push_back(bottomCenter);
//            indices.push_back(base + 5);
//            indices.push_back(base + 1);
//            
//            // Side
//            indices.push_back(base + 2);
//            indices.push_back(base + 3);
//            indices.push_back(base + 7);
//            indices.push_back(base + 2);
//            indices.push_back(base + 7);
//            indices.push_back(base + 6);
//        }
//    }
//    
//    void generateSlopedMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                            float3 halfSize, float4 color) {
//        // Bloc incliné (rampe)
//        float3 positions[6] = {
//            {-halfSize.x, -halfSize.y, -halfSize.z},
//            { halfSize.x, -halfSize.y, -halfSize.z},
//            { halfSize.x, -halfSize.y,  halfSize.z},
//            {-halfSize.x, -halfSize.y,  halfSize.z},
//            { halfSize.x,  halfSize.y,  halfSize.z},
//            {-halfSize.x,  halfSize.y,  halfSize.z}
//        };
//        
//        // Bottom
//        addQuad(vertices, indices, positions[0], positions[1], positions[2], positions[3],
//                {0, -1, 0}, color);
//        // Back
//        addQuad(vertices, indices, positions[1], positions[0], positions[5], positions[4],
//                simd::normalize(float3{0, halfSize.z, halfSize.y}), color);
//        // Slope
//        addQuad(vertices, indices, positions[3], positions[2], positions[4], positions[5],
//                simd::normalize(float3{0, halfSize.z, -halfSize.y}), color);
//        // Left
//        addTriangle(vertices, indices, positions[0], positions[3], positions[5], {-1, 0, 0}, color);
//        // Right
//        addTriangle(vertices, indices, positions[2], positions[1], positions[4], {1, 0, 0}, color);
//    }
//    
//    void generateThrusterMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                              float3 halfSize, float4 color) {
//        // Corps cylindrique + cône de sortie
//        generateCylinderMesh(vertices, indices, halfSize.x * 0.8f, halfSize.z * 1.5f, 12, color);
//        
//        // Ajouter le cône lumineux (émissif)
//        float4 emissiveColor = {1.f, 0.5f, 0.1f, 1.f};
//        size_t baseIndex = vertices.size();
//        
//        float coneRadius = halfSize.x * 0.6f;
//        float coneLength = halfSize.z * 0.8f;
//        int segments = 12;
//        
//        // Pointe du cône
//        vertices.push_back({{0, -halfSize.z - coneLength, 0}, {0, -1, 0}, {0.5f, 0.5f}, emissiveColor});
//        
//        for (int i = 0; i <= segments; ++i) {
//            float angle = (float)i / segments * 2.f * M_PI;
//            float x = cos(angle) * coneRadius;
//            float y = sin(angle) * coneRadius;
//            
//            float3 normal = simd::normalize(float3{x, coneLength, y});
//            vertices.push_back({{x, -halfSize.z, y}, normal, {(float)i/segments, 0}, emissiveColor});
//        }
//        
//        for (int i = 0; i < segments; ++i) {
//            indices.push_back(static_cast<uint32_t>(baseIndex));
//            indices.push_back(static_cast<uint32_t>(baseIndex + 1 + i));
//            indices.push_back(static_cast<uint32_t>(baseIndex + 2 + i));
//        }
//    }
//    
//    void generateWingMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                          float3 halfSize, float4 color) {
//        // Aile plate avec profil aérodynamique simplifié
//        float thickness = halfSize.y * 0.3f;
//        
//        float3 positions[8] = {
//            {-halfSize.x, -thickness,  halfSize.z},
//            { halfSize.x, -thickness,  halfSize.z * 0.3f},
//            { halfSize.x,  thickness,  halfSize.z * 0.3f},
//            {-halfSize.x,  thickness,  halfSize.z},
//            {-halfSize.x, -thickness, -halfSize.z},
//            { halfSize.x, -thickness, -halfSize.z * 0.3f},
//            { halfSize.x,  thickness, -halfSize.z * 0.3f},
//            {-halfSize.x,  thickness, -halfSize.z}
//        };
//        
//        // Top
//        addQuad(vertices, indices, positions[3], positions[2], positions[6], positions[7], {0, 1, 0}, color);
//        // Bottom
//        addQuad(vertices, indices, positions[0], positions[4], positions[5], positions[1], {0, -1, 0}, color);
//        // Front
//        addQuad(vertices, indices, positions[0], positions[1], positions[2], positions[3], {0, 0, 1}, color);
//        // Back
//        addQuad(vertices, indices, positions[5], positions[4], positions[7], positions[6], {0, 0, -1}, color);
//        // Right tip
//        addQuad(vertices, indices, positions[1], positions[5], positions[6], positions[2], {1, 0, 0}, color);
//        // Left root
//        addQuad(vertices, indices, positions[4], positions[0], positions[3], positions[7], {-1, 0, 0}, color);
//    }
//    
//    void generateSphereMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                            float radius, int latSegments, int lonSegments, float4 color) {
//        for (int lat = 0; lat <= latSegments; ++lat) {
//            float theta = lat * M_PI / latSegments;
//            float sinTheta = sin(theta);
//            float cosTheta = cos(theta);
//            
//            for (int lon = 0; lon <= lonSegments; ++lon) {
//                float phi = lon * 2.f * M_PI / lonSegments;
//                float sinPhi = sin(phi);
//                float cosPhi = cos(phi);
//                
//                float3 normal = {cosPhi * sinTheta, cosTheta, sinPhi * sinTheta};
//                float3 position = normal * radius;
//                float2 texCoord = {(float)lon / lonSegments, (float)lat / latSegments};
//                
//                vertices.push_back({position, normal, texCoord, color});
//            }
//        }
//        
//        for (int lat = 0; lat < latSegments; ++lat) {
//            for (int lon = 0; lon < lonSegments; ++lon) {
//                uint32_t current = lat * (lonSegments + 1) + lon;
//                uint32_t next = current + lonSegments + 1;
//                
//                indices.push_back(current);
//                indices.push_back(next);
//                indices.push_back(current + 1);
//                
//                indices.push_back(current + 1);
//                indices.push_back(next);
//                indices.push_back(next + 1);
//            }
//        }
//    }
//    
//    void addQuad(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                 float3 p0, float3 p1, float3 p2, float3 p3, float3 normal, float4 color) {
//        uint32_t base = static_cast<uint32_t>(vertices.size());
//        vertices.push_back({p0, normal, {0, 0}, color});
//        vertices.push_back({p1, normal, {1, 0}, color});
//        vertices.push_back({p2, normal, {1, 1}, color});
//        vertices.push_back({p3, normal, {0, 1}, color});
//        
//        indices.push_back(base);
//        indices.push_back(base + 1);
//        indices.push_back(base + 2);
//        indices.push_back(base);
//        indices.push_back(base + 2);
//        indices.push_back(base + 3);
//    }
//    
//    void addTriangle(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
//                     float3 p0, float3 p1, float3 p2, float3 normal, float4 color) {
//        uint32_t base = static_cast<uint32_t>(vertices.size());
//        vertices.push_back({p0, normal, {0, 0}, color});
//        vertices.push_back({p1, normal, {1, 0}, color});
//        vertices.push_back({p2, normal, {0.5f, 1}, color});
//        
//        indices.push_back(base);
//        indices.push_back(base + 1);
//        indices.push_back(base + 2);
//    }
//    
//    void initializeTerrain() {
//        // Générer des chunks de terrain basiques
//        const int chunkSize = 64;
//        const int numChunks = 4;
//        
//        for (int cx = -numChunks; cx < numChunks; ++cx) {
//            for (int cz = -numChunks; cz < numChunks; ++cz) {
//                TerrainChunk chunk;
//                chunk.position = {cx * chunkSize * BLOCK_SIZE, 0, cz * chunkSize * BLOCK_SIZE};
//                
//                // Générer un terrain simple avec du bruit
//                std::vector<float> heightmap((chunkSize + 1) * (chunkSize + 1));
//                for (int z = 0; z <= chunkSize; ++z) {
//                    for (int x = 0; x <= chunkSize; ++x) {
//                        float wx = (cx * chunkSize + x) * 0.05f;
//                        float wz = (cz * chunkSize + z) * 0.05f;
//                        heightmap[z * (chunkSize + 1) + x] =
//                            sin(wx) * cos(wz) * 5.f +
//                            sin(wx * 2.3f) * cos(wz * 2.1f) * 2.f;
//                    }
//                }
//                
//                // Générer le mesh
//                for (int z = 0; z <= chunkSize; ++z) {
//                    for (int x = 0; x <= chunkSize; ++x) {
//                        float height = heightmap[z * (chunkSize + 1) + x];
//                        float3 pos = {
//                            chunk.position.x + x * BLOCK_SIZE,
//                            height,
//                            chunk.position.z + z * BLOCK_SIZE
//                        };
//                        
//                        // Calculer la normale
//                        float hL = x > 0 ? heightmap[z * (chunkSize + 1) + x - 1] : height;
//                        float hR = x < chunkSize ? heightmap[z * (chunkSize + 1) + x + 1] : height;
//                        float hD = z > 0 ? heightmap[(z - 1) * (chunkSize + 1) + x] : height;
//                        float hU = z < chunkSize ? heightmap[(z + 1) * (chunkSize + 1) + x] : height;
//                        float3 normal = simd::normalize(float3{hL - hR, 2.f, hD - hU});
//                        
//                        // Couleur basée sur la hauteur
//                        float4 color;
//                        if (height < 0) {
//                            color = {0.2f, 0.3f, 0.8f, 1.f}; // Eau
//                        } else if (height < 2) {
//                            color = {0.3f, 0.6f, 0.2f, 1.f}; // Herbe
//                        } else if (height < 5) {
//                            color = {0.5f, 0.4f, 0.3f, 1.f}; // Terre
//                        } else {
//                            color = {0.6f, 0.6f, 0.65f, 1.f}; // Roche
//                        }
//                        
//                        chunk.vertices.push_back({pos, normal, {(float)x/chunkSize, (float)z/chunkSize}, color});
//                    }
//                }
//                
//                // Indices
//                for (int z = 0; z < chunkSize; ++z) {
//                    for (int x = 0; x < chunkSize; ++x) {
//                        uint32_t tl = z * (chunkSize + 1) + x;
//                        uint32_t tr = tl + 1;
//                        uint32_t bl = tl + chunkSize + 1;
//                        uint32_t br = bl + 1;
//                        
//                        chunk.indices.push_back(tl);
//                        chunk.indices.push_back(bl);
//                        chunk.indices.push_back(tr);
//                        chunk.indices.push_back(tr);
//                        chunk.indices.push_back(bl);
//                        chunk.indices.push_back(br);
//                    }
//                }
//                
//                chunk.vertexBuffer = m_device->newBuffer(chunk.vertices.data(),
//                    chunk.vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
//                chunk.indexBuffer = m_device->newBuffer(chunk.indices.data(),
//                    chunk.indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
//                
//                m_terrainChunks.push_back(std::move(chunk));
//            }
//        }
//        
//        // Ajouter des ressources
//        std::mt19937 rng(42);
//        std::uniform_real_distribution<float> posDist(-200.f, 200.f);
//        std::uniform_int_distribution<int> typeDist(0, static_cast<int>(ResourceType::COUNT) - 1);
//        std::uniform_real_distribution<float> amountDist(50.f, 200.f);
//        
//        for (int i = 0; i < 50; ++i) {
//            ResourceChunk resource;
//            resource.type = static_cast<ResourceType>(typeDist(rng));
//            resource.position = {posDist(rng), 0.5f, posDist(rng)};
//            resource.amount = amountDist(rng);
//            resource.maxAmount = resource.amount;
//            m_resources.push_back(resource);
//        }
//    }
//    
//    void initializeDefaultTech() {
//        // Créer le tech du joueur
//        uint32_t playerId = createTech("Player Tech", Faction::Player, {0, 5, 0}, true);
//        
//        // Ajouter une configuration de base
//        addBlockToTech(playerId, BlockType::CabSmall, {32, 32, 32});
//        addBlockToTech(playerId, BlockType::Chassis, {31, 32, 32});
//        addBlockToTech(playerId, BlockType::Chassis, {33, 32, 32});
//        addBlockToTech(playerId, BlockType::Chassis, {32, 32, 31});
//        addBlockToTech(playerId, BlockType::Chassis, {32, 32, 33});
//        addBlockToTech(playerId, BlockType::WheelSmall, {31, 31, 31});
//        addBlockToTech(playerId, BlockType::WheelSmall, {33, 31, 31});
//        addBlockToTech(playerId, BlockType::WheelSmall, {31, 31, 33});
//        addBlockToTech(playerId, BlockType::WheelSmall, {33, 31, 33});
//        addBlockToTech(playerId, BlockType::MachineGun, {32, 33, 32});
//        addBlockToTech(playerId, BlockType::Battery, {32, 32, 34});
//        
//        // Créer quelques ennemis
//        for (int i = 0; i < 3; ++i) {
//            uint32_t enemyId = createTech("Enemy " + std::to_string(i), Faction::GSO,
//                                          {50.f + i * 30.f, 5, 50.f + i * 20.f}, false);
//            addBlockToTech(enemyId, BlockType::CabSmall, {32, 32, 32});
//            addBlockToTech(enemyId, BlockType::Chassis, {31, 32, 32});
//            addBlockToTech(enemyId, BlockType::Chassis, {33, 32, 32});
//            addBlockToTech(enemyId, BlockType::WheelSmall, {31, 31, 32});
//            addBlockToTech(enemyId, BlockType::WheelSmall, {33, 31, 32});
//            addBlockToTech(enemyId, BlockType::MachineGun, {32, 33, 32});
//        }
//    }
//    
//    // ========================================================================
//    // UPDATE PRIVÉ
//    // ========================================================================
//    
//    void updateInput() {
//        Tech* playerTech = getTech(m_playerTechId);
//        if (!playerTech) return;
//        
//        playerTech->inputDirection = m_inputMove;
//        playerTech->isFiring = m_inputFire;
//    }
//    
//    void updatePhysics(float deltaTime) {
//        for (auto& tech : m_techs) {
//            if (!tech.isValid()) continue;
//            
//            // Gravité
//            if (!tech.isGrounded) {
//                tech.velocity.y -= GRAVITY * deltaTime;
//            }
//            
//            // Mouvement
//            if (tech.isPlayer && !m_buildMode) {
//                float3 forward = getForward(tech.rotation);
//                float3 right = getRight(tech.rotation);
//                
//                float3 moveForce = {0, 0, 0};
//                moveForce += forward * tech.inputDirection.z;
//                moveForce += right * tech.inputDirection.x;
//                
//                if (simd::length(moveForce) > 0.f) {
//                    moveForce = simd::normalize(moveForce);
//                    float acceleration = tech.totalThrust / std::max(tech.totalMass, 1.f);
//                    tech.velocity += moveForce * acceleration * deltaTime;
//                }
//                
//                // Friction
//                float friction = tech.isGrounded ? 5.f : 0.5f;
//                tech.velocity.x *= 1.f - friction * deltaTime;
//                tech.velocity.z *= 1.f - friction * deltaTime;
//                
//                // Limiter la vitesse
//                float speed = simd::length(float3{tech.velocity.x, 0, tech.velocity.z});
//                if (speed > tech.maxSpeed) {
//                    float scale = tech.maxSpeed / speed;
//                    tech.velocity.x *= scale;
//                    tech.velocity.z *= scale;
//                }
//                
//                // Rotation vers la direction du mouvement
//                if (speed > 0.1f && simd::length(tech.inputDirection) > 0.f) {
//                    float targetYaw = atan2(tech.velocity.x, tech.velocity.z);
//                    float currentYaw = getYaw(tech.rotation);
//                    float yawDiff = targetYaw - currentYaw;
//                    
//                    // Normaliser
//                    while (yawDiff > M_PI) yawDiff -= 2.f * M_PI;
//                    while (yawDiff < -M_PI) yawDiff += 2.f * M_PI;
//                    
//                    float turnRate = 3.f * deltaTime;
//                    yawDiff = std::clamp(yawDiff, -turnRate, turnRate);
//                    tech.rotation = simd::quatf{0, sin((currentYaw + yawDiff) * 0.5f), 0,
//                                                cos((currentYaw + yawDiff) * 0.5f)};
//                }
//            }
//            
//            // Appliquer la vélocité
//            tech.position += tech.velocity * deltaTime;
//            
//            // Collision avec le sol
//            float groundHeight = getGroundHeight(tech.position.x, tech.position.z);
//            float techBottom = tech.position.y - getTechHeight(tech) * 0.5f;
//            
//            if (techBottom < groundHeight) {
//                tech.position.y = groundHeight + getTechHeight(tech) * 0.5f;
//                tech.velocity.y = 0.f;
//                tech.isGrounded = true;
//            } else {
//                tech.isGrounded = false;
//            }
//        }
//    }
//    
//    void updateTechs(float deltaTime) {
//        for (auto& tech : m_techs) {
//            if (!tech.isValid()) continue;
//            
//            // Régénération d'énergie
//            float netEnergy = tech.energyGeneration - tech.energyConsumption;
//            tech.currentEnergy = std::clamp(tech.currentEnergy + netEnergy * deltaTime,
//                                            0.f, tech.maxEnergy);
//            
//            // Mise à jour des blocs
//            for (auto& block : tech.blocks) {
//                // Cooldown des armes
//                if (block.weaponCooldown > 0.f) {
//                    block.weaponCooldown -= deltaTime;
//                }
//                
//                // Rotation des roues
//                auto defIt = BLOCK_DEFINITIONS.find(block.type);
//                if (defIt != BLOCK_DEFINITIONS.end() && defIt->second.category == BlockCategory::Wheels) {
//                    float speed = simd::length(tech.velocity);
//                    block.wheelRotation += speed * deltaTime * 2.f;
//                }
//            }
//            
//            // Tir automatique si demandé
//            if (tech.isFiring) {
//                for (const auto& block : tech.blocks) {
//                    auto defIt = BLOCK_DEFINITIONS.find(block.type);
//                    if (defIt != BLOCK_DEFINITIONS.end() && defIt->second.category == BlockCategory::Weapons) {
//                        fireWeapon(tech.techId, block.blockId);
//                    }
//                }
//            }
//            
//            // IA basique pour les ennemis
//            if (tech.isAI && !tech.isPlayer) {
//                updateAI(tech, deltaTime);
//            }
//        }
//    }
//    
//    void updateAI(Tech& tech, float deltaTime) {
//        Tech* playerTech = getTech(m_playerTechId);
//        if (!playerTech) return;
//        
//        float3 toPlayer = playerTech->position - tech.position;
//        float distance = simd::length(toPlayer);
//        
//        if (distance < 100.f && distance > 10.f) {
//            // Se rapprocher du joueur
//            tech.inputDirection = simd::normalize(toPlayer);
//        } else if (distance <= 10.f) {
//            // Tirer
//            tech.isFiring = true;
//            tech.inputDirection = {0, 0, 0};
//        } else {
//            // Patrouiller
//            tech.inputDirection = {sin(m_time * 0.5f + tech.techId), 0, cos(m_time * 0.3f + tech.techId)};
//            tech.isFiring = false;
//        }
//    }
//    
//    void updateProjectiles(float deltaTime) {
//        for (auto it = m_projectiles.begin(); it != m_projectiles.end();) {
//            Projectile& proj = *it;
//            
//            proj.lifetime += deltaTime;
//            if (proj.lifetime >= proj.maxLifetime) {
//                it = m_projectiles.erase(it);
//                continue;
//            }
//            
//            // Homing
//            if (proj.isHoming && proj.targetTechId != 0) {
//                Tech* target = getTech(proj.targetTechId);
//                if (target) {
//                    float3 toTarget = target->position - proj.position;
//                    float3 desiredVel = simd::normalize(toTarget) * simd::length(proj.velocity);
//                    proj.velocity = simd_mix(proj.velocity, desiredVel, deltaTime * 3.f);
//                }
//            }
//            
//            proj.position += proj.velocity * deltaTime;
//            
//            // Collision avec les techs
//            bool hit = false;
//            for (auto& tech : m_techs) {
//                if (tech.techId == proj.ownerTechId) continue;
//                if (!tech.isValid()) continue;
//                
//                // AABB simple
//                float3 techHalfSize = getTechBounds(tech) * 0.5f;
//                float3 diff = proj.position - tech.position;
//                
//                if (std::abs(diff.x) < techHalfSize.x + proj.radius &&
//                    std::abs(diff.y) < techHalfSize.y + proj.radius &&
//                    std::abs(diff.z) < techHalfSize.z + proj.radius) {
//                    
//                    // Trouver le bloc le plus proche
//                    uint32_t closestBlockId = 0;
//                    float closestDist = FLT_MAX;
//                    
//                    for (const auto& block : tech.blocks) {
//                        float3 blockWorldPos = tech.position + gridToWorld(block.gridPosition);
//                        float dist = simd::length(proj.position - blockWorldPos);
//                        if (dist < closestDist) {
//                            closestDist = dist;
//                            closestBlockId = block.blockId;
//                        }
//                    }
//                    
//                    if (closestBlockId != 0) {
//                        damageBlock(tech.techId, closestBlockId, proj.damage);
//                        spawnHitEffect(proj.position, proj.weaponType);
//                        hit = true;
//                        break;
//                    }
//                }
//            }
//            
//            // Collision avec le sol
//            float groundHeight = getGroundHeight(proj.position.x, proj.position.z);
//            if (proj.position.y < groundHeight) {
//                spawnHitEffect(proj.position, proj.weaponType);
//                hit = true;
//            }
//            
//            if (hit) {
//                it = m_projectiles.erase(it);
//            } else {
//                ++it;
//            }
//        }
//    }
//    
//    void updateResources(float deltaTime) {
//        Tech* playerTech = getTech(m_playerTechId);
//        if (!playerTech) return;
//        
//        // Vérifier si le joueur a un collecteur
//        bool hasCollector = false;
//        for (const auto& block : playerTech->blocks) {
//            if (block.type == BlockType::ResourceCollector) {
//                hasCollector = true;
//                break;
//            }
//        }
//        
//        if (!hasCollector) return;
//        
//        for (auto& resource : m_resources) {
//            if (resource.isCollected) continue;
//            
//            float distance = simd::length(resource.position - playerTech->position);
//            if (distance < 5.f) {
//                // Collecter
//                float collectRate = 20.f * deltaTime;
//                float collected = std::min(collectRate, resource.amount);
//                resource.amount -= collected;
//                
//                if (resource.amount <= 0.f) {
//                    resource.isCollected = true;
//                }
//                
//                // Ajouter au stockage du joueur (simplifié)
//            }
//        }
//    }
//    
//    void updateCamera(float deltaTime) {
//        Tech* playerTech = getTech(m_playerTechId);
//        if (!playerTech) return;
//        
//        // Suivre le joueur
//        m_cameraTarget = playerTech->position;
//        
//        // Position de la caméra en orbite
//        float3 offset = {
//            sin(m_cameraYaw) * cos(m_cameraPitch) * m_cameraDistance,
//            sin(m_cameraPitch) * m_cameraDistance,
//            cos(m_cameraYaw) * cos(m_cameraPitch) * m_cameraDistance
//        };
//        
//        m_cameraPosition = m_cameraTarget + offset;
//    }
//    
//    void updateUniforms() {
//        // Camera
//        CameraUniforms* camera = static_cast<CameraUniforms*>(m_cameraBuffer->contents());
//        
//        camera->viewMatrix = lookAt(m_cameraPosition, m_cameraTarget, float3{0, 1, 0});
//        camera->projectionMatrix = perspective(45.f * M_PI / 180.f,
//                                               m_viewportSize.x / m_viewportSize.y,
//                                               0.1f, 1000.f);
//        camera->viewProjectionMatrix = camera->projectionMatrix * camera->viewMatrix;
//        camera->cameraPosition = m_cameraPosition;
//        camera->time = m_time;
//        
//        // Instances
//        if (m_instancesDirty) {
//            rebuildInstances();
//            m_instancesDirty = false;
//        }
//    }
//    
//    void rebuildInstances() {
//        std::vector<InstanceData> instances;
//        
//        for (const auto& tech : m_techs) {
//            if (!tech.isValid()) continue;
//            
//            for (const auto& block : tech.blocks) {
//                auto defIt = BLOCK_DEFINITIONS.find(block.type);
//                if (defIt == BLOCK_DEFINITIONS.end()) continue;
//                
//                const auto& def = defIt->second;
//                
//                InstanceData inst;
//                
//                // Matrice de transformation
//                float3 worldPos = tech.position + gridToWorld(block.gridPosition);
//                float4x4 translation = translationMatrix(worldPos);
//                float4x4 rotation = rotationMatrix(tech.rotation) * blockRotationMatrix(block.rotation);
//                inst.modelMatrix = translation * rotation;
//                
//                inst.color = def.color;
//                
//                // Émissif pour certains blocs
//                if (def.category == BlockCategory::Power && def.energyGeneration > 0) {
//                    inst.emissive = {0.2f, 0.8f, 0.2f, 1.f};
//                } else if (block.type == BlockType::Laser) {
//                    inst.emissive = {0.2f, 0.8f, 0.9f, 1.f};
//                } else {
//                    inst.emissive = {0, 0, 0, 0};
//                }
//                
//                inst.metallic = (def.category == BlockCategory::Weapons) ? 0.8f : 0.3f;
//                inst.roughness = 0.5f;
//                inst.health = block.currentHealth / def.health;
//                inst.blockType = static_cast<uint>(block.type);
//                
//                instances.push_back(inst);
//            }
//        }
//        
//        if (instances.size() > m_maxInstances) {
//            m_maxInstances = instances.size() * 2;
//            m_instanceBuffer->release();
//            m_instanceBuffer = m_device->newBuffer(sizeof(InstanceData) * m_maxInstances,
//                                                   MTL::ResourceStorageModeShared);
//        }
//        
//        memcpy(m_instanceBuffer->contents(), instances.data(), instances.size() * sizeof(InstanceData));
//    }
//    
//    // ========================================================================
//    // RENDU PRIVÉ
//    // ========================================================================
//    
//    void renderTerrain(MTL::RenderCommandEncoder* encoder) {
//        if (!m_terrainPipeline) return;
//        
//        encoder->setRenderPipelineState(m_terrainPipeline);
//        encoder->setDepthStencilState(m_depthState);
//        encoder->setVertexBuffer(m_cameraBuffer, 0, 1);
//        encoder->setFragmentBuffer(m_lightBuffer, 0, 0);
//        encoder->setFragmentBuffer(m_cameraBuffer, 0, 1);
//        
//        for (const auto& chunk : m_terrainChunks) {
//            encoder->setVertexBuffer(chunk.vertexBuffer, 0, 0);
//            encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
//                                           static_cast<NS::UInteger>(chunk.indices.size()),
//                                           MTL::IndexTypeUInt32, chunk.indexBuffer, 0);
//        }
//    }
//    
//    void renderResources(MTL::RenderCommandEncoder* encoder) {
//        // Rendu des ressources comme des sphères colorées
//        // Simplifié - utiliser le mesh de bloc sphérique
//    }
//    
//    void renderTechs(MTL::RenderCommandEncoder* encoder) {
//        if (!m_blockPipeline) return;
//        
//        encoder->setRenderPipelineState(m_blockPipeline);
//        encoder->setDepthStencilState(m_depthState);
//        encoder->setVertexBuffer(m_cameraBuffer, 0, 1);
//        encoder->setVertexBuffer(m_instanceBuffer, 0, 2);
//        encoder->setFragmentBuffer(m_lightBuffer, 0, 0);
//        encoder->setFragmentBuffer(m_cameraBuffer, 0, 1);
//        
//        // Rendu instancié par type de bloc
//        uint32_t instanceOffset = 0;
//        for (const auto& tech : m_techs) {
//            if (!tech.isValid()) continue;
//            
//            for (const auto& block : tech.blocks) {
//                auto meshIt = m_blockMeshes.find(block.type);
//                if (meshIt == m_blockMeshes.end()) continue;
//                
//                const BlockMesh& mesh = meshIt->second;
//                encoder->setVertexBuffer(mesh.vertexBuffer, 0, 0);
//                encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(mesh.indexCount), MTL::IndexTypeUInt32, mesh.indexBuffer, NS::UInteger(1), NS::UInteger(instanceOffset));
//                instanceOffset++;
//            }
//        }
//    }
//    
//    void renderProjectiles(MTL::RenderCommandEncoder* encoder) {
//        // Projectiles comme petites sphères émissives
//        for (const auto& proj : m_projectiles) {
//            // Rendu simplifié
//        }
//    }
//    
//    void renderEffects(MTL::RenderCommandEncoder* encoder) {
//        // Boucliers, particules, traînées de propulseurs
//    }
//    
//    void renderBuildUI(MTL::RenderCommandEncoder* encoder) {
//        // Afficher le curseur de construction et le bloc fantôme
//    }
//    
//    // ========================================================================
//    // UTILITAIRES
//    // ========================================================================
//    
//    Tech* getTech(uint32_t techId) {
//        for (auto& tech : m_techs) {
//            if (tech.techId == techId) return &tech;
//        }
//        return nullptr;
//    }
//    
//    void destroyTech(uint32_t techId) {
//        auto it = std::find_if(m_techs.begin(), m_techs.end(),
//            [techId](const Tech& t) { return t.techId == techId; });
//        if (it != m_techs.end()) {
//            // Spawn debris
//            m_techs.erase(it);
//        }
//    }
//    
//    float3 gridToWorld(int3 gridPos) const {
//        return float3{
//            (gridPos.x - Tech::GRID_SIZE/2) * BLOCK_SIZE,
//            (gridPos.y - Tech::GRID_SIZE/2) * BLOCK_SIZE,
//            (gridPos.z - Tech::GRID_SIZE/2) * BLOCK_SIZE
//        };
//    }
//    
//    bool hasAdjacentBlock(const Tech& tech, int3 pos, int3 size) const {
//        for (int x = -1; x <= size.x; ++x) {
//            for (int y = -1; y <= size.y; ++y) {
//                for (int z = -1; z <= size.z; ++z) {
//                    if (x >= 0 && x < size.x && y >= 0 && y < size.y && z >= 0 && z < size.z)
//                        continue;
//                    int3 checkPos = {pos.x + x, pos.y + y, pos.z + z};
//                    if (tech.isOccupied(checkPos)) return true;
//                }
//            }
//        }
//        return false;
//    }
//    
//    float getTechHeight(const Tech& tech) const {
//        int minY = Tech::GRID_SIZE, maxY = 0;
//        for (const auto& block : tech.blocks) {
//            minY = std::min(minY, block.gridPosition.y);
//            auto defIt = BLOCK_DEFINITIONS.find(block.type);
//            int h = defIt != BLOCK_DEFINITIONS.end() ? defIt->second.size.y : 1;
//            maxY = std::max(maxY, block.gridPosition.y + h);
//        }
//        return (maxY - minY) * BLOCK_SIZE;
//    }
//    
//    float3 getTechBounds(const Tech& tech) const {
//        int3 minB = {Tech::GRID_SIZE, Tech::GRID_SIZE, Tech::GRID_SIZE};
//        int3 maxB = {0, 0, 0};
//        for (const auto& block : tech.blocks) {
//            minB.x = std::min(minB.x, block.gridPosition.x);
//            minB.y = std::min(minB.y, block.gridPosition.y);
//            minB.z = std::min(minB.z, block.gridPosition.z);
//            auto defIt = BLOCK_DEFINITIONS.find(block.type);
//            int3 s = defIt != BLOCK_DEFINITIONS.end() ? defIt->second.size : int3{1,1,1};
//            maxB.x = std::max(maxB.x, block.gridPosition.x + s.x);
//            maxB.y = std::max(maxB.y, block.gridPosition.y + s.y);
//            maxB.z = std::max(maxB.z, block.gridPosition.z + s.z);
//        }
//        return float3{(float)(maxB.x-minB.x), (float)(maxB.y-minB.y), (float)(maxB.z-minB.z)} * BLOCK_SIZE;
//    }
//    
//    float getGroundHeight(float x, float z) const {
//        return sin(x * 0.05f) * cos(z * 0.05f) * 5.f +
//               sin(x * 0.115f) * cos(z * 0.105f) * 2.f;
//    }
//    
//    float3 getForward(simd::quatf q) const {
//        return simd_act(q, float3{0, 0, 1});
//    }
//    
//    float3 getRight(simd::quatf q) const {
//        return simd_act(q, float3{1, 0, 0});
//    }
//    
//    float getYaw(simd::quatf q) const {
//        return atan2(2.f * (q.vector.y * q.vector.w + q.vector.x * q.vector.z),
//                     1.f - 2.f * (q.vector.x * q.vector.x + q.vector.y * q.vector.y));
//    }
//    
//    float3 getBlockForward(uint8_t blockRot, simd::quatf techRot) const {
//        // 24 rotations possibles - simplifié
//        float3 localForward = {0, 0, 1};
//        return simd_act(techRot, localForward);
//    }
//    
//    float getProjectileSpeed(BlockType type) const {
//        switch (type) {
//            case BlockType::MachineGun: return 100.f;
//            case BlockType::Cannon: return 80.f;
//            case BlockType::Laser: return 200.f;
//            case BlockType::MissileLauncher: return 40.f;
//            case BlockType::PlasmaGun: return 60.f;
//            default: return 50.f;
//        }
//    }
//    
//    float getProjectileLifetime(BlockType type) const {
//        switch (type) {
//            case BlockType::MissileLauncher: return 5.f;
//            case BlockType::Laser: return 0.5f;
//            default: return 3.f;
//        }
//    }
//    
//    float getProjectileRadius(BlockType type) const {
//        switch (type) {
//            case BlockType::Cannon: return 0.5f;
//            case BlockType::MissileLauncher: return 0.3f;
//            default: return 0.1f;
//        }
//    }
//    
//    void spawnBlockDebris(float3 pos, BlockType type) {
//        // Spawner des particules de débris
//    }
//    
//    void spawnHitEffect(float3 pos, BlockType type) {
//        // Spawner un effet d'impact
//    }
//    
//    // Matrices utilitaires
//    float4x4 translationMatrix(float3 t) const {
//        return float4x4{
//            float4{1, 0, 0, 0},
//            float4{0, 1, 0, 0},
//            float4{0, 0, 1, 0},
//            float4{t.x, t.y, t.z, 1}
//        };
//    }
//    
//    float4x4 rotationMatrix(simd::quatf q) const {
//        float x = q.vector.x, y = q.vector.y, z = q.vector.z, w = q.vector.w;
//        return float4x4{
//            float4{1-2*(y*y+z*z), 2*(x*y+w*z), 2*(x*z-w*y), 0},
//            float4{2*(x*y-w*z), 1-2*(x*x+z*z), 2*(y*z+w*x), 0},
//            float4{2*(x*z+w*y), 2*(y*z-w*x), 1-2*(x*x+y*y), 0},
//            float4{0, 0, 0, 1}
//        };
//    }
//    
//    float4x4 blockRotationMatrix(uint8_t rot) const {
//        // 24 rotations - retourne identité pour simplifier
//        return float4x4{
//            float4{1,0,0,0}, float4{0,1,0,0}, float4{0,0,1,0}, float4{0,0,0,1}
//        };
//    }
//    
//    float4x4 lookAt(float3 eye, float3 target, float3 up) const {
//        float3 f = simd::normalize(target - eye);
//        float3 r = simd::normalize(simd::cross(f, up));
//        float3 u = simd::cross(r, f);
//        return float4x4{
//            float4{r.x, u.x, -f.x, 0},
//            float4{r.y, u.y, -f.y, 0},
//            float4{r.z, u.z, -f.z, 0},
//            float4{-simd::dot(r,eye), -simd::dot(u,eye), simd::dot(f,eye), 1}
//        };
//    }
//    
//    float4x4 perspective(float fovY, float aspect, float near, float far) const {
//        float tanHalfFov = tan(fovY * 0.5f);
//        float4x4 m = {};
//        m.columns[0][0] = 1.f / (aspect * tanHalfFov);
//        m.columns[1][1] = 1.f / tanHalfFov;
//        m.columns[2][2] = far / (near - far);
//        m.columns[2][3] = -1.f;
//        m.columns[3][2] = (far * near) / (near - far);
//        return m;
//    }
//    
//    void cleanup() {
//        if (m_cameraBuffer) m_cameraBuffer->release();
//        if (m_lightBuffer) m_lightBuffer->release();
//        if (m_instanceBuffer) m_instanceBuffer->release();
//        if (m_blockPipeline) m_blockPipeline->release();
//        if (m_terrainPipeline) m_terrainPipeline->release();
//        if (m_depthState) m_depthState->release();
//        
//        for (auto& [type, mesh] : m_blockMeshes) {
//            if (mesh.vertexBuffer) mesh.vertexBuffer->release();
//            if (mesh.indexBuffer) mesh.indexBuffer->release();
//        }
//        
//        for (auto& chunk : m_terrainChunks) {
//            if (chunk.vertexBuffer) chunk.vertexBuffer->release();
//            if (chunk.indexBuffer) chunk.indexBuffer->release();
//        }
//    }
//};
//
//} // namespace Terratech
//
//#endif /* RMDLCmd_hpp */
