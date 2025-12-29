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
{
    m_vertices.reserve(m_maxVertices);
    m_indices.reserve(m_maxIndices);

    fontAtlas = createFont(device);
    createShadersAndPipelineStates(shaderLibrary, pixelFormat, depthPixelFormat, device);
    createBuffers(device);

    auto samplerDesc = NS::TransferPtr(MTL::SamplerDescriptor::alloc()->init());
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setRAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDesc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDesc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    m_sampler = NS::TransferPtr(device->newSamplerState(samplerDesc.get()));
}

MetalUIManager::~MetalUIManager()
{
}

bool MetalUIManager::createShadersAndPipelineStates(MTL::Library *shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device)
{
    const char* shaderSource = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct RMDLUi
        {
            float4x4 uiProjectionMatrix;
        };

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

        vertex VertexOut vertexShaderTxt(VertexIn in [[stage_in]],
                                         constant RMDLUi& uniforms [[buffer(1)]])
        {
            VertexOut out;
            out.position = uniforms.uiProjectionMatrix * float4(in.position, 1.0);
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
    vertexDesc->attributes()->object(1)->setOffset(16); //sizeof(float) * 3);
    vertexDesc->attributes()->object(1)->setBufferIndex(0);

    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat4);
    vertexDesc->attributes()->object(2)->setOffset(24); //sizeof(float) * 5);
    vertexDesc->attributes()->object(2)->setBufferIndex(0);
    printf("%lu\n", sizeof(simd::float2));
    printf("%lu\n", sizeof(simd::float3));
    printf("%lu\n", sizeof(simd::float4));

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

    m_frameDataBuffer = NS::TransferPtr(device->newBuffer(sizeof(uiMatrix), MTL::ResourceStorageModeShared));

    return m_vertexBuffer && m_indexBuffer;
}

void MetalUIManager::beginFrame(MTL::RenderCommandEncoder* encoder,
                                NS::UInteger width, NS::UInteger height)
{
    uiMatrix.uiProjectionMatrix = math::makeOrtho(0, (float)width, (float)height, 0, -1, 1);

    m_vertices.clear();
    m_indices.clear();
    m_batches.clear();
}

void MetalUIManager::endFrame(MTL::RenderCommandEncoder* encoder)
{
    if (m_vertices.empty())
        return;

    ft_memcpy(m_vertexBuffer->contents(), m_vertices.data(), m_vertices.size() * sizeof(UIVertex));
    ft_memcpy(m_indexBuffer->contents(), m_indices.data(), m_indices.size() * sizeof(uint16_t));
    ft_memcpy(m_frameDataBuffer->contents(), &uiMatrix, sizeof(uiMatrix));

    encoder->setVertexBuffer(m_vertexBuffer.get(), 0, 0);
    encoder->setVertexBuffer(m_frameDataBuffer.get(), 0, 1);

    for (size_t i = 0; i < m_batches.size(); i++)
    {
        const RenderBatch& batch = m_batches[i];

        if (batch.type == BatchType::Text)
        {
            encoder->setRenderPipelineState(m_textPipeline.get());
            encoder->setFragmentTexture(fontAtlas.texture.get(), 0);
            encoder->setFragmentSamplerState(m_sampler.get(), 0);
        } else {
            encoder->setRenderPipelineState(m_shapePipeline.get());
        }
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, batch.indexCount,
                                       MTL::IndexTypeUInt16, m_indexBuffer.get(),
                                       batch.startIndex * sizeof(uint16_t));
    }
//    ft_memcpy(m_vertexBuffer->contents(), m_vertices.data(),
//           m_vertices.size() * sizeof(UIVertex));
//    ft_memcpy(m_indexBuffer->contents(), m_indices.data(),
//           m_indices.size() * sizeof(uint16_t));
//
//    encoder->setVertexBuffer(m_vertexBuffer.get(), 0, 0);
//    encoder->setVertexBytes(&m_projectionMatrix, sizeof(simd::float4x4), 1);
//    encoder->setRenderPipelineState(m_shapePipeline.get());
////    encoder->setRenderPipelineState(m_textPipeline.get());
//    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
//                                   m_indices.size(),
//                                   MTL::IndexTypeUInt16,
//                                   m_indexBuffer.get(), 0);
}

void MetalUIManager::drawText(const std::string& text, float x, float y, float scale, simd::float4 color)
{
    float currentX = x;
    
    for (char c : text)
    {
        auto it = fontAtlas.charUV.find(c);
        if (it == fontAtlas.charUV.end())
            continue;
        
        const Font::CharUV& uv = it->second;
        float w = uv.width * scale;
        float h = uv.height * scale;

        UIVertex vertices[4] = {
            {{currentX, y, 0}, uv.nw, color},
            {{currentX + w, y, 0}, uv.ne, color},
            {{currentX + w, y + h, 0}, uv.se, color},
            {{currentX, y + h, 0}, uv.sw, color}
        };

        addQuad(vertices, BatchType::Text);
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

    addQuad(vertices, BatchType::Shape);
}

void MetalUIManager::addQuad(const UIVertex* vertices, BatchType type)
{
    if (m_batches.empty() || m_batches.back().type != type)
            startNewBatch(type);

    uint16_t baseIndex = m_vertices.size();
    
    m_vertices.insert(m_vertices.end(), vertices, vertices + 4);
    
    uint16_t indices[] = {
        baseIndex, uint16_t(baseIndex + 1), uint16_t(baseIndex + 2),
        baseIndex, uint16_t(baseIndex + 2), uint16_t(baseIndex + 3)
    };
    
    m_indices.insert(m_indices.end(), indices, indices + 6);
    m_batches.back().indexCount += 6;
}

void MetalUIManager::startNewBatch(BatchType type)
{
    RenderBatch batch;
    batch.type = type;
    batch.startIndex = m_indices.size();
    batch.indexCount = 0;
    m_batches.push_back(batch);
}

float MetalUIManager::toNDCX(float x, float width) const
{
    return (x / width) * 2.0f - 1.0f;
}

float MetalUIManager::toNDCY(float y, float height) const
{
    return 1.0f - (y / height) * 2.0f;
}
