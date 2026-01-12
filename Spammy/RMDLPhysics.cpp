//
//  RMDLPhysics.cpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include "RMDLPhysics.hpp"

PhysicsSystem::PhysicsSystem(MTL::Device* device, MTL::Library* library)
    : m_device(device->retain())
    , m_library(library->retain())
    , m_gravity{0.0f, -20.0f, 0.0f}
    , m_airDrag(0.01f)
    , m_groundDrag(0.8f)
    , m_terrain(nullptr)
    , m_voxelGrid(nullptr)
{
    // Allocate GPU buffers
    m_entityBuffer = m_device->newBuffer(1024 * sizeof(PhysicsEntity),
                                        MTL::ResourceStorageModeShared);
    m_uniformsBuffer = m_device->newBuffer(sizeof(PhysicsUniforms),
                                          MTL::ResourceStorageModeShared);
    
    // Initialize player at spawn
    m_player.entity.position = {0.0f, 70.0f, 0.0f};
    m_player.entity.affectedByGravity = true;
    
    // Initialize vehicle
    m_vehicle.entity.position = {0.0f, 65.0f, 0.0f};
    m_vehicle.entity.mass = 500.0f;
    m_vehicle.entity.radius = 2.0f;
    
    createComputePipelines();
}

PhysicsSystem::~PhysicsSystem() {
    m_entityBuffer->release();
    m_uniformsBuffer->release();
    
    if (m_physicsPipeline) m_physicsPipeline->release();
    if (m_collisionPipeline) m_collisionPipeline->release();
    
    m_library->release();
    m_device->release();
}

void PhysicsSystem::createComputePipelines() {
    // Physics integration compute shader
    MTL::Function* physicsFunc = m_library->newFunction(
        NS::String::string("physicsIntegration", NS::UTF8StringEncoding));
    
    if (physicsFunc) {
        NS::Error* error = nullptr;
        m_physicsPipeline = m_device->newComputePipelineState(physicsFunc, &error);
        if (error) {
            printf("Physics pipeline error: %s\n", error->localizedDescription()->utf8String());
        }
        physicsFunc->release();
    }
    
    // Collision detection compute shader
    MTL::Function* collisionFunc = m_library->newFunction(
        NS::String::string("collisionDetection", NS::UTF8StringEncoding));
    
    if (collisionFunc) {
        NS::Error* error = nullptr;
        m_collisionPipeline = m_device->newComputePipelineState(collisionFunc, &error);
        collisionFunc->release();
    }
}

void PhysicsSystem::update(float deltaTime) {
    // Update player
    updatePlayer(deltaTime);
    
    // Update vehicle
    updateVehicle(deltaTime);
    
    // Update all entities
    for (auto& entity : m_entities) {
        if (!entity.isStatic) {
            integrateEntity(entity, deltaTime);
            resolveTerrainCollision(entity);
            resolveVoxelCollision(entity);
        }
    }
    
    // Resolve entity-entity collisions
    resolveEntityCollisions();
}

void PhysicsSystem::updateGPU(float deltaTime) {
    if (!m_physicsPipeline) return;
    
    // Upload to GPU
    uploadToGPU();
    
    // Setup uniforms
    PhysicsUniforms uniforms;
    uniforms.gravity = m_gravity;
    uniforms.deltaTime = deltaTime;
    uniforms.airDrag = m_airDrag;
    uniforms.groundDrag = m_groundDrag;
    uniforms.entityCount = (uint32_t)m_entities.size();
    
    memcpy(m_uniformsBuffer->contents(), &uniforms, sizeof(PhysicsUniforms));
    
    // Execute compute shader
    MTL::CommandQueue* queue = m_device->newCommandQueue();
    MTL::CommandBuffer* commandBuffer = queue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    
    encoder->setComputePipelineState(m_physicsPipeline);
    encoder->setBuffer(m_entityBuffer, 0, 0);
    encoder->setBuffer(m_uniformsBuffer, 0, 1);
    
    MTL::Size gridSize = MTL::Size::Make(m_entities.size(), 1, 1);
    NS::UInteger threadGroupSize = m_physicsPipeline->maxTotalThreadsPerThreadgroup();
    if (threadGroupSize > m_entities.size()) threadGroupSize = m_entities.size();
    MTL::Size threadgroupSize = MTL::Size::Make(threadGroupSize, 1, 1);
    
    encoder->dispatchThreads(gridSize, threadgroupSize);
    encoder->endEncoding();
    
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
    
    // Download results
    downloadFromGPU();
    
    queue->release();
}

