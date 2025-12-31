// FountainEffect.hpp
#pragma once

#include "../../../../engine/ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace lq
{
    struct FlamePartSys : public sage::ParticleSystem
    {
        Texture2D texCircle16{};

        explicit FlamePartSys(Camera3D* cam);
    };
} // namespace lq