//
// Created by steve on 03/08/2024.
//

#pragma once

// #include "entt/entt.hpp"
#include "components/CombatableActor.hpp"

namespace lq
{

    class CombatSystem
    {
        entt::registry* registry;
        // Can have callbacks for certain types of damage being inflicted so that other
        // systems can react and modify it (chain effects etc).
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void RegisterAttack(AttackData attackData);
        explicit CombatSystem(entt::registry* _registry);
    };

} // namespace sage
