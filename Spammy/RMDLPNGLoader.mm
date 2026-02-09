//
//  RMDLPNGLoader.m
//  Spammy
//
//  Created by Rémy on 23/01/2026.
//

#include "RMDLPNGLoader.h"

#import <MetalKit/MetalKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>

#include <stdio.h>

MTL::Texture* loadTextureForArray(const std::string& resourcesPath, MTL::Device* pDevice)
{
    NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:resourcesPath.c_str()]];
    MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)pDevice];
    
    NSError* __autoreleasing error = nil;
    id<MTLTexture> texture = [loader newTextureWithContentsOfURL:url options:@{MTKTextureLoaderOptionLoadAsArray : @(YES),
                                                                               MTKTextureLoaderOptionTextureStorageMode: @(MTLStorageModeShared),
                                                                               MTKTextureLoaderOptionTextureUsage: @(MTLTextureUsageShaderRead),
                                                                               MTKTextureLoaderOptionGenerateMipmaps: @(NO)} error:&error];
    if (!texture)
        NSLog(@"Error loading texture at \"%s\": %@", resourcesPath.c_str(), error.localizedDescription);
    assert(texture);
    return (__bridge MTL::Texture *)texture;
}

MTL::Texture* newTextureFromFile(const std::string& texturePath, MTL::Device* pDevice)
{
    NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:texturePath.c_str()]];
    MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)pDevice];
    
    NSError* __autoreleasing error = nil;
    id<MTLTexture> texture = [loader newTextureWithContentsOfURL:url options:@{MTKTextureLoaderOptionLoadAsArray : @(YES),
                                                                               MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
                                                                               MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageShaderRead),
                                                                               MTKTextureLoaderOptionGenerateMipmaps : @(YES)} error:&error];
    if (!texture)
        NSLog(@"Error loading texture at \"%s\": %@", texturePath.c_str(), error.localizedDescription);
    assert(texture);
    return (__bridge MTL::Texture *)texture;
}

MTL::Texture* newTextureArrayFromFiles(const std::vector<std::string>& texturePaths, MTL::Device* pDevice, MTL::CommandBuffer* pCommandBuffer)
{
    std::vector<NS::SharedPtr<MTL::Texture>> textures;

    for (size_t i = 0; i < texturePaths.size(); ++i)
        textures.push_back(NS::TransferPtr(newTextureFromFile(texturePaths[i], pDevice)));
    
    MTL::Texture* pTexture = nullptr;
    if (textures.size() > 0)
    {
        auto pTextureDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
        pTextureDesc->setWidth(textures[0]->width());
        pTextureDesc->setHeight(textures[0]->height());
        pTextureDesc->setPixelFormat(textures[0]->pixelFormat());
        pTextureDesc->setTextureType(MTL::TextureType2DArray);
        pTextureDesc->setUsage(MTL::TextureUsageShaderRead);
        pTextureDesc->setStorageMode(MTL::StorageModePrivate);
        pTextureDesc->setArrayLength(textures.size());
        pTextureDesc->setMipmapLevelCount(std::log2(textures[0]->width()));
        pTexture = pDevice->newTexture(pTextureDesc.get());
        auto pAutoreleasePool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
        MTL::CommandBuffer* pCmd       = pCommandBuffer;
        MTL::BlitCommandEncoder* pBlit = pCmd->blitCommandEncoder();
        for (size_t i = 0; i < textures.size(); ++i)
        {
            auto w = textures[i]->width();
            auto h = textures[i]->height();
            auto d = textures[i]->depth();

            pBlit->copyFromTexture(textures[i].get(), 0, 0, MTL::Origin(0,0,0), MTL::Size(w,h,d), pTexture, i, 0, MTL::Origin(0,0,0));
        }
        pBlit->generateMipmaps(pTexture);
        pBlit->endEncoding();
    }
    return (pTexture);
}
