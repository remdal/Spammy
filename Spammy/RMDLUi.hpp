//
//  RMDLUi.hpp
//  Spammy
//
//  Created by Rémy on 28/12/2025.
//

#ifndef RMDLUi_hpp
#define RMDLUi_hpp

#include <Metal/Metal.hpp>

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

#include <simd/simd.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "RMDLMathUtils.hpp"
#include "RMDLUtils.hpp"
#include "RMDLFontLoader.h"
#include "RMDLUtilities.h"
#include "RMDLPetitPrince.hpp"

#include "RMDLMainRenderer_shared.h"

struct RMDLUi
{
    simd::float4x4 uiProjectionMatrix;
};

struct RectangleUIData
{
    VertexRectangle vertex0;
    VertexRectangle vertex1;
    VertexRectangle vertex2;
    VertexRectangle vertex3;
    VertexRectangle vertex4;
    VertexRectangle vertex5;
};

struct UIVertex
{
    simd::float3 position;
    simd::float2 texCoord;
    simd::float4 color;
};

struct UIRect
{
    float x, y, width, height;

    UIRect(float _x = 0, float _y = 0, float _w = 0, float _h = 0) : x(_x), y(_y), width(_w), height(_h) {}

    bool contains(float px, float py) const {
        return px >= x && px <= (x + width) && py >= y && py <= (y + height);
    }
};

enum class BatchType { Shape, Text };

struct RenderBatch
{
    BatchType type;
    size_t startIndex;
    size_t indexCount;
};

class MetalUIManager
{
public:
    MetalUIManager(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary);
    ~MetalUIManager();
    
    void beginFrame(NS::UInteger width, NS::UInteger height);
    void endFrame(MTL::RenderCommandEncoder* encoder);
    
    void drawText(const std::string& text, float x, float y, float scale = 1.0f, simd::float4 color = {1, 1, 1, 1});
    
    void drawRect(const UIRect& rect, simd::float4 color, bool filled = true);
    void drawRoundedRect(const UIRect& rect, float radius, simd::float4 color, bool filled = true);
    void drawCircle(float x, float y, float radius, simd::float4 color, bool filled = true);
    void drawLine(float x1, float y1, float x2, float y2, float thickness, simd::float4 color);

    void drawPanel(const UIRect& rect, simd::float4 bgColor, float borderWidth = 2.0f, simd::float4 borderColor = {1, 1, 1, 1});
    void drawProgressBar(const UIRect& rect, float progress, simd::float4 bgColor, simd::float4 fgColor);
    
private:
    void createShadersAndPipelineStates(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device);
    void createBuffers(MTL::Device* device);

    void startNewBatch(BatchType type);
    void addQuad(const UIVertex* vertices, BatchType type);

    NS::SharedPtr<MTL::RenderPipelineState> m_textPipeline;
    NS::SharedPtr<MTL::RenderPipelineState> m_shapePipeline;
    NS::SharedPtr<MTL::DepthStencilState> m_depthState;

    std::vector<UIVertex> m_vertices;
    std::vector<uint16_t> m_indices;
    NS::SharedPtr<MTL::Buffer> m_vertexBuffer;
    NS::SharedPtr<MTL::Buffer> m_indexBuffer;
    NS::SharedPtr<MTL::Buffer> m_frameDataBuffer;
    size_t m_maxVertices;
    size_t m_maxIndices;

    RMDLUi uiMatrix;
    Font fontAtlas;
    NS::SharedPtr<MTL::SamplerState> m_sampler;
    std::vector<RenderBatch> m_batches;

    MTL::Buffer*                        m_rectangleDataBuffer;
    MTL4::ArgumentTable*                m_argumentTable;
    MTL::ResidencySet*                  m_residencySet;
    MTL::Buffer*                        m_viewportSizeBuffer;
    simd_uint2                          m_viewportSize;
    MTL::RenderPipelineState*           m_psoUi;
};

