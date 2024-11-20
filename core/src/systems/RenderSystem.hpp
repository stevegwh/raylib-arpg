//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "BaseSystem.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class RenderSystem : public BaseSystem
    {
      public:
        RenderTexture renderTexture;
        void Update() override;
        void Draw();
        explicit RenderSystem(entt::registry* _registry);
    };
} // namespace sage
