//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "engine/Event.hpp"
#include "engine/TextureTerrainOverlay.hpp"

// #include "entt/entt.hpp"

// #include <memory>
// #include <vector>

namespace lq
{
    class ControllableActor
    {

      public:
        std::unique_ptr<sage::TextureTerrainOverlay>
            selectedIndicator; // Initialised by ControllableActorSystem on creation

        // We forward the cursor's clicks onto our own events (and inject this entity's id into it)
        // Persists between state changes (do not try to add them via state.AddConnection)
        sage::Subscription cursorOnEnemyLeftClickSub{};
        sage::Subscription cursorOnEnemyRightClickSub{};
        sage::Subscription cursorOnFloorClickSub{};
        sage::Subscription cursorOnNPCLeftClickSub{};
        sage::Subscription cursorOnChestClickSub{};

        // The forwarded events' subscriptions (to unsubscribe)
        sage::Subscription onEnemyLeftClickSub{};
        sage::Subscription onEnemyRightClickSub{};
        sage::Subscription onFloorClickSub{};
        sage::Subscription onNPCLeftClickSub{};
        sage::Subscription onChestClickSub{};

        // The events themselves
        sage::Event<entt::entity, entt::entity> onEnemyLeftClick{};  // Self, Clicked enemy
        sage::Event<entt::entity, entt::entity> onEnemyRightClick{}; // Self, Clicked enemy
        sage::Event<entt::entity, entt::entity> onFloorClick{};      // Self, Clicked Col (can discard)
        sage::Event<entt::entity, entt::entity> onNPCLeftClick{};    // Self, Clicked NPC
        sage::Event<entt::entity, entt::entity> onChestClick{};      // Self, Clicked Chest
    };
} // namespace lq