namespace NASAAtTheHelm {

struct InvVertex {
    simd_float2 position;   // Position écran normalisée
    simd_float2 uv;
    simd_float4 color;
    float cornerRadius;     // Pour le shader
    float borderWidth;
};

struct UIUniforms
{
    simd_float2 screenSize;
    float time;
    float padding;
};

class InventoryUIRenderer
{
public:
    MTL::Device* device = nullptr;
    MTL::RenderPipelineState* uiPipeline = nullptr;
    MTL::RenderPipelineState* iconPipeline = nullptr;
    MTL::Buffer* vertexBuffer = nullptr;
    MTL::Buffer* uniformBuffer = nullptr;
    MTL::SamplerState* sampler = nullptr;
    
    // Textures d'icônes (atlas)
    MTL::Texture* iconAtlas = nullptr;
    
    // Configuration visuelle
    struct Theme {
        simd_float4 slotBackground      = {0.1f, 0.12f, 0.15f, 0.9f};
        simd_float4 slotBorder          = {0.3f, 0.35f, 0.4f, 1.0f};
        simd_float4 slotHover           = {0.2f, 0.25f, 0.3f, 0.95f};
        simd_float4 slotSelected        = {0.25f, 0.5f, 0.8f, 0.95f};
        simd_float4 slotEmpty           = {0.08f, 0.09f, 0.1f, 0.7f};
        simd_float4 textColor           = {0.9f, 0.92f, 0.95f, 1.0f};
        simd_float4 countBadgeColor     = {0.9f, 0.3f, 0.2f, 1.0f};
        simd_float4 tooltipBackground   = {0.05f, 0.05f, 0.08f, 0.95f};
        simd_float4 dragGhostColor      = {1.0f, 1.0f, 1.0f, 0.6f};
        float cornerRadius = 8.0f;
        float borderWidth = 2.0f;
        float slotPadding = 4.0f;
    } theme;
    
    // État du survol
    int32_t hoveredSlot = -1;
    simd_float2 mousePosition = {0, 0};
    
    // Données de rendu
    std::vector<InvVertex> vertices;
    uint32_t vertexCount = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // INITIALISATION
    // ═══════════════════════════════════════════════════════════════════════
    void initialize(MTL::Device* dev) {
        device = dev;
        
        createPipelines();
        createBuffers();
        createSampler();
        createDefaultIconAtlas();
    }
    
