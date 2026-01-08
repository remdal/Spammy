//
//  RMDLWorld.cpp
//  Spammy
//
//  Created by Rémy on 05/01/2026.
//

#include "RMDLWorld.hpp"

static const float PHI = 1.618033988749895f;

const float CELL_SIZE = 2.0f;

GeometricGrid::GeometricGrid(int w, int h, int d)
    : width(w), height(h), depth(d)
    , vertexBuffer(nullptr), indexBuffer(nullptr)
    , indexCount(0), needsRebuild(true)
{
    cells.resize(w * h * d);
    for (auto& cell : cells) {
        cell.shape = CellShape::EMPTY;
        cell.color = simd::make_float4(1, 1, 1, 1);
        cell.rotation = 0.0f;
        cell.variant = 0;
    }
}

GeometricGrid::~GeometricGrid() {
    if (vertexBuffer) vertexBuffer->release();
    if (indexBuffer) indexBuffer->release();
}

int GeometricGrid::index(int x, int y, int z) const {
    return x + y * width + z * width * height;
}

bool GeometricGrid::isValidCoord(int x, int y, int z) const {
    return x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth;
}

void GeometricGrid::setCell(int x, int y, int z, CellShape shape, simd::float4 color) {
    if (!isValidCoord(x, y, z)) return;
    cells[index(x, y, z)].shape = shape;
    cells[index(x, y, z)].color = color;
    needsRebuild = true;
}

GridCell GeometricGrid::getCell(int x, int y, int z) const {
    if (!isValidCoord(x, y, z)) {
        GridCell empty;
        empty.shape = CellShape::EMPTY;
        return empty;
    }
    return cells[index(x, y, z)];
}

bool GeometricGrid::isEmpty(int x, int y, int z) const {
    return getCell(x, y, z).shape == CellShape::EMPTY;
}

void GeometricGrid::clear() {
    for (auto& cell : cells) {
        cell.shape = CellShape::EMPTY;
    }
    needsRebuild = true;
}

simd::float3 GeometricGrid::gridToWorld(int x, int y, int z) const {
    return simd::make_float3(
        (x - width * 0.5f) * CELL_SIZE,
        y * CELL_SIZE,
        (z - depth * 0.5f) * CELL_SIZE
    );
}

float GeometricGrid::perlinNoise3D(float x, float y, float z) {
    int xi = (int)floorf(x) & 255;
    int yi = (int)floorf(y) & 255;
    int zi = (int)floorf(z) & 255;
    
    float xf = x - floorf(x);
    float yf = y - floorf(y);
    float zf = z - floorf(z);
    
    auto fade = [](float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); };
    
    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);
    
    auto hash = [](int x, int y, int z) -> float {
        int n = x * 1619 + y * 31337 + z * 6971;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    };
    
    float c000 = hash(xi, yi, zi);
    float c100 = hash(xi + 1, yi, zi);
    float c010 = hash(xi, yi + 1, zi);
    float c110 = hash(xi + 1, yi + 1, zi);
    float c001 = hash(xi, yi, zi + 1);
    float c101 = hash(xi + 1, yi, zi + 1);
    float c011 = hash(xi, yi + 1, zi + 1);
    float c111 = hash(xi + 1, yi + 1, zi + 1);
    
    float x1 = c000 * (1 - u) + c100 * u;
    float x2 = c010 * (1 - u) + c110 * u;
    float y1 = x1 * (1 - v) + x2 * v;
    
    float x3 = c001 * (1 - u) + c101 * u;
    float x4 = c011 * (1 - u) + c111 * u;
    float y2 = x3 * (1 - v) + x4 * v;
    
    return y1 * (1 - w) + y2 * w;
}

void GeometricGrid::generatePerlinTerrain(float scale, float threshold) {
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            float noise = perlinNoise3D(x * scale, 0, z * scale) * 0.5f + 0.5f;
            noise += perlinNoise3D(x * scale * 2.0f, 0, z * scale * 2.0f) * 0.25f;
            noise += perlinNoise3D(x * scale * 4.0f, 0, z * scale * 4.0f) * 0.125f;
            
            int maxHeight = (int)(noise * height);
            
            for (int y = 0; y < maxHeight && y < height; y++) {
                float heightRatio = (float)y / height;
                
                CellShape shape;
                simd::float4 color;
                
                if (heightRatio < 0.3f) {
                    shape = CellShape::CUBE;
                    color = simd::make_float4(0.4f, 0.3f, 0.2f, 1.0f);
                } else if (heightRatio < 0.6f) {
                    shape = (noise > 0.6f) ? CellShape::ICOSPHERE : CellShape::CUBE;
                    color = simd::make_float4(0.3f, 0.6f, 0.3f, 1.0f);
                } else {
                    shape = CellShape::TRIANGULAR_PRISM;
                    color = simd::make_float4(0.6f + noise * 0.2f, 0.7f, 0.8f, 1.0f);
                }
                
                setCell(x, y, z, shape, color);
            }
        }
    }
}

