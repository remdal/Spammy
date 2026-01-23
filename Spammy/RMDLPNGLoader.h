//
//  RMDLPNGLoader.h
//  Spammy
//
//  Created by Rémy on 23/01/2026.
//

#ifndef RMDLPNGLoader_h
#define RMDLPNGLoader_h

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include <string>
#include <simd/simd.h>

#include "RMDLUtils.hpp"

MTL::Texture* loadSingleTexture(const std::string& path, MTL::Device* device);
MTL::Texture* newTextureFromFile( const std::string& texturePath, MTL::Device* pDevice );
MTL::Texture* newTextureArrayFromFiles( const std::vector<std::string>& texturePaths, MTL::Device* pDevice, MTL::CommandBuffer* pCommandBuffer );

#endif /* RMDLPNGLoader_h */
