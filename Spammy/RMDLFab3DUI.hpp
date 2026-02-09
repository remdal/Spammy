//
//  RMDLFab3DUI.hpp
//  Spammy
//
//  Created by Rémy on 30/01/2026.
//

#ifndef RMDLFab3DUI_hpp
#define RMDLFab3DUI_hpp

#include "RMDLFab3D.hpp"

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <functional>
#include <optional>
#include <cmath>
#include <algorithm>

#include "RMDLMathUtils.hpp"

namespace inventoryWindow {

static constexpr uint32_t FAB_GRID_SIZE = 5;
static constexpr uint32_t FAB_GRID_TOTAL = FAB_GRID_SIZE * FAB_GRID_SIZE * FAB_GRID_SIZE;

using FabItemID = uint32_t;
static constexpr FabItemID FAB_EMPTY = 0;

struct FabSlot
{
    FabItemID itemId = FAB_EMPTY;
    uint32_t quantity = 0;
    
    bool isEmpty() const { return itemId == FAB_EMPTY || quantity == 0; }
    void clear() { itemId = FAB_EMPTY; quantity = 0; }
};

struct texture2d
{
    NS::SharedPtr<MTL::Texture>             one;
    NS::SharedPtr<MTL::Texture>             two;
    NS::SharedPtr<MTL::Texture>             three;
    NS::SharedPtr<MTL::Texture>             four;
    NS::SharedPtr<MTL::Texture>             five;
};

class FabGrid3D
{
public:
    std::array<FabSlot, FAB_GRID_TOTAL> slots;
    
    FabGrid3D() { clear(); }
    
    void clear() {
        for (auto& s : slots) s.clear();
    }
    
    static inline size_t toIndex(uint32_t x, uint32_t y, uint32_t z) {
        return z * (FAB_GRID_SIZE * FAB_GRID_SIZE) + y * FAB_GRID_SIZE + x;
    }
    
    FabSlot& at(uint32_t x, uint32_t y, uint32_t z) {
        return slots[toIndex(x, y, z)];
    }
    
    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const {
        return slots[toIndex(x, y, z)];
    }
    
    bool place(uint32_t x, uint32_t y, uint32_t z, FabItemID id, uint32_t qty = 1) {
        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return false;
        auto& slot = at(x, y, z);
        if (slot.isEmpty()) {
            slot.itemId = id;
            slot.quantity = qty;
            return true;
        }
        if (slot.itemId == id) {
            slot.quantity += qty;
            return true;
        }
        return false;
    }
    
    FabSlot take(uint32_t x, uint32_t y, uint32_t z) {
        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return {};
        FabSlot result = at(x, y, z);
        at(x, y, z).clear();
        return result;
    }
};

struct FabPanelVertex
{
    simd::float2 position;
    simd::float2 uv;
};

// Uniforms pour le panel 2D
struct FabPanelUniforms {
    simd::float2 screenSize;
    simd::float2 panelPosition;   // Centre en pixels
    simd::float2 panelSize;       // Taille totale en pixels
    simd::float4 backgroundColor;
    simd::float4 borderColor;
    simd::float4 headerColor;
    float cornerRadius;
    float borderWidth;
    float headerHeight;
    float time;
    int32_t showInventory;
    float inventoryWidth;
    float padding[2];
};

// Vertex pour les cubes 3D
struct FabCubeVertex {
    simd::float3 position;
    simd::float3 normal;
    simd::float2 uv;
};

// Instance data pour chaque slot
struct FabSlotInstance {
    simd::float3 worldPosition;
    float scale;
    simd::float4 color;
    uint32_t flags;  // bit0=empty, bit1=selected, bit2=hovered
    uint32_t _pad[3];
};

// Uniforms pour le rendu 3D
struct Fab3DUniforms {
    simd::float4x4 viewProjectionMatrix;
    simd::float3 cameraPosition;
    float time;
    simd::float3 lightDirection;
    float gridHalfSize;
    simd::float4 selectionColor;
};

// Vertex pour les axes (position + couleur)
struct FabAxisVertex {
    simd::float3 position;
    simd::float4 color;
};

// ============================================================================
// CLASSE PRINCIPALE
// ============================================================================

class FabPanel3D {
public:
    // ===== Constructeur / Destructeur =====
    FabPanel3D(MTL::Device* device,
               MTL::PixelFormat colorFormat,
               MTL::PixelFormat depthFormat,
               MTL::Library* shaderLibrary, NS::UInteger width, NS::UInteger height);
    ~FabPanel3D();
    
    // ===== État =====
    bool visible = false;
    bool inventoryOpen = true;
    
    void show() { visible = true; }
    void hide() { visible = false; }
    void toggleVisible() { visible = !visible; }
    void toggleInventory() { inventoryOpen = !inventoryOpen; }
    
    // ===== Données =====
    FabGrid3D grid;
    simd::uint3 cursor = {2, 2, 2};  // Position sélectionnée
    
    // ===== Caméra 3D =====
    float cameraYaw = 0.8f;      // Rotation horizontale (radians)
    float cameraPitch = 0.5f;    // Rotation verticale (radians)
    float cameraZoom = 1.0f;     // Niveau de zoom
    
    // ===== Layout =====
    simd::float2 panelCenter = {0.5f, 0.5f};  // Position normalisée (0-1)
    simd::float2 panelPixelSize = {500.f, 500};
    float sidebarWidth = 220.f;
    float headerHeight = 44.f;
    
    // ===== Style visuel =====
    struct {
        simd::float4 panelBg      = {0.05f, 0.06f, 0.09f, 0.95f};
        simd::float4 panelBorder  = {0.30f, 0.40f, 0.60f, 1.0f};
        simd::float4 headerBg     = {0.08f, 0.10f, 0.15f, 1.0f};
        simd::float4 emptySlot    = {0.20f, 0.22f, 0.28f, 0.35f};
        simd::float4 selection    = {1.0f, 0.85f, 0.15f, 1.0f};
        simd::float4 axisX        = {0.95f, 0.15f, 0.15f, 1.0f};
        simd::float4 axisY        = {0.15f, 0.95f, 0.20f, 1.0f};
        simd::float4 axisZ        = {0.20f, 0.45f, 0.95f, 1.0f};
        float cornerRadius = 16.f;
        float borderWidth = 3.f;
        float cubeSize = 0.40f;
        float cubeSpacing = 1.1f;
        float axisLength = 3.8f;
        float axisThickness = 0.07f;
    } style;
    
    // ===== Callback pour couleur des items =====
    std::function<simd::float4(FabItemID)> itemColorCallback = nullptr;
    
    // ===== Input =====
    bool handleMouseDown(simd::float2 mousePos, simd::float2 screenSize, int button);
    bool handleMouseUp(simd::float2 mousePos, simd::float2 screenSize, int button);
    bool handleMouseDrag(simd::float2 mousePos, simd::float2 delta, simd::float2 screenSize, int button);
    bool handleScroll(simd::float2 mousePos, float scrollDelta, simd::float2 screenSize);
    bool handleKey(uint16_t keyCode);
    
    void moveCursor(int dx, int dy, int dz);
    void placeAtCursor(FabItemID id, uint32_t qty = 1);
    FabSlot takeFromCursor();
    
    void update(float deltaTime);
    void render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize);
    void setViewportWindow(NS::UInteger width, NS::UInteger height);
    
private:
    MTL::Device* m_device = nullptr;
    
    // Pipelines
    MTL::RenderPipelineState* m_panelPipeline = nullptr;
    MTL::RenderPipelineState* m_cubePipeline = nullptr;
    MTL::RenderPipelineState* m_axisPipeline = nullptr;
    MTL::RenderPipelineState* m_linePipeline = nullptr;
    
    // Depth states
    MTL::DepthStencilState* m_depthEnabled = nullptr;
    MTL::DepthStencilState* m_depthDisabled = nullptr;
    
    // Buffers
    MTL::Buffer* m_panelQuadVB = nullptr;
    MTL::Buffer* m_panelUniforms = nullptr;
    MTL::Buffer* m_cubeVB = nullptr;
    MTL::Buffer* m_cubeIB = nullptr;
    MTL::Buffer* m_instanceBuffer = nullptr;
    MTL::Buffer* m_3dUniforms = nullptr;
    MTL::Buffer* m_axisVB = nullptr;
    MTL::Buffer* m_gridLinesVB = nullptr;
    
    MTL::Viewport m_viewport3D;
    MTL::ScissorRect m_scissor;
    float aspect = 1;
    
    texture2d texture2D;
    
    // Counts
    uint32_t m_cubeIndexCount = 0;
    uint32_t m_axisVertexCount = 0;
    uint32_t m_gridLineCount = 0;
    
    // État interne
    float m_time = 0.f;
    bool m_draggingPanel = false;
    bool m_draggingCamera = false;
    simd::float2 m_panelDragOffset = {0, 0};
    
    // Construction
    void createBuffers();
    void createPipelines(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* lib);
    void buildCubeGeometry();
    void buildAxisGeometry();
    void buildGridLines();
    
    // Mise à jour
    void updateInstanceBuffer();
    
    // Matrices
    simd::float4x4 computeViewMatrix() const;
    simd::float4x4 computeProjectionMatrix(float aspectRatio) const;
    
    // Couleurs
    simd::float4 getColorForItem(FabItemID id) const;
    
    // Hit testing
    bool isInsidePanel(simd::float2 pos, simd::float2 screenSize) const;
    bool isInsideHeader(simd::float2 pos, simd::float2 screenSize) const;
    bool isInside3DView(simd::float2 pos, simd::float2 screenSize) const;
    
    // Viewport 3D
    void get3DViewportRect(simd::float2 screenSize, float& outX, float& outY, float& outW, float& outH) const;
};

inline FabPanel3D::FabPanel3D(MTL::Device* device,
                              MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat,
                              MTL::Library* shaderLibrary, NS::UInteger width, NS::UInteger height)
: m_device(device)
{
    createBuffers();
    buildCubeGeometry();
    buildAxisGeometry();
    buildGridLines();
    createPipelines(colorFormat, depthFormat, shaderLibrary);
    
    // Ajouter quelques items de test
    grid.place(0, 0, 0, 1);
    grid.place(4, 4, 4, 2);
    grid.place(2, 2, 2, 3);
    grid.place(1, 3, 2, 4);
    grid.place(3, 1, 4, 5);
}

inline FabPanel3D::~FabPanel3D()
{
    if (m_panelPipeline) m_panelPipeline->release();
    if (m_cubePipeline) m_cubePipeline->release();
    if (m_axisPipeline) m_axisPipeline->release();
    if (m_linePipeline) m_linePipeline->release();
    if (m_depthEnabled) m_depthEnabled->release();
    if (m_depthDisabled) m_depthDisabled->release();
    if (m_panelQuadVB) m_panelQuadVB->release();
    if (m_panelUniforms) m_panelUniforms->release();
    if (m_cubeVB) m_cubeVB->release();
    if (m_cubeIB) m_cubeIB->release();
    if (m_instanceBuffer) m_instanceBuffer->release();
    if (m_3dUniforms) m_3dUniforms->release();
    if (m_axisVB) m_axisVB->release();
    if (m_gridLinesVB) m_gridLinesVB->release();
}

inline void FabPanel3D::createBuffers() {
    // Quad pour le panel (2 triangles)
    FabPanelVertex quadVerts[6] = {
        {{-1, -1}, {0, 1}},
        {{ 1, -1}, {1, 1}},
        {{ 1,  1}, {1, 0}},
        {{-1, -1}, {0, 1}},
        {{ 1,  1}, {1, 0}},
        {{-1,  1}, {0, 0}}
    };
    m_panelQuadVB = m_device->newBuffer(quadVerts, sizeof(quadVerts), MTL::ResourceStorageModeShared);
    m_panelUniforms = m_device->newBuffer(sizeof(FabPanelUniforms), MTL::ResourceStorageModeShared);
    
    // Buffers 3D
    m_instanceBuffer = m_device->newBuffer(sizeof(FabSlotInstance) * FAB_GRID_TOTAL, MTL::ResourceStorageModeShared);
    m_3dUniforms = m_device->newBuffer(sizeof(Fab3DUniforms), MTL::ResourceStorageModeShared);
}

inline void FabPanel3D::buildCubeGeometry() {
    std::vector<FabCubeVertex> vertices;
    std::vector<uint16_t> indices;
    
    // Helper pour ajouter une face
    auto addFace = [&](simd::float3 normal, simd::float3 right, simd::float3 up) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        simd::float3 center = normal * 0.5f;
        
        vertices.push_back({center - right * 0.5f - up * 0.5f, normal, {0, 0}});
        vertices.push_back({center + right * 0.5f - up * 0.5f, normal, {1, 0}});
        vertices.push_back({center + right * 0.5f + up * 0.5f, normal, {1, 1}});
        vertices.push_back({center - right * 0.5f + up * 0.5f, normal, {0, 1}});
        
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    };
    
    // 6 faces du cube
    addFace(simd::float3{ 0, 0, 1}, simd::float3{ 1, 0, 0}, simd::float3{0, 1, 0});
    addFace(simd::float3{ 0, 0,-1}, simd::float3{-1, 0, 0}, simd::float3{0, 1, 0});
    addFace(simd::float3{ 0, 1, 0}, simd::float3{ 1, 0, 0}, simd::float3{0, 0,-1});
    addFace(simd::float3{ 0,-1, 0}, simd::float3{ 1, 0, 0}, simd::float3{0, 0, 1});
    addFace(simd::float3{ 1, 0, 0}, simd::float3{ 0, 0,-1}, simd::float3{0, 1, 0});
    addFace(simd::float3{-1, 0, 0}, simd::float3{ 0, 0, 1}, simd::float3{0, 1, 0});
    
    m_cubeVB = m_device->newBuffer(vertices.data(), vertices.size() * sizeof(FabCubeVertex), MTL::ResourceStorageModeShared);
    m_cubeIB = m_device->newBuffer(indices.data(), indices.size() * sizeof(uint16_t), MTL::ResourceStorageModeShared);
    m_cubeIndexCount = static_cast<uint32_t>(indices.size());
}

inline void FabPanel3D::buildAxisGeometry() {
    std::vector<FabAxisVertex> verts;
    
    float len = style.axisLength;
    float thick = style.axisThickness;
    
    // Fonction pour créer un cylindre (axe)
    auto addCylinder = [&](simd::float3 direction, simd::float4 color, float length, float radius) {
        // Trouver des vecteurs perpendiculaires
        simd::float3 perp1, perp2;
        if (std::abs(direction.y) < 0.9f) {
            perp1 = simd::normalize(simd::cross(direction, simd::float3{0, 1, 0}));
        } else {
            perp1 = simd::normalize(simd::cross(direction, simd::float3{1, 0, 0}));
        }
        perp2 = simd::cross(direction, perp1);
        
        constexpr int segments = 8;
        for (int i = 0; i < segments; i++) {
            float angle1 = static_cast<float>(i) / segments * M_PI * 2.0f;
            float angle2 = static_cast<float>(i + 1) / segments * M_PI * 2.0f;
            
            simd::float3 p1 = perp1 * (radius * std::cos(angle1)) + perp2 * (radius * std::sin(angle1));
            simd::float3 p2 = perp1 * (radius * std::cos(angle2)) + perp2 * (radius * std::sin(angle2));
            simd::float3 endOffset = direction * length;
            
            // Face latérale (2 triangles)
            verts.push_back({p1, color});
            verts.push_back({p2, color});
            verts.push_back({p2 + endOffset, color});
            
            verts.push_back({p1, color});
            verts.push_back({p2 + endOffset, color});
            verts.push_back({p1 + endOffset, color});
        }
    };
    
    // Fonction pour créer un cône (flèche)
    auto addCone = [&](simd::float3 basePos, simd::float3 direction, simd::float4 color) {
        float height = 0.35f;
        float radius = 0.14f;
        simd::float3 tip = basePos + direction * height;
        
        simd::float3 perp1, perp2;
        if (std::abs(direction.y) < 0.9f) {
            perp1 = simd::normalize(simd::cross(direction, simd::float3{0, 1, 0}));
        } else {
            perp1 = simd::normalize(simd::cross(direction, simd::float3{1, 0, 0}));
        }
        perp2 = simd::cross(direction, perp1);
        
        constexpr int segments = 12;
        for (int i = 0; i < segments; i++) {
            float angle1 = static_cast<float>(i) / segments * M_PI * 2.0f;
            float angle2 = static_cast<float>(i + 1) / segments * M_PI * 2.0f;
            
            simd::float3 p1 = basePos + perp1 * (radius * std::cos(angle1)) + perp2 * (radius * std::sin(angle1));
            simd::float3 p2 = basePos + perp1 * (radius * std::cos(angle2)) + perp2 * (radius * std::sin(angle2));
            
            // Côté du cône
            verts.push_back({tip, color});
            verts.push_back({p2, color});
            verts.push_back({p1, color});
            
            // Base du cône
            verts.push_back({basePos, color});
            verts.push_back({p1, color});
            verts.push_back({p2, color});
        }
    };
    
    // Fonction pour créer une sphère
    auto addSphere = [&](simd::float3 center, float radius, simd::float4 color) {
        constexpr int stacks = 10;
        constexpr int slices = 14;
        
        for (int i = 0; i < stacks; i++) {
            float phi1 = static_cast<float>(i) / stacks * M_PI - M_PI / 2.0f;
            float phi2 = static_cast<float>(i + 1) / stacks * M_PI - M_PI / 2.0f;
            
            for (int j = 0; j < slices; j++) {
                float theta1 = static_cast<float>(j) / slices * M_PI * 2.0f;
                float theta2 = static_cast<float>(j + 1) / slices * M_PI * 2.0f;
                
                simd::float3 p1 = center + simd::float3{
                    radius * std::cos(phi1) * std::cos(theta1),
                    radius * std::sin(phi1),
                    radius * std::cos(phi1) * std::sin(theta1)
                };
                simd::float3 p2 = center + simd::float3{
                    radius * std::cos(phi1) * std::cos(theta2),
                    radius * std::sin(phi1),
                    radius * std::cos(phi1) * std::sin(theta2)
                };
                simd::float3 p3 = center + simd::float3{
                    radius * std::cos(phi2) * std::cos(theta2),
                    radius * std::sin(phi2),
                    radius * std::cos(phi2) * std::sin(theta2)
                };
                simd::float3 p4 = center + simd::float3{
                    radius * std::cos(phi2) * std::cos(theta1),
                    radius * std::sin(phi2),
                    radius * std::cos(phi2) * std::sin(theta1)
                };
                
                verts.push_back({p1, color});
                verts.push_back({p2, color});
                verts.push_back({p3, color});
                
                verts.push_back({p1, color});
                verts.push_back({p3, color});
                verts.push_back({p4, color});
            }
        }
    };
    
    // === Construire les axes ===
    
    // Axes positifs (cylindres épais)
    addCylinder(simd::float3{1, 0, 0}, style.axisX, len, thick);
    addCylinder(simd::float3{0, 1, 0}, style.axisY, len, thick);
    addCylinder(simd::float3{0, 0, 1}, style.axisZ, len, thick);
    
    // Axes négatifs (plus fins, semi-transparents)
    simd::float4 xNeg = style.axisX; xNeg.w = 0.4f;
    simd::float4 yNeg = style.axisY; yNeg.w = 0.4f;
    simd::float4 zNeg = style.axisZ; zNeg.w = 0.4f;
    addCylinder(simd::float3{-1, 0, 0}, xNeg, len * 0.4f, thick * 0.5f);
    addCylinder(simd::float3{0, -1, 0}, yNeg, len * 0.4f, thick * 0.5f);
    addCylinder(simd::float3{0, 0, -1}, zNeg, len * 0.4f, thick * 0.5f);
    
    // Cônes aux extrémités (flèches)
    addCone(simd::float3{len, 0, 0}, simd::float3{1, 0, 0}, style.axisX);
    addCone(simd::float3{0, len, 0}, simd::float3{0, 1, 0}, style.axisY);
    addCone(simd::float3{0, 0, len}, simd::float3{0, 0, 1}, style.axisZ);
    
    // Sphère centrale blanche
    addSphere(simd::float3{0, 0, 0}, thick * 3.0f, simd::float4{0.95f, 0.95f, 0.98f, 1.0f});
    
    m_axisVertexCount = static_cast<uint32_t>(verts.size());
    m_axisVB = m_device->newBuffer(verts.data(), verts.size() * sizeof(FabAxisVertex), MTL::ResourceStorageModeShared);
}

