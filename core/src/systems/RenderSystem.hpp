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
        entt::entity FindRenderableByName(const std::string& name);
        entt::entity FindRenderableByMeshName(const std::string& name);
        void Update() override;
        void Draw();
        explicit RenderSystem(entt::registry* _registry);
    };
} // namespace sage
