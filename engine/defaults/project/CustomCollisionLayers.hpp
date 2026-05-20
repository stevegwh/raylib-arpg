#pragma once

// Default empty stub used when the engine is built standalone (no consuming
// project provides project/CustomCollisionLayers.hpp). The consuming project
// overrides this by setting SAGE_PROJECT_INCLUDE_DIR in its CMake.

#include "engine/CollisionLayers.hpp"

namespace sage
{
    inline constexpr std::array<CollisionLayer, 0> CustomCollisionLayers{};
} // namespace sage
