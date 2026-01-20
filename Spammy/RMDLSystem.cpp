//
//  RMDLSystem.cpp
//  Spammy
//
//  Created by Rémy on 12/01/2026.
//

#include "RMDLSystem.hpp"

RenderSystem::RenderSystem(MTL::Device* device, MTL::PixelFormat colorFormat,
                          MTL::PixelFormat depthFormat, NS::UInteger width, NS::UInteger height,
                          const std::string& shaderPath)
    : _device(device)
    , _width(width)
    , _height(height)
    , _colorFormat(colorFormat)
    , _depthFormat(depthFormat)
    , _colorTarget(nullptr)
    , _depthTarget(nullptr)
    , _normalTarget(nullptr)
    , _shadowMap(nullptr)
    , _aoTarget(nullptr)
    , _bloomTarget(nullptr)
    , _terrainPipeline(nullptr)
    , _shadowPipeline(nullptr)
    , _compositePipeline(nullptr)
    , _fxaaPipeline(nullptr)
    , _ssaoPipeline(nullptr)
    , _bloomThresholdPipeline(nullptr)
    , _bloomBlurHPipeline(nullptr)
    , _bloomBlurVPipeline(nullptr)
    , _depthWriteState(nullptr)
    , _depthReadState(nullptr)
    , _noDepthState(nullptr)
    , _linearSampler(nullptr)
    , _nearestSampler(nullptr)
    , _shadowSampler(nullptr)
    , _cameraBuffer(nullptr)
    , _lightBuffer(nullptr)
    , _frameTime(0.0f)
{
    _commandQueue = _device->newCommandQueue();
    
    _bloomBlurTargets[0] = nullptr;
    _bloomBlurTargets[1] = nullptr;
    
    createPipelines(shaderPath);
    createRenderTargets();
    createSamplers();
    
    _cameraBuffer = _device->newBuffer(sizeof(GPU::CameraData), MTL::ResourceStorageModeShared);
    _lightBuffer = _device->newBuffer(sizeof(GPU::LightData), MTL::ResourceStorageModeShared);
    
    _postParams.screenSize = simd::make_float2(width, height);
}

RenderSystem::~RenderSystem() {
    if (_commandQueue) _commandQueue->release();
    if (_shaderLibrary) _shaderLibrary->release();
    
    if (_colorTarget) _colorTarget->release();
    if (_depthTarget) _depthTarget->release();
    if (_normalTarget) _normalTarget->release();
    if (_shadowMap) _shadowMap->release();
    if (_aoTarget) _aoTarget->release();
    if (_bloomTarget) _bloomTarget->release();
    if (_bloomBlurTargets[0]) _bloomBlurTargets[0]->release();
    if (_bloomBlurTargets[1]) _bloomBlurTargets[1]->release();
    
    if (_terrainPipeline) _terrainPipeline->release();
    if (_shadowPipeline) _shadowPipeline->release();
    if (_compositePipeline) _compositePipeline->release();
    if (_fxaaPipeline) _fxaaPipeline->release();
    if (_ssaoPipeline) _ssaoPipeline->release();
    if (_bloomThresholdPipeline) _bloomThresholdPipeline->release();
    if (_bloomBlurHPipeline) _bloomBlurHPipeline->release();
    if (_bloomBlurVPipeline) _bloomBlurVPipeline->release();
    
    if (_depthWriteState) _depthWriteState->release();
    if (_depthReadState) _depthReadState->release();
    if (_noDepthState) _noDepthState->release();
    
    if (_linearSampler) _linearSampler->release();
    if (_nearestSampler) _nearestSampler->release();
    if (_shadowSampler) _shadowSampler->release();
    
    if (_cameraBuffer) _cameraBuffer->release();
    if (_lightBuffer) _lightBuffer->release();
}

void RenderSystem::createRenderTargets() {
    // Release existing
    if (_colorTarget) _colorTarget->release();
    if (_depthTarget) _depthTarget->release();
    if (_normalTarget) _normalTarget->release();
    if (_aoTarget) _aoTarget->release();
    if (_bloomTarget) _bloomTarget->release();
    if (_bloomBlurTargets[0]) _bloomBlurTargets[0]->release();
    if (_bloomBlurTargets[1]) _bloomBlurTargets[1]->release();
    
    // Color target (HDR)
    MTL::TextureDescriptor* colorDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA16Float, _width, _height, false);
    colorDesc->setStorageMode(MTL::StorageModePrivate);
    colorDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    _colorTarget = _device->newTexture(colorDesc);
    
    // Depth target
    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::texture2DDescriptor(_depthFormat, _width, _height, false);
    depthDesc->setStorageMode(MTL::StorageModePrivate);
    depthDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    _depthTarget = _device->newTexture(depthDesc);
    
    // Normal target
    MTL::TextureDescriptor* normalDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA8Snorm, _width, _height, false);
    normalDesc->setStorageMode(MTL::StorageModePrivate);
    normalDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    _normalTarget = _device->newTexture(normalDesc);
    
    // Shadow map
    MTL::TextureDescriptor* shadowDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatDepth32Float, OfficialConfig::SHADOW_MAP_SIZE, OfficialConfig::SHADOW_MAP_SIZE, false);
    shadowDesc->setStorageMode(MTL::StorageModePrivate);
    shadowDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    _shadowMap = _device->newTexture(shadowDesc);
    
    // AO target
    MTL::TextureDescriptor* aoDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatR8Unorm, _width, _height, false);
    aoDesc->setStorageMode(MTL::StorageModePrivate);
    aoDesc->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
    _aoTarget = _device->newTexture(aoDesc);
    
    // Bloom targets (half resolution)
    NS::UInteger bloomWidth = _width / 2;
    NS::UInteger bloomHeight = _height / 2;
    
    MTL::TextureDescriptor* bloomDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA16Float, bloomWidth, bloomHeight, false);
    bloomDesc->setStorageMode(MTL::StorageModePrivate);
    bloomDesc->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
    
    _bloomTarget = _device->newTexture(bloomDesc);
    _bloomBlurTargets[0] = _device->newTexture(bloomDesc);
    _bloomBlurTargets[1] = _device->newTexture(bloomDesc);
}

void RenderSystem::createPipelines(const std::string& shaderPath)
{
    NS::Error* error = nullptr;
    
    // Load shader library
    NS::String* path = NS::String::string(shaderPath.c_str(), NS::UTF8StringEncoding);
    _shaderLibrary = _device->newLibrary(path, &error);
    
    if (!_shaderLibrary) {
        // Try loading from source
        // Note: In production, use pre-compiled metallib
        return;
    }
    
    // Terrain pipeline
    {
        MTL::Function* vertexFunc = _shaderLibrary->newFunction(
            NS::String::string("terrain_vertex", NS::UTF8StringEncoding));
        MTL::Function* fragmentFunc = _shaderLibrary->newFunction(
            NS::String::string("terrain_fragment", NS::UTF8StringEncoding));
        
        if (vertexFunc && fragmentFunc) {
            MTL::RenderPipelineDescriptor* desc = MTL::RenderPipelineDescriptor::alloc()->init();
            desc->setVertexFunction(vertexFunc);
            desc->setFragmentFunction(fragmentFunc);
            desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatRGBA16Float);
            desc->setDepthAttachmentPixelFormat(_depthFormat);
            
            // Vertex descriptor
            MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();
            
            // Position
            vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
            vertexDesc->attributes()->object(0)->setOffset(0);
            vertexDesc->attributes()->object(0)->setBufferIndex(0);
            
            // Normal
            vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
            vertexDesc->attributes()->object(1)->setOffset(16);
            vertexDesc->attributes()->object(1)->setBufferIndex(0);
            
            // Tangent
            vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat4);
            vertexDesc->attributes()->object(2)->setOffset(32);
            vertexDesc->attributes()->object(2)->setBufferIndex(0);
            
            // UV
            vertexDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat2);
            vertexDesc->attributes()->object(3)->setOffset(48);
            vertexDesc->attributes()->object(3)->setBufferIndex(0);
            
            // UV2
            vertexDesc->attributes()->object(4)->setFormat(MTL::VertexFormatFloat2);
            vertexDesc->attributes()->object(4)->setOffset(56);
            vertexDesc->attributes()->object(4)->setBufferIndex(0);
            
            vertexDesc->layouts()->object(0)->setStride(sizeof(GPU::Vertex));
            vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
            
            desc->setVertexDescriptor(vertexDesc);
            
            _terrainPipeline = _device->newRenderPipelineState(desc, &error);
            
            vertexDesc->release();
            desc->release();
            vertexFunc->release();
            fragmentFunc->release();
        }
    }
    
    // Shadow pipeline
    {
        MTL::Function* vertexFunc = _shaderLibrary->newFunction(
            NS::String::string("shadow_vertex", NS::UTF8StringEncoding));
        MTL::Function* fragmentFunc = _shaderLibrary->newFunction(
            NS::String::string("shadow_fragment", NS::UTF8StringEncoding));
        
        if (vertexFunc && fragmentFunc) {
            MTL::RenderPipelineDescriptor* desc = MTL::RenderPipelineDescriptor::alloc()->init();
            desc->setVertexFunction(vertexFunc);
            desc->setFragmentFunction(fragmentFunc);
            desc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
            
            _shadowPipeline = _device->newRenderPipelineState(desc, &error);
            
            desc->release();
            vertexFunc->release();
            fragmentFunc->release();
        }
    }
    
    // Composite pipeline
    {
        MTL::Function* vertexFunc = _shaderLibrary->newFunction(
            NS::String::string("fullscreen_vertex", NS::UTF8StringEncoding));
        MTL::Function* fragmentFunc = _shaderLibrary->newFunction(
            NS::String::string("postprocess_composite", NS::UTF8StringEncoding));
        
        if (vertexFunc && fragmentFunc) {
            MTL::RenderPipelineDescriptor* desc = MTL::RenderPipelineDescriptor::alloc()->init();
            desc->setVertexFunction(vertexFunc);
            desc->setFragmentFunction(fragmentFunc);
            desc->colorAttachments()->object(0)->setPixelFormat(_colorFormat);
            
            _compositePipeline = _device->newRenderPipelineState(desc, &error);
            
            desc->release();
            vertexFunc->release();
            fragmentFunc->release();
        }
    }
    
    // Compute pipelines
    {
        MTL::Function* ssaoFunc = _shaderLibrary->newFunction(
            NS::String::string("ssao_compute", NS::UTF8StringEncoding));
        if (ssaoFunc) {
            _ssaoPipeline = _device->newComputePipelineState(ssaoFunc, &error);
            ssaoFunc->release();
        }
        
        MTL::Function* bloomThresholdFunc = _shaderLibrary->newFunction(
            NS::String::string("bloomThreshold", NS::UTF8StringEncoding));
        if (bloomThresholdFunc) {
            _bloomThresholdPipeline = _device->newComputePipelineState(bloomThresholdFunc, &error);
            bloomThresholdFunc->release();
        }
        
        MTL::Function* bloomBlurHFunc = _shaderLibrary->newFunction(
            NS::String::string("bloomBlurHorizontal", NS::UTF8StringEncoding));
        if (bloomBlurHFunc) {
            _bloomBlurHPipeline = _device->newComputePipelineState(bloomBlurHFunc, &error);
            bloomBlurHFunc->release();
        }
        
        MTL::Function* bloomBlurVFunc = _shaderLibrary->newFunction(
            NS::String::string("bloomBlurVertical", NS::UTF8StringEncoding));
        if (bloomBlurVFunc) {
            _bloomBlurVPipeline = _device->newComputePipelineState(bloomBlurVFunc, &error);
            bloomBlurVFunc->release();
        }
    }
    
    // Depth stencil states
    {
        MTL::DepthStencilDescriptor* desc = MTL::DepthStencilDescriptor::alloc()->init();
        
        desc->setDepthCompareFunction(MTL::CompareFunctionLess);
        desc->setDepthWriteEnabled(true);
        _depthWriteState = _device->newDepthStencilState(desc);
        
        desc->setDepthWriteEnabled(false);
        _depthReadState = _device->newDepthStencilState(desc);
        
        desc->setDepthCompareFunction(MTL::CompareFunctionAlways);
        _noDepthState = _device->newDepthStencilState(desc);
        
        desc->release();
    }
}

