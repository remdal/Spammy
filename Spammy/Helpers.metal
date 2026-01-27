//
//  Helpers.metal
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include <metal_stdlib>
using namespace metal;

// FRAGMENT SHADERS - PBR RENDERING

// PBR
inline float distributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI_F * denom * denom;
    
    return nom / denom;
    return a2 / max(denom, 0.0001);
}

inline float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / max(denom, 0.0001);
    return num / denom;
    return NdotV / (NdotV * (1.0 - k) + k);
}

inline float geometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

inline float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

inline float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// NOISE

struct NoiseParams {
    float2 worldOffset;
    float scale;
    uint biomeId;
    uint64_t seed;
};

// HASH FUNCTIONS

inline uint hashPerlin(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

inline uint hash2(uint2 v) {
    return hashPerlin(v.x ^ hashPerlin(v.y));
}

inline uint hash3(uint3 v) {
    return hashPerlin(v.x ^ hashPerlin(v.y ^ hashPerlin(v.z)));
}

inline float hashToFloat(uint h) {
    return float(h) / float(0xFFFFFFFF);
}

// GRADIENT NOISE (Perlin-style)

inline float3 gradient3D(uint hash)
{
    uint h = hash & 15;
    float u = h < 8 ? 1.0 : -1.0;
    float v = h < 4 ? 1.0 : (h == 12 || h == 14 ? 1.0 : -1.0);
    float w = h < 2 ? 1.0 : (h == 12 || h == 13 ? -1.0 : 1.0);
    return normalize(float3(u, v, w));
}

inline float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

inline float perlinNoise3D(float3 p, constant uint* perm) {
    float3 pi = floor(p);
    float3 pf = fract(p);
    
    int3 i = int3(pi) & 255;
    
    float3 u = float3(fade(pf.x), fade(pf.y), fade(pf.z));
    
    uint aaa = perm[perm[perm[i.x] + i.y] + i.z];
    uint baa = perm[perm[perm[i.x + 1] + i.y] + i.z];
    uint aba = perm[perm[perm[i.x] + i.y + 1] + i.z];
    uint bba = perm[perm[perm[i.x + 1] + i.y + 1] + i.z];
    uint aab = perm[perm[perm[i.x] + i.y] + i.z + 1];
    uint bab = perm[perm[perm[i.x + 1] + i.y] + i.z + 1];
    uint abb = perm[perm[perm[i.x] + i.y + 1] + i.z + 1];
    uint bbb = perm[perm[perm[i.x + 1] + i.y + 1] + i.z + 1];
    
    float3 g000 = gradient3D(aaa);
    float3 g100 = gradient3D(baa);
    float3 g010 = gradient3D(aba);
    float3 g110 = gradient3D(bba);
    float3 g001 = gradient3D(aab);
    float3 g101 = gradient3D(bab);
    float3 g011 = gradient3D(abb);
    float3 g111 = gradient3D(bbb);
    
    float n000 = dot(g000, pf - float3(0, 0, 0));
    float n100 = dot(g100, pf - float3(1, 0, 0));
    float n010 = dot(g010, pf - float3(0, 1, 0));
    float n110 = dot(g110, pf - float3(1, 1, 0));
    float n001 = dot(g001, pf - float3(0, 0, 1));
    float n101 = dot(g101, pf - float3(1, 0, 1));
    float n011 = dot(g011, pf - float3(0, 1, 1));
    float n111 = dot(g111, pf - float3(1, 1, 1));
    
    float nx00 = mix(n000, n100, u.x);
    float nx10 = mix(n010, n110, u.x);
    float nx01 = mix(n001, n101, u.x);
    float nx11 = mix(n011, n111, u.x);
    
    float nxy0 = mix(nx00, nx10, u.y);
    float nxy1 = mix(nx01, nx11, u.y);
    
    return mix(nxy0, nxy1, u.z);
}

// ============================================
// SIMPLEX NOISE 2D
// ============================================

inline float2 simplexGrad2D(uint hash) {
    float angle = float(hash) / float(0xFFFFFFFF) * 2.0 * M_PI_F;
    return float2(cos(angle), sin(angle));
}

inline float simplexNoise2D(float2 p, uint seed) {
    const float F2 = 0.5 * (sqrt(3.0) - 1.0);
    const float G2 = (3.0 - sqrt(3.0)) / 6.0;
    
    float s = (p.x + p.y) * F2;
    float2 pi = floor(p + s);
    
    float t = (pi.x + pi.y) * G2;
    float2 p0 = p - (pi - t);
    
    int2 i1 = p0.x > p0.y ? int2(1, 0) : int2(0, 1);
    
    float2 p1 = p0 - float2(i1) + G2;
    float2 p2 = p0 - 1.0 + 2.0 * G2;
    
    uint h0 = hash2(uint2(pi) + seed);
    uint h1 = hash2(uint2(pi + float2(i1)) + seed);
    uint h2 = hash2(uint2(pi + 1) + seed);
    
    float2 g0 = simplexGrad2D(h0);
    float2 g1 = simplexGrad2D(h1);
    float2 g2 = simplexGrad2D(h2);
    
    float n0 = 0.0, n1 = 0.0, n2 = 0.0;
    
    float t0 = 0.5 - dot(p0, p0);
    if (t0 > 0.0) {
        t0 *= t0;
        n0 = t0 * t0 * dot(g0, p0);
    }
    
    float t1 = 0.5 - dot(p1, p1);
    if (t1 > 0.0) {
        t1 *= t1;
        n1 = t1 * t1 * dot(g1, p1);
    }
    
    float t2 = 0.5 - dot(p2, p2);
    if (t2 > 0.0) {
        t2 *= t2;
        n2 = t2 * t2 * dot(g2, p2);
    }
    
    return 70.0 * (n0 + n1 + n2);
}

// ============================================
// VORONOI NOISE
// ============================================

struct VoronoiResult {
    float distance1;
    float distance2;
    float2 cellPoint;
    uint cellId;
};

inline VoronoiResult voronoi2D(float2 p, uint seed) {
    float2 pi = floor(p);
    float2 pf = fract(p);
    
    VoronoiResult result;
    result.distance1 = 100.0;
    result.distance2 = 100.0;
    
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            float2 neighbor = float2(x, y);
            uint h = hash2(uint2(pi + neighbor) + seed);
            float2 point = neighbor + float2(hashToFloat(h), hashToFloat(hashPerlin(h)));
            
            float dist = length(pf - point);
            
            if (dist < result.distance1) {
                result.distance2 = result.distance1;
                result.distance1 = dist;
                result.cellPoint = pi + point;
                result.cellId = h;
            } else if (dist < result.distance2) {
                result.distance2 = dist;
            }
        }
    }
    
    return result;
}