void GeometricGrid::generateVoronoiStructure(int numSeeds) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    std::vector<simd::float3> seeds;
    for (int i = 0; i < numSeeds; i++) {
        seeds.push_back(simd::make_float3(
            dist(rng) * width,
            dist(rng) * height,
            dist(rng) * depth
        ));
    }
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                simd::float3 pos = simd::make_float3(x, y, z);
                
                float minDist = 10000.0f;
                int closestSeed = 0;
                
                for (int i = 0; i < numSeeds; i++) {
                    float dist = simd::length(pos - seeds[i]);
                    if (dist < minDist) {
                        minDist = dist;
                        closestSeed = i;
                    }
                }
                
                if (minDist < 8.0f) {
                    CellShape shape;
                    if (closestSeed % 3 == 0) shape = CellShape::CUBE;
                    else if (closestSeed % 3 == 1) shape = CellShape::ICOSPHERE;
                    else shape = CellShape::TRIANGULAR_PRISM;
                    
                    float hue = (float)closestSeed / numSeeds;
                    simd::float4 color = simd::make_float4(
                        0.5f + 0.5f * sinf(hue * 6.28f),
                        0.5f + 0.5f * sinf(hue * 6.28f + 2.09f),
                        0.5f + 0.5f * sinf(hue * 6.28f + 4.18f),
                        1.0f
                    );
                    
                    setCell(x, y, z, shape, color);
                }
            }
        }
    }
}

void GeometricGrid::generateWave(float amplitude, float frequency) {
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < depth; z++) {
            float wave1 = sinf(x * frequency) * amplitude;
            float wave2 = cosf(z * frequency) * amplitude;
            int y = (int)(height * 0.5f + wave1 + wave2);
            
            if (y >= 0 && y < height) {
                CellShape shape;
                if ((x + z) % 3 == 0) shape = CellShape::CUBE;
                else if ((x + z) % 3 == 1) shape = CellShape::ICOSPHERE;
                else shape = CellShape::TRIANGULAR_PRISM;
                
                float colorPhase = (x * frequency + z * frequency);
                simd::float4 color = simd::make_float4(
                    0.5f + 0.5f * sinf(colorPhase),
                    0.5f + 0.5f * cosf(colorPhase),
                    0.7f,
                    1.0f
                );
                
                setCell(x, y, z, shape, color);
            }
        }
    }
}

void GeometricGrid::generateSphere(simd::float3 center, float radius, CellShape shape) {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                simd::float3 pos = simd::make_float3(x, y, z);
                float dist = simd::length(pos - center);
                
                if (dist < radius) {
                    float ratio = dist / radius;
                    simd::float4 color = simd::make_float4(
                        1.0f - ratio * 0.5f,
                        0.5f + ratio * 0.3f,
                        0.8f,
                        1.0f
                    );
                    setCell(x, y, z, shape, color);
                }
            }
        }
    }
}