void RenderSystem::createSamplers() {
    MTL::SamplerDescriptor* desc = MTL::SamplerDescriptor::alloc()->init();
    
    // Linear sampler
    desc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    desc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    desc->setMipFilter(MTL::SamplerMipFilterLinear);
    desc->setSAddressMode(MTL::SamplerAddressModeRepeat);
    desc->setTAddressMode(MTL::SamplerAddressModeRepeat);
    _linearSampler = _device->newSamplerState(desc);
    
    // Nearest sampler
    desc->setMinFilter(MTL::SamplerMinMagFilterNearest);
    desc->setMagFilter(MTL::SamplerMinMagFilterNearest);
    _nearestSampler = _device->newSamplerState(desc);
    
    // Shadow sampler
    desc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    desc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    desc->setCompareFunction(MTL::CompareFunctionLessEqual);
    desc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    desc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    _shadowSampler = _device->newSamplerState(desc);
    
    desc->release();
}

void RenderSystem::resize(NS::UInteger width, NS::UInteger height) {
    _width = width;
    _height = height;
    _postParams.screenSize = simd::make_float2(width, height);
    createRenderTargets();
}

void RenderSystem::beginFrame() {
    _frameTime += 1.0f / 60.0f;
    _postParams.time = _frameTime;
}

void RenderSystem::render(MTL::CommandBuffer* cmd, CA::MetalDrawable* drawable,
                         TerrainManager* terrain, BiomeManager* biomes,
                         const GPU::CameraData& camera, const GPU::LightData& light) {
    // Update uniform buffers
    memcpy(_cameraBuffer->contents(), &camera, sizeof(GPU::CameraData));
    memcpy(_lightBuffer->contents(), &light, sizeof(GPU::LightData));
    
    // Shadow pass
    shadowPass(cmd, terrain, light);
    
    // Geometry pass
    geometryPass(cmd, terrain, biomes, camera, light);
    
    // SSAO pass
    ssaoPass(cmd, camera);
    
    // Bloom pass
    bloomPass(cmd);
    
    // Final composite
    compositePass(cmd, drawable->texture());
}

void RenderSystem::shadowPass(MTL::CommandBuffer* cmd, TerrainManager* terrain,
                              const GPU::LightData& light) {
    if (!_shadowPipeline || !_shadowMap) return;
    
    MTL::RenderPassDescriptor* passDesc = MTL::RenderPassDescriptor::alloc()->init();
    passDesc->depthAttachment()->setTexture(_shadowMap);
    passDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    passDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    passDesc->depthAttachment()->setClearDepth(1.0);
    
    MTL::RenderCommandEncoder* encoder = cmd->renderCommandEncoder(passDesc);
    encoder->setRenderPipelineState(_shadowPipeline);
    encoder->setDepthStencilState(_depthWriteState);
    encoder->setCullMode(MTL::CullModeFront); // Peter panning fix
    encoder->setDepthBias(0.005f, 1.0f, 0.005f);
    
    encoder->setVertexBytes(&light.lightSpaceMatrix, sizeof(simd::float4x4), 0);
    
    // Render terrain to shadow map
    // terrain->renderShadow(encoder);
    
    encoder->endEncoding();
    passDesc->release();
}

void RenderSystem::geometryPass(MTL::CommandBuffer* cmd, TerrainManager* terrain,
                                BiomeManager* biomes, const GPU::CameraData& camera,
                                const GPU::LightData& light) {
    if (!_terrainPipeline || !_colorTarget || !_depthTarget) return;
    
    MTL::RenderPassDescriptor* passDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    // Color attachment
    passDesc->colorAttachments()->object(0)->setTexture(_colorTarget);
    passDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    passDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    passDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.5, 0.7, 0.9, 1.0));
    
    // Depth attachment
    passDesc->depthAttachment()->setTexture(_depthTarget);
    passDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    passDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    passDesc->depthAttachment()->setClearDepth(1.0);
    
    MTL::RenderCommandEncoder* encoder = cmd->renderCommandEncoder(passDesc);
    encoder->setRenderPipelineState(_terrainPipeline);
    encoder->setDepthStencilState(_depthWriteState);
    encoder->setCullMode(MTL::CullModeBack);
    
    encoder->setVertexBuffer(_cameraBuffer, 0, 0);
    encoder->setVertexBuffer(_lightBuffer, 0, 2);
    
    encoder->setFragmentBuffer(_cameraBuffer, 0, 0);
    encoder->setFragmentBuffer(_lightBuffer, 0, 2);
    encoder->setFragmentBuffer(biomes->getBiomeBuffer(), 0, 3);
    
    encoder->setFragmentTexture(_shadowMap, 4);
    encoder->setFragmentSamplerState(_linearSampler, 0);
    encoder->setFragmentSamplerState(_shadowSampler, 1);
    
    terrain->render(encoder, camera);
    
    encoder->endEncoding();
    passDesc->release();
}

void RenderSystem::ssaoPass(MTL::CommandBuffer* cmd, const GPU::CameraData& camera) {
    if (!_ssaoPipeline || !_aoTarget) return;
    
    MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
    encoder->setComputePipelineState(_ssaoPipeline);
    
    encoder->setTexture(_depthTarget, 0);
    encoder->setTexture(_normalTarget, 1);
    encoder->setTexture(_aoTarget, 2);
    
    encoder->setBytes(&camera.projectionMatrix, sizeof(simd::float4x4), 0);
    encoder->setBytes(&camera.invViewProjectionMatrix, sizeof(simd::float4x4), 1);
    
    float radius = 0.5f;
    float bias = 0.025f;
    encoder->setBytes(&radius, sizeof(float), 2);
    encoder->setBytes(&bias, sizeof(float), 3);
    
    MTL::Size threadgroups(_width / 16, _height / 16, 1);
    MTL::Size threadsPerGroup(16, 16, 1);
    encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
    
    encoder->endEncoding();
}

void RenderSystem::bloomPass(MTL::CommandBuffer* cmd) {
    if (!_bloomThresholdPipeline || !_bloomBlurHPipeline || !_bloomBlurVPipeline) return;
    
    NS::UInteger bloomWidth = _width / 2;
    NS::UInteger bloomHeight = _height / 2;
    MTL::Size threadgroups(bloomWidth / 16, bloomHeight / 16, 1);
    MTL::Size threadsPerGroup(16, 16, 1);
    
    // Threshold
    {
        MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
        encoder->setComputePipelineState(_bloomThresholdPipeline);
        encoder->setTexture(_colorTarget, 0);
        encoder->setTexture(_bloomTarget, 1);
        encoder->setBytes(&_postParams, sizeof(_postParams), 0);
        encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
        encoder->endEncoding();
    }
    
    // Blur passes
    for (int i = 0; i < 3; i++) {
        // Horizontal
        {
            MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
            encoder->setComputePipelineState(_bloomBlurHPipeline);
            encoder->setTexture(i == 0 ? _bloomTarget : _bloomBlurTargets[1], 0);
            encoder->setTexture(_bloomBlurTargets[0], 1);
            encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
            encoder->endEncoding();
        }
        
        // Vertical
        {
            MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();
            encoder->setComputePipelineState(_bloomBlurVPipeline);
            encoder->setTexture(_bloomBlurTargets[0], 0);
            encoder->setTexture(_bloomBlurTargets[1], 1);
            encoder->dispatchThreadgroups(threadgroups, threadsPerGroup);
            encoder->endEncoding();
        }
    }
}

void RenderSystem::compositePass(MTL::CommandBuffer* cmd, MTL::Texture* target) {
    if (!_compositePipeline || !target) return;
    
    MTL::RenderPassDescriptor* passDesc = MTL::RenderPassDescriptor::alloc()->init();
    passDesc->colorAttachments()->object(0)->setTexture(target);
    passDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionDontCare);
    passDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    MTL::RenderCommandEncoder* encoder = cmd->renderCommandEncoder(passDesc);
    encoder->setRenderPipelineState(_compositePipeline);
    encoder->setDepthStencilState(_noDepthState);
    
    encoder->setFragmentTexture(_colorTarget, 0);
    encoder->setFragmentTexture(_bloomBlurTargets[1], 1);
    encoder->setFragmentTexture(_depthTarget, 2);
    encoder->setFragmentBytes(&_postParams, sizeof(_postParams), 0);
    encoder->setFragmentSamplerState(_linearSampler, 0);
    
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
    
    encoder->endEncoding();
    passDesc->release();
}

void RenderSystem::endFrame() {
    // Stats, profiling, etc.
}

BiomeManager::BiomeManager(MTL::Device* device)
: _device(device), _biomeBuffer(nullptr), _spawnCenter{0.0f, 0.0f}
{
    initializeBiomes();
    updateGPUBuffer();
}

BiomeManager::~BiomeManager() {
    if (_biomeBuffer) _biomeBuffer->release();
}

