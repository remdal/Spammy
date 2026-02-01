#include <metal_stdlib>
using namespace metal;

// ============================================================================
// Structures GPU - Panel
// ============================================================================

struct FabPanelUniforms {
    float2 screenSize;
    float2 panelPos;
    float2 panelSize;
    float4 bgColor;
    float4 borderColor;
    float4 headerColor;
    float4 shadowColor;
    float cornerRadius;
    float borderWidth;
    float headerHeight;
    float time;
    float shadowBlur;
    float shadowOffsetY;
    int isInventoryOpen;
    float inventoryWidth;
};

struct FabPanelVertexIn {
    float2 position [[attribute(0)]];
    float2 uv       [[attribute(1)]];
};

struct FabPanelVertexOut {
    float4 position [[position]];
    float2 uv;
    float2 localPos;
    float2 panelSize;
};

// ============================================================================
// Structures GPU - Grid 3D
// ============================================================================

struct FabGridUniforms {
    float4x4 viewProjection;
    float4x4 model;
    float3 cameraPos;
    float time;
    float3 selectedSlot;
    float slotSpacing;
    float4 emptySlotColor;
    float4 selectedColor;
    float4 hoveredColor;
    float gridSize;
    float padding[3];
};

struct FabSlotInstance {
    float3 position;
    float scale;
    float4 color;
    uint flags;
    uint slotX;
    uint slotY;
    uint slotZ;
};

struct FabGridVertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
};

struct FabGridVertexOut {
    float4 position [[position]];
    float3 worldPos;
    float3 worldNormal;
    float2 uv;
    float4 color;
    uint flags;
    float3 slotCoord;
};

// ============================================================================
// Structures GPU - Axes
// ============================================================================

struct FabAxisUniforms {
    float4x4 viewProjection;
    float time;
    float axisLength;
    float axisThickness;
    float padding;
};

struct FabAxisVertexIn {
    float3 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
};

struct FabAxisVertexOut {
    float4 position [[position]];
    float4 color;
    float3 worldPos;
};

// ============================================================================
// Helper Functions
// ============================================================================

