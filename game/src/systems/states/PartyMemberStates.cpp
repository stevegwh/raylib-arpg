#include "PartyMemberStates.hpp"

#include "PartyMemberStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "Systems.hpp"
#include "components/PartyMemberComponent.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "raylib.h"

#include <cassert>

namespace
{
    constexpr int FOLLOW_DISTANCE = 15;
    constexpr float RETRY_TIME_THRESHOLD = 1.5f;
    constexpr unsigned int MAX_TRIES = 10;
}

namespace lq
{
    void PartyMemberDefaultState::OnEnter(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        machine.registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void PartyMemberDefaultState::OnExit(PartyMemberStateMachine&, entt::entity)
    {
    }

    void PartyMemberDefaultState::Update(PartyMemberStateMachine&, entt::entity)
    {
    }

    void PartyMemberFollowingLeaderState::OnEnter(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        const auto followTarget = partyMember.followTarget.value();

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);

        auto& moveable = registry->get<sage::MoveableActor>(entity);
        moveable.movementCollisionTarget = followTarget;
        auto& target = registry->get<sage::MoveableActor>(followTarget);
        auto& state = registry->get<PartyMemberState>(entity);

        auto onTargetReached = [machinePtr](const entt::entity e) {
            machinePtr->ChangeState(e, PartyMemberDefaultState{});
        };
        auto onMovementCancelled = [machinePtr](const entt::entity e) {
            machinePtr->ChangeState(e, PartyMemberDefaultState{});
        };
        auto onDestinationUnreachable = [machinePtr](const entt::entity e, const Vector3 requestedPos) {
            machinePtr->ChangeState(
                e,
                PartyMemberDestinationUnreachableState{
                    .originalDestination = requestedPos, .timeStart = GetTime()});
        };

        state.BindSubscription(moveable.onDestinationReached.Subscribe(onTargetReached));
        state.BindSubscription(target.onPathChanged.Subscribe(
            [machinePtr, entity](const entt::entity t) { machinePtr->onFollowingTargetPathChanged(entity, t); }));
        state.BindSubscription(moveable.onMovementCancel.Subscribe(onMovementCancelled));
        state.BindSubscription(moveable.onDestinationUnreachable.Subscribe(onDestinationUnreachable));

        machine.onFollowingTargetPathChanged(entity, followTarget);
    }

    void PartyMemberFollowingLeaderState::OnExit(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        auto& moveable = machine.registry->get<sage::MoveableActor>(entity);
        moveable.movementCollisionTarget.reset();
        machine.sys->engine.actorMovementSystem->CancelMovement(entity);
    }

    void PartyMemberFollowingLeaderState::Update(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        const auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        const auto& transform = registry->get<sage::sgTransform>(entity);
        const auto& followTrans = registry->get<sage::sgTransform>(partyMember.followTarget.value());
        const auto& followMoveable = registry->get<sage::MoveableActor>(partyMember.followTarget.value());

        if (followMoveable.IsMoving() &&
            Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE >
                Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
        {
            machine.ChangeState(entity, PartyMemberWaitingForLeaderState{});
        }
    }

    void PartyMemberWaitingForLeaderState::OnEnter(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        const auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PartyMemberState>(entity);

        auto onMovementCancelled = [machinePtr](const entt::entity e) {
            machinePtr->ChangeState(e, PartyMemberDefaultState{});
        };

        state.BindSubscription(moveable.onMovementCancel.Subscribe(onMovementCancelled));
        state.BindSubscription(sys->selectionSystem->onSelectedActorChange.Subscribe(
            [onMovementCancelled](entt::entity, const entt::entity e) { onMovementCancelled(e); }));

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void PartyMemberWaitingForLeaderState::OnExit(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        machine.registry->get<sage::MoveableActor>(entity).movementCollisionTarget.reset();
    }

    void PartyMemberWaitingForLeaderState::Update(PartyMemberStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        if (entity == sys->selectionSystem->GetSelectedActor())
        {
            machine.ChangeState(entity, PartyMemberDefaultState{});
            return;
        }

        const auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        const auto& transform = registry->get<sage::sgTransform>(entity);
        const auto& followTrans = registry->get<sage::sgTransform>(partyMember.followTarget.value());
        const auto& followMoveable = registry->get<sage::MoveableActor>(partyMember.followTarget.value());

        if (followMoveable.IsMoving() &&
            Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE <
                Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
        {
            machine.ChangeState(entity, PartyMemberFollowingLeaderState{});
        }
    }

    void PartyMemberDestinationUnreachableState::OnEnter(
        PartyMemberStateMachine& machine,
        const entt::entity entity)
    {
        machine.registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void PartyMemberDestinationUnreachableState::OnExit(PartyMemberStateMachine&, entt::entity)
    {
    }

    void PartyMemberDestinationUnreachableState::Update(
        PartyMemberStateMachine& machine,
        const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        if (moveable.IsMoving()) return;

        if (tryCount >= MAX_TRIES)
        {
            moveable.movementCollisionTarget.reset();
            sys->engine.actorMovementSystem->CancelMovement(entity);
            machine.ChangeState(entity, PartyMemberDefaultState{});
            return;
        }

        if (GetTime() < timeStart + RETRY_TIME_THRESHOLD) return;

        ++tryCount;
        timeStart = GetTime();
        if (sys->engine.actorMovementSystem->TryPathfindToLocation(entity, originalDestination, true))
        {
            machine.ChangeState(entity, PartyMemberFollowingLeaderState{});
            return;
        }

        const auto& target = registry->get<PartyMemberComponent>(entity).followTarget;
        assert(target.has_value());
        const auto leaderPos = registry->get<sage::sgTransform>(target.value()).GetWorldPos();
        if (sys->engine.actorMovementSystem->TryPathfindToLocation(entity, leaderPos, true))
        {
            tryCount = 0;
        }
    }
} // namespace lq
