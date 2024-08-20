//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "GameData.hpp"
#include "systems/LightSubSystem.hpp"
#include <GameObjectFactory.hpp>

#include <memory>

namespace sage
{
    class Scene
    {
      protected:
        entt::registry* registry;

      public:
        std::unique_ptr<GameData> data;
        std::unique_ptr<LightSubSystem> lightSubSystem;
        entt::sigh<void()> sceneChange;

        virtual void Update();

        virtual void Draw3D();

        virtual void Draw2D();

        virtual ~Scene() = default;

        virtual void DrawDebug();

        explicit Scene(entt::registry* _registry, std::unique_ptr<GameData> _data, const std::string& mapPath);
    };
} // namespace sage
