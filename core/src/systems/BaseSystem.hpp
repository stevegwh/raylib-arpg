//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class BaseSystem
    {
      protected:
        bool enabled = true;
        entt::registry* registry;

      public:
        explicit BaseSystem(entt::registry* _registry) : registry(_registry)
        {
        }
        virtual void Update() {};
        virtual void Draw3D() {};
    };
} // namespace sage
