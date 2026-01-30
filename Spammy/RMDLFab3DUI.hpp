//
//  RMDLFab3DUI.hpp
//  Spammy
//
//  Created by Rémy on 30/01/2026.
//

#ifndef RMDLFab3DUI_hpp
#define RMDLFab3DUI_hpp

#include "RMDLFab3D.hpp"

using namespace Fabieur;

// IDs d'items exemple
enum ItemIDs : ItemID {
    ITEM_NONE = 0,
    ITEM_IRON_INGOT = 1,
    ITEM_GOLD_INGOT = 2,
    ITEM_DIAMOND = 3,
    ITEM_WOOD = 4,
    ITEM_STONE = 5,
    ITEM_REDSTONE = 6,
    ITEM_IRON_BLOCK = 100,
    ITEM_GOLD_BLOCK = 101,
    ITEM_DIAMOND_BLOCK = 102,
    ITEM_FUSION_CORE = 200,
    ITEM_POWER_CUBE = 201,
};

inline void setupExampleRecipes(FabSystem3D& craftSystem)
{
    {
        FabRecipe recipe;
        recipe.id = "iron_block";
        recipe.name = "Bloc de Fer";
        recipe.resultItemId = ITEM_IRON_BLOCK;
        recipe.resultQuantity = 1;
        recipe.shapeless = false;
        recipe.mirrorable = true;
        recipe.rotatable = true;
        
        // Pattern 3x3 sur le plan XY (z=0)
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                recipe.pattern.at(x, y, 0).itemId = ITEM_IRON_INGOT;
                recipe.pattern.at(x, y, 0).quantity = 1;
            }
        }
        recipe.pattern.computeBounds();
        
        craftSystem.registerRecipe(recipe);
    }
    
    // ========================================
    // Recette 2: Fusion Core (cube 3x3x3 avec diamant au centre)
    // ========================================
    {
        FabRecipe recipe;
        recipe.id = "fusion_core";
        recipe.name = "Fusion Core";
        recipe.resultItemId = ITEM_FUSION_CORE;
        recipe.resultQuantity = 1;
        recipe.shapeless = false;
        
        // Coque externe en or
        for (int z = 0; z < 3; ++z) {
            for (int y = 0; y < 3; ++y) {
                for (int x = 0; x < 3; ++x) {
                    // Faces externes seulement
                    if (x == 0 || x == 2 || y == 0 || y == 2 || z == 0 || z == 2) {
                        recipe.pattern.at(x, y, z).itemId = ITEM_GOLD_INGOT;
                        recipe.pattern.at(x, y, z).quantity = 1;
                    }
                }
            }
        }
        
        // Diamant au centre
        recipe.pattern.at(1, 1, 1).itemId = ITEM_DIAMOND;
        recipe.pattern.at(1, 1, 1).quantity = 1;
        
        recipe.pattern.computeBounds();
        craftSystem.registerRecipe(recipe);
    }
    
    // ========================================
    // Recette 3: Power Cube (5x5x5 complet!)
    // ========================================
    {
        FabRecipe recipe;
        recipe.id = "power_cube";
        recipe.name = "Power Cube Ultime";
        recipe.resultItemId = ITEM_POWER_CUBE;
        recipe.resultQuantity = 1;
        recipe.shapeless = false;
        recipe.mirrorable = false;
        recipe.rotatable = false;
        
        // Structure en couches
        for (int z = 0; z < 5; ++z) {
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 5; ++x) {
                    // Coins = diamants
                    bool isCorner = (x == 0 || x == 4) && (y == 0 || y == 4) && (z == 0 || z == 4);
                    // Arêtes = or
                    bool isEdge = ((x == 0 || x == 4) && (y == 0 || y == 4)) ||
                                  ((x == 0 || x == 4) && (z == 0 || z == 4)) ||
                                  ((y == 0 || y == 4) && (z == 0 || z == 4));
                    // Centre de chaque face = redstone
                    bool isFaceCenter = (x == 2 && (y == 0 || y == 4) && z == 2) ||
                                        (y == 2 && (x == 0 || x == 4) && z == 2) ||
                                        (z == 2 && x == 2 && (y == 0 || y == 4));
                    // Coeur = fusion core
                    bool isCore = (x == 2 && y == 2 && z == 2);
                    
                    if (isCore) {
                        recipe.pattern.at(x, y, z).itemId = ITEM_FUSION_CORE;
                    } else if (isCorner) {
                        recipe.pattern.at(x, y, z).itemId = ITEM_DIAMOND;
                    } else if (isEdge) {
                        recipe.pattern.at(x, y, z).itemId = ITEM_GOLD_INGOT;
                    } else if (isFaceCenter) {
                        recipe.pattern.at(x, y, z).itemId = ITEM_REDSTONE;
                    } else {
                        recipe.pattern.at(x, y, z).itemId = ITEM_IRON_INGOT;
                    }
                    recipe.pattern.at(x, y, z).quantity = 1;
                }
            }
        }
        
        recipe.pattern.computeBounds();
        craftSystem.registerRecipe(recipe);
    }
    
    // ========================================
    // Recette 4: Shapeless (n'importe quel arrangement)
    // ========================================
    {
        FabRecipe recipe;
        recipe.id = "mixed_alloy";
        recipe.name = "Alliage Mixte";
        recipe.resultItemId = 300;
        recipe.resultQuantity = 4;
        recipe.shapeless = true; // L'arrangement n'importe pas!
        
        // 2 fer + 2 or + 1 diamant = alliage
        recipe.pattern.at(0, 0, 0).itemId = ITEM_IRON_INGOT;
        recipe.pattern.at(1, 0, 0).itemId = ITEM_IRON_INGOT;
        recipe.pattern.at(2, 0, 0).itemId = ITEM_GOLD_INGOT;
        recipe.pattern.at(3, 0, 0).itemId = ITEM_GOLD_INGOT;
        recipe.pattern.at(4, 0, 0).itemId = ITEM_DIAMOND;
        
        for (int i = 0; i < 5; ++i) {
            recipe.pattern.at(i, 0, 0).quantity = 1;
        }
        
        recipe.pattern.computeBounds();
        craftSystem.registerRecipe(recipe);
    }
}