float roundedBoxSDF(float2 p, float2 halfSize, float radius) {
    float2 q = abs(p) - halfSize + radius;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

float shadowSDF(float2 p, float2 halfSize, float radius, float blur) {
    float d = roundedBoxSDF(p, halfSize, radius);
    return smoothstep(blur, 0.0, d);
}

// ============================================================================
// Panel Shaders
// ============================================================================

vertex FabPanelVertexOut fabPanelVertexShader(
    FabPanelVertexIn in [[stage_in]],
    constant FabPanelUniforms& u [[buffer(1)]]
) {
    // Calculer la position écran
    float2 screenPos = in.position * u.panelSize * 0.5 + u.panelPos;
    
    // Projection ortho
    float4 clipPos;
    clipPos.x = (screenPos.x / u.screenSize.x) * 2.0 - 1.0;
    clipPos.y = 1.0 - (screenPos.y / u.screenSize.y) * 2.0;
    clipPos.z = 0.0;
    clipPos.w = 1.0;
    
    FabPanelVertexOut out;
    out.position = clipPos;
    out.uv = in.uv;
    out.localPos = in.position * u.panelSize * 0.5;
    out.panelSize = u.panelSize;
    return out;
}

fragment float4 fabPanelFragmentShader(
    FabPanelVertexOut in [[stage_in]],
    constant FabPanelUniforms& u [[buffer(0)]]
) {
    float2 halfSize = u.panelSize * 0.5;
    float2 p = in.localPos;
    
    // SDF du panel principal
    float d = roundedBoxSDF(p, halfSize, u.cornerRadius);
    
    // Ombre portée
    float2 shadowP = p - float2(0.0, u.shadowOffsetY);
    float shadow = shadowSDF(shadowP, halfSize + 5.0, u.cornerRadius + 3.0, u.shadowBlur);
    
    // Si en dehors du panel ET de l'ombre
    if (d > u.shadowBlur && shadow < 0.01) {
        discard_fragment();
    }
    
    // Dessiner l'ombre d'abord
    if (d > 0.0) {
        return float4(u.shadowColor.rgb, u.shadowColor.a * shadow * 0.6);
    }
    
    // Couleur de base
    float4 color = u.bgColor;
    
    // Zone header
    float headerY = -halfSize.y + u.headerHeight;
    if (p.y < headerY) {
        color = u.headerColor;
        
        // Dégradé subtil dans le header
        float headerGrad = (p.y + halfSize.y) / u.headerHeight;
        color.rgb *= 0.9 + headerGrad * 0.15;
    }
    
    // Zone inventaire (si ouverte)
    if (u.isInventoryOpen != 0) {
        float invRight = -halfSize.x + u.inventoryWidth;
        if (p.x < invRight && p.y >= headerY) {
            // Fond inventaire plus sombre
            color.rgb *= 0.85;
            
            // Séparateur vertical
            if (p.x > invRight - 2.0 && p.x < invRight) {
                color = mix(color, u.borderColor, 0.7);
            }
        }
    }
    
    // Bordure
    if (d > -u.borderWidth) {
        float t = smoothstep(-u.borderWidth, -u.borderWidth + 1.5, d);
        color = mix(color, u.borderColor, 1.0 - t);
        
        // Glow subtil sur la bordure
        float glow = sin(u.time * 2.0) * 0.1 + 0.9;
        color.rgb *= glow;
    }
    
    // Ligne sous le header
    if (abs(p.y - headerY) < 1.5) {
        color = mix(color, u.borderColor, 0.8);
    }
    
    // Gradient vertical subtil
    float vGrad = (p.y / halfSize.y + 1.0) * 0.5;
    color.rgb *= 0.92 + vGrad * 0.12;
    
    // Bouton toggle inventaire (coin haut droit)
    float2 btnPos = float2(halfSize.x - 20.0, -halfSize.y + u.headerHeight * 0.5);
    float btnDist = length(p - btnPos);
    if (btnDist < 12.0) {
        float4 btnColor = u.isInventoryOpen != 0 ?
            float4(0.4, 0.7, 0.5, 1.0) : float4(0.7, 0.4, 0.4, 1.0);
        float btnT = smoothstep(12.0, 8.0, btnDist);
        color = mix(color, btnColor, btnT * 0.8);
        
        // Icône (flèches)
        float2 iconP = p - btnPos;
        if (abs(iconP.x) < 5.0 && abs(iconP.y) < 3.0) {
            color.rgb = float3(1.0);
        }
    }
    
    // Titre "FABRICATION" dans le header
    // (Simplifié - juste une zone plus claire)
    if (p.y < headerY && p.x > -60.0 && p.x < 60.0 && p.y > -halfSize.y + 8.0) {
        color.rgb += 0.05;
    }
    
    // Vignette subtile
    float2 vignette = p / halfSize;
    float vignetteAmount = 1.0 - dot(vignette, vignette) * 0.06;
    color.rgb *= vignetteAmount;
    
    return color;
}

// ============================================================================
// Grid 3D Shaders
// ============================================================================

vertex FabGridVertexOut fabGridVertexShader(
    FabGridVertexIn in [[stage_in]],
    constant FabGridUniforms& u [[buffer(1)]],
    constant FabSlotInstance* instances [[buffer(2)]],
    uint instanceId [[instance_id]]
) {
    FabSlotInstance inst = instances[instanceId];
    
    // Position locale transformée
    float3 localPos = in.position * inst.scale + inst.position;
    
    // Animations
    bool isEmpty = (inst.flags & 1u) != 0;
    bool isSelected = (inst.flags & 2u) != 0;
    bool isHovered = (inst.flags & 4u) != 0;
    
    if (!isEmpty) {
        // Breathing subtil pour les items
        float breathe = sin(u.time * 1.5 + float(instanceId) * 0.15) * 0.008;
        localPos += in.normal * breathe;
        
        // Léger flottement
        float bob = sin(u.time * 0.8 + float(instanceId) * 0.3) * 0.015;
        localPos.y += bob;
    }
    
    if (isSelected) {
        // Pulse de sélection
        float pulse = sin(u.time * 5.0) * 0.02 + 0.015;
        localPos += in.normal * pulse;
    }
    
    if (isHovered && !isSelected) {
        // Légère expansion au hover
        localPos += in.normal * 0.01;
    }
    
    float4 worldPos = u.model * float4(localPos, 1.0);
    
    FabGridVertexOut out;
    out.position = u.viewProjection * worldPos;
    out.worldPos = worldPos.xyz;
    out.worldNormal = normalize((u.model * float4(in.normal, 0.0)).xyz);
    out.uv = in.uv;
    out.color = inst.color;
    out.flags = inst.flags;
    out.slotCoord = float3(inst.slotX, inst.slotY, inst.slotZ);
    
    return out;
}

fragment float4 fabGridFragmentShader(
    FabGridVertexOut in [[stage_in]],
    constant FabGridUniforms& u [[buffer(0)]]
) {
    bool isEmpty = (in.flags & 1u) != 0;
    bool isSelected = (in.flags & 2u) != 0;
    bool isHovered = (in.flags & 4u) != 0;
    
    // Lighting multi-sources
    float3 lightDir1 = normalize(float3(0.4, 0.9, 0.3));
    float3 lightDir2 = normalize(float3(-0.3, 0.4, -0.5));
    float3 lightDir3 = normalize(float3(0.0, -0.5, 0.8));
    
    float NdotL1 = max(dot(in.worldNormal, lightDir1), 0.0);
    float NdotL2 = max(dot(in.worldNormal, lightDir2), 0.0);
    float NdotL3 = max(dot(in.worldNormal, lightDir3), 0.0);
    
    float ambient = 0.32;
    float diffuse = NdotL1 * 0.45 + NdotL2 * 0.15 + NdotL3 * 0.08;
    
    float4 baseColor = in.color;
    
    // Slots vides: wireframe élégant
    if (isEmpty) {
        float2 uvEdge = abs(in.uv - 0.5) * 2.0;
        float edge = max(uvEdge.x, uvEdge.y);
        
        // Ne garder que les bords
        if (edge < 0.78) {
            discard_fragment();
        }
        
        // Couleur de bordure avec animation
        float edgePulse = sin(u.time * 0.5 + in.slotCoord.x + in.slotCoord.y + in.slotCoord.z) * 0.08;
        baseColor.a = 0.18 + edgePulse;
        baseColor.rgb = float3(0.4, 0.45, 0.55);
        
        // Plus lumineux aux intersections
        if (edge > 0.92) {
            baseColor.rgb *= 1.3;
        }
    }
    
    // Highlight sélection
    if (isSelected) {
        float glow = sin(u.time * 6.0) * 0.2 + 0.8;
        float4 selectColor = u.selectedColor;
        baseColor = mix(baseColor, selectColor, 0.55 * glow);
        
        // Bordure brillante
        float2 uvEdge = abs(in.uv - 0.5) * 2.0;
        float edge = max(uvEdge.x, uvEdge.y);
        if (edge > 0.85) {
            baseColor.rgb += selectColor.rgb * 0.4 * glow;
        }
        
        // Inner glow
        baseColor.rgb += selectColor.rgb * 0.15 * glow;
    }
    
    // Highlight hover
    if (isHovered && !isSelected) {
        float4 hoverColor = u.hoveredColor;
        baseColor = mix(baseColor, hoverColor, 0.35);
    }
    
    // Calcul éclairage final
    float3 finalColor = baseColor.rgb * (ambient + diffuse);
    
    // Bords des cubes (relief)
    if (!isEmpty) {
        float2 uvEdge = abs(in.uv - 0.5) * 2.0;
        float edge = max(uvEdge.x, uvEdge.y);
        
        // Bordure sombre
        if (edge > 0.88) {
            finalColor *= 0.55;
        }
        // Légère brillance sur les arêtes
        else if (edge > 0.82) {
            finalColor *= 0.75;
        }
    }
    
    // Specular simple
    if (!isEmpty) {
        float3 viewDir = normalize(u.cameraPos - in.worldPos);
        float3 halfDir = normalize(lightDir1 + viewDir);
        float spec = pow(max(dot(in.worldNormal, halfDir), 0.0), 32.0);
        finalColor += spec * 0.2;
    }
    
    // Fresnel/rim lighting
    float3 viewDir = normalize(u.cameraPos - in.worldPos);
    float rim = 1.0 - max(dot(in.worldNormal, viewDir), 0.0);
    rim = pow(rim, 3.0) * 0.2;
    finalColor += rim * baseColor.rgb;
    
    // Depth-based fog subtil
    float depth = length(in.worldPos) / 15.0;
    float fog = smoothstep(0.3, 1.2, depth) * 0.15;
    finalColor = mix(finalColor, float3(0.08, 0.09, 0.12), fog);
    
    return float4(finalColor, baseColor.a);
}

// ============================================================================
// Axes Shaders
// ============================================================================

vertex FabAxisVertexOut fabAxisVertexShader(
    FabAxisVertexIn in [[stage_in]],
    constant FabAxisUniforms& u [[buffer(1)]]
) {
    FabAxisVertexOut out;
    out.position = u.viewProjection * float4(in.position, 1.0);
    out.color = in.color;
    out.worldPos = in.position;
    return out;
}

fragment float4 fabAxisFragmentShader(
    FabAxisVertexOut in [[stage_in]],
    constant FabAxisUniforms& u [[buffer(0)]]
) {
    float4 color = in.color;
    
    // Glow pulsé subtil pour les axes
    float pulse = sin(u.time * 2.5) * 0.08 + 0.92;
    color.rgb *= pulse;
    
    // Atténuation avec la distance du centre
    float dist = length(in.worldPos);
    float falloff = 1.0 - smoothstep(0.0, u.axisLength * 1.2, dist) * 0.2;
    color.rgb *= falloff;
    
    // Légère brillance
    color.rgb += 0.05;
    
    return color;
}
