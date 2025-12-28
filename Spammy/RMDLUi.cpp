//
//  RMDLUi.cpp
//  Spammy
//
//  Created by Rémy on 28/12/2025.
//

#include "RMDLUi.hpp"

MetalUIManager::MetalUIManager(MTL::Device* device, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, NS::UInteger width, NS::UInteger height, MTL::Library* shaderLibrary)
    : m_fontSize(16.0f)
    , m_maxVertices(10000)
    , m_maxIndices(30000)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_mousePressed(false)
    , m_mousePressedLastFrame(false)
{
    m_vertices.reserve(m_maxVertices);
    m_indices.reserve(m_maxIndices);

    createFontAtlas(device, pixelFormat);
    createShadersAndPipelineStates(shaderLibrary, pixelFormat, depthPixelFormat, device);
    createBuffers(device);
}

MetalUIManager::~MetalUIManager()
{
}

bool MetalUIManager::createFontAtlas(MTL::Device* device, MTL::PixelFormat pixelFormat)
{
    const uint32_t atlasWidth = 512;
    const uint32_t atlasHeight = 512;
    const uint32_t charWidth = atlasWidth / 16;
    const uint32_t charHeight = atlasHeight / 6;

    std::vector<uint8_t> atlasData(atlasWidth * atlasHeight * 4, 122); // 0 or 255 noir / blanc

    for (int i = 0; i < 96; i++)
    {
        char c = ' ' + i;
        int gridX = i % 16;
        int gridY = i / 16;

        FontCharacter& fc = m_fontCharacters[c];
        fc.uvMin = simd_make_float2(gridX / 16.0f, gridY / 6.0f);
        fc.uvMax = simd_make_float2((gridX + 1) / 16.0f, (gridY + 1) / 6.0f);
        fc.advance = charWidth;
    }

    auto textureDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    textureDesc->setWidth(atlasWidth);
    textureDesc->setHeight(atlasHeight);
    textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);//_sRGB);
    textureDesc->setTextureType(MTL::TextureType2D);
    textureDesc->setUsage(MTL::TextureUsageShaderRead);
    textureDesc->setStorageMode(MTL::StorageModeShared);

    m_fontTexture = NS::TransferPtr(device->newTexture(textureDesc.get()));
    m_fontTexture->replaceRegion(MTL::Region(0, 0, atlasWidth, atlasHeight), 0, atlasData.data(), atlasWidth * 4);

    return true;
}

bool MetalUIManager::createShadersAndPipelineStates(MTL::Library *shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device)
{
    const char* shaderSource = R"(
        #include <metal_stdlib>
        using namespace metal;
        
        struct VertexIn
        {
            float3 position [[attribute(0)]];
            float2 texCoord [[attribute(1)]];
            float4 color [[attribute(2)]];
        };
        
        struct VertexOut
        {
            float4 position [[position]];
            float2 texCoord;
            float4 color;
        };
        
        struct Uniforms
        {
            float4x4 projection;
        };
        
        vertex VertexOut vertexShaderTxt(VertexIn in [[stage_in]],
                                         constant Uniforms& uniforms [[buffer(1)]])
        {
            VertexOut out;
            out.position = uniforms.projection * float4(in.position, 1.0);
            out.texCoord = in.texCoord;
            out.color = in.color;
            return out;
        }
        
        fragment float4 fragmentShaderText(VertexOut in [[stage_in]],
                                          texture2d<float> tex [[texture(0)]])
        {
            constexpr sampler s(filter::linear);
            float alpha = tex.sample(s, in.texCoord).r;
            return float4(in.color.rgb, in.color.a * alpha);
        }
        
        fragment float4 fragmentShaderShape(VertexOut in [[stage_in]])
        {
            return in.color;
        }
    )";
    
    NS::Error* error = nullptr;
    MTL::Library* library = device->newLibrary(NS::String::string(shaderSource, NS::UTF8StringEncoding), nullptr, &error);
    if (!library)
    {
        __builtin_printf("%s", error->localizedDescription()->utf8String());
        printf("Failed to create shader library intern\n");
        return false;
    }

    auto vertexFunc = NS::TransferPtr(library->newFunction(NS::String::string("vertexShaderTxt", NS::UTF8StringEncoding)));
    auto fragFuncText = NS::TransferPtr(library->newFunction(NS::String::string("fragmentShaderText", NS::UTF8StringEncoding)));
    auto fragFuncShape = NS::TransferPtr(library->newFunction(NS::String::string("fragmentShaderShape", NS::UTF8StringEncoding)));

    auto vertexDesc = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());

    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertexDesc->attributes()->object(0)->setOffset(0);
    vertexDesc->attributes()->object(0)->setBufferIndex(0);

    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vertexDesc->attributes()->object(1)->setOffset(sizeof(float) * 3);
    vertexDesc->attributes()->object(1)->setBufferIndex(0);

    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat4);
    vertexDesc->attributes()->object(2)->setOffset(sizeof(float) * 5);
    vertexDesc->attributes()->object(2)->setBufferIndex(0);

    vertexDesc->layouts()->object(0)->setStride(sizeof(UIVertex));
    vertexDesc->layouts()->object(0)->setStepRate(1);
    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    auto pipelineDesc = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pipelineDesc->setVertexFunction(vertexFunc.get());
    pipelineDesc->setFragmentFunction(fragFuncText.get());
    pipelineDesc->setVertexDescriptor(vertexDesc.get());
    pipelineDesc->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pipelineDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
    pipelineDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    pipelineDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    pipelineDesc->setDepthAttachmentPixelFormat(depthPixelFormat);

    m_textPipeline = NS::TransferPtr(device->newRenderPipelineState(pipelineDesc.get(), &error));
    pipelineDesc->setFragmentFunction(fragFuncShape.get());
    m_shapePipeline = NS::TransferPtr(device->newRenderPipelineState(pipelineDesc.get(), &error));

    if ( !m_shapePipeline || !m_textPipeline)
    {
        __builtin_printf( "%s", error->localizedDescription()->utf8String() );
        assert( false );
    }

    shaderLibrary = library;
    return m_textPipeline && m_shapePipeline;
}

