//
//  RMDLUi.hpp
//  Spammy
//
//  Created by Rémy on 28/12/2025.
//

#ifndef RMDLUi_hpp
#define RMDLUi_hpp

#include <Metal/Metal.hpp>

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

#include <simd/simd.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "RMDLMathUtils.hpp"
#include "RMDLUtils.hpp"
#include "RMDLFontLoader.h"
#include "RMDLUtilities.h"

#include "RMDLMainRenderer_shared.h"

struct RMDLUi
{
    simd::float4x4 uiProjectionMatrix;
};

struct RectangleUIData
{
    VertexRectangle vertex0;
    VertexRectangle vertex1;
    VertexRectangle vertex2;
    VertexRectangle vertex3;
    VertexRectangle vertex4;
    VertexRectangle vertex5;
};

struct UIVertex
{
    simd::float3 position;
    simd::float2 texCoord;
    simd::float4 color;
};

struct UIRect
{
    float x, y, width, height;

    UIRect(float _x = 0, float _y = 0, float _w = 0, float _h = 0) : x(_x), y(_y), width(_w), height(_h) {}

    bool contains(float px, float py) const {
        return px >= x && px <= (x + width) && py >= y && py <= (y + height);
    }
};

enum class BatchType { Shape, Text };

struct RenderBatch
{
    BatchType type;
    size_t startIndex;
    size_t indexCount;
};

class MetalUIManager
{
public:
    MetalUIManager(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary);
    ~MetalUIManager();
    
    void beginFrame(NS::UInteger width, NS::UInteger height);
    void endFrame(MTL::RenderCommandEncoder* encoder);
    
    void drawText(const std::string& text, float x, float y, float scale = 1.0f, simd::float4 color = {1, 1, 1, 1});
    
    void drawRect(const UIRect& rect, simd::float4 color, bool filled = true);
    void drawRoundedRect(const UIRect& rect, float radius, simd::float4 color, bool filled = true);
    void drawCircle(float x, float y, float radius, simd::float4 color, bool filled = true);
    void drawLine(float x1, float y1, float x2, float y2, float thickness, simd::float4 color);

    void drawPanel(const UIRect& rect, simd::float4 bgColor, float borderWidth = 2.0f, simd::float4 borderColor = {1, 1, 1, 1});
    void drawProgressBar(const UIRect& rect, float progress, simd::float4 bgColor, simd::float4 fgColor);
    
private:
    void createShadersAndPipelineStates(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device);
    void createBuffers(MTL::Device* device);

    void startNewBatch(BatchType type);
    void addQuad(const UIVertex* vertices, BatchType type);

    NS::SharedPtr<MTL::RenderPipelineState> m_textPipeline;
    NS::SharedPtr<MTL::RenderPipelineState> m_shapePipeline;
    NS::SharedPtr<MTL::DepthStencilState> m_depthState;

    std::vector<UIVertex> m_vertices;
    std::vector<uint16_t> m_indices;
    NS::SharedPtr<MTL::Buffer> m_vertexBuffer;
    NS::SharedPtr<MTL::Buffer> m_indexBuffer;
    NS::SharedPtr<MTL::Buffer> m_frameDataBuffer;
    size_t m_maxVertices;
    size_t m_maxIndices;

    RMDLUi uiMatrix;
    Font fontAtlas;
    NS::SharedPtr<MTL::SamplerState> m_sampler;
    std::vector<RenderBatch> m_batches;

    MTL::Buffer*                        m_rectangleDataBuffer;
    MTL4::ArgumentTable*                m_argumentTable;
    MTL::ResidencySet*                  m_residencySet;
    MTL::Buffer*                        m_viewportSizeBuffer;
    simd_uint2                          m_viewportSize;
    MTL::RenderPipelineState*           m_psoUi;
};

#endif /* RMDLUi_hpp */
