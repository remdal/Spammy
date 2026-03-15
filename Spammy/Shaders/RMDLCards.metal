//
//  RMDLCards.metal
//  Spammy
//
//  Created by Rémy on 14/03/2026.
//

#include <metal_stdlib>
using namespace metal;

struct HoloCardVertex
{
    float3 position  [[attribute(0)]];
    float2 uv        [[attribute(1)]];
    float3 normal    [[attribute(2)]];
    float3 tangent   [[attribute(3)]];
};

struct HoloCardVaryings
{
    float4 clipPos [[position]];
    float2 uv;
    float3 worldNormal;
    float3 worldTangent;
    float3 worldBitangent;
    float3 worldPos;
    float2 screenUV;
};

struct HoloCardUniforms
{
    float4x4 modelMatrix;
    float4x4 viewProjMatrix;
    float3   cameraPos;
    float    time;
    float2   tilt;
    float    refractionStrength;
    float    chromaticAberration;
    float    fresnelPower;
    float    glassOpacity;
    float    holoIntensity;
    float    holoScale;
    float    holoShift;
    float    sparkleIntensity;
    float3   lightDir;
    float    glareIntensity;
    float    glareShininess;
    float2   screenSize;
    float2   _pad;
};

float hash21(float2 p)
{
    p = fract(p * float2(234.34f, 435.345f));
    p += dot(p, p + 34.23f);
    return fract(p.x * p.y);
}

float smoothNoise(float2 uv)
{
    float2 i = floor(uv);
    float2 f = fract(uv);
    float2 u = f * f * (3.0f - 2.0f * f);
    float a = hash21(i);
    float b = hash21(i + float2(1, 0));
    float c = hash21(i + float2(0, 1));
    float d = hash21(i + float2(1, 1));
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(float2 uv)
{
    float  v = 0.0f, a = 0.5f;
    float2 shift = float2(100.0f);
    float2x2 rot = float2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.5f));
    for (int i = 0; i < 4; i++) {
        v += a * smoothNoise(uv);
        uv = rot * uv * 2.0f + shift;
        a *= 0.5f;
    }
    return v;
}

float3 fbmNormal(float2 uv, float t)
{
    const float eps = 0.01f;
    float2 aUV = uv + float2(t * 0.05f, t * 0.03f);
    float h  = fbm(aUV);
    float hx = fbm(aUV + float2(eps, 0.0f));
    float hy = fbm(aUV + float2(0.0f, eps));
    return normalize(float3(h - hx, h - hy, 0.08f));
}

float3 hsvToRgb(float h, float s, float v)
{
    float4 K = float4(1.0f, 2.0f/3.0f, 1.0f/3.0f, 3.0f);
    float3 p = abs(fract(float3(h) + K.xyz) * 6.0f - K.www);
    return v * mix(K.xxx, clamp(p - K.xxx, 0.0f, 1.0f), s);
}

float fresnelSchlick(float3 v, float3 n, float power)
{
    return pow(1.0f - saturate(dot(v, n)), power);
}

vertex HoloCardVaryings holoCard_vertex(HoloCardVertex in [[stage_in]],
                                        constant HoloCardUniforms& uniforms [[buffer(1)]])
{
    HoloCardVaryings out;

    float4 wp4        = uniforms.modelMatrix * float4(in.position, 1.0f);
    out.worldPos      = wp4.xyz;
    out.clipPos       = uniforms.viewProjMatrix * wp4;
    out.uv            = in.uv;

    // Normal matrix (3x3 sous-matrice de modelMatrix)
    // Valide pour des transforms uniformes ; utilise inverse-transpose si scale non-uniforme
    float3x3 nm = float3x3(uniforms.modelMatrix[0].xyz, uniforms.modelMatrix[1].xyz, uniforms.modelMatrix[2].xyz);
    out.worldNormal    = normalize(nm * in.normal);
    out.worldTangent   = normalize(nm * in.tangent);
    out.worldBitangent = cross(out.worldNormal, out.worldTangent);

    // Screen UV précalculé — évite la division dans le fragment
    float2 ndc     = out.clipPos.xy / out.clipPos.w;
    out.screenUV   = ndc * 0.5f + 0.5f;
    out.screenUV.y = 1.0f - out.screenUV.y;  // Metal NDC → texture space
    return out;
}

