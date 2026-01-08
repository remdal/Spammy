//
//  RMDLCarray.hpp
//  Spammy
//
//  Created by Rémy on 05/01/2026.
//

#ifndef RMDLCarray_hpp
#define RMDLCarray_hpp

#include "Metal/Metal.hpp"
#include "simd/simd.h"

struct VertexCarray
{
    simd::float3 position;
    simd::float4 color;
    simd::float3 normal;
};

MTL::DepthStencilState*     m_depthStencilState;
MTL::RenderPipelineState*   m_renderPipelineState;

#endif /* RMDLCarray_hpp */
