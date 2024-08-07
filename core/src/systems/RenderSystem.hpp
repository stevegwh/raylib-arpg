//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "components/Renderable.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class RenderSystem : public BaseSystem
    {
      public:
        explicit RenderSystem(entt::registry* _registry);
        ~RenderSystem();
        void Update() override;
        void Draw() const;
    };
} // namespace sage
