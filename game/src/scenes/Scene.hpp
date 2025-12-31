//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "engine/Event.hpp"
#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    struct Settings;
    struct KeyMapping;
    class AudioManager;
} // namespace sage

namespace lq
{
    class Systems;
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
        std::unique_ptr<Systems> sys;
        sage::Event<entt::entity> sceneLoadingFinished; // Currently unused
        sage::Event<entt::entity> sceneChange;

        virtual void Init() = 0;
        virtual void Update();
        virtual void Draw3D();
        virtual void DrawDebug3D();
        virtual void Draw2D();
        virtual void DrawDebug2D();
        virtual ~Scene();

        explicit Scene(
            entt::registry* _registry,
            sage::KeyMapping* _keyMapping,
            sage::Settings* _settings,
            sage::AudioManager* _audioManager);
    };
} // namespace lq
