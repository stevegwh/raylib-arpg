//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    class EntityReflectionSignalRouter;
    class TextureTerrainOverlay;

    class ControllableActor
    {
        entt::entity self;
        std::vector<int> hooks;

      public:
        void AddHook(int id);
        void ReleaseAllHooks(EntityReflectionSignalRouter* router);
        std::unique_ptr<TextureTerrainOverlay>
            selectedIndicator; // Initialised by ControllableActorSystem on creation
        entt::sigh<void(entt::entity, entt::entity)> onEnemyLeftClick{};  // Self, Clicked enemy
        entt::sigh<void(entt::entity, entt::entity)> onEnemyRightClick{}; // Self, Clicked enemy
        entt::sigh<void(entt::entity, entt::entity)> onFloorClick{};      // Self, object clicked (can discard)
        entt::sigh<void(entt::entity, entt::entity)> onNPCLeftClick{};

        explicit ControllableActor(entt::entity _self);
    };
} // namespace sage