void BiomeManager::initializeBiomes() {
    // Spawn - Cratère AAA plat et sécurisé
    _biomes[0] = {
        .type = BiomeType::Spawn,
        .name = "Spawn Crater",
        .baseHeight = 0.0f,
        .heightVariation = 2.0f,
        .noiseFrequency = 0.005f,
        .octaves = 2,
        .persistence = 0.3f,
        .lacunarity = 2.0f,
        .groundColor = simd::make_float3(0.4f, 0.35f, 0.3f),
        .cliffColor = simd::make_float3(0.5f, 0.45f, 0.4f),
        .cliffAngleThreshold = 0.85f,
        .roughnessBase = 0.7f,
        .metallicBase = 0.0f,
        .vegetationDensity = 0.1f,
        .rockDensity = 0.3f,
        .waterLevel = -100.0f,
        .transitionWidth = 64.0f
    };
    
    // Plains
    _biomes[1] = {
        .type = BiomeType::Plains,
        .name = "Plains",
        .baseHeight = 10.0f,
        .heightVariation = 15.0f,
        .noiseFrequency = 0.008f,
        .octaves = 4,
        .persistence = 0.45f,
        .lacunarity = 2.1f,
        .groundColor = simd::make_float3(0.3f, 0.5f, 0.2f),
        .cliffColor = simd::make_float3(0.4f, 0.35f, 0.25f),
        .cliffAngleThreshold = 0.7f,
        .roughnessBase = 0.6f,
        .metallicBase = 0.0f,
        .vegetationDensity = 0.7f,
        .rockDensity = 0.1f,
        .waterLevel = 5.0f,
        .transitionWidth = 48.0f
    };
    
    // Hills
    _biomes[2] = {
        .type = BiomeType::Hills,
        .name = "Rolling Hills",
        .baseHeight = 25.0f,
        .heightVariation = 40.0f,
        .noiseFrequency = 0.012f,
        .octaves = 5,
        .persistence = 0.5f,
        .lacunarity = 2.2f,
        .groundColor = simd::make_float3(0.35f, 0.45f, 0.2f),
        .cliffColor = simd::make_float3(0.45f, 0.4f, 0.3f),
        .cliffAngleThreshold = 0.6f,
        .roughnessBase = 0.65f,
        .metallicBase = 0.0f,
        .vegetationDensity = 0.5f,
        .rockDensity = 0.25f,
        .waterLevel = 8.0f,
        .transitionWidth = 40.0f
    };
    
    // Mountains
    _biomes[3] = {
        .type = BiomeType::Mountains,
        .name = "Mountains",
        .baseHeight = 80.0f,
        .heightVariation = 120.0f,
        .noiseFrequency = 0.015f,
        .octaves = 6,
        .persistence = 0.55f,
        .lacunarity = 2.3f,
        .groundColor = simd::make_float3(0.5f, 0.5f, 0.5f),
        .cliffColor = simd::make_float3(0.6f, 0.58f, 0.55f),
        .cliffAngleThreshold = 0.4f,
        .roughnessBase = 0.8f,
        .metallicBase = 0.1f,
        .vegetationDensity = 0.1f,
        .rockDensity = 0.7f,
        .waterLevel = 20.0f,
        .transitionWidth = 56.0f
    };
    
    // Desert
    _biomes[4] = {
        .type = BiomeType::Desert,
        .name = "Desert",
        .baseHeight = 15.0f,
        .heightVariation = 35.0f,
        .noiseFrequency = 0.006f,
        .octaves = 3,
        .persistence = 0.4f,
        .lacunarity = 2.5f,
        .groundColor = simd::make_float3(0.76f, 0.6f, 0.42f),
        .cliffColor = simd::make_float3(0.65f, 0.5f, 0.35f),
        .cliffAngleThreshold = 0.5f,
        .roughnessBase = 0.9f,
        .metallicBase = 0.0f,
        .vegetationDensity = 0.02f,
        .rockDensity = 0.15f,
        .waterLevel = -50.0f,
        .transitionWidth = 60.0f
    };
    
    // Tundra
    _biomes[5] = {
        .type = BiomeType::Tundra,
        .name = "Frozen Tundra",
        .baseHeight = 5.0f,
        .heightVariation = 20.0f,
        .noiseFrequency = 0.01f,
        .octaves = 4,
        .persistence = 0.48f,
        .lacunarity = 2.0f,
        .groundColor = simd::make_float3(0.85f, 0.9f, 0.95f),
        .cliffColor = simd::make_float3(0.7f, 0.75f, 0.8f),
        .cliffAngleThreshold = 0.65f,
        .roughnessBase = 0.3f,
        .metallicBase = 0.05f,
        .vegetationDensity = 0.05f,
        .rockDensity = 0.2f,
        .waterLevel = 2.0f,
        .transitionWidth = 52.0f
    };
    
    // Forest
    _biomes[6] = {
        .type = BiomeType::Forest,
        .name = "Dense Forest",
        .baseHeight = 20.0f,
        .heightVariation = 30.0f,
        .noiseFrequency = 0.009f,
        .octaves = 5,
        .persistence = 0.52f,
        .lacunarity = 2.15f,
        .groundColor = simd::make_float3(0.2f, 0.35f, 0.15f),
        .cliffColor = simd::make_float3(0.35f, 0.3f, 0.2f),
        .cliffAngleThreshold = 0.55f,
        .roughnessBase = 0.55f,
        .metallicBase = 0.0f,
        .vegetationDensity = 0.9f,
        .rockDensity = 0.15f,
        .waterLevel = 10.0f,
        .transitionWidth = 44.0f
    };
    
    // Volcanic
    _biomes[7] = {
        .type = BiomeType::Volcanic,
        .name = "Volcanic Wastes",
        .baseHeight = 40.0f,
        .heightVariation = 80.0f,
        .noiseFrequency = 0.02f,
        .octaves = 5,
        .persistence = 0.6f,
        .lacunarity = 2.4f,
        .groundColor = simd::make_float3(0.15f, 0.1f, 0.08f),
        .cliffColor = simd::make_float3(0.3f, 0.15f, 0.1f),
        .cliffAngleThreshold = 0.45f,
        .roughnessBase = 0.85f,
        .metallicBase = 0.2f,
        .vegetationDensity = 0.0f,
        .rockDensity = 0.8f,
        .waterLevel = -100.0f,
        .transitionWidth = 72.0f
    };
}

void BiomeManager::updateGPUBuffer() {
    std::array<GPU::BiomeData, OfficialConfig::BIOME_COUNT> gpuData;
    
    for (size_t i = 0; i < OfficialConfig::BIOME_COUNT; i++) {
        gpuData[i] = {
            .groundColor = _biomes[i].groundColor,
            .heightScale = _biomes[i].heightVariation,
            .cliffColor = _biomes[i].cliffColor,
            .cliffThreshold = _biomes[i].cliffAngleThreshold,
            .noiseFrequency = _biomes[i].noiseFrequency,
            .noiseAmplitude = _biomes[i].heightVariation,
            .vegetationDensity = _biomes[i].vegetationDensity,
            .rockDensity = _biomes[i].rockDensity
        };
    }
    
    if (_biomeBuffer) _biomeBuffer->release();
    _biomeBuffer = _device->newBuffer(gpuData.data(), sizeof(gpuData), MTL::ResourceStorageModeShared);
}

bool BiomeManager::isInSpawnArea(float x, float z) const {
    float dx = x - _spawnCenter.x;
    float dz = z - _spawnCenter.y;
//    float dz = z - _spawnCenter.z;
    return (dx * dx + dz * dz) < (OfficialConfig::SPAWN_SAFE_RADIUS * OfficialConfig::SPAWN_SAFE_RADIUS);
}

float BiomeManager::getSpawnModifier(float x, float z) const {
    float dx = x - _spawnCenter.x;
    float dz = z - _spawnCenter.y;
    float dist = std::sqrt(dx * dx + dz * dz);
    
    if (dist < OfficialConfig::SPAWN_CRATER_RADIUS) {
        // Dans le cratère - forme de cuvette douce
        float t = dist / OfficialConfig::SPAWN_CRATER_RADIUS;
        float crater = 1.0f - std::cos(t * M_PI * 0.5f);
        return -OfficialConfig::SPAWN_CRATER_DEPTH * (1.0f - crater);
    } else if (dist < OfficialConfig::SPAWN_SAFE_RADIUS) {
        // Zone de transition vers terrain normal
        float t = (dist - OfficialConfig::SPAWN_CRATER_RADIUS) / (OfficialConfig::SPAWN_SAFE_RADIUS - OfficialConfig::SPAWN_CRATER_RADIUS);
        return simd::mix(0.0f, 1.0f, t * t);
    }
    return 1.0f;
}

BiomeType BiomeManager::getBiomeAt(float x, float z, uint64_t seed) const {
    if (isInSpawnArea(x, z)) {
        return BiomeType::Spawn;
    }
    
    float biomeValue = voronoiBiome(x, z, seed);
    uint32_t biomeIndex = static_cast<uint32_t>(biomeValue * (OfficialConfig::BIOME_COUNT - 1)) + 1;
    return static_cast<BiomeType>(std::min(biomeIndex, OfficialConfig::BIOME_COUNT - 1));
}

float BiomeManager::getBlendFactor(float x, float z, BiomeType biome, uint64_t seed) const {
    // Implémentation simplifiée du blend factor
    return 1.0f;
}

const BiomeDefinition& BiomeManager::getDefinition(BiomeType type) const {
    return _biomes[static_cast<size_t>(type)];
}

float BiomeManager::voronoiBiome(float x, float z, uint64_t seed) const {
    // Voronoi simplifié pour distribution des biomes
    const float cellSize = 512.0f;
    
    int cellX = static_cast<int>(std::floor(x / cellSize));
    int cellZ = static_cast<int>(std::floor(z / cellSize));
    
    float minDist = 1e10f;
    float closestValue = 0.0f;
    
    for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
            int cx = cellX + dx;
            int cz = cellZ + dz;
            
            // Hash pour position du point
            uint64_t h = seed ^ (static_cast<uint64_t>(cx) * 374761393ULL) ^ (static_cast<uint64_t>(cz) * 668265263ULL);
            h = (h ^ (h >> 13)) * 1274126177ULL;
            
            float px = (cx + (h & 0xFFFF) / 65535.0f) * cellSize;
            h >>= 16;
            float pz = (cz + (h & 0xFFFF) / 65535.0f) * cellSize;
            h >>= 16;
            float value = (h & 0xFFFF) / 65535.0f;
            
            float dist = (x - px) * (x - px) + (z - pz) * (z - pz);
            if (dist < minDist) {
                minDist = dist;
                closestValue = value;
            }
        }
    }
    
    return closestValue;
}

// Implementation
ChunkMap::ChunkMap(MTL::Device* device, Types::ChunkCoord coord)
    : _device(device)
    , _coord(coord)
    , _state(ChunkState::Unloaded)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
    , _indexCount(0)
    , _currentLOD(0)
    , _heightmap(nullptr)
    , _normalMap(nullptr)
    , _splatMap(nullptr)
    , _biomeId(0)
    , _distanceToCamera(0.0f)
{
    _lodIndexBuffers.fill(nullptr);
    _lodIndexCounts.fill(0);
    
    _bounds.min = simd::make_float3(0, 0, 0);
    _bounds.max = simd::make_float3(OfficialConfig::CHUNK_SIZE, OfficialConfig::CHUNK_HEIGHT, OfficialConfig::CHUNK_SIZE);
}

ChunkMap::~ChunkMap() {
    if (_vertexBuffer) _vertexBuffer->release();
    if (_indexBuffer) _indexBuffer->release();
    if (_heightmap) _heightmap->release();
    if (_normalMap) _normalMap->release();
    if (_splatMap) _splatMap->release();
    
    for (auto& buf : _lodIndexBuffers) {
        if (buf) buf->release();
    }
}

simd::float2 ChunkMap::getWorldPosition() const {
    return simd::make_float2(
        _coord.x * static_cast<float>(OfficialConfig::CHUNK_SIZE),
        _coord.z * static_cast<float>(OfficialConfig::CHUNK_SIZE)
    );
}

void ChunkMap::uploadMesh(const ChunkMeshData& data) {
    if (_vertexBuffer) _vertexBuffer->release();
    if (_indexBuffer) _indexBuffer->release();
    
    if (!data.vertices.empty()) {
        _vertexBuffer = _device->newBuffer(
            data.vertices.data(),
            data.vertices.size() * sizeof(GPU::Vertex),
            MTL::ResourceStorageModeShared
        );
    }
    
    if (!data.indices.empty()) {
        _indexBuffer = _device->newBuffer(
            data.indices.data(),
            data.indices.size() * sizeof(uint32_t),
            MTL::ResourceStorageModeShared
        );
        _indexCount = static_cast<uint32_t>(data.indices.size());
    }
    
    _bounds = data.bounds;
}

void ChunkMap::setHeightmap(MTL::Texture* tex) {
    if (_heightmap) _heightmap->release();
    _heightmap = tex;
    if (tex) tex->retain();
}

void ChunkMap::setNormalMap(MTL::Texture* tex) {
    if (_normalMap) _normalMap->release();
    _normalMap = tex;
    if (tex) tex->retain();
}

void ChunkMap::setSplatMap(MTL::Texture* tex) {
    if (_splatMap) _splatMap->release();
    _splatMap = tex;
    if (tex) tex->retain();
}

void ChunkMap::setCollisionData(ChunkCollisionData&& data) {
    _collisionData = std::move(data);
}

float ChunkMap::getHeightAt(float localX, float localZ) const {
    if (_collisionData.heightfield.empty()) return 0.0f;
    
    float fx = (localX / OfficialConfig::CHUNK_SIZE) * (_collisionData.resolution - 1);
    float fz = (localZ / OfficialConfig::CHUNK_SIZE) * (_collisionData.resolution - 1);
    
    int x0 = static_cast<int>(fx);
    int z0 = static_cast<int>(fz);
    int x1 = std::min(x0 + 1, static_cast<int>(_collisionData.resolution - 1));
    int z1 = std::min(z0 + 1, static_cast<int>(_collisionData.resolution - 1));
    
    float tx = fx - x0;
    float tz = fz - z0;
    
    float h00 = _collisionData.heightfield[z0 * _collisionData.resolution + x0];
    float h10 = _collisionData.heightfield[z0 * _collisionData.resolution + x1];
    float h01 = _collisionData.heightfield[z1 * _collisionData.resolution + x0];
    float h11 = _collisionData.heightfield[z1 * _collisionData.resolution + x1];
    
    float h0 = simd::mix(h00, h10, tx);
    float h1 = simd::mix(h01, h11, tx);
    
    return simd::mix(h0, h1, tz);
}

simd::float3 ChunkMap::getNormalAt(float localX, float localZ) const {
    const float epsilon = 0.5f;
    
    float hL = getHeightAt(localX - epsilon, localZ);
    float hR = getHeightAt(localX + epsilon, localZ);
    float hD = getHeightAt(localX, localZ - epsilon);
    float hU = getHeightAt(localX, localZ + epsilon);
    
    simd::float3 normal = simd::make_float3(hL - hR, 2.0f * epsilon, hD - hU);
    return simd::normalize(normal);
}

