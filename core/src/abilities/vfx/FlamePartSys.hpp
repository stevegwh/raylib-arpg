// FountainEffect.hpp
#pragma once

#include "ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
    struct FlamePartSys : public ParticleSystem
    {
        Texture2D texCircle16{};

        ~FlamePartSys();
        explicit FlamePartSys(Camera3D* cam);
    };
} // namespace sage