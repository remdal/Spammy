//
//  RMDLFab3D.hpp
//  Spammy
//
//  Created by Rémy on 30/01/2026.
//

#ifndef RMDLFab3D_hpp
#define RMDLFab3D_hpp

#include <Metal/Metal.hpp>

#include <simd/simd.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <functional>
#include <optional>

namespace Fabieur {

constexpr uint32_t FAB_GRID_SIZE = 5;
constexpr uint32_t FAB_GRID_TOTAL = FAB_GRID_SIZE * FAB_GRID_SIZE * FAB_GRID_SIZE;

using ItemID = uint32_t;
constexpr ItemID ITEM_EMPTY = 0;

struct FabSlot
{
    ItemID itemId = ITEM_EMPTY;
    uint32_t quantity = 0;
    
    bool isEmpty() const { return itemId == ITEM_EMPTY || quantity == 0; }
    void clear() { itemId = ITEM_EMPTY; quantity = 0; }
    
    bool operator==(const FabSlot& other) const
    {
        return itemId == other.itemId;
    }
    
    bool operator< (const FabSlot& other) const
    {
        return itemId < other.itemId;
    }
};

struct FabPattern3D
{
    std::array<FabSlot, FAB_GRID_TOTAL> slots;
    simd::uint3 dimensions;
    simd::uint3 offset;
    
    FabPattern3D() : dimensions{0, 0, 0}, offset{0, 0, 0}
    {
        for (auto& slot : slots) slot.clear();
    }
    
    // Accès 3D -> 1D
    static size_t index(uint32_t x, uint32_t y, uint32_t z)
    {
        return z * FAB_GRID_SIZE * FAB_GRID_SIZE + y * FAB_GRID_SIZE + x;
    }
    
    FabSlot& at(uint32_t x, uint32_t y, uint32_t z)
    {
        return slots[index(x, y, z)];
    }
    
    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const
    {
        return slots[index(x, y, z)];
    }
    
    void computeBounds()
    {
        simd::uint3 minBound = { FAB_GRID_SIZE, FAB_GRID_SIZE, FAB_GRID_SIZE };
        simd::uint3 maxBound = { 0, 0, 0 };
        
        for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
        {
            for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
            {
                for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
                {
                    if (!at(x, y, z).isEmpty())
                    {
                        minBound.x = std::min(minBound.x, x);
                        minBound.y = std::min(minBound.y, y);
                        minBound.z = std::min(minBound.z, z);
                        maxBound.x = std::max(maxBound.x, x + 1);
                        maxBound.y = std::max(maxBound.y, y + 1);
                        maxBound.z = std::max(maxBound.z, z + 1);
                    }
                }
            }
        }
        
        if (maxBound.x > minBound.x)
        {
            dimensions = maxBound - minBound;
            offset = minBound;
        }
        else
        {
            dimensions = { 0, 0, 0 };
            offset = { 0, 0, 0 };
        }
    }
};

struct FabRecipe
{
    std::string id;
    std::string name;
    FabPattern3D pattern;
    
    ItemID resultItemId;
    uint32_t resultQuantity;
    
    bool shapeless = false;
    bool mirrorable = true;
    bool rotatable = true;
    
    std::function<bool()> condition = nullptr;
};

class FabGrid3D
{
public:
    std::array<FabSlot, FAB_GRID_TOTAL> slots;
    
    FabGrid3D()
    {
        clear();
    }
    
    void clear()
    {
        for (auto& slot : slots) slot.clear();
    }
    
    static size_t index(uint32_t x, uint32_t y, uint32_t z)
    {
        return z * FAB_GRID_SIZE * FAB_GRID_SIZE + y * FAB_GRID_SIZE + x;
    }
    
    FabSlot& at(uint32_t x, uint32_t y, uint32_t z)
    {
        return slots[index(x, y, z)];
    }
    
    const FabSlot& at(uint32_t x, uint32_t y, uint32_t z) const
    {
        return slots[index(x, y, z)];
    }
    
    bool placeItem(uint32_t x, uint32_t y, uint32_t z, ItemID itemId, uint32_t qty = 1)
    {
        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) return false;
        
        auto& slot = at(x, y, z);
        
        if (slot.isEmpty())
        {
            slot.itemId = itemId;
            slot.quantity = qty;
            return true;
        }
        else if (slot.itemId == itemId)
        {
            slot.quantity += qty;
            return true;
        }
        return false;
    }
    