void PhysicsSystem::integrateEntity(PhysicsEntity& entity, float deltaTime) {
    if (entity.isStatic) return;
    
    // Apply gravity
    if (entity.affectedByGravity) {
        entity.acceleration += m_gravity;
    }
    
    // Apply drag
    float drag = entity.onGround ? m_groundDrag : m_airDrag;
    entity.velocity *= (1.0f - drag * deltaTime);
    
    // Integrate velocity
    entity.velocity += entity.acceleration * deltaTime;
    
    // Integrate position
    entity.position += entity.velocity * deltaTime;
    
    // Integrate rotation (angular velocity)
    if (simd::length(entity.angularVelocity) > 0.001f) {
        float angle = simd::length(entity.angularVelocity) * deltaTime;
        simd::float3 axis = simd::normalize(entity.angularVelocity);
        simd::float4 deltaRot = QuaternionMath::fromAxisAngle(axis, angle);
        entity.rotation = QuaternionMath::multiply(entity.rotation, deltaRot);
        entity.rotation = QuaternionMath::normalize(entity.rotation);
    }
    
    // Reset acceleration
    entity.acceleration = {0, 0, 0};
}

void PhysicsSystem::resolveTerrainCollision(PhysicsEntity& entity) {
    if (!m_terrain) return;
    
    simd::float3 resolved = m_terrain->resolveTerrainCollision(
        entity.position, entity.velocity, entity.radius);
    
    if (resolved.y != entity.position.y) {
        entity.onGround = true;
        
        // Apply bounce
        if (entity.velocity.y < 0) {
            entity.velocity.y *= -entity.restitution;
            if (fabsf(entity.velocity.y) < 0.1f) {
                entity.velocity.y = 0.0f;
            }
        }
        
        // Apply friction
        float frictionForce = entity.friction * m_groundDrag;
        entity.velocity.x *= (1.0f - frictionForce);
        entity.velocity.z *= (1.0f - frictionForce);
    } else {
        entity.onGround = false;
    }
    
    entity.position = resolved;
}

void PhysicsSystem::resolveVoxelCollision(PhysicsEntity& entity) {
    if (!m_voxelGrid) return;
    
    simd::float3 resolved = m_voxelGrid->resolveCollision(
        entity.position, entity.velocity, entity.radius);
    
    if (simd::length(resolved - entity.position) > 1e-5f) {//if (resolved != entity.position) { gpt
        // Collision occurred
        simd::float3 delta = resolved - entity.position;
        float penetration = simd::length(delta);
        
        if (penetration > 0.001f) {
            simd::float3 normal = simd::normalize(delta);
            
            // Reflect velocity
            float dotProduct = simd::dot(entity.velocity, normal);
            if (dotProduct < 0) {
                entity.velocity -= normal * dotProduct * (1.0f + entity.restitution);
            }
            
            // Check if on ground (normal pointing up)
            if (normal.y > 0.7f) {
                entity.onGround = true;
            }
        }
    }
    
    entity.position = resolved;
}

void PhysicsSystem::resolveEntityCollisions() {
    for (size_t i = 0; i < m_entities.size(); i++) {
        for (size_t j = i + 1; j < m_entities.size(); j++) {
            PhysicsEntity& a = m_entities[i];
            PhysicsEntity& b = m_entities[j];
            
            if (a.isStatic && b.isStatic) continue;
            
            simd::float3 delta = b.position - a.position;
            float distance = simd::length(delta);
            float minDist = a.radius + b.radius;
            
            if (distance < minDist && distance > 0.001f) {
                // Collision detected
                simd::float3 normal = delta / distance;
                float penetration = minDist - distance;
                
                // Separate objects
                float totalMass = a.mass + b.mass;
                float ratio = b.mass / totalMass;
                
                if (!a.isStatic) {
                    a.position -= normal * penetration * ratio;
                }
                if (!b.isStatic) {
                    b.position += normal * penetration * (1.0f - ratio);
                }
                
                // Resolve velocities (elastic collision)
                if (!a.isStatic && !b.isStatic) {
                    simd::float3 relVel = b.velocity - a.velocity;
                    float velAlongNormal = simd::dot(relVel, normal);
                    
                    if (velAlongNormal < 0) {
                        float restitution = (a.restitution + b.restitution) * 0.5f;
                        float j = -(1.0f + restitution) * velAlongNormal;
                        j /= (1.0f / a.mass + 1.0f / b.mass);
                        
                        simd::float3 impulse = normal * j;
                        a.velocity -= impulse / a.mass;
                        b.velocity += impulse / b.mass;
                    }
                }
            }
        }
    }
}

