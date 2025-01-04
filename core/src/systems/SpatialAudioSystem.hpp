//
// Created by steve on 04/01/2025.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{

    struct GameData;

    class SpatialAudioSystem
    {
        entt::registry* registry;
        GameData* gameData;

      public:
        void Update() const;
        SpatialAudioSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
