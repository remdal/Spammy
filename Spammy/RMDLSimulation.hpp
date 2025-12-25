//
//  RMDLSimulation.hpp
//  Spammy
//
//  Created by Rémy on 23/12/2025.
//

#ifndef RMDLSimulation_hpp
#define RMDLSimulation_hpp

#include <Metal/Metal.hpp>
#include <stdio.h>
#include "simd/simd.h"

class RMDLSimulation
{
public:
    RMDLSimulation(MTL::Device* pDevice);
    ~RMDLSimulation();
    
    void render(MTL::RenderCommandEncoder* pEncoder);

private:
    void createPipeline(MTL::Library* pShaderLibrary, MTL::PixelFormat pPixelFormat, MTL::PixelFormat pDepthPixelFormat);
    
    MTL::Device*                    _pDevice;
    MTL::Buffer*                    _pVertexBuffer;
    MTL::Buffer*                    _pUniformBuffer;
    MTL::DepthStencilState*         _pDepthState;
    MTL::RenderPipelineState*       _pPipelineStateBlender;
};

#endif /* RMDLSimulation_hpp */
