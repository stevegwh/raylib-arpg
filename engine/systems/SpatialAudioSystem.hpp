//
// Created by steve on 04/01/2025.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{

    struct EngineSystems;

    class SpatialAudioSystem
    {
        entt::registry* registry;
        EngineSystems* sys;

      public:
        void Update() const;
        SpatialAudioSystem(entt::registry* _registry, EngineSystems* _sys);
    };

} // namespace sage