inline void FabPanel3D::buildGridLines() {
    std::vector<FabAxisVertex> verts;
    
    float halfSize = (FAB_GRID_SIZE - 1) * style.cubeSpacing * 0.5f;
    float step = style.cubeSpacing;
    simd::float4 lineColor = {0.35f, 0.40f, 0.50f, 0.30f};
    simd::float4 pillarColor = {0.30f, 0.35f, 0.45f, 0.20f};
    
    // Lignes au sol (grille XZ à Y = -halfSize)
    float groundY = -halfSize - 0.05f;
    for (uint32_t i = 0; i <= FAB_GRID_SIZE; i++) {
        float t = -halfSize + i * step;
        
        // Lignes parallèles à X
        verts.push_back({{-halfSize - 0.5f, groundY, t}, lineColor});
        verts.push_back({{ halfSize + 0.5f, groundY, t}, lineColor});
        
        // Lignes parallèles à Z
        verts.push_back({{t, groundY, -halfSize - 0.5f}, lineColor});
        verts.push_back({{t, groundY,  halfSize + 0.5f}, lineColor});
    }
    
    // Piliers verticaux aux 4 coins
    float corners[4][2] = {
        {-halfSize, -halfSize},
        { halfSize, -halfSize},
        { halfSize,  halfSize},
        {-halfSize,  halfSize}
    };
    for (int c = 0; c < 4; c++) {
        verts.push_back({{corners[c][0], -halfSize - 0.4f, corners[c][1]}, pillarColor});
        verts.push_back({{corners[c][0],  halfSize + 0.4f, corners[c][1]}, pillarColor});
    }
    
    // Lignes en haut (grille XZ à Y = +halfSize, plus transparentes)
    simd::float4 topLineColor = {0.30f, 0.35f, 0.45f, 0.12f};
    float topY = halfSize + 0.05f;
    for (uint32_t i = 0; i <= FAB_GRID_SIZE; i++) {
        float t = -halfSize + i * step;
        
        verts.push_back({{-halfSize, topY, t}, topLineColor});
        verts.push_back({{ halfSize, topY, t}, topLineColor});
        
        verts.push_back({{t, topY, -halfSize}, topLineColor});
        verts.push_back({{t, topY,  halfSize}, topLineColor});
    }
    
    m_gridLineCount = static_cast<uint32_t>(verts.size());
    m_gridLinesVB = m_device->newBuffer(verts.data(), verts.size() * sizeof(FabAxisVertex), MTL::ResourceStorageModeShared);
}

inline void FabPanel3D::createPipelines(MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt, MTL::Library* lib) {
    NS::Error* error = nullptr;
    
    // ===== Pipeline pour le panel 2D =====
    {
        auto* vsFunc = lib->newFunction(MTLSTR("fabPanelVertexShader"));
        auto* fsFunc = lib->newFunction(MTLSTR("fabPanelFragmentShader"));
        
        if (vsFunc && fsFunc) {
            auto* vd = MTL::VertexDescriptor::alloc()->init();
            vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
            vd->attributes()->object(0)->setOffset(0);
            vd->attributes()->object(0)->setBufferIndex(0);
            vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
            vd->attributes()->object(1)->setOffset(offsetof(FabPanelVertex, uv));
            vd->attributes()->object(1)->setBufferIndex(0);
            vd->layouts()->object(0)->setStride(sizeof(FabPanelVertex));
            
            auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();
            desc->setVertexFunction(vsFunc);
            desc->setFragmentFunction(fsFunc);
            desc->setVertexDescriptor(vd);
            desc->colorAttachments()->object(0)->setPixelFormat(colorFmt);
            desc->colorAttachments()->object(0)->setBlendingEnabled(true);
            desc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            desc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            desc->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            desc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            desc->setDepthAttachmentPixelFormat(depthFmt);
            
            m_panelPipeline = m_device->newRenderPipelineState(desc, &error);
            if (error) {
                printf("FabPanel3D: Panel pipeline error: %s\n", error->localizedDescription()->utf8String());
            }
            
            vd->release();
            desc->release();
        } else {
            printf("FabPanel3D: Could not find panel shaders (fabPanelVertexShader / fabPanelFragmentShader)\n");
        }
        
        if (vsFunc) vsFunc->release();
        if (fsFunc) fsFunc->release();
    }
    
    // ===== Pipeline pour les cubes 3D =====
    {
        auto* vsFunc = lib->newFunction(MTLSTR("fabCubeVertexShader"));
        auto* fsFunc = lib->newFunction(MTLSTR("fabCubeFragmentShader"));
        
        if (vsFunc && fsFunc) {
            auto* vd = MTL::VertexDescriptor::alloc()->init();
            vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
            vd->attributes()->object(0)->setOffset(0);
            vd->attributes()->object(0)->setBufferIndex(0);
            vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
            vd->attributes()->object(1)->setOffset(offsetof(FabCubeVertex, normal));
            vd->attributes()->object(1)->setBufferIndex(0);
            vd->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
            vd->attributes()->object(2)->setOffset(offsetof(FabCubeVertex, uv));
            vd->attributes()->object(2)->setBufferIndex(0);
            vd->layouts()->object(0)->setStride(sizeof(FabCubeVertex));
            
            auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();
            desc->setVertexFunction(vsFunc);
            desc->setFragmentFunction(fsFunc);
            desc->setVertexDescriptor(vd);
            desc->colorAttachments()->object(0)->setPixelFormat(colorFmt);
            desc->colorAttachments()->object(0)->setBlendingEnabled(true);
            desc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            desc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            desc->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            desc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            desc->setDepthAttachmentPixelFormat(depthFmt);
            
            m_cubePipeline = m_device->newRenderPipelineState(desc, &error);
            if (error) {
                printf("FabPanel3D: Cube pipeline error: %s\n", error->localizedDescription()->utf8String());
            }
            
            vd->release();
            desc->release();
        } else {
            printf("FabPanel3D: Could not find cube shaders (fabCubeVertexShader / fabCubeFragmentShader)\n");
        }
        
        if (vsFunc) vsFunc->release();
        if (fsFunc) fsFunc->release();
    }
    
    // ===== Pipeline pour les axes et lignes =====
    {
        auto* vsFunc = lib->newFunction(MTLSTR("fabAxisVertexShader"));
        auto* fsFunc = lib->newFunction(MTLSTR("fabAxisFragmentShader"));
        
        if (vsFunc && fsFunc) {
            auto* vd = MTL::VertexDescriptor::alloc()->init();
            vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
            vd->attributes()->object(0)->setOffset(0);
            vd->attributes()->object(0)->setBufferIndex(0);
            vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
            vd->attributes()->object(1)->setOffset(offsetof(FabAxisVertex, color));
            vd->attributes()->object(1)->setBufferIndex(0);
            vd->layouts()->object(0)->setStride(sizeof(FabAxisVertex));
            
            auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();
            desc->setVertexFunction(vsFunc);
            desc->setFragmentFunction(fsFunc);
            desc->setVertexDescriptor(vd);
            desc->colorAttachments()->object(0)->setPixelFormat(colorFmt);
            desc->colorAttachments()->object(0)->setBlendingEnabled(true);
            desc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            desc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            desc->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            desc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            desc->setDepthAttachmentPixelFormat(depthFmt);
            
            m_axisPipeline = m_device->newRenderPipelineState(desc, &error);
            if (error) {
                printf("FabPanel3D: Axis pipeline error: %s\n", error->localizedDescription()->utf8String());
            }
            
            // Réutiliser le même pipeline pour les lignes
            m_linePipeline = m_axisPipeline;
            if (m_linePipeline) m_linePipeline->retain();
            
            vd->release();
            desc->release();
        } else {
            printf("FabPanel3D: Could not find axis shaders (fabAxisVertexShader / fabAxisFragmentShader)\n");
        }
        
        if (vsFunc) vsFunc->release();
        if (fsFunc) fsFunc->release();
    }
    
    // ===== Depth stencil states =====
    {
        auto* dsd = MTL::DepthStencilDescriptor::alloc()->init();
        
        dsd->setDepthCompareFunction(MTL::CompareFunctionLess);
        dsd->setDepthWriteEnabled(true);
        m_depthEnabled = m_device->newDepthStencilState(dsd);
        
        dsd->setDepthCompareFunction(MTL::CompareFunctionAlways);
        dsd->setDepthWriteEnabled(false);
        m_depthDisabled = m_device->newDepthStencilState(dsd);
        
        dsd->release();
    }
}

inline simd::float4x4 FabPanel3D::computeViewMatrix() const {
    float distance = 12.0f / cameraZoom;
    float cosP = std::cos(cameraPitch);
    float sinP = std::sin(cameraPitch);
    float cosY = std::cos(cameraYaw);
    float sinY = std::sin(cameraYaw);
    
    simd::float3 eye = {
        distance * cosP * sinY,
        distance * sinP,
        distance * cosP * cosY
    };
    simd::float3 target = {0, 0, 0};
    simd::float3 up = {0, 1, 0};
    
    simd::float3 f = simd::normalize(target - eye);
    simd::float3 s = simd::normalize(simd::cross(f, up));
    simd::float3 u = simd::cross(s, f);
    
    return simd::float4x4{
        simd::float4{ s.x,  u.x, -f.x, 0},
        simd::float4{ s.y,  u.y, -f.y, 0},
        simd::float4{ s.z,  u.z, -f.z, 0},
        simd::float4{-simd::dot(s, eye), -simd::dot(u, eye), simd::dot(f, eye), 1}
    };
}

inline simd::float4x4 FabPanel3D::computeProjectionMatrix(float aspectRatio) const
{
    float near = 0.1f;
    float far = 100.0f;
    float fov = M_PI / 3.0f; // 60 degrés
    float yScale = 1.0f / std::tan(fov * 0.5f);
    float xScale = yScale / aspectRatio;
    float zRange = far - near;
        
    return simd::float4x4{
        simd::float4{xScale, 0, 0, 0},
        simd::float4{0, yScale, 0, 0},
        simd::float4{0, 0, -far / zRange, -1},
        simd::float4{0, 0, -far * near / zRange, 0}
    };
//    // Projection orthographique
//    float size = 5.0f / cameraZoom;
//    float left = -size * aspectRatio;
//    float right = size * aspectRatio;
//    float bottom = -size;
//    float top = size;
//    
//    return simd::float4x4{
//        simd::float4{2.0f / (right - left), 0, 0, 0},
//        simd::float4{0, 2.0f / (top - bottom), 0, 0},
//        simd::float4{0, 0, -2.0f / (far - near), 0},
//        simd::float4{-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1}
//    };
}

inline simd::float4 FabPanel3D::getColorForItem(FabItemID id) const {
    if (id == FAB_EMPTY) return style.emptySlot;
    if (itemColorCallback) return itemColorCallback(id);
    
    // Couleurs par défaut basées sur l'ID
    switch (id) {
        case 1: return {0.75f, 0.75f, 0.80f, 1.0f}; // Fer/métal
        case 2: return {1.00f, 0.85f, 0.20f, 1.0f}; // Or
        case 3: return {0.30f, 0.90f, 0.95f, 1.0f}; // Diamant
        case 4: return {0.60f, 0.45f, 0.30f, 1.0f}; // Bois
        case 5: return {0.55f, 0.55f, 0.52f, 1.0f}; // Pierre
        case 6: return {0.90f, 0.20f, 0.20f, 1.0f}; // Redstone
        case 7: return {0.20f, 0.85f, 0.35f, 1.0f}; // Émeraude
        case 8: return {0.25f, 0.25f, 0.28f, 1.0f}; // Charbon
        case 9: return {0.90f, 0.55f, 0.25f, 1.0f}; // Cuivre
        default: {
            // Génération pseudo-aléatoire basée sur l'ID
            float r = std::fmod(id * 0.618034f, 1.0f) * 0.5f + 0.45f;
            float g = std::fmod(id * 0.381966f + 0.25f, 1.0f) * 0.5f + 0.40f;
            float b = std::fmod(id * 0.723607f + 0.50f, 1.0f) * 0.5f + 0.45f;
            return {r, g, b, 1.0f};
        }
    }
}

inline void FabPanel3D::updateInstanceBuffer()
{
    auto* instances = static_cast<FabSlotInstance*>(m_instanceBuffer->contents());
    
    float halfExtent = (FAB_GRID_SIZE - 1) * style.cubeSpacing * 0.5f;
    
    for (uint32_t z = 0; z < FAB_GRID_SIZE; z++) {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; y++) {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; x++) {
                size_t idx = FabGrid3D::toIndex(x, y, z);
                const FabSlot& slot = grid.slots[idx];
                
                bool isEmpty = slot.isEmpty();
                bool isSelected = (x == cursor.x && y == cursor.y && z == cursor.z);
                
                instances[idx].worldPosition = {
                    static_cast<float>(x) * style.cubeSpacing - halfExtent,
                    static_cast<float>(y) * style.cubeSpacing - halfExtent,
                    static_cast<float>(z) * style.cubeSpacing - halfExtent
                };
                
                instances[idx].scale = isEmpty ? style.cubeSize * 0.5f : style.cubeSize;
                instances[idx].color = getColorForItem(slot.itemId);
                instances[idx].flags = (isEmpty ? 1u : 0u) | (isSelected ? 2u : 0u);
            }
        }
    }
}

inline void FabPanel3D::get3DViewportRect(simd::float2 screenSize, float& outX, float& outY, float& outW, float& outH) const
{
    float totalWidth = panelPixelSize.x + (inventoryOpen ? sidebarWidth : 0.0f);
    float panelX = panelCenter.x * screenSize.x;
    float panelY = panelCenter.y * screenSize.y;
    
    // La zone 3D est à droite de l'inventaire (si ouvert)
    float viewLeft = panelX - totalWidth * 0.5f + (inventoryOpen ? sidebarWidth : 0.0f) + 10.0f;
    float viewTop = panelY - panelPixelSize.y * 0.5f + headerHeight + 10.0f + 10.f - 50.f; // pile
//    float viewTop = panelY - panelPixelSize.y * 0.5f + headerHeight + 10.0f + 10.f; // pile
    float viewWidth = panelPixelSize.x - 20.0f;
    float viewHeight = panelPixelSize.y - headerHeight - 20.0f;
    
    // Convertir en coordonnées Metal (Y inversé)
    outX = viewLeft;
    outY = screenSize.y - (viewTop + viewHeight);
//    outY = screenSize.y - viewTop - viewHeight;
    outW = viewWidth;
    outH = viewHeight;
}

inline void FabPanel3D::update(float deltaTime)
{
    m_time += deltaTime;
}

inline void FabPanel3D::setViewportWindow(NS::UInteger width, NS::UInteger height)
{
    float vpX, vpY, vpW, vpH;
    get3DViewportRect(simd::float2{static_cast<float>(width), static_cast<float>(height)}, vpX, vpY, vpW, vpH);
    
    m_viewport3D.originX = vpX;
    m_viewport3D.originY = vpY;
    m_viewport3D.width = (double)vpW;
    m_viewport3D.height = (double)vpH;
    m_viewport3D.znear = 0.0;
    m_viewport3D.zfar = 1.0;
    
    m_scissor.x = static_cast<NS::UInteger>(std::max(0.0f, vpX));
    m_scissor.y = static_cast<NS::UInteger>(std::max(0.0f, vpY));
    m_scissor.width = static_cast<NS::UInteger>(vpW);
    m_scissor.height = static_cast<NS::UInteger>(vpH);
    
    aspect = vpW / vpH;
}

inline void FabPanel3D::render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize)
{
    if (!visible) return;
    
    // Calculer les dimensions
    float totalWidth = panelPixelSize.x + (inventoryOpen ? sidebarWidth : 0.0f);
    float panelX = panelCenter.x * screenSize.x;
    float panelY = panelCenter.y * screenSize.y;
    
    updateInstanceBuffer();
    
    // ========================================
    // PASS 1: Panel 2D (fond)
    // ========================================
    if (m_panelPipeline) {
        auto* uniforms = static_cast<FabPanelUniforms*>(m_panelUniforms->contents());
        uniforms->screenSize = screenSize;
        uniforms->panelPosition = {panelX, panelY};
        uniforms->panelSize = {totalWidth, panelPixelSize.y};
        uniforms->backgroundColor = style.panelBg;
        uniforms->borderColor = style.panelBorder;
        uniforms->headerColor = style.headerBg;
        uniforms->cornerRadius = style.cornerRadius;
        uniforms->borderWidth = style.borderWidth;
        uniforms->headerHeight = headerHeight;
        uniforms->time = m_time;
        uniforms->showInventory = inventoryOpen ? 1 : 0;
        uniforms->inventoryWidth = sidebarWidth;
        
        encoder->setRenderPipelineState(m_panelPipeline);
        encoder->setDepthStencilState(m_depthDisabled);
        encoder->setVertexBuffer(m_panelQuadVB, 0, 0);
        encoder->setVertexBuffer(m_panelUniforms, 0, 1);
        encoder->setFragmentBuffer(m_panelUniforms, 0, 0);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
    }
    encoder->setViewport(m_viewport3D);
    encoder->setScissorRect(m_scissor);

    simd::float4x4 viewMat = computeViewMatrix();
    simd::float4x4 projMat = computeProjectionMatrix(aspect);
    simd::float4x4 viewProjMat = simd_mul(projMat, viewMat);
    
    float dist = 12.0f / cameraZoom;
    simd::float3 camPos = {
        dist * std::cos(cameraPitch) * std::sin(cameraYaw),
        dist * std::sin(cameraPitch),
        dist * std::cos(cameraPitch) * std::cos(cameraYaw)
    };
    
    // Uniforms 3D
    auto* uniforms3D = static_cast<Fab3DUniforms*>(m_3dUniforms->contents());
    uniforms3D->viewProjectionMatrix = viewProjMat;
    uniforms3D->cameraPosition = camPos;
    uniforms3D->time = m_time;
    uniforms3D->lightDirection = simd::normalize(simd::float3{0.4f, 0.8f, 0.4f});
    uniforms3D->gridHalfSize = (FAB_GRID_SIZE - 1) * style.cubeSpacing * 0.5f;
    uniforms3D->selectionColor = style.selection;
    
    // ----- Lignes de grille -----
    if (m_linePipeline && m_gridLineCount > 0) {
        encoder->setRenderPipelineState(m_linePipeline);
        encoder->setDepthStencilState(m_depthEnabled);
        encoder->setVertexBuffer(m_gridLinesVB, 0, 0);
        encoder->setVertexBuffer(m_3dUniforms, 0, 1);
        encoder->setFragmentBuffer(m_3dUniforms, 0, 0);
        encoder->drawPrimitives(MTL::PrimitiveTypeLine, NS::UInteger(0), NS::UInteger(m_gridLineCount));
    }
    
    // ----- Cubes (instances) -----
    if (m_cubePipeline) {
        encoder->setRenderPipelineState(m_cubePipeline);
        encoder->setDepthStencilState(m_depthEnabled);
        encoder->setVertexBuffer(m_cubeVB, 0, 0);
        encoder->setVertexBuffer(m_3dUniforms, 0, 1);
        encoder->setVertexBuffer(m_instanceBuffer, 0, 2);
        encoder->setFragmentBuffer(m_3dUniforms, 0, 0);
        encoder->drawIndexedPrimitives(
            MTL::PrimitiveTypeTriangle,
            m_cubeIndexCount,
            MTL::IndexTypeUInt16,
            m_cubeIB,
            0,
            FAB_GRID_TOTAL
        );
    }
    
    // ----- Axes XYZ -----
    if (m_axisPipeline && m_axisVertexCount > 0) {
        encoder->setRenderPipelineState(m_axisPipeline);
        encoder->setDepthStencilState(m_depthEnabled);
        encoder->setVertexBuffer(m_axisVB, 0, 0);
        encoder->setVertexBuffer(m_3dUniforms, 0, 1);
        encoder->setFragmentBuffer(m_3dUniforms, 0, 0);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(m_axisVertexCount));
    }
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

