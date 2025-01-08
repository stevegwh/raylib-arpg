//
// Created by steve on 04/01/2025.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{

    struct Systems;

    class SpatialAudioSystem
    {
        entt::registry* registry;
        Systems* sys;

      public:
        void Update() const;
        SpatialAudioSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
