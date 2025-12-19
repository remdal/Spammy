/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLObjLoader.cpp            +++     +++		**/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 21/09/2025 15:22:51      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "RMDLObjLoader.hpp"

RMDLObjMesh::RMDLObjMesh()
{}

RMDLObjMesh::~RMDLObjMesh()
{}

int RMDLObjMesh::vertexCount()
{
    return (_vertexBuffer->length() / sizeof(RMDLObjVertex));
}

int RMDLObjMesh::indexCount()
{
    return (_indexBuffer->length() / sizeof(uint16_t));
}


RMDLObjLoader::RMDLObjLoader()
{}

RMDLObjLoader::~RMDLObjLoader()
{}

static constexpr uint kBufferSize = 2048;

RMDLObjLoader& RMDLObjLoader::initWithDevice( MTL::Device* device ) // == MTL::Device* pDevice ): _pDevice( pDevice->retain() )
{
    _device = device;
    return (*this);
}