inline bool FabPanel3D::isInsidePanel(simd::float2 pos, simd::float2 screenSize) const {
    float totalWidth = panelPixelSize.x + (inventoryOpen ? sidebarWidth : 0.0f);
    float px = panelCenter.x * screenSize.x;
    float py = panelCenter.y * screenSize.y;
    float halfW = totalWidth * 0.5f;
    float halfH = panelPixelSize.y * 0.5f;
    
    return pos.x >= px - halfW && pos.x <= px + halfW &&
           pos.y >= py - halfH && pos.y <= py + halfH;
}

inline bool FabPanel3D::isInsideHeader(simd::float2 pos, simd::float2 screenSize) const {
    float totalWidth = panelPixelSize.x + (inventoryOpen ? sidebarWidth : 0.0f);
    float px = panelCenter.x * screenSize.x;
    float py = panelCenter.y * screenSize.y;
    float left = px - totalWidth * 0.5f;
    float top = py - panelPixelSize.y * 0.5f;
    
    return pos.x >= left && pos.x <= left + totalWidth &&
           pos.y >= top && pos.y <= top + headerHeight;
}

inline bool FabPanel3D::isInside3DView(simd::float2 pos, simd::float2 screenSize) const {
    float vpX, vpY, vpW, vpH;
    get3DViewportRect(screenSize, vpX, vpY, vpW, vpH);
    
    // Convertir vpY de Metal coords (Y up from bottom) à screen coords (Y down from top)
    float screenTop = screenSize.y - vpY - vpH;
    
    return pos.x >= vpX && pos.x <= vpX + vpW &&
           pos.y >= screenTop && pos.y <= screenTop + vpH;
}

inline bool FabPanel3D::handleMouseDown(simd::float2 mousePos, simd::float2 screenSize, int button) {
    if (!visible || !isInsidePanel(mousePos, screenSize)) return false;
    
    if (isInsideHeader(mousePos, screenSize) && button == 0) {
        m_draggingPanel = true;
        float px = panelCenter.x * screenSize.x;
        float py = panelCenter.y * screenSize.y;
        m_panelDragOffset = {mousePos.x - px, mousePos.y - py};
        return true;
    }
    
    if (isInside3DView(mousePos, screenSize)) {
        m_draggingCamera = true;
        return true;
    }
    
    return true;
}

inline bool FabPanel3D::handleMouseUp(simd::float2 mousePos, simd::float2 screenSize, int button) {
    m_draggingPanel = false;
    m_draggingCamera = false;
    return visible && isInsidePanel(mousePos, screenSize);
}

inline bool FabPanel3D::handleMouseDrag(simd::float2 mousePos, simd::float2 delta, simd::float2 screenSize, int button) {
    if (!visible) return false;
    
    if (m_draggingPanel) {
        panelCenter.x = (mousePos.x - m_panelDragOffset.x) / screenSize.x;
        panelCenter.y = (mousePos.y - m_panelDragOffset.y) / screenSize.y;
        
        // Contraindre aux bords de l'écran
        float totalWidth = panelPixelSize.x + (inventoryOpen ? sidebarWidth : 0.0f);
        float marginX = totalWidth * 0.5f / screenSize.x + 0.02f;
        float marginY = panelPixelSize.y * 0.5f / screenSize.y + 0.02f;
        panelCenter.x = std::clamp(panelCenter.x, marginX, 1.0f - marginX);
        panelCenter.y = std::clamp(panelCenter.y, marginY, 1.0f - marginY);
        return true;
    }
    
    if (m_draggingCamera) {
        cameraYaw += delta.x * 0.008f;
        cameraPitch = std::clamp(cameraPitch + delta.y * 0.008f, -1.5f, 1.5f);
        return true;
    }
    
    return false;
}

inline bool FabPanel3D::handleScroll(simd::float2 mousePos, float scrollDelta, simd::float2 screenSize) {
    if (!visible || !isInsidePanel(mousePos, screenSize)) return false;
    
    cameraZoom = std::clamp(cameraZoom + scrollDelta * 0.1f, 0.3f, 3.0f);
    return true;
}

inline bool FabPanel3D::handleKey(uint16_t keyCode)
{
    if (!visible) return false;
    
    switch (keyCode)
    {
        case 13: moveCursor(0, 1, 0); return true;  // W - haut
        case 1:  moveCursor(0, -1, 0); return true; // S - bas
        case 0:  moveCursor(-1, 0, 0); return true; // A - gauche
        case 2:  moveCursor(1, 0, 0); return true;  // D - droite
        case 12: moveCursor(0, 0, -1); return true; // Q - arrière
        case 14: moveCursor(0, 0, 1); return true;  // E - avant
        case 34: toggleInventory(); return true;    // I - toggle inventaire
    }
    
    return false;
}

inline void FabPanel3D::moveCursor(int dx, int dy, int dz) {
    cursor.x = static_cast<uint32_t>(std::clamp(static_cast<int>(cursor.x) + dx, 0, static_cast<int>(FAB_GRID_SIZE) - 1));
    cursor.y = static_cast<uint32_t>(std::clamp(static_cast<int>(cursor.y) + dy, 0, static_cast<int>(FAB_GRID_SIZE) - 1));
    cursor.z = static_cast<uint32_t>(std::clamp(static_cast<int>(cursor.z) + dz, 0, static_cast<int>(FAB_GRID_SIZE) - 1));
}

inline void FabPanel3D::placeAtCursor(FabItemID id, uint32_t qty) {
    grid.place(cursor.x, cursor.y, cursor.z, id, qty);
}

inline FabSlot FabPanel3D::takeFromCursor() {
    return grid.take(cursor.x, cursor.y, cursor.z);
}

}