void ChunkMap::updateDistance(simd::float3 cameraPos) {
    simd::float2 worldPos = getWorldPosition();
    simd::float2 chunkCenter = worldPos + simd::make_float2(OfficialConfig::CHUNK_SIZE * 0.5f);
    
    float dx = cameraPos.x - chunkCenter.x;
    float dz = cameraPos.z - chunkCenter.y;
    
    _distanceToCamera = std::sqrt(dx * dx + dz * dz);
}

TerrainGenerator::TerrainGenerator(MTL::Device* device, MTL::CommandQueue* queue,
                                   NoiseGenerator* noise, BiomeManager* biomes)
    : _device(device)
    , _commandQueue(queue)
    , _noiseGenerator(noise)
    , _biomeManager(biomes)
    , _running(true)
    , _heightGenPipeline(nullptr)
    , _normalGenPipeline(nullptr)
    , _meshGenPipeline(nullptr)
    , _currentLOD(0)
    , _pendingCount(0)
    , _generatedCount(0)
{
    // Démarrer les workers
    uint32_t numWorkers = std::max(2u, std::thread::hardware_concurrency() / 2);
    for (uint32_t i = 0; i < numWorkers; i++) {
        _workers.emplace_back(&TerrainGenerator::workerThread, this);
    }
}

TerrainGenerator::~TerrainGenerator() {
    _running = false;
    _queueCondition.notify_all();
    
    for (auto& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    if (_heightGenPipeline) _heightGenPipeline->release();
    if (_normalGenPipeline) _normalGenPipeline->release();
    if (_meshGenPipeline) _meshGenPipeline->release();
}

void TerrainGenerator::requestChunk(Types::ChunkCoord coord, uint32_t priority,
                                    std::function<void(std::shared_ptr<ChunkMap>)> callback) {
    std::lock_guard<std::mutex> lock(_queueMutex);
    _requestQueue.push({ coord, priority, std::move(callback) });
    _pendingCount++;
    _queueCondition.notify_one();
}

void TerrainGenerator::cancelRequest(Types::ChunkCoord coord) {
    // Note: simplification - dans une vraie implémentation, on filtrerait la queue
}

void TerrainGenerator::workerThread() {
    while (_running) {
        TerrainGenerationRequest request;
        
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _queueCondition.wait(lock, [this] {
                return !_running || !_requestQueue.empty();
            });
            
            if (!_running && _requestQueue.empty()) return;
            
            request = _requestQueue.top();
            _requestQueue.pop();
        }
        
        generateChunk(request);
        _pendingCount--;
        _generatedCount++;
    }
}

void TerrainGenerator::generateChunk(const TerrainGenerationRequest& request) {
    auto chunk = std::make_shared<ChunkMap>(_device, request.coord);
    chunk->setState(ChunkState::Generating);
    
    // Déterminer le biome
    simd::float2 worldPos = chunk->getWorldPosition();
    simd::float2 center = worldPos + simd::make_float2(OfficialConfig::CHUNK_SIZE * 0.5f);
    BiomeType biome = _biomeManager->getBiomeAt(center.x, center.y, _noiseGenerator->getSeed());
    chunk->setBiomeId(static_cast<uint32_t>(biome));
    
    const BiomeDefinition& biomeDef = _biomeManager->getDefinition(biome);
    
    // Générer heightfield pour collision
    uint32_t resolution = OfficialConfig::CHUNK_SIZE + 1;
    std::vector<float> heights(resolution * resolution);
    
    // Génération CPU du heightfield (peut être remplacé par GPU)
    uint64_t seed = _noiseGenerator->getSeed();
    
    for (uint32_t z = 0; z < resolution; z++) {
        for (uint32_t x = 0; x < resolution; x++) {
            float wx = worldPos.x + x;
            float wz = worldPos.y + z;
            
            // Noise multi-octave
            float height = biomeDef.baseHeight;
            float amplitude = biomeDef.heightVariation;
            float frequency = biomeDef.noiseFrequency;
            
            for (int o = 0; o < biomeDef.octaves; o++) {
                // Simplex noise approximé
                float nx = wx * frequency;
                float nz = wz * frequency;
                
                // Hash-based noise
                int ix = static_cast<int>(std::floor(nx));
                int iz = static_cast<int>(std::floor(nz));
                float fx = nx - ix;
                float fz = nz - iz;
                
                auto hash = [seed](int x, int z) -> float {
                    uint64_t h = seed ^ (x * 374761393ULL) ^ (z * 668265263ULL);
                    h = (h ^ (h >> 13)) * 1274126177ULL;
                    return (h & 0xFFFFFF) / static_cast<float>(0xFFFFFF) * 2.0f - 1.0f;
                };
                
                float u = fx * fx * (3.0f - 2.0f * fx);
                float v = fz * fz * (3.0f - 2.0f * fz);
                
                float n00 = hash(ix, iz);
                float n10 = hash(ix + 1, iz);
                float n01 = hash(ix, iz + 1);
                float n11 = hash(ix + 1, iz + 1);
                
                float nx0 = simd::mix(n00, n10, u);
                float nx1 = simd::mix(n01, n11, u);
                float noise = simd::mix(nx0, nx1, v);
                
                height += noise * amplitude;
                amplitude *= biomeDef.persistence;
                frequency *= biomeDef.lacunarity;
            }
            
            // Appliquer modificateur de spawn
            float spawnMod = _biomeManager->getSpawnModifier(wx, wz);
            if (spawnMod < 1.0f) {
                height = simd::mix(biomeDef.baseHeight + spawnMod * OfficialConfig::SPAWN_CRATER_DEPTH,
                                  height, spawnMod);
            }
            
            heights[z * resolution + x] = height;
        }
    }
    
    // Construire le mesh
    chunk->setState(ChunkState::MeshBuilding);
    ChunkMeshData meshData = buildMeshCPU(request.coord, _currentLOD, heights);
    chunk->uploadMesh(meshData);
    
    // Stocker collision data
    ChunkCollisionData collisionData;
    collisionData.heightfield = std::move(heights);
    collisionData.resolution = resolution;
    
    // Calculer min/max height
    collisionData.minHeight = *std::min_element(collisionData.heightfield.begin(),
                                                collisionData.heightfield.end());
    collisionData.maxHeight = *std::max_element(collisionData.heightfield.begin(),
                                                collisionData.heightfield.end());
    
    chunk->setCollisionData(std::move(collisionData));
    
    chunk->setState(ChunkState::Ready);
    
    // Callback sur main thread
    if (request.callback) {
        dispatch_async(dispatch_get_main_queue(), ^{
            request.callback(chunk);
        });
    }
}

ChunkMeshData TerrainGenerator::buildMeshCPU(Types::ChunkCoord coord, uint32_t lod,
                                             const std::vector<float>& heights) {
    ChunkMeshData data;
    
    uint32_t step = 1 << lod;
    uint32_t resolution = OfficialConfig::CHUNK_SIZE / step + 1;
    uint32_t srcResolution = OfficialConfig::CHUNK_SIZE + 1;
    
    simd::float2 worldPos = simd::make_float2(
        coord.x * static_cast<float>(OfficialConfig::CHUNK_SIZE),
        coord.z * static_cast<float>(OfficialConfig::CHUNK_SIZE)
    );
    
    data.bounds.min = simd::make_float3(worldPos.x, 1e10f, worldPos.y);
    data.bounds.max = simd::make_float3(worldPos.x + OfficialConfig::CHUNK_SIZE, -1e10f,
                                        worldPos.y + OfficialConfig::CHUNK_SIZE);
    
    // Vertices
    data.vertices.reserve(resolution * resolution);
    
    for (uint32_t z = 0; z < resolution; z++) {
        for (uint32_t x = 0; x < resolution; x++) {
            uint32_t srcX = x * step;
            uint32_t srcZ = z * step;
            
            float height = heights[srcZ * srcResolution + srcX];
            
            GPU::Vertex vertex;
            vertex.position = simd::make_float3(
                worldPos.x + srcX,
                height,
                worldPos.y + srcZ
            );
            
            // Calculer normale
            float hL = (srcX > 0) ? heights[srcZ * srcResolution + srcX - 1] : height;
            float hR = (srcX < OfficialConfig::CHUNK_SIZE) ? heights[srcZ * srcResolution + srcX + 1] : height;
            float hD = (srcZ > 0) ? heights[(srcZ - 1) * srcResolution + srcX] : height;
            float hU = (srcZ < OfficialConfig::CHUNK_SIZE) ? heights[(srcZ + 1) * srcResolution + srcX] : height;
            
            vertex.normal = simd::normalize(simd::make_float3(hL - hR, 2.0f, hD - hU));
            
            // Tangent
            vertex.tangent = simd::make_float4(1.0f, 0.0f, 0.0f, 1.0f);
            
            // UVs
            vertex.uv = simd::make_float2(
                static_cast<float>(srcX) / OfficialConfig::CHUNK_SIZE,
                static_cast<float>(srcZ) / OfficialConfig::CHUNK_SIZE
            );
            vertex.uv2 = simd::make_float2(
                vertex.position.x / 64.0f,
                vertex.position.z / 64.0f
            );
            
            data.vertices.push_back(vertex);
            
            // Update bounds
            data.bounds.min.y = std::min(data.bounds.min.y, height);
            data.bounds.max.y = std::max(data.bounds.max.y, height);
        }
    }
    
    // Indices
    data.indices.reserve((resolution - 1) * (resolution - 1) * 6);
    
    for (uint32_t z = 0; z < resolution - 1; z++) {
        for (uint32_t x = 0; x < resolution - 1; x++) {
            uint32_t i00 = z * resolution + x;
            uint32_t i10 = i00 + 1;
            uint32_t i01 = i00 + resolution;
            uint32_t i11 = i01 + 1;
            
            data.indices.push_back(i00);
            data.indices.push_back(i01);
            data.indices.push_back(i10);
            
            data.indices.push_back(i10);
            data.indices.push_back(i01);
            data.indices.push_back(i11);
        }
    }
    
    return data;
}

TerrainManager::TerrainManager(MTL::Device* device, MTL::CommandQueue* queue,
                               NoiseGenerator* noise, BiomeManager* biomes)
    : _device(device)
    , _generator(new TerrainGenerator(device, queue, noise, biomes))
    , _lastCameraPosition(simd::make_float3(0.0f))
    , _visibleChunkCount(0)
    , _terrainPipeline(nullptr)
    , _depthState(nullptr)
{
}

TerrainManager::~TerrainManager() {
    delete _generator;
    if (_terrainPipeline) _terrainPipeline->release();
    if (_depthState) _depthState->release();
}

void TerrainManager::update(simd::float3 cameraPosition, float deltaTime) {
    _lastCameraPosition = cameraPosition;
    
    updateChunkLoading(cameraPosition);
    updateChunkLODs(cameraPosition);
    unloadDistantChunks(cameraPosition);
}

void TerrainManager::updateChunkLoading(simd::float3 cameraPosition) {
    Types::ChunkCoord cameraChunk = {
        static_cast<int32_t>(std::floor(cameraPosition.x / OfficialConfig::CHUNK_SIZE)),
        static_cast<int32_t>(std::floor(cameraPosition.z / OfficialConfig::CHUNK_SIZE))
    };
    
    int32_t viewDist = static_cast<int32_t>(OfficialConfig::VIEW_DISTANCE_CHUNKS);
    
    for (int32_t dz = -viewDist; dz <= viewDist; dz++) {
        for (int32_t dx = -viewDist; dx <= viewDist; dx++) {
            Types::ChunkCoord coord = {
                cameraChunk.x + dx,
                cameraChunk.z + dz
            };
            
            // Vérifier distance circulaire
            float dist = std::sqrt(static_cast<float>(dx * dx + dz * dz));
            if (dist > viewDist) continue;
            
            uint64_t hash = coord.hash();
            
            std::lock_guard<std::mutex> lock(_chunksMutex);
            
            // Déjà chargé ou en cours?
            if (_chunks.find(coord) != _chunks.end()) continue;
            if (_pendingChunks.find(hash) != _pendingChunks.end()) continue;
            
            // Limite de chunks
            if (_chunks.size() + _pendingChunks.size() >= OfficialConfig::MAX_LOADED_CHUNKS) continue;
            
            _pendingChunks.insert(hash);
            
            uint32_t priority = calculatePriority(coord, cameraPosition);
            
            _generator->requestChunk(coord, priority, [this, hash](std::shared_ptr<ChunkMap> chunk) {
                onChunkGenerated(chunk);
                std::lock_guard<std::mutex> lock(_chunksMutex);
                _pendingChunks.erase(hash);
            });
        }
    }
}

