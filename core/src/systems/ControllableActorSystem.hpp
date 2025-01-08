//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "BaseSystem.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include "Event.hpp"

namespace sage
{
    class Systems;

    class ControllableActorSystem : public BaseSystem
    {
        static constexpr Color activeCol = {0, 255, 0, 255};
        static constexpr Color inactiveCol = {0, 255, 0, 75};
        Systems* sys;
        entt::entity selectedActorId = entt::null;
        void onComponentAdded(entt::entity addedEntity);
        void onComponentRemoved(entt::entity removedEntity);

      public:
        void SetSelectedActor(entt::entity id);
        [[nodiscard]] entt::entity GetSelectedActor() const;
        Event<entt::entity, entt::entity> onSelectedActorChange; // prev, current
        void Update() const;
        ControllableActorSystem(entt::registry* _registry, Systems* _sys);
    };
} // namespace sage
