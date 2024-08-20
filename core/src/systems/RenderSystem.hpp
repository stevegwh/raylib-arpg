//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class RenderSystem : public BaseSystem
    {
      public:
        void Update() override;
        void Draw() const;
        ~RenderSystem();
        explicit RenderSystem(entt::registry* _registry);
    };
} // namespace sage
