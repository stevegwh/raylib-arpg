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
        // Persists between state changes (do not try to add them via state.AddConnection)
        std::shared_ptr<Connection> cursorOnEnemyLeftClickCnx{};
        std::shared_ptr<Connection> cursorOnEnemyRightClickCnx{};
        std::shared_ptr<Connection> cursorOnFloorClickCnx{};
        std::shared_ptr<Connection> cursorOnNPCLeftClickCnx{};

        // The forwarded events' connections (to unsubscribe)
        std::shared_ptr<Connection> onEnemyLeftClickCnx{};
        std::shared_ptr<Connection> onEnemyRightClickCnx{};
        std::shared_ptr<Connection> onFloorClickCnx{};
        std::shared_ptr<Connection> onNPCLeftClickCnx{};

        // The events themselves
        const std::unique_ptr<Event<entt::entity, entt::entity>> onEnemyLeftClick{};  // Self, Clicked enemy
        const std::unique_ptr<Event<entt::entity, entt::entity>> onEnemyRightClick{}; // Self, Clicked enemy
        const std::unique_ptr<Event<entt::entity, entt::entity>> onFloorClick{}; // Self, Clicked Col (can discard)
        const std::unique_ptr<Event<entt::entity, entt::entity>> onNPCLeftClick{}; // Self, Clicked NPC

        ControllableActor();
    };
} // namespace sage