void GeometricGrid::buildMesh(MTL::Device* device) {
    if (!needsRebuild) return;
    
    std::vector<GeometricVertex> vertices;
    std::vector<uint32_t> indices;
    
    vertices.reserve(width * height * depth * 24);
    indices.reserve(width * height * depth * 36);
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                GridCell cell = getCell(x, y, z);
                if (cell.shape == CellShape::EMPTY) continue;
                
                switch (cell.shape) {
                    case CellShape::CUBE:
                        addCube(x, y, z, cell, vertices, indices);
                        break;
                    case CellShape::ICOSPHERE:
                        addIcosphere(x, y, z, cell, vertices, indices);
                        break;
                    case CellShape::TRIANGULAR_PRISM:
                        addPrism(x, y, z, cell, vertices, indices);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    if (vertexBuffer) vertexBuffer->release();
    if (indexBuffer) indexBuffer->release();
    
    if (vertices.empty()) {
        vertexBuffer = nullptr;
        indexBuffer = nullptr;
        indexCount = 0;
    } else {
        vertexBuffer = device->newBuffer(
            vertices.data(),
            vertices.size() * sizeof(GeometricVertex),
            MTL::ResourceStorageModeShared
        );
        
        indexBuffer = device->newBuffer(
            indices.data(),
            indices.size() * sizeof(uint32_t),
            MTL::ResourceStorageModeShared
        );
        
        indexCount = (uint32_t)indices.size();
    }
    
    needsRebuild = false;
}

void GeometricGrid::addCube(int x, int y, int z, const GridCell& cell,
                           std::vector<GeometricVertex>& vertices,
                           std::vector<uint32_t>& indices)
{
    simd::float3 center = gridToWorld(x, y, z);
    float halfSize = CELL_SIZE * 0.5f;
    
    bool faces[6];
    faces[0] = isEmpty(x, y + 1, z);
    faces[1] = isEmpty(x, y - 1, z);
    faces[2] = isEmpty(x, y, z - 1);
    faces[3] = isEmpty(x, y, z + 1);
    faces[4] = isEmpty(x + 1, y, z);
    faces[5] = isEmpty(x - 1, y, z);
    
    ShapeLibrary::generateCubeFaces(center, CELL_SIZE, cell.color, faces, vertices, indices);
}

void GeometricGrid::addIcosphere(int x, int y, int z, const GridCell& cell,
                                std::vector<GeometricVertex>& vertices,
                                std::vector<uint32_t>& indices)
{
    simd::float3 center = gridToWorld(x, y, z);
    float radius = CELL_SIZE * 0.45f;
    
    ShapeLibrary::generateIcosphere(center, radius, cell.color, 1, vertices, indices);
}

void GeometricGrid::addPrism(int x, int y, int z, const GridCell& cell,
                            std::vector<GeometricVertex>& vertices,
                            std::vector<uint32_t>& indices)
{
    simd::float3 center = gridToWorld(x, y, z);
    float size = CELL_SIZE * 0.8f;
    float rotation = cell.rotation;
    
    ShapeLibrary::generateTriangularPrism(center, size, rotation, cell.color, vertices, indices);
}

void GeometricGrid::render(MTL::RenderCommandEncoder* encoder) {
    if (indexCount == 0 || !vertexBuffer || !indexBuffer) return;
    
    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    encoder->drawIndexedPrimitives(
        MTL::PrimitiveTypeTriangle,
        indexCount,
        MTL::IndexTypeUInt32,
        indexBuffer,
        0
    );
}

void ShapeLibrary::generateCubeFaces(
    simd::float3 center,
    float size,
    simd::float4 color,
    const bool faces[6],
    std::vector<GeometricVertex>& vertices,
    std::vector<uint32_t>& indices)
{
    float h = size * 0.5f;
    
    simd::float3 normals[6] = {
        {0, 1, 0}, {0, -1, 0}, {0, 0, -1},
        {0, 0, 1}, {1, 0, 0}, {-1, 0, 0}
    };
    
    simd::float3 faceVerts[6][4] = {
        {{-h,h,-h}, {h,h,-h}, {h,h,h}, {-h,h,h}},
        {{-h,-h,-h}, {-h,-h,h}, {h,-h,h}, {h,-h,-h}},
        {{-h,-h,-h}, {h,-h,-h}, {h,h,-h}, {-h,h,-h}},
        {{h,-h,h}, {-h,-h,h}, {-h,h,h}, {h,h,h}},
        {{h,-h,-h}, {h,-h,h}, {h,h,h}, {h,h,-h}},
        {{-h,-h,h}, {-h,-h,-h}, {-h,h,-h}, {-h,h,h}}
    };
    
    float brightness[6] = {1.0f, 0.5f, 0.7f, 0.7f, 0.9f, 0.6f};
    
    for (int f = 0; f < 6; f++) {
        if (!faces[f]) continue;
        
        uint32_t baseIdx = (uint32_t)vertices.size();
        simd::float4 faceColor = color * brightness[f];
        
        for (int v = 0; v < 4; v++) {
            GeometricVertex vertex;
            vertex.position = center + faceVerts[f][v];
            vertex.normal = normals[f];
            vertex.color = faceColor;
            vertex.uv = simd::make_float2(v % 2, v / 2);
            vertices.push_back(vertex);
        }
        
        indices.push_back(baseIdx + 0);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 0);
        indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 3);
    }
}

