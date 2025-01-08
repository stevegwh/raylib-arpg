//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "Scene.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class ExampleScene final : public Scene
    {
      public:
        void Init() override;
        ~ExampleScene() override = default;
        ExampleScene(
            entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager);
    };
} // namespace sage