//struct FabGrid3D {
//    std::array<FabSlot, FAB_GRID_TOTAL> slots;
//    
//    FabGrid3D() { clear(); }
//    void clear() { for (auto& s : slots) s.clear(); }
//    
//    static size_t index(uint32_t x, uint32_t y, uint32_t z) {
//        return z * FAB_GRID_SIZE * FAB_GRID_SIZE + y * FAB_GRID_SIZE + x;
//    }
//    
//    FabSlot& at(uint32_t x, uint32_t y, uint32_t z) { return slots[index(x, y, z)]; }
//    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const { return slots[index(x, y, z)]; }
//    
//    bool placeItem(uint32_t x, uint32_t y, uint32_t z, FabItemID itemId, uint32_t qty = 1) {
//        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return false;
//        auto& slot = at(x, y, z);
//        if (slot.isEmpty()) { slot.itemId = itemId; slot.quantity = qty; return true; }
//        if (slot.itemId == itemId) { slot.quantity += qty; return true; }
//        return false;
//    }
//    
//    FabSlot removeItem(uint32_t x, uint32_t y, uint32_t z) {
//        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return {};
//        auto& slot = at(x, y, z);
//        FabSlot removed = slot;
//        slot.clear();
//        return removed;
//    }
//};
//
//// ============================================================================
//// GPU Structures
//// ============================================================================
//
//struct FabCubeVertex {
//    simd::float3 position;
//    simd::float3 normal;
//    simd::float2 uv;
//};
//
//struct FabSlotInstanceData {
//    simd::float3 position;
//    float scale;
//    simd::float4 color;
//    uint32_t flags;      // bit0: empty, bit1: selected, bit2: hovered
//    uint32_t slotX;
//    uint32_t slotY;
//    uint32_t slotZ;
//};
//
//struct FabGridUniforms {
//    simd::float4x4 viewProjection;
//    simd::float4x4 model;
//    simd::float3 cameraPos;
//    float time;
//    simd::float3 selectedSlot;
//    float slotSpacing;
//    simd::float4 emptySlotColor;
//    simd::float4 selectedColor;
//    simd::float4 hoveredColor;
//    float gridSize;
//    float padding[3];
//};
//
//struct FabAxisVertex {
//    simd::float3 position;
//    simd::float4 color;
//};
//
//struct FabAxisUniforms {
//    simd::float4x4 viewProjection;
//    float time;
//    float axisLength;
//    float axisThickness;
//    float padding;
//};
//
//struct FabPanelVertex {
//    simd::float2 position;
//    simd::float2 uv;
//};
//
//struct FabPanelUniforms {
//    simd::float2 screenSize;
//    simd::float2 panelPos;
//    simd::float2 panelSize;
//    simd::float4 bgColor;
//    simd::float4 borderColor;
//    simd::float4 headerColor;
//    simd::float4 shadowColor;
//    float cornerRadius;
//    float borderWidth;
//    float headerHeight;
//    float time;
//    float shadowBlur;
//    float shadowOffsetY;
//    int isInventoryOpen;
//    float inventoryWidth;
//};
//
//// ============================================================================
//// FabPanel3D - Classe principale
//// ============================================================================
//
//class FabPanel3D {
//public:
//    FabPanel3D(MTL::Device* device, MTL::PixelFormat colorFormat,
//               MTL::PixelFormat depthFormat, MTL::Library* shaderLibrary);
//    ~FabPanel3D();
//    
//    // ===== État =====
//    bool isVisible = false;
//    bool isInventoryOpen = true;
//    
//    void show() { isVisible = true; }
//    void hide() { isVisible = false; }
//    void toggle() { isVisible = !isVisible; }
//    void toggleInventory() { isInventoryOpen = !isInventoryOpen; }
//    
//    // ===== Grille 3D =====
//    FabGrid3D grid;
//    simd::uint3 selectedSlot = {2, 2, 2};
//    simd::uint3 hoveredSlot = {255, 255, 255};
//    
//    // ===== Vue 3D =====
//    float rotationY = 0.75f;
//    float rotationX = 0.5f;
//    float zoom = 1.0f;
//    float targetRotationY = 0.75f;
//    float targetRotationX = 0.5f;
//    float targetZoom = 1.0f;
//    
//    // ===== Position du panel =====
//    simd::float2 panelPosition = {0.5f, 0.5f};
//    simd::float2 panelSize = {700.f, 500.f};
//    float inventoryWidth = 220.f;
//    float headerHeight = 38.f;
//    
//    // ===== Couleurs et style =====
//    struct Style {
//        simd::float4 panelBg = {0.08f, 0.09f, 0.12f, 0.96f};
//        simd::float4 panelBorder = {0.25f, 0.32f, 0.48f, 1.0f};
//        simd::float4 headerBg = {0.12f, 0.14f, 0.20f, 1.0f};
//        simd::float4 shadow = {0.0f, 0.0f, 0.0f, 0.5f};
//        simd::float4 inventoryBg = {0.06f, 0.07f, 0.10f, 0.95f};
//        simd::float4 emptySlot = {0.18f, 0.20f, 0.26f, 0.25f};
//        simd::float4 selectedSlot = {1.0f, 0.85f, 0.25f, 1.0f};
//        simd::float4 hoveredSlot = {0.5f, 0.7f, 1.0f, 0.8f};
//        simd::float4 axisX = {0.95f, 0.25f, 0.25f, 1.0f};
//        simd::float4 axisY = {0.25f, 0.9f, 0.3f, 1.0f};
//        simd::float4 axisZ = {0.3f, 0.5f, 0.95f, 1.0f};
//        float cornerRadius = 12.f;
//        float borderWidth = 2.f;
//        float shadowBlur = 25.f;
//        float shadowOffsetY = 8.f;
//        float slotScale = 0.42f;
//        float slotSpacing = 1.0f;
//        float axisLength = 3.2f;
//        float axisThickness = 0.08f;
//    } style;
//    
//    // ===== Callback couleur items =====
//    std::function<simd::float4(FabItemID)> getItemColor = nullptr;
//    
//    // ===== Input =====
//    void onMouseDown(simd::float2 screenPos, simd::float2 screenSize, int button);
//    void onMouseUp(simd::float2 screenPos, simd::float2 screenSize, int button);
//    void onMouseMoved(simd::float2 screenPos, simd::float2 screenSize);
//    void onMouseDragged(simd::float2 screenPos, simd::float2 screenSize, simd::float2 delta, int button);
//    void onMouseScroll(float delta);
//    void onKeyPressed(int keyCode);
//    
//    void moveSelection(int dx, int dy, int dz);
//    void placeItem(FabItemID itemId, uint32_t qty = 1);
//    FabSlot removeItem();
//    
//    // ===== Update & Render =====
//    void update(float deltaTime);
//    void render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize);
//    
//    // ===== Hit testing =====
//    bool hitTestPanel(simd::float2 screenPos, simd::float2 screenSize) const;
//    bool hitTestHeader(simd::float2 screenPos, simd::float2 screenSize) const;
//    bool hitTest3DView(simd::float2 screenPos, simd::float2 screenSize) const;
//    bool hitTestInventoryToggle(simd::float2 screenPos, simd::float2 screenSize) const;
//    
//private:
//    MTL::Device* m_device;
//    
//    // Pipelines
//    MTL::RenderPipelineState* m_panelPipeline = nullptr;
//    MTL::RenderPipelineState* m_gridPipeline = nullptr;
//    MTL::RenderPipelineState* m_axisPipeline = nullptr;
//    MTL::RenderPipelineState* m_gridLinesPipeline = nullptr;
//    MTL::DepthStencilState* m_depthStateOn = nullptr;
//    MTL::DepthStencilState* m_depthStateOff = nullptr;
//    
//    // Buffers
//    MTL::Buffer* m_panelVertexBuffer = nullptr;
//    MTL::Buffer* m_panelUniformBuffer = nullptr;
//    MTL::Buffer* m_cubeVertexBuffer = nullptr;
//    MTL::Buffer* m_cubeIndexBuffer = nullptr;
//    MTL::Buffer* m_instanceBuffer = nullptr;
//    MTL::Buffer* m_gridUniformBuffer = nullptr;
//    MTL::Buffer* m_axisVertexBuffer = nullptr;
//    MTL::Buffer* m_axisUniformBuffer = nullptr;
//    MTL::Buffer* m_gridLinesVertexBuffer = nullptr;
//    
//    float m_time = 0.f;
//    bool m_isDraggingPanel = false;
//    bool m_isDraggingRotation = false;
//    simd::float2 m_dragOffset = {0.f, 0.f};
//    
//    uint32_t m_cubeIndexCount = 0;
//    uint32_t m_axisVertexCount = 0;
//    uint32_t m_gridLinesVertexCount = 0;
//    
//    void buildPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* library);
//    void buildGeometry();
//    void buildAxisGeometry();
//    void buildGridLinesGeometry();
//    void updateInstanceData();
//    
//    simd::float4x4 createViewMatrix() const;
//    simd::float4x4 createProjectionMatrix(float aspect) const;
//    simd::float4 defaultItemColor(FabItemID id) const;
//    
//    simd::float2 getPanelScreenPos(simd::float2 screenSize) const;
//};
//
//// ============================================================================
//// Implémentation
//// ============================================================================
//
//inline FabPanel3D::FabPanel3D(MTL::Device* device, MTL::PixelFormat colorFormat,
//                              MTL::PixelFormat depthFormat, MTL::Library* shaderLibrary)
//    : m_device(device)
//{
//    buildGeometry();
//    buildAxisGeometry();
//    buildGridLinesGeometry();
//    buildPipelines(colorFormat, depthFormat, shaderLibrary);
//}
//
//inline FabPanel3D::~FabPanel3D() {
//    if (m_panelPipeline) m_panelPipeline->release();
//    if (m_gridPipeline) m_gridPipeline->release();
//    if (m_axisPipeline) m_axisPipeline->release();
//    if (m_gridLinesPipeline) m_gridLinesPipeline->release();
//    if (m_depthStateOn) m_depthStateOn->release();
//    if (m_depthStateOff) m_depthStateOff->release();
//    if (m_panelVertexBuffer) m_panelVertexBuffer->release();
//    if (m_panelUniformBuffer) m_panelUniformBuffer->release();
//    if (m_cubeVertexBuffer) m_cubeVertexBuffer->release();
//    if (m_cubeIndexBuffer) m_cubeIndexBuffer->release();
//    if (m_instanceBuffer) m_instanceBuffer->release();
//    if (m_gridUniformBuffer) m_gridUniformBuffer->release();
//    if (m_axisVertexBuffer) m_axisVertexBuffer->release();
//    if (m_axisUniformBuffer) m_axisUniformBuffer->release();
//    if (m_gridLinesVertexBuffer) m_gridLinesVertexBuffer->release();
//}
//
//inline void FabPanel3D::buildGeometry() {
//    // Panel quad
//    FabPanelVertex panelVerts[] = {
//        {{-1,-1}, {0,1}}, {{1,-1}, {1,1}}, {{1,1}, {1,0}},
//        {{-1,-1}, {0,1}}, {{1,1}, {1,0}}, {{-1,1}, {0,0}}
//    };
//    m_panelVertexBuffer = m_device->newBuffer(panelVerts, sizeof(panelVerts), MTL::ResourceStorageModeShared);
//    m_panelUniformBuffer = m_device->newBuffer(sizeof(FabPanelUniforms), MTL::ResourceStorageModeShared);
//    
//    // Cube avec normales et UVs
//    float s = 0.5f;
//    std::vector<FabCubeVertex> cubeVerts;
//    std::vector<uint16_t> cubeIndices;
//    
//    auto addFace = [&](simd::float3 n, simd::float3 right, simd::float3 up) {
//        uint16_t base = (uint16_t)cubeVerts.size();
//        simd::float3 center = n * s;
//        cubeVerts.push_back({center - right * s - up * s, n, {0, 0}});
//        cubeVerts.push_back({center + right * s - up * s, n, {1, 0}});
//        cubeVerts.push_back({center + right * s + up * s, n, {1, 1}});
//        cubeVerts.push_back({center - right * s + up * s, n, {0, 1}});
//        cubeIndices.insert(cubeIndices.end(), {base, uint16_t(base+1), uint16_t(base+2),
//                                               base, uint16_t(base+2), uint16_t(base+3)});
//    };
//    
//    addFace(simd::float3{ 0, 0, 1}, simd::float3{ 1, 0, 0}, simd::float3{0, 1, 0});
//    addFace(simd::float3{ 0, 0,-1}, simd::float3{-1, 0, 0}, simd::float3{0, 1, 0});
//    addFace(simd::float3{ 0, 1, 0}, simd::float3{ 1, 0, 0}, simd::float3{0, 0,-1});
//    addFace(simd::float3{ 0,-1, 0}, simd::float3{ 1, 0, 0}, simd::float3{0, 0, 1});
//    addFace(simd::float3{ 1, 0, 0}, simd::float3{ 0, 0,-1}, simd::float3{0, 1, 0});
//    addFace(simd::float3{-1, 0, 0}, simd::float3{ 0, 0, 1}, simd::float3{0, 1, 0});
//    
//    m_cubeVertexBuffer = m_device->newBuffer(cubeVerts.data(), cubeVerts.size() * sizeof(FabCubeVertex), MTL::ResourceStorageModeShared);
//    m_cubeIndexBuffer = m_device->newBuffer(cubeIndices.data(), cubeIndices.size() * sizeof(uint16_t), MTL::ResourceStorageModeShared);
//    m_cubeIndexCount = (uint32_t)cubeIndices.size();
//    
//    m_instanceBuffer = m_device->newBuffer(sizeof(FabSlotInstanceData) * FAB_GRID_TOTAL, MTL::ResourceStorageModeShared);
//    m_gridUniformBuffer = m_device->newBuffer(sizeof(FabGridUniforms), MTL::ResourceStorageModeShared);
//}
//
//inline void FabPanel3D::buildAxisGeometry() {
//    std::vector<FabAxisVertex> axisVerts;
//    
//    float len = style.axisLength;
//    float t = style.axisThickness;
//    
//    auto addAxisBox = [&](simd::float3 dir, simd::float4 color, float length, float thickness) {
//        simd::float3 end = dir * length;
//        simd::float3 perp1, perp2;
//        
//        if (fabsf(dir.x) < 0.9f) {
//            perp1 = simd::normalize(simd::cross(dir, simd::float3{1, 0, 0}));
//        } else {
//            perp1 = simd::normalize(simd::cross(dir, simd::float3{0, 1, 0}));
//        }
//        perp2 = simd::cross(dir, perp1);
//        
//        perp1 = perp1 * thickness;
//        perp2 = perp2 * thickness;
//        
//        simd::float3 v[8] = {
//            -perp1 - perp2, perp1 - perp2, perp1 + perp2, -perp1 + perp2,
//            end - perp1 - perp2, end + perp1 - perp2, end + perp1 + perp2, end - perp1 + perp2
//        };
//        
//        int faces[6][4] = {{0,1,2,3}, {4,5,6,7}, {0,1,5,4}, {2,3,7,6}, {1,2,6,5}, {3,0,4,7}};
//        
//        for (int f = 0; f < 6; f++) {
//            axisVerts.push_back({v[faces[f][0]], color});
//            axisVerts.push_back({v[faces[f][1]], color});
//            axisVerts.push_back({v[faces[f][2]], color});
//            axisVerts.push_back({v[faces[f][0]], color});
//            axisVerts.push_back({v[faces[f][2]], color});
//            axisVerts.push_back({v[faces[f][3]], color});
//        }
//    };
//    
//    // Axes positifs
//    addAxisBox(simd::float3{1, 0, 0}, style.axisX, len, t);
//    addAxisBox(simd::float3{0, 1, 0}, style.axisY, len, t);
//    addAxisBox(simd::float3{0, 0, 1}, style.axisZ, len, t);
//    
//    // Axes négatifs (plus fins, plus transparents)
//    simd::float4 xNeg = style.axisX; xNeg.w = 0.35f;
//    simd::float4 yNeg = style.axisY; yNeg.w = 0.35f;
//    simd::float4 zNeg = style.axisZ; zNeg.w = 0.35f;
//    
//    addAxisBox(simd::float3{-1, 0, 0}, xNeg, len * 0.5f, t * 0.6f);
//    addAxisBox(simd::float3{0, -1, 0}, yNeg, len * 0.5f, t * 0.6f);
//    addAxisBox(simd::float3{0, 0, -1}, zNeg, len * 0.5f, t * 0.6f);
//    
//    // Sphère centrale
//    float r = t * 3.0f;
//    simd::float4 centerColor = {0.92f, 0.93f, 0.97f, 1.0f};
//    int segments = 16;
//    
//    for (int i = 0; i < segments; i++) {
//        for (int j = 0; j < segments; j++) {
//            float theta1 = (float)i / segments * M_PI * 2.f;
//            float theta2 = (float)(i + 1) / segments * M_PI * 2.f;
//            float phi1 = (float)j / segments * M_PI - M_PI / 2.f;
//            float phi2 = (float)(j + 1) / segments * M_PI - M_PI / 2.f;
//            
//            simd::float3 p1 = {r * cosf(phi1) * cosf(theta1), r * sinf(phi1), r * cosf(phi1) * sinf(theta1)};
//            simd::float3 p2 = {r * cosf(phi1) * cosf(theta2), r * sinf(phi1), r * cosf(phi1) * sinf(theta2)};
//            simd::float3 p3 = {r * cosf(phi2) * cosf(theta2), r * sinf(phi2), r * cosf(phi2) * sinf(theta2)};
//            simd::float3 p4 = {r * cosf(phi2) * cosf(theta1), r * sinf(phi2), r * cosf(phi2) * sinf(theta1)};
//            
//            axisVerts.push_back({p1, centerColor});
//            axisVerts.push_back({p2, centerColor});
//            axisVerts.push_back({p3, centerColor});
//            axisVerts.push_back({p1, centerColor});
//            axisVerts.push_back({p3, centerColor});
//            axisVerts.push_back({p4, centerColor});
//        }
//    }
//    
//    // Cônes aux extrémités des axes
//    auto addCone = [&](simd::float3 pos, simd::float3 dir, simd::float4 color) {
//        float coneH = 0.25f;
//        float coneR = 0.12f;
//        simd::float3 tip = pos + dir * coneH;
//        
//        simd::float3 perp1, perp2;
//        if (fabsf(dir.x) < 0.9f) {
//            perp1 = simd::normalize(simd::cross(dir, simd::float3{1, 0, 0}));
//        } else {
//            perp1 = simd::normalize(simd::cross(dir, simd::float3{0, 1, 0}));
//        }
//        perp2 = simd::cross(dir, perp1);
//        
//        int segs = 12;
//        for (int i = 0; i < segs; i++) {
//            float a1 = (float)i / segs * M_PI * 2.f;
//            float a2 = (float)(i + 1) / segs * M_PI * 2.f;
//            
//            simd::float3 p1 = pos + perp1 * (coneR * cosf(a1)) + perp2 * (coneR * sinf(a1));
//            simd::float3 p2 = pos + perp1 * (coneR * cosf(a2)) + perp2 * (coneR * sinf(a2));
//            
//            axisVerts.push_back({tip, color});
//            axisVerts.push_back({p1, color});
//            axisVerts.push_back({p2, color});
//            
//            // Base
//            axisVerts.push_back({pos, color});
//            axisVerts.push_back({p2, color});
//            axisVerts.push_back({p1, color});
//        }
//    };
//    
//    addCone(simd::float3{len, 0, 0}, simd::float3{1, 0, 0}, style.axisX);
//    addCone(simd::float3{0, len, 0}, simd::float3{0, 1, 0}, style.axisY);
//    addCone(simd::float3{0, 0, len}, simd::float3{0, 0, 1}, style.axisZ);
//    
//    m_axisVertexCount = (uint32_t)axisVerts.size();
//    m_axisVertexBuffer = m_device->newBuffer(axisVerts.data(), axisVerts.size() * sizeof(FabAxisVertex), MTL::ResourceStorageModeShared);
//    m_axisUniformBuffer = m_device->newBuffer(sizeof(FabAxisUniforms), MTL::ResourceStorageModeShared);
//}
//
//inline void FabPanel3D::buildGridLinesGeometry() {
//    std::vector<FabAxisVertex> lines;
//    
//    float halfSize = (FAB_GRID_SIZE - 1) * style.slotSpacing * 0.5f;
//    float step = style.slotSpacing;
//    simd::float4 lineColor = {0.35f, 0.4f, 0.5f, 0.2f};
//    simd::float4 cornerColor = {0.3f, 0.35f, 0.45f, 0.12f};
//    
//    // Grille au sol
//    float y = -halfSize - 0.02f;
//    for (int i = 0; i <= FAB_GRID_SIZE; i++) {
//        float t = -halfSize + i * step;
//        lines.push_back({{-halfSize - 0.3f, y, t}, lineColor});
//        lines.push_back({{ halfSize + 0.3f, y, t}, lineColor});
//        lines.push_back({{t, y, -halfSize - 0.3f}, lineColor});
//        lines.push_back({{t, y,  halfSize + 0.3f}, lineColor});
//    }
//    
//    // Piliers aux coins
//    float corners[4][2] = {{-halfSize, -halfSize}, {halfSize, -halfSize},
//                           {halfSize, halfSize}, {-halfSize, halfSize}};
//    for (int c = 0; c < 4; c++) {
//        lines.push_back({{corners[c][0], -halfSize - 0.3f, corners[c][1]}, cornerColor});
//        lines.push_back({{corners[c][0],  halfSize + 0.3f, corners[c][1]}, cornerColor});
//    }
//    
//    // Grille du haut (plus légère)
//    simd::float4 topLineColor = {0.3f, 0.35f, 0.45f, 0.08f};
//    y = halfSize + 0.02f;
//    for (int i = 0; i <= FAB_GRID_SIZE; i++) {
//        float t = -halfSize + i * step;
//        lines.push_back({{-halfSize, y, t}, topLineColor});
//        lines.push_back({{ halfSize, y, t}, topLineColor});
//        lines.push_back({{t, y, -halfSize}, topLineColor});
//        lines.push_back({{t, y,  halfSize}, topLineColor});
//    }
//    
//    m_gridLinesVertexCount = (uint32_t)lines.size();
//    m_gridLinesVertexBuffer = m_device->newBuffer(lines.data(), lines.size() * sizeof(FabAxisVertex), MTL::ResourceStorageModeShared);
//}
//
//inline void FabPanel3D::buildPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat,
//                                        MTL::Library* library) {
//    NS::Error* error = nullptr;
//    
//    // ===== Panel Pipeline =====
//    {
//        MTL::Function* vert = library->newFunction(MTLSTR("fabPanelVertexShader"));
//        MTL::Function* frag = library->newFunction(MTLSTR("fabPanelFragmentShader"));
//        if (vert && frag) {
//            MTL::VertexDescriptor* vd = MTL::VertexDescriptor::alloc()->init();
//            vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
//            vd->attributes()->object(0)->setOffset(0);
//            vd->attributes()->object(0)->setBufferIndex(0);
//            vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
//            vd->attributes()->object(1)->setOffset(8);
//            vd->attributes()->object(1)->setBufferIndex(0);
//            vd->layouts()->object(0)->setStride(sizeof(FabPanelVertex));
//            
//            MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
//            pd->setVertexFunction(vert);
//            pd->setFragmentFunction(frag);
//            pd->setVertexDescriptor(vd);
//            pd->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//            pd->colorAttachments()->object(0)->setBlendingEnabled(true);
//            pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//            pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//            pd->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
//            pd->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//            pd->setDepthAttachmentPixelFormat(depthFormat);
//            
//            m_panelPipeline = m_device->newRenderPipelineState(pd, &error);
//            vd->release(); pd->release();
//        }
//        if (vert) vert->release();
//        if (frag) frag->release();
//    }
//    
//    // ===== Grid Pipeline =====
//    {
//        MTL::Function* vert = library->newFunction(MTLSTR("fabGridVertexShader"));
//        MTL::Function* frag = library->newFunction(MTLSTR("fabGridFragmentShader"));
//        if (vert && frag) {
//            MTL::VertexDescriptor* vd = MTL::VertexDescriptor::alloc()->init();
//            vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//            vd->attributes()->object(0)->setOffset(0);
//            vd->attributes()->object(0)->setBufferIndex(0);
//            vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//            vd->attributes()->object(1)->setOffset(12);
//            vd->attributes()->object(1)->setBufferIndex(0);
//            vd->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
//            vd->attributes()->object(2)->setOffset(24);
//            vd->attributes()->object(2)->setBufferIndex(0);
//            vd->layouts()->object(0)->setStride(sizeof(FabCubeVertex));
//            
//            MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
//            pd->setVertexFunction(vert);
//            pd->setFragmentFunction(frag);
//            pd->setVertexDescriptor(vd);
//            pd->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//            pd->colorAttachments()->object(0)->setBlendingEnabled(true);
//            pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//            pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//            pd->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
//            pd->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//            pd->setDepthAttachmentPixelFormat(depthFormat);
//            
//            m_gridPipeline = m_device->newRenderPipelineState(pd, &error);
//            vd->release(); pd->release();
//        }
//        if (vert) vert->release();
//        if (frag) frag->release();
//    }
//    
//    // ===== Axis Pipeline =====
//    {
//        MTL::Function* vert = library->newFunction(MTLSTR("fabAxisVertexShader"));
//        MTL::Function* frag = library->newFunction(MTLSTR("fabAxisFragmentShader"));
//        if (vert && frag) {
//            MTL::VertexDescriptor* vd = MTL::VertexDescriptor::alloc()->init();
//            vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//            vd->attributes()->object(0)->setOffset(0);
//            vd->attributes()->object(0)->setBufferIndex(0);
//            vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
//            vd->attributes()->object(1)->setOffset(12);
//            vd->attributes()->object(1)->setBufferIndex(0);
//            vd->layouts()->object(0)->setStride(sizeof(FabAxisVertex));
//            
//            MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
//            pd->setVertexFunction(vert);
//            pd->setFragmentFunction(frag);
//            pd->setVertexDescriptor(vd);
//            pd->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//            pd->colorAttachments()->object(0)->setBlendingEnabled(true);
//            pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//            pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//            pd->setDepthAttachmentPixelFormat(depthFormat);
//            
//            m_axisPipeline = m_device->newRenderPipelineState(pd, &error);
//            m_gridLinesPipeline = m_axisPipeline;
//            if (m_gridLinesPipeline) m_gridLinesPipeline->retain();
//            vd->release(); pd->release();
//        }
//        if (vert) vert->release();
//        if (frag) frag->release();
//    }
//    
//    // Depth States
//    MTL::DepthStencilDescriptor* dsd = MTL::DepthStencilDescriptor::alloc()->init();
//    dsd->setDepthCompareFunction(MTL::CompareFunctionLess);
//    dsd->setDepthWriteEnabled(true);
//    m_depthStateOn = m_device->newDepthStencilState(dsd);
//    
//    dsd->setDepthCompareFunction(MTL::CompareFunctionAlways);
//    dsd->setDepthWriteEnabled(false);
//    m_depthStateOff = m_device->newDepthStencilState(dsd);
//    dsd->release();
//}
//
//inline simd::float4x4 FabPanel3D::createViewMatrix() const {
//    float dist = 9.0f / zoom;
//    float cx = cosf(rotationX), sx = sinf(rotationX);
//    float cy = cosf(rotationY), sy = sinf(rotationY);
//    
//    simd::float3 eye = {dist * sy * cx, dist * sx, dist * cy * cx};
//    simd::float3 center = {0, 0, 0};
//    simd::float3 up = {0, 1, 0};
//    
//    simd::float3 f = simd::normalize(center - eye);
//    simd::float3 s = simd::normalize(simd::cross(f, up));
//    simd::float3 u = simd::cross(s, f);
//    
//    return simd::float4x4{
//        simd::float4{s.x, u.x, -f.x, 0},
//        simd::float4{s.y, u.y, -f.y, 0},
//        simd::float4{s.z, u.z, -f.z, 0},
//        simd::float4{-simd::dot(s, eye), -simd::dot(u, eye), simd::dot(f, eye), 1}
//    };
//}
//
//inline simd::float4x4 FabPanel3D::createProjectionMatrix(float aspect) const {
//    float size = 4.0f / zoom;
//    float l = -size * aspect, r = size * aspect;
//    float b = -size, t = size;
//    float n = 0.1f, f = 100.f;
//    
//    return simd::float4x4{
//        simd::float4{2.f/(r-l), 0, 0, 0},
//        simd::float4{0, 2.f/(t-b), 0, 0},
//        simd::float4{0, 0, -2.f/(f-n), 0},
//        simd::float4{-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1}
//    };
//}
//
//inline simd::float4 FabPanel3D::defaultItemColor(FabItemID id) const {
//    if (id == FAB_ITEM_EMPTY) return style.emptySlot;
//    float r = fmodf(id * 0.618034f, 1.f) * 0.4f + 0.55f;
//    float g = fmodf(id * 0.381966f + 0.25f, 1.f) * 0.4f + 0.45f;
//    float b = fmodf(id * 0.723607f + 0.5f, 1.f) * 0.5f + 0.45f;
//    return {r, g, b, 1.f};
//}
//
//inline void FabPanel3D::updateInstanceData() {
//    auto* data = static_cast<FabSlotInstanceData*>(m_instanceBuffer->contents());
//    float offset = (FAB_GRID_SIZE - 1) * style.slotSpacing * 0.5f;
//    
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
//                size_t idx = FabGrid3D::index(x, y, z);
//                const FabSlot& slot = grid.slots[idx];
//                
//                data[idx].position = {
//                    x * style.slotSpacing - offset,
//                    y * style.slotSpacing - offset,
//                    z * style.slotSpacing - offset
//                };
//                
//                bool isEmpty = slot.isEmpty();
//                bool isSelected = (x == selectedSlot.x && y == selectedSlot.y && z == selectedSlot.z);
//                bool isHovered = (x == hoveredSlot.x && y == hoveredSlot.y && z == hoveredSlot.z);
//                
//                data[idx].scale = isEmpty ? style.slotScale * 0.45f : style.slotScale;
//                data[idx].color = getItemColor ? getItemColor(slot.itemId) : defaultItemColor(slot.itemId);
//                
//                uint32_t flags = 0;
//                if (isEmpty) flags |= 1;
//                if (isSelected) flags |= 2;
//                if (isHovered) flags |= 4;
//                data[idx].flags = flags;
//                data[idx].slotX = x;
//                data[idx].slotY = y;
//                data[idx].slotZ = z;
//            }
//        }
//    }
//}
//
//inline simd::float2 FabPanel3D::getPanelScreenPos(simd::float2 screenSize) const {
//    return {panelPosition.x * screenSize.x, panelPosition.y * screenSize.y};
//}
//
//inline void FabPanel3D::update(float deltaTime) {
//    m_time += deltaTime;
//    
//    float smoothing = 1.f - powf(0.001f, deltaTime);
//    rotationX += (targetRotationX - rotationX) * smoothing;
//    rotationY += (targetRotationY - rotationY) * smoothing;
//    zoom += (targetZoom - zoom) * smoothing;
//}
//
//inline void FabPanel3D::render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize) {
//    if (!isVisible) return;
//    
//    updateInstanceData();
//    
//    simd::float2 panelPos = getPanelScreenPos(screenSize);
//    float totalWidth = panelSize.x + (isInventoryOpen ? inventoryWidth : 0.f);
//    
//    // ===== 1. Panel Background =====
//    if (m_panelPipeline) {
//        auto* u = static_cast<FabPanelUniforms*>(m_panelUniformBuffer->contents());
//        u->screenSize = screenSize;
//        u->panelPos = panelPos;
//        u->panelSize = {totalWidth, panelSize.y};
//        u->bgColor = style.panelBg;
//        u->borderColor = style.panelBorder;
//        u->headerColor = style.headerBg;
//        u->shadowColor = style.shadow;
//        u->cornerRadius = style.cornerRadius;
//        u->borderWidth = style.borderWidth;
//        u->headerHeight = headerHeight;
//        u->time = m_time;
//        u->shadowBlur = style.shadowBlur;
//        u->shadowOffsetY = style.shadowOffsetY;
//        u->isInventoryOpen = isInventoryOpen ? 1 : 0;
//        u->inventoryWidth = inventoryWidth;
//        
//        encoder->setRenderPipelineState(m_panelPipeline);
//        encoder->setDepthStencilState(m_depthStateOff);
//        encoder->setVertexBuffer(m_panelVertexBuffer, 0, 0);
//        encoder->setVertexBuffer(m_panelUniformBuffer, 0, 1);
//        encoder->setFragmentBuffer(m_panelUniformBuffer, 0, 0);
//        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
//    }
//    
//    // ===== 2. Grille 3D =====
//    float viewW = panelSize.x;
//    float viewH = panelSize.y - headerHeight;
//    float aspect = viewW / viewH;
//    
//    simd::float4x4 view = createViewMatrix();
//    simd::float4x4 proj = createProjectionMatrix(aspect);
//    simd::float4x4 vp = simd_mul(proj, view);
//    
//    // Grid uniforms
//    auto* gu = static_cast<FabGridUniforms*>(m_gridUniformBuffer->contents());
//    gu->viewProjection = vp;
//    gu->model = simd::float4x4{
//        simd::float4{1,0,0,0}, simd::float4{0,1,0,0},
//        simd::float4{0,0,1,0}, simd::float4{0,0,0,1}
//    };
//    float dist = 9.0f / zoom;
//    float cx = cosf(rotationX), sx = sinf(rotationX);
//    float cy = cosf(rotationY), sy = sinf(rotationY);
//    gu->cameraPos = {dist * sy * cx, dist * sx, dist * cy * cx};
//    gu->time = m_time;
//    gu->selectedSlot = {(float)selectedSlot.x, (float)selectedSlot.y, (float)selectedSlot.z};
//    gu->slotSpacing = style.slotSpacing;
//    gu->emptySlotColor = style.emptySlot;
//    gu->selectedColor = style.selectedSlot;
//    gu->hoveredColor = style.hoveredSlot;
//    gu->gridSize = FAB_GRID_SIZE;
//    
//    // Axis uniforms
//    auto* au = static_cast<FabAxisUniforms*>(m_axisUniformBuffer->contents());
//    au->viewProjection = vp;
//    au->time = m_time;
//    au->axisLength = style.axisLength;
//    au->axisThickness = style.axisThickness;
//    
//    // Grid Lines
//    if (m_gridLinesPipeline && m_gridLinesVertexCount > 0) {
//        encoder->setRenderPipelineState(m_gridLinesPipeline);
//        encoder->setDepthStencilState(m_depthStateOn);
//        encoder->setVertexBuffer(m_gridLinesVertexBuffer, 0, 0);
//        encoder->setVertexBuffer(m_axisUniformBuffer, 0, 1);
//        encoder->setFragmentBuffer(m_axisUniformBuffer, 0, 0);
//        encoder->drawPrimitives(MTL::PrimitiveTypeLine, NS::UInteger(0), NS::UInteger(m_gridLinesVertexCount));
//    }
//    
//    // Cubes
//    if (m_gridPipeline) {
//        encoder->setRenderPipelineState(m_gridPipeline);
//        encoder->setDepthStencilState(m_depthStateOn);
//        encoder->setVertexBuffer(m_cubeVertexBuffer, 0, 0);
//        encoder->setVertexBuffer(m_gridUniformBuffer, 0, 1);
//        encoder->setVertexBuffer(m_instanceBuffer, 0, 2);
//        encoder->setFragmentBuffer(m_gridUniformBuffer, 0, 0);
//        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, m_cubeIndexCount,
//                                       MTL::IndexTypeUInt16, m_cubeIndexBuffer, 0, FAB_GRID_TOTAL);
//    }
//    
//    // Axes
//    if (m_axisPipeline && m_axisVertexCount > 0) {
//        encoder->setRenderPipelineState(m_axisPipeline);
//        encoder->setDepthStencilState(m_depthStateOn);
//        encoder->setVertexBuffer(m_axisVertexBuffer, 0, 0);
//        encoder->setVertexBuffer(m_axisUniformBuffer, 0, 1);
//        encoder->setFragmentBuffer(m_axisUniformBuffer, 0, 0);
//        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(m_axisVertexCount));
//    }
//}
//
//// ============================================================================
//// Input
//// ============================================================================
//
//inline bool FabPanel3D::hitTestPanel(simd::float2 screenPos, simd::float2 screenSize) const {
//    simd::float2 panelPos = getPanelScreenPos(screenSize);
//    float totalWidth = panelSize.x + (isInventoryOpen ? inventoryWidth : 0.f);
//    float hw = totalWidth * 0.5f;
//    float hh = panelSize.y * 0.5f;
//    return screenPos.x >= panelPos.x - hw && screenPos.x <= panelPos.x + hw &&
//           screenPos.y >= panelPos.y - hh && screenPos.y <= panelPos.y + hh;
//}
//
//inline bool FabPanel3D::hitTestHeader(simd::float2 screenPos, simd::float2 screenSize) const {
//    simd::float2 panelPos = getPanelScreenPos(screenSize);
//    float totalWidth = panelSize.x + (isInventoryOpen ? inventoryWidth : 0.f);
//    float hw = totalWidth * 0.5f;
//    float top = panelPos.y - panelSize.y * 0.5f;
//    return screenPos.x >= panelPos.x - hw && screenPos.x <= panelPos.x + hw &&
//           screenPos.y >= top && screenPos.y <= top + headerHeight;
//}
//
//inline bool FabPanel3D::hitTest3DView(simd::float2 screenPos, simd::float2 screenSize) const {
//    simd::float2 panelPos = getPanelScreenPos(screenSize);
//    float totalWidth = panelSize.x + (isInventoryOpen ? inventoryWidth : 0.f);
//    float left = panelPos.x - totalWidth * 0.5f + (isInventoryOpen ? inventoryWidth : 0.f);
//    float top = panelPos.y - panelSize.y * 0.5f + headerHeight;
//    return screenPos.x >= left && screenPos.x <= left + panelSize.x &&
//           screenPos.y >= top && screenPos.y <= top + panelSize.y - headerHeight;
//}
//
//inline bool FabPanel3D::hitTestInventoryToggle(simd::float2 screenPos, simd::float2 screenSize) const {
//    simd::float2 panelPos = getPanelScreenPos(screenSize);
//    float totalWidth = panelSize.x + (isInventoryOpen ? inventoryWidth : 0.f);
//    float right = panelPos.x + totalWidth * 0.5f;
//    float top = panelPos.y - panelSize.y * 0.5f;
//    return screenPos.x >= right - 35.f && screenPos.x <= right - 5.f &&
//           screenPos.y >= top + 5.f && screenPos.y <= top + headerHeight - 5.f;
//}
//
//inline void FabPanel3D::onMouseDown(simd::float2 screenPos, simd::float2 screenSize, int button) {
//    if (!hitTestPanel(screenPos, screenSize)) return;
//    
//    if (hitTestInventoryToggle(screenPos, screenSize) && button == 0) {
//        toggleInventory();
//        return;
//    }
//    
//    if (hitTestHeader(screenPos, screenSize) && button == 0) {
//        m_isDraggingPanel = true;
//        simd::float2 panelPos = getPanelScreenPos(screenSize);
//        m_dragOffset = {screenPos.x - panelPos.x, screenPos.y - panelPos.y};
//        return;
//    }
//    
//    if (hitTest3DView(screenPos, screenSize)) {
//        m_isDraggingRotation = true;
//    }
//}
//
//inline void FabPanel3D::onMouseUp(simd::float2 screenPos, simd::float2 screenSize, int button) {
//    m_isDraggingPanel = false;
//    m_isDraggingRotation = false;
//}
//
//inline void FabPanel3D::onMouseMoved(simd::float2 screenPos, simd::float2 screenSize) {
//    // Hover
//}
//
//inline void FabPanel3D::onMouseDragged(simd::float2 screenPos, simd::float2 screenSize, simd::float2 delta, int button) {
//    if (m_isDraggingPanel) {
//        panelPosition = {
//            (screenPos.x - m_dragOffset.x) / screenSize.x,
//            (screenPos.y - m_dragOffset.y) / screenSize.y
//        };
//        
//        float totalWidth = panelSize.x + (isInventoryOpen ? inventoryWidth : 0.f);
//        float marginX = totalWidth * 0.5f / screenSize.x + 0.01f;
//        float marginY = panelSize.y * 0.5f / screenSize.y + 0.01f;
//        panelPosition.x = std::clamp(panelPosition.x, marginX, 1.f - marginX);
//        panelPosition.y = std::clamp(panelPosition.y, marginY, 1.f - marginY);
//        return;
//    }
//    
//    if (m_isDraggingRotation) {
//        targetRotationY += delta.x * 0.01f;
//        targetRotationX = std::clamp(targetRotationX + delta.y * 0.01f, -1.4f, 1.4f);
//    }
//}
//
//inline void FabPanel3D::onMouseScroll(float delta) {
//    targetZoom = std::clamp(targetZoom + delta * 0.15f, 0.4f, 3.0f);
//}
//
//inline void FabPanel3D::onKeyPressed(int keyCode) {
//    switch (keyCode) {
//        case 13: moveSelection(0, 1, 0); break;  // W
//        case 1:  moveSelection(0, -1, 0); break; // S
//        case 0:  moveSelection(-1, 0, 0); break; // A
//        case 2:  moveSelection(1, 0, 0); break;  // D
//        case 12: moveSelection(0, 0, -1); break; // Q
//        case 14: moveSelection(0, 0, 1); break;  // E
//    }
//}
//
//inline void FabPanel3D::moveSelection(int dx, int dy, int dz) {
//    selectedSlot.x = std::clamp((int)selectedSlot.x + dx, 0, (int)FAB_GRID_SIZE - 1);
//    selectedSlot.y = std::clamp((int)selectedSlot.y + dy, 0, (int)FAB_GRID_SIZE - 1);
//    selectedSlot.z = std::clamp((int)selectedSlot.z + dz, 0, (int)FAB_GRID_SIZE - 1);
//}
//
//inline void FabPanel3D::placeItem(FabItemID itemId, uint32_t qty) {
//    grid.placeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z, itemId, qty);
//}
//
//inline FabSlot FabPanel3D::removeItem() {
//    return grid.removeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z);
//}
//
//} // namespace inventoryWindow

