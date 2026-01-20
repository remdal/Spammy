//
//  RMDLInventory.cpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include "RMDLInventory.hpp"

namespace inventoryWindow {

InventorySlotData::InventorySlotData()
: itemTypeID(0), itemCount(0), itemColor{0.5f, 0.5f, 0.5f, 1.f}, isEmpty(true)
{
    
}

InventoryPanel::InventoryPanel(MTL::Device* device, MTL::PixelFormat colorPixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary)
: m_device(device)
, m_renderPipeline(nullptr), m_depthStencilState(nullptr)
, m_vertexBuffer(nullptr), m_indexBuffer(nullptr), m_uniformBuffer(nullptr)
, m_panelPosition{0.5f, 0.5f}
, m_panelSizePixels{540.f, 280.f}
, m_slotSizePixels{48.f, 48.f}
, m_slotSpacingPixels{6.f}
, m_titleBarHeightPixels{28.f}
, m_isVisible(true)
, m_isDragging(false)
, m_dragStartOffset{0.f, 0.f}
, m_hoveredSlotIndex(-1)
, m_selectedSlotIndex(0)
, m_time(0.f)
, m_vertexCount(0), m_indexCount(0)
{
    m_uniforms.panelBackgroundColor = {0.08f, 0.09f, 0.12f, 0.74f};
    m_uniforms.slotNormalColor = {0.14f, 0.16f, 0.20f, 1.f};
    m_uniforms.slotHoveredColor = {0.22f, 0.26f, 0.32f, 1.f};
    m_uniforms.slotSelectedColor = {0.25f, 0.45f, 0.75f, 1.f};
    m_uniforms.titleBarColor = {0.12f, 0.14f, 0.18f, 1.f};
    m_uniforms.panelCornerRadius = 12.f;
    m_uniforms.slotCornerRadius = 6.f;
    m_uniforms.borderThickness = 1.5f;
    
    for (uint32_t i = 0; i < PANEL_SLOT_COUNT; i++)
    {
        m_slots[i] = InventorySlotData();
    }
    
    buildRenderPipeline(colorPixelFormat, depthPixelFormat, shaderLibrary);
    buildGeometry();
}

InventoryPanel::~InventoryPanel()
{
    if (m_renderPipeline) m_renderPipeline->release();
    if (m_depthStencilState) m_depthStencilState->release();
    if (m_vertexBuffer) m_vertexBuffer->release();
    if (m_indexBuffer) m_indexBuffer->release();
    if (m_uniformBuffer) m_uniformBuffer->release();
}

void InventoryPanel::buildRenderPipeline(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* library)
{
    NS::Error* error = nullptr;
    
    MTL::Function* vertexFunction = library->newFunction(MTLSTR("inventoryPanelVertexShader"));
    MTL::Function* fragmentFunction = library->newFunction(MTLSTR("inventoryPanelFragmentShader"));
    
    if (!vertexFunction || !fragmentFunction) {
        printf("InventoryPanel: Failed to load shader functions\n");
        return;
    }
    
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    // position
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    // texCoord
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd::float2));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    // color
    vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd::float2) * 2);
    vertexDescriptor->attributes()->object(2)->setBufferIndex(0);
    // cornerRadius
    vertexDescriptor->attributes()->object(3)->setFormat(MTL::VertexFormatFloat);
    vertexDescriptor->attributes()->object(3)->setOffset(sizeof(simd::float2) * 2 + sizeof(simd::float4));
    vertexDescriptor->attributes()->object(3)->setBufferIndex(0);
    // borderWidth
    vertexDescriptor->attributes()->object(4)->setFormat(MTL::VertexFormatFloat);
    vertexDescriptor->attributes()->object(4)->setOffset(sizeof(simd::float2) * 2 + sizeof(simd::float4) + sizeof(float));
    vertexDescriptor->attributes()->object(4)->setBufferIndex(0);
    // slotIndex
    vertexDescriptor->attributes()->object(5)->setFormat(MTL::VertexFormatUInt);
    vertexDescriptor->attributes()->object(5)->setOffset(sizeof(simd::float2) * 2 + sizeof(simd::float4) + sizeof(float) * 2);
    vertexDescriptor->attributes()->object(5)->setBufferIndex(0);
    // flags
    vertexDescriptor->attributes()->object(6)->setFormat(MTL::VertexFormatUInt);
    vertexDescriptor->attributes()->object(6)->setOffset(sizeof(simd::float2) * 2 + sizeof(simd::float4) + sizeof(float) * 2 + sizeof(uint32_t));
    vertexDescriptor->attributes()->object(6)->setBufferIndex(0);
    
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(InventoryPanelVertex));
    vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vertexFunction);
    pipelineDescriptor->setFragmentFunction(fragmentFunction);
    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(colorFormat);
    pipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    pipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    pipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    pipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    pipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    pipelineDescriptor->setDepthAttachmentPixelFormat(depthFormat);
    
    m_renderPipeline = m_device->newRenderPipelineState(pipelineDescriptor, &error);
    if (!m_renderPipeline)
        printf("InventoryPanel: Pipeline creation failed\n");
    
    MTL::DepthStencilDescriptor* depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(false);
    m_depthStencilState = m_device->newDepthStencilState(depthDescriptor);
    
    vertexFunction->release();
    fragmentFunction->release();
    pipelineDescriptor->release();
    vertexDescriptor->release();
    depthDescriptor->release();
}