    void createPipelines() {
        const char* shaderSrc = R"(
            #include <metal_stdlib>
            using namespace metal;
            
            struct InvVertex {
                float2 position [[attribute(0)]];
                float2 uv       [[attribute(1)]];
                float4 color    [[attribute(2)]];
                float cornerRadius [[attribute(3)]];
                float borderWidth  [[attribute(4)]];
            };
            
            struct UIVarying {
                float4 position [[position]];
                float2 uv;
                float4 color;
                float2 localPos;
                float2 size;
                float cornerRadius;
                float borderWidth;
            };
            
            struct UIUniforms {
                float2 screenSize;
                float time;
                float padding;
            };
            
            // Signed distance function pour rectangle arrondi
            float sdRoundedBox(float2 p, float2 b, float r) {
                float2 q = abs(p) - b + r;
                return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
            }
            
            vertex UIVarying uiVertexShader(
                InvVertex in [[stage_in]],
                constant UIUniforms& uniforms [[buffer(1)]]
            ) {
                UIVarying out;
                
                // Convertir de [0,1] à [-1,1] pour Metal
                float2 clipPos = in.position * 2.0 - 1.0;
                clipPos.y = -clipPos.y;  // Inverser Y
                
                out.position = float4(clipPos, 0.0, 1.0);
                out.uv = in.uv;
                out.color = in.color;
                out.cornerRadius = in.cornerRadius;
                out.borderWidth = in.borderWidth;
                out.localPos = in.uv * 2.0 - 1.0;  // Pour SDF
                out.size = float2(1.0);  // Sera ajusté par le quad
                
                return out;
            }
            
            fragment float4 uiFragmentShader(
                UIVarying in [[stage_in]],
                constant UIUniforms& uniforms [[buffer(1)]]
            ) {
                // Rectangle arrondi avec SDF
                float2 size = float2(0.9, 0.9);  // Un peu de marge
                float d = sdRoundedBox(in.localPos, size, in.cornerRadius * 0.1);
                
                // Anti-aliasing
                float aa = fwidth(d) * 1.5;
                float alpha = 1.0 - smoothstep(-aa, aa, d);
                
                // Bordure
                float borderD = abs(d) - in.borderWidth * 0.02;
                float borderAlpha = 1.0 - smoothstep(-aa, aa, borderD);
                
                // Couleur finale
                float4 fillColor = in.color;
                float4 borderColor = float4(in.color.rgb * 1.5, 1.0);
                
                float4 finalColor = mix(fillColor, borderColor, borderAlpha * 0.5);
                finalColor.a *= alpha;
                
                // Effet de brillance subtil
                float shine = pow(max(0.0, 1.0 - in.localPos.y), 3.0) * 0.15;
                finalColor.rgb += shine;
                
                return finalColor;
            }
            
            // Shader pour les icônes
            fragment float4 iconFragmentShader(
                UIVarying in [[stage_in]],
                texture2d<float> iconTexture [[texture(0)]],
                sampler iconSampler [[sampler(0)]]
            ) {
                float4 texColor = iconTexture.sample(iconSampler, in.uv);
                return texColor * in.color;
            }
        )";
        
        NS::Error* error = nullptr;
        MTL::Library* library = device->newLibrary(
            NS::String::string(shaderSrc, NS::UTF8StringEncoding),
            nullptr, &error
        );
        
        if (!library) return;
        
        // Pipeline UI standard
        MTL::Function* vertFunc = library->newFunction(
            NS::String::string("uiVertexShader", NS::UTF8StringEncoding));
        MTL::Function* fragFunc = library->newFunction(
            NS::String::string("uiFragmentShader", NS::UTF8StringEncoding));
        
