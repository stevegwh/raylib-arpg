// FountainEffect.hpp
#pragma once

#include "ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
    struct FountainPartSys : public ParticleSystem
    {
        Texture2D texCircle16{};
        Texture2D texCircle8{};

        ~FountainPartSys();
        explicit FountainPartSys(Camera3D* cam);
    };
} // namespace sage