void TerrainManager::updateChunkLODs(simd::float3 cameraPosition) {
    std::lock_guard<std::mutex> lock(_chunksMutex);
    
    for (auto& [coord, chunk] : _chunks) {
        if (chunk->getState() != ChunkState::Ready) continue;
        
        chunk->updateDistance(cameraPosition);
        uint32_t newLOD = calculateLOD(chunk->getDistanceToCamera());
        
        if (newLOD != chunk->getLODLevel()) {
            chunk->setLODLevel(newLOD);
            // Optionnel: régénérer mesh avec nouveau LOD
        }
    }
}

void TerrainManager::unloadDistantChunks(simd::float3 cameraPosition) {
    float maxDist = OfficialConfig::VIEW_DISTANCE_CHUNKS * OfficialConfig::CHUNK_SIZE * 1.5f;
    
    std::lock_guard<std::mutex> lock(_chunksMutex);
    
    for (auto it = _chunks.begin(); it != _chunks.end(); ) {
        if (it->second->getDistanceToCamera() > maxDist) {
            it = _chunks.erase(it);
        } else {
            ++it;
        }
    }
}

uint32_t TerrainManager::calculateLOD(float distance) const {
    for (uint32_t i = 0; i < OfficialConfig::LOD_LEVELS; i++) {
        if (distance < OfficialConfig::LOD_DISTANCES[i]) {
            return i;
        }
    }
    return OfficialConfig::LOD_LEVELS - 1;
}

uint32_t TerrainManager::calculatePriority(Types::ChunkCoord coord, simd::float3 cameraPos) const {
    float dx = (coord.x + 0.5f) * OfficialConfig::CHUNK_SIZE - cameraPos.x;
    float dz = (coord.z + 0.5f) * OfficialConfig::CHUNK_SIZE - cameraPos.z;
    return static_cast<uint32_t>(std::sqrt(dx * dx + dz * dz));
}

void TerrainManager::onChunkGenerated(std::shared_ptr<ChunkMap> chunk) {
    std::lock_guard<std::mutex> lock(_chunksMutex);
    _chunks[chunk->getCoord()] = chunk;
}

void TerrainManager::render(MTL::RenderCommandEncoder* encoder, const GPU::CameraData& camera) {
    if (!_terrainPipeline) return;
    
    encoder->setRenderPipelineState(_terrainPipeline);
    encoder->setDepthStencilState(_depthState);
    
    _visibleChunkCount = 0;
    
    std::lock_guard<std::mutex> lock(_chunksMutex);
    
    for (auto& [coord, chunk] : _chunks) {
        if (chunk->getState() != ChunkState::Ready) continue;
        if (!chunk->getVertexBuffer()) continue;
        
        // Frustum culling basique
        // TODO: implémenter frustum culling complet
        
        GPU::TerrainPushConstants constants = {
            .chunkWorldPos = chunk->getWorldPosition(),
            .lodScale = static_cast<float>(1 << chunk->getLODLevel()),
            .biomeId = chunk->getBiomeId(),
            .lodLevel = chunk->getLODLevel(),
            .morphFactor = 0.0f
        };
        
        encoder->setVertexBuffer(chunk->getVertexBuffer(), 0, 0);
        encoder->setVertexBytes(&constants, sizeof(constants), 1);
        encoder->setFragmentBytes(&constants, sizeof(constants), 1);
        
        if (chunk->getHeightmap()) {
            encoder->setFragmentTexture(chunk->getHeightmap(), 0);
        }
        if (chunk->getNormalMap()) {
            encoder->setFragmentTexture(chunk->getNormalMap(), 1);
        }
        
        encoder->drawIndexedPrimitives(
            MTL::PrimitiveTypeTriangle,
            chunk->getIndexCount(),
            MTL::IndexTypeUInt32,
            chunk->getIndexBuffer(),
            0
        );
        
        _visibleChunkCount++;
    }
}

float TerrainManager::getHeightAt(float x, float z) const {
    ChunkMap* chunk = getChunkAt(x, z);
    if (!chunk || chunk->getState() != ChunkState::Ready) return 0.0f;
    
    simd::float2 worldPos = chunk->getWorldPosition();
    float localX = x - worldPos.x;
    float localZ = z - worldPos.y;
    
    return chunk->getHeightAt(localX, localZ);
}

simd::float3 TerrainManager::getNormalAt(float x, float z) const {
    ChunkMap* chunk = getChunkAt(x, z);
    if (!chunk || chunk->getState() != ChunkState::Ready) {
        return simd::make_float3(0.0f, 1.0f, 0.0f);
    }
    
    simd::float2 worldPos = chunk->getWorldPosition();
    float localX = x - worldPos.x;
    float localZ = z - worldPos.y;
    
    return chunk->getNormalAt(localX, localZ);
}

ChunkMap* TerrainManager::getChunkAt(float x, float z) const {
    Types::ChunkCoord coord = {
        static_cast<int32_t>(std::floor(x / OfficialConfig::CHUNK_SIZE)),
        static_cast<int32_t>(std::floor(z / OfficialConfig::CHUNK_SIZE))
    };
    return getChunk(coord);
}

ChunkMap* TerrainManager::getChunk(Types::ChunkCoord coord) const {
    std::lock_guard<std::mutex> lock(_chunksMutex);
    auto it = _chunks.find(coord);
    if (it != _chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool TerrainManager::raycast(const Types::Ray& ray, Types::RayHit& hit, float maxDistance) const {
    hit.hit = false;
    hit.distance = maxDistance;
    
    // Raymarching simple
    float step = 0.5f;
    simd::float3 pos = ray.origin;
    
    for (float t = 0.0f; t < maxDistance; t += step) {
        pos = ray.origin + ray.direction * t;
        float terrainHeight = getHeightAt(pos.x, pos.z);
        
        if (pos.y < terrainHeight) {
            // Binary search pour précision
            float tMin = t - step;
            float tMax = t;
            
            for (int i = 0; i < 8; i++) {
                float tMid = (tMin + tMax) * 0.5f;
                simd::float3 midPos = ray.origin + ray.direction * tMid;
                float midHeight = getHeightAt(midPos.x, midPos.z);
                
                if (midPos.y < midHeight) {
                    tMax = tMid;
                } else {
                    tMin = tMid;
                }
            }
            
            hit.hit = true;
            hit.distance = (tMin + tMax) * 0.5f;
            hit.position = ray.origin + ray.direction * hit.distance;
            hit.normal = getNormalAt(hit.position.x, hit.position.z);
            hit.materialId = 0;
            return true;
        }
        
        // Adapter le step selon la hauteur au-dessus du terrain
        float heightAbove = pos.y - terrainHeight;
        step = std::max(0.5f, std::min(heightAbove * 0.5f, 10.0f));
    }
    
    return false;
}

PhysicsSystem::PhysicsSystem(TerrainManager* terrain)
    : _terrain(terrain)
    , _gravity(simd::make_float3(0.0f, OfficialConfig::GRAVITY, 0.0f))
    , _timeAccumulator(0.0f)
    , _nextId(1)
{
    _bodies.reserve(1024);
}

PhysicsSystem::~PhysicsSystem() {
}

void PhysicsSystem::update(float deltaTime) {
    _timeAccumulator += deltaTime;
    
    uint32_t substeps = 0;
    while (_timeAccumulator >= OfficialConfig::PHYSICS_TIMESTEP && substeps < OfficialConfig::MAX_PHYSICS_SUBSTEPS) {
        fixedUpdate();
        _timeAccumulator -= OfficialConfig::PHYSICS_TIMESTEP;
        substeps++;
    }
}

void PhysicsSystem::fixedUpdate() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    const float dt = OfficialConfig::PHYSICS_TIMESTEP;
    
    // Appliquer gravité et intégrer vélocités
    integrateVelocities(dt);
    
    // Détecter collisions
    detectCollisions();
    
    // Résoudre collisions
    resolveCollisions();
    
    // Intégrer positions
    integratePositions(dt);
    
    // Clear forces
    for (auto& body : _bodies) {
        if (!body.active) continue;
        body.accumulatedForce = simd::make_float3(0.0f);
        body.accumulatedTorque = simd::make_float3(0.0f);
    }
}

void PhysicsSystem::integrateVelocities(float dt) {
    for (auto& body : _bodies) {
        if (!body.active || body.isStatic) continue;
        
        // Gravité
        if (body.useGravity) {
            body.linearVelocity += _gravity * dt;
        }
        
        // Forces accumulées
        body.linearVelocity += body.accumulatedForce * body.invMass * dt;
        body.angularVelocity += body.accumulatedTorque * body.invInertia * dt;
        
        // Damping
        body.linearVelocity *= (1.0f - body.linearDamping * dt);
        body.angularVelocity *= (1.0f - body.angularDamping * dt);
    }
}

void PhysicsSystem::integratePositions(float dt) {
    for (auto& body : _bodies) {
        if (!body.active || body.isStatic) continue;
        
        body.position += body.linearVelocity * dt;
        
        // Rotation via quaternion
        if (simd::length(body.angularVelocity) > 0.001f) {
            float angle = simd::length(body.angularVelocity) * dt;
            simd::float3 axis = simd::normalize(body.angularVelocity);
            simd::quatf deltaRot = simd::quatf(angle, axis);
            body.rotation = simd::normalize(deltaRot * body.rotation);
        }
        
        // Update bounds
        body.bounds.min = body.position - simd::make_float3(body.radius);
        body.bounds.max = body.position + simd::make_float3(body.radius);
    }
}

void PhysicsSystem::detectCollisions() {
    _contacts.clear();
    
    for (auto& body : _bodies) {
        if (!body.active || body.isStatic) continue;
        
        // Collision avec terrain
        collideWithTerrain(body);
        
        // TODO: Collision body-body avec broad phase
    }
}

void PhysicsSystem::collideWithTerrain(PhysicsBody& body) {
    body.onGround = false;
    
    if (!_terrain) return;
    
    float terrainHeight = _terrain->getHeightAt(body.position.x, body.position.z);
    float penetration = (terrainHeight + body.radius) - body.position.y;
    
    if (penetration > 0.0f) {
        simd::float3 normal = _terrain->getNormalAt(body.position.x, body.position.z);
        
        CollisionContact contact;
        contact.bodyA = body.id;
        contact.bodyB = 0; // Terrain
        contact.point = body.position - simd::make_float3(0.0f, body.radius, 0.0f);
        contact.normal = normal;
        contact.penetration = penetration;
        
        _contacts.push_back(contact);
        
        body.onGround = true;
        body.groundNormal = normal;
    }
}

void PhysicsSystem::resolveCollisions() {
    for (const auto& contact : _contacts) {
        PhysicsBody* bodyA = getBody(contact.bodyA);
        if (!bodyA) continue;
        
        // Terrain collision (bodyB = 0)
        if (contact.bodyB == 0) {
            // Position correction
            bodyA->position += contact.normal * contact.penetration;
            
            // Velocity reflection
            float vn = simd::dot(bodyA->linearVelocity, contact.normal);
            
            if (vn < 0.0f) {
                simd::float3 vNormal = contact.normal * vn;
                simd::float3 vTangent = bodyA->linearVelocity - vNormal;
                
                // Restitution (bounce)
                vNormal *= -bodyA->restitution;
                
                // Friction
                float frictionCoeff = bodyA->friction;
                if (simd::length(vTangent) > 0.001f) {
                    vTangent *= std::max(0.0f, 1.0f - frictionCoeff);
                }
                
                bodyA->linearVelocity = vNormal + vTangent;
            }
        }
    }
}

uint32_t PhysicsSystem::createBody() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    uint32_t id;
    
    if (!_freeIds.empty()) {
        id = _freeIds.back();
        _freeIds.pop_back();
    } else {
        id = _nextId++;
        _bodies.emplace_back();
    }
    
    PhysicsBody& body = _bodies[id - 1];
    body.id = id;
    body.active = true;
    body.isStatic = false;
    body.useGravity = true;
    body.position = simd::make_float3(0.0f);
    body.rotation = simd::quatf(0.0f, simd::make_float3(0.0f, 1.0f, 0.0f));
    body.scale = simd::make_float3(1.0f);
    body.linearVelocity = simd::make_float3(0.0f);
    body.angularVelocity = simd::make_float3(0.0f);
    body.mass = 1.0f;
    body.invMass = 1.0f;
    body.inertia = simd::make_float3(1.0f);
    body.invInertia = simd::make_float3(1.0f);
    body.friction = 0.5f;
    body.restitution = 0.3f;
    body.linearDamping = 0.1f;
    body.angularDamping = 0.1f;
    body.radius = 0.5f;
    body.onGround = false;
    body.groundNormal = simd::make_float3(0.0f, 1.0f, 0.0f);
    body.accumulatedForce = simd::make_float3(0.0f);
    body.accumulatedTorque = simd::make_float3(0.0f);
    
    return id;
}