// ============================================================================
// Exemple d'intégration dans ta game loop
// ============================================================================

class FabUI {
public:
    FabSystem3D craftSystem;
    FabGrid3D currentGrid;
    
    simd::uint3 selectedSlot = {2, 2, 2}; // Centre par défaut
    bool isOpen = false;
    
    FabUI(MTL::Device* device) : craftSystem(device) {
        setupExampleRecipes(craftSystem);
    }
    
    void open() { isOpen = true; }
    void close() { isOpen = false; currentGrid.clear(); }
    
    // Appelé quand le joueur place un item depuis son inventaire
    void placeItemFromInventory(ItemID itemId, uint32_t qty = 1) {
        currentGrid.placeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z, itemId, qty);
        checkRecipe();
    }
    
    // Retirer un item vers l'inventaire
    FabSlot removeToInventory() {
        auto slot = currentGrid.removeItem(selectedSlot.x, selectedSlot.y, selectedSlot.z);
        checkRecipe();
        return slot;
    }
    
    // Navigation 3D dans la grille
    void moveSelection(int dx, int dy, int dz) {
        int nx = (int)selectedSlot.x + dx;
        int ny = (int)selectedSlot.y + dy;
        int nz = (int)selectedSlot.z + dz;
        
        selectedSlot.x = std::clamp(nx, 0, (int)FAB_GRID_SIZE - 1);
        selectedSlot.y = std::clamp(ny, 0, (int)FAB_GRID_SIZE - 1);
        selectedSlot.z = std::clamp(nz, 0, (int)FAB_GRID_SIZE - 1);
    }
    
    // Vérifier si une recette match
    std::optional<FabRecipe> currentRecipeMatch;
    
    void checkRecipe()
    {
        currentRecipeMatch = craftSystem.checkCraft(currentGrid);
        if (currentRecipeMatch) {
            printf("Recette trouvée: %s (produit %u x item #%u)\n",
                   currentRecipeMatch->name.c_str(),
                   currentRecipeMatch->resultQuantity,
                   currentRecipeMatch->resultItemId);
        }
    }
    
    // Exécuter le craft
    bool doCraft(ItemID& resultItem, uint32_t& resultQty) {
        if (craftSystem.executeCraft(currentGrid, resultItem, resultQty)) {
            currentRecipeMatch = std::nullopt;
            checkRecipe(); // Re-vérifier au cas où il reste des items
            return true;
        }
        return false;
    }
    
    // Rendu
    void render(MTL::RenderCommandEncoder* encoder,
                const simd::float4x4& viewProj,
                const simd::float3& uiPosition) {
        if (!isOpen) return;
        craftSystem.render(encoder, currentGrid, viewProj, uiPosition);
    }
};

/*
if (keyPressed('C')) {
    if (craftUI.isOpen) craftUI.close();
    else craftUI.open();
}

if (craftUI.isOpen) {
    // Navigation WASD + Q/E pour Z
    if (keyPressed('W')) craftUI.moveSelection(0, 1, 0);
    if (keyPressed('S')) craftUI.moveSelection(0, -1, 0);
    if (keyPressed('A')) craftUI.moveSelection(-1, 0, 0);
    if (keyPressed('D')) craftUI.moveSelection(1, 0, 0);
    if (keyPressed('Q')) craftUI.moveSelection(0, 0, -1);
    if (keyPressed('E')) craftUI.moveSelection(0, 0, 1);
    
    // Placer/retirer
    if (leftClick) {
        craftUI.placeItemFromInventory(currentHeldItem);
    }
    if (rightClick) {
        auto removed = craftUI.removeToInventory();
        addToInventory(removed);
    }
    
    // Craft!
    if (keyPressed(ENTER) && craftUI.currentRecipeMatch) {
        ItemID result;
        uint32_t qty;
        if (craftUI.doCraft(result, qty)) {
            addToInventory({result, qty});
        }
    }
}
*/

#endif /* RMDLFab3DUI_hpp */
