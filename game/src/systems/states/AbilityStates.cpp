#include "AbilityStates.hpp"

#include "abilities/AbilityData.hpp"
#include "abilities/AbilityFunctions.hpp"
#include "abilities/AbilityIndicator.hpp"
#include "AbilityFactory.hpp"
#include "AbilityStateMachine.hpp"
#include "components/Ability.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/Cursor.hpp"
#include "engine/Timer.hpp"
#include "Systems.hpp"

#include "raylib.h"

namespace lq
{
    void AbilityIdleState::OnEnter(AbilityStateMachine&, entt::entity)
    {
    }

    void AbilityIdleState::OnExit(AbilityStateMachine&, entt::entity)
    {
    }

    void AbilityIdleState::Update(AbilityStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto& ab = registry->get<Ability>(entity);
        const auto& ad = registry->get<AbilityData>(entity);
        ab.cooldownTimer.Update(GetFrameTime());
        if (ab.cooldownTimer.HasFinished() && ad.base.HasOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO))
        {
            machine.startCast(entity);
        }
    }

    void AbilityCursorSelectState::OnEnter(AbilityStateMachine& machine, const entt::entity entity)
    {
        machine.enableCursor(entity);
    }

    void AbilityCursorSelectState::OnExit(AbilityStateMachine& machine, const entt::entity entity)
    {
        machine.disableCursor(entity);
    }

    void AbilityCursorSelectState::Update(AbilityStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto& ab = registry->get<Ability>(entity);
        ab.abilityIndicator->Update(sys->engine.cursor->getFirstNaviCollision().point);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            machine.spawnAbility(entity);
        }
    }

    void AbilityAwaitingExecutionState::OnEnter(AbilityStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        auto& ab = registry->get<Ability>(entity);
        ab.cooldownTimer.Start();
        ab.castTimer.Start();

        const auto& ad = registry->get<AbilityData>(entity);
        if (ad.base.HasBehaviour(AbilityBehaviour::MOVEMENT_PROJECTILE))
        {
            createProjectile(registry, ab.caster, entity, sys);
            auto& moveable = registry->get<sage::MoveableActor>(entity);
            auto& state = registry->get<AbilityState>(entity);
            state.BindSubscription(moveable.onDestinationReached.Subscribe(
                [machinePtr](const entt::entity e) { machinePtr->executeAbility(e); }));
        }
    }

    void AbilityAwaitingExecutionState::OnExit(AbilityStateMachine&, entt::entity)
    {
    }

    void AbilityAwaitingExecutionState::Update(AbilityStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto& ab = registry->get<Ability>(entity);
        ab.castTimer.Update(GetFrameTime());
        const auto& ad = registry->get<AbilityData>(entity);

        // "executionDelayTimer" should just be a cast timer. Therefore, below should check for cast time
        // behaviour
        if (ab.castTimer.HasFinished() && !ad.base.HasBehaviour(AbilityBehaviour::CAST_REGULAR))
        {
            machine.executeAbility(entity);
        }
    }
} // namespace lq