void PhysicsSystem::destroyBody(uint32_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (id == 0 || id > _bodies.size()) return;
    
    _bodies[id - 1].active = false;
    _freeIds.push_back(id);
}

PhysicsBody* PhysicsSystem::getBody(uint32_t id) {
    if (id == 0 || id > _bodies.size()) return nullptr;
    PhysicsBody& body = _bodies[id - 1];
    return body.active ? &body : nullptr;
}

void PhysicsSystem::applyForce(uint32_t id, simd::float3 force) {
    PhysicsBody* body = getBody(id);
    if (body && !body->isStatic) {
        body->accumulatedForce += force;
    }
}

void PhysicsSystem::applyForceAtPoint(uint32_t id, simd::float3 force, simd::float3 point) {
    PhysicsBody* body = getBody(id);
    if (body && !body->isStatic) {
        body->accumulatedForce += force;
        simd::float3 r = point - body->position;
        body->accumulatedTorque += simd::cross(r, force);
    }
}

void PhysicsSystem::applyImpulse(uint32_t id, simd::float3 impulse) {
    PhysicsBody* body = getBody(id);
    if (body && !body->isStatic) {
        body->linearVelocity += impulse * body->invMass;
    }
}

void PhysicsSystem::applyTorque(uint32_t id, simd::float3 torque) {
    PhysicsBody* body = getBody(id);
    if (body && !body->isStatic) {
        body->accumulatedTorque += torque;
    }
}

bool PhysicsSystem::raycast(const Types::Ray& ray, Types::RayHit& hit, float maxDistance, uint32_t mask) const {
    hit.hit = false;
    hit.distance = maxDistance;
    
    // Raycast terrain
    Types::RayHit terrainHit;
    if (_terrain && _terrain->raycast(ray, terrainHit, maxDistance)) {
        if (terrainHit.distance < hit.distance) {
            hit = terrainHit;
        }
    }
    
    // Raycast bodies
    std::lock_guard<std::mutex> lock(_mutex);
    
    for (const auto& body : _bodies) {
        if (!body.active) continue;
        
        // Sphere intersection
        simd::float3 oc = ray.origin - body.position;
        float b = simd::dot(oc, ray.direction);
        float c = simd::dot(oc, oc) - body.radius * body.radius;
        float discriminant = b * b - c;
        
        if (discriminant > 0.0f) {
            float t = -b - std::sqrt(discriminant);
            if (t > 0.0f && t < hit.distance) {
                hit.hit = true;
                hit.distance = t;
                hit.position = ray.origin + ray.direction * t;
                hit.normal = simd::normalize(hit.position - body.position);
                hit.materialId = body.id;
            }
        }
    }
    
    return hit.hit;
}

void PhysicsSystem::overlapSphere(simd::float3 center, float radius, std::vector<uint32_t>& results) const {
    results.clear();
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    float radiusSq = radius * radius;
    
    for (const auto& body : _bodies) {
        if (!body.active) continue;
        
        simd::float3 diff = body.position - center;
        float distSq = simd::dot(diff, diff);
        float combinedRadius = radius + body.radius;
        
        if (distSq < combinedRadius * combinedRadius) {
            results.push_back(body.id);
        }
    }
}

