//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    class GameData;
    struct Settings;
    struct KeyMapping;

    class Scene
    {

      protected:
        entt::registry* registry;

      public:
        std::unique_ptr<GameData> data;
        entt::sigh<void()> sceneLoadingFinished; // Currently unused
        entt::sigh<void()> sceneChange;

        virtual void Update();
        virtual void Draw3D();
        virtual void DrawDebug3D();
        virtual void Draw2D();
        virtual void DrawDebug2D();
        virtual ~Scene();

        explicit Scene(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings);
    };
} // namespace sage