//namespace inventoryWindow {
//
//constexpr uint32_t FAB_GRID_SIZE = 5;
//constexpr uint32_t FAB_GRID_TOTAL = FAB_GRID_SIZE * FAB_GRID_SIZE * FAB_GRID_SIZE;
//
//using FabItemID = uint32_t;
//constexpr FabItemID FAB_ITEM_EMPTY = 0;
//
//// ============================================================================
//// Structures de données
//// ============================================================================
//
//struct FabSlot {
//    FabItemID itemId = FAB_ITEM_EMPTY;
//    uint32_t quantity = 0;
//    
//    bool isEmpty() const { return itemId == FAB_ITEM_EMPTY || quantity == 0; }
//    void clear() { itemId = FAB_ITEM_EMPTY; quantity = 0; }
//};
//
//struct FabGrid3D {
//    std::array<FabSlot, FAB_GRID_TOTAL> slots;
//    
//    FabGrid3D() { clear(); }
//    void clear() { for (auto& s : slots) s.clear(); }
//    
//    static size_t index(uint32_t x, uint32_t y, uint32_t z) {
//        return z * FAB_GRID_SIZE * FAB_GRID_SIZE + y * FAB_GRID_SIZE + x;
//    }
//    
//    FabSlot& at(uint32_t x, uint32_t y, uint32_t z) { return slots[index(x, y, z)]; }
//    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const { return slots[index(x, y, z)]; }
//    
//    bool placeItem(uint32_t x, uint32_t y, uint32_t z, FabItemID itemId, uint32_t qty = 1) {
//        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return false;
//        auto& slot = at(x, y, z);
//        if (slot.isEmpty()) { slot.itemId = itemId; slot.quantity = qty; return true; }
//        if (slot.itemId == itemId) { slot.quantity += qty; return true; }
//        return false;
//    }
//    
//    FabSlot removeItem(uint32_t x, uint32_t y, uint32_t z) {
//        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return {};
//        auto& slot = at(x, y, z);
//        FabSlot removed = slot;
//        slot.clear();
//        return removed;
//    }
//};
//
//// ============================================================================
//// Structures GPU
//// ============================================================================
//
//struct FabPanelVertex {
//    simd::float3 position;      // Position 3D du vertex
//    simd::float3 normal;        // Normale pour lighting
//    simd::float2 uv;            // Coordonnées texture
//    simd::float4 color;         // Couleur du slot
//    uint32_t slotIndex;         // Index du slot (0-124)
//    uint32_t flags;             // 0=empty, 1=filled, 2=selected, 4=hovered
//};
//
//struct FabPanelUniforms {
//    simd::float4x4 modelViewProj;
//    simd::float4x4 model;
//    simd::float2 screenSize;
//    simd::float2 panelOrigin;       // Position du panel en NDC (0-1)
//    simd::float2 panelSize;         // Taille du panel en pixels
//    float time;
//    float rotationY;
//    float rotationX;
//    float zoom;
//    uint32_t selectedX;
//    uint32_t selectedY;
//    uint32_t selectedZ;
//    float slotScale;
//    float slotSpacing;
//    float padding[3];
//};
//
//struct FabBackgroundVertex {
//    simd::float2 position;
//    simd::float2 uv;
//};
//
//struct FabBackgroundUniforms {
//    simd::float4x4 projection;
//    simd::float4 panelRect;         // x, y, width, height
//    simd::float4 backgroundColor;
//    simd::float4 borderColor;
//    float cornerRadius;
//    float borderWidth;
//    float time;
//    float padding;
//};
//
//// ============================================================================
//// Classe principale FabPanel3D
//// ============================================================================
//
//class FabPanel3D {
//public:
//    FabPanel3D(MTL::Device* device, MTL::PixelFormat colorFormat,
//               MTL::PixelFormat depthFormat, MTL::Library* shaderLibrary);
//    ~FabPanel3D();
//    
//    // État
//    bool isVisible = false;
//    void show() { isVisible = true; }
//    void hide() { isVisible = false; }
//    void toggle() { isVisible = !isVisible; }
//    
//    // Grille
//    FabGrid3D grid;
//    simd::uint3 selectedSlot = {2, 2, 2};
//    
//    // Rotation/Zoom de la vue 3D
//    float rotationY = 0.65f;    // ~37° horizontal
//    float rotationX = 0.45f;    // ~26° vertical
//    float zoom = 1.0f;
//    
//    // Position du panel (en coordonnées normalisées 0-1)
//    simd::float2 panelPosition = {0.75f, 0.5f};  // Droite de l'écran
//    simd::float2 panelSizePixels = {400.f, 400.f};
//    
//    // Couleurs
//    simd::float4 backgroundColor = {0.06f, 0.07f, 0.10f, 0.92f};
//    simd::float4 borderColor = {0.25f, 0.35f, 0.55f, 1.0f};
//    simd::float4 emptySlotColor = {0.18f, 0.20f, 0.26f, 0.35f};
//    simd::float4 selectedColor = {1.0f, 0.85f, 0.25f, 1.0f};
//    simd::float4 hoveredColor = {0.5f, 0.65f, 0.95f, 0.9f};
//    float cornerRadius = 14.f;
//    float borderWidth = 2.5f;
//    float slotScale = 0.38f;
//    float slotSpacing = 0.9f;
//    
//    // Callback couleur items
//    std::function<simd::float4(FabItemID)> getItemColor = nullptr;
//    
//    // Input
//    void rotate(float deltaX, float deltaY);
//    void moveSelection(int dx, int dy, int dz);
//    void placeItem(FabItemID itemId, uint32_t qty = 1);
//    FabSlot removeItem();
//    void setZoom(float z) { zoom = std::clamp(z, 0.5f, 2.5f); }
//    
//    // Drag du panel
//    bool hitTestPanel(simd::float2 screenPos, simd::float2 screenSize) const;
//    void startDrag(simd::float2 screenPos, simd::float2 screenSize);
//    void updateDrag(simd::float2 screenPos, simd::float2 screenSize);
//    void endDrag();
//    
//    // Update & Render
//    void update(float deltaTime);
//    void render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize);
//    
//private:
//    MTL::Device* m_device;
//    
//    // Pipelines
//    MTL::RenderPipelineState* m_gridPipeline = nullptr;
//    MTL::RenderPipelineState* m_bgPipeline = nullptr;
//    MTL::DepthStencilState* m_depthStateGrid = nullptr;
//    MTL::DepthStencilState* m_depthStateBg = nullptr;
//    
//    // Buffers
//    MTL::Buffer* m_cubeVertexBuffer = nullptr;
//    MTL::Buffer* m_cubeIndexBuffer = nullptr;
//    MTL::Buffer* m_instanceDataBuffer = nullptr;
//    MTL::Buffer* m_gridUniformBuffer = nullptr;
//    MTL::Buffer* m_bgVertexBuffer = nullptr;
//    MTL::Buffer* m_bgUniformBuffer = nullptr;
//    
//    float m_time = 0.f;
//    bool m_isDragging = false;
//    simd::float2 m_dragOffset = {0.f, 0.f};
//    
//    void buildPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* library);
//    void buildBuffers();
//    void updateInstanceData();
//    
//    simd::float4x4 createViewMatrix() const;
//    simd::float4x4 createProjectionMatrix(float aspect) const;
//    simd::float4 defaultItemColor(FabItemID id) const;
//};
//
//// ============================================================================
//// Implémentation
//// ============================================================================
//
//inline FabPanel3D::FabPanel3D(MTL::Device* device, MTL::PixelFormat colorFormat,
//                              MTL::PixelFormat depthFormat, MTL::Library* shaderLibrary)
//    : m_device(device)
//{
//    buildBuffers();
//    buildPipelines(colorFormat, depthFormat, shaderLibrary);
//}
//
//inline FabPanel3D::~FabPanel3D() {
//    if (m_gridPipeline) m_gridPipeline->release();
//    if (m_bgPipeline) m_bgPipeline->release();
//    if (m_depthStateGrid) m_depthStateGrid->release();
//    if (m_depthStateBg) m_depthStateBg->release();
//    if (m_cubeVertexBuffer) m_cubeVertexBuffer->release();
//    if (m_cubeIndexBuffer) m_cubeIndexBuffer->release();
//    if (m_instanceDataBuffer) m_instanceDataBuffer->release();
//    if (m_gridUniformBuffer) m_gridUniformBuffer->release();
//    if (m_bgVertexBuffer) m_bgVertexBuffer->release();
//    if (m_bgUniformBuffer) m_bgUniformBuffer->release();
//}
//
//inline void FabPanel3D::buildBuffers() {
//    // Cube unitaire avec normales et UVs
//    float s = 0.5f;
//    struct CubeVertex { simd::float3 pos; simd::float3 norm; simd::float2 uv; };
//    
//    CubeVertex cubeVerts[] = {
//        // Front (+Z)
//        {{-s,-s, s}, { 0, 0, 1}, {0,0}}, {{ s,-s, s}, { 0, 0, 1}, {1,0}},
//        {{ s, s, s}, { 0, 0, 1}, {1,1}}, {{-s, s, s}, { 0, 0, 1}, {0,1}},
//        // Back (-Z)
//        {{ s,-s,-s}, { 0, 0,-1}, {0,0}}, {{-s,-s,-s}, { 0, 0,-1}, {1,0}},
//        {{-s, s,-s}, { 0, 0,-1}, {1,1}}, {{ s, s,-s}, { 0, 0,-1}, {0,1}},
//        // Top (+Y)
//        {{-s, s, s}, { 0, 1, 0}, {0,0}}, {{ s, s, s}, { 0, 1, 0}, {1,0}},
//        {{ s, s,-s}, { 0, 1, 0}, {1,1}}, {{-s, s,-s}, { 0, 1, 0}, {0,1}},
//        // Bottom (-Y)
//        {{-s,-s,-s}, { 0,-1, 0}, {0,0}}, {{ s,-s,-s}, { 0,-1, 0}, {1,0}},
//        {{ s,-s, s}, { 0,-1, 0}, {1,1}}, {{-s,-s, s}, { 0,-1, 0}, {0,1}},
//        // Right (+X)
//        {{ s,-s, s}, { 1, 0, 0}, {0,0}}, {{ s,-s,-s}, { 1, 0, 0}, {1,0}},
//        {{ s, s,-s}, { 1, 0, 0}, {1,1}}, {{ s, s, s}, { 1, 0, 0}, {0,1}},
//        // Left (-X)
//        {{-s,-s,-s}, {-1, 0, 0}, {0,0}}, {{-s,-s, s}, {-1, 0, 0}, {1,0}},
//        {{-s, s, s}, {-1, 0, 0}, {1,1}}, {{-s, s,-s}, {-1, 0, 0}, {0,1}},
//    };
//    
//    uint16_t cubeIndices[] = {
//        0,1,2, 0,2,3,       // Front
//        4,5,6, 4,6,7,       // Back
//        8,9,10, 8,10,11,    // Top
//        12,13,14, 12,14,15, // Bottom
//        16,17,18, 16,18,19, // Right
//        20,21,22, 20,22,23  // Left
//    };
//    
//    m_cubeVertexBuffer = m_device->newBuffer(cubeVerts, sizeof(cubeVerts), MTL::ResourceStorageModeShared);
//    m_cubeIndexBuffer = m_device->newBuffer(cubeIndices, sizeof(cubeIndices), MTL::ResourceStorageModeShared);
//    
//    // Instance data buffer (couleur + position + flags par slot)
//    struct SlotInstanceData {
//        simd::float3 position;
//        float scale;
//        simd::float4 color;
//        uint32_t flags;
//        uint32_t padding[3];
//    };
//    m_instanceDataBuffer = m_device->newBuffer(sizeof(SlotInstanceData) * FAB_GRID_TOTAL, MTL::ResourceStorageModeShared);
//    m_gridUniformBuffer = m_device->newBuffer(sizeof(FabPanelUniforms), MTL::ResourceStorageModeShared);
//    
//    // Background quad
//    FabBackgroundVertex bgVerts[] = {
//        {{-1,-1}, {0,1}}, {{ 1,-1}, {1,1}}, {{ 1, 1}, {1,0}},
//        {{-1,-1}, {0,1}}, {{ 1, 1}, {1,0}}, {{-1, 1}, {0,0}}
//    };
//    m_bgVertexBuffer = m_device->newBuffer(bgVerts, sizeof(bgVerts), MTL::ResourceStorageModeShared);
//    m_bgUniformBuffer = m_device->newBuffer(sizeof(FabBackgroundUniforms), MTL::ResourceStorageModeShared);
//}
//
//inline void FabPanel3D::buildPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat,
//                                       MTL::Library* library) {
//    NS::Error* error = nullptr;
//    
//    // ===== Grid Pipeline (3D cubes) =====
//    {
//        MTL::Function* vertFn = library->newFunction(MTLSTR("fabGrid3DVertexShader"));
//        MTL::Function* fragFn = library->newFunction(MTLSTR("fabGrid3DFragmentShader"));
//        
//        if (!vertFn || !fragFn) {
//            printf("FabPanel3D: Grid shader functions not found!\n");
//            if (vertFn) vertFn->release();
//            if (fragFn) fragFn->release();
//            return;
//        }
//        
//        MTL::VertexDescriptor* vd = MTL::VertexDescriptor::alloc()->init();
//        // Position
//        vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//        vd->attributes()->object(0)->setOffset(0);
//        vd->attributes()->object(0)->setBufferIndex(0);
//        // Normal
//        vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//        vd->attributes()->object(1)->setOffset(sizeof(simd::float3));
//        vd->attributes()->object(1)->setBufferIndex(0);
//        // UV
//        vd->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
//        vd->attributes()->object(2)->setOffset(sizeof(simd::float3) * 2);
//        vd->attributes()->object(2)->setBufferIndex(0);
//        vd->layouts()->object(0)->setStride(sizeof(simd::float3) * 2 + sizeof(simd::float2));
//        
//        MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
//        pd->setVertexFunction(vertFn);
//        pd->setFragmentFunction(fragFn);
//        pd->setVertexDescriptor(vd);
//        pd->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//        pd->colorAttachments()->object(0)->setBlendingEnabled(true);
//        pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        pd->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
//        pd->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        pd->setDepthAttachmentPixelFormat(depthFormat);
//        
//        m_gridPipeline = m_device->newRenderPipelineState(pd, &error);
//        if (error) printf("FabPanel3D: Grid pipeline error: %s\n", error->localizedDescription()->utf8String());
//        
//        vd->release();
//        pd->release();
//        vertFn->release();
//        fragFn->release();
//    }
//    
//    // ===== Background Pipeline (2D quad) =====
//    {
//        MTL::Function* vertFn = library->newFunction(MTLSTR("fabBackgroundVertexShader"));
//        MTL::Function* fragFn = library->newFunction(MTLSTR("fabBackgroundFragmentShader"));
//        
//        if (!vertFn || !fragFn) {
//            printf("FabPanel3D: Background shader functions not found!\n");
//            if (vertFn) vertFn->release();
//            if (fragFn) fragFn->release();
//            return;
//        }
//        
//        MTL::VertexDescriptor* vd = MTL::VertexDescriptor::alloc()->init();
//        vd->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
//        vd->attributes()->object(0)->setOffset(0);
//        vd->attributes()->object(0)->setBufferIndex(0);
//        vd->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
//        vd->attributes()->object(1)->setOffset(sizeof(simd::float2));
//        vd->attributes()->object(1)->setBufferIndex(0);
//        vd->layouts()->object(0)->setStride(sizeof(FabBackgroundVertex));
//        
//        MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
//        pd->setVertexFunction(vertFn);
//        pd->setFragmentFunction(fragFn);
//        pd->setVertexDescriptor(vd);
//        pd->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//        pd->colorAttachments()->object(0)->setBlendingEnabled(true);
//        pd->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        pd->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        pd->setDepthAttachmentPixelFormat(depthFormat);
//        
//        m_bgPipeline = m_device->newRenderPipelineState(pd, &error);
//        if (error) printf("FabPanel3D: BG pipeline error: %s\n", error->localizedDescription()->utf8String());
//        
//        vd->release();
//        pd->release();
//        vertFn->release();
//        fragFn->release();
//    }
//    
//    // Depth states
//    MTL::DepthStencilDescriptor* dsd = MTL::DepthStencilDescriptor::alloc()->init();
//    
//    // Pour la grille 3D: depth test activé
//    dsd->setDepthCompareFunction(MTL::CompareFunctionLess);
//    dsd->setDepthWriteEnabled(true);
//    m_depthStateGrid = m_device->newDepthStencilState(dsd);
//    
//    // Pour le background: pas de depth
//    dsd->setDepthCompareFunction(MTL::CompareFunctionAlways);
//    dsd->setDepthWriteEnabled(false);
//    m_depthStateBg = m_device->newDepthStencilState(dsd);
//    
//    dsd->release();
//}
//
//inline simd::float4x4 FabPanel3D::createViewMatrix() const {
//    float dist = 7.5f / zoom;
//    float cx = cosf(rotationX), sx = sinf(rotationX);
//    float cy = cosf(rotationY), sy = sinf(rotationY);
//    
//    simd::float3 eye = { dist * sy * cx, dist * sx, dist * cy * cx };
//    simd::float3 center = {0, 0, 0};
//    simd::float3 up = {0, 1, 0};
//    
//    simd::float3 f = simd::normalize(center - eye);
//    simd::float3 s = simd::normalize(simd::cross(f, up));
//    simd::float3 u = simd::cross(s, f);
//    
//    return simd::float4x4{
//        simd::float4{ s.x, u.x, -f.x, 0 },
//        simd::float4{ s.y, u.y, -f.y, 0 },
//        simd::float4{ s.z, u.z, -f.z, 0 },
//        simd::float4{ -simd::dot(s, eye), -simd::dot(u, eye), simd::dot(f, eye), 1 }
//    };
//}
//
//inline simd::float4x4 FabPanel3D::createProjectionMatrix(float aspect) const {
//    // Projection orthographique
//    float size = 3.2f / zoom;
//    float l = -size * aspect, r = size * aspect;
//    float b = -size, t = size;
//    float n = 0.1f, f = 100.0f;
//    
//    return simd::float4x4{
//        simd::float4{ 2.f/(r-l), 0, 0, 0 },
//        simd::float4{ 0, 2.f/(t-b), 0, 0 },
//        simd::float4{ 0, 0, -2.f/(f-n), 0 },
//        simd::float4{ -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1 }
//    };
//}
//
//inline simd::float4 FabPanel3D::defaultItemColor(FabItemID id) const {
//    if (id == FAB_ITEM_EMPTY) return emptySlotColor;
//    // Hash doré pour générer des couleurs variées
//    float r = fmodf(id * 0.618034f, 1.f) * 0.5f + 0.5f;
//    float g = fmodf(id * 0.381966f + 0.3f, 1.f) * 0.5f + 0.4f;
//    float b = fmodf(id * 0.723607f + 0.6f, 1.f) * 0.6f + 0.4f;
//    return {r, g, b, 1.f};
//}
//
//inline void FabPanel3D::updateInstanceData() {
//    struct SlotInstanceData {
//        simd::float3 position;
//        float scale;
//        simd::float4 color;
//        uint32_t flags;
//        uint32_t padding[3];
//    };
//    
//    auto* data = static_cast<SlotInstanceData*>(m_instanceDataBuffer->contents());
//    float offset = (FAB_GRID_SIZE - 1) * slotSpacing * 0.5f;
//    
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
//                size_t idx = FabGrid3D::index(x, y, z);
//                const FabSlot& slot = grid.slots[idx];
//                
//                data[idx].position = {
//                    x * slotSpacing - offset,
//                    y * slotSpacing - offset,
//                    z * slotSpacing - offset
//                };
//                
//                bool isEmpty = slot.isEmpty();
//                bool isSelected = (x == selectedSlot.x && y == selectedSlot.y && z == selectedSlot.z);
//                
//                data[idx].scale = isEmpty ? slotScale * 0.5f : slotScale;
//                data[idx].color = getItemColor ? getItemColor(slot.itemId) : defaultItemColor(slot.itemId);
//                
//                uint32_t flags = 0;
//                if (isEmpty) flags |= 1;
//                if (isSelected) flags |= 2;
//                data[idx].flags = flags;
//            }
//        }
//    }
//}
//
//inline void FabPanel3D::update(float deltaTime) {
//    m_time += deltaTime;
//}
//
//inline void FabPanel3D::render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize) {
//    if (!isVisible) return;
//    if (!m_gridPipeline || !m_bgPipeline) return;
//    
//    updateInstanceData();
//    
//    // ===== 1. Background Panel =====
//    {
//        auto* bgUniforms = static_cast<FabBackgroundUniforms*>(m_bgUniformBuffer->contents());
//        
//        // Projection ortho pour UI (0,0 en haut-gauche)
//        bgUniforms->projection = simd::float4x4{
//            simd::float4{ 2.f / screenSize.x, 0, 0, 0 },
//            simd::float4{ 0, -2.f / screenSize.y, 0, 0 },
//            simd::float4{ 0, 0, 1, 0 },
//            simd::float4{ -1.f, 1.f, 0, 1 }
//        };
//        
//        // Position en pixels
//        float px = panelPosition.x * screenSize.x;
//        float py = panelPosition.y * screenSize.y;
//        bgUniforms->panelRect = { px, py, panelSizePixels.x, panelSizePixels.y };
//        bgUniforms->backgroundColor = backgroundColor;
//        bgUniforms->borderColor = borderColor;
//        bgUniforms->cornerRadius = cornerRadius;
//        bgUniforms->borderWidth = borderWidth;
//        bgUniforms->time = m_time;
//        
//        encoder->setRenderPipelineState(m_bgPipeline);
//        encoder->setDepthStencilState(m_depthStateBg);
//        encoder->setVertexBuffer(m_bgVertexBuffer, 0, 0);
//        encoder->setVertexBuffer(m_bgUniformBuffer, 0, 1);
//        encoder->setFragmentBuffer(m_bgUniformBuffer, 0, 0);
//        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
//    }
//    
//    // ===== 2. Grille 3D =====
//    {
//        float aspect = panelSizePixels.x / panelSizePixels.y;
//        simd::float4x4 view = createViewMatrix();
//        simd::float4x4 proj = createProjectionMatrix(aspect);
//        simd::float4x4 mvp = simd_mul(proj, view);
//        
//        auto* uniforms = static_cast<FabPanelUniforms*>(m_gridUniformBuffer->contents());
//        uniforms->modelViewProj = mvp;
//        uniforms->model = simd::float4x4{
//            simd::float4{1,0,0,0}, simd::float4{0,1,0,0},
//            simd::float4{0,0,1,0}, simd::float4{0,0,0,1}
//        };
//        uniforms->screenSize = screenSize;
//        uniforms->panelOrigin = panelPosition;
//        uniforms->panelSize = panelSizePixels;
//        uniforms->time = m_time;
//        uniforms->rotationY = rotationY;
//        uniforms->rotationX = rotationX;
//        uniforms->zoom = zoom;
//        uniforms->selectedX = selectedSlot.x;
//        uniforms->selectedY = selectedSlot.y;
//        uniforms->selectedZ = selectedSlot.z;
//        uniforms->slotScale = slotScale;
//        uniforms->slotSpacing = slotSpacing;
//        
//        encoder->setRenderPipelineState(m_gridPipeline);
//        encoder->setDepthStencilState(m_depthStateGrid);
//        encoder->setVertexBuffer(m_cubeVertexBuffer, 0, 0);
//        encoder->setVertexBuffer(m_gridUniformBuffer, 0, 1);
//        encoder->setVertexBuffer(m_instanceDataBuffer, 0, 2);
//        encoder->setFragmentBuffer(m_gridUniformBuffer, 0, 0);
//        
//        encoder->drawIndexedPrimitives(
//            MTL::PrimitiveTypeTriangle,
//            36,
//            MTL::IndexTypeUInt16,
//            m_cubeIndexBuffer,
//            0,
//            FAB_GRID_TOTAL
//        );
//    }
//}
//
//// ============================================================================
//// Input
//// ============================================================================
//
//inline void FabPanel3D::rotate(float deltaX, float deltaY) {
//    rotationY += deltaX * 0.008f;
//    rotationX = std::clamp(rotationX + deltaY * 0.008f, -1.3f, 1.3f);
//}
//
//inline void FabPanel3D::moveSelection(int dx, int dy, int dz) {
//    selectedSlot.x = std::clamp((int)selectedSlot.x + dx, 0, (int)FAB_GRID_SIZE - 1);
//    selectedSlot.y = std::clamp((int)selectedSlot.y + dy, 0, (int)FAB_GRID_SIZE - 1);
//    selectedSlot.z = std::clamp((int)selectedSlot.z + dz, 0, (int)FAB_GRID_SIZE - 1);
//}
//
//inline void FabPanel3D::placeItem(FabItemID itemId, uint32_t qty) {
//    grid.placeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z, itemId, qty);
//}
//
//inline FabSlot FabPanel3D::removeItem() {
//    return grid.removeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z);
//}
//
//inline bool FabPanel3D::hitTestPanel(simd::float2 screenPos, simd::float2 screenSize) const {
//    float px = panelPosition.x * screenSize.x;
//    float py = panelPosition.y * screenSize.y;
//    float hw = panelSizePixels.x * 0.5f;
//    float hh = panelSizePixels.y * 0.5f;
//    
//    return screenPos.x >= px - hw && screenPos.x <= px + hw &&
//           screenPos.y >= py - hh && screenPos.y <= py + hh;
//}
//
//inline void FabPanel3D::startDrag(simd::float2 screenPos, simd::float2 screenSize) {
//    m_isDragging = true;
//    m_dragOffset = {
//        screenPos.x / screenSize.x - panelPosition.x,
//        screenPos.y / screenSize.y - panelPosition.y
//    };
//}
//
//inline void FabPanel3D::updateDrag(simd::float2 screenPos, simd::float2 screenSize) {
//    if (!m_isDragging) return;
//    
//    panelPosition = {
//        screenPos.x / screenSize.x - m_dragOffset.x,
//        screenPos.y / screenSize.y - m_dragOffset.y
//    };
//    
//    // Clamp to screen
//    float marginX = panelSizePixels.x * 0.5f / screenSize.x;
//    float marginY = panelSizePixels.y * 0.5f / screenSize.y;
//    panelPosition.x = std::clamp(panelPosition.x, marginX + 0.01f, 1.f - marginX - 0.01f);
//    panelPosition.y = std::clamp(panelPosition.y, marginY + 0.01f, 1.f - marginY - 0.01f);
//}
//
//inline void FabPanel3D::endDrag() {
//    m_isDragging = false;
//}
//
//}

