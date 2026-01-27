//
//  RMDLPhysics.hpp
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#ifndef RMDLPhysics_hpp
#define RMDLPhysics_hpp

#include <stdio.h>

#include <simd/simd.h>
#include <vector>

struct RigidBody
{
    simd::float3 position;
    simd::float3 velocity;
    simd::float3 angularVelocity;
    simd::quatf rotation;
    float mass;
    float radius;
    bool isStatic;
};

class CelestialBody
{
public:
    enum class Type {
        Planet,
        Moon,
        Sea,
        Asteroid,
        Station
    };
    
    CelestialBody(Type type, float mass, float radius, simd::float3 position);
    virtual ~CelestialBody() = default;
    
    // Physics
    void update(float dt);
    void applyForce(simd::float3 force);
    void applyGravityTo(RigidBody& other) const;
    
    // Gravity field
    simd::float3 getGravityAt(simd::float3 point) const;
    float getGravityStrength() const { return m_gravityStrength; }
    void setGravityStrength(float g) { m_gravityStrength = g; }
    
    // Accessors
    Type getType() const { return m_type; }
    const RigidBody& getBody() const { return m_body; }
    RigidBody& getBody() { return m_body; }
    
    simd::float3 getPosition() const { return m_body.position; }
    simd::quatf getRotation() const { return m_body.rotation; }
    float getRadius() const { return m_body.radius; }
    
    // Surface check
    bool isOnSurface(simd::float3 point, float tolerance = 1.0f) const;
    simd::float3 getSurfaceNormal(simd::float3 point) const;
    
protected:
    Type m_type;
    RigidBody m_body;
    float m_gravityStrength;
    
    static constexpr float G = 6.674e-11f; // Constante gravitationnelle (scaled)
};

class Planet : public CelestialBody {
public:
    Planet(float mass, float radius, simd::float3 position)
        : CelestialBody(Type::Planet, mass, radius, position) {
        m_gravityStrength = 9.81f; // Earth-like
        m_body.isStatic = true;
    }
    
    void setAtmosphereRadius(float r) { m_atmosphereRadius = r; }
    float getAtmosphereRadius() const { return m_atmosphereRadius; }
    bool isInAtmosphere(simd::float3 point) const;
    
private:
    float m_atmosphereRadius = 0.0f;
};

class Moon : public CelestialBody {
public:
    Moon(float mass, float radius, simd::float3 position, CelestialBody* parent = nullptr)
        : CelestialBody(Type::Moon, mass, radius, position)
        , m_parent(parent)
        , m_orbitRadius(0.0f)
        , m_orbitSpeed(0.0f) {
        m_gravityStrength = 1.62f; // Moon-like
    }
    
    void setOrbit(float radius, float speed);
    void updateOrbit(float dt);
    
private:
    CelestialBody* m_parent;
    float m_orbitRadius;
    float m_orbitSpeed;
    float m_orbitAngle = 0.0f;
};

class Sea : public CelestialBody {
public:
    Sea(float radius, simd::float3 position, float waterLevel)
        : CelestialBody(Type::Sea, 0.0f, radius, position)
        , m_waterLevel(waterLevel) {
        m_gravityStrength = 0.0f; // Pas de gravité propre
        m_body.isStatic = true;
    }
    
    float getWaterLevel() const { return m_waterLevel; }
    bool isSubmerged(simd::float3 point) const;
    float getDepth(simd::float3 point) const;
    
private:
    float m_waterLevel;
};

#endif /* RMDLPhysics_hpp */
