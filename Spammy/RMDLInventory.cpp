//
//  RMDLInventory.cpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include "RMDLInventory.hpp"

void InventoryItem::updateProperties() {
    auto props = ItemProperties::getProperties(type);
    name = props.name;
    color = props.color;
    maxStack = props.maxStack;
}

ItemProperties ItemProperties::getProperties(ItemType type) {
    switch (type) {
        case ItemType::CommandBlock:
            return {type, "Command Block", {1.0f, 0.5f, 0.0f, 1.0f}, 1, true, 100.0f};
        case ItemType::SolidBlock:
            return {type, "Solid Block", {0.5f, 0.5f, 0.5f, 1.0f}, 64, true, 50.0f};
        case ItemType::GlassBlock:
            return {type, "Glass Block", {0.8f, 0.9f, 1.0f, 0.6f}, 64, true, 20.0f};
        case ItemType::MetalBlock:
            return {type, "Metal Block", {0.7f, 0.7f, 0.8f, 1.0f}, 64, true, 150.0f};
        case ItemType::CrystalBlock:
            return {type, "Crystal Block", {0.5f, 0.8f, 1.0f, 0.8f}, 64, true, 30.0f};
        case ItemType::EngineBlock:
            return {type, "Engine Block", {0.9f, 0.3f, 0.2f, 1.0f}, 16, true, 200.0f};
        case ItemType::ThrusterBlock:
            return {type, "Thruster Block", {0.3f, 0.5f, 0.9f, 1.0f}, 16, true, 120.0f};
        case ItemType::WeaponBlock:
            return {type, "Weapon Block", {0.8f, 0.2f, 0.2f, 1.0f}, 8, true, 180.0f};
        case ItemType::ShieldBlock:
            return {type, "Shield Block", {0.4f, 0.7f, 1.0f, 0.7f}, 16, true, 90.0f};
        default:
            return {ItemType::None, "Empty", {1,1,1,1}, 1, false, 0.0f};
    }
}

const char* ItemProperties::getItemName(ItemType type) {
    return getProperties(type).name.c_str();
}

simd::float4 ItemProperties::getItemColor(ItemType type) {
    return getProperties(type).color;
}

// ============================================================================
// Vehicle Grid
// ============================================================================

void VehicleGrid::setBlock(int x, int y, int z, ItemType type) {
    GridCell* cell = getCell(x, y, z);
    if (!cell) return;
    
    cell->blockType = type;
    cell->active = true;
    cell->color = ItemProperties::getItemColor(type);
    
    // Check if this is the command block
    if (type == ItemType::CommandBlock) {
        centerBlock = {x, y, z};
        hasCenterBlock = true;
    }
}

void VehicleGrid::removeBlock(int x, int y, int z) {
    GridCell* cell = getCell(x, y, z);
    if (!cell) return;
    
    if (x == centerBlock.x && y == centerBlock.y && z == centerBlock.z) {
        hasCenterBlock = false;
    }
    
    cell->blockType = ItemType::None;
    cell->active = false;
}

bool VehicleGrid::isConnected(int x, int y, int z) {
    if (!hasCenterBlock) return false;
    
    // BFS to check connectivity to command block
    std::vector<bool> visited(GRID_SIZE * GRID_SIZE * GRID_SIZE, false);
    std::vector<simd::int3> queue;
    
    queue.push_back(centerBlock);
    int idx = centerBlock.x + centerBlock.y * GRID_SIZE + centerBlock.z * GRID_SIZE * GRID_SIZE;
    visited[idx] = true;
    
    simd::int3 target = {x, y, z};
    
    while (!queue.empty()) {
        simd::int3 current = queue.back();
        queue.pop_back();
        
        if (current.x == target.x && current.y == target.y && current.z == target.z) {
            return true;
        }
        
        // Check 6 neighbors
        simd::int3 offsets[6] = {
            {-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1}
        };
        
        for (const auto& offset : offsets) {
            simd::int3 neighbor = current + offset;
            
            GridCell* cell = getCell(neighbor.x, neighbor.y, neighbor.z);
            if (!cell || !cell->active) continue;
            
            int nIdx = neighbor.x + neighbor.y * GRID_SIZE + neighbor.z * GRID_SIZE * GRID_SIZE;
            if (visited[nIdx]) continue;
            
            visited[nIdx] = true;
            queue.push_back(neighbor);
        }
    }
    
    return false;
}