//constexpr uint32_t FAB_GRID_SIZE = 5;
//constexpr uint32_t FAB_GRID_TOTAL = FAB_GRID_SIZE * FAB_GRID_SIZE * FAB_GRID_SIZE;
//
//using ItemID = uint32_t;
//constexpr ItemID ITEM_EMPTY = 0;
//
//struct FabSlot
//{
//    ItemID itemId = ITEM_EMPTY;
//    uint32_t quantity = 0;
//    
//    bool isEmpty() const { return itemId == ITEM_EMPTY || quantity == 0; }
//    void clear() { itemId = ITEM_EMPTY; quantity = 0; }
//    
//    bool operator==(const FabSlot& other) const
//    {
//        return itemId == other.itemId;
//    }
//    
//    bool operator< (const FabSlot& other) const
//    {
//        return itemId < other.itemId;
//    }
//};
//
//struct FabPattern3D
//{
//    std::array<FabSlot, FAB_GRID_TOTAL> slots;
//    simd::uint3 dimensions;
//    simd::uint3 offset;
//    
//    FabPattern3D() : dimensions{0, 0, 0}, offset{0, 0, 0}
//    {
//        for (auto& slot : slots) slot.clear();
//    }
//    
//    // Accès 3D -> 1D
//    static size_t index(uint32_t x, uint32_t y, uint32_t z)
//    {
//        return z * FAB_GRID_SIZE * FAB_GRID_SIZE + y * FAB_GRID_SIZE + x;
//    }
//    
//    FabSlot& at(uint32_t x, uint32_t y, uint32_t z)
//    {
//        return slots[index(x, y, z)];
//    }
//    
//    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const
//    {
//        return slots[index(x, y, z)];
//    }
//    
//    void computeBounds()
//    {
//        simd::uint3 minBound = {FAB_GRID_SIZE, FAB_GRID_SIZE, FAB_GRID_SIZE};
//        simd::uint3 maxBound = {0, 0, 0};
//        
//        for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
//        {
//            for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
//            {
//                for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
//                {
//                    if (!at(x, y, z).isEmpty())
//                    {
//                        minBound.x = std::min(minBound.x, x);
//                        minBound.y = std::min(minBound.y, y);
//                        minBound.z = std::min(minBound.z, z);
//                        maxBound.x = std::max(maxBound.x, x + 1);
//                        maxBound.y = std::max(maxBound.y, y + 1);
//                        maxBound.z = std::max(maxBound.z, z + 1);
//                    }
//                }
//            }
//        }
//        
//        if (maxBound.x > minBound.x)
//        {
//            dimensions = maxBound - minBound;
//            offset = minBound;
//        }
//        else
//        {
//            dimensions = {0, 0, 0};
//            offset = {0, 0, 0};
//        }
//    }
//};
//
//struct FabRecipe
//{
//    std::string id;
//    std::string name;
//    FabPattern3D pattern;
//    ItemID resultItemId;
//    uint32_t resultQuantity;
//    bool shapeless = false;
//    bool mirrorable = true;
//    bool rotatable = true;
//    std::function<bool()> condition = nullptr;
//};
//
//// ============================================================================
//// Grille de craft 3D
//// ============================================================================
//
//class FabGrid3D {
//public:
//    std::array<FabSlot, FAB_GRID_TOTAL> slots;
//    
//    FabGrid3D() { clear(); }
//    
//    void clear() { for (auto& slot : slots) slot.clear(); }
//    
//    static size_t index(uint32_t x, uint32_t y, uint32_t z) {
//        return z * FAB_GRID_SIZE * FAB_GRID_SIZE + y * FAB_GRID_SIZE + x;
//    }
//    
//    FabSlot& at(uint32_t x, uint32_t y, uint32_t z) { return slots[index(x, y, z)]; }
//    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const { return slots[index(x, y, z)]; }
//    
//    bool placeItem(uint32_t x, uint32_t y, uint32_t z, ItemID itemId, uint32_t qty = 1) {
//        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return false;
//        auto& slot = at(x, y, z);
//        if (slot.isEmpty()) {
//            slot.itemId = itemId;
//            slot.quantity = qty;
//            return true;
//        } else if (slot.itemId == itemId) {
//            slot.quantity += qty;
//            return true;
//        }
//        return false;
//    }
//    
//    FabSlot removeItem(uint32_t x, uint32_t y, uint32_t z, uint32_t qty = 1) {
//        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return {};
//        auto& slot = at(x, y, z);
//        FabSlot removed;
//        if (!slot.isEmpty()) {
//            removed.itemId = slot.itemId;
//            removed.quantity = std::min(qty, slot.quantity);
//            slot.quantity -= removed.quantity;
//            if (slot.quantity == 0) slot.itemId = ITEM_EMPTY;
//        }
//        return removed;
//    }
//    
//    FabPattern3D getCurrentPattern() const {
//        FabPattern3D pattern;
//        pattern.slots = slots;
//        pattern.computeBounds();
//        return pattern;
//    }
//    
//    uint32_t countItems() const {
//        uint32_t count = 0;
//        for (const auto& slot : slots) if (!slot.isEmpty()) ++count;
//        return count;
//    }
//};
//
//// ============================================================================
//// Structures GPU
//// ============================================================================
//
//struct FabSlotInstance {
//    simd::float3 position;
//    float scale;
//    simd::float4 color;
//    uint32_t itemId;
//    uint32_t flags;
//    simd::float2 padding;
//};
//
//struct FabUniforms {
//    simd::float4x4 viewProjection;
//    simd::float4x4 model;
//    float time;
//    float gridAlpha;
//    uint32_t selectedX;
//    uint32_t selectedY;
//    uint32_t selectedZ;
//    float padding[3];
//};
//
//struct FabPanelVertex {
//    simd::float2 position;
//    simd::float2 uv;
//};
//
//struct FabPanelUniforms {
//    simd::float4x4 projection;
//    simd::float4 panelRect;      // x, y, width, height (NDC)
//    simd::float4 backgroundColor;
//    simd::float4 borderColor;
//    float borderWidth;
//    float cornerRadius;
//    float padding[2];
//};
//
//// ============================================================================
//// Système FAB principal avec Panel 2D
//// ============================================================================
//
//class FabUI {
//public:
//    FabUI(MTL::Device* device);
//    ~FabUI();
//    
//    // État
//    bool isOpen = false;
//    void open() { isOpen = true; }
//    void close() { isOpen = false; }
//    void toggle() { isOpen = !isOpen; }
//    
//    // Grille
//    FabGrid3D grid;
//    simd::uint3 selectedSlot = {2, 2, 2};
//    
//    // Rotation de la vue 3D interne
//    float rotationY = 0.785f;   // 45° initial
//    float rotationX = 0.524f;   // 30° inclinaison
//    float zoom = 1.0f;
//    
//    // Input
//    void rotate(float deltaX, float deltaY);
//    void moveSelection(int dx, int dy, int dz);
//    void placeItem(ItemID itemId, uint32_t qty = 1);
//    FabSlot removeItem();
//    
//    // Recettes
//    void registerRecipe(const FabRecipe& recipe);
//    std::optional<FabRecipe> checkCraft() const;
//    bool executeCraft(ItemID& resultItem, uint32_t& resultQty);
//    
//    // Rendu - appeler avec la matrice ortho de ton UI
//    void render(MTL::RenderCommandEncoder* encoder,
//                const simd::float4x4& uiProjection,
//                simd::float2 panelCenter,
//                simd::float2 panelSize);
//    
//    // Config visuelle
//    struct VisualConfig {
//        simd::float4 panelBgColor = {0.08f, 0.08f, 0.12f, 0.95f};
//        simd::float4 panelBorderColor = {0.3f, 0.35f, 0.5f, 1.0f};
//        simd::float4 emptySlotColor = {0.2f, 0.22f, 0.28f, 0.4f};
//        simd::float4 selectedColor = {1.0f, 0.85f, 0.2f, 1.0f};
//        simd::float4 hoveredColor = {0.5f, 0.6f, 0.9f, 0.8f};
//        float slotSpacing = 0.85f;
//        float slotScale = 0.35f;
//        float borderWidth = 3.0f;
//        float cornerRadius = 12.0f;
//    } config;
//    
//    // Callback pour couleur des items
//    std::function<simd::float4(ItemID)> getItemColor = nullptr;
//    
//private:
//    MTL::Device* _device;
//    
//    // Pipelines
//    MTL::RenderPipelineState* _gridPipeline = nullptr;
//    MTL::RenderPipelineState* _panelPipeline = nullptr;
//    MTL::DepthStencilState* _depthState = nullptr;
//    MTL::DepthStencilState* _noDepthState = nullptr;
//    
//    // Buffers
//    MTL::Buffer* _cubeVertexBuffer = nullptr;
//    MTL::Buffer* _cubeIndexBuffer = nullptr;
//    MTL::Buffer* _instanceBuffer = nullptr;
//    MTL::Buffer* _gridUniformBuffer = nullptr;
//    MTL::Buffer* _panelVertexBuffer = nullptr;
//    MTL::Buffer* _panelUniformBuffer = nullptr;
//    
//    // Recettes
//    std::vector<FabRecipe> _recipes;
//    
//    float _time = 0.0f;
//    
//    void initMetal();
//    void createBuffers();
//    void createPipelines();
//    
//    void updateInstances();
//    void renderPanel(MTL::RenderCommandEncoder* encoder, const simd::float4x4& uiProj,
//                     simd::float2 center, simd::float2 size);
//    void renderGrid(MTL::RenderCommandEncoder* encoder, simd::float2 center, simd::float2 size);
//    
//    simd::float4x4 createGridViewMatrix() const;
//    simd::float4x4 createGridProjectionMatrix(float aspect) const;
//    
//    // Pattern matching
//    bool matchPattern(const FabPattern3D& gridPattern, const FabPattern3D& recipePattern,
//                      bool shapeless, bool mirrorable, bool rotatable) const;
//    bool matchPatternExact(const FabPattern3D& a, const FabPattern3D& b, simd::int3 offset) const;
//    bool matchPatternShapeless(const FabPattern3D& a, const FabPattern3D& b) const;
//    FabPattern3D rotatePatternY(const FabPattern3D& pattern, int times) const;
//    FabPattern3D mirrorPatternX(const FabPattern3D& pattern) const;
//    FabPattern3D mirrorPatternZ(const FabPattern3D& pattern) const;
//    
//    simd::float4 defaultItemColor(ItemID id) const;
//};
//
//// ============================================================================
//// Implémentation
//// ============================================================================
//
//inline FabUI::FabUI(MTL::Device* device) : _device(device) {
//    initMetal();
//}
//
//inline FabUI::~FabUI() {
//    if (_gridPipeline) _gridPipeline->release();
//    if (_panelPipeline) _panelPipeline->release();
//    if (_depthState) _depthState->release();
//    if (_noDepthState) _noDepthState->release();
//    if (_cubeVertexBuffer) _cubeVertexBuffer->release();
//    if (_cubeIndexBuffer) _cubeIndexBuffer->release();
//    if (_instanceBuffer) _instanceBuffer->release();
//    if (_gridUniformBuffer) _gridUniformBuffer->release();
//    if (_panelVertexBuffer) _panelVertexBuffer->release();
//    if (_panelUniformBuffer) _panelUniformBuffer->release();
//}
//
//inline void FabUI::initMetal() {
//    createBuffers();
//    createPipelines();
//}
//
//inline void FabUI::createBuffers() {
//    // Cube pour les slots
//    float s = 0.5f;
//    float cubeVerts[] = {
//        // pos, normal, uv
//        -s,-s, s,  0, 0, 1,  0,0,   s,-s, s,  0, 0, 1,  1,0,   s, s, s,  0, 0, 1,  1,1,  -s, s, s,  0, 0, 1,  0,1,
//         s,-s,-s,  0, 0,-1,  0,0,  -s,-s,-s,  0, 0,-1,  1,0,  -s, s,-s,  0, 0,-1,  1,1,   s, s,-s,  0, 0,-1,  0,1,
//        -s, s, s,  0, 1, 0,  0,0,   s, s, s,  0, 1, 0,  1,0,   s, s,-s,  0, 1, 0,  1,1,  -s, s,-s,  0, 1, 0,  0,1,
//        -s,-s,-s,  0,-1, 0,  0,0,   s,-s,-s,  0,-1, 0,  1,0,   s,-s, s,  0,-1, 0,  1,1,  -s,-s, s,  0,-1, 0,  0,1,
//         s,-s, s,  1, 0, 0,  0,0,   s,-s,-s,  1, 0, 0,  1,0,   s, s,-s,  1, 0, 0,  1,1,   s, s, s,  1, 0, 0,  0,1,
//        -s,-s,-s, -1, 0, 0,  0,0,  -s,-s, s, -1, 0, 0,  1,0,  -s, s, s, -1, 0, 0,  1,1,  -s, s,-s, -1, 0, 0,  0,1,
//    };
//    uint16_t cubeIdx[] = {
//        0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11, 12,13,14,12,14,15, 16,17,18,16,18,19, 20,21,22,20,22,23
//    };
//    
//    _cubeVertexBuffer = _device->newBuffer(cubeVerts, sizeof(cubeVerts), MTL::ResourceStorageModeShared);
//    _cubeIndexBuffer = _device->newBuffer(cubeIdx, sizeof(cubeIdx), MTL::ResourceStorageModeShared);
//    _instanceBuffer = _device->newBuffer(sizeof(FabSlotInstance) * FAB_GRID_TOTAL, MTL::ResourceStorageModeShared);
//    _gridUniformBuffer = _device->newBuffer(sizeof(FabUniforms), MTL::ResourceStorageModeShared);
//    
//    // Panel quad
//    FabPanelVertex panelVerts[] = {
//        {{-1, -1}, {0, 1}}, {{1, -1}, {1, 1}}, {{1, 1}, {1, 0}}, {{-1, 1}, {0, 0}},
//        {{-1, -1}, {0, 1}}, {{1, 1}, {1, 0}}, {{-1, 1}, {0, 0}}, {{1, -1}, {1, 1}} // triangle strip fallback
//    };
//    _panelVertexBuffer = _device->newBuffer(panelVerts, sizeof(panelVerts), MTL::ResourceStorageModeShared);
//    _panelUniformBuffer = _device->newBuffer(sizeof(FabPanelUniforms), MTL::ResourceStorageModeShared);
//}
//
//inline void FabUI::createPipelines() {
//    const char* shaderSrc = R"(
//        #include <metal_stdlib>
//        using namespace metal;
//        
//        // ========== GRID 3D SHADERS ==========
//        struct GridVertexIn {
//            float3 position [[attribute(0)]];
//            float3 normal   [[attribute(1)]];
//            float2 uv       [[attribute(2)]];
//        };
//        
//        struct SlotInstance {
//            float3 position;
//            float scale;
//            float4 color;
//            uint itemId;
//            uint flags;
//            float2 padding;
//        };
//        
//        struct GridUniforms {
//            float4x4 viewProjection;
//            float4x4 model;
//            float time;
//            float gridAlpha;
//            uint selectedX;
//            uint selectedY;
//            uint selectedZ;
//        };
//        
//        struct GridVertexOut {
//            float4 position [[position]];
//            float3 worldPos;
//            float3 normal;
//            float2 uv;
//            float4 color;
//            uint itemId;
//            uint flags;
//            float depth;
//        };
//        
//        vertex GridVertexOut fab_grid_vertex(
//            GridVertexIn in [[stage_in]],
//            constant GridUniforms& uniforms [[buffer(1)]],
//            constant SlotInstance* instances [[buffer(2)]],
//            uint instanceId [[instance_id]]
//        ) {
//            SlotInstance inst = instances[instanceId];
//            
//            float3 localPos = in.position * inst.scale + inst.position;
//            float4 worldPos = uniforms.model * float4(localPos, 1.0);
//            
//            // Subtle breathing pour items
//            if (inst.itemId != 0) {
//                float pulse = sin(uniforms.time * 1.5 + float(instanceId) * 0.3) * 0.015;
//                worldPos.xyz += in.normal * pulse;
//            }
//            
//            // Selection pulse
//            if ((inst.flags & 1u) != 0) {
//                float selectPulse = sin(uniforms.time * 4.0) * 0.03 + 0.02;
//                worldPos.xyz += in.normal * selectPulse;
//            }
//            
//            GridVertexOut out;
//            out.position = uniforms.viewProjection * worldPos;
//            out.worldPos = worldPos.xyz;
//            out.normal = normalize((uniforms.model * float4(in.normal, 0.0)).xyz);
//            out.uv = in.uv;
//            out.color = inst.color;
//            out.itemId = inst.itemId;
//            out.flags = inst.flags;
//            out.depth = out.position.z / out.position.w;
//            return out;
//        }
//        
//        fragment float4 fab_grid_fragment(
//            GridVertexOut in [[stage_in]],
//            constant GridUniforms& uniforms [[buffer(1)]]
//        ) {
//            // Lighting
//            float3 lightDir = normalize(float3(0.4, 0.8, 0.5));
//            float NdotL = max(dot(in.normal, lightDir), 0.0);
//            float ambient = 0.35;
//            float diffuse = NdotL * 0.65;
//            
//            float4 baseColor = in.color;
//            
//            // Slot vide - wireframe style
//            if (in.itemId == 0) {
//                float2 uvEdge = abs(in.uv - 0.5) * 2.0;
//                float edge = max(uvEdge.x, uvEdge.y);
//                if (edge < 0.85) {
//                    discard_fragment();
//                }
//                baseColor.a = 0.25 + sin(uniforms.time * 0.5) * 0.05;
//            }
//            
//            // Selection highlight
//            if ((in.flags & 1u) != 0) {
//                float glow = sin(uniforms.time * 4.0) * 0.3 + 0.7;
//                baseColor = mix(baseColor, float4(1.0, 0.9, 0.3, 1.0), 0.4 * glow);
//            }
//            
//            // Hovered
//            if ((in.flags & 2u) != 0) {
//                baseColor = mix(baseColor, float4(0.5, 0.7, 1.0, 1.0), 0.3);
//            }
//            
//            float3 finalColor = baseColor.rgb * (ambient + diffuse);
//            
//            // Edge darkening pour depth
//            float2 uvEdge = abs(in.uv - 0.5) * 2.0;
//            float edge = max(uvEdge.x, uvEdge.y);
//            if (edge > 0.92 && in.itemId != 0) {
//                finalColor *= 0.6;
//            }
//            
//            // Depth fog subtil
//            float fog = smoothstep(0.3, 1.0, in.depth) * 0.15;
//            finalColor = mix(finalColor, float3(0.1, 0.1, 0.15), fog);
//            
//            return float4(finalColor, baseColor.a * uniforms.gridAlpha);
//        }
//        
//        // ========== PANEL 2D SHADERS ==========
//        struct PanelVertexIn {
//            float2 position [[attribute(0)]];
//            float2 uv       [[attribute(1)]];
//        };
//        
//        struct PanelUniforms {
//            float4x4 projection;
//            float4 panelRect;
//            float4 backgroundColor;
//            float4 borderColor;
//            float borderWidth;
//            float cornerRadius;
//        };
//        
//        struct PanelVertexOut {
//            float4 position [[position]];
//            float2 uv;
//            float2 localPos;
//        };
//        
//        vertex PanelVertexOut fab_panel_vertex(
//            PanelVertexIn in [[stage_in]],
//            constant PanelUniforms& uniforms [[buffer(1)]]
//        ) {
//            float2 pos = in.position * uniforms.panelRect.zw * 0.5 + uniforms.panelRect.xy;
//            
//            PanelVertexOut out;
//            out.position = uniforms.projection * float4(pos, 0.0, 1.0);
//            out.uv = in.uv;
//            out.localPos = in.position * uniforms.panelRect.zw * 0.5;
//            return out;
//        }
//        
//        float roundedBoxSDF(float2 p, float2 size, float radius) {
//            float2 q = abs(p) - size + radius;
//            return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
//        }
//        
//        fragment float4 fab_panel_fragment(
//            PanelVertexOut in [[stage_in]],
//            constant PanelUniforms& uniforms [[buffer(1)]]
//        ) {
//            float2 halfSize = uniforms.panelRect.zw * 0.5;
//            float d = roundedBoxSDF(in.localPos, halfSize, uniforms.cornerRadius);
//            
//            if (d > 0.0) discard_fragment();
//            
//            float4 color = uniforms.backgroundColor;
//            
//            // Border
//            float borderDist = abs(d) - uniforms.borderWidth;
//            if (d > -uniforms.borderWidth) {
//                float t = smoothstep(-1.0, 0.0, borderDist);
//                color = mix(uniforms.borderColor, color, t);
//            }
//            
//            // Inner shadow/gradient
//            float innerGrad = smoothstep(0.0, halfSize.y * 0.8, -in.localPos.y + halfSize.y * 0.3);
//            color.rgb *= 0.85 + innerGrad * 0.15;
//            
//            return color;
//        }
//    )";
//    
//    NS::Error* error = nullptr;
//    MTL::Library* library = _device->newLibrary(NS::String::string(shaderSrc, NS::UTF8StringEncoding), nullptr, &error);
//    
//    if (!library) {
//        printf("FabUI: Shader compilation failed\n");
//        return;
//    }
//    
//    // Grid pipeline
//    {
//        auto vertFn = library->newFunction(NS::String::string("fab_grid_vertex", NS::UTF8StringEncoding));
//        auto fragFn = library->newFunction(NS::String::string("fab_grid_fragment", NS::UTF8StringEncoding));
//        
//        auto vertDesc = MTL::VertexDescriptor::alloc()->init();
//        vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//        vertDesc->attributes()->object(0)->setOffset(0);
//        vertDesc->attributes()->object(0)->setBufferIndex(0);
//        vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//        vertDesc->attributes()->object(1)->setOffset(12);
//        vertDesc->attributes()->object(1)->setBufferIndex(0);
//        vertDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
//        vertDesc->attributes()->object(2)->setOffset(24);
//        vertDesc->attributes()->object(2)->setBufferIndex(0);
//        vertDesc->layouts()->object(0)->setStride(32);
//        
//        auto pipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//        pipeDesc->setVertexFunction(vertFn);
//        pipeDesc->setFragmentFunction(fragFn);
//        pipeDesc->setVertexDescriptor(vertDesc);
//        pipeDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
//        pipeDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
//        pipeDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        pipeDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        pipeDesc->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
//        pipeDesc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        pipeDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
//        
//        _gridPipeline = _device->newRenderPipelineState(pipeDesc, &error);
//        
//        vertDesc->release();
//        pipeDesc->release();
//        vertFn->release();
//        fragFn->release();
//    }
//    
//    // Panel pipeline
//    {
//        auto vertFn = library->newFunction(NS::String::string("fab_panel_vertex", NS::UTF8StringEncoding));
//        auto fragFn = library->newFunction(NS::String::string("fab_panel_fragment", NS::UTF8StringEncoding));
//        
//        auto vertDesc = MTL::VertexDescriptor::alloc()->init();
//        vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
//        vertDesc->attributes()->object(0)->setOffset(0);
//        vertDesc->attributes()->object(0)->setBufferIndex(0);
//        vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
//        vertDesc->attributes()->object(1)->setOffset(8);
//        vertDesc->attributes()->object(1)->setBufferIndex(0);
//        vertDesc->layouts()->object(0)->setStride(16);
//        
//        auto pipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//        pipeDesc->setVertexFunction(vertFn);
//        pipeDesc->setFragmentFunction(fragFn);
//        pipeDesc->setVertexDescriptor(vertDesc);
//        pipeDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
//        pipeDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
//        pipeDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        pipeDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        pipeDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
//        
//        _panelPipeline = _device->newRenderPipelineState(pipeDesc, &error);
//        
//        vertDesc->release();
//        pipeDesc->release();
//        vertFn->release();
//        fragFn->release();
//    }
//    
//    // Depth states
//    {
//        auto depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
//        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
//        depthDesc->setDepthWriteEnabled(true);
//        _depthState = _device->newDepthStencilState(depthDesc);
//        
//        depthDesc->setDepthCompareFunction(MTL::CompareFunctionAlways);
//        depthDesc->setDepthWriteEnabled(false);
//        _noDepthState = _device->newDepthStencilState(depthDesc);
//        
//        depthDesc->release();
//    }
//    
//    library->release();
//}
//
//inline simd::float4x4 FabUI::createGridViewMatrix() const {
//    // Vue isométrique-ish avec rotation utilisateur
//    float dist = 8.0f / zoom;
//    
//    float cx = cosf(rotationX), sx = sinf(rotationX);
//    float cy = cosf(rotationY), sy = sinf(rotationY);
//    
//    simd::float3 eye = {
//        dist * sy * cx,
//        dist * sx,
//        dist * cy * cx
//    };
//    
//    simd::float3 center = {0, 0, 0};
//    simd::float3 up = {0, 1, 0};
//    
//    simd::float3 f = simd::normalize(center - eye);
//    simd::float3 s = simd::normalize(simd::cross(f, up));
//    simd::float3 u = simd::cross(s, f);
//    
//    simd::float4x4 view = {
//        simd::float4{ s.x, u.x, -f.x, 0 },
//        simd::float4{ s.y, u.y, -f.y, 0 },
//        simd::float4{ s.z, u.z, -f.z, 0 },
//        simd::float4{ -simd::dot(s, eye), -simd::dot(u, eye), simd::dot(f, eye), 1 }
//    };
//    
//    return view;
//}
//
//inline simd::float4x4 FabUI::createGridProjectionMatrix(float aspect) const {
//    // Projection orthographique pour vue "renfermée"
//    float size = 3.5f / zoom;
//    float l = -size * aspect, r = size * aspect;
//    float b = -size, t = size;
//    float n = 0.1f, f = 50.0f;
//    
//    return simd::float4x4{
//        simd::float4{ 2.0f/(r-l), 0, 0, 0 },
//        simd::float4{ 0, 2.0f/(t-b), 0, 0 },
//        simd::float4{ 0, 0, -2.0f/(f-n), 0 },
//        simd::float4{ -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1 }
//    };
//}
//
//inline simd::float4 FabUI::defaultItemColor(ItemID id) const {
//    if (id == ITEM_EMPTY) return config.emptySlotColor;
//    float r = fmodf(id * 0.618034f, 1.0f) * 0.6f + 0.4f;
//    float g = fmodf(id * 0.381966f + 0.3f, 1.0f) * 0.6f + 0.3f;
//    float b = fmodf(id * 0.723607f + 0.6f, 1.0f) * 0.7f + 0.3f;
//    return {r, g, b, 1.0f};
//}
//
//inline void FabUI::updateInstances()
//{
//    auto* instances = static_cast<FabSlotInstance *>(_instanceBuffer->contents());
//    
//    float spacing = config.slotSpacing;
//    float offset = (FAB_GRID_SIZE - 1) * spacing * 0.5f;
//    
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
//                size_t idx = FabGrid3D::index(x, y, z);
//                const FabSlot& slot = grid.slots[idx];
//                
//                instances[idx].position = {
//                    x * spacing - offset,
//                    y * spacing - offset,
//                    z * spacing - offset
//                };
//                
//                instances[idx].scale = slot.isEmpty() ? config.slotScale * 0.6f : config.slotScale;
//                instances[idx].color = getItemColor ? getItemColor(slot.itemId) : defaultItemColor(slot.itemId);
//                instances[idx].itemId = slot.itemId;
//                
//                // Flags
//                uint32_t flags = 0;
//                if (x == selectedSlot.x && y == selectedSlot.y && z == selectedSlot.z) {
//                    flags |= 1; // selected
//                }
//                instances[idx].flags = flags;
//            }
//        }
//    }
//}
//
//inline void FabUI::renderPanel(MTL::RenderCommandEncoder* encoder,
//                               const simd::float4x4& uiProj,
//                               simd::float2 center, simd::float2 size) {
//    auto* uniforms = static_cast<FabPanelUniforms*>(_panelUniformBuffer->contents());
//    uniforms->projection = uiProj;
//    uniforms->panelRect = {center.x, center.y, size.x, size.y};
//    uniforms->backgroundColor = config.panelBgColor;
//    uniforms->borderColor = config.panelBorderColor;
//    uniforms->borderWidth = config.borderWidth;
//    uniforms->cornerRadius = config.cornerRadius;
//    
//    encoder->setRenderPipelineState(_panelPipeline);
//    encoder->setDepthStencilState(_noDepthState);
//    encoder->setVertexBuffer(_panelVertexBuffer, 0, 0);
//    encoder->setVertexBuffer(_panelUniformBuffer, 0, 1);
//    encoder->setFragmentBuffer(_panelUniformBuffer, 0, 1);
//    encoder->drawPrimitives(MTL::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
//}
//
//inline void FabUI::renderGrid(MTL::RenderCommandEncoder* encoder,
//                              simd::float2 center, simd::float2 size) {
//    updateInstances();
//    
//    float aspect = size.x / size.y;
//    simd::float4x4 view = createGridViewMatrix();
//    simd::float4x4 proj = createGridProjectionMatrix(aspect);
//    simd::float4x4 viewProj = simd_mul(proj, view);
//    
//    // Model matrix (identité, la grille est centrée à l'origine)
//    simd::float4x4 model = {
//        simd::float4{1, 0, 0, 0},
//        simd::float4{0, 1, 0, 0},
//        simd::float4{0, 0, 1, 0},
//        simd::float4{0, 0, 0, 1}
//    };
//    
//    auto* uniforms = static_cast<FabUniforms*>(_gridUniformBuffer->contents());
//    uniforms->viewProjection = viewProj;
//    uniforms->model = model;
//    uniforms->time = _time;
//    uniforms->gridAlpha = 1.0f;
//    uniforms->selectedX = selectedSlot.x;
//    uniforms->selectedY = selectedSlot.y;
//    uniforms->selectedZ = selectedSlot.z;
//    
//    encoder->setRenderPipelineState(_gridPipeline);
//    encoder->setDepthStencilState(_depthState);
//    encoder->setVertexBuffer(_cubeVertexBuffer, 0, 0);
//    encoder->setVertexBuffer(_gridUniformBuffer, 0, 1);
//    encoder->setVertexBuffer(_instanceBuffer, 0, 2);
//    encoder->setFragmentBuffer(_gridUniformBuffer, 0, 1);
//    
//    encoder->drawIndexedPrimitives(
//        MTL::PrimitiveTypeTriangle,
//        36,
//        MTL::IndexTypeUInt16,
//        _cubeIndexBuffer,
//        0,
//        FAB_GRID_TOTAL
//    );
//}
//
//inline void FabUI::render(MTL::RenderCommandEncoder* encoder,
//                          const simd::float4x4& uiProjection,
//                          simd::float2 panelCenter,
//                          simd::float2 panelSize) {
//    if (!isOpen) return;
//    
//    _time += 0.016f;
//    
//    // 1. Panel background
//    renderPanel(encoder, uiProjection, panelCenter, panelSize);
//    
//    // 2. Grille 3D à l'intérieur (utilise sa propre view/proj)
//    // Note: le viewport reste le même, on utilise juste une projection différente
//    renderGrid(encoder, panelCenter, panelSize);
//}
//
//// ============================================================================
//// Input
//// ============================================================================
//
//inline void FabUI::rotate(float deltaX, float deltaY) {
//    rotationY += deltaX * 0.01f;
//    rotationX = std::clamp(rotationX + deltaY * 0.01f, -1.2f, 1.2f);
//}
//
//inline void FabUI::moveSelection(int dx, int dy, int dz) {
//    selectedSlot.x = std::clamp((int)selectedSlot.x + dx, 0, (int)FAB_GRID_SIZE - 1);
//    selectedSlot.y = std::clamp((int)selectedSlot.y + dy, 0, (int)FAB_GRID_SIZE - 1);
//    selectedSlot.z = std::clamp((int)selectedSlot.z + dz, 0, (int)FAB_GRID_SIZE - 1);
//}
//
//inline void FabUI::placeItem(ItemID itemId, uint32_t qty) {
//    grid.placeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z, itemId, qty);
//}
//
//inline FabSlot FabUI::removeItem() {
//    return grid.removeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z);
//}
//
//// ============================================================================
//// Recettes
//// ============================================================================
//
//inline void FabUI::registerRecipe(const FabRecipe& recipe) {
//    for (const auto& r : _recipes) if (r.id == recipe.id) return;
//    FabRecipe r = recipe;
//    r.pattern.computeBounds();
//    _recipes.push_back(r);
//}
//
//inline std::optional<FabRecipe> FabUI::checkCraft() const {
//    FabPattern3D gridPattern = grid.getCurrentPattern();
//    if (gridPattern.dimensions.x == 0) return std::nullopt;
//    
//    for (const auto& recipe : _recipes) {
//        if (recipe.condition && !recipe.condition()) continue;
//        if (matchPattern(gridPattern, recipe.pattern, recipe.shapeless,
//                        recipe.mirrorable, recipe.rotatable)) {
//            return recipe;
//        }
//    }
//    return std::nullopt;
//}
//
//inline bool FabUI::executeCraft(ItemID& resultItem, uint32_t& resultQty) {
//    auto recipe = checkCraft();
//    if (!recipe) return false;
//    
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
//                auto& slot = grid.at(x, y, z);
//                if (!slot.isEmpty()) {
//                    slot.quantity--;
//                    if (slot.quantity == 0) slot.itemId = ITEM_EMPTY;
//                }
//            }
//        }
//    }
//    
//    resultItem = recipe->resultItemId;
//    resultQty = recipe->resultQuantity;
//    return true;
//}
//
//// ============================================================================
//// Pattern Matching (même logique que avant)
//// ============================================================================
//
//inline bool FabUI::matchPattern(const FabPattern3D& gridPattern, const FabPattern3D& recipePattern,
//                                bool shapeless, bool mirrorable, bool rotatable) const {
//    if (shapeless) return matchPatternShapeless(gridPattern, recipePattern);
//    
//    std::vector<FabPattern3D> variants = {recipePattern};
//    
//    if (rotatable) {
//        variants.push_back(rotatePatternY(recipePattern, 1));
//        variants.push_back(rotatePatternY(recipePattern, 2));
//        variants.push_back(rotatePatternY(recipePattern, 3));
//    }
//    
//    if (mirrorable) {
//        size_t count = variants.size();
//        for (size_t i = 0; i < count; ++i) {
//            variants.push_back(mirrorPatternX(variants[i]));
//            variants.push_back(mirrorPatternZ(variants[i]));
//        }
//    }
//    
//    for (const auto& variant : variants) {
//        int maxOffsetX = FAB_GRID_SIZE - variant.dimensions.x;
//        int maxOffsetY = FAB_GRID_SIZE - variant.dimensions.y;
//        int maxOffsetZ = FAB_GRID_SIZE - variant.dimensions.z;
//        
//        for (int oz = 0; oz <= maxOffsetZ; ++oz) {
//            for (int oy = 0; oy <= maxOffsetY; ++oy) {
//                for (int ox = 0; ox <= maxOffsetX; ++ox) {
//                    simd::int3 offset = {
//                        ox - (int)gridPattern.offset.x + (int)variant.offset.x,
//                        oy - (int)gridPattern.offset.y + (int)variant.offset.y,
//                        oz - (int)gridPattern.offset.z + (int)variant.offset.z
//                    };
//                    if (matchPatternExact(gridPattern, variant, offset)) return true;
//                }
//            }
//        }
//    }
//    return false;
//}
//
//inline bool FabUI::matchPatternExact(const FabPattern3D& grid, const FabPattern3D& recipe, simd::int3 offset) const {
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
//                int rx = (int)x - offset.x;
//                int ry = (int)y - offset.y;
//                int rz = (int)z - offset.z;
//                
//                bool inBounds = rx >= 0 && rx < FAB_GRID_SIZE &&
//                               ry >= 0 && ry < FAB_GRID_SIZE &&
//                               rz >= 0 && rz < FAB_GRID_SIZE;
//                
//                const FabSlot& gridSlot = grid.at(x, y, z);
//                
//                if (inBounds) {
//                    if (gridSlot.itemId != recipe.at(rx, ry, rz).itemId) return false;
//                } else {
//                    if (!gridSlot.isEmpty()) return false;
//                }
//            }
//        }
//    }
//    return true;
//}
//
//inline bool FabUI::matchPatternShapeless(const FabPattern3D& grid, const FabPattern3D& recipe) const {
//    std::unordered_map<ItemID, uint32_t> gridItems, recipeItems;
//    for (const auto& slot : grid.slots) if (!slot.isEmpty()) gridItems[slot.itemId]++;
//    for (const auto& slot : recipe.slots) if (!slot.isEmpty()) recipeItems[slot.itemId]++;
//    return gridItems == recipeItems;
//}
//
//inline FabPattern3D FabUI::rotatePatternY(const FabPattern3D& pattern, int times) const {
//    FabPattern3D result;
//    times = times % 4;
//    if (times == 0) return pattern;
//    
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
//                uint32_t nx = x, nz = z;
//                for (int t = 0; t < times; ++t) {
//                    uint32_t tmp = nx;
//                    nx = FAB_GRID_SIZE - 1 - nz;
//                    nz = tmp;
//                }
//                result.at(nx, y, nz) = pattern.at(x, y, z);
//            }
//        }
//    }
//    result.computeBounds();
//    return result;
//}
//
//inline FabPattern3D FabUI::mirrorPatternX(const FabPattern3D& pattern) const {
//    FabPattern3D result;
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
//                result.at(FAB_GRID_SIZE - 1 - x, y, z) = pattern.at(x, y, z);
//    result.computeBounds();
//    return result;
//}
//
//inline FabPattern3D FabUI::mirrorPatternZ(const FabPattern3D& pattern) const {
//    FabPattern3D result;
//    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
//        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
//            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
//                result.at(x, y, FAB_GRID_SIZE - 1 - z) = pattern.at(x, y, z);
//    result.computeBounds();
//    return result;
//}