void PhysicsSystem::applyPlayerInput(const simd::float3& moveDirection,
                                     bool jump, bool crouch, bool run) {
    m_player.isCrouching = crouch;
    
    // Calculate movement
    simd::float3 movement = calculatePlayerMovement(moveDirection, run);
    
    // Apply movement force (not directly to velocity for better control)
    if (m_player.entity.onGround) {
        m_player.entity.velocity.x = movement.x;
        m_player.entity.velocity.z = movement.z;
    } else {
        // Air control (reduced)
        m_player.entity.velocity.x += movement.x * 0.1f;
        m_player.entity.velocity.z += movement.z * 0.1f;
    }
    
    // Handle jump
    handlePlayerJump(jump);
}

simd::float3 PhysicsSystem::calculatePlayerMovement(const simd::float3& inputDir, bool run) {
    if (simd::length(inputDir) < 0.01f) return {0, 0, 0};
    
    simd::float3 normalizedDir = simd::normalize(inputDir);
    float speed = run ? m_player.runSpeed : m_player.walkSpeed;
    
    if (m_player.isCrouching) {
        speed *= 0.5f;
    }
    
    return normalizedDir * speed;
}

void PhysicsSystem::handlePlayerJump(bool jump) {
    if (jump && m_player.entity.onGround && m_player.canJump && !m_player.isCrouching) {
        m_player.entity.velocity.y = m_player.jumpForce;
        m_player.entity.onGround = false;
        m_player.isJumping = true;
        m_player.canJump = false;
    }
    
    if (!jump) {
        m_player.canJump = true;
    }
}

void PhysicsSystem::updatePlayer(float deltaTime) {
    // Update player entity with physics
    integrateEntity(m_player.entity, deltaTime);
    
    // Terrain collision
    resolveTerrainCollision(m_player.entity);
    
    // Voxel collision
    resolveVoxelCollision(m_player.entity);
    
    // Update grounded state
    updatePlayerGrounded();
    
    // Adjust radius based on crouch
    if (m_player.isCrouching) {
        m_player.entity.radius = m_player.crouchHeight * 0.5f;
    } else {
        m_player.entity.radius = m_player.standHeight * 0.5f;
    }
}

void PhysicsSystem::updatePlayerGrounded() {
    // Additional check for grounded state
    if (m_terrain) {
        float terrainHeight = m_terrain->getHeightAt(
            m_player.entity.position.x, m_player.entity.position.z);
        
        float distToGround = m_player.entity.position.y - terrainHeight;
        
        if (distToGround < m_player.entity.radius + 0.1f) {
            m_player.entity.onGround = true;
        }
    }
}

void PhysicsSystem::applyVehicleInput(const simd::float3& thrust, float turn) {
    // Apply thrust in vehicle's forward direction
    simd::float3 forward = QuaternionMath::rotate({0, 0, -1}, m_vehicle.entity.rotation);
    simd::float3 right = QuaternionMath::rotate({1, 0, 0}, m_vehicle.entity.rotation);
    
    m_vehicle.engineForce = forward * thrust.z * m_vehicle.acceleration;
    m_vehicle.engineForce += right * thrust.x * m_vehicle.acceleration * 0.5f;
    m_vehicle.engineForce.y += thrust.y * m_vehicle.thrustPower; // Vertical thrust
    
    // Apply turning torque
    m_vehicle.torque = {0, turn * m_vehicle.turning, 0};
}

void PhysicsSystem::updateVehicle(float deltaTime) {
    // Apply engine force
    m_vehicle.entity.acceleration += m_vehicle.engineForce / m_vehicle.entity.mass;
    
    // Apply angular velocity from torque
    m_vehicle.entity.angularVelocity += m_vehicle.torque * deltaTime;
    
    // Update physics
    integrateEntity(m_vehicle.entity, deltaTime);
    
    // Collisions
    resolveTerrainCollision(m_vehicle.entity);
    resolveVoxelCollision(m_vehicle.entity);
    
    // Limit max speed
    float speed = simd::length(m_vehicle.entity.velocity);
    if (speed > m_vehicle.maxSpeed) {
        m_vehicle.entity.velocity *= (m_vehicle.maxSpeed / speed);
    }
    
    // Reset forces
    m_vehicle.engineForce = {0, 0, 0};
    m_vehicle.torque = {0, 0, 0};
}

uint32_t PhysicsSystem::addEntity(const PhysicsEntity& entity) {
    m_entities.push_back(entity);
    return (uint32_t)(m_entities.size() - 1);
}