// ============================================
// FBM (Fractal Brownian Motion)
// ============================================

inline float fbm2D(float2 p, int octaves, float persistence, float lacunarity, uint seed) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        value += amplitude * simplexNoise2D(p * frequency, seed + i * 1337);
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

// ============================================
// DOMAIN WARPING
// ============================================

inline float warpedNoise(float2 p, uint seed) {
    float2 q = float2(
        fbm2D(p, 4, 0.5, 2.0, seed),
        fbm2D(p + float2(5.2, 1.3), 4, 0.5, 2.0, seed)
    );
    
    float2 r = float2(
        fbm2D(p + 4.0 * q + float2(1.7, 9.2), 4, 0.5, 2.0, seed),
        fbm2D(p + 4.0 * q + float2(8.3, 2.8), 4, 0.5, 2.0, seed)
    );
    
    return fbm2D(p + 4.0 * r, 4, 0.5, 2.0, seed);
}


// PACK

// Geometry buffer fragments
struct GBufferFragOut
{
    float4 gBuffer0 [[color(0)]];
    float4 gBuffer1 [[color(1)]];
#ifdef __METAL_IOS__
    float  gBufferDepth [[color(3)]];
#endif
};

// The bi-directional reflectance distribution properties
//  - Defines how light is reflected on an opaque surface
struct BrdfProperties
{
    float3 albedo;
    float3 normal;
    float specIntensity;
    float specPower;
    float ao;
    float shadow;
};

inline float2 OctahedronWrap(float2 n)
{
    float2 signMask = {n.x >= 0? 1.f : -1.f, n.y >= 0? 1.f : -1.f};
    return (1.f - abs(n.yx)) * signMask;
}

inline float2 EncodeNormal(float3 n)
{
    float3 ret = n / (abs(n.x) + abs(n.y) + abs(n.z));
    ret.xy = ret.z >= 0? ret.xy : OctahedronWrap(ret.xy);
    ret.xy = ret.xy * 0.5 + 0.5;
    return ret.xy;
}

inline float3 DecodeNormal(float2 enc)
{
    enc = enc * 2 - 1;

    float3 ret;
    ret.z = 1.f - abs(enc.x) - abs(enc.y);
    ret.xy = ret.z >= 0? enc.xy : OctahedronWrap(enc.xy);
    return normalize(ret);
}

inline float PackFloat2(float x, float y)
{
    uint val = uint(ceil(saturate(x) * 15.f)) | (uint(ceil(saturate(y) * 15.f)) << 4);
    return val / 255.f;
}

inline float2 UnpackFloat2(float val)
{
    uint uval = val * 255;
    return float2((uval & 15) / 15.f, ((uval >> 4) & 15) / 15.f);
}

// GBuffer0: r g b specPower/specIntensity
// GBuffer1: nx ny shadow ao
inline GBufferFragOut PackBrdfProperties(thread BrdfProperties &brdfProperties)
{
    GBufferFragOut frag_out;
    frag_out.gBuffer0 = float4(sqrt(brdfProperties.albedo), PackFloat2(brdfProperties.specPower / 32, brdfProperties.specIntensity));
    frag_out.gBuffer1 = float4(EncodeNormal(brdfProperties.normal), brdfProperties.shadow, brdfProperties.ao);
    return frag_out;
}

inline BrdfProperties UnpackBrdfProperties(float4 gBuffer0, float4 gBuffer1)
{
    BrdfProperties ret;
    ret.albedo = gBuffer0.xyz * gBuffer0.xyz;

    float2 powerIntensity = UnpackFloat2(gBuffer0.w);
    ret.specPower = powerIntensity.x * 32;
    ret.specIntensity = powerIntensity.y;

    ret.normal = DecodeNormal(gBuffer1.xy);
    ret.shadow = gBuffer1.z;
    ret.ao = gBuffer1.w;

    return ret;
}

inline float3 UnpackNormal(float2 gBuffer1)
{
    return DecodeNormal(gBuffer1.xy);
}

inline uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
