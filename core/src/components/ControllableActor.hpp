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
        Subscription cursorOnEnemyLeftClickSub{};
        Subscription cursorOnEnemyRightClickSub{};
        Subscription cursorOnFloorClickSub{};
        Subscription cursorOnNPCLeftClickSub{};
        Subscription cursorOnChestClickSub{};

        // The forwarded events' subscriptions (to unsubscribe)
        Subscription onEnemyLeftClickSub{};
        Subscription onEnemyRightClickSub{};
        Subscription onFloorClickSub{};
        Subscription onNPCLeftClickSub{};
        Subscription onChestClickSub{};

        // The events themselves
        Event<entt::entity, entt::entity> onEnemyLeftClick{};  // Self, Clicked enemy
        Event<entt::entity, entt::entity> onEnemyRightClick{}; // Self, Clicked enemy
        Event<entt::entity, entt::entity> onFloorClick{};      // Self, Clicked Col (can discard)
        Event<entt::entity, entt::entity> onNPCLeftClick{};    // Self, Clicked NPC
        Event<entt::entity, entt::entity> onChestClick{};      // Self, Clicked Chest
    };
} // namespace sage
