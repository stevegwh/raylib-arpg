//
// Created by Steve Wheeler on 06/12/2024.
//

#pragma once

#include "Timer.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    class CursorClickIndicator
    {
        entt::registry* registry;
        GameData* gameData;
        entt::entity self;
        float k = 0;

        entt::connection destinationReachedCnx;

        void onCursorClick(entt::entity entity);
        void onReachLocation();
        void onSelectedActorChanged();

      public:
        void Update();
        CursorClickIndicator(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
