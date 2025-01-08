//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "Event.hpp"

#include <memory>

namespace sage
{
    class Systems;
    struct Settings;
    struct KeyMapping;
    class AudioManager;
    class SpiralFountainVFX;

    class Scene
    {
        std::unique_ptr<SpiralFountainVFX> spiral;
        void initAssets() const;
        void initUI() const;
        void loadSpawners() const;

      protected:
        entt::registry* registry;

      public:
        std::unique_ptr<Systems> data;
        Event<entt::entity> sceneLoadingFinished; // Currently unused
        Event<entt::entity> sceneChange;

        virtual void Init() = 0;
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