void VehicleGrid::calculateMassAndProperties(float& mass, simd::float3& centerOfMass) {
    mass = 0.0f;
    centerOfMass = {0, 0, 0};
    int activeCount = 0;
    
    for (int z = 0; z < GRID_SIZE; z++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                GridCell* cell = getCell(x, y, z);
                if (!cell || !cell->active) continue;
                
                float blockMass = ItemProperties::getProperties(cell->blockType).mass;
                mass += blockMass;
                
                centerOfMass += simd::float3{(float)x, (float)y, (float)z} * blockMass;
                activeCount++;
            }
        }
    }
    
    if (mass > 0.0f) {
        centerOfMass /= mass;
    }
}

// ============================================================================
// Inventory System
// ============================================================================

InventorySystem::InventorySystem(MTL::Device* device, MTL::Library* library,
                                 float screenWidth, float screenHeight)
    : m_device(device->retain())
    , m_library(library->retain())
    , m_selectedHotbarSlot(0)
    , m_vehicleBuildMode(false)
    , m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    initializeSlots();
    createRenderPipelines();
    loadTextures();
    
    // Give player starting items
    addItem(ItemType::CommandBlock, 1);
    addItem(ItemType::SolidBlock, 64);
    addItem(ItemType::MetalBlock, 32);
}

InventorySystem::~InventorySystem() {
    if (m_uiPipeline) m_uiPipeline->release();
    if (m_vehicleGridPipeline) m_vehicleGridPipeline->release();
    if (m_uiVertexBuffer) m_uiVertexBuffer->release();
    if (m_uiIndexBuffer) m_uiIndexBuffer->release();
    if (m_vehicleGridVertexBuffer) m_vehicleGridVertexBuffer->release();
    if (m_vehicleGridIndexBuffer) m_vehicleGridIndexBuffer->release();
    if (m_itemIconAtlas) m_itemIconAtlas->release();
    if (m_uiElementsTexture) m_uiElementsTexture->release();
    
    m_library->release();
    m_device->release();
}

void InventorySystem::initializeSlots() {
    m_slots.resize(INVENTORY_SIZE);
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        m_slots[i].index = i;
        m_slots[i].size = {48, 48};
    }
    
    updateSlotPositions();
}

void InventorySystem::createRenderPipelines() {
    // UI rendering pipeline
    MTL::RenderPipelineDescriptor* uiDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    MTL::Function* uiVertFunc = m_library->newFunction(
        NS::String::string("inventoryUIVertex", NS::UTF8StringEncoding));
    MTL::Function* uiFragFunc = m_library->newFunction(
        NS::String::string("inventoryUIFragment", NS::UTF8StringEncoding));
    
    if (uiVertFunc && uiFragFunc) {
        uiDesc->setVertexFunction(uiVertFunc);
        uiDesc->setFragmentFunction(uiFragFunc);
        uiDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
        
        // Enable alpha blending
        auto colorAttachment = uiDesc->colorAttachments()->object(0);
        colorAttachment->setBlendingEnabled(true);
        colorAttachment->setRgbBlendOperation(MTL::BlendOperationAdd);
        colorAttachment->setAlphaBlendOperation(MTL::BlendOperationAdd);
        colorAttachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        colorAttachment->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
        colorAttachment->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        colorAttachment->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        
        NS::Error* error = nullptr;
        m_uiPipeline = m_device->newRenderPipelineState(uiDesc, &error);
        
        uiVertFunc->release();
        uiFragFunc->release();
    }
    uiDesc->release();
    
    // Vehicle grid rendering pipeline
    MTL::RenderPipelineDescriptor* gridDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    MTL::Function* gridVertFunc = m_library->newFunction(
        NS::String::string("vehicleGridVertex", NS::UTF8StringEncoding));
    MTL::Function* gridFragFunc = m_library->newFunction(
        NS::String::string("vehicleGridFragment", NS::UTF8StringEncoding));
    
    if (gridVertFunc && gridFragFunc) {
        gridDesc->setVertexFunction(gridVertFunc);
        gridDesc->setFragmentFunction(gridFragFunc);
        gridDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA32Float);
        gridDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
        
        NS::Error* error = nullptr;
        m_vehicleGridPipeline = m_device->newRenderPipelineState(gridDesc, &error);
        
        gridVertFunc->release();
        gridFragFunc->release();
    }
    gridDesc->release();
    
    // Allocate buffers
    m_uiVertexBuffer = m_device->newBuffer(4096 * sizeof(UIVertex),
                                          MTL::ResourceStorageModeShared);
    m_uiIndexBuffer = m_device->newBuffer(8192 * sizeof(uint32_t),
                                         MTL::ResourceStorageModeShared);
    m_vehicleGridVertexBuffer = m_device->newBuffer(65536 * sizeof(simd::float3),
                                                   MTL::ResourceStorageModeShared);
    m_vehicleGridIndexBuffer = m_device->newBuffer(131072 * sizeof(uint32_t),
                                                  MTL::ResourceStorageModeShared);
}

