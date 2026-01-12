//
//  RMDLPhysics.hpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#ifndef RMDLPhysics_hpp
#define RMDLPhysics_hpp

#include <Metal/Metal.hpp>

#include <simd/simd.h>
#include <vector>
#include <cmath>

#include "RMDLSystem.hpp"
#include "RMDLGrid.hpp"

// Forward declarations
class RMDLGrid;
class TerrainSystem;

// Physics entity
struct PhysicsEntity {
    simd::float3 position;
    simd::float3 velocity;
    simd::float3 acceleration;
    simd::float3 angularVelocity;
    simd::float4 rotation;  // Quaternion
    
    float mass;
    float radius;           // Simplified collision shape (sphere)
    float friction;
    float restitution;      // Bounciness
    
    bool isStatic;
    bool onGround;
    bool affectedByGravity;
    
    PhysicsEntity()
        : position{0,0,0}
        , velocity{0,0,0}
        , acceleration{0,0,0}
        , angularVelocity{0,0,0}
        , rotation{0,0,0,1}
        , mass(1.0f)
        , radius(0.5f)
        , friction(0.5f)
        , restitution(0.3f)
        , isStatic(false)
        , onGround(false)
        , affectedByGravity(true)
    {}
};

// Player physics (character controller)
struct PlayerPhysics {
    PhysicsEntity entity;
    
    float walkSpeed;
    float runSpeed;
    float jumpForce;
    float crouchHeight;
    float standHeight;
    
    bool isCrouching;
    bool isJumping;
    bool canJump;
    
    PlayerPhysics()
        : walkSpeed(5.0f)
        , runSpeed(10.0f)
        , jumpForce(8.0f)
        , crouchHeight(0.9f)
        , standHeight(1.8f)
        , isCrouching(false)
        , isJumping(false)
        , canJump(true)
    {
        entity.radius = 0.4f;
        entity.mass = 70.0f;
    }
};

// Vehicle physics (for the command block vehicle)
struct VehiclePhysics {
    PhysicsEntity entity;
    
    simd::float3 engineForce;
    simd::float3 torque;
    
    float maxSpeed;
    float acceleration;
    float turning;
    float thrustPower;
    
    bool isGrounded;
    
    VehiclePhysics()
        : engineForce{0,0,0}
        , torque{0,0,0}
        , maxSpeed(20.0f)
        , acceleration(15.0f)
        , turning(2.0f)
        , thrustPower(100.0f)
        , isGrounded(false)
    {}
};

// GPU uniforms for physics compute shaders
struct PhysicsUniforms {
    simd::float3 gravity;
    float deltaTime;
    float airDrag;
    float groundDrag;
    uint entityCount;
    float padding;
};

class PhysicsSystem {
public:
    PhysicsSystem(MTL::Device* device, MTL::Library* library);
    ~PhysicsSystem();
    
    // Initialize
    void setGravity(const simd::float3& gravity) { m_gravity = gravity; }
    void setTerrainSystem(TerrainSystem* terrain) { m_terrain = terrain; }
    void setVoxelGrid(RMDLGrid* voxelGrid) { m_voxelGrid = voxelGrid; }
    
    // Update
    void update(float deltaTime);
    void updateGPU(float deltaTime);  // GPU-accelerated physics
    
    // Player control
    PlayerPhysics& getPlayer() { return m_player; }
    void applyPlayerInput(const simd::float3& moveDirection, bool jump, bool crouch, bool run);
    void updatePlayer(float deltaTime);
    
    // Vehicle control
    VehiclePhysics& getVehicle() { return m_vehicle; }
    void applyVehicleInput(const simd::float3& thrust, float turn);
    void updateVehicle(float deltaTime);
    
    // Entity management
    uint32_t addEntity(const PhysicsEntity& entity);
    void removeEntity(uint32_t index);
    PhysicsEntity* getEntity(uint32_t index);
    
    // Collision detection
    bool checkSphereCollision(const simd::float3& pos, float radius,
                             simd::float3& normal, float& penetration);
    simd::float3 resolveSphereCollision(const simd::float3& pos,
                                        const simd::float3& vel,
                                        float radius);
    
    // Raycasting
    struct RaycastResult {
        bool hit;
        simd::float3 point;
        simd::float3 normal;
        float distance;
        uint32_t entityIndex;
    };
    RaycastResult raycast(const simd::float3& origin, const simd::float3& direction,
                         float maxDistance, bool checkTerrain = true,
                         bool checkVoxels = true, bool checkEntities = true);
    
    // Gravity zones (pour planète, etc.)
    void addGravityZone(const simd::float3& center, float radius, const simd::float3& direction);
    
private:
    MTL::Device* m_device;
    MTL::Library* m_library;
    
    // Physics parameters
    simd::float3 m_gravity;
    float m_airDrag;
    float m_groundDrag;
    
    // Entities
    std::vector<PhysicsEntity> m_entities;
    PlayerPhysics m_player;
    VehiclePhysics m_vehicle;
    
    // References to other systems
    TerrainSystem* m_terrain;
    RMDLGrid* m_voxelGrid;
    
    // GPU resources
    MTL::Buffer* m_entityBuffer;
    MTL::Buffer* m_uniformsBuffer;
    MTL::ComputePipelineState* m_physicsPipeline;
    MTL::ComputePipelineState* m_collisionPipeline;
    
    // Helpers
    void integrateEntity(PhysicsEntity& entity, float deltaTime);
    void resolveTerrainCollision(PhysicsEntity& entity);
    void resolveVoxelCollision(PhysicsEntity& entity);
    void resolveEntityCollisions();
    
    void createComputePipelines();
    void uploadToGPU();
    void downloadFromGPU();
    
    // Character controller specific
    simd::float3 calculatePlayerMovement(const simd::float3& inputDir, bool run);
    void handlePlayerJump(bool jump);
    void updatePlayerGrounded();
};

// Quaternion utilities for rotation
namespace QuaternionMath {
    inline simd::float4 multiply(const simd::float4& q1, const simd::float4& q2) {
        return {
            q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
        };
    }
    
    inline simd::float4 fromAxisAngle(const simd::float3& axis, float angle) {
        float halfAngle = angle * 0.5f;
        float s = sinf(halfAngle);
        return {
            axis.x * s,
            axis.y * s,
            axis.z * s,
            cosf(halfAngle)
        };
    }
    
    inline simd::float3 rotate(const simd::float3& v, const simd::float4& q) {
        simd::float4 qv = {v.x, v.y, v.z, 0.0f};
        simd::float4 qConj = {-q.x, -q.y, -q.z, q.w};
        simd::float4 temp = multiply(q, qv);
        simd::float4 result = multiply(temp, qConj);
        return {result.x, result.y, result.z};
    }
    
    inline simd::float4 normalize(const simd::float4& q) {
        float len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        if (len < 0.0001f) return {0, 0, 0, 1};
        return q / len;
    }
}

#endif /* RMDLPhysics_hpp */
