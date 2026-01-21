//
//  RMDLPhysics.cpp
//  Spammy
//
//  Created by Rémy on 21/01/2026.
//

#include "RMDLPhysics.hpp"

#include <cmath>

CelestialBody::CelestialBody(Type type, float mass, float radius, simd::float3 position)
    : m_type(type)
    , m_gravityStrength(9.81f)
{
    m_body.position = position;
    m_body.velocity = simd::float3{0, 0, 0};
    m_body.angularVelocity = simd::float3{0, 0, 0};
    m_body.rotation = simd::quatf{0, 0, 0, 1}; // Identity
    m_body.mass = mass;
    m_body.radius = radius;
    m_body.isStatic = false;
}

void CelestialBody::update(float dt) {
    if (m_body.isStatic) return;
    
    // Intégration position
    m_body.position += m_body.velocity * dt;
    
    // Intégration rotation (simplified)
    if (simd::length(m_body.angularVelocity) > 0.0001f) {
        float angle = simd::length(m_body.angularVelocity) * dt;
        simd::float3 axis = simd::normalize(m_body.angularVelocity);
        simd::quatf deltaRot = simd::quatf{
            axis.x * sinf(angle * 0.5f),
            axis.y * sinf(angle * 0.5f),
            axis.z * sinf(angle * 0.5f),
            cosf(angle * 0.5f)
        };
        m_body.rotation = simd::normalize(deltaRot * m_body.rotation);
    }
}

void CelestialBody::applyForce(simd::float3 force) {
    if (m_body.isStatic || m_body.mass <= 0.0f) return;
    m_body.velocity += force / m_body.mass;
}

simd::float3 CelestialBody::getGravityAt(simd::float3 point) const {
    simd::float3 dir = m_body.position - point;
    float dist = simd::length(dir);
    
    if (dist < 0.1f) return simd::float3{0, 0, 0};
    
    // Gravité simplifiée: g * (R/r)² vers le centre
    float surfaceDist = dist / m_body.radius;
    float strength = m_gravityStrength / (surfaceDist * surfaceDist);
    
    // Clamp pour éviter les singularités
    strength = fminf(strength, m_gravityStrength * 10.0f);
    
    return simd::normalize(dir) * strength;
}

void CelestialBody::applyGravityTo(RigidBody& other) const {
    simd::float3 gravity = getGravityAt(other.position);
    other.velocity += gravity; // Multiplier par dt dans la boucle principale
}

bool CelestialBody::isOnSurface(simd::float3 point, float tolerance) const {
    float dist = simd::length(point - m_body.position);
    return fabsf(dist - m_body.radius) < tolerance;
}

simd::float3 CelestialBody::getSurfaceNormal(simd::float3 point) const {
    return simd::normalize(point - m_body.position);
}

// Planet
bool Planet::isInAtmosphere(simd::float3 point) const {
    float dist = simd::length(point - m_body.position);
    return dist < m_atmosphereRadius && dist > m_body.radius;
}

// Moon
void Moon::setOrbit(float radius, float speed) {
    m_orbitRadius = radius;
    m_orbitSpeed = speed;
}

void Moon::updateOrbit(float dt) {
    if (!m_parent) return;
    
    m_orbitAngle += m_orbitSpeed * dt;
    
    simd::float3 parentPos = m_parent->getPosition();
    m_body.position = parentPos + simd::float3{
        cosf(m_orbitAngle) * m_orbitRadius,
        0.0f,
        sinf(m_orbitAngle) * m_orbitRadius
    };
}

// Sea
bool Sea::isSubmerged(simd::float3 point) const {
    return point.y < m_waterLevel;
}

float Sea::getDepth(simd::float3 point) const {
    return m_waterLevel - point.y;
}