        MTL::RenderPipelineDescriptor* desc = MTL::RenderPipelineDescriptor::alloc()->init();
        desc->setVertexFunction(vertFunc);
        desc->setFragmentFunction(fragFunc);
        desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        desc->colorAttachments()->object(0)->setBlendingEnabled(true);
        desc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        desc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        
        // Vertex descriptor
        MTL::VertexDescriptor* vertDesc = MTL::VertexDescriptor::alloc()->init();
        vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
        vertDesc->attributes()->object(0)->setOffset(0);
        vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
        vertDesc->attributes()->object(1)->setOffset(8);
        vertDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat4);
        vertDesc->attributes()->object(2)->setOffset(16);
        vertDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat);
        vertDesc->attributes()->object(3)->setOffset(32);
        vertDesc->attributes()->object(4)->setFormat(MTL::VertexFormatFloat);
        vertDesc->attributes()->object(4)->setOffset(36);
        vertDesc->layouts()->object(0)->setStride(sizeof(InvVertex));
        
        desc->setVertexDescriptor(vertDesc);
        
        uiPipeline = device->newRenderPipelineState(desc, &error);
        
        // Pipeline pour icônes
        MTL::Function* iconFrag = library->newFunction(
            NS::String::string("iconFragmentShader", NS::UTF8StringEncoding));
        desc->setFragmentFunction(iconFrag);
        iconPipeline = device->newRenderPipelineState(desc, &error);
        
        vertFunc->release();
        fragFunc->release();
        iconFrag->release();
        desc->release();
        vertDesc->release();
        library->release();
    }
    
    void createBuffers() {
        // Buffer de vertices (assez grand pour tout l'UI)
        vertexBuffer = device->newBuffer(sizeof(InvVertex) * 10000,
                                         MTL::ResourceStorageModeShared);
        uniformBuffer = device->newBuffer(sizeof(UIUniforms),
                                          MTL::ResourceStorageModeShared);
    }
    
    void createSampler() {
        MTL::SamplerDescriptor* desc = MTL::SamplerDescriptor::alloc()->init();
        desc->setMinFilter(MTL::SamplerMinMagFilterLinear);
        desc->setMagFilter(MTL::SamplerMinMagFilterLinear);
        sampler = device->newSamplerState(desc);
        desc->release();
    }
    
    void createDefaultIconAtlas() {
        // Créer une texture d'icônes par défaut (placeholder)
        MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA8Unorm, 256, 256, false);
        iconAtlas = device->newTexture(desc);
        desc->release();
        
        // TODO: Charger vraies icônes
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // RENDU
    // ═══════════════════════════════════════════════════════════════════════
    void render(MTL::RenderCommandEncoder* encoder,
                Inventory& inventory,
                DragDropSystem& dragDrop,
                simd_float2 screenSize,
                float time) {
        
        if (!inventory.isVisible && dragDrop.state == DragDropSystem::State::Idle) {
            return;
        }
        
        // Reset vertices
        vertices.clear();
        
        // Mise à jour uniforms
        UIUniforms* uniforms = (UIUniforms*)uniformBuffer->contents();
        uniforms->screenSize = screenSize;
        uniforms->time = time;
        
        // Dessiner l'inventaire
        if (inventory.isVisible) {
            drawInventory(inventory, screenSize);
        }
        
        // Dessiner le hotbar (toujours visible)
        drawHotbar(inventory, screenSize);
        
        // Dessiner l'item draggé
        if (dragDrop.state == DragDropSystem::State::DraggingFromInventory ||
            dragDrop.state == DragDropSystem::State::PlacingBlock) {
            drawDraggedItem(dragDrop, screenSize);
        }
        
        // Dessiner le tooltip
        if (hoveredSlot >= 0 && inventory.isVisible) {
            drawTooltip(inventory.slots[hoveredSlot], screenSize);
        }
        
        // Uploader vertices
        if (vertices.empty()) return;
        
        memcpy(vertexBuffer->contents(), vertices.data(),
               vertices.size() * sizeof(InvVertex));
        vertexCount = (uint32_t)vertices.size();
        
        // Encoder
        encoder->setRenderPipelineState(uiPipeline);
        encoder->setVertexBuffer(vertexBuffer, 0, 0);
        encoder->setVertexBuffer(uniformBuffer, 0, 1);
        encoder->setFragmentBuffer(uniformBuffer, 0, 1);
        
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(vertexCount));
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // DESSIN DE L'INVENTAIRE
    // ═══════════════════════════════════════════════════════════════════════
    void drawInventory(Inventory& inventory, simd_float2 screenSize) {
        // Fond de l'inventaire
        float invWidth = Inventory::SLOTS_PER_ROW * (inventory.slotSize.x + inventory.padding);
        float invHeight = Inventory::MAX_ROWS * (inventory.slotSize.y + inventory.padding);
        
        simd_float2 invPos = {
            inventory.position.x - invWidth * 0.5f,
            inventory.position.y
        };
        
        // Fond semi-transparent
        addRect(invPos - simd_float2{0.02f, 0.02f},
                {invWidth + 0.04f, invHeight + 0.04f},
                {0.02f, 0.03f, 0.05f, 0.85f},
                12.0f, 2.0f);
        
        // Dessiner chaque slot
        for (uint32_t row = 0; row < Inventory::MAX_ROWS; row++) {
            for (uint32_t col = 0; col < Inventory::SLOTS_PER_ROW; col++) {
                uint32_t slotIdx = row * Inventory::SLOTS_PER_ROW + col;
                
                simd_float2 slotPos = {
                    invPos.x + col * (inventory.slotSize.x + inventory.padding),
                    invPos.y - row * (inventory.slotSize.y + inventory.padding)
                };
                
                drawSlot(inventory.slots[slotIdx], slotPos, inventory.slotSize,
                        slotIdx == inventory.selectedSlot,
                        slotIdx == hoveredSlot);
            }
        }
    }
    
    void drawHotbar(Inventory& inventory, simd_float2 screenSize) {
        // Hotbar en bas (10 premiers slots)
        float hotbarWidth = 10 * (0.05f + 0.008f);
        simd_float2 hotbarPos = {0.5f - hotbarWidth * 0.5f, 0.05f};
        
        for (int i = 0; i < 10; i++) {
            simd_float2 slotPos = {
                hotbarPos.x + i * (0.05f + 0.008f),
                hotbarPos.y
            };
            
            bool selected = (i == inventory.selectedSlot);
            bool hovered = (!inventory.isVisible && i == hoveredSlot);
            
            drawSlot(inventory.slots[i], slotPos, {0.05f, 0.06f},
                    selected, hovered);
            
            // Numéro du slot
            // TODO: Dessiner texte "1-9, 0"
        }
    }
    
    void drawSlot(const InventoryItem& item, simd_float2 pos, simd_float2 size,
                  bool selected, bool hovered) {
        simd_float4 bgColor;
        
        if (selected) {
            bgColor = theme.slotSelected;
        } else if (hovered) {
            bgColor = theme.slotHover;
        } else if (item.count == 0) {
            bgColor = theme.slotEmpty;
        } else {
            bgColor = theme.slotBackground;
        }
        
        // Fond du slot
        addRect(pos, size, bgColor, theme.cornerRadius, theme.borderWidth);
        
        // Icône de l'item
        if (item.count > 0) {
            simd_float2 iconPos = pos + simd_float2{theme.slotPadding / 100.0f,
                                                     -theme.slotPadding / 100.0f};
            simd_float2 iconSize = size - simd_float2{theme.slotPadding * 2 / 100.0f,
                                                       theme.slotPadding * 2 / 100.0f};
            
            // Placeholder pour l'icône (carré coloré selon le type)
            simd_float4 iconColor = getBlockTypeColor(item.type);
            addRect(iconPos, iconSize * 0.8f, iconColor, 4.0f, 0.0f);
            
            // Badge de quantité
            if (item.count > 1) {
                simd_float2 badgePos = pos + simd_float2{size.x - 0.02f, -size.y + 0.02f};
                addRect(badgePos, {0.018f, 0.022f}, theme.countBadgeColor, 4.0f, 0.0f);
                // TODO: Dessiner le nombre
            }
        }
    }
    
    void drawDraggedItem(DragDropSystem& dragDrop, simd_float2 screenSize) {
        if (!dragDrop.draggedItem) return;
        
        // Item qui suit la souris
        simd_float2 itemPos = mousePosition - simd_float2{0.03f, -0.035f};
        
        simd_float4 color = getBlockTypeColor(dragDrop.draggedItem->type);
        color.a = 0.7f;
        
        addRect(itemPos, {0.06f, 0.07f}, color, 8.0f, 2.0f);
    }
    
    void drawTooltip(const InventoryItem& item, simd_float2 screenSize) {
        if (item.count == 0) return;
        
        simd_float2 tooltipPos = mousePosition + simd_float2{0.02f, 0.02f};
        simd_float2 tooltipSize = {0.2f, 0.08f};
        
        // Fond
        addRect(tooltipPos, tooltipSize, theme.tooltipBackground, 6.0f, 1.0f);
        
        // TODO: Dessiner texte du nom et description
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // HELPERS
    // ═══════════════════════════════════════════════════════════════════════
    void addRect(simd_float2 pos, simd_float2 size, simd_float4 color,
                 float cornerRadius, float borderWidth) {
        // 6 vertices pour 2 triangles
        InvVertex v[6];
        
        // Triangle 1
        v[0] = {pos, {0, 0}, color, cornerRadius, borderWidth};
        v[1] = {{pos.x + size.x, pos.y}, {1, 0}, color, cornerRadius, borderWidth};
        v[2] = {{pos.x + size.x, pos.y - size.y}, {1, 1}, color, cornerRadius, borderWidth};
        
        // Triangle 2
        v[3] = {pos, {0, 0}, color, cornerRadius, borderWidth};
        v[4] = {{pos.x + size.x, pos.y - size.y}, {1, 1}, color, cornerRadius, borderWidth};
        v[5] = {{pos.x, pos.y - size.y}, {0, 1}, color, cornerRadius, borderWidth};
        
        for (int i = 0; i < 6; i++) {
            vertices.push_back(v[i]);
        }
    }
    
    simd_float4 getBlockTypeColor(BlockType type) {
        switch (type) {
            case BlockType::Commander: return {0.2f, 0.4f, 0.8f, 1.0f};
            case BlockType::Wheel:     return {0.3f, 0.3f, 0.35f, 1.0f};
            case BlockType::Thruster:  return {0.8f, 0.4f, 0.1f, 1.0f};
            case BlockType::Weapon:    return {0.7f, 0.2f, 0.2f, 1.0f};
            case BlockType::Armor:     return {0.4f, 0.45f, 0.5f, 1.0f};
            case BlockType::Generator: return {0.9f, 0.8f, 0.2f, 1.0f};
            case BlockType::Storage:   return {0.5f, 0.35f, 0.2f, 1.0f};
            case BlockType::Sensor:    return {0.2f, 0.7f, 0.6f, 1.0f};
            case BlockType::Connector: return {0.5f, 0.5f, 0.5f, 1.0f};
            default:                   return {0.5f, 0.5f, 0.5f, 1.0f};
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // INPUT HANDLING
    // ═══════════════════════════════════════════════════════════════════════
    void updateMousePosition(simd_float2 normalizedPos) {
        mousePosition = normalizedPos;
    }
    
    int32_t getSlotAtPosition(Inventory& inventory, simd_float2 pos) {
        return inventory.hitTest(pos);
    }
    
    void handleMouseDown(Inventory& inventory, DragDropSystem& dragDrop,
                        simd_float2 mousePos) {
        mousePosition = mousePos;
        
        int32_t slot = getSlotAtPosition(inventory, mousePos);
        if (slot >= 0) {
            inventory.selectedSlot = slot;
            dragDrop.beginDragFromInventory(inventory, slot);
        }
    }
    
    void handleMouseUp(Vehicle& vehicle, Inventory& inventory,
                      DragDropSystem& dragDrop, simd_float2 mousePos) {
        dragDrop.endDrag(vehicle, inventory);
    }
    
    void handleMouseMove(Inventory& inventory, simd_float2 mousePos) {
        mousePosition = mousePos;
        hoveredSlot = getSlotAtPosition(inventory, mousePos);
    }
    
    void handleKeyPress(Inventory& inventory, int key) {
        // Touches 1-9, 0 pour hotbar
        if (key >= '1' && key <= '9') {
            inventory.selectedSlot = key - '1';
        } else if (key == '0') {
            inventory.selectedSlot = 9;
        }
        // E ou Tab pour toggle inventaire
        else if (key == 'E' || key == '\t') {
            inventory.isVisible = !inventory.isVisible;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // CLEANUP
    // ═══════════════════════════════════════════════════════════════════════
    void cleanup() {
        if (uiPipeline) uiPipeline->release();
        if (iconPipeline) iconPipeline->release();
        if (vertexBuffer) vertexBuffer->release();
        if (uniformBuffer) uniformBuffer->release();
        if (sampler) sampler->release();
        if (iconAtlas) iconAtlas->release();
    }
};

} // namespace Metal4

#endif /* RMDLUi_hpp */
