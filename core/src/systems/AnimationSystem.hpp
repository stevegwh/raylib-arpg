//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class AnimationSystem : public BaseSystem
    {
      public:
        void Update() const;
        void Draw();
        explicit AnimationSystem(entt::registry* _registry);
    };
} // namespace sage