void InventoryPanel::buildGeometry()
{
    // 1 quad pour le fond + 1 quad pour la title bar + PANEL_SLOT_COUNT
    uint32_t totalQuads = 2 + PANEL_SLOT_COUNT + 1;
    uint32_t maxVertices = totalQuads * 4;
    uint32_t maxIndices = totalQuads * 6;
    
    m_vertexBuffer = m_device->newBuffer(maxVertices * sizeof(InventoryPanelVertex), MTL::ResourceStorageModeShared);
    m_indexBuffer = m_device->newBuffer(maxIndices * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    m_uniformBuffer = m_device->newBuffer(sizeof(InventoryPanelUniforms), MTL::ResourceStorageModeShared);
}

void InventoryPanel::rebuildVertices(simd::float2 screenSize)
{
    std::vector<InventoryPanelVertex> vertices;
    std::vector<uint32_t> indices;
    
    float panelW = m_panelSizePixels.x / screenSize.x;
    float panelH = m_panelSizePixels.y / screenSize.y;
    float titleH = m_titleBarHeightPixels / screenSize.y;
    float slotW = m_slotSizePixels.x / screenSize.x;
    float slotH = m_slotSizePixels.y / screenSize.y;
    float spacing = m_slotSpacingPixels / screenSize.x;
    float spacingY = m_slotSpacingPixels / screenSize.y;
    
    float left = m_panelPosition.x - panelW * 0.5f;
    float top = m_panelPosition.y - panelH * 0.5f;
    
    auto addQuad = [&](simd::float2 pos, simd::float2 size, simd::float4 color, float radius, float border, uint32_t slot, uint32_t flags)
    {
        uint32_t base = (uint32_t)vertices.size();
        vertices.push_back({{pos.x, pos.y}, {0, 0}, color, radius, border, slot, flags});
        vertices.push_back({{pos.x + size.x, pos.y}, {1, 0}, color, radius, border, slot, flags});
        vertices.push_back({{pos.x + size.x, pos.y + size.y}, {1, 1}, color, radius, border, slot, flags});
        vertices.push_back({{pos.x, pos.y + size.y}, {0, 1}, color, radius, border, slot, flags});
        indices.insert(indices.end(), {base, base+1, base+2, base, base+2, base+3});
    };
    
    // Panel background
    addQuad(simd::float2{left, top}, simd::float2{panelW, panelH}, m_uniforms.panelBackgroundColor,
            m_uniforms.panelCornerRadius, 0.f, 0, 1);
    
    // Title bar
    addQuad(simd::float2{left, top}, simd::float2{panelW, titleH}, m_uniforms.titleBarColor,
            m_uniforms.panelCornerRadius, 0.f, 0, 1);
    
    // Slots
    float contentTop = top + titleH + spacingY;
    float contentLeft = left + spacing;
    
    for (uint32_t row = 0; row < PANEL_ROWS; row++)
    {
        for (uint32_t col = 0; col < PANEL_COLUMNS; col++)
        {
            uint32_t slotIdx = row * PANEL_COLUMNS + col;
            float sx = contentLeft + col * (slotW + spacing);
            float sy = contentTop + row * (slotH + spacingY);
            
            simd::float4 slotColor = m_uniforms.slotNormalColor;
            if ((int32_t)slotIdx == m_selectedSlotIndex)
                slotColor = m_uniforms.slotSelectedColor;
            else if ((int32_t)slotIdx == m_hoveredSlotIndex)
                slotColor = m_uniforms.slotHoveredColor;
            
            addQuad(simd::float2{sx, sy}, simd::float2{slotW, slotH}, slotColor,
                    m_uniforms.slotCornerRadius, m_uniforms.borderThickness, slotIdx, 2);
            
            // Item icon (if not empty)
            if (!m_slots[slotIdx].isEmpty)
            {
                float iconPad = 4.f / screenSize.x;
                float iconPadY = 4.f / screenSize.y;
                addQuad(simd::float2{sx + iconPad, sy + iconPadY},
                        simd::float2{slotW - iconPad * 2.f, slotH - iconPadY * 2.f},
                        m_slots[slotIdx].itemColor, 4.f, 0.f, slotIdx, 4);
            }
        }
    }
    
    m_vertexCount = (uint32_t)vertices.size();
    m_indexCount = (uint32_t)indices.size();
    
    memcpy(m_vertexBuffer->contents(), vertices.data(), vertices.size() * sizeof(InventoryPanelVertex));
    memcpy(m_indexBuffer->contents(), indices.data(), indices.size() * sizeof(uint32_t));
}

void InventoryPanel::update(float deltaTime)
{
    m_time += deltaTime;
}

void InventoryPanel::render(MTL::RenderCommandEncoder* encoder, simd::float2 screenSize)
{
    if (!m_isVisible || !m_renderPipeline) return;
    
    rebuildVertices(screenSize);
    
    m_uniforms.screenSize = screenSize;
    m_uniforms.panelOrigin = m_panelPosition;
    m_uniforms.panelSize = m_panelSizePixels;
    m_uniforms.slotDimensions = m_slotSizePixels;
    m_uniforms.slotSpacing = m_slotSpacingPixels;
    m_uniforms.time = m_time;
    m_uniforms.hoveredSlotIndex = m_hoveredSlotIndex;
    m_uniforms.selectedSlotIndex = m_selectedSlotIndex;
    m_uniforms.titleBarHeight = m_titleBarHeightPixels;
    
    memcpy(m_uniformBuffer->contents(), &m_uniforms, sizeof(InventoryPanelUniforms));
    
    encoder->setRenderPipelineState(m_renderPipeline);
    encoder->setDepthStencilState(m_depthStencilState);
    encoder->setVertexBuffer(m_vertexBuffer, 0, 0);
    encoder->setVertexBuffer(m_uniformBuffer, 0, 1);
    encoder->setFragmentBuffer(m_uniformBuffer, 0, 0);
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, m_indexCount, MTL::IndexTypeUInt32, m_indexBuffer, 0);
}