void InventorySystem::loadTextures() {
    // TODO: Load actual textures
    // For now, create placeholder textures
    
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA32Float, 512, 512, false);
    texDesc->setUsage(MTL::TextureUsageShaderRead);
    
    m_itemIconAtlas = m_device->newTexture(texDesc);
    m_uiElementsTexture = m_device->newTexture(texDesc);
}

bool InventorySystem::addItem(ItemType type, uint16_t count) {
    if (type == ItemType::None || count == 0) return false;
    
    uint16_t remaining = count;
    
    // Try to stack with existing items
    for (auto& slot : m_slots) {
        if (slot.item.type == type && slot.item.count < slot.item.maxStack) {
            uint16_t space = slot.item.maxStack - slot.item.count;
            uint16_t toAdd = std::min(remaining, space);
            
            slot.item.count += toAdd;
            remaining -= toAdd;
            
            if (remaining == 0) return true;
        }
    }
    
    // Find empty slots
    for (auto& slot : m_slots) {
        if (slot.item.isEmpty()) {
            slot.item = InventoryItem(type, std::min(remaining,
                                     ItemProperties::getProperties(type).maxStack));
            remaining -= slot.item.count;
            
            if (remaining == 0) return true;
        }
    }
    
    return remaining < count; // Partial success
}

bool InventorySystem::removeItem(ItemType type, uint16_t count) {
    uint16_t toRemove = count;
    
    for (auto& slot : m_slots) {
        if (slot.item.type == type) {
            uint16_t removed = std::min(toRemove, slot.item.count);
            slot.item.count -= removed;
            toRemove -= removed;
            
            if (slot.item.count == 0) {
                slot.item = InventoryItem();
            }
            
            if (toRemove == 0) return true;
        }
    }
    
    return false;
}

bool InventorySystem::hasItem(ItemType type, uint16_t count) const {
    return getItemCount(type) >= count;
}

int InventorySystem::getItemCount(ItemType type) const {
    int total = 0;
    for (const auto& slot : m_slots) {
        if (slot.item.type == type) {
            total += slot.item.count;
        }
    }
    return total;
}

InventorySlot* InventorySystem::getSlot(int index) {
    if (index >= 0 && index < INVENTORY_SIZE) {
        return &m_slots[index];
    }
    return nullptr;
}

bool InventorySystem::swapSlots(int slotA, int slotB) {
    if (slotA < 0 || slotA >= INVENTORY_SIZE || slotB < 0 || slotB >= INVENTORY_SIZE)
        return false;
    
    std::swap(m_slots[slotA].item, m_slots[slotB].item);
    return true;
}

