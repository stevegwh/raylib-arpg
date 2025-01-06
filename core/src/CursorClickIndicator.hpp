//
// Created by Steve Wheeler on 06/12/2024.
//

#pragma once

#include "Timer.hpp"

#include <entt/entt.hpp>
#include <Event.hpp>
#include <memory>

namespace sage
{
    class GameData;

    class CursorClickIndicator
    {
        entt::registry* registry;
        GameData* gameData;
        entt::entity self;
        float k = 0;

        std::unique_ptr<Connection> destinationReachedCnx;

        void onCursorClick(entt::entity entity) const;
        void disableIndicator() const;
        void onSelectedActorChanged(entt::entity, entt::entity current);

      public:
        void Update();
        CursorClickIndicator(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
