//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    class GameData;
    class LightSubSystem;
    struct Settings;
    struct KeyMapping;

    class Scene
    {

      protected:
        entt::registry* registry;

      public:
        std::unique_ptr<LightSubSystem> lightSubSystem;
        std::unique_ptr<GameData> data;
        entt::sigh<void()> sceneLoadingFinished; // Currently unused
        entt::sigh<void()> sceneChange;

        virtual void Update();
        virtual void Draw3D();
        virtual void Draw2D();
        virtual ~Scene();
        virtual void DrawDebug();
        explicit Scene(
            entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, const std::string& mapPath);
    };
} // namespace sage
