// FountainEffect.hpp
#pragma once

#include "engine/ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace lq
{
    struct FountainPartSys : public sage::ParticleSystem
    {
        explicit FountainPartSys(Camera3D* cam);
    };
} // namespace lq
