#include <metal_stdlib>
using namespace metal;

// ============================================================================
// STRUCTURES GPU - Panel 2D
// ============================================================================

struct FabPanelUniforms {
    float2 screenSize;
    float2 panelPosition;
    float2 panelSize;
    float4 backgroundColor;
    float4 borderColor;
    float4 headerColor;
    float cornerRadius;
    float borderWidth;
    float headerHeight;
    float time;
    int showInventory;
    float inventoryWidth;
    float padding[2];
};

struct FabPanelVertexIn {
    float2 position [[attribute(0)]];
    float2 uv       [[attribute(1)]];
};

struct FabPanelVertexOut {
    float4 position [[position]];
    float2 localPos;
    float2 uv;
};

// ============================================================================
// STRUCTURES GPU - 3D
// ============================================================================

struct Fab3DUniforms {
    float4x4 viewProjectionMatrix;
    float3 cameraPosition;
    float time;
    float3 lightDirection;
    float gridHalfSize;
    float4 selectionColor;
};

struct FabSlotInstance {
    float3 worldPosition;
    float scale;
    float4 color;
    uint flags;
    uint _pad[3];
};

struct FabCubeVertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
};

struct FabCubeVertexOut {
    float4 position [[position]];
    float3 worldPos;
    float3 normal;
    float2 uv;
    float4 color;
    uint flags;
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
// HELPER FUNCTIONS
// ============================================================================

// SDF pour rectangle arrondi
float fabRoundedBoxSDF(float2 p, float2 halfSize, float radius) {
    float2 q = abs(p) - halfSize + radius;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

// ============================================================================
// PANEL 2D SHADERS
// ============================================================================

vertex FabPanelVertexOut fabPanelVertexShader(
    FabPanelVertexIn in [[stage_in]],
    constant FabPanelUniforms& u [[buffer(1)]]
) {
    FabPanelVertexOut out;
    
    // Position locale dans le panel
    float2 localPos = in.position * u.panelSize * 0.5;
    
    // Position écran
    float2 screenPos = localPos + u.panelPosition;
    
    // Convertir en clip space
    out.position.x = (screenPos.x / u.screenSize.x) * 2.0 - 1.0;
    out.position.y = 1.0 - (screenPos.y / u.screenSize.y) * 2.0;
    out.position.z = 0.0;
    out.position.w = 1.0;
    
    out.localPos = localPos;
    out.uv = in.uv;
    
    return out;
}

fragment float4 fabPanelFragmentShader(
    FabPanelVertexOut in [[stage_in]],
    constant FabPanelUniforms& u [[buffer(0)]]
) {
    float2 halfSize = u.panelSize * 0.5;
    float2 p = in.localPos;
    
    // Distance au bord du panel
    float d = fabRoundedBoxSDF(p, halfSize, u.cornerRadius);
    
    // En dehors du panel
    if (d > 0.0) {
        // Ombre douce
        float shadow = exp(-d * 0.08);
        return float4(0.0, 0.0, 0.0, shadow * 0.4);
    }
    
    // Couleur de base
    float4 color = u.backgroundColor;
    
    // Zone header (haut du panel)
    float headerY = -halfSize.y + u.headerHeight;
    if (p.y < headerY) {
        color = u.headerColor;
        
        // Gradient subtil
        float grad = (p.y + halfSize.y) / u.headerHeight;
        color.rgb *= 0.85 + grad * 0.2;
    }
    
    // Zone inventaire (à gauche)
    if (u.showInventory != 0) {
        float invRight = -halfSize.x + u.inventoryWidth;
        if (p.x < invRight && p.y >= headerY) {
            // Fond plus sombre pour l'inventaire
            color.rgb *= 0.8;
            
            // Ligne de séparation verticale
            if (p.x > invRight - 2.0 && p.x < invRight) {
                color = mix(color, u.borderColor, 0.6);
            }
        }
    }
    
    // Bordure
    if (d > -u.borderWidth) {
        float t = smoothstep(-u.borderWidth, -u.borderWidth + 1.5, d);
        color = mix(color, u.borderColor, 1.0 - t);
    }
    
    // Ligne sous le header
    if (abs(p.y - headerY) < 1.5 && p.y >= headerY) {
        color = mix(color, u.borderColor, 0.7);
    }
    
    // Effet de vignette subtil
    float2 vignette = p / halfSize;
    float vignetteAmount = 1.0 - dot(vignette, vignette) * 0.04;
    color.rgb *= vignetteAmount;
    
    // Gradient vertical subtil
    float vertGrad = (p.y / halfSize.y + 1.0) * 0.5;
    color.rgb *= 0.92 + vertGrad * 0.1;
    
    return color;
}

// ============================================================================
// CUBE 3D SHADERS (pour les slots de la grille)
// ============================================================================

vertex FabCubeVertexOut fabCubeVertexShader(
    FabCubeVertexIn in [[stage_in]],
    constant Fab3DUniforms& u [[buffer(1)]],
    constant FabSlotInstance* instances [[buffer(2)]],
    uint instanceId [[instance_id]]
) {
    FabSlotInstance inst = instances[instanceId];
    
    // Position world
    float3 worldPos = in.position * inst.scale + inst.worldPosition;
    
    // Animation pour les slots non-vides
    bool isEmpty = (inst.flags & 1u) != 0;
    bool isSelected = (inst.flags & 2u) != 0;
    
    if (!isEmpty) {
        // Respiration subtile
        float breathe = sin(u.time * 1.2 + float(instanceId) * 0.1) * 0.01;
        worldPos += in.normal * breathe;
        
        // Léger flottement
        float bob = sin(u.time * 0.7 + float(instanceId) * 0.2) * 0.02;
        worldPos.y += bob;
    }
    
    if (isSelected) {
        // Pulse de sélection
        float pulse = sin(u.time * 5.0) * 0.025 + 0.02;
        worldPos += in.normal * pulse;
    }
    
    FabCubeVertexOut out;
    out.position = u.viewProjectionMatrix * float4(worldPos, 1.0);
    out.worldPos = worldPos;
    out.normal = in.normal;
    out.uv = in.uv;
    out.color = inst.color;
    out.flags = inst.flags;
    
    return out;
}

fragment float4 fabCubeFragmentShader(
    FabCubeVertexOut in [[stage_in]],
    constant Fab3DUniforms& u [[buffer(0)]]
) {
    bool isEmpty = (in.flags & 1u) != 0;
    bool isSelected = (in.flags & 2u) != 0;
    
    float4 baseColor = in.color;
    
    // Slots vides: wireframe élégant
    if (isEmpty) {
        float2 uvEdge = abs(in.uv - 0.5) * 2.0;
        float edge = max(uvEdge.x, uvEdge.y);
        
        // Garder seulement les bords
        if (edge < 0.75) {
            discard_fragment();
        }
        
        // Animation subtile
        float edgePulse = sin(u.time * 0.4) * 0.1 + 0.25;
        baseColor.a = edgePulse;
        baseColor.rgb = float3(0.4, 0.45, 0.55);
        
        // Points plus lumineux aux coins
        if (edge > 0.9) {
            baseColor.rgb *= 1.4;
            baseColor.a += 0.15;
        }
        
        return baseColor;
    }
    
    // Éclairage multi-sources
    float3 N = normalize(in.normal);
    float3 L1 = normalize(u.lightDirection);
    float3 L2 = normalize(float3(-0.3, 0.5, -0.6));
    
    float NdotL1 = max(dot(N, L1), 0.0);
    float NdotL2 = max(dot(N, L2), 0.0);
    
    float ambient = 0.35;
    float diffuse = NdotL1 * 0.5 + NdotL2 * 0.15;
    
    float3 finalColor = baseColor.rgb * (ambient + diffuse);
    
    // Specular
    float3 viewDir = normalize(u.cameraPosition - in.worldPos);
    float3 halfDir = normalize(L1 + viewDir);
    float spec = pow(max(dot(N, halfDir), 0.0), 32.0);
    finalColor += spec * 0.25;
    
    // Fresnel / rim light
    float rim = 1.0 - max(dot(N, viewDir), 0.0);
    rim = pow(rim, 3.0) * 0.2;
    finalColor += rim * baseColor.rgb;
    
    // Bordures des cubes
    float2 uvEdge = abs(in.uv - 0.5) * 2.0;
    float edge = max(uvEdge.x, uvEdge.y);
    if (edge > 0.88) {
        finalColor *= 0.55;
    } else if (edge > 0.80) {
        finalColor *= 0.75;
    }
    
    // Highlight de sélection
    if (isSelected) {
        float glow = sin(u.time * 5.0) * 0.2 + 0.8;
        finalColor = mix(finalColor, u.selectionColor.rgb, 0.5 * glow);
        
        // Bordure brillante
        if (edge > 0.82) {
            finalColor += u.selectionColor.rgb * 0.4 * glow;
        }
    }
    
    // Depth fog subtil
    float depth = length(in.worldPos) / 15.0;
    float fog = smoothstep(0.3, 1.2, depth) * 0.12;
    finalColor = mix(finalColor, float3(0.06, 0.07, 0.1), fog);
    
    return float4(finalColor, baseColor.a);
}

// ============================================================================
// AXIS SHADERS (pour les axes XYZ et les lignes de grille)
// ============================================================================

vertex FabAxisVertexOut fabAxisVertexShader(
    FabAxisVertexIn in [[stage_in]],
    constant Fab3DUniforms& u [[buffer(1)]]
) {
    FabAxisVertexOut out;
    out.position = u.viewProjectionMatrix * float4(in.position, 1.0);
    out.color = in.color;
    out.worldPos = in.position;
    return out;
}

fragment float4 fabAxisFragmentShader(
    FabAxisVertexOut in [[stage_in]],
    constant Fab3DUniforms& u [[buffer(0)]]
) {
    float4 color = in.color;
    
    // Pulsation subtile
    float pulse = sin(u.time * 2.0) * 0.08 + 0.92;
    color.rgb *= pulse;
    
    // Atténuation avec la distance depuis le centre
    float dist = length(in.worldPos);
    float fade = 1.0 - smoothstep(0.0, u.gridHalfSize * 1.5, dist) * 0.15;
    color.rgb *= fade;
    
    // Légère brillance
    color.rgb += 0.03;
    
    return color;
}
