//
//  RMDLUi.hpp
//  Spammy
//
//  Created by Rémy on 28/12/2025.
//

#ifndef RMDLUi_hpp
#define RMDLUi_hpp

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "RMDLMathUtils.hpp"
#include "RMDLUtils.hpp"

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

struct FontCharacter
{
    simd::float2 uvMin;
    simd::float2 uvMax;
    float advance;
};

class MetalUIManager
{
public:
    MetalUIManager(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger heigth, MTL::Library* shaderLibrary);
    ~MetalUIManager();
    
    void beginFrame(MTL::RenderCommandEncoder* encoder, NS::UInteger width, NS::UInteger height);
    void endFrame(MTL::RenderCommandEncoder* encoder);
    
    void drawText(const std::string& text, float x, float y, float scale = 1.0f, simd::float4 color = {1, 1, 1, 1});
    
    void drawRect(const UIRect& rect, simd::float4 color, bool filled = true);
    void drawRoundedRect(const UIRect& rect, float radius, simd::float4 color, bool filled = true);
    void drawCircle(float x, float y, float radius, simd::float4 color, bool filled = true);
    void drawLine(float x1, float y1, float x2, float y2, float thickness, simd::float4 color);
    
    bool drawButton(const std::string& label, const UIRect& rect, bool* hovered = nullptr);
    void drawPanel(const UIRect& rect, simd::float4 bgColor, float borderWidth = 2.0f, simd::float4 borderColor = {1, 1, 1, 1});
    void drawProgressBar(const UIRect& rect, float progress, simd::float4 bgColor, simd::float4 fgColor);

    void setMousePosition(float x, float y) { m_mouseX = x; m_mouseY = y; }
    void setMouseButton(bool pressed) { m_mousePressed = pressed; }
    
private:
    bool createFontAtlas(MTL::Device* device, MTL::PixelFormat pixelFormat);
    bool createShadersAndPipelineStates(MTL::Library *shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device);
    bool createBuffers(MTL::Device* device);

    void addQuad(const UIVertex* vertices);

    float toNDCX(float x, float width) const;
    float toNDCY(float y, float height) const;

    NS::SharedPtr<MTL::RenderPipelineState> m_textPipeline;
    NS::SharedPtr<MTL::RenderPipelineState> m_shapePipeline;
    NS::SharedPtr<MTL::DepthStencilState> m_depthState;

    NS::SharedPtr<MTL::Texture> m_fontTexture;
    std::unordered_map<char, FontCharacter> m_fontCharacters;
    float m_fontSize;

    std::vector<UIVertex> m_vertices;
    std::vector<uint16_t> m_indices;
    NS::SharedPtr<MTL::Buffer> m_vertexBuffer;
    NS::SharedPtr<MTL::Buffer> m_indexBuffer;
    size_t m_maxVertices;
    size_t m_maxIndices;
    simd::float4x4 m_projectionMatrix;
    
    // Input state
    float m_mouseX, m_mouseY;
    bool m_mousePressed;
    bool m_mousePressedLastFrame;
};

#endif /* RMDLUi_hpp */