//BiomeGen::BiomeGen(uint32_t seed)
//    : m_rng(seed)
//    , m_cellBuffer(nullptr)
//{
//    
//}
//
//BiomeType BiomeGen::determineBiomeType(float distanceFromSpawn, float randomValue)
//{
//    // Nearby spawn: gentler biomes
//    if (distanceFromSpawn < 10.0f) // 1000
//    {
//        if (randomValue < 0.6f)
//            return BiomeType::VoronoiPlains;
//        else if (randomValue < 0.9f)
//            return BiomeType::VoronoiTerrace;
//        else
//            return BiomeType::VoronoiEroded;
//    }
//    
//    // Mid-range: more variety
//    if (distanceFromSpawn < 500.0f)
//    {
//        if (randomValue < 0.2f)
//            return BiomeType::VoronoiIslands;
//        else if (randomValue < 0.4f)
//            return BiomeType::VoronoiEroded;
//        else if (randomValue < 0.6f)
//            return BiomeType::CrystalCaves;
//        else if (randomValue < 0.8f)
//            return BiomeType::VoronoiTerrace;
//        else
//            return BiomeType::Ocean;
//    }
//    
//    // Far away: extreme biomes
//    if (randomValue < 0.15f) return BiomeType::Planet;
//    else if (randomValue < 0.3f) return BiomeType::Moon;
//    else if (randomValue < 0.45f) return BiomeType::Ocean;
//    else if (randomValue < 0.6f) return BiomeType::FloatingIslands;
//    else if (randomValue < 0.75f) return BiomeType::LavaFields;
//    else if (randomValue < 0.9f) return BiomeType::IceShelf;
//    else return BiomeType::CrystalCaves;
//}
//
//void BiomeGen::generate(float worldSize, int numCells)
//{
//    m_cells.clear();
//    m_cells.reserve(numCells);
//    
//    std::uniform_real_distribution<float> posDist(-worldSize/2, worldSize/2);
//    std::uniform_real_distribution<float> radiusDist(50.0f, 150.0f);
//    std::uniform_int_distribution<int> biomeDist(0, (int)BiomeType::COUNT - 1);
//    
//    // Always create spawn crater at center
//    VoronoiCell spawnCell;
//    spawnCell.center = {0.0f, 0.0f};
//    spawnCell.biomeType = BiomeType::SpawnCrater;
//    spawnCell.radius = 100.0f;
//    spawnCell.color = {0.6f, 0.55f, 0.4f, 1.0f}; // Sandy/rocky color
//    m_cells.push_back(spawnCell);
//    
//    // Generate other cells
//    for (int i = 1; i < numCells; i++) {
//        VoronoiCell cell;
//        cell.center = {posDist(m_rng), posDist(m_rng)};
//        
//        float distFromSpawn = simd::length(cell.center);
//        
//        // Determine biome type based on distance from spawn
//        cell.biomeType = determineBiomeType(distFromSpawn, (float)biomeDist(m_rng) / (float)BiomeType::COUNT);
//        cell.radius = radiusDist(m_rng);
//        
//        // Assign color based on biome
//        switch (cell.biomeType) {
//            case BiomeType::VoronoiPlains:
//                cell.color = {0.4f, 0.7f, 0.3f, 1.0f}; // Green
//                break;
//            case BiomeType::VoronoiIslands:
//                cell.color = {0.2f, 0.5f, 0.8f, 1.0f}; // Blue-green
//                break;
//            case BiomeType::VoronoiTerrace:
//                cell.color = {0.7f, 0.6f, 0.4f, 1.0f}; // Brown
//                break;
//            case BiomeType::VoronoiEroded:
//                cell.color = {0.6f, 0.4f, 0.3f, 1.0f}; // Red-brown
//                break;
//            case BiomeType::Planet:
//                cell.color = {0.5f, 0.3f, 0.2f, 1.0f}; // Mars-like
//                break;
//            case BiomeType::Moon:
//                cell.color = {0.7f, 0.7f, 0.7f, 1.0f}; // Gray
//                break;
//            case BiomeType::Ocean:
//                cell.color = {0.1f, 0.3f, 0.6f, 1.0f}; // Deep blue
//                break;
//            case BiomeType::CrystalCaves:
//                cell.color = {0.5f, 0.7f, 0.9f, 1.0f}; // Crystal blue
//                break;
//            case BiomeType::FloatingIslands:
//                cell.color = {0.3f, 0.6f, 0.4f, 1.0f}; // Green with altitude
//                break;
//            case BiomeType::LavaFields:
//                cell.color = {0.9f, 0.3f, 0.1f, 1.0f}; // Orange-red
//                break;
//            case BiomeType::IceShelf:
//                cell.color = {0.8f, 0.9f, 1.0f, 1.0f}; // Icy white
//                break;
//            default:
//                cell.color = {0.5f, 0.5f, 0.5f, 1.0f};
//        }
//        
//        m_cells.push_back(cell);
//    }
//}
//
//BiomeType BiomeGen::getBiomeAt(const simd::float2& position) const {
//    float minDist = FLT_MAX;
//    BiomeType closestBiome = BiomeType::SpawnCrater;
//    
//    for (const auto& cell : m_cells) {
//        float dist = simd::length(position - cell.center);
//        if (dist < minDist) {
//            minDist = dist;
//            closestBiome = cell.biomeType;
//        }
//    }
//    
//    return closestBiome;
//}
//
//simd::float2 BiomeGen::getNearestCellCenter(const simd::float2& position) const {
//    float minDist = FLT_MAX;
//    simd::float2 nearest = {0, 0};
//    
//    for (const auto& cell : m_cells) {
//        float dist = simd::length(position - cell.center);
//        if (dist < minDist) {
//            minDist = dist;
//            nearest = cell.center;
//        }
//    }
//    
//    return nearest;
//}
//
//float BiomeGen::getDistanceToNearestEdge(const simd::float2& position) const {
//    float minDist1 = FLT_MAX;
//    float minDist2 = FLT_MAX;
//    
//    for (const auto& cell : m_cells) {
//        float dist = simd::length(position - cell.center);
//        
//        if (dist < minDist1) {
//            minDist2 = minDist1;
//            minDist1 = dist;
//        } else if (dist < minDist2) {
//            minDist2 = dist;
//        }
//    }
//    
//    return minDist2 - minDist1;
//}
//
//void BiomeGen::uploadToGPU(MTL::Device* device) {
//    if (m_cellBuffer) {
//        m_cellBuffer->release();
//    }
//    
//    m_cellBuffer = device->newBuffer(m_cells.data(),
//                                     m_cells.size() * sizeof(VoronoiCell),
//                                     MTL::ResourceStorageModeShared);
//}
//
//// ============================================================================
//// TerrainSystem Implementation
//// ============================================================================
//
//TerrainSystem::TerrainSystem(MTL::Device* device,
//                             MTL::PixelFormat colorFormat,
//                             MTL::PixelFormat depthFormat,
//                             MTL::Library* library)
//    : m_device(device->retain())
//    , m_library(library->retain())
//    , m_seed(89)
//    , m_heightScale(1.0f)
//    , m_maxRenderDistance(100.0f)
//    , m_lodEnabled(true)
//    , m_wireframe(false)
//    , m_time(0.0f)
//    , m_octaves(6)
//    , m_persistence(0.5f)
//    , m_lacunarity(2.0f)
//    , m_noiseScale(0.01f)
//{
//    m_uniformsBuffer = m_device->newBuffer(sizeof(TerrainUniforms),
//                                          MTL::ResourceStorageModeShared);
//    
//    createPipelines(colorFormat, depthFormat);
//    createTextures();
//    
//    initialize(m_seed);
//}
//
//TerrainSystem::~TerrainSystem() {
//    if (m_terrainPipeline) m_terrainPipeline->release();
//    if (m_depthStencilState) m_depthStencilState->release();
//    if (m_uniformsBuffer) m_uniformsBuffer->release();
//    
//    if (m_voronoiGeneratorPipeline) m_voronoiGeneratorPipeline->release();
//    if (m_heightmapGeneratorPipeline) m_heightmapGeneratorPipeline->release();
//    if (m_normalCalculatorPipeline) m_normalCalculatorPipeline->release();
//    if (m_erosionPipeline) m_erosionPipeline->release();
//    
//    if (m_heightmapTexture) m_heightmapTexture->release();
//    if (m_normalTexture) m_normalTexture->release();
//    if (m_biomeTexture) m_biomeTexture->release();
//    if (m_voronoiTexture) m_voronoiTexture->release();
//    
//    if (m_albedoArray) m_albedoArray->release();
//    if (m_normalMapArray) m_normalMapArray->release();
//    if (m_roughnessArray) m_roughnessArray->release();
//    if (m_metallicArray) m_metallicArray->release();
//    if (m_aoArray) m_aoArray->release();
//    
//    m_library->release();
//    m_device->release();
//}
//
//void TerrainSystem::createPipelines(MTL::PixelFormat colorFormat, MTL::PixelFormat depthFormat) {
//    // Terrain rendering pipeline
//    MTL::RenderPipelineDescriptor* terrainDesc = MTL::RenderPipelineDescriptor::alloc()->init();
//    MTL::Function* terrainVertFunc = m_library->newFunction(
//        NS::String::string("terrainVertex", NS::UTF8StringEncoding));
//    MTL::Function* terrainFragFunc = m_library->newFunction(
//        NS::String::string("terrainFragment", NS::UTF8StringEncoding));
//    
//    MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();
//
//    // 0 : position
//    vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
//    vertexDesc->attributes()->object(0)->setOffset(0);
//    vertexDesc->attributes()->object(0)->setBufferIndex(0);
//
//    // 1 : normal
//    vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat3);
//    vertexDesc->attributes()->object(1)->setOffset(12);
//    vertexDesc->attributes()->object(1)->setBufferIndex(0);
//
//    // 2 : tangent
//    vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
//    vertexDesc->attributes()->object(2)->setOffset(24);
//    vertexDesc->attributes()->object(2)->setBufferIndex(0);
//
//    // 3 : texCoord
//    vertexDesc->attributes()->object(3)->setFormat(MTL::VertexFormatFloat2);
//    vertexDesc->attributes()->object(3)->setOffset(36);
//    vertexDesc->attributes()->object(3)->setBufferIndex(0);
//
//    // 4 : color
//    vertexDesc->attributes()->object(4)->setFormat(MTL::VertexFormatFloat4);
//    vertexDesc->attributes()->object(4)->setOffset(44);
//    vertexDesc->attributes()->object(4)->setBufferIndex(0);
//
//    // 5 : height
//    vertexDesc->attributes()->object(5)->setFormat(MTL::VertexFormatFloat);
//    vertexDesc->attributes()->object(5)->setOffset(60);
//    vertexDesc->attributes()->object(5)->setBufferIndex(0);
//
//    // 6 : biomeId
//    vertexDesc->attributes()->object(6)->setFormat(MTL::VertexFormatFloat);
//    vertexDesc->attributes()->object(6)->setOffset(64);
//    vertexDesc->attributes()->object(6)->setBufferIndex(0);
//
//    // layout
//    vertexDesc->layouts()->object(0)->setStride(72);
//    vertexDesc->layouts()->object(0)->setStepRate(1);
//    vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
//
//    if (terrainVertFunc && terrainFragFunc) {
//        terrainDesc->setVertexDescriptor(vertexDesc);
//        terrainDesc->setVertexFunction(terrainVertFunc);
//        terrainDesc->setFragmentFunction(terrainFragFunc);
//        terrainDesc->colorAttachments()->object(0)->setPixelFormat(colorFormat);
//        terrainDesc->setDepthAttachmentPixelFormat(depthFormat);
//        
//        NS::Error* error = nullptr;
//        m_terrainPipeline = m_device->newRenderPipelineState(terrainDesc, &error);
//        if (error) {
//            printf("Terrain pipeline error: %s\n",
//                   error->localizedDescription()->utf8String());
//        }
//        
//        terrainVertFunc->release();
//        terrainFragFunc->release();
//    }
//    terrainDesc->release();
//    
//    // Depth stencil state
//    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
//    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
//    depthDesc->setDepthWriteEnabled(true);
//    m_depthStencilState = m_device->newDepthStencilState(depthDesc);
//    depthDesc->release();
//    
//    // Compute pipelines
//    NS::Error* error = nullptr;
//    
//    MTL::Function* voronoiFunc = m_library->newFunction(
//        NS::String::string("advancedVoronoiGenerator", NS::UTF8StringEncoding));
//    if (voronoiFunc) {
//        m_voronoiGeneratorPipeline = m_device->newComputePipelineState(voronoiFunc, &error);
//        voronoiFunc->release();
//    }
//    
//    MTL::Function* heightFunc = m_library->newFunction(
//        NS::String::string("voronoiHeightmapGenerator", NS::UTF8StringEncoding));
//    if (heightFunc) {
//        m_heightmapGeneratorPipeline = m_device->newComputePipelineState(heightFunc, &error);
//        heightFunc->release();
//    }
//    
//    MTL::Function* normalFunc = m_library->newFunction(
//        NS::String::string("calculateNormals", NS::UTF8StringEncoding));
//    if (normalFunc) {
//        m_normalCalculatorPipeline = m_device->newComputePipelineState(normalFunc, &error);
//        normalFunc->release();
//    }
//    
//    MTL::Function* erosionFunc = m_library->newFunction(
//        NS::String::string("thermalErosion", NS::UTF8StringEncoding));
//    if (erosionFunc) {
//        m_erosionPipeline = m_device->newComputePipelineState(erosionFunc, &error);
//        erosionFunc->release();
//    }
//}
//
//void TerrainSystem::createTextures()
//{
//    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA16Float, 2048, 2048, false);
//    texDesc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
//    texDesc->setStorageMode(MTL::StorageModePrivate);
//    
//    m_heightmapTexture = m_device->newTexture(texDesc);
//    m_normalTexture = m_device->newTexture(texDesc);
//    m_biomeTexture = m_device->newTexture(texDesc);
//    m_voronoiTexture = m_device->newTexture(texDesc);
//
//    // TODO: Create texture arrays for materials
//    // For now, create placeholder textures
//    MTL::TextureDescriptor* arrayDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA16Float, 512, 512, false);
//    arrayDesc->setTextureType(MTL::TextureType2DArray);
//    arrayDesc->setArrayLength((int)BiomeType::COUNT);
//    arrayDesc->setUsage(MTL::TextureUsageShaderRead);
//    
//    m_albedoArray = m_device->newTexture(arrayDesc);
//    m_normalMapArray = m_device->newTexture(arrayDesc);
//    m_roughnessArray = m_device->newTexture(arrayDesc);
//    m_metallicArray = m_device->newTexture(arrayDesc);
//    m_aoArray = m_device->newTexture(arrayDesc);
//}
//
//void TerrainSystem::initialize(uint32_t seed)
//{
//    m_seed = seed;
//    
//    // Generate biome map
//    m_biomeGenerator = std::make_unique<BiomeGen>(seed);
//    m_biomeGenerator->generate(2048.0f, 50); // 50 Voronoi cells
//    m_biomeGenerator->uploadToGPU(m_device);
//    
//    // Generate heightmap on GPU
//    generateHeightmapGPU();
//    
//    // Calculate normals on GPU
//    generateNormalsGPU();
//    
//    // Apply erosion
//    applyErosionGPU(5);
//    
//    // Generate initial chunks
//    generateChunkMesh(getOrCreateChunk(0, 0));
//}
//
//void TerrainSystem::generateHeightmapGPU()
//{
//    if (!m_voronoiGeneratorPipeline)
//        return;
//    
//    MTL::CommandQueue* queue = m_device->newCommandQueue();
//    MTL::CommandBuffer* cmd = queue->commandBuffer();
//    MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
//    
//    enc->setComputePipelineState(m_voronoiGeneratorPipeline);
//    enc->setTexture(m_heightmapTexture, 0);
//    enc->setTexture(m_biomeTexture, 1);
//    enc->setTexture(m_voronoiTexture, 2);
//    enc->setBuffer(m_biomeGenerator->getCellBuffer(), 0, 0);
//    
//    uint32_t numCells = (uint32_t)m_biomeGenerator->getCells().size();
//    enc->setBytes(&numCells, sizeof(uint32_t), 1);
//    enc->setBytes(&m_seed, sizeof(uint32_t), 2);
//    
//    float worldSize = 2048.0f;
//    enc->setBytes(&worldSize, sizeof(float), 3);
//    
//    MTL::Size gridSize = MTL::Size::Make(2048, 2048, 1);
//    MTL::Size threadgroupSize = MTL::Size::Make(16, 16, 1);
//    
//    enc->dispatchThreads(gridSize, threadgroupSize);
//    enc->endEncoding();
//    
//    cmd->commit();
//    cmd->waitUntilCompleted();
//    
//    queue->release();
//}
//
//void TerrainSystem::generateNormalsGPU() {
//    if (!m_normalCalculatorPipeline) return;
//    
//    MTL::CommandQueue* queue = m_device->newCommandQueue();
//    MTL::CommandBuffer* cmd = queue->commandBuffer();
//    MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
//    
//    enc->setComputePipelineState(m_normalCalculatorPipeline);
//    enc->setTexture(m_heightmapTexture, 0);
//    enc->setTexture(m_normalTexture, 1);
//    enc->setBytes(&m_heightScale, sizeof(float), 0);
//    
//    MTL::Size gridSize = MTL::Size::Make(2048, 2048, 1);
//    MTL::Size threadgroupSize = MTL::Size::Make(16, 16, 1);
//    
//    enc->dispatchThreads(gridSize, threadgroupSize);
//    enc->endEncoding();
//    
//    cmd->commit();
//    cmd->waitUntilCompleted();
//    
//    queue->release();
//}
//
//void TerrainSystem::applyErosionGPU(int iterations) {
//    if (!m_erosionPipeline) return;
//    
//    MTL::CommandQueue* queue = m_device->newCommandQueue();
//    
//    // Create temp texture for ping-pong
//    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(
//        MTL::PixelFormatRGBA16Float, 2048, 2048, false);
//    texDesc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
//    texDesc->setStorageMode(MTL::StorageModePrivate);
//    MTL::Texture* tempTexture = m_device->newTexture(texDesc);
//    
//    float talusAngle = 0.7f; // ~40 degrees
//    float erosionRate = 0.1f;
//    
//    for (int i = 0; i < iterations; i++) {
//        MTL::CommandBuffer* cmd = queue->commandBuffer();
//        MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
//        
//        enc->setComputePipelineState(m_erosionPipeline);
//        enc->setTexture((i % 2 == 0) ? m_heightmapTexture : tempTexture, 0);
//        enc->setTexture((i % 2 == 0) ? tempTexture : m_heightmapTexture, 1);
//        enc->setBytes(&talusAngle, sizeof(float), 0);
//        enc->setBytes(&erosionRate, sizeof(float), 1);
//        
//        MTL::Size gridSize = MTL::Size::Make(2048, 2048, 1);
//        MTL::Size threadgroupSize = MTL::Size::Make(16, 16, 1);
//        
//        enc->dispatchThreads(gridSize, threadgroupSize);
//        enc->endEncoding();
//        
//        cmd->commit();
//        cmd->waitUntilCompleted();
//    }
//    
//    tempTexture->release();
//    queue->release();
//    
//    // Recalculate normals after erosion
//    generateNormalsGPU();
//}
//
//void TerrainSystem::update(float deltaTime, const simd::float3& cameraPosition)
//{
//    m_time += deltaTime;
//    
//    // Update chunks (streaming, LOD)
//    streamChunks(cameraPosition);
//    unloadDistantChunks(cameraPosition);
//    
//    // Update uniforms
//    m_uniforms.cameraPosition = cameraPosition;
//    m_uniforms.time = m_time;
//    m_uniforms.seed = m_seed;
//    m_uniforms.heightScale = m_heightScale;
//    m_uniforms.maxRenderDistance = m_maxRenderDistance;
//    m_uniforms.fogColor = {0.6f, 0.7f, 0.8f, 1.0f};
//    m_uniforms.fogDensity = 0.001f;
//    
//    memcpy(m_uniformsBuffer->contents(), &m_uniforms, sizeof(TerrainUniforms));
//}
//
//void TerrainSystem::render(MTL::RenderCommandEncoder* encoder,
//                          const simd::float4x4& viewProjection)
//{
//    if (!m_terrainPipeline)
//        return;
//
//    encoder->setRenderPipelineState(m_terrainPipeline);
//    encoder->setDepthStencilState(m_depthStencilState);
//    
//    m_uniforms.viewProjectionMatrix = viewProjection;
//    m_uniforms.modelMatrix = math::makeIdentity();
//    memcpy(m_uniformsBuffer->contents(), &m_uniforms, sizeof(TerrainUniforms));
//    
//    encoder->setVertexBuffer(m_uniformsBuffer, 0, 1);
//    encoder->setFragmentBuffer(m_uniformsBuffer, 0, 0);
//    
//    // Set textures
//    encoder->setFragmentTexture(m_albedoArray, 0);
//    encoder->setFragmentTexture(m_normalMapArray, 1);
//    encoder->setFragmentTexture(m_roughnessArray, 2);
//    encoder->setFragmentTexture(m_metallicArray, 3);
//    encoder->setFragmentTexture(m_aoArray, 4);
//    
//    // Render all visible chunks
//    for (const auto& chunk : m_chunks)
//    {
//        if (!chunk->visible || chunk->indexCount == 0)
//            continue;
//        
//        encoder->setVertexBuffer(chunk->vertexBuffer, 0, 0);
//        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
//                                      chunk->indexCount,
//                                      MTL::IndexTypeUInt32,
//                                      chunk->indexBuffer,
//                                      0);
//    }
//}
//
//TerrainChunk* TerrainSystem::getOrCreateChunk(int chunkX, int chunkZ) {
//    // Check if chunk exists
//    for (auto& chunk : m_chunks) {
//        if (chunk->chunkCoord.x == chunkX && chunk->chunkCoord.y == chunkZ) {
//            return chunk.get();
//        }
//    }
//    
//    // Create new chunk
//    auto chunk = std::make_unique<TerrainChunk>();
//    chunk->chunkCoord = {chunkX, chunkZ};
//    chunk->needsUpdate = true;
//    
//    TerrainChunk* ptr = chunk.get();
//    m_chunks.push_back(std::move(chunk));
//    
//    return ptr;
//}
//
//void TerrainSystem::generateChunkMesh(TerrainChunk* chunk) {
//    // Generate mesh for this chunk
//    // Simplified version - full implementation would sample heightmap texture
//    
//    std::vector<TerrainVertex> vertices;
//    std::vector<uint32_t> indices;
//    
//    int resolution = TerrainChunk::VERTEX_RESOLUTION;
//    float chunkWorldSize = TerrainChunk::CHUNK_SIZE;
//    float vertexSpacing = chunkWorldSize / (resolution - 1);
//    
//    float offsetX = chunk->chunkCoord.x * chunkWorldSize;
//    float offsetZ = chunk->chunkCoord.y * chunkWorldSize;
//    
//    // Generate grid vertices
//    for (int z = 0; z < resolution; z++) {
//        for (int x = 0; x < resolution; x++) {
//            TerrainVertex vertex;
//            vertex.position = {
//                offsetX + x * vertexSpacing,
//                0.0f, // Will be set from heightmap
//                offsetZ + z * vertexSpacing
//            };
//            
//            // Sample heightmap (simplified - would read from GPU texture)
//            float height = sampleHeightmap(vertex.position.x, vertex.position.z);
//            vertex.position.y = height;
//            vertex.height = height;
//            
//            // Calculate UVs
//            vertex.texCoord = {
//                (float)x / (resolution - 1),
//                (float)z / (resolution - 1)
//            };
//            
//            // Normal and tangent would be calculated or read from texture
////            vertex.normal = {0, 1, 0};
////            vertex.tangent = {1, 0, 0};
//            // Calculer la normale à partir des voisins
//            float hL = sampleHeightmap(vertex.position.x - vertexSpacing, vertex.position.z);
//            float hR = sampleHeightmap(vertex.position.x + vertexSpacing, vertex.position.z);
//            float hD = sampleHeightmap(vertex.position.x, vertex.position.z - vertexSpacing);
//            float hU = sampleHeightmap(vertex.position.x, vertex.position.z + vertexSpacing);
//
//            simd::float3 normal;
//            normal.x = (hL - hR) / (2.0f * vertexSpacing);
//            normal.y = 1.0f;
//            normal.z = (hD - hU) / (2.0f * vertexSpacing);
//            vertex.normal = simd::normalize(normal);
//
//            vertex.tangent = simd::normalize(simd::float3{1, normal.x, 0}); // Tangent le long de X
//            
//            // Biome color
//            vertex.biomeId = (float)m_biomeGenerator->getBiomeAt({vertex.position.x, vertex.position.z});
//            vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};
//
//            
//            vertices.push_back(vertex);
//        }
//    }
//    
//    // Generate indices
//    for (int z = 0; z < resolution - 1; z++) {
//        for (int x = 0; x < resolution - 1; x++) {
//            int baseIndex = z * resolution + x;
//            
//            // Triangle 1
//            indices.push_back(baseIndex);
//            indices.push_back(baseIndex + resolution);
//            indices.push_back(baseIndex + 1);
//            
//            // Triangle 2
//            indices.push_back(baseIndex + 1);
//            indices.push_back(baseIndex + resolution);
//            indices.push_back(baseIndex + resolution + 1);
//        }
//    }
//    
//    chunk->indexCount = indices.size();
//    
//    // Create GPU buffers
//    if (chunk->vertexBuffer) chunk->vertexBuffer->release();
//    if (chunk->indexBuffer) chunk->indexBuffer->release();
//    
//    chunk->vertexBuffer = m_device->newBuffer(vertices.data(),
//                                             vertices.size() * sizeof(TerrainVertex),
//                                             MTL::ResourceStorageModeShared);
//    chunk->indexBuffer = m_device->newBuffer(indices.data(),
//                                            indices.size() * sizeof(uint32_t),
//                                            MTL::ResourceStorageModeShared);
//    
//    chunk->needsUpdate = false;
//}
//
//float TerrainSystem::sampleHeightmap(float x, float z) const
//{
//    // Simplified CPU version - in real implementation would read from GPU texture
//    // For now, use biome generator
//    
//    BiomeType biome = m_biomeGenerator->getBiomeAt({x, z});
//    // Noise multi-octave simple
//    auto simpleNoise = [](float x, float z, float freq) -> float {
//        return sin(x * freq) * cos(z * freq);
//    };
//
//    float height = 0.0f;
//    float amplitude = 20.0f;
//    float frequency = 0.005f;
//    
//    // FBM simple
//    for (int i = 0; i < 6; i++)
//    {
//        height += simpleNoise(x, z, frequency) * amplitude;
//        amplitude *= 0.5f;
//        frequency *= 2.0f;
//    }
//    
//    // Base height par biome
//    float baseHeight = 4.0f;
//    float distFromCenter = sqrt(x*x + z*z);
//    switch (biome)
//    {
//        case BiomeType::SpawnCrater:
//            // Cratère plat au centre
//            baseHeight = std::max(0.0f, 20.0f - distFromCenter * 0.1f);
//            break;
//        case BiomeType::VoronoiPlains:
//            baseHeight = 15.0f;
//            break;
//            
//        case BiomeType::VoronoiIslands:
//            baseHeight = (height > 5.0f) ? height : -10.0f;
//            break;
//            
//        case BiomeType::VoronoiTerrace:
//            baseHeight = floor(height / 10.0f) * 10.0f;
//            break;
//            
//        case BiomeType::Ocean:
//            baseHeight = -15.0f;
//            break;
//            
//        case BiomeType::Moon:
//            baseHeight = 12.0f + height * 0.3f;
//            break;
//            
//        default:
//            baseHeight = 10.0f;
//    }
//    return (baseHeight + height * 0.5f) * m_heightScale;
//    
//    // Simple noise-based height
//    // In real version, this would read from the GPU-generated heightmap texture
//    
//    return height * m_heightScale;
//}
//
//float TerrainSystem::getHeightAt(float x, float z) const {
//    return sampleHeightmap(x, z);
//}
//
//simd::float3 TerrainSystem::getNormalAt(float x, float z) const {
//    float h = getHeightAt(x, z);
//    float hL = getHeightAt(x - 1.0f, z);
//    float hR = getHeightAt(x + 1.0f, z);
//    float hD = getHeightAt(x, z - 1.0f);
//    float hU = getHeightAt(x, z + 1.0f);
//    
//    simd::float3 normal;
//    normal.x = hL - hR;
//    normal.y = 2.0f;
//    normal.z = hD - hU;
//    
//    return simd::normalize(normal);
//}
//
//BiomeType TerrainSystem::getBiomeAt(float x, float z) const {
//    return m_biomeGenerator->getBiomeAt({x, z});
//}
//
//void TerrainSystem::streamChunks(const simd::float3& cameraPosition)
//{
//    // Load chunks around camera
//    int cameraChunkX = (int)floorf(cameraPosition.x / TerrainChunk::CHUNK_SIZE);
//    int cameraChunkZ = (int)floorf(cameraPosition.z / TerrainChunk::CHUNK_SIZE);
//    
//    int loadRadius = 8; // Load 8 chunks in each direction
//    
//    for (int z = -loadRadius; z <= loadRadius; z++) {
//        for (int x = -loadRadius; x <= loadRadius; x++) {
//            int chunkX = cameraChunkX + x;
//            int chunkZ = cameraChunkZ + z;
//            
//            TerrainChunk* chunk = getOrCreateChunk(chunkX, chunkZ);
//            if (chunk->needsUpdate)
//            {
//                generateChunkMesh(chunk);
//                chunk->needsUpdate = false;
//            }
//        }
//    }
//}
//
//void TerrainSystem::unloadDistantChunks(const simd::float3& cameraPosition) {
//    // Remove chunks that are too far
//    m_chunks.erase(
//        std::remove_if(m_chunks.begin(), m_chunks.end(),
//            [&](const std::unique_ptr<TerrainChunk>& chunk) {
//                simd::float2 chunkCenter = {
//                    chunk->chunkCoord.x * TerrainChunk::CHUNK_SIZE + TerrainChunk::CHUNK_SIZE * 0.5f,
//                    chunk->chunkCoord.y * TerrainChunk::CHUNK_SIZE + TerrainChunk::CHUNK_SIZE * 0.5f
//                };
//                
//                float distance = simd::length(simd::float2{cameraPosition.x, cameraPosition.z} - chunkCenter);
//                
//                if (distance > m_maxRenderDistance * 1.5f) {
//                    if (chunk->vertexBuffer) chunk->vertexBuffer->release();
//                    if (chunk->indexBuffer) chunk->indexBuffer->release();
//                    return true;
//                }
//                
//                chunk->visible = distance < m_maxRenderDistance;
//                return false;
//            }
//        ),
//        m_chunks.end()
//    );
//}
//
//bool TerrainSystem::checkTerrainCollision(const simd::float3& position, float radius) const {
//    float terrainHeight = getHeightAt(position.x, position.z);
//    return position.y - radius < terrainHeight;
//}
//
//simd::float3 TerrainSystem::resolveTerrainCollision(const simd::float3& position,
//                                                    const simd::float3& velocity,
//                                                    float radius) const {
//    float terrainHeight = getHeightAt(position.x, position.z);
//    simd::float3 resolved = position;
//    
//    if (position.y - radius < terrainHeight) {
//        resolved.y = terrainHeight + radius;
//    }
//    
//    return resolved;
//}
