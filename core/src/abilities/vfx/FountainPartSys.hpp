// FountainEffect.hpp
#pragma once

#include "ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
    struct FountainPartSys : public ParticleSystem
    {
        explicit FountainPartSys(Camera3D* cam);
    };
} // namespace sage
