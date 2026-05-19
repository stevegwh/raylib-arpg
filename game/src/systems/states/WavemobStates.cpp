#include "WavemobStates.hpp"

#include "WavemobStateMachine.hpp"
#include "AbilityFactory.hpp"
#include "Systems.hpp"
#include "animation/RpgAnimationIds.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"

#include "raylib.h"

namespace lq
{
    void WavemobDefaultState::OnEnter(WavemobStateMachine& machine, const entt::entity entity)
    {
        machine.registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void WavemobDefaultState::OnExit(WavemobStateMachine&, entt::entity)
    {
    }

    void WavemobDefaultState::Update(WavemobStateMachine&, entt::entity)
    {
    }

    void WavemobTargetOutOfRangeState::OnEnter(
        WavemobStateMachine& machine,
        const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);

        auto& moveable = registry->get<sage::MoveableActor>(entity);
        const auto& combatable = registry->get<CombatableActor>(entity);
        moveable.movementCollisionTarget = combatable.target;
        auto& target = registry->get<sage::MoveableActor>(combatable.target);
        auto& state = registry->get<WavemobState>(entity);

        auto onTargetReached = [machinePtr](const entt::entity e) {
            machinePtr->ChangeState(e, WavemobCombatState{});
        };

        state.BindSubscription(target.onPathChanged.Subscribe(
            [machinePtr, entity](const entt::entity t) { machinePtr->onTargetPosUpdate(entity, t); }));
        state.BindSubscription(moveable.onDestinationReached.Subscribe(onTargetReached));

        machine.onTargetPosUpdate(entity, combatable.target);
    }

    void WavemobTargetOutOfRangeState::OnExit(WavemobStateMachine& machine, const entt::entity entity)
    {
        machine.registry->get<sage::MoveableActor>(entity).movementCollisionTarget.reset();
    }

    void WavemobTargetOutOfRangeState::Update(WavemobStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        const auto& combatable = registry->get<CombatableActor>(entity);
        if (combatable.target == entt::null || machine.isTargetOutOfSight(entity))
        {
            machine.ChangeState(entity, WavemobDefaultState{});
        }
    }

    void WavemobCombatState::OnEnter(WavemobStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);
    }

    void WavemobCombatState::OnExit(WavemobStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
    }

    void WavemobCombatState::Update(WavemobStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        const auto& combatable = registry->get<CombatableActor>(entity);
        if (combatable.dying || combatable.target == entt::null)
        {
            machine.ChangeState(entity, WavemobDefaultState{});
            return;
        }
        const auto& actorTrans = registry->get<sage::sgTransform>(entity);
        const auto target = registry->get<sage::sgTransform>(combatable.target).GetWorldPos();
        const float distance = Vector3Distance(actorTrans.GetWorldPos(), target);
        // TODO: Arbitrary number. Should probably use the navigation system to
        // find the "next best square" from current position
        if (distance >= 8.0f)
        {
            machine.ChangeState(entity, WavemobTargetOutOfRangeState{});
        }
    }

    void WavemobDyingState::OnEnter(WavemobStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        auto& combatable = registry->get<CombatableActor>(entity);
        combatable.target = entt::null;
        combatable.dying = true;
        const auto& bb = registry->get<sage::Collideable>(entity).worldBoundingBox;
        sys->engine.navigationGridSystem->MarkSquareAreaOccupied(bb, false);

        auto& animation = registry->get<sage::Animation>(entity);
        animation.ChangeAnimationById(lq::animation_ids::Death, true);

        auto& state = registry->get<WavemobState>(entity);
        state.BindSubscription(
            animation.onAnimationEnd.Subscribe([machinePtr](const entt::entity e) { machinePtr->destroyEntity(e); }));

        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);

        sys->engine.actorMovementSystem->CancelMovement(entity);
    }

    void WavemobDyingState::OnExit(WavemobStateMachine&, entt::entity)
    {
    }

    void WavemobDyingState::Update(WavemobStateMachine&, entt::entity)
    {
    }
} // namespace lq