bool InventorySystem::mergeSlots(int source, int dest) {
    if (source < 0 || source >= INVENTORY_SIZE || dest < 0 || dest >= INVENTORY_SIZE)
        return false;
    
    InventorySlot& srcSlot = m_slots[source];
    InventorySlot& dstSlot = m_slots[dest];
    
    if (srcSlot.item.isEmpty()) return false;
    
    if (dstSlot.item.isEmpty()) {
        dstSlot.item = srcSlot.item;
        srcSlot.item = InventoryItem();
        return true;
    }
    
    if (srcSlot.item.canStack(dstSlot.item)) {
        uint16_t space = dstSlot.item.maxStack - dstSlot.item.count;
        uint16_t toTransfer = std::min(space, srcSlot.item.count);
        
        dstSlot.item.count += toTransfer;
        srcSlot.item.count -= toTransfer;
        
        if (srcSlot.item.count == 0) {
            srcSlot.item = InventoryItem();
        }
        
        return true;
    }
    
    return false;
}

void InventorySystem::clearSlot(int index) {
    if (index >= 0 && index < INVENTORY_SIZE) {
        m_slots[index].item = InventoryItem();
    }
}

InventoryItem* InventorySystem::getSelectedItem() {
    if (m_selectedHotbarSlot >= 0 && m_selectedHotbarSlot < HOTBAR_SIZE) {
        return &m_slots[m_selectedHotbarSlot].item;
    }
    return nullptr;
}

void InventorySystem::handleMouseDown(const simd::float2& mousePos, bool rightClick) {
    if (!m_uiState.isOpen) return;
    
    int slotIndex = findSlotAt(mousePos);
    
    if (slotIndex >= 0) {
        if (!rightClick) {
            // Start dragging
            m_uiState.isDragging = true;
            m_uiState.draggedSlotIndex = slotIndex;
            m_uiState.dragOffset = mousePos - m_slots[slotIndex].position;
        } else {
            // Right click - split stack or quick move
            if (!m_slots[slotIndex].item.isEmpty() && m_slots[slotIndex].item.count > 1) {
                // Split stack (simplified)
                uint16_t half = m_slots[slotIndex].item.count / 2;
                // ... would need to handle where to place the split
            }
        }
    }
}

void InventorySystem::handleMouseUp(const simd::float2& mousePos) {
    if (!m_uiState.isDragging) return;
    
    int dropSlotIndex = findSlotAt(mousePos);
    
    if (dropSlotIndex >= 0 && dropSlotIndex != m_uiState.draggedSlotIndex) {
        // Try to merge, otherwise swap
        if (!mergeSlots(m_uiState.draggedSlotIndex, dropSlotIndex)) {
            swapSlots(m_uiState.draggedSlotIndex, dropSlotIndex);
        }
    }
    
    m_uiState.isDragging = false;
    m_uiState.draggedSlotIndex = -1;
}

void InventorySystem::handleMouseMove(const simd::float2& mousePos) {
    // Update hover states
    for (auto& slot : m_slots) {
        float dx = mousePos.x - slot.position.x;
        float dy = mousePos.y - slot.position.y;
        slot.highlighted = (dx >= 0 && dx < slot.size.x && dy >= 0 && dy < slot.size.y);
    }
}

void InventorySystem::handleScroll(float delta) {
    // Scroll through hotbar
    m_selectedHotbarSlot -= (int)delta;
    if (m_selectedHotbarSlot < 0) m_selectedHotbarSlot = HOTBAR_SIZE - 1;
    if (m_selectedHotbarSlot >= HOTBAR_SIZE) m_selectedHotbarSlot = 0;
}

int InventorySystem::findSlotAt(const simd::float2& pos) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        const InventorySlot& slot = m_slots[i];
        if (pos.x >= slot.position.x && pos.x < slot.position.x + slot.size.x &&
            pos.y >= slot.position.y && pos.y < slot.position.y + slot.size.y) {
            return i;
        }
    }
    return -1;
}

