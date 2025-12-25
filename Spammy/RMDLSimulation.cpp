//
//  RMDLSimulation.cpp
//  Spammy
//
//  Created by Rémy on 23/12/2025.
//

#include "RMDLSimulation.hpp"

RMDLSimulation::RMDLSimulation(MTL::Device* pDevice) : _pDevice(pDevice->retain())
{
}

RMDLSimulation::~RMDLSimulation()
{
    
}
