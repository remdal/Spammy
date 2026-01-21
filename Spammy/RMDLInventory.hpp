//
//  RMDLInventory.hpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#ifndef RMDLInventory_hpp
#define RMDLInventory_hpp

#include <Metal/Metal.hpp>

#include <simd/simd.h>
#include <vector>
#include <string>

namespace inventoryWindow {

struct InventoryPanelVertex
{
    simd::float2 position;
    simd::float2 texCoord;
    simd::float4 color;
    float cornerRadius;
    float borderWidth;
    uint32_t elementType; // 0=slot, 1=icon, 2=handle, 3=scrollIndicator
    uint32_t slotIndex;
};

struct InventoryPanelUniforms
{
    simd::float2 screenSize;
    simd::float2 panelOrigin;
    simd::float2 slotDimensions;
    float slotSpacing;
    float time;
    int32_t hoveredSlotIndex;
    int32_t selectedSlotIndex;
    simd::float4 slotNormalColor;
    simd::float4 slotHoveredColor;
    simd::float4 slotSelectedColor;
    simd::float4 handleColor;
    float slotCornerRadius;
    float scrollOffset;
    float maxScrollOffset;
    float handleSize;
};

struct InventoryItemData
{
    uint32_t itemTypeID;
    uint32_t itemCount;
    simd::float4 displayColor;
    MTL::Texture* iconTexture; // PNG
    bool hasItem;
    
    InventoryItemData();
};

class InventoryPanel
{
public:
    static constexpr uint32_t VISIBLE_COLUMNS = 10;
    static constexpr uint32_t VISIBLE_ROWS = 4;
    static constexpr uint32_t TOTAL_ROWS = 8;
    static constexpr uint32_t TOTAL_SLOTS = VISIBLE_COLUMNS * TOTAL_ROWS;
    
    InventoryPanel(MTL::Device* device, MTL::PixelFormat colorPixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Library* shaderLibrary);
    ~InventoryPanel();
    
    void render(MTL::RenderCommandEncoder* renderCommandEncoder, simd::float2 screenSize);
    void update(float deltaTime);
    
    void onMouseDown(simd::float2 screenPosition, simd::float2 screenSize);
    void onMouseUp(simd::float2 screenPosition, simd::float2 screenSize);
    void onMouseMoved(simd::float2 screenPosition, simd::float2 screenSize);
    void onMouseScroll(float deltaY);
    
    void setSlotItem(uint32_t slotIndex, uint32_t typeID, uint32_t count, simd::float4 color);
    void setSlotTexture(uint32_t slotIndex, MTL::Texture* texture);
    void clearSlot(uint32_t slotIndex);
    
    MTL::Texture* loadIconTexture(const std::string& filePath);
    
    int32_t getHoveredSlot() const;
    int32_t getSelectedSlot() const;
    void setSelectedSlot(int32_t slotIndex);
    
    void setVisible(bool visible);
    bool isVisible() const;
    
private:
    MTL::Device* m_device;
    MTL::RenderPipelineState* m_slotPipeline;
    MTL::RenderPipelineState* m_iconPipeline;
    MTL::DepthStencilState* m_depthState;
    MTL::SamplerState* m_iconSampler;
    MTL::Buffer* m_vertexBuffer;
    MTL::Buffer* m_indexBuffer;
    MTL::Buffer* m_uniformBuffer;
    
    InventoryPanelUniforms m_uniforms;
    InventoryItemData m_items[TOTAL_SLOTS];
    
    simd::float2 m_panelPosition;
    simd::float2 m_slotSizePixels;
    float m_slotSpacingPixels;
    float m_handleSizePixels;
    float m_handleThicknessPixels;
    
    bool m_isVisible;
    bool m_isDraggingPanel;
    bool m_isDraggingHandle;
    simd::float2 m_dragOffset;
    int32_t m_activeHandle; // 0=top, 1=bottom, 2=left, 3=right
    
    float m_scrollOffset;
    float m_scrollVelocity;
    float m_maxScrollOffset;
    
    int32_t m_hoveredSlotIndex;
    int32_t m_selectedSlotIndex;
    float m_time;
    
    uint32_t m_vertexCount;
    uint32_t m_indexCount;
    
    void buildPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat, MTL::Library* library);
    void buildBuffers();
    void rebuildGeometry(simd::float2 screenSize);
    