void ShapeLibrary::generateIcosphere(
    simd::float3 center,
    float radius,
    simd::float4 color,
    int subdivisions,
    std::vector<GeometricVertex>& vertices,
    std::vector<uint32_t>& indices)
{
    const float t = (1.0f + sqrtf(5.0f)) / 2.0f;
    
    simd::float3 baseVerts[12] = {
        simd::normalize(simd::make_float3(-1, t, 0)) * radius,
        simd::normalize(simd::make_float3(1, t, 0)) * radius,
        simd::normalize(simd::make_float3(-1, -t, 0)) * radius,
        simd::normalize(simd::make_float3(1, -t, 0)) * radius,
        simd::normalize(simd::make_float3(0, -1, t)) * radius,
        simd::normalize(simd::make_float3(0, 1, t)) * radius,
        simd::normalize(simd::make_float3(0, -1, -t)) * radius,
        simd::normalize(simd::make_float3(0, 1, -t)) * radius,
        simd::normalize(simd::make_float3(t, 0, -1)) * radius,
        simd::normalize(simd::make_float3(t, 0, 1)) * radius,
        simd::normalize(simd::make_float3(-t, 0, -1)) * radius,
        simd::normalize(simd::make_float3(-t, 0, 1)) * radius
    };
    
    uint32_t baseIdx = (uint32_t)vertices.size();
    
    for (int i = 0; i < 12; i++) {
        GeometricVertex v;
        v.position = center + baseVerts[i];
        v.normal = simd::normalize(baseVerts[i]);
        v.color = color;
        v.uv = simd::make_float2(0, 0);
        vertices.push_back(v);
    }
    
    uint32_t tris[20][3] = {
        {0,11,5}, {0,5,1}, {0,1,7}, {0,7,10}, {0,10,11},
        {1,5,9}, {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
        {3,9,4}, {3,4,2}, {3,2,6}, {3,6,8}, {3,8,9},
        {4,9,5}, {2,4,11}, {6,2,10}, {8,6,7}, {9,8,1}
    };
    
    for (int i = 0; i < 20; i++) {
        indices.push_back(baseIdx + tris[i][0]);
        indices.push_back(baseIdx + tris[i][1]);
        indices.push_back(baseIdx + tris[i][2]);
    }
}

void ShapeLibrary::generateTriangularPrism(
    simd::float3 center,
    float size,
    float rotation,
    simd::float4 color,
    std::vector<GeometricVertex>& vertices,
    std::vector<uint32_t>& indices)
{
    float h = size * 0.5f;
    float r = size * 0.4f;
    
    simd::float3 baseVerts[3];
    simd::float3 topVerts[3];
    
    for (int i = 0; i < 3; i++) {
        float angle = rotation + i * 2.0944f;
        float x = cosf(angle) * r;
        float z = sinf(angle) * r;
        baseVerts[i] = center + simd::make_float3(x, -h, z);
        topVerts[i] = center + simd::make_float3(x, h, z);
    }
    
    uint32_t baseIdx = (uint32_t)vertices.size();
    
    for (int i = 0; i < 3; i++) {
        GeometricVertex v;
        v.position = baseVerts[i];
        v.normal = simd::make_float3(0, -1, 0);
        v.color = color;
        v.uv = simd::make_float2(0, 0);
        vertices.push_back(v);
    }
    
    for (int i = 0; i < 3; i++) {
        GeometricVertex v;
        v.position = topVerts[i];
        v.normal = simd::make_float3(0, 1, 0);
        v.color = color;
        v.uv = simd::make_float2(0, 0);
        vertices.push_back(v);
    }
    
    indices.push_back(baseIdx + 0);
    indices.push_back(baseIdx + 2);
    indices.push_back(baseIdx + 1);
    
    indices.push_back(baseIdx + 3);
    indices.push_back(baseIdx + 4);
    indices.push_back(baseIdx + 5);
    
    for (int i = 0; i < 3; i++) {
        int next = (i + 1) % 3;
        
        simd::float3 edge = topVerts[i] - baseVerts[i];
        simd::float3 side = baseVerts[next] - baseVerts[i];
        simd::float3 normal = simd::normalize(simd::cross(edge, side));
        
        uint32_t vIdx = (uint32_t)vertices.size();
        
        for (int v = 0; v < 4; v++) {
            GeometricVertex vert;
            if (v == 0) vert.position = baseVerts[i];
            else if (v == 1) vert.position = baseVerts[next];
            else if (v == 2) vert.position = topVerts[i];
            else vert.position = topVerts[next];
            
            vert.normal = normal;
            vert.color = color * 0.9f;
            vert.uv = simd::make_float2(0, 0);
            vertices.push_back(vert);
        }
        
        indices.push_back(vIdx + 0);
        indices.push_back(vIdx + 1);
        indices.push_back(vIdx + 2);
        
        indices.push_back(vIdx + 1);
        indices.push_back(vIdx + 3);
        indices.push_back(vIdx + 2);
    }
}

ModularWorld::ModularWorld(
    MTL::Device* device,
    MTL::PixelFormat pixelFormat,
    MTL::PixelFormat depthPixelFormat,
    MTL::Library* library)
    : device(device->retain())
{
    grid = std::make_unique<GeometricGrid>(32, 32, 32);
    createPipeline(library, pixelFormat, depthPixelFormat);
}

ModularWorld::~ModularWorld() {
    if (pipelineState) pipelineState->release();
    if (depthStencilState) depthStencilState->release();
    device->release();
}

void ModularWorld::createPipeline(
    MTL::Library* library,
    MTL::PixelFormat pixelFormat,
    MTL::PixelFormat depthPixelFormat)
{
    auto vertexFunc = library->newFunction(
        NS::String::string("voxel_vertex", NS::UTF8StringEncoding)
    );
    auto fragmentFunc = library->newFunction(
        NS::String::string("voxel_fragment", NS::UTF8StringEncoding)
    );
    
    auto pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setVertexFunction(vertexFunc);
    pipelineDesc->setFragmentFunction(fragmentFunc);
    pipelineDesc->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pipelineDesc->setDepthAttachmentPixelFormat(depthPixelFormat);
    
    auto vertexDesc = MTL::VertexDescriptor::alloc()->init();
    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(0)->setOffset(0);
    vertexDesc->attributes()->object(0)->setBufferIndex(0);
    
    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
    vertexDesc->attributes()->object(1)->setOffset(sizeof(simd::float3));
    vertexDesc->attributes()->object(1)->setBufferIndex(0);
    
    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(2)->setOffset(sizeof(simd::float3) + sizeof(simd::float4));
    vertexDesc->attributes()->object(2)->setBufferIndex(0);
    
    vertexDesc->layouts()->object(0)->setStride(sizeof(GeometricVertex));
    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    
    pipelineDesc->setVertexDescriptor(vertexDesc);
    
    NS::Error* error = nullptr;
    pipelineState = device->newRenderPipelineState(pipelineDesc, &error);
    
    auto depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    depthStencilState = device->newDepthStencilState(depthDesc);
    
    vertexFunc->release();
    fragmentFunc->release();
    pipelineDesc->release();
    vertexDesc->release();
    depthDesc->release();
}

void ModularWorld::regenerateWorld() {
    grid->clear();
    grid->generatePerlinTerrain(0.1f, 0.5f);
}

void ModularWorld::setWorldSize(int width, int height, int depth) {
    grid = std::make_unique<GeometricGrid>(width, height, depth);
}

void ModularWorld::update(float dt, simd::float3 cameraPos) {
    grid->buildMesh(device);
}

void ModularWorld::render(MTL::RenderCommandEncoder* encoder, simd::float4x4 viewProjectionMatrix) {
    encoder->setRenderPipelineState(pipelineState);
    encoder->setDepthStencilState(depthStencilState);
    encoder->setCullMode(MTL::CullModeBack);
    
    grid->render(encoder);
}

void ModularWorld::editCell(simd::float3 worldPos, CellShape shape, simd::float4 color) {
    int x = (int)((worldPos.x / CELL_SIZE) + grid->getWidth() * 0.5f);
    int y = (int)(worldPos.y / CELL_SIZE);
    int z = (int)((worldPos.z / CELL_SIZE) + grid->getDepth() * 0.5f);
    
    grid->setCell(x, y, z, shape, color);
}

void ModularWorld::removeCell(simd::float3 worldPos) {
    editCell(worldPos, CellShape::EMPTY, simd::make_float4(0, 0, 0, 0));
}
