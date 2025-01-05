//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <Event.hpp>

#include <memory>
#include <vector>

namespace sage
{
    class TextureTerrainOverlay;

    class ControllableActor
    {

      public:
        std::unique_ptr<TextureTerrainOverlay>
            selectedIndicator; // Initialised by ControllableActorSystem on creation

        // We forward the cursor's clicks onto our own events (and inject this entity's id into it)
        Connection<entt::entity> cursorOnEnemyLeftClickCnx{};
        Connection<entt::entity> cursorOnEnemyRightClickCnx{};
        Connection<entt::entity> cursorOnFloorClickCnx{};
        Connection<entt::entity> cursorOnNPCLeftClickCnx{};

        Connection<entt::entity, entt::entity> onEnemyLeftClickCnx{};
        Connection<entt::entity, entt::entity> onEnemyRightClickCnx{};
        Connection<entt::entity, entt::entity> onFloorClickCnx{};
        Connection<entt::entity, entt::entity> onNPCLeftClickCnx{};

        std::unique_ptr<Event<entt::entity, entt::entity>> onEnemyLeftClick{};  // Self, Clicked enemy
        std::unique_ptr<Event<entt::entity, entt::entity>> onEnemyRightClick{}; // Self, Clicked enemy
        std::unique_ptr<Event<entt::entity, entt::entity>> onFloorClick{};      // Self, Clicked Col (can discard)
        std::unique_ptr<Event<entt::entity, entt::entity>> onNPCLeftClick{};    // Self, Clicked NPC

        ControllableActor();
    };
} // namespace sage
