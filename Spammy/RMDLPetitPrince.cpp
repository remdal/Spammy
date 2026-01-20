//
//  RMDLPetitPrince.cpp
//  Spammy
//
//  Created by Rémy on 20/01/2026.
//

#include "RMDLPetitPrince.hpp"

namespace NASAAtTheHelm {

VehicleBlock::VehicleBlock(BlockType t, uint32_t id)
: type(t), uniqueID(id)
{
    initializeAttachmentPoints();
}

void VehicleBlock::initializeAttachmentPoints()
{
    attachPoints.clear();
    const float halfSize = 0.5f;
    
    attachPoints.push_back({{+halfSize, 0, 0}, {+1, 0, 0}, false, 0, AttachmentPoint::PosX});
    attachPoints.push_back({{-halfSize, 0, 0}, {-1, 0, 0}, false, 0, AttachmentPoint::NegX});
    attachPoints.push_back({{0, +halfSize, 0}, {0, +1, 0}, false, 0, AttachmentPoint::PosY});
    attachPoints.push_back({{0, -halfSize, 0}, {0, -1, 0}, false, 0, AttachmentPoint::NegY});
    attachPoints.push_back({{0, 0, +halfSize}, {0, 0, +1}, false, 0, AttachmentPoint::PosZ});
    attachPoints.push_back({{0, 0, -halfSize}, {0, 0, -1}, false, 0, AttachmentPoint::NegZ});
}

simd_float3 VehicleBlock::getWorldPosition(const simd_float3& vehiclePos, const simd_quatf& vehicleRot) const
{
    simd_float3 rotated = simd_act(vehicleRot, localPosition);
    return vehiclePos + rotated;
}

simd_float4x4 VehicleBlock::getModelMatrix(const simd_float3& vehiclePos, const simd_quatf& vehicleRot) const
{
    simd_float3 worldPos = getWorldPosition(vehiclePos, vehicleRot);
    simd_quatf worldRot = simd_mul(vehicleRot, localRotation);
    
    // Construire matrice TRS
    simd_float4x4 T = matrix_identity_float4x4;
    T.columns[3] = simd_make_float4(worldPos.x, worldPos.y, worldPos.z, 1.0f);
    
    simd_float4x4 R = simd_matrix4x4(worldRot);
    
    return simd_mul(T, R);
}


CommandBlock::CommandBlock(uint32_t id)
: VehicleBlock(BlockType::Commander, id)
{
    localPosition = {0, 0, 0};
    localRotation = simd_quaternion(0.0f, simd_make_float3(0, 1, 0));
    
    stats = {
        .mass = 50.0f,
        .health = 200.0f,
        .energyConsumption = 1.0f,
        .energyProduction = 0.0f,
        .drag = 0.3f,
        .size = {1.0f, 1.0f, 1.0f}
    };
    currentHealth = stats.health;
}

simd_float3 CommandBlock::getCameraWorldPosition(const simd_float3& vehiclePos, const simd_quatf& vehicleRot) const
{
    // Caméra orbite autour du commandant
    float x = cameraDistance * cosf(cameraPitch) * sinf(cameraYaw);
    float y = cameraDistance * sinf(cameraPitch);
    float z = cameraDistance * cosf(cameraPitch) * cosf(cameraYaw);
    
    simd_float3 localCamPos = {x, y + 1.5f, z};  // +1.5 pour hauteur
    simd_float3 rotatedOffset = simd_act(vehicleRot, localCamPos);
    
    return vehiclePos + rotatedOffset;
}

simd_float3 CommandBlock::getCameraTarget(const simd_float3& vehiclePos) const
{
    return vehiclePos + simd_make_float3(0, 1.0f, 0);  // Regarde légèrement au-dessus
}



}
