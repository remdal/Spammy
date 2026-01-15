//
//  RMDLSnow.metal
//  Spammy
//
//  Created by Rémy on 21/12/2025.
//
//
//#include <metal_stdlib>
//using namespace metal;
//
//#include "RMDLMainRenderer_shared.h"
//
//struct SnowParticle
//{
//    float3 position;
//    float lifetime;
//    float3 velocity;
//    float size;
//};
//
//struct SnowVertexOut
//{
//    float4 position [[position]];
//    float2 texCoord;
//    float alpha;
//    float pointSize [[point_size]];
//};
//
//float hash(float3 p) {
//    p = fract(p * 0.3183099 + 0.1);
//    p *= 17.0;
//    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
//}
//
//float noise3D(float3 p) {
//    float3 i = floor(p);
//    float3 f = fract(p);
//    f = f * f * (3.0 - 2.0 * f);
//    
//    return mix(mix(mix(hash(i + float3(0,0,0)), hash(i + float3(1,0,0)), f.x),
//                   mix(hash(i + float3(0,1,0)), hash(i + float3(1,1,0)), f.x), f.y),
//               mix(mix(hash(i + float3(0,0,1)), hash(i + float3(1,0,1)), f.x),
//                   mix(hash(i + float3(0,1,1)), hash(i + float3(1,1,1)), f.x), f.y), f.z);
//}
//
//kernel void snow_update_particles(
//    device SnowParticle* particles [[buffer(0)]],
//    constant RMDLSnowUniforms& uniforms [[buffer(1)]],
//    uint gid [[thread_position_in_grid]])
//{
//    device SnowParticle& particle = particles[gid];
//    
//    particle.lifetime += uniforms.deltaTime;
//    
//    // Turbulence
//    float3 noisePos = particle.position * 0.1 + float3(uniforms.time * 0.1);
//    float3 turbulence = float3(
//        noise3D(noisePos),
//        noise3D(noisePos + float3(123.4, 567.8, 901.2)),
//        noise3D(noisePos + float3(234.5, 678.9, 12.3))
//    ) * 2.0 - 1.0;
//    
//    turbulence *= uniforms.turbulence;
//    
//    // Forces
//    float3 gravity = float3(0.0, -uniforms.fallSpeed, 0.0);
//    float3 wind = normalize(uniforms.windDirection) * uniforms.windStrength;
//    
//    particle.velocity += (gravity + wind + turbulence) * uniforms.deltaTime;
//    particle.velocity *= 0.99;
//    
//    particle.position += particle.velocity * uniforms.deltaTime * uniforms.intensity;
//    
//    // Respawn
//    if (particle.position.y < uniforms.respawnThreshold ||
//        length(particle.position.xz - uniforms.cameraPos.xz) > uniforms.spawnRadius * 1.5)
//    {
//        float angle = hash(float3(gid, uniforms.time, 0.0)) * 2.0 * M_PI_F;
//        float radius = hash(float3(gid, 0.0, uniforms.time)) * uniforms.spawnRadius;
//        
//        particle.position = float3(
//            uniforms.cameraPos.x + cos(angle) * radius,
//            uniforms.spawnHeight + hash(float3(0.0, gid, uniforms.time)) * 20.0,
//            uniforms.cameraPos.z + sin(angle) * radius
//        );
//        
//        particle.velocity = float3(0.0);
//        particle.lifetime = 0.0;
//        particle.size = 0.03 + hash(float3(gid, 123.0, 456.0)) * 0.05;
//    }
//}
//
//vertex SnowVertexOut snow_vertex(
//    device SnowParticle* particles [[buffer(0)]],
//    constant float4x4& viewProj [[buffer(1)]],
//    constant float3& cameraPos [[buffer(2)]],
//    uint vid [[vertex_id]])
//{
//    SnowVertexOut out;
//    
//    device SnowParticle& particle = particles[vid];
//    
//    float4 worldPos = float4(particle.position, 1.0);
//    out.position = viewProj * worldPos;
//    
//    float dist = length(particle.position - cameraPos);
//    float distFade = 1.0 - saturate(dist / 50.0);
//    
//    out.alpha = distFade * saturate(particle.lifetime * 2.0);
//    
//    float pointSize = particle.size * 1000.0 / max(out.position.w, 1.0);
//    out.pointSize = clamp(pointSize, 2.0, 20.0);
//    
//    out.texCoord = float2(0.5);
//    
//    return out;
//}
//
//fragment float4 snow_fragment(
//    SnowVertexOut in [[stage_in]],
//    float2 pointCoord [[point_coord]])
//{
//    float2 centered = pointCoord - 0.5;
//    float dist = length(centered);
//    
//    // Flocon étoile 6 branches
//    float angle = atan2(centered.y, centered.x);
//    float branches = abs(sin(angle * 3.0)) * 0.3 + 0.7;
//    
//    float edge = 1.0 - smoothstep(0.3 * branches, 0.5 * branches, dist);
//    
//    float alpha = edge * in.alpha;
//    
//    if (alpha < 0.01) discard_fragment();
//    
//    float3 color = float3(0.95, 0.97, 1.0);
//    
//    return float4(color, alpha);
//}