    FabSlot removeItem(uint32_t x, uint32_t y, uint32_t z, uint32_t qty = 1)
    {
        if (x >= FAB_GRID_SIZE || y >= FAB_GRID_SIZE || z >= FAB_GRID_SIZE) {
            return FabSlot{};
        }
        
        auto& slot = at(x, y, z);
        FabSlot removed;
        
        if (!slot.isEmpty()) {
            removed.itemId = slot.itemId;
            removed.quantity = std::min(qty, slot.quantity);
            slot.quantity -= removed.quantity;
            if (slot.quantity == 0) slot.itemId = ITEM_EMPTY;
        }
        
        return removed;
    }
    
    FabPattern3D getCurrentPattern() const
    {
        FabPattern3D pattern;
        pattern.slots = slots;
        pattern.computeBounds();
        return pattern;
    }
    
    uint32_t countItems() const
    {
        uint32_t count = 0;
        for (const auto& slot : slots)
        {
            if (!slot.isEmpty()) ++count;
        }
        return count;
    }
};

class FabSystem3D
{
public:
    FabSystem3D(MTL::Device* device);
    ~FabSystem3D();
    
    void registerRecipe(const FabRecipe& recipe);
    void unregisterRecipe(const std::string& recipeId);
    const FabRecipe* findRecipe(const std::string& recipeId) const;
    
    std::optional<FabRecipe> checkCraft(const FabGrid3D& grid) const;
    bool executeCraft(FabGrid3D& grid, ItemID& resultItem, uint32_t& resultQty);
    
    std::vector<const FabRecipe*> getMatchingRecipes(ItemID itemId) const;
    std::vector<const FabRecipe*> getAllRecipes() const;
    
    void render(MTL::RenderCommandEncoder* encoder, const FabGrid3D& grid, const simd::float4x4& viewProj, const simd::float3& gridPosition);
    
    void updateGPUData(const FabGrid3D& grid);
    
private:
    MTL::Device* _device;
    MTL::RenderPipelineState* _pipelineState = nullptr;
    MTL::Buffer* _vertexBuffer = nullptr;
    MTL::Buffer* _indexBuffer = nullptr;
    MTL::Buffer* _instanceBuffer = nullptr;
    MTL::Buffer* _uniformBuffer = nullptr;
    
    std::vector<FabRecipe> _recipes;
    
    bool matchPattern(const FabPattern3D& gridPattern, const FabPattern3D& recipePattern, bool shapeless, bool mirrorable, bool rotatable) const;
    bool matchPatternExact(const FabPattern3D& a, const FabPattern3D& b, simd::int3 offset) const;
    bool matchPatternShapeless(const FabPattern3D& a, const FabPattern3D& b) const;
    
    FabPattern3D rotatePatternY(const FabPattern3D& pattern, int times) const;
    FabPattern3D mirrorPatternX(const FabPattern3D& pattern) const;
    FabPattern3D mirrorPatternZ(const FabPattern3D& pattern) const;
    
    void initMetal();
    void createBuffers();
    void createPipeline();
};

struct FabSlotInstance
{
    simd::float3 position;
    float scale;
    simd::float4 color;
    uint32_t itemId;
    uint32_t flags;
    simd::float2 padding;
};

struct FabUniforms
{
    simd::float4x4 viewProjection;
    simd::float3 gridPosition;
    float time;
    simd::float3 cameraPosition;
    float slotSize;
    simd::float4 highlightColor;
    simd::float4 emptySlotColor;
};

inline FabSystem3D::FabSystem3D(MTL::Device* device) : _device(device)
{
    initMetal();
}

inline FabSystem3D::~FabSystem3D()
{
    if (_pipelineState) _pipelineState->release();
    if (_vertexBuffer) _vertexBuffer->release();
    if (_indexBuffer) _indexBuffer->release();
    if (_instanceBuffer) _instanceBuffer->release();
    if (_uniformBuffer) _uniformBuffer->release();
}

inline void FabSystem3D::initMetal()
{
    createBuffers();
    createPipeline();
}