bool InventoryPanel::hitTestTitleBar(simd::float2 normalizedPos, simd::float2 screenSize) const
{
    float panelW = m_panelSizePixels.x / screenSize.x;
    float panelH = m_panelSizePixels.y / screenSize.y;
    float titleH = m_titleBarHeightPixels / screenSize.y;
    
    float left = m_panelPosition.x - panelW * 0.5f;
    float top = m_panelPosition.y - panelH * 0.5f;
    
    return normalizedPos.x >= left && normalizedPos.x <= left + panelW &&
           normalizedPos.y >= top && normalizedPos.y <= top + titleH;
}

int32_t InventoryPanel::hitTestSlot(simd::float2 normalizedPos, simd::float2 screenSize) const
{
    float panelW = m_panelSizePixels.x / screenSize.x;
    float panelH = m_panelSizePixels.y / screenSize.y;
    float titleH = m_titleBarHeightPixels / screenSize.y;
    float slotW = m_slotSizePixels.x / screenSize.x;
    float slotH = m_slotSizePixels.y / screenSize.y;
    float spacing = m_slotSpacingPixels / screenSize.x;
    float spacingY = m_slotSpacingPixels / screenSize.y;
    
    float left = m_panelPosition.x - panelW * 0.5f;
    float top = m_panelPosition.y - panelH * 0.5f;
    float contentTop = top + titleH + spacingY;
    float contentLeft = left + spacing;
    
    for (uint32_t row = 0; row < PANEL_ROWS; row++) {
        for (uint32_t col = 0; col < PANEL_COLUMNS; col++) {
            float sx = contentLeft + col * (slotW + spacing);
            float sy = contentTop + row * (slotH + spacingY);
            
            if (normalizedPos.x >= sx && normalizedPos.x <= sx + slotW &&
                normalizedPos.y >= sy && normalizedPos.y <= sy + slotH) {
                return (int32_t)(row * PANEL_COLUMNS + col);
            }
        }
    }
    return -1;
}

simd::float2 InventoryPanel::getSlotPositionNormalized(uint32_t slotIndex, simd::float2 screenSize) const
{
    if (slotIndex >= PANEL_SLOT_COUNT) return {0, 0};
    
    float panelW = m_panelSizePixels.x / screenSize.x;
    float panelH = m_panelSizePixels.y / screenSize.y;
    float titleH = m_titleBarHeightPixels / screenSize.y;
    float slotW = m_slotSizePixels.x / screenSize.x;
    float slotH = m_slotSizePixels.y / screenSize.y;
    float spacing = m_slotSpacingPixels / screenSize.x;
    float spacingY = m_slotSpacingPixels / screenSize.y;
    
    float left = m_panelPosition.x - panelW * 0.5f;
    float top = m_panelPosition.y - panelH * 0.5f;
    
    uint32_t col = slotIndex % PANEL_COLUMNS;
    uint32_t row = slotIndex / PANEL_COLUMNS;
    
    float sx = left + spacing + col * (slotW + spacing);
    float sy = top + titleH + spacingY + row * (slotH + spacingY);
    
    return {sx + slotW * 0.5f, sy + slotH * 0.5f};
}

void InventoryPanel::onMouseDown(simd::float2 screenPosition, simd::float2 screenSize)
{
    simd::float2 normalizedPos = {screenPosition.x / screenSize.x, screenPosition.y / screenSize.y};
    
    if (hitTestTitleBar(normalizedPos, screenSize))
    {
        m_isDragging = true;
        m_dragStartOffset = {normalizedPos.x - m_panelPosition.x, normalizedPos.y - m_panelPosition.y};
        return;
    }
    
    int32_t slot = hitTestSlot(normalizedPos, screenSize);
    if (slot >= 0)
        m_selectedSlotIndex = slot;
}

void InventoryPanel::onMouseUp(simd::float2 screenPosition, simd::float2 screenSize)
{
    m_isDragging = false;
}

void InventoryPanel::onMouseMoved(simd::float2 screenPosition, simd::float2 screenSize)
{
    simd::float2 normalizedPos = {screenPosition.x / screenSize.x, screenPosition.y / screenSize.y};
    
    if (m_isDragging)
    {
        m_panelPosition = {normalizedPos.x - m_dragStartOffset.x, normalizedPos.y - m_dragStartOffset.y};
        
        // Clamp to screen
        float panelW = m_panelSizePixels.x / screenSize.x;
        float panelH = m_panelSizePixels.y / screenSize.y;
        m_panelPosition.x = fmaxf(panelW * 0.5f, fminf(1.f - panelW * 0.5f, m_panelPosition.x));
        m_panelPosition.y = fmaxf(panelH * 0.5f, fminf(1.f - panelH * 0.5f, m_panelPosition.y));
        return;
    }
    
    m_hoveredSlotIndex = hitTestSlot(normalizedPos, screenSize);
}

void InventoryPanel::setSlotData(uint32_t slotIndex, uint32_t typeID, uint32_t count, simd::float4 color)
{
    if (slotIndex >= PANEL_SLOT_COUNT) return;
    m_slots[slotIndex].itemTypeID = typeID;
    m_slots[slotIndex].itemCount = count;
    m_slots[slotIndex].itemColor = color;
    m_slots[slotIndex].isEmpty = (count == 0);
}

