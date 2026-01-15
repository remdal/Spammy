//
//  RMDLWorld.metal
//  Spammy
//
//  Created by Rémy on 05/01/2026.
//

//#include <metal_stdlib>
//using namespace metal;
//
//#include "../RMDLMainRenderer_shared.h"
//
//struct GeometricVertex
//{
//    float3 position [[attribute(0)]];
//    float4 color [[attribute(1)]];
//    float3 normal [[attribute(2)]];
//};
//
//struct GeometricFragmentInput
//{
//    float4 position [[position]];
//    float4 color;
//    float3 normal;
//    float3 worldPosition;
//};
//
//vertex GeometricFragmentInput geometric_vertex(GeometricVertex in [[stage_in]],
//                                               constant RMDLCameraUniforms& camera [[buffer(1)]])
//{
//    GeometricFragmentInput out;
//    float4 worldPos = float4(in.position, 1.0);
//    out.position = camera.viewProjectionMatrix * worldPos;
//    out.worldPosition = in.position;
//    out.color = in.color;
//    out.normal = in.normal;
//    return out;
//}
//
//fragment float4 geometric_fragment(GeometricFragmentInput in [[stage_in]],
//                                   constant RMDLCameraUniforms& camera [[buffer(1)]])
//{
//    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
//    float3 normal = normalize(in.normal);
//    
//    float diffuse = max(dot(normal, lightDir), 0.0);
//    
//    float3 viewDir = normalize(camera.position - in.worldPosition);
//    float3 halfDir = normalize(lightDir + viewDir);
//    float specular = pow(max(dot(normal, halfDir), 0.0), 32.0);
//    
//    float ambient = 0.2;
//    float lighting = ambient + diffuse * 0.7 + specular * 0.3;
//    
//    float distance = length(in.worldPosition - camera.position);
//    float fogStart = 80.0;
//    float fogEnd = 200.0;
//    float fogFactor = smoothstep(fogStart, fogEnd, distance);
//    float4 fogColor = float4(0.5, 0.7, 0.9, 1.0);
//    
//    float4 litColor = in.color * lighting;
//    return mix(litColor, fogColor, fogFactor);
//}
//
//struct IcosphereParams
//{
//    float3 center;
//    float radius;
//    uint subdivisions;
//    float4 color;
//};
//
//struct PrismParams
//{
//    float3 baseCenter;
//    float3 direction;
//    float height;
//    float baseRadius;
//    float4 color;
//};
//
//kernel void generateIcosphereGeometry(device GeometricVertex* vertices [[buffer(0)]],
//                                      device uint32_t* indices [[buffer(1)]],
//                                      device atomic_uint* vertexCounter [[buffer(2)]],
//                                      device atomic_uint* indexCounter [[buffer(3)]],
//                                      constant IcosphereParams& params [[buffer(4)]],
//                                      uint gid [[thread_position_in_grid]])
//{
//    if (gid >= 20) return;
//    
//    float t = (1.0 + sqrt(5.0)) / 2.0;
//    
//    float3 basePositions[12] = {
//        normalize(float3(-1, t, 0)),
//        normalize(float3(1, t, 0)),
//        normalize(float3(-1, -t, 0)),
//        normalize(float3(1, -t, 0)),
//        normalize(float3(0, -1, t)),
//        normalize(float3(0, 1, t)),
//        normalize(float3(0, -1, -t)),
//        normalize(float3(0, 1, -t)),
//        normalize(float3(t, 0, -1)),
//        normalize(float3(t, 0, 1)),
//        normalize(float3(-t, 0, -1)),
//        normalize(float3(-t, 0, 1))
//    };
//    
//    uint3 baseIndices[20] = {
//        uint3(0, 11, 5), uint3(0, 5, 1), uint3(0, 1, 7), uint3(0, 7, 10), uint3(0, 10, 11),
//        uint3(1, 5, 9), uint3(5, 11, 4), uint3(11, 10, 2), uint3(10, 7, 6), uint3(7, 1, 8),
//        uint3(3, 9, 4), uint3(3, 4, 2), uint3(3, 2, 6), uint3(3, 6, 8), uint3(3, 8, 9),
//        uint3(4, 9, 5), uint3(2, 4, 11), uint3(6, 2, 10), uint3(8, 6, 7), uint3(9, 8, 1)
//    };
//    
//    uint3 tri = baseIndices[gid];
//    
//    float3 v0 = params.center + basePositions[tri.x] * params.radius;
//    float3 v1 = params.center + basePositions[tri.y] * params.radius;
//    float3 v2 = params.center + basePositions[tri.z] * params.radius;
//    
//    uint baseVertex = atomic_fetch_add_explicit(vertexCounter, 3, memory_order_relaxed);
//    uint baseIndex = atomic_fetch_add_explicit(indexCounter, 3, memory_order_relaxed);
//    
//    vertices[baseVertex + 0].position = v0;
//    vertices[baseVertex + 0].normal = normalize(v0 - params.center);
//    vertices[baseVertex + 0].color = params.color;
//    
//    vertices[baseVertex + 1].position = v1;
//    vertices[baseVertex + 1].normal = normalize(v1 - params.center);
//    vertices[baseVertex + 1].color = params.color;
//    
//    vertices[baseVertex + 2].position = v2;
//    vertices[baseVertex + 2].normal = normalize(v2 - params.center);
//    vertices[baseVertex + 2].color = params.color;
//    
//    indices[baseIndex + 0] = baseVertex + 0;
//    indices[baseIndex + 1] = baseVertex + 1;
//    indices[baseIndex + 2] = baseVertex + 2;
//}
//
//kernel void generatePrismGeometry(device GeometricVertex* vertices [[buffer(0)]],
//                                  device uint32_t* indices [[buffer(1)]],
//                                  device atomic_uint* vertexCounter [[buffer(2)]],
//                                  device atomic_uint* indexCounter [[buffer(3)]],
//                                  constant PrismParams& params [[buffer(4)]],
//                                  uint gid [[thread_position_in_grid]])
//{
//    if (gid >= 1) return;
//    
//    float3 dir = normalize(params.direction);
//    float3 up = float3(0, 1, 0);
//    float3 right = normalize(cross(up, dir));
//    float3 actualUp = cross(dir, right);
//    
//    float angles[3] = {0.0, 2.0944, 4.18879};
//    float3 baseVerts[3];
//    float3 topVerts[3];
//    
//    for (int i = 0; i < 3; i++) {
//        float x = cos(angles[i]) * params.baseRadius;
//        float z = sin(angles[i]) * params.baseRadius;
//        float3 local = right * x + actualUp * z;
//        baseVerts[i] = params.baseCenter + local;
//        topVerts[i] = baseVerts[i] + dir * params.height;
//    }
//    
//    uint baseVertex = atomic_fetch_add_explicit(vertexCounter, 18, memory_order_relaxed);
//    uint baseIndex = atomic_fetch_add_explicit(indexCounter, 24, memory_order_relaxed);
//    
//    for (int i = 0; i < 3; i++) {
//        vertices[baseVertex + i].position = baseVerts[i];
//        vertices[baseVertex + i].normal = -dir;
//        vertices[baseVertex + i].color = params.color;
//        
//        vertices[baseVertex + 3 + i].position = topVerts[i];
//        vertices[baseVertex + 3 + i].normal = dir;
//        vertices[baseVertex + 3 + i].color = params.color;
//    }
//    
//    indices[baseIndex + 0] = baseVertex + 0;
//    indices[baseIndex + 1] = baseVertex + 2;
//    indices[baseIndex + 2] = baseVertex + 1;
//    
//    indices[baseIndex + 3] = baseVertex + 3;
//    indices[baseIndex + 4] = baseVertex + 4;
//    indices[baseIndex + 5] = baseVertex + 5;
//    
//    for (int i = 0; i < 3; i++) {
//        int next = (i + 1) % 3;
//        int vIdx = 6 + i * 4;
//        int iIdx = 6 + i * 6;
//        
//        float3 edge = topVerts[i] - baseVerts[i];
//        float3 side = baseVerts[next] - baseVerts[i];
//        float3 normal = normalize(cross(edge, side));
//        
//        vertices[baseVertex + vIdx + 0].position = baseVerts[i];
//        vertices[baseVertex + vIdx + 0].normal = normal;
//        vertices[baseVertex + vIdx + 0].color = params.color * 0.9;
//        
//        vertices[baseVertex + vIdx + 1].position = baseVerts[next];
//        vertices[baseVertex + vIdx + 1].normal = normal;
//        vertices[baseVertex + vIdx + 1].color = params.color * 0.9;
//        
//        vertices[baseVertex + vIdx + 2].position = topVerts[i];
//        vertices[baseVertex + vIdx + 2].normal = normal;
//        vertices[baseVertex + vIdx + 2].color = params.color * 0.9;
//        
//        vertices[baseVertex + vIdx + 3].position = topVerts[next];
//        vertices[baseVertex + vIdx + 3].normal = normal;
//        vertices[baseVertex + vIdx + 3].color = params.color * 0.9;
//        
//        indices[baseIndex + iIdx + 0] = baseVertex + vIdx + 0;
//        indices[baseIndex + iIdx + 1] = baseVertex + vIdx + 1;
//        indices[baseIndex + iIdx + 2] = baseVertex + vIdx + 2;
//        
//        indices[baseIndex + iIdx + 3] = baseVertex + vIdx + 1;
//        indices[baseIndex + iIdx + 4] = baseVertex + vIdx + 3;
//        indices[baseIndex + iIdx + 5] = baseVertex + vIdx + 2;
//    }
//}