inline void FabSystem3D::createBuffers()
{
    float s = 0.5f;
    float cubeVertices[] = {
        // Front
        -s, -s,  s,  0, 0, 1,  0, 0,
         s, -s,  s,  0, 0, 1,  1, 0,
         s,  s,  s,  0, 0, 1,  1, 1,
        -s,  s,  s,  0, 0, 1,  0, 1,
        // Back
         s, -s, -s,  0, 0,-1,  0, 0,
        -s, -s, -s,  0, 0,-1,  1, 0,
        -s,  s, -s,  0, 0,-1,  1, 1,
         s,  s, -s,  0, 0,-1,  0, 1,
        // Top
        -s,  s,  s,  0, 1, 0,  0, 0,
         s,  s,  s,  0, 1, 0,  1, 0,
         s,  s, -s,  0, 1, 0,  1, 1,
        -s,  s, -s,  0, 1, 0,  0, 1,
        // Bottom
        -s, -s, -s,  0,-1, 0,  0, 0,
         s, -s, -s,  0,-1, 0,  1, 0,
         s, -s,  s,  0,-1, 0,  1, 1,
        -s, -s,  s,  0,-1, 0,  0, 1,
        // Right
         s, -s,  s,  1, 0, 0,  0, 0,
         s, -s, -s,  1, 0, 0,  1, 0,
         s,  s, -s,  1, 0, 0,  1, 1,
         s,  s,  s,  1, 0, 0,  0, 1,
        // Left
        -s, -s, -s, -1, 0, 0,  0, 0,
        -s, -s,  s, -1, 0, 0,  1, 0,
        -s,  s,  s, -1, 0, 0,  1, 1,
        -s,  s, -s, -1, 0, 0,  0, 1,
    };
    
    uint16_t cubeIndices[] = {
        0,1,2, 0,2,3,       // Front
        4,5,6, 4,6,7,       // Back
        8,9,10, 8,10,11,    // Top
        12,13,14, 12,14,15, // Bottom
        16,17,18, 16,18,19, // Right
        20,21,22, 20,22,23  // Left
    };
    
    _vertexBuffer = _device->newBuffer(cubeVertices, sizeof(cubeVertices), MTL::ResourceStorageModeShared);
    _indexBuffer = _device->newBuffer(cubeIndices, sizeof(cubeIndices), MTL::ResourceStorageModeShared);
    _instanceBuffer = _device->newBuffer(sizeof(FabSlotInstance) * FAB_GRID_TOTAL, MTL::ResourceStorageModeShared);
    _uniformBuffer = _device->newBuffer(sizeof(FabUniforms), MTL::ResourceStorageModeShared);
}