    int32_t hitTestSlot(simd::float2 normPos, simd::float2 screenSize) const;
    int32_t hitTestHandle(simd::float2 normPos, simd::float2 screenSize) const;
    simd::float2 getSlotScreenPosition(uint32_t slotIndex, simd::float2 screenSize) const;
    simd::float2 getPanelDimensions(simd::float2 screenSize) const;
};

}

//// Block/Item type definition
//enum class ItemType : uint16_t {
//    None = 0,
//    CommandBlock = 1,      // Le bloc carré commandant
//    SolidBlock = 2,
//    GlassBlock = 3,
//    MetalBlock = 4,
//    CrystalBlock = 5,
//    EngineBlock = 6,
//    ThrusterBlock = 7,
//    WeaponBlock = 8,
//    ShieldBlock = 9,
//    COUNT
//};
//
//// Item stack in inventory
//struct InventoryItem {
//    ItemType type;
//    uint16_t count;
//    uint16_t maxStack;
//    simd::float4 color;
//    std::string name;
//    
//    InventoryItem()
//        : type(ItemType::None)
//        , count(0)
//        , maxStack(64)
//        , color{1, 1, 1, 1}
//        , name("Empty")
//    {}
//    
//    InventoryItem(ItemType t, uint16_t c = 1)
//        : type(t)
//        , count(c)
//        , maxStack(64)
//    {
//        updateProperties();
//    }
//    
//    void updateProperties();
//    bool isEmpty() const { return type == ItemType::None || count == 0; }
//    bool canStack(const InventoryItem& other) const {
//        return type == other.type && count < maxStack;
//    }
//};
//
//// Inventory slot
//struct InventorySlot {
//    int index;
//    InventoryItem item;
//    simd::float2 position;  // UI position
//    simd::float2 size;
//    bool highlighted;
//    bool locked;
//    
//    InventorySlot()
//        : index(0)
//        , position{0, 0}
//        , size{50, 50}
//        , highlighted(false)
//        , locked(false)
//    {}
//};
//
//// Vehicle grid (3D grid for vehicle construction)
//struct VehicleGrid {
//    static constexpr int GRID_SIZE = 16; // 16x16x16 vehicle grid
//    
//    struct GridCell {
//        ItemType blockType;
//        bool active;
//        simd::float4 color;
//        
//        GridCell() : blockType(ItemType::None), active(false), color{1,1,1,1} {}
//    };
//    
//    std::vector<GridCell> cells;
//    simd::int3 centerBlock;  // Position du bloc commandant (centre de contrôle)
//    bool hasCenterBlock;
//    
//    VehicleGrid() : hasCenterBlock(false) {
//        cells.resize(GRID_SIZE * GRID_SIZE * GRID_SIZE);
//        centerBlock = {GRID_SIZE/2, GRID_SIZE/2, GRID_SIZE/2};
//    }
//    
//    GridCell* getCell(int x, int y, int z) {
//        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE || z < 0 || z >= GRID_SIZE)
//            return nullptr;
//        return &cells[x + y * GRID_SIZE + z * GRID_SIZE * GRID_SIZE];
//    }
//    
//    void setBlock(int x, int y, int z, ItemType type);
//    void removeBlock(int x, int y, int z);
//    bool isConnected(int x, int y, int z); // Check if block is connected to command block
//    void calculateMassAndProperties(float& mass, simd::float3& centerOfMass);
//};
//
//// Inventory UI state
//struct InventoryUIState {
//    bool isOpen;
//    bool isDragging;
//    int draggedSlotIndex;
//    simd::float2 dragOffset;
//    simd::float2 windowPosition;
//    simd::float2 windowSize;
//    
//    InventoryUIState()
//        : isOpen(false)
//        , isDragging(false)
//        , draggedSlotIndex(-1)
//        , dragOffset{0, 0}
//        , windowPosition{100, 100}
//        , windowSize{600, 400}
//    {}
//};
//
//class InventorySystem {
//public:
//    static constexpr int INVENTORY_SIZE = 36;  // 9x4 grid
//    static constexpr int HOTBAR_SIZE = 9;
//    
//    InventorySystem(MTL::Device* device, MTL::Library* library,
//                   float screenWidth, float screenHeight);
//    ~InventorySystem();
//    
//    // Inventory management
//    bool addItem(ItemType type, uint16_t count = 1);
//    bool removeItem(ItemType type, uint16_t count = 1);
//    bool hasItem(ItemType type, uint16_t count = 1) const;
//    int getItemCount(ItemType type) const;
//    
//    // Slot management
//    InventorySlot* getSlot(int index);
//    bool swapSlots(int slotA, int slotB);
//    bool mergeSlots(int source, int dest);
//    void clearSlot(int index);
//    
//    // Hotbar
//    void setSelectedHotbarSlot(int slot) { m_selectedHotbarSlot = slot; }
//    int getSelectedHotbarSlot() const { return m_selectedHotbarSlot; }
//    InventoryItem* getSelectedItem();
//    
//    // UI
//    void toggleInventory() { m_uiState.isOpen = !m_uiState.isOpen; }
//    bool isInventoryOpen() const { return m_uiState.isOpen; }
//    
//    void handleMouseDown(const simd::float2& mousePos, bool rightClick);
//    void handleMouseUp(const simd::float2& mousePos);
//    void handleMouseMove(const simd::float2& mousePos);
//    void handleScroll(float delta);
//    
//    void updateUI(float deltaTime);
//    void renderUI(MTL::RenderCommandEncoder* encoder,
//                 const simd::float2& screenSize);
//    
//    // Vehicle grid
//    VehicleGrid& getVehicleGrid() { return m_vehicleGrid; }
//    void renderVehicleGrid(MTL::RenderCommandEncoder* encoder,
//                          const simd::float4x4& viewProjection,
//                          const simd::float3& vehiclePosition);
//    
//    bool placeBlockOnVehicle(const simd::int3& gridPos, ItemType type);
//    bool removeBlockFromVehicle(const simd::int3& gridPos);
//    void clearVehicleGrid();
//    
//    // Vehicle build mode
//    void setVehicleBuildMode(bool enabled) { m_vehicleBuildMode = enabled; }
//    bool isVehicleBuildMode() const { return m_vehicleBuildMode; }
//    
//    // Save/Load
//    bool saveToFile(const std::string& filepath);
//    bool loadFromFile(const std::string& filepath);
//    
//private:
//    MTL::Device* m_device;
//    MTL::Library* m_library;
//    
//    // Inventory data
//    std::vector<InventorySlot> m_slots;
//    int m_selectedHotbarSlot;
//    VehicleGrid m_vehicleGrid;
//    bool m_vehicleBuildMode;
//    
//    // UI state
//    InventoryUIState m_uiState;
//    float m_screenWidth;
//    float m_screenHeight;
//    
//    // Rendering
//    MTL::RenderPipelineState* m_uiPipeline;
//    MTL::RenderPipelineState* m_vehicleGridPipeline;
//    MTL::Buffer* m_uiVertexBuffer;
//    MTL::Buffer* m_uiIndexBuffer;
//    MTL::Buffer* m_vehicleGridVertexBuffer;
//    MTL::Buffer* m_vehicleGridIndexBuffer;
//    
//    // Textures
//    MTL::Texture* m_itemIconAtlas;      // Atlas d'icônes d'items
//    MTL::Texture* m_uiElementsTexture;  // Textures UI (slots, bordures, etc.)
//    
//    // Helpers
//    void initializeSlots();
//    void createRenderPipelines();
//    void loadTextures();
//    
//    int findSlotAt(const simd::float2& pos);
//    void updateSlotPositions();
//    void renderInventoryWindow(MTL::RenderCommandEncoder* encoder);
//    void renderHotbar(MTL::RenderCommandEncoder* encoder);
//    void renderDraggedItem(MTL::RenderCommandEncoder* encoder);
//    void renderVehicleGridCube(int x, int y, int z, const VehicleGrid::GridCell& cell,
//                              std::vector<simd::float3>& vertices,
//                              std::vector<uint32_t>& indices,
//                              const simd::float3& offset);
//    
//    // UI rendering helpers
//    struct UIVertex {
//        simd::float2 position;
//        simd::float2 texCoord;
//        simd::float4 color;
//    };
//    
//    void addQuad(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices,
//                const simd::float2& pos, const simd::float2& size,
//                const simd::float2& uvMin, const simd::float2& uvMax,
//                const simd::float4& color);
//};
//
//// Item properties database
//struct ItemProperties {
//    ItemType type;
//    std::string name;
//    simd::float4 color;
//    uint16_t maxStack;
//    bool isBlock;
//    float mass;  // For vehicle physics
//    
//    static ItemProperties getProperties(ItemType type);
//    static const char* getItemName(ItemType type);
//    static simd::float4 getItemColor(ItemType type);
//};

#endif /* RMDLInventory_hpp */
