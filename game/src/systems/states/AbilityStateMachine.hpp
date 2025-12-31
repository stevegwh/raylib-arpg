#pragma once

#include "components/Ability.hpp"
#include "components/States.hpp"
#include "engine/systems/states/StateMachine.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;
    class VisualFX;

    class AbilityStateMachine : sage::StateMachine<AbilityState, AbilityStateEnum>
    {
        class IdleState;
        class AwaitingExecutionState;
        class CursorSelectState;

        Systems* sys;

        void executeAbility(entt::entity abilityEntity);
        bool checkRange(entt::entity abilityEntity) const;
        void spawnAbility(entt::entity abilityEntity);
        void cancelCast(entt::entity abilityEntity);
        void startCast(entt::entity abilityEntity);

        void onComponentAdded(entt::entity addedEntity);
        void onComponentRemoved(entt::entity addedEntity) const;

      public:
        void Update() const;
        void Draw3D();

        ~AbilityStateMachine() override = default;
        AbilityStateMachine(const AbilityStateMachine&) = delete;
        AbilityStateMachine& operator=(const AbilityStateMachine&) = delete;
        AbilityStateMachine(entt::registry* _registry, Systems* _sys);

        friend class StateMachine; // Required for CRTP
    };

} // namespace lq