inline void FabSystem3D::createPipeline()
{
    const char* shaderSrc = R"(
        #include <metal_stdlib>
        using namespace metal;
        
        struct VertexIn {
            float3 position [[attribute(0)]];
            float3 normal   [[attribute(1)]];
            float2 uv       [[attribute(2)]];
        };
        
        struct SlotInstance {
            float3 position;
            float scale;
            float4 color;
            uint itemId;
            uint flags;
            float2 padding;
        };
        
        struct Uniforms {
            float4x4 viewProjection;
            float3 gridPosition;
            float time;
            float3 cameraPosition;
            float slotSize;
            float4 highlightColor;
            float4 emptySlotColor;
        };
        
        struct VertexOut {
            float4 position [[position]];
            float3 worldPos;
            float3 normal;
            float2 uv;
            float4 color;
            uint itemId;
            uint flags;
        };
        
        vertex VertexOut fab_vertex(
            VertexIn in [[stage_in]],
            constant Uniforms& uniforms [[buffer(1)]],
            constant SlotInstance* instances [[buffer(2)]],
            uint instanceId [[instance_id]]
        ) {
            SlotInstance inst = instances[instanceId];
            
            float3 worldPos = in.position * inst.scale + inst.position + uniforms.gridPosition;
            
            // Animation légère pour les slots non-vides
            if (inst.itemId != 0) {
                float pulse = sin(uniforms.time * 2.0 + float(instanceId) * 0.5) * 0.02;
                worldPos += in.normal * pulse;
            }
            
            VertexOut out;
            out.position = uniforms.viewProjection * float4(worldPos, 1.0);
            out.worldPos = worldPos;
            out.normal = in.normal;
            out.uv = in.uv;
            out.color = inst.color;
            out.itemId = inst.itemId;
            out.flags = inst.flags;
            return out;
        }
        
        fragment float4 fab_fragment(
            VertexOut in [[stage_in]],
            constant Uniforms& uniforms [[buffer(1)]]
        ) {
            float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
            float NdotL = max(dot(in.normal, lightDir), 0.0);
            float ambient = 0.3;
            float diffuse = NdotL * 0.7;
            
            float4 baseColor = in.color;
            
            // Slot vide = semi-transparent
            if (in.itemId == 0) {
                baseColor = uniforms.emptySlotColor;
                baseColor.a = 0.15 + sin(uniforms.time + in.worldPos.x + in.worldPos.y + in.worldPos.z) * 0.05;
            }
            
            // Highlight si sélectionné
            if ((in.flags & 1u) != 0) {
                baseColor = mix(baseColor, uniforms.highlightColor, 0.5);
            }
            
            float3 finalColor = baseColor.rgb * (ambient + diffuse);
            
            // Bord des cubes
            float2 uvEdge = abs(in.uv - 0.5) * 2.0;
            float edge = max(uvEdge.x, uvEdge.y);
            if (edge > 0.9) {
                finalColor *= 0.7;
            }
            
            return float4(finalColor, baseColor.a);
        }
    )";
    
    NS::Error* error = nullptr;
    MTL::Library* library = _device->newLibrary(NS::String::string(shaderSrc, NS::UTF8StringEncoding), nullptr, &error);
    
    if (!library) {
        printf("FabSystem3D: Failed to create shader library\n");
        return;
    }
    
    MTL::Function* vertexFn = library->newFunction(NS::String::string("fab_vertex", NS::UTF8StringEncoding));
    MTL::Function* fragmentFn = library->newFunction(NS::String::string("fab_fragment", NS::UTF8StringEncoding));
    
    MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();
    
    // Position
    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(0)->setOffset(0);
    vertexDesc->attributes()->object(0)->setBufferIndex(0);
    
    // Normal
    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(1)->setOffset(sizeof(float) * 3);
    vertexDesc->attributes()->object(1)->setBufferIndex(0);
    
    // UV
    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(2)->setOffset(sizeof(float) * 6);
    vertexDesc->attributes()->object(2)->setBufferIndex(0);
    
    vertexDesc->layouts()->object(0)->setStride(sizeof(float) * 8);
    
    MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setVertexFunction(vertexFn);
    pipelineDesc->setFragmentFunction(fragmentFn);
    pipelineDesc->setVertexDescriptor(vertexDesc);
    pipelineDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
    pipelineDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
    pipelineDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    pipelineDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    pipelineDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
    
    _pipelineState = _device->newRenderPipelineState(pipelineDesc, &error);
    
    vertexDesc->release();
    pipelineDesc->release();
    vertexFn->release();
    fragmentFn->release();
    library->release();
}

inline void FabSystem3D::registerRecipe(const FabRecipe& recipe)
{
    // Éviter les doublons
    for (const auto& r : _recipes) {
        if (r.id == recipe.id) return;
    }
    
    FabRecipe r = recipe;
    r.pattern.computeBounds();
    _recipes.push_back(r);
}

inline void FabSystem3D::unregisterRecipe(const std::string& recipeId)
{
    _recipes.erase(
        std::remove_if(_recipes.begin(), _recipes.end(),
            [&](const FabRecipe& r) { return r.id == recipeId; }),
        _recipes.end()
    );
}

inline const FabRecipe* FabSystem3D::findRecipe(const std::string& recipeId) const
{
    for (const auto& r : _recipes) {
        if (r.id == recipeId) return &r;
    }
    return nullptr;
}

inline std::optional<FabRecipe> FabSystem3D::checkCraft(const FabGrid3D& grid) const
{
    FabPattern3D gridPattern = grid.getCurrentPattern();
    
    if (gridPattern.dimensions.x == 0) return std::nullopt; // Grille vide
    
    for (const auto& recipe : _recipes) {
        // Vérifier condition
        if (recipe.condition && !recipe.condition()) continue;
        
        if (matchPattern(gridPattern, recipe.pattern, recipe.shapeless,
                        recipe.mirrorable, recipe.rotatable)) {
            return recipe;
        }
    }
    
    return std::nullopt;
}

inline bool FabSystem3D::executeCraft(FabGrid3D& grid, ItemID& resultItem, uint32_t& resultQty) {
    auto recipe = checkCraft(grid);
    if (!recipe) return false;
    
    // Consommer les ingrédients (1 de chaque slot utilisé)
    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
                auto& slot = grid.at(x, y, z);
                if (!slot.isEmpty()) {
                    slot.quantity--;
                    if (slot.quantity == 0) slot.itemId = ITEM_EMPTY;
                }
            }
        }
    }
    
    resultItem = recipe->resultItemId;
    resultQty = recipe->resultQuantity;
    return true;
}

inline bool FabSystem3D::matchPattern(const FabPattern3D& gridPattern,
                                           const FabPattern3D& recipePattern,
                                           bool shapeless, bool mirrorable, bool rotatable) const {
    if (shapeless) {
        return matchPatternShapeless(gridPattern, recipePattern);
    }
    
    // Test toutes les positions possibles dans la grille 5x5x5
    std::vector<FabPattern3D> variants;
    variants.push_back(recipePattern);
    
    if (rotatable) {
        variants.push_back(rotatePatternY(recipePattern, 1));
        variants.push_back(rotatePatternY(recipePattern, 2));
        variants.push_back(rotatePatternY(recipePattern, 3));
    }
    
    if (mirrorable) {
        size_t count = variants.size();
        for (size_t i = 0; i < count; ++i) {
            variants.push_back(mirrorPatternX(variants[i]));
            variants.push_back(mirrorPatternZ(variants[i]));
        }
    }
    
    for (const auto& variant : variants) {
        // Essayer tous les offsets possibles
        int maxOffsetX = FAB_GRID_SIZE - variant.dimensions.x;
        int maxOffsetY = FAB_GRID_SIZE - variant.dimensions.y;
        int maxOffsetZ = FAB_GRID_SIZE - variant.dimensions.z;
        
        for (int oz = 0; oz <= maxOffsetZ; ++oz) {
            for (int oy = 0; oy <= maxOffsetY; ++oy) {
                for (int ox = 0; ox <= maxOffsetX; ++ox) {
                    simd::int3 offset = {ox - (int)gridPattern.offset.x + (int)variant.offset.x,
                                         oy - (int)gridPattern.offset.y + (int)variant.offset.y,
                                         oz - (int)gridPattern.offset.z + (int)variant.offset.z};
                    if (matchPatternExact(gridPattern, variant, offset)) {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

inline bool FabSystem3D::matchPatternExact(const FabPattern3D& grid, const FabPattern3D& recipe, simd::int3 offset) const
{
    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z) {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y) {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x) {
                int rx = (int)x - offset.x;
                int ry = (int)y - offset.y;
                int rz = (int)z - offset.z;
                
                bool inRecipeBounds = rx >= 0 && rx < FAB_GRID_SIZE &&
                                      ry >= 0 && ry < FAB_GRID_SIZE &&
                                      rz >= 0 && rz < FAB_GRID_SIZE;
                
                const FabSlot& gridSlot = grid.at(x, y, z);
                
                if (inRecipeBounds) {
                    const FabSlot& recipeSlot = recipe.at(rx, ry, rz);
                    if (gridSlot.itemId != recipeSlot.itemId) return false;
                } else {
                    if (!gridSlot.isEmpty()) return false;
                }
            }
        }
    }
    return true;
}

inline bool FabSystem3D::matchPatternShapeless(const FabPattern3D& grid, const FabPattern3D& recipe) const
{
    std::unordered_map<ItemID, uint32_t> gridItems, recipeItems;
    
    for (const auto& slot : grid.slots)
    {
        if (!slot.isEmpty()) gridItems[slot.itemId]++;
    }
    
    for (const auto& slot : recipe.slots)
    {
        if (!slot.isEmpty()) recipeItems[slot.itemId]++;
    }
    
    return gridItems == recipeItems;
}

inline FabPattern3D FabSystem3D::rotatePatternY(const FabPattern3D& pattern, int times) const
{
    FabPattern3D result;
    times = times % 4;
    if (times == 0) return pattern;
    
    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
    {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
        {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
            {
                uint32_t nx = x, nz = z;
                for (int t = 0; t < times; ++t) {
                    uint32_t tmp = nx;
                    nx = FAB_GRID_SIZE - 1 - nz;
                    nz = tmp;
                }
                result.at(nx, y, nz) = pattern.at(x, y, z);
            }
        }
    }
    
    result.computeBounds();
    return result;
}

inline FabPattern3D FabSystem3D::mirrorPatternX(const FabPattern3D& pattern) const
{
    FabPattern3D result;
    
    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
    {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
        {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
            {
                result.at(FAB_GRID_SIZE - 1 - x, y, z) = pattern.at(x, y, z);
            }
        }
    }
    
    result.computeBounds();
    return result;
}

inline FabPattern3D FabSystem3D::mirrorPatternZ(const FabPattern3D& pattern) const
{
    FabPattern3D result;
    
    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
    {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
        {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
            {
                result.at(x, y, FAB_GRID_SIZE - 1 - z) = pattern.at(x, y, z);
            }
        }
    }
    
    result.computeBounds();
    return result;
}

inline std::vector<const FabRecipe*> FabSystem3D::getMatchingRecipes(ItemID itemId) const
{
    std::vector<const FabRecipe*> matches;
    
    for (const auto& recipe : _recipes)
    {
        for (const auto& slot : recipe.pattern.slots)
        {
            if (slot.itemId == itemId) {
                matches.push_back(&recipe);
                break;
            }
        }
    }
    
    return matches;
}

inline std::vector<const FabRecipe*> FabSystem3D::getAllRecipes() const
{
    std::vector<const FabRecipe*> all;
    all.reserve(_recipes.size());
    for (const auto& r : _recipes) all.push_back(&r);
    return all;
}

inline void FabSystem3D::updateGPUData(const FabGrid3D& grid)
{
    auto* instances = static_cast<FabSlotInstance*>(_instanceBuffer->contents());
    
    float spacing = 1.1f; // Espacement entre les slots
    float slotScale = 0.45f;
    
    // Couleurs par défaut pour les items (à personnaliser selon ton système)
    auto getItemColor = [](ItemID id) -> simd::float4
    {
        if (id == ITEM_EMPTY) return {0.5f, 0.5f, 0.5f, 0.1f};
        // Hash simple pour générer des couleurs
        float r = fmodf(id * 0.618034f, 1.0f);
        float g = fmodf(id * 0.381966f, 1.0f);
        float b = fmodf(id * 0.723607f, 1.0f);
        return {r, g, b, 1.0f};
    };
    
    float offset = (FAB_GRID_SIZE - 1) * spacing * 0.5f;
    
    for (uint32_t z = 0; z < FAB_GRID_SIZE; ++z)
    {
        for (uint32_t y = 0; y < FAB_GRID_SIZE; ++y)
        {
            for (uint32_t x = 0; x < FAB_GRID_SIZE; ++x)
            {
                size_t idx = FabGrid3D::index(x, y, z);
                const FabSlot& slot = grid.slots[idx];
                
                instances[idx].position = {
                    x * spacing - offset,
                    y * spacing - offset,
                    z * spacing - offset
                };
                instances[idx].scale = slot.isEmpty() ? slotScale * 0.3f : slotScale;
                instances[idx].color = getItemColor(slot.itemId);
                instances[idx].itemId = slot.itemId;
                instances[idx].flags = 0;
            }
        }
    }
}

inline void FabSystem3D::render(MTL::RenderCommandEncoder* encoder, const FabGrid3D& grid, const simd::float4x4& viewProj, const simd::float3& gridPosition)
{
    if (!_pipelineState) return;
    
    updateGPUData(grid);
    
    auto* uniforms = static_cast<FabUniforms*>(_uniformBuffer->contents());
    uniforms->viewProjection = viewProj;
    uniforms->gridPosition = gridPosition;
    static float time = 0;
    time += 0.016f;
    uniforms->time = time;
    uniforms->slotSize = 1.0f;
    uniforms->highlightColor = {1.0f, 0.9f, 0.3f, 1.0f};
    uniforms->emptySlotColor = {0.4f, 0.4f, 0.5f, 0.15f};
    
    encoder->setRenderPipelineState(_pipelineState);
    encoder->setVertexBuffer(_vertexBuffer, 0, 0);
    encoder->setVertexBuffer(_uniformBuffer, 0, 1);
    encoder->setVertexBuffer(_instanceBuffer, 0, 2);
    encoder->setFragmentBuffer(_uniformBuffer, 0, 1);
    
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 36, MTL::IndexTypeUInt16, _indexBuffer, 0, FAB_GRID_TOTAL);
}

}

#endif /* RMDLFab3D_hpp */
