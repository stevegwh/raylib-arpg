//
// Created by Steve Wheeler on 29/12/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

    class GameData;

    class InteractableSystem
    {
        entt::registry* registry;
        GameData* gameData;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void OnInteractableClick(entt::entity entity) const;
        InteractableSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