void InventorySystem::updateSlotPositions() {
    float slotSize = 52.0f;
    float padding = 4.0f;
    
    simd::float2 windowPos = m_uiState.windowPosition;
    simd::float2 startPos = windowPos + simd::float2{20, 60};
    
    // Main inventory grid (9x3)
    for (int i = 0; i < 27; i++) {
        int col = i % 9;
        int row = i / 9;
        m_slots[i].position = startPos + simd::float2{
            col * (slotSize + padding),
            row * (slotSize + padding)
        };
    }
    
    // Hotbar (9x1) below main inventory
    startPos.y += (slotSize + padding) * 3 + 20;
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        m_slots[27 + i].position = startPos + simd::float2{
            i * (slotSize + padding), 0
        };
    }
}

void InventorySystem::updateUI(float deltaTime) {
    updateSlotPositions();
}

void InventorySystem::renderUI(MTL::RenderCommandEncoder* encoder,
                              const simd::float2& screenSize) {
    if (!m_uiPipeline) return;
    
    // Render hotbar (always visible)
    renderHotbar(encoder);
    
    // Render full inventory if open
    if (m_uiState.isOpen) {
        renderInventoryWindow(encoder);
    }
    
    // Render dragged item on top
    if (m_uiState.isDragging) {
        renderDraggedItem(encoder);
    }
}

void InventorySystem::addQuad(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices,
                             const simd::float2& pos, const simd::float2& size,
                             const simd::float2& uvMin, const simd::float2& uvMax,
                             const simd::float4& color) {
    uint32_t baseIndex = (uint32_t)vertices.size();
    
    vertices.push_back({{pos.x, pos.y}, {uvMin.x, uvMin.y}, color});
    vertices.push_back({{pos.x + size.x, pos.y}, {uvMax.x, uvMin.y}, color});
    vertices.push_back({{pos.x + size.x, pos.y + size.y}, {uvMax.x, uvMax.y}, color});
    vertices.push_back({{pos.x, pos.y + size.y}, {uvMin.x, uvMax.y}, color});
    
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

void InventorySystem::renderInventoryWindow(MTL::RenderCommandEncoder* encoder) {
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    
    // Background
    addQuad(vertices, indices, m_uiState.windowPosition, m_uiState.windowSize,
           {0, 0}, {1, 1}, {0.1f, 0.1f, 0.15f, 0.9f});
    
    // Render slots
    for (const auto& slot : m_slots) {
        simd::float4 slotColor = slot.highlighted ?
            simd::float4{0.4f, 0.4f, 0.5f, 1.0f} :
            simd::float4{0.2f, 0.2f, 0.25f, 1.0f};
        
        addQuad(vertices, indices, slot.position, slot.size,
               {0, 0}, {1, 1}, slotColor);
        
        // Render item if present
        if (!slot.item.isEmpty()) {
            addQuad(vertices, indices, slot.position + simd::float2{4, 4},
                   slot.size - simd::float2{8, 8},
                   {0, 0}, {1, 1}, slot.item.color);
        }
    }
    
    // Upload and draw
    if (!vertices.empty()) {
        memcpy(m_uiVertexBuffer->contents(), vertices.data(),
               vertices.size() * sizeof(UIVertex));
        memcpy(m_uiIndexBuffer->contents(), indices.data(),
               indices.size() * sizeof(uint32_t));
        
        encoder->setRenderPipelineState(m_uiPipeline);
        encoder->setVertexBuffer(m_uiVertexBuffer, 0, 0);
        encoder->setVertexBytes(&m_screenWidth, sizeof(float), 1);
        encoder->setVertexBytes(&m_screenHeight, sizeof(float), 2);
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                      indices.size(),
                                      MTL::IndexTypeUInt32,
                                      m_uiIndexBuffer, 0);
    }
}

void InventorySystem::renderHotbar(MTL::RenderCommandEncoder* encoder) {
    // Simplified hotbar rendering at bottom of screen
    // ...implementation similar to renderInventoryWindow
}

void InventorySystem::renderDraggedItem(MTL::RenderCommandEncoder* encoder) {
    // Render dragged item following cursor
    // ...implementation
}