//using namespace Fabieur;
//
//enum ItemIDs : ItemID {
//    ITEM_NONE = 0,
//    ITEM_IRON_INGOT = 1,
//    ITEM_GOLD_INGOT = 2,
//    ITEM_DIAMOND = 3,
//    ITEM_WOOD = 4,
//    ITEM_STONE = 5,
//    ITEM_REDSTONE = 6,
//    ITEM_IRON_BLOCK = 100,
//    ITEM_GOLD_BLOCK = 101,
//    ITEM_DIAMOND_BLOCK = 102,
//    ITEM_FUSION_CORE = 200,
//    ITEM_POWER_CUBE = 201,
//};
//
//inline void setupExampleRecipes(FabSystem3D& craftSystem)
//{
//    {
//        FabRecipe recipe;
//        recipe.id = "iron_block";
//        recipe.name = "Bloc de Fer";
//        recipe.resultItemId = ITEM_IRON_BLOCK;
//        recipe.resultQuantity = 1;
//        recipe.shapeless = false;
//        recipe.mirrorable = true;
//        recipe.rotatable = true;
//        
//        // Pattern 3x3 sur le plan XY (z=0)
//        for (int y = 0; y < 3; ++y)
//        {
//            for (int x = 0; x < 3; ++x)
//            {
//                recipe.pattern.at(x, y, 0).itemId = ITEM_IRON_INGOT;
//                recipe.pattern.at(x, y, 0).quantity = 1;
//            }
//        }
//        recipe.pattern.computeBounds();
//        
//        craftSystem.registerRecipe(recipe);
//    }
//    
//    // ========================================
//    // Recette 2: Fusion Core (cube 3x3x3 avec diamant au centre)
//    // ========================================
//    {
//        FabRecipe recipe;
//        recipe.id = "fusion_core";
//        recipe.name = "Fusion Core";
//        recipe.resultItemId = ITEM_FUSION_CORE;
//        recipe.resultQuantity = 1;
//        recipe.shapeless = false;
//        
//        // Coque externe en or
//        for (int z = 0; z < 3; ++z) {
//            for (int y = 0; y < 3; ++y) {
//                for (int x = 0; x < 3; ++x) {
//                    // Faces externes seulement
//                    if (x == 0 || x == 2 || y == 0 || y == 2 || z == 0 || z == 2) {
//                        recipe.pattern.at(x, y, z).itemId = ITEM_GOLD_INGOT;
//                        recipe.pattern.at(x, y, z).quantity = 1;
//                    }
//                }
//            }
//        }
//        
//        // Diamant au centre
//        recipe.pattern.at(1, 1, 1).itemId = ITEM_DIAMOND;
//        recipe.pattern.at(1, 1, 1).quantity = 1;
//        
//        recipe.pattern.computeBounds();
//        craftSystem.registerRecipe(recipe);
//    }
//    
//    // ========================================
//    // Recette 3: Power Cube (5x5x5 complet!)
//    // ========================================
//    {
//        FabRecipe recipe;
//        recipe.id = "power_cube";
//        recipe.name = "Power Cube Ultime";
//        recipe.resultItemId = ITEM_POWER_CUBE;
//        recipe.resultQuantity = 1;
//        recipe.shapeless = false;
//        recipe.mirrorable = false;
//        recipe.rotatable = false;
//        
//        // Structure en couches
//        for (int z = 0; z < 5; ++z) {
//            for (int y = 0; y < 5; ++y) {
//                for (int x = 0; x < 5; ++x) {
//                    // Coins = diamants
//                    bool isCorner = (x == 0 || x == 4) && (y == 0 || y == 4) && (z == 0 || z == 4);
//                    // Arêtes = or
//                    bool isEdge = ((x == 0 || x == 4) && (y == 0 || y == 4)) ||
//                                  ((x == 0 || x == 4) && (z == 0 || z == 4)) ||
//                                  ((y == 0 || y == 4) && (z == 0 || z == 4));
//                    // Centre de chaque face = redstone
//                    bool isFaceCenter = (x == 2 && (y == 0 || y == 4) && z == 2) ||
//                                        (y == 2 && (x == 0 || x == 4) && z == 2) ||
//                                        (z == 2 && x == 2 && (y == 0 || y == 4));
//                    // Coeur = fusion core
//                    bool isCore = (x == 2 && y == 2 && z == 2);
//                    
//                    if (isCore) {
//                        recipe.pattern.at(x, y, z).itemId = ITEM_FUSION_CORE;
//                    } else if (isCorner) {
//                        recipe.pattern.at(x, y, z).itemId = ITEM_DIAMOND;
//                    } else if (isEdge) {
//                        recipe.pattern.at(x, y, z).itemId = ITEM_GOLD_INGOT;
//                    } else if (isFaceCenter) {
//                        recipe.pattern.at(x, y, z).itemId = ITEM_REDSTONE;
//                    } else {
//                        recipe.pattern.at(x, y, z).itemId = ITEM_IRON_INGOT;
//                    }
//                    recipe.pattern.at(x, y, z).quantity = 1;
//                }
//            }
//        }
//        
//        recipe.pattern.computeBounds();
//        craftSystem.registerRecipe(recipe);
//    }
//    
//    // ========================================
//    // Recette 4: Shapeless (n'importe quel arrangement)
//    // ========================================
//    {
//        FabRecipe recipe;
//        recipe.id = "mixed_alloy";
//        recipe.name = "Alliage Mixte";
//        recipe.resultItemId = 300;
//        recipe.resultQuantity = 4;
//        recipe.shapeless = true; // L'arrangement n'importe pas!
//        
//        // 2 fer + 2 or + 1 diamant = alliage
//        recipe.pattern.at(0, 0, 0).itemId = ITEM_IRON_INGOT;
//        recipe.pattern.at(1, 0, 0).itemId = ITEM_IRON_INGOT;
//        recipe.pattern.at(2, 0, 0).itemId = ITEM_GOLD_INGOT;
//        recipe.pattern.at(3, 0, 0).itemId = ITEM_GOLD_INGOT;
//        recipe.pattern.at(4, 0, 0).itemId = ITEM_DIAMOND;
//        
//        for (int i = 0; i < 5; ++i) {
//            recipe.pattern.at(i, 0, 0).quantity = 1;
//        }
//        
//        recipe.pattern.computeBounds();
//        craftSystem.registerRecipe(recipe);
//    }
//}
//
//class FabUI
//{
//public:
//    FabSystem3D craftSystem;
//    FabGrid3D currentGrid;
//    
//    simd::uint3 selectedSlot = {2, 2, 2};
//    bool isOpen = false;
//    
//    FabUI(MTL::Device* device) : craftSystem(device) {
//        setupExampleRecipes(craftSystem);
//    }
//    
//    void open() { isOpen = true; }
//    void close() { isOpen = false; currentGrid.clear(); }
//    
//    void placeItemFromInventory(ItemID itemId, uint32_t qty = 1)
//    {
//        currentGrid.placeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z, itemId, qty);
//        checkRecipe();
//    }
//    
//    FabSlot removeToInventory()
//    {
//        auto slot = currentGrid.removeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z);
//        checkRecipe();
//        return slot;
//    }
//    
//    void moveSelection(int dx, int dy, int dz)
//    {
//        int nx = (int)selectedSlot.x + dx;
//        int ny = (int)selectedSlot.y + dy;
//        int nz = (int)selectedSlot.z + dz;
//        
//        selectedSlot.x = std::clamp(nx, 0, (int)FAB_GRID_SIZE - 1);
//        selectedSlot.y = std::clamp(ny, 0, (int)FAB_GRID_SIZE - 1);
//        selectedSlot.z = std::clamp(nz, 0, (int)FAB_GRID_SIZE - 1);
//    }
//    
//    std::optional<FabRecipe> currentRecipeMatch;
//    
//    void checkRecipe()
//    {
//        currentRecipeMatch = craftSystem.checkCraft(currentGrid);
//        if (currentRecipeMatch) {
//            printf("Recette trouvée: %s (produit %u x item #%u)\n",
//                   currentRecipeMatch->name.c_str(),
//                   currentRecipeMatch->resultQuantity,
//                   currentRecipeMatch->resultItemId);
//        }
//    }
//    
//    bool doCraft(ItemID& resultItem, uint32_t& resultQty)
//    {
//        if (craftSystem.executeCraft(currentGrid, resultItem, resultQty))
//        {
//            currentRecipeMatch = std::nullopt;
//            checkRecipe(); // Re-vérifier au cas où il reste des items
//            
//            return true;
//        }
//        return false;
//    }
//    
//    void render(MTL::RenderCommandEncoder* encoder, const simd::float4x4& viewProj, const simd::float3& uiPosition)
//    {
//        if (!isOpen) return;
//        craftSystem.render(encoder, currentGrid, viewProj, uiPosition);
//    }
//};

#endif /* RMDLFab3DUI_hpp */
