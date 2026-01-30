//
//  RMDLFPS.cpp
//  Spammy
//
//  Created by Rémy on 29/01/2026.
//

#include "RMDLFPS.hpp"

FPSSystem::FPSSystem()
{
    m_rng.seed(std::random_device{}()); // random_device is generally only used to seed a PRNG such as mt19937 == range
//    m_rng.seed(89);
}