bool InventorySystem::placeBlockOnVehicle(const simd::int3& gridPos, ItemType type) {
    // Check if we have the item
    if (!hasItem(type, 1)) return false;
    
    // Place block
    m_vehicleGrid.setBlock(gridPos.x, gridPos.y, gridPos.z, type);
    
    // Remove from inventory
    removeItem(type, 1);
    
    return true;
}

bool InventorySystem::removeBlockFromVehicle(const simd::int3& gridPos) {
    auto* cell = m_vehicleGrid.getCell(gridPos.x, gridPos.y, gridPos.z);
    if (!cell || !cell->active) return false;
    
    // Add to inventory
    addItem(cell->blockType, 1);
    
    // Remove from grid
    m_vehicleGrid.removeBlock(gridPos.x, gridPos.y, gridPos.z);
    
    return true;
}

void InventorySystem::renderVehicleGrid(MTL::RenderCommandEncoder* encoder,
                                       const simd::float4x4& viewProjection,
                                       const simd::float3& vehiclePosition) {
    if (!m_vehicleGridPipeline) return;
    
    std::vector<simd::float3> vertices;
    std::vector<uint32_t> indices;
    
    // Generate mesh for active blocks
    for (int z = 0; z < VehicleGrid::GRID_SIZE; z++) {
        for (int y = 0; y < VehicleGrid::GRID_SIZE; y++) {
            for (int x = 0; x < VehicleGrid::GRID_SIZE; x++) {
                const auto* cell = m_vehicleGrid.getCell(x, y, z);
                if (!cell || !cell->active) continue;
                
                renderVehicleGridCube(x, y, z, *cell, vertices, indices, vehiclePosition);
            }
        }
    }
    
    if (!vertices.empty()) {
        memcpy(m_vehicleGridVertexBuffer->contents(), vertices.data(),
               vertices.size() * sizeof(simd::float3));
        memcpy(m_vehicleGridIndexBuffer->contents(), indices.data(),
               indices.size() * sizeof(uint32_t));
        
        encoder->setRenderPipelineState(m_vehicleGridPipeline);
        encoder->setVertexBuffer(m_vehicleGridVertexBuffer, 0, 0);
        encoder->setVertexBytes(&viewProjection, sizeof(simd::float4x4), 1);
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                      indices.size(),
                                      MTL::IndexTypeUInt32,
                                      m_vehicleGridIndexBuffer, 0);
    }
}

void InventorySystem::renderVehicleGridCube(int x, int y, int z,
                                           const VehicleGrid::GridCell& cell,
                                           std::vector<simd::float3>& vertices,
                                           std::vector<uint32_t>& indices,
                                           const simd::float3& offset) {
    // Generate cube mesh for this block
    // ...simplified cube generation
}

bool InventorySystem::saveToFile(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;
    
    // Save inventory
    for (const auto& slot : m_slots) {
        file.write(reinterpret_cast<const char*>(&slot.item.type), sizeof(ItemType));
        file.write(reinterpret_cast<const char*>(&slot.item.count), sizeof(uint16_t));
    }
    
    // Save vehicle grid
    for (const auto& cell : m_vehicleGrid.cells) {
        file.write(reinterpret_cast<const char*>(&cell.blockType), sizeof(ItemType));
        file.write(reinterpret_cast<const char*>(&cell.active), sizeof(bool));
    }
    
    file.close();
    return true;
}

bool InventorySystem::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;
    
    // Load inventory
    for (auto& slot : m_slots) {
        ItemType type;
        uint16_t count;
        file.read(reinterpret_cast<char*>(&type), sizeof(ItemType));
        file.read(reinterpret_cast<char*>(&count), sizeof(uint16_t));
        slot.item = InventoryItem(type, count);
    }
    
    // Load vehicle grid
    for (auto& cell : m_vehicleGrid.cells) {
        file.read(reinterpret_cast<char*>(&cell.blockType), sizeof(ItemType));
        file.read(reinterpret_cast<char*>(&cell.active), sizeof(bool));
        if (cell.active) {
            cell.color = ItemProperties::getItemColor(cell.blockType);
        }
    }
    
    file.close();
    return true;
}
