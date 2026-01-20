//
//  RMDLNeedNasa.cpp
//  Spammy
//
//  Created by Rémy on 19/01/2026.
//

#include "RMDLNeedNasa.hpp"

namespace NASAAtTheHelm {

void CommandBlockMeshGenerator::generate()
{
    vertices.clear();
    indices.clear();
    
    const float SIZE = 1.0f;
    const float HALF = SIZE * 0.5f;
    
    // Cube biseauté avec coins arrondis
    generateBeveledBox(HALF * 0.9f, HALF * 0.85f, HALF * 0.9f, 0.08f, primaryColor);
    
    // ANNEAU CENTRAL - Bande décorative au milieu
    generateRing({0, 0, 0}, 0.42f, 0.38f, 0.08f, 24, secondaryColor);
    
    // COCKPIT/VISIÈRE - Dôme sur le dessus
    generateCockpit({0, HALF * 0.65f, 0.1f}, 0.25f, 0.15f, glowColor);

    // ANTENNE - Sur le dessus
    generateAntenna({0.2f, HALF * 0.85f, -0.15f}, accentColor);
    
    // POINTS D'ATTACHE - Connecteurs visibles sur chaque face
    const float attachOffset = HALF * 0.95f;
    generateAttachmentConnector({+attachOffset, 0, 0}, {1, 0, 0});
    generateAttachmentConnector({-attachOffset, 0, 0}, {-1, 0, 0});
    generateAttachmentConnector({0, +attachOffset, 0}, {0, 1, 0});
    generateAttachmentConnector({0, -attachOffset, 0}, {0, -1, 0});
    generateAttachmentConnector({0, 0, +attachOffset}, {0, 0, 1});
    generateAttachmentConnector({0, 0, -attachOffset}, {0, 0, -1});
    
    // DÉTAILS - Lignes, grilles, etc
    generateTechLines();
    generateVentGrilles();
    generatePowerCore({0, 0, 0}, 0.15f);
}

void CommandBlockMeshGenerator::generateBeveledBox(float hx, float hy, float hz, float bevel, simd_float4 color)
{
    // Sommets avec biseaux
    const float bx = hx - bevel;
    const float by = hy - bevel;
    const float bz = hz - bevel;
    
    // Face +Y (dessus)
    addQuad({{-bx, hy, -bz}, {0, 1, 0}, {0, 0}, color},
            {{+bx, hy, -bz}, {0, 1, 0}, {1, 0}, color},
            {{+bx, hy, +bz}, {0, 1, 0}, {1, 1}, color},
            {{-bx, hy, +bz}, {0, 1, 0}, {0, 1}, color});
    
    // Face -Y (dessous)
    addQuad({{-bx, -hy, +bz}, {0, -1, 0}, {0, 0}, color},
            {{+bx, -hy, +bz}, {0, -1, 0}, {1, 0}, color},
            {{+bx, -hy, -bz}, {0, -1, 0}, {1, 1}, color},
            {{-bx, -hy, -bz}, {0, -1, 0}, {0, 1}, color});
    
    // Face +X (droite)
    addQuad({{hx, -by, -bz}, {1, 0, 0}, {0, 0}, color},
            {{hx, -by, +bz}, {1, 0, 0}, {1, 0}, color},
            {{hx, +by, +bz}, {1, 0, 0}, {1, 1}, color},
            {{hx, +by, -bz}, {1, 0, 0}, {0, 1}, color});
    
    // Face -X (gauche)
    addQuad({{-hx, -by, +bz}, {-1, 0, 0}, {0, 0}, color},
            {{-hx, -by, -bz}, {-1, 0, 0}, {1, 0}, color},
            {{-hx, +by, -bz}, {-1, 0, 0}, {1, 1}, color},
            {{-hx, +by, +bz}, {-1, 0, 0}, {0, 1}, color});
    
    // Face +Z (avant)
    addQuad({{+bx, -by, hz}, {0, 0, 1}, {0, 0}, color},
            {{-bx, -by, hz}, {0, 0, 1}, {1, 0}, color},
            {{-bx, +by, hz}, {0, 0, 1}, {1, 1}, color},
            {{+bx, +by, hz}, {0, 0, 1}, {0, 1}, color});
    
    // Face -Z (arrière)
    addQuad({{-bx, -by, -hz}, {0, 0, -1}, {0, 0}, color},
            {{+bx, -by, -hz}, {0, 0, -1}, {1, 0}, color},
            {{+bx, +by, -hz}, {0, 0, -1}, {1, 1}, color},
            {{-bx, +by, -hz}, {0, 0, -1}, {0, 1}, color});
    
    // Biseaux (arêtes chanfreinées)
    generateEdgeBevels(bx, by, bz, hx, hy, hz, bevel, color);
}

void CommandBlockMeshGenerator::generateEdgeBevels(float bx, float by, float bz, float hx, float hy, float hz, float bevel, simd_float4 color)
{
    simd_float4 bevelColor = color * 0.9f;
    bevelColor.w = 1.0f;
    
    // Arêtes verticales (4)
    simd_float3 n1 = simd_normalize(simd_make_float3(1, 0, 1));
    addQuad({{hx, -by, bz}, n1, {0, 0}, bevelColor},
            {{bx, -by, hz}, n1, {1, 0}, bevelColor},
            {{bx, +by, hz}, n1, {1, 1}, bevelColor},
            {{hx, +by, bz}, n1, {0, 1}, bevelColor});
    
    simd_float3 n2 = simd_normalize(simd_make_float3(-1, 0, 1));
    addQuad({{-bx, -by, hz}, n2, {0, 0}, bevelColor},
            {{-hx, -by, bz}, n2, {1, 0}, bevelColor},
            {{-hx, +by, bz}, n2, {1, 1}, bevelColor},
            {{-bx, +by, hz}, n2, {0, 1}, bevelColor});
    
    simd_float3 n3 = simd_normalize(simd_make_float3(1, 0, -1));
    addQuad({{bx, -by, -hz}, n3, {0, 0}, bevelColor},
            {{hx, -by, -bz}, n3, {1, 0}, bevelColor},
            {{hx, +by, -bz}, n3, {1, 1}, bevelColor},
            {{bx, +by, -hz}, n3, {0, 1}, bevelColor});
    
    simd_float3 n4 = simd_normalize(simd_make_float3(-1, 0, -1));
    addQuad({{-hx, -by, -bz}, n4, {0, 0}, bevelColor},
            {{-bx, -by, -hz}, n4, {1, 0}, bevelColor},
            {{-bx, +by, -hz}, n4, {1, 1}, bevelColor},
            {{-hx, +by, -bz}, n4, {0, 1}, bevelColor});
}

void CommandBlockMeshGenerator::generateRing(simd_float3 center, float outerR, float innerR, float height, int segments, simd_float4 color)
{
    float halfH = height * 0.5f;
    
    for (int i = 0; i < segments; i++)
    {
        float a0 = (float(i) / segments) * M_PI * 2.0f;
        float a1 = (float(i + 1) / segments) * M_PI * 2.0f;
        
        float c0 = cosf(a0), s0 = sinf(a0);
        float c1 = cosf(a1), s1 = sinf(a1);
        
        // Face extérieure
        simd_float3 n0 = {c0, 0, s0};
        simd_float3 n1 = {c1, 0, s1};
        
        addQuad({center + simd_make_float3(outerR * c0, -halfH, outerR * s0), n0, {0, 0}, color},
                {center + simd_make_float3(outerR * c1, -halfH, outerR * s1), n1, {1, 0}, color},
                {center + simd_make_float3(outerR * c1, +halfH, outerR * s1), n1, {1, 1}, color},
                {center + simd_make_float3(outerR * c0, +halfH, outerR * s0), n0, {0, 1}, color});
        
        // Face supérieure
        addQuad({center + simd_make_float3(innerR * c0, halfH, innerR * s0), {0, 1, 0}, {0, 0}, color},
                {center + simd_make_float3(innerR * c1, halfH, innerR * s1), {0, 1, 0}, {1, 0}, color},
                {center + simd_make_float3(outerR * c1, halfH, outerR * s1), {0, 1, 0}, {1, 1}, color},
                {center + simd_make_float3(outerR * c0, halfH, outerR * s0), {0, 1, 0}, {0, 1}, color});
    }
}

void CommandBlockMeshGenerator::generateCockpit(simd_float3 center, float width, float height, simd_float4 color)
{
    const int segments = 12;
    const int rings = 6;
    
    for (int ring = 0; ring < rings; ring++)
    {
        float phi0 = (float(ring) / rings) * M_PI * 0.5f;
        float phi1 = (float(ring + 1) / rings) * M_PI * 0.5f;
        
        float y0 = sinf(phi0) * height;
        float y1 = sinf(phi1) * height;
        float r0 = cosf(phi0) * width;
        float r1 = cosf(phi1) * width;
        
        for (int seg = 0; seg < segments; seg++)
        {
            float theta0 = (float(seg) / segments) * M_PI * 2.0f;
            float theta1 = (float(seg + 1) / segments) * M_PI * 2.0f;
            
            simd_float3 p00 = center + simd_make_float3(r0 * cosf(theta0), y0, r0 * sinf(theta0));
            simd_float3 p10 = center + simd_make_float3(r0 * cosf(theta1), y0, r0 * sinf(theta1));
            simd_float3 p01 = center + simd_make_float3(r1 * cosf(theta0), y1, r1 * sinf(theta0));
            simd_float3 p11 = center + simd_make_float3(r1 * cosf(theta1), y1, r1 * sinf(theta1));
            
            simd_float3 n00 = simd_normalize(p00 - center);
            simd_float3 n10 = simd_normalize(p10 - center);
            simd_float3 n01 = simd_normalize(p01 - center);
            simd_float3 n11 = simd_normalize(p11 - center);
            
            addQuad({p00, n00, {0, 0}, color},
                    {p10, n10, {1, 0}, color},
                    {p11, n11, {1, 1}, color},
                    {p01, n01, {0, 1}, color});
        }
    }
}

void CommandBlockMeshGenerator::generateAntenna(simd_float3 base, simd_float4 color)
{
    const float baseR = 0.03f;
    const float height = 0.12f;
    const int segments = 8;
    
    // Tige cylindrique
    for (int i = 0; i < segments; i++)
    {
        float a0 = (float(i) / segments) * M_PI * 2.0f;
        float a1 = (float(i + 1) / segments) * M_PI * 2.0f;
        
        simd_float3 n0 = {cosf(a0), 0, sinf(a0)};
        simd_float3 n1 = {cosf(a1), 0, sinf(a1)};
        
        addQuad({base + simd_make_float3(baseR * n0.x, 0, baseR * n0.z), n0, {0, 0}, color},
                {base + simd_make_float3(baseR * n1.x, 0, baseR * n1.z), n1, {1, 0}, color},
                {base + simd_make_float3(baseR * n1.x * 0.5f, height, baseR * n1.z * 0.5f), n1, {1, 1}, color},
                {base + simd_make_float3(baseR * n0.x * 0.5f, height, baseR * n0.z * 0.5f), n0, {0, 1}, color});
    }
    // Sphère au sommet
    generateSphere(base + simd_make_float3(0, height + 0.02f, 0), 0.025f, 6, glowColor);
}

void CommandBlockMeshGenerator::generateAttachmentConnector(simd_float3 position, simd_float3 normal)
{
    const float size = 0.12f;
    const float depth = 0.03f;
    
    // Créer une base orthonormale
    simd_float3 up = fabsf(normal.y) > 0.9f ? simd_make_float3(1, 0, 0) : simd_make_float3(0, 1, 0);
    simd_float3 right = simd_normalize(simd_cross(up, normal));
    up = simd_cross(normal, right);
    
    // Hexagone
    const int sides = 6;
    simd_float3 center = position;
    
    // Face avant (hexagonale)
    uint32_t centerIdx = (uint32_t)vertices.size();
    vertices.push_back({center, normal, {0.5f, 0.5f}, accentColor});
    
    for (int i = 0; i < sides; i++) {
        float a0 = (float(i) / sides) * M_PI * 2.0f;
        float a1 = (float(i + 1) / sides) * M_PI * 2.0f;
        
        simd_float3 p0 = center + (right * cosf(a0) + up * sinf(a0)) * size;
        simd_float3 p1 = center + (right * cosf(a1) + up * sinf(a1)) * size;
        
        uint32_t idx = (uint32_t)vertices.size();
        vertices.push_back({p0, normal, {cosf(a0) * 0.5f + 0.5f, sinf(a0) * 0.5f + 0.5f}, accentColor});
        vertices.push_back({p1, normal, {cosf(a1) * 0.5f + 0.5f, sinf(a1) * 0.5f + 0.5f}, accentColor});
        
        indices.push_back(centerIdx);
        indices.push_back(idx);
        indices.push_back(idx + 1);
    }
    
    // Bord intérieur (petit cercle lumineux)
    generateRing(center + normal * 0.001f, size * 0.4f, size * 0.3f,
                 0.01f, 12, glowColor);
}

void CommandBlockMeshGenerator::generateTechLines()
{
    simd_float4 lineColor = secondaryColor * 1.2f;
    lineColor.w = 1.0f;
    
    const float offset = 0.001f;  // Légèrement au-dessus de la surface
    const float lineWidth = 0.015f;
    const float hx = 0.45f;
    
    // Lignes horizontales sur les côtés
    float lineY[] = {0.2f, -0.2f};
    for (float y : lineY)
    {
        // Face +X
        addQuad({{hx + offset, y - lineWidth, -0.3f}, {1, 0, 0}, {0, 0}, lineColor},
                {{hx + offset, y - lineWidth, +0.3f}, {1, 0, 0}, {1, 0}, lineColor},
                {{hx + offset, y + lineWidth, +0.3f}, {1, 0, 0}, {1, 1}, lineColor},
                {{hx + offset, y + lineWidth, -0.3f}, {1, 0, 0}, {0, 1}, lineColor});
        
        // Face -X
        addQuad({{-hx - offset, y - lineWidth, +0.3f}, {-1, 0, 0}, {0, 0}, lineColor},
                {{-hx - offset, y - lineWidth, -0.3f}, {-1, 0, 0}, {1, 0}, lineColor},
                {{-hx - offset, y + lineWidth, -0.3f}, {-1, 0, 0}, {1, 1}, lineColor},
                {{-hx - offset, y + lineWidth, +0.3f}, {-1, 0, 0}, {0, 1}, lineColor});
    }
}

void CommandBlockMeshGenerator::generateVentGrilles()
{
    simd_float4 grillColor = {0.05f, 0.05f, 0.08f, 1.0f};
    
    // Grille arrière
    const float offset = 0.001f;
    const float hz = 0.45f;
    const int slats = 4;
    const float slatWidth = 0.02f;
    const float slatSpacing = 0.06f;
    const float grillWidth = 0.2f;
    
    for (int i = 0; i < slats; i++)
    {
        float y = -0.15f + i * slatSpacing;
        
        addQuad({{-grillWidth, y - slatWidth, -hz - offset}, {0, 0, -1}, {0, 0}, grillColor},
                {{+grillWidth, y - slatWidth, -hz - offset}, {0, 0, -1}, {1, 0}, grillColor},
                {{+grillWidth, y + slatWidth, -hz - offset}, {0, 0, -1}, {1, 1}, grillColor},
                {{-grillWidth, y + slatWidth, -hz - offset}, {0, 0, -1}, {0, 1}, grillColor});
    }
}

void CommandBlockMeshGenerator::generatePowerCore(simd_float3 center, float radius)
{
    // Juste une indication visuelle - sera animé dans le shader
    // Petit icosaèdre lumineux au centre
    generateSphere(center, radius * 0.3f, 8, glowColor);
}

void CommandBlockMeshGenerator::generateSphere(simd_float3 center, float radius, int detail, simd_float4 color)
{
    const int segments = detail * 2;
    const int rings = detail;
    
    for (int ring = 0; ring < rings; ring++)
    {
        float phi0 = (float(ring) / rings) * M_PI;
        float phi1 = (float(ring + 1) / rings) * M_PI;
        
        for (int seg = 0; seg < segments; seg++)
        {
            float theta0 = (float(seg) / segments) * M_PI * 2.0f;
            float theta1 = (float(seg + 1) / segments) * M_PI * 2.0f;
            
            simd_float3 n00 = {sinf(phi0) * cosf(theta0), cosf(phi0), sinf(phi0) * sinf(theta0)};
            simd_float3 n10 = {sinf(phi0) * cosf(theta1), cosf(phi0), sinf(phi0) * sinf(theta1)};
            simd_float3 n01 = {sinf(phi1) * cosf(theta0), cosf(phi1), sinf(phi1) * sinf(theta0)};
            simd_float3 n11 = {sinf(phi1) * cosf(theta1), cosf(phi1), sinf(phi1) * sinf(theta1)};
            
            addQuad({center + n00 * radius, n00, {0, 0}, color},
                    {center + n10 * radius, n10, {1, 0}, color},
                    {center + n11 * radius, n11, {1, 1}, color},
                    {center + n01 * radius, n01, {0, 1}, color});
        }
    }
}

void CommandBlockMeshGenerator::addQuad(BlockVertex v0, BlockVertex v1, BlockVertex v2, BlockVertex v3)
{
    uint32_t baseIdx = (uint32_t)vertices.size();
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    
    // Deux triangles
    indices.push_back(baseIdx + 0);
    indices.push_back(baseIdx + 1);
    indices.push_back(baseIdx + 2);
    
    indices.push_back(baseIdx + 0);
    indices.push_back(baseIdx + 2);
    indices.push_back(baseIdx + 3);
}
}
