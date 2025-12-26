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
    class ControllableActor
    {

      public:
        std::unique_ptr<TextureTerrainOverlay>
            selectedIndicator; // Initialised by ControllableActorSystem on creation

        // We forward the cursor's clicks onto our own events (and inject this entity's id into it)
        // Persists between state changes (do not try to add them via state.AddConnection)
        Connection cursorOnEnemyLeftClickCnx{};
        Connection cursorOnEnemyRightClickCnx{};
        Connection cursorOnFloorClickCnx{};
        Connection cursorOnNPCLeftClickCnx{};
        Connection cursorOnChestClickCnx{};

        // The forwarded events' connections (to unsubscribe)
        Connection onEnemyLeftClickCnx{};
        Connection onEnemyRightClickCnx{};
        Connection onFloorClickCnx{};
        Connection onNPCLeftClickCnx{};
        Connection onChestClickCnx{};

        // The events themselves
        Event<entt::entity, entt::entity> onEnemyLeftClick{};  // Self, Clicked enemy
        Event<entt::entity, entt::entity> onEnemyRightClick{}; // Self, Clicked enemy
        Event<entt::entity, entt::entity> onFloorClick{};      // Self, Clicked Col (can discard)
        Event<entt::entity, entt::entity> onNPCLeftClick{};    // Self, Clicked NPC
        Event<entt::entity, entt::entity> onChestClick{};      // Self, Clicked Chest
    };
} // namespace sage