bool MetalUIManager::createBuffers(MTL::Device* device)
{
    m_vertexBuffer = NS::TransferPtr(device->newBuffer(m_maxVertices * sizeof(UIVertex), MTL::ResourceStorageModeShared));

    m_indexBuffer = NS::TransferPtr(device->newBuffer(m_maxIndices * sizeof(uint16_t), MTL::ResourceStorageModeShared));

    return m_vertexBuffer && m_indexBuffer;
}

void MetalUIManager::beginFrame(MTL::RenderCommandEncoder* encoder,
                                NS::UInteger width, NS::UInteger height)
{
//    m_currentEncoder = encoder;
    m_projectionMatrix = math::makeOrtho(0, (float)width, (float)height, 0, -1, 1);

    m_vertices.clear();
    m_indices.clear();

    m_mousePressedLastFrame = m_mousePressed;
}

void MetalUIManager::endFrame(MTL::RenderCommandEncoder* encoder)
{
    if (m_vertices.empty())
        return;

    ft_memcpy(m_vertexBuffer->contents(), m_vertices.data(),
           m_vertices.size() * sizeof(UIVertex));
    ft_memcpy(m_indexBuffer->contents(), m_indices.data(),
           m_indices.size() * sizeof(uint16_t));
    
    encoder->setVertexBuffer(m_vertexBuffer.get(), 0, 0);
    encoder->setVertexBytes(&m_projectionMatrix, sizeof(simd::float4x4), 1);
    encoder->setRenderPipelineState(m_shapePipeline.get());
//    encoder->setRenderPipelineState(m_textPipeline.get());
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                   m_indices.size(),
                                   MTL::IndexTypeUInt16,
                                   m_indexBuffer.get(), 0);

//    m_currentEncoder = nullptr;
}

void MetalUIManager::drawText(const std::string& text, float x, float y, float scale, simd::float4 color)
{
    float currentX = x;
    
    for (char c : text)
    {
        if (m_fontCharacters.find(c) == m_fontCharacters.end()) continue;

        const FontCharacter& fc = m_fontCharacters[c];
        float w = fc.advance * scale;
        float h = m_fontSize * scale;

        UIVertex vertices[4] = {
            {{currentX, y, 0}, fc.uvMin, color},
            {{currentX + w, y, 0}, {fc.uvMax.x, fc.uvMin.y}, color},
            {{currentX + w, y + h, 0}, fc.uvMax, color},
            {{currentX, y + h, 0}, {fc.uvMin.x, fc.uvMax.y}, color}
        };

        addQuad(vertices);
        currentX += w;
    }
}

void MetalUIManager::drawRect(const UIRect& rect, simd::float4 color, bool filled)
{
    UIVertex vertices[4] = {
        {{rect.x, rect.y, 0}, {0, 0}, color},
        {{rect.x + rect.width, rect.y, 0}, {1, 0}, color},
        {{rect.x + rect.width, rect.y + rect.height, 0}, {1, 1}, color},
        {{rect.x, rect.y + rect.height, 0}, {0, 1}, color}
    };

    addQuad(vertices);
}

bool MetalUIManager::drawButton(const std::string& label, const UIRect& rect, bool* hovered)
{
    bool isHovered = rect.contains(m_mouseX, m_mouseY);
    if (hovered) *hovered = isHovered;

    simd::float4 bgColor = isHovered ?
        simd_make_float4(0.3f, 0.5f, 0.8f, 1.0f) :
        simd_make_float4(0.2f, 0.4f, 0.7f, 1.0f);

    drawRect(rect, bgColor, true);

    // Texte centré
    float textX = rect.x + (rect.width - label.length() * 8) * 0.5f;
    float textY = rect.y + (rect.height - 16) * 0.5f;
    drawText(label, textX, textY, 1.0f, {1, 1, 1, 1});
    
    bool clicked = isHovered && m_mousePressed && !m_mousePressedLastFrame;
    return clicked;
}

void MetalUIManager::addQuad(const UIVertex* vertices)
{
    uint16_t baseIndex = m_vertices.size();
    
    m_vertices.insert(m_vertices.end(), vertices, vertices + 4);
    
    uint16_t indices[] = {
        baseIndex, uint16_t(baseIndex + 1), uint16_t(baseIndex + 2),
        baseIndex, uint16_t(baseIndex + 2), uint16_t(baseIndex + 3)
    };
    
    m_indices.insert(m_indices.end(), indices, indices + 6);
}

float MetalUIManager::toNDCX(float x, float width) const
{
    return (x / width) * 2.0f - 1.0f;
}

float MetalUIManager::toNDCY(float y, float height) const
{
    return 1.0f - (y / height) * 2.0f;
}
