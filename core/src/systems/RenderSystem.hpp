//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "LightSubSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class LightSubSystem;

    class RenderSystem : public BaseSystem
    {
        Shader skinningShader{};
        LightSubSystem* lightSubSystem;

      public:
        void Update() override;
        void Draw();
        explicit RenderSystem(entt::registry* _registry, LightSubSystem* _lightSubSystem);
    };
} // namespace sage
