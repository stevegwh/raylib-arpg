//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "Scene.hpp"

namespace sage
{
    struct KeyMapping;
    struct Settings;
    class AudioManager;
} // namespace sage

namespace lq
{
    class ExampleScene final : public Scene
    {
      public:
        void Init() override;
        ~ExampleScene() override = default;
        ExampleScene(
            entt::registry* _registry,
            sage::KeyMapping* _keyMapping,
            sage::Settings* _settings,
            sage::AudioManager* _audioManager);
    };
} // namespace lq
