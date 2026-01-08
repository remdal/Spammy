//
//  RMDLCarray.cpp
//  Spammy
//
//  Created by Rémy on 05/01/2026.
//

#include "RMDLCarray.hpp"

void createAPerfectPipeline(MTL::Library* shaderLibrary, MTL::PixelFormat pixelFormat, MTL::PixelFormat depthPixelFormat, MTL::Device* device)
{
    NS::Error* error = nullptr;

    NS::SharedPtr<MTL::Function> pVertexFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("voxel_vertex")));
    NS::SharedPtr<MTL::Function> pFragmentFunction = NS::TransferPtr(shaderLibrary->newFunction(MTLSTR("voxel_fragment")));

    NS::SharedPtr<MTL::RenderPipelineDescriptor> pRenderDescriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pRenderDescriptor->setVertexFunction(pVertexFunction.get());
    pRenderDescriptor->setFragmentFunction(pFragmentFunction.get());
    pRenderDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pRenderDescriptor->setDepthAttachmentPixelFormat(depthPixelFormat);
    
    NS::SharedPtr<MTL::VertexDescriptor> pVertexDesc = NS::TransferPtr(MTL::VertexDescriptor::alloc()->init());
    pVertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(0)->setOffset(0);
    pVertexDesc->attributes()->object(0)->setBufferIndex(0);

    pVertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
    pVertexDesc->attributes()->object(1)->setOffset(sizeof(simd::float3));
    pVertexDesc->attributes()->object(1)->setBufferIndex(0);

    pVertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
    pVertexDesc->attributes()->object(2)->setOffset(sizeof(simd::float3) + sizeof(simd::float4));
    pVertexDesc->attributes()->object(2)->setBufferIndex(0);

    pVertexDesc->layouts()->object(0)->setStride(sizeof(VertexCarray));
    pVertexDesc->layouts()->object(0)->setStepRate(1);
    pVertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    NS::SharedPtr<MTL::DepthStencilDescriptor> depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);

    pRenderDescriptor->setVertexDescriptor(pVertexDesc.get());
    m_renderPipelineState = device->newRenderPipelineState(pRenderDescriptor.get(), &error);
    m_depthStencilState = device->newDepthStencilState(depthStencilDescriptor.get());
}