void InventoryPanel::clearSlot(uint32_t slotIndex)
{
    if (slotIndex >= PANEL_SLOT_COUNT) return;
    m_slots[slotIndex] = InventorySlotData();
}

int32_t InventoryPanel::getHoveredSlot() const { return m_hoveredSlotIndex; }
int32_t InventoryPanel::getSelectedSlot() const { return m_selectedSlotIndex; }
void InventoryPanel::setSelectedSlot(int32_t slotIndex) { m_selectedSlotIndex = slotIndex; }
void InventoryPanel::setVisible(bool visible) { m_isVisible = visible; }
bool InventoryPanel::isVisible() const { return m_isVisible; }
void InventoryPanel::setPanelPosition(simd::float2 normalizedPosition) { m_panelPosition = normalizedPosition; }
simd::float2 InventoryPanel::getPanelPosition() const { return m_panelPosition; }

}
//void InventoryItem::updateProperties() {
//    auto props = ItemProperties::getProperties(type);
//    name = props.name;
//    color = props.color;
//    maxStack = props.maxStack;
//}
//
//ItemProperties ItemProperties::getProperties(ItemType type) {
//    switch (type) {
//        case ItemType::CommandBlock:
//            return {type, "Command Block", {1.0f, 0.5f, 0.0f, 1.0f}, 1, true, 100.0f};
//        case ItemType::SolidBlock:
//            return {type, "Solid Block", {0.5f, 0.5f, 0.5f, 1.0f}, 64, true, 50.0f};
//        case ItemType::GlassBlock:
//            return {type, "Glass Block", {0.8f, 0.9f, 1.0f, 0.6f}, 64, true, 20.0f};
//        case ItemType::MetalBlock:
//            return {type, "Metal Block", {0.7f, 0.7f, 0.8f, 1.0f}, 64, true, 150.0f};
//        case ItemType::CrystalBlock:
//            return {type, "Crystal Block", {0.5f, 0.8f, 1.0f, 0.8f}, 64, true, 30.0f};
//        case ItemType::EngineBlock:
//            return {type, "Engine Block", {0.9f, 0.3f, 0.2f, 1.0f}, 16, true, 200.0f};
//        case ItemType::ThrusterBlock:
//            return {type, "Thruster Block", {0.3f, 0.5f, 0.9f, 1.0f}, 16, true, 120.0f};
//        case ItemType::WeaponBlock:
//            return {type, "Weapon Block", {0.8f, 0.2f, 0.2f, 1.0f}, 8, true, 180.0f};
//        case ItemType::ShieldBlock:
//            return {type, "Shield Block", {0.4f, 0.7f, 1.0f, 0.7f}, 16, true, 90.0f};
//        default:
//            return {ItemType::None, "Empty", {1,1,1,1}, 1, false, 0.0f};
//    }
//}
//
//const char* ItemProperties::getItemName(ItemType type) {
//    return getProperties(type).name.c_str();
//}
//
//simd::float4 ItemProperties::getItemColor(ItemType type) {
//    return getProperties(type).color;
//}
//
//// ============================================================================
//// Vehicle Grid
//// ============================================================================
//
//void VehicleGrid::setBlock(int x, int y, int z, ItemType type) {
//    GridCell* cell = getCell(x, y, z);
//    if (!cell) return;
//    
//    cell->blockType = type;
//    cell->active = true;
//    cell->color = ItemProperties::getItemColor(type);
//    
//    // Check if this is the command block
//    if (type == ItemType::CommandBlock) {
//        centerBlock = {x, y, z};
//        hasCenterBlock = true;
//    }
//}
//
//void VehicleGrid::removeBlock(int x, int y, int z) {
//    GridCell* cell = getCell(x, y, z);
//    if (!cell) return;
//    
//    if (x == centerBlock.x && y == centerBlock.y && z == centerBlock.z) {
//        hasCenterBlock = false;
//    }
//    
//    cell->blockType = ItemType::None;
//    cell->active = false;
//}
//
//bool VehicleGrid::isConnected(int x, int y, int z) {
//    if (!hasCenterBlock) return false;
//    
//    // BFS to check connectivity to command block
//    std::vector<bool> visited(GRID_SIZE * GRID_SIZE * GRID_SIZE, false);
//    std::vector<simd::int3> queue;
//    
//    queue.push_back(centerBlock);
//    int idx = centerBlock.x + centerBlock.y * GRID_SIZE + centerBlock.z * GRID_SIZE * GRID_SIZE;
//    visited[idx] = true;
//    
//    simd::int3 target = {x, y, z};
//    
//    while (!queue.empty()) {
//        simd::int3 current = queue.back();
//        queue.pop_back();
//        
//        if (current.x == target.x && current.y == target.y && current.z == target.z) {
//            return true;
//        }
//        
//        // Check 6 neighbors
//        simd::int3 offsets[6] = {
//            {-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1}
//        };
//        
//        for (const auto& offset : offsets) {
//            simd::int3 neighbor = current + offset;
//            
//            GridCell* cell = getCell(neighbor.x, neighbor.y, neighbor.z);
//            if (!cell || !cell->active) continue;
//            
//            int nIdx = neighbor.x + neighbor.y * GRID_SIZE + neighbor.z * GRID_SIZE * GRID_SIZE;
//            if (visited[nIdx]) continue;
//            
//            visited[nIdx] = true;
//            queue.push_back(neighbor);
//        }
//    }
//    
//    return false;
//}
//
//void VehicleGrid::calculateMassAndProperties(float& mass, simd::float3& centerOfMass) {
//    mass = 0.0f;
//    centerOfMass = {0, 0, 0};
//    int activeCount = 0;
//    
//    for (int z = 0; z < GRID_SIZE; z++) {
//        for (int y = 0; y < GRID_SIZE; y++) {
//            for (int x = 0; x < GRID_SIZE; x++) {
//                GridCell* cell = getCell(x, y, z);
//                if (!cell || !cell->active) continue;
//                
//                float blockMass = ItemProperties::getProperties(cell->blockType).mass;
//                mass += blockMass;
//                
//                centerOfMass += simd::float3{(float)x, (float)y, (float)z} * blockMass;
//                activeCount++;
//            }
//        }
//    }
//    
//    if (mass > 0.0f) {
//        centerOfMass /= mass;
//    }
//}
//
//// ============================================================================
//// Inventory System
//// ============================================================================
//
//InventorySystem::InventorySystem(MTL::Device* device, MTL::Library* library,
//                                 float screenWidth, float screenHeight)
//    : m_device(device->retain())
//    , m_library(library->retain())
//    , m_selectedHotbarSlot(0)
//    , m_vehicleBuildMode(false)
//    , m_screenWidth(screenWidth)
//    , m_screenHeight(screenHeight)
//{
//    initializeSlots();
//    createRenderPipelines();
//    loadTextures();
//    
//    // Give player starting items
//    addItem(ItemType::CommandBlock, 1);
//    addItem(ItemType::SolidBlock, 64);
//    addItem(ItemType::MetalBlock, 32);
//}
//
//InventorySystem::~InventorySystem() {
//    if (m_uiPipeline) m_uiPipeline->release();
//    if (m_vehicleGridPipeline) m_vehicleGridPipeline->release();
//    if (m_uiVertexBuffer) m_uiVertexBuffer->release();
//    if (m_uiIndexBuffer) m_uiIndexBuffer->release();
//    if (m_vehicleGridVertexBuffer) m_vehicleGridVertexBuffer->release();
//    if (m_vehicleGridIndexBuffer) m_vehicleGridIndexBuffer->release();
//    if (m_itemIconAtlas) m_itemIconAtlas->release();
//    if (m_uiElementsTexture) m_uiElementsTexture->release();
//    
//    m_library->release();
//    m_device->release();
//}
//
//void InventorySystem::initializeSlots() {
//    m_slots.resize(INVENTORY_SIZE);
//    
//    for (int i = 0; i < INVENTORY_SIZE; i++) {
//        m_slots[i].index = i;
//        m_slots[i].size = {48, 48};
//    }
//    
//    updateSlotPositions();
//}
//
//void InventorySystem::createRenderPipelines() {
//    // UI rendering pipeline
//    MTL::RenderPipelineDescriptor* uiDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//    MTL::Function* uiVertFunc = m_library->newFunction(
//        NS::String::string("inventoryUIVertex", NS::UTF8StringEncoding));
//    MTL::Function* uiFragFunc = m_library->newFunction(
//        NS::String::string("inventoryUIFragment", NS::UTF8StringEncoding));
//    
//    if (uiVertFunc && uiFragFunc) {
//        uiDesc->setVertexFunction(uiVertFunc);
//        uiDesc->setFragmentFunction(uiFragFunc);
//        uiDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
//        
//        // Enable alpha blending
//        auto colorAttachment = uiDesc->colorAttachments()->object(0);
//        colorAttachment->setBlendingEnabled(true);
//        colorAttachment->setRgbBlendOperation(MTL::BlendOperationAdd);
//        colorAttachment->setAlphaBlendOperation(MTL::BlendOperationAdd);
//        colorAttachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
//        colorAttachment->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
//        colorAttachment->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        colorAttachment->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
//        
//        NS::Error* error = nullptr;
//        m_uiPipeline = m_device->newRenderPipelineState(uiDesc, &error);
//        
//        uiVertFunc->release();
//        uiFragFunc->release();
//    }
//    uiDesc->release();
//    
//    // Vehicle grid rendering pipeline
//    MTL::RenderPipelineDescriptor* gridDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//    MTL::Function* gridVertFunc = m_library->newFunction(
//        NS::String::string("vehicleGridVertex", NS::UTF8StringEncoding));
//    MTL::Function* gridFragFunc = m_library->newFunction(
//        NS::String::string("vehicleGridFragment", NS::UTF8StringEncoding));
//    
//    if (gridVertFunc && gridFragFunc) {
//        gridDesc->setVertexFunction(gridVertFunc);
//        gridDesc->setFragmentFunction(gridFragFunc);
//        gridDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA32Float);
//        gridDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
//        
//        NS::Error* error = nullptr;
//        m_vehicleGridPipeline = m_device->newRenderPipelineState(gridDesc, &error);
//        
//        gridVertFunc->release();
//        gridFragFunc->release();
//    }
//    gridDesc->release();
//    
//    // Allocate buffers
//    m_uiVertexBuffer = m_device->newBuffer(4096 * sizeof(UIVertex),
//                                          MTL::ResourceStorageModeShared);
//    m_uiIndexBuffer = m_device->newBuffer(8192 * sizeof(uint32_t),
//                                         MTL::ResourceStorageModeShared);
//    m_vehicleGridVertexBuffer = m_device->newBuffer(65536 * sizeof(simd::float3),
//                                                   MTL::ResourceStorageModeShared);
//    m_vehicleGridIndexBuffer = m_device->newBuffer(131072 * sizeof(uint32_t),
//                                                  MTL::ResourceStorageModeShared);
//}
//
//void InventorySystem::loadTextures() {
//    // TODO: Load actual textures
//    // For now, create placeholder textures
//    
//    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA32Float, 512, 512, false);
//    texDesc->setUsage(MTL::TextureUsageShaderRead);
//    
//    m_itemIconAtlas = m_device->newTexture(texDesc);
//    m_uiElementsTexture = m_device->newTexture(texDesc);
//}
//
//bool InventorySystem::addItem(ItemType type, uint16_t count) {
//    if (type == ItemType::None || count == 0) return false;
//    
//    uint16_t remaining = count;
//    
//    // Try to stack with existing items
//    for (auto& slot : m_slots) {
//        if (slot.item.type == type && slot.item.count < slot.item.maxStack) {
//            uint16_t space = slot.item.maxStack - slot.item.count;
//            uint16_t toAdd = std::min(remaining, space);
//            
//            slot.item.count += toAdd;
//            remaining -= toAdd;
//            
//            if (remaining == 0) return true;
//        }
//    }
//    
//    // Find empty slots
//    for (auto& slot : m_slots) {
//        if (slot.item.isEmpty()) {
//            slot.item = InventoryItem(type, std::min(remaining,
//                                     ItemProperties::getProperties(type).maxStack));
//            remaining -= slot.item.count;
//            
//            if (remaining == 0) return true;
//        }
//    }
//    
//    return remaining < count; // Partial success
//}
//
//bool InventorySystem::removeItem(ItemType type, uint16_t count) {
//    uint16_t toRemove = count;
//    
//    for (auto& slot : m_slots) {
//        if (slot.item.type == type) {
//            uint16_t removed = std::min(toRemove, slot.item.count);
//            slot.item.count -= removed;
//            toRemove -= removed;
//            
//            if (slot.item.count == 0) {
//                slot.item = InventoryItem();
//            }
//            
//            if (toRemove == 0) return true;
//        }
//    }
//    
//    return false;
//}
//
//bool InventorySystem::hasItem(ItemType type, uint16_t count) const {
//    return getItemCount(type) >= count;
//}
//
//int InventorySystem::getItemCount(ItemType type) const {
//    int total = 0;
//    for (const auto& slot : m_slots) {
//        if (slot.item.type == type) {
//            total += slot.item.count;
//        }
//    }
//    return total;
//}
//
//InventorySlot* InventorySystem::getSlot(int index) {
//    if (index >= 0 && index < INVENTORY_SIZE) {
//        return &m_slots[index];
//    }
//    return nullptr;
//}
//
//bool InventorySystem::swapSlots(int slotA, int slotB) {
//    if (slotA < 0 || slotA >= INVENTORY_SIZE || slotB < 0 || slotB >= INVENTORY_SIZE)
//        return false;
//    
//    std::swap(m_slots[slotA].item, m_slots[slotB].item);
//    return true;
//}
//
//bool InventorySystem::mergeSlots(int source, int dest) {
//    if (source < 0 || source >= INVENTORY_SIZE || dest < 0 || dest >= INVENTORY_SIZE)
//        return false;
//    
//    InventorySlot& srcSlot = m_slots[source];
//    InventorySlot& dstSlot = m_slots[dest];
//    
//    if (srcSlot.item.isEmpty()) return false;
//    
//    if (dstSlot.item.isEmpty()) {
//        dstSlot.item = srcSlot.item;
//        srcSlot.item = InventoryItem();
//        return true;
//    }
//    
//    if (srcSlot.item.canStack(dstSlot.item)) {
//        uint16_t space = dstSlot.item.maxStack - dstSlot.item.count;
//        uint16_t toTransfer = std::min(space, srcSlot.item.count);
//        
//        dstSlot.item.count += toTransfer;
//        srcSlot.item.count -= toTransfer;
//        
//        if (srcSlot.item.count == 0) {
//            srcSlot.item = InventoryItem();
//        }
//        
//        return true;
//    }
//    
//    return false;
//}
//
//void InventorySystem::clearSlot(int index) {
//    if (index >= 0 && index < INVENTORY_SIZE) {
//        m_slots[index].item = InventoryItem();
//    }
//}
//
//InventoryItem* InventorySystem::getSelectedItem() {
//    if (m_selectedHotbarSlot >= 0 && m_selectedHotbarSlot < HOTBAR_SIZE) {
//        return &m_slots[m_selectedHotbarSlot].item;
//    }
//    return nullptr;
//}
//
//void InventorySystem::handleMouseDown(const simd::float2& mousePos, bool rightClick) {
//    if (!m_uiState.isOpen) return;
//    
//    int slotIndex = findSlotAt(mousePos);
//    
//    if (slotIndex >= 0) {
//        if (!rightClick) {
//            // Start dragging
//            m_uiState.isDragging = true;
//            m_uiState.draggedSlotIndex = slotIndex;
//            m_uiState.dragOffset = mousePos - m_slots[slotIndex].position;
//        } else {
//            // Right click - split stack or quick move
//            if (!m_slots[slotIndex].item.isEmpty() && m_slots[slotIndex].item.count > 1) {
//                // Split stack (simplified)
//                uint16_t half = m_slots[slotIndex].item.count / 2;
//                // ... would need to handle where to place the split
//            }
//        }
//    }
//}
//
//void InventorySystem::handleMouseUp(const simd::float2& mousePos) {
//    if (!m_uiState.isDragging) return;
//    
//    int dropSlotIndex = findSlotAt(mousePos);
//    
//    if (dropSlotIndex >= 0 && dropSlotIndex != m_uiState.draggedSlotIndex) {
//        // Try to merge, otherwise swap
//        if (!mergeSlots(m_uiState.draggedSlotIndex, dropSlotIndex)) {
//            swapSlots(m_uiState.draggedSlotIndex, dropSlotIndex);
//        }
//    }
//    
//    m_uiState.isDragging = false;
//    m_uiState.draggedSlotIndex = -1;
//}
//
//void InventorySystem::handleMouseMove(const simd::float2& mousePos) {
//    // Update hover states
//    for (auto& slot : m_slots) {
//        float dx = mousePos.x - slot.position.x;
//        float dy = mousePos.y - slot.position.y;
//        slot.highlighted = (dx >= 0 && dx < slot.size.x && dy >= 0 && dy < slot.size.y);
//    }
//}
//
//void InventorySystem::handleScroll(float delta) {
//    // Scroll through hotbar
//    m_selectedHotbarSlot -= (int)delta;
//    if (m_selectedHotbarSlot < 0) m_selectedHotbarSlot = HOTBAR_SIZE - 1;
//    if (m_selectedHotbarSlot >= HOTBAR_SIZE) m_selectedHotbarSlot = 0;
//}
//
//int InventorySystem::findSlotAt(const simd::float2& pos) {
//    for (int i = 0; i < INVENTORY_SIZE; i++) {
//        const InventorySlot& slot = m_slots[i];
//        if (pos.x >= slot.position.x && pos.x < slot.position.x + slot.size.x &&
//            pos.y >= slot.position.y && pos.y < slot.position.y + slot.size.y) {
//            return i;
//        }
//    }
//    return -1;
//}
//
//void InventorySystem::updateSlotPositions() {
//    float slotSize = 52.0f;
//    float padding = 4.0f;
//    
//    simd::float2 windowPos = m_uiState.windowPosition;
//    simd::float2 startPos = windowPos + simd::float2{20, 60};
//    
//    // Main inventory grid (9x3)
//    for (int i = 0; i < 27; i++) {
//        int col = i % 9;
//        int row = i / 9;
//        m_slots[i].position = startPos + simd::float2{
//            col * (slotSize + padding),
//            row * (slotSize + padding)
//        };
//    }
//    
//    // Hotbar (9x1) below main inventory
//    startPos.y += (slotSize + padding) * 3 + 20;
//    for (int i = 0; i < HOTBAR_SIZE; i++) {
//        m_slots[27 + i].position = startPos + simd::float2{
//            i * (slotSize + padding), 0
//        };
//    }
//}
//
//void InventorySystem::updateUI(float deltaTime) {
//    updateSlotPositions();
//}
//
//void InventorySystem::renderUI(MTL::RenderCommandEncoder* encoder,
//                              const simd::float2& screenSize) {
//    if (!m_uiPipeline) return;
//    
//    // Render hotbar (always visible)
//    renderHotbar(encoder);
//    
//    // Render full inventory if open
//    if (m_uiState.isOpen) {
//        renderInventoryWindow(encoder);
//    }
//    
//    // Render dragged item on top
//    if (m_uiState.isDragging) {
//        renderDraggedItem(encoder);
//    }
//}
//
//void InventorySystem::addQuad(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices,
//                             const simd::float2& pos, const simd::float2& size,
//                             const simd::float2& uvMin, const simd::float2& uvMax,
//                             const simd::float4& color) {
//    uint32_t baseIndex = (uint32_t)vertices.size();
//    
//    vertices.push_back({{pos.x, pos.y}, {uvMin.x, uvMin.y}, color});
//    vertices.push_back({{pos.x + size.x, pos.y}, {uvMax.x, uvMin.y}, color});
//    vertices.push_back({{pos.x + size.x, pos.y + size.y}, {uvMax.x, uvMax.y}, color});
//    vertices.push_back({{pos.x, pos.y + size.y}, {uvMin.x, uvMax.y}, color});
//    
//    indices.push_back(baseIndex);
//    indices.push_back(baseIndex + 1);
//    indices.push_back(baseIndex + 2);
//    indices.push_back(baseIndex);
//    indices.push_back(baseIndex + 2);
//    indices.push_back(baseIndex + 3);
//}
//
//void InventorySystem::renderInventoryWindow(MTL::RenderCommandEncoder* encoder) {
//    std::vector<UIVertex> vertices;
//    std::vector<uint32_t> indices;
//    
//    // Background
//    addQuad(vertices, indices, m_uiState.windowPosition, m_uiState.windowSize,
//           {0, 0}, {1, 1}, {0.1f, 0.1f, 0.15f, 0.9f});
//    
//    // Render slots
//    for (const auto& slot : m_slots) {
//        simd::float4 slotColor = slot.highlighted ?
//            simd::float4{0.4f, 0.4f, 0.5f, 1.0f} :
//            simd::float4{0.2f, 0.2f, 0.25f, 1.0f};
//        
//        addQuad(vertices, indices, slot.position, slot.size,
//               {0, 0}, {1, 1}, slotColor);
//        
//        // Render item if present
//        if (!slot.item.isEmpty()) {
//            addQuad(vertices, indices, slot.position + simd::float2{4, 4},
//                   slot.size - simd::float2{8, 8},
//                   {0, 0}, {1, 1}, slot.item.color);
//        }
//    }
//    
//    // Upload and draw
//    if (!vertices.empty()) {
//        memcpy(m_uiVertexBuffer->contents(), vertices.data(),
//               vertices.size() * sizeof(UIVertex));
//        memcpy(m_uiIndexBuffer->contents(), indices.data(),
//               indices.size() * sizeof(uint32_t));
//        
//        encoder->setRenderPipelineState(m_uiPipeline);
//        encoder->setVertexBuffer(m_uiVertexBuffer, 0, 0);
//        encoder->setVertexBytes(&m_screenWidth, sizeof(float), 1);
//        encoder->setVertexBytes(&m_screenHeight, sizeof(float), 2);
//        
//        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
//                                      indices.size(),
//                                      MTL::IndexTypeUInt32,
//                                      m_uiIndexBuffer, 0);
//    }
//}
//
//void InventorySystem::renderHotbar(MTL::RenderCommandEncoder* encoder) {
//    // Simplified hotbar rendering at bottom of screen
//    // ...implementation similar to renderInventoryWindow
//}
//
//void InventorySystem::renderDraggedItem(MTL::RenderCommandEncoder* encoder) {
//    // Render dragged item following cursor
//    // ...implementation
//}
//
//bool InventorySystem::placeBlockOnVehicle(const simd::int3& gridPos, ItemType type) {
//    // Check if we have the item
//    if (!hasItem(type, 1)) return false;
//    
//    // Place block
//    m_vehicleGrid.setBlock(gridPos.x, gridPos.y, gridPos.z, type);
//    
//    // Remove from inventory
//    removeItem(type, 1);
//    
//    return true;
//}
//
//bool InventorySystem::removeBlockFromVehicle(const simd::int3& gridPos) {
//    auto* cell = m_vehicleGrid.getCell(gridPos.x, gridPos.y, gridPos.z);
//    if (!cell || !cell->active) return false;
//    
//    // Add to inventory
//    addItem(cell->blockType, 1);
//    
//    // Remove from grid
//    m_vehicleGrid.removeBlock(gridPos.x, gridPos.y, gridPos.z);
//    
//    return true;
//}
//
//void InventorySystem::renderVehicleGrid(MTL::RenderCommandEncoder* encoder,
//                                       const simd::float4x4& viewProjection,
//                                       const simd::float3& vehiclePosition) {
//    if (!m_vehicleGridPipeline) return;
//    
//    std::vector<simd::float3> vertices;
//    std::vector<uint32_t> indices;
//    
//    // Generate mesh for active blocks
//    for (int z = 0; z < VehicleGrid::GRID_SIZE; z++) {
//        for (int y = 0; y < VehicleGrid::GRID_SIZE; y++) {
//            for (int x = 0; x < VehicleGrid::GRID_SIZE; x++) {
//                const auto* cell = m_vehicleGrid.getCell(x, y, z);
//                if (!cell || !cell->active) continue;
//                
//                renderVehicleGridCube(x, y, z, *cell, vertices, indices, vehiclePosition);
//            }
//        }
//    }
//    
//    if (!vertices.empty()) {
//        memcpy(m_vehicleGridVertexBuffer->contents(), vertices.data(),
//               vertices.size() * sizeof(simd::float3));
//        memcpy(m_vehicleGridIndexBuffer->contents(), indices.data(),
//               indices.size() * sizeof(uint32_t));
//        
//        encoder->setRenderPipelineState(m_vehicleGridPipeline);
//        encoder->setVertexBuffer(m_vehicleGridVertexBuffer, 0, 0);
//        encoder->setVertexBytes(&viewProjection, sizeof(simd::float4x4), 1);
//        
//        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
//                                      indices.size(),
//                                      MTL::IndexTypeUInt32,
//                                      m_vehicleGridIndexBuffer, 0);
//    }
//}
//
//void InventorySystem::renderVehicleGridCube(int x, int y, int z,
//                                           const VehicleGrid::GridCell& cell,
//                                           std::vector<simd::float3>& vertices,
//                                           std::vector<uint32_t>& indices,
//                                           const simd::float3& offset) {
//    // Generate cube mesh for this block
//    // ...simplified cube generation
//}
//
//bool InventorySystem::saveToFile(const std::string& filepath) {
//    std::ofstream file(filepath, std::ios::binary);
//    if (!file.is_open()) return false;
//    
//    // Save inventory
//    for (const auto& slot : m_slots) {
//        file.write(reinterpret_cast<const char*>(&slot.item.type), sizeof(ItemType));
//        file.write(reinterpret_cast<const char*>(&slot.item.count), sizeof(uint16_t));
//    }
//    
//    // Save vehicle grid
//    for (const auto& cell : m_vehicleGrid.cells) {
//        file.write(reinterpret_cast<const char*>(&cell.blockType), sizeof(ItemType));
//        file.write(reinterpret_cast<const char*>(&cell.active), sizeof(bool));
//    }
//    
//    file.close();
//    return true;
//}
//
//bool InventorySystem::loadFromFile(const std::string& filepath) {
//    std::ifstream file(filepath, std::ios::binary);
//    if (!file.is_open()) return false;
//    
//    // Load inventory
//    for (auto& slot : m_slots) {
//        ItemType type;
//        uint16_t count;
//        file.read(reinterpret_cast<char*>(&type), sizeof(ItemType));
//        file.read(reinterpret_cast<char*>(&count), sizeof(uint16_t));
//        slot.item = InventoryItem(type, count);
//    }
//    
//    // Load vehicle grid
//    for (auto& cell : m_vehicleGrid.cells) {
//        file.read(reinterpret_cast<char*>(&cell.blockType), sizeof(ItemType));
//        file.read(reinterpret_cast<char*>(&cell.active), sizeof(bool));
//        if (cell.active) {
//            cell.color = ItemProperties::getItemColor(cell.blockType);
//        }
//    }
//    
//    file.close();
//    return true;
//}