void PhysicsSystem::removeEntity(uint32_t index) {
    if (index < m_entities.size()) {
        m_entities.erase(m_entities.begin() + index);
    }
}

PhysicsEntity* PhysicsSystem::getEntity(uint32_t index) {
    if (index < m_entities.size()) {
        return &m_entities[index];
    }
    return nullptr;
}

bool PhysicsSystem::checkSphereCollision(const simd::float3& pos, float radius,
                                        simd::float3& normal, float& penetration) {
    // Check terrain
    if (m_terrain && m_terrain->checkTerrainCollision(pos, radius)) {
        // Calculate normal and penetration
        float terrainHeight = m_terrain->getHeightAt(pos.x, pos.z);
        penetration = (terrainHeight + radius) - pos.y;
        if (penetration > 0) {
            normal = m_terrain->getNormalAt(pos.x, pos.z);
            return true;
        }
    }
    
    // Check voxels
    if (m_voxelGrid && m_voxelGrid->checkCollision(pos, radius)) {
        // Simplified normal calculation
        normal = {0, 1, 0};
        penetration = radius;
        return true;
    }
    
    return false;
}

simd::float3 PhysicsSystem::resolveSphereCollision(const simd::float3& pos,
                                                   const simd::float3& vel,
                                                   float radius) {
    simd::float3 resolved = pos;
    
    if (m_terrain) {
        resolved = m_terrain->resolveTerrainCollision(pos, vel, radius);
    }
    
    if (m_voxelGrid) {
        resolved = m_voxelGrid->resolveCollision(resolved, vel, radius);
    }
    
    return resolved;
}

PhysicsSystem::RaycastResult PhysicsSystem::raycast(
    const simd::float3& origin, const simd::float3& direction,
    float maxDistance, bool checkTerrain, bool checkVoxels, bool checkEntities)
{
    RaycastResult result;
    result.hit = false;
    result.distance = maxDistance;
    
    // Check voxels first (they're discrete)
    if (checkVoxels && m_voxelGrid) {
        auto voxelHit = m_voxelGrid->raycast(origin, direction, maxDistance);
        if (voxelHit.hit) {
            float dist = simd::length(voxelHit.hitPoint - origin);
            if (dist < result.distance) {
                result.hit = true;
                result.point = voxelHit.hitPoint;
                result.distance = dist;
                // Calculate normal from face index
                int face = voxelHit.faceIndex;
                if (face == 0) result.normal = {-1, 0, 0};
                else if (face == 1) result.normal = {0, -1, 0};
                else result.normal = {0, 0, -1};
            }
        }
    }
    
    // Check terrain (continuous surface)
    if (checkTerrain && m_terrain) {
        // Raymarch through terrain
        float t = 0.0f;
        float stepSize = 0.5f;
        
        while (t < maxDistance && t < result.distance) {
            simd::float3 point = origin + direction * t;
            float terrainHeight = m_terrain->getHeightAt(point.x, point.z);
            
            if (point.y <= terrainHeight) {
                result.hit = true;
                result.point = point;
                result.normal = m_terrain->getNormalAt(point.x, point.z);
                result.distance = t;
                break;
            }
            
            t += stepSize;
        }
    }
    
    // Check entities
    if (checkEntities) {
        for (size_t i = 0; i < m_entities.size(); i++) {
            const PhysicsEntity& entity = m_entities[i];
            
            // Sphere intersection
            simd::float3 oc = origin - entity.position;
            float b = simd::dot(oc, direction);
            float c = simd::dot(oc, oc) - entity.radius * entity.radius;
            float discriminant = b * b - c;
            
            if (discriminant >= 0) {
                float t = -b - sqrtf(discriminant);
                if (t > 0 && t < result.distance) {
                    result.hit = true;
                    result.distance = t;
                    result.point = origin + direction * t;
                    result.normal = simd::normalize(result.point - entity.position);
                    result.entityIndex = (uint32_t)i;
                }
            }
        }
    }
    
    return result;
}

void PhysicsSystem::uploadToGPU() {
    if (!m_entities.empty()) {
        memcpy(m_entityBuffer->contents(), m_entities.data(),
               m_entities.size() * sizeof(PhysicsEntity));
    }
}

void PhysicsSystem::downloadFromGPU() {
    if (!m_entities.empty()) {
        memcpy(m_entities.data(), m_entityBuffer->contents(),
               m_entities.size() * sizeof(PhysicsEntity));
    }
}

void PhysicsSystem::addGravityZone(const simd::float3& center, float radius,
                                  const simd::float3& direction) {
    // TODO: Implement gravity zones for planet biomes
    // This would modify gravity for entities within the zone
}
