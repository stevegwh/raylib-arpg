//
// Created by steve on 04/01/2025.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{

    struct BaseSystems;

    class SpatialAudioSystem
    {
        entt::registry* registry;
        BaseSystems* sys;

      public:
        void Update() const;
        SpatialAudioSystem(entt::registry* _registry, BaseSystems* _sys);
    };

} // namespace sage
