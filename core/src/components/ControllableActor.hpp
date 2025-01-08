//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Event.hpp"
#include "TextureTerrainOverlay.hpp"

// #include "entt/entt.hpp"

// #include <memory>
// #include <vector>

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
        std::unique_ptr<Connection> cursorOnEnemyLeftClickCnx{};
        std::unique_ptr<Connection> cursorOnEnemyRightClickCnx{};
        std::unique_ptr<Connection> cursorOnFloorClickCnx{};
        std::unique_ptr<Connection> cursorOnNPCLeftClickCnx{};

        // The forwarded events' connections (to unsubscribe)
        std::unique_ptr<Connection> onEnemyLeftClickCnx{};
        std::unique_ptr<Connection> onEnemyRightClickCnx{};
        std::unique_ptr<Connection> onFloorClickCnx{};
        std::unique_ptr<Connection> onNPCLeftClickCnx{};

        // The events themselves
        Event<entt::entity, entt::entity> onEnemyLeftClick{};  // Self, Clicked enemy
        Event<entt::entity, entt::entity> onEnemyRightClick{}; // Self, Clicked enemy
        Event<entt::entity, entt::entity> onFloorClick{};      // Self, Clicked Col (can discard)
        Event<entt::entity, entt::entity> onNPCLeftClick{};    // Self, Clicked NPC
    };
} // namespace sage