fragment float4 holoCard_fragment(HoloCardVaryings in [[stage_in]],
                                  constant HoloCardUniforms& uniforms [[buffer(1)]],

    // texture(0) : artwork de la carte  (chargé via ton loadTexture habituel)
    // texture(1) : scène déjà rendue    (RT de ta passe principale — pas de blit)
    depth2d<float>           artworkTex [[texture(0)]],
    texture2d<float>           sceneTex   [[texture(1)]],

    sampler                    linearSamp [[sampler(0)]],
    sampler                    clampSamp  [[sampler(1)]])
{
    const float2 uv = in.uv;
    const float  t  = uniforms.time;

    // ── 1. Normal map procédurale animée (TBN) ───────────────────
    float3 pA = fbmNormal(uv * uniforms.holoScale,              t);
    float3 pB = fbmNormal(uv * uniforms.holoScale * 0.5f + 10.0f, t * 0.7f);
    float3 pN = normalize(pA + pB * 0.5f);

    float3 N = normalize(
        pN.x * in.worldTangent   +
        pN.y * in.worldBitangent +
        pN.z * in.worldNormal
    );
    // Tilt (gyro/souris) perturbe la normale → effet parallaxe
    N.xy += uniforms.tilt * 0.3f;
    N = normalize(N);

    // ── 2. Réfraction + dispersion chromatique ───────────────────
    // sceneTex = ta render target de la passe scène, déjà terminée.
    // On la lit directement, sans copie. Les deux textures sont différentes
    // donc pas de hazard lecture/écriture.
    float2 refOff = N.xy * uniforms.refractionStrength;
    float  ca     = uniforms.chromaticAberration;

    float scR = sceneTex.sample(clampSamp, in.screenUV + refOff + N.xy * ca).r;
    float scG = sceneTex.sample(clampSamp, in.screenUV + refOff            ).g;
    float scB = sceneTex.sample(clampSamp, in.screenUV + refOff - N.xy * ca).b;
    float4 refracted = float4(scR, scG, scB, 1.0f);

    // ── 3. Artwork de la carte ────────────────────────────────────
    float4 artwork   = artworkTex.sample(linearSamp, uv);
    float4 baseColor = mix(artwork, refracted, uniforms.glassOpacity);

    // ── 4. Holographique — iridescence HSV ───────────────────────
    float3 viewDir = normalize(uniforms.cameraPos - in.worldPos);
    float  vDot    = dot(viewDir, N);

    float bands  = sin((uv.x - uv.y) * uniforms.holoScale * 3.14159f
                       + vDot * 4.0f + t * uniforms.holoShift)          * 0.5f + 0.5f;
    float bands2 = sin((uv.x + uv.y) * uniforms.holoScale * 2.0f
                       + vDot * 6.0f + t * uniforms.holoShift * 0.6f)   * 0.5f + 0.5f;

    float  hueShift  = fract(bands * 0.5f + bands2 * 0.3f + vDot * 0.4f + t * 0.05f);
    float3 holoRGB   = hsvToRgb(hueShift, 0.8f, 1.0f);
    float  holoMask  = pow(bands * bands2, 0.5f) * uniforms.holoIntensity;

    // ── 5. Sparkles (paillettes) ─────────────────────────────────
    float  sRnd    = hash21(floor(uv * 40.0f));
    float  sPhase  = fract(sRnd * 17.3f + t * (0.5f + sRnd * 0.5f));
    float  sparkle = smoothstep(0.95f, 1.0f, sRnd)
                   * smoothstep(0.6f,  1.0f, sPhase)
                   * smoothstep(1.0f,  0.6f, sPhase)
                   * (0.5f + saturate(dot(N.xy, uniforms.tilt + 0.5f)))
                   * uniforms.sparkleIntensity;

    // ── 6. Fresnel + glare anisotrope ────────────────────────────
    float  F       = fresnelSchlick(viewDir, N, uniforms.fresnelPower);
    float3 halfVec = normalize(normalize(uniforms.lightDir) + viewDir);
    float  aOff    = abs(dot(halfVec, in.worldTangent));
    float  aniso   = pow(sqrt(1.0f - aOff * aOff), uniforms.glareShininess * 2.0f);
    float  glare   = mix(pow(max(dot(N, halfVec), 0.0f), uniforms.glareShininess), aniso, 0.4f)
                   * uniforms.glareIntensity;

    // ── 7. Composition additive ───────────────────────────────────
    float4 result  = baseColor;


    // Depth texture → luminance → alpha de la carte
    float depthAlpha = sceneTex.sample(clampSamp, in.screenUV).r;

    // Optionnel : contraste pour rendre l'effet plus tranché
    depthAlpha = smoothstep(0.0f, 0.6f, depthAlpha);

    result.rgb = result.rgb / (result.rgb + 1.0f);
    result.rgb = pow(result.rgb, float3(1.0f / 2.2f));
    result.a   = artwork.a * depthAlpha;   // ← ici
    return result;
    
    result.rgb    += holoRGB * holoMask;
    result.rgb    += sparkle;
    result.rgb    += float3(0.85f, 0.92f, 1.0f) * F * 0.4f;
    result.rgb    += float3(1.0f,  0.97f, 0.9f) * glare;

    // Tone mapping Reinhard + gamma
    // Si ton pipeline est déjà HDR linéaire (RGBA16Float), retire le gamma
    result.rgb  = result.rgb / (result.rgb + 1.0f);
    result.rgb  = pow(result.rgb, float3(1.0f / 2.2f));
    result.a    = artwork.a;   // alpha artwork pour coins arrondis
    return result;
}
