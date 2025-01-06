//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "Event.hpp"

#include <memory>

namespace sage
{
    class GameData;
    struct Settings;
    struct KeyMapping;
    class AudioManager;
    class SpiralFountainVFX;

    class Scene
    {

        std::unique_ptr<SpiralFountainVFX> spiral;

      protected:
        entt::registry* registry;

      public:
        std::unique_ptr<GameData> data;
        std::unique_ptr<Event<entt::entity>> sceneLoadingFinished; // Currently unused
        std::unique_ptr<Event<entt::entity>> sceneChange;

        virtual void Update();
        virtual void Draw3D();
        virtual void DrawDebug3D();
        virtual void Draw2D();
        virtual void DrawDebug2D();
        virtual ~Scene();

        explicit Scene(
            entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager);
    };
} // namespace sage
