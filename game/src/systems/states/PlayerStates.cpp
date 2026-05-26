#include "PlayerStates.hpp"

#include "PlayerStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "AbilityFactory.hpp"
#include "Systems.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/DialogComponent.hpp"
#include "engine/Cursor.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/LootSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "raylib.h"

#include <cassert>

namespace lq
{
    void PlayerDefaultState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void PlayerDefaultState::OnExit(PlayerStateMachine&, entt::entity)
    {
    }

    void PlayerMovingToLocationState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        sys->engine.actorMovementSystem->CancelMovement(entity);
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);
        auto* machinePtr = &machine;

        const auto party = sys->partySystem->GetAllMembers();
        for (const auto& member : party)
        {
            const auto& collideable = registry->get<sage::Collideable>(member);
            sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
        }

        if (sys->engine.actorMovementSystem->TryPathfindToLocation(
                entity, sys->engine.cursor->getFirstNaviCollision().point))
        {
            registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
            state.BindSubscription(moveable.onDestinationReached.Subscribe(
                [machinePtr](const entt::entity e) { machinePtr->ChangeState(e, PlayerDefaultState{}); }));
            state.BindSubscription(moveable.onMovementCancel.Subscribe(
                [machinePtr](const entt::entity e) { machinePtr->ChangeState(e, PlayerDefaultState{}); }));
            state.BindSubscription(moveable.onDestinationUnreachable.Subscribe(
                [machinePtr](const entt::entity e, Vector3) { machinePtr->ChangeState(e, PlayerDefaultState{}); }));
        }
        else
        {
            machine.ChangeState(entity, PlayerDefaultState{});
        }

        for (const auto& member : party)
        {
            const auto& collideable = registry->get<sage::Collideable>(member);
            sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);
        }
    }

    void PlayerMovingToLocationState::OnExit(PlayerStateMachine&, entt::entity)
    {
    }

    void PlayerMovingToAttackEnemyState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
        auto& moveableActor = registry->get<sage::MoveableActor>(entity);
        auto& combatable = registry->get<CombatableActor>(entity);
        assert(combatable.target != entt::null);
        auto& state = registry->get<PlayerState>(entity);
        auto onTargetReached = [machinePtr](const entt::entity e) { machinePtr->ChangeState(e, PlayerCombatState{}); };
        state.BindSubscription(moveableActor.onDestinationReached.Subscribe(onTargetReached));
        state.BindSubscription(sys->engine.cursor->onNavigationClick.Subscribe(
            [registry, machinePtr, entity](entt::entity, sage::CollisionLayer) {
                registry->get<CombatableActor>(entity).target = entt::null;
                machinePtr->ChangeState(entity, PlayerDefaultState{});
            }));
        const Vector3 playerPos = registry->get<sage::sgTransform>(entity).GetWorldPos();
        const Vector3 enemyPos = registry->get<sage::sgTransform>(combatable.target).GetWorldPos();
        const Vector3 offset =
            Vector3Scale(Vector3Normalize(Vector3Subtract(enemyPos, playerPos)), combatable.attackRange);
        sys->engine.actorMovementSystem->PathfindToLocation(entity, Vector3Subtract(enemyPos, offset));
    }

    void PlayerMovingToAttackEnemyState::OnExit(PlayerStateMachine&, entt::entity)
    {
    }

    void PlayerMovingToTalkState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        assert(target != entt::null);
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);

        moveable.movementCollisionTarget = target;
        const auto destination = registry->get<DialogComponent>(target).conversationPos;
        sys->engine.actorMovementSystem->PathfindToLocation(entity, destination);

        state.BindSubscription(moveable.onDestinationReached.Subscribe([registry, machinePtr](const entt::entity e) {
            const auto target = std::get<PlayerMovingToTalkState>(registry->get<PlayerState>(e).current).target;
            machinePtr->ChangeState(e, PlayerInDialogState{.target = target});
        }));
        state.BindSubscription(moveable.onMovementCancel.Subscribe(
            [machinePtr](const entt::entity e) { machinePtr->ChangeState(e, PlayerDefaultState{}); }));

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
    }

    void PlayerMovingToTalkState::OnExit(PlayerStateMachine& machine, const entt::entity entity)
    {
        machine.registry->get<sage::MoveableActor>(entity).movementCollisionTarget.reset();
    }

    void PlayerMovingToLootState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        assert(target != entt::null);
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);

        const auto& trans = registry->get<sage::sgTransform>(entity);
        const auto& chestTrans = registry->get<sage::sgTransform>(target);
        const auto destination = Vector3Add(
            trans.GetWorldPos(),
            sage::Vector3MultiplyByValue(Vector3Subtract(chestTrans.GetWorldPos(), trans.GetWorldPos()), 0.85));
        sys->engine.actorMovementSystem->PathfindToLocation(entity, destination);

        state.BindSubscription(moveable.onDestinationReached.Subscribe([registry, sys, machinePtr](const entt::entity e) {
            const auto target = std::get<PlayerMovingToLootState>(registry->get<PlayerState>(e).current).target;
            sys->lootSystem->OnChestClick(target);
            machinePtr->ChangeState(e, PlayerDefaultState{});
        }));
        state.BindSubscription(moveable.onMovementCancel.Subscribe(
            [machinePtr](const entt::entity e) { machinePtr->ChangeState(e, PlayerDefaultState{}); }));

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
    }

    void PlayerMovingToLootState::OnExit(PlayerStateMachine&, entt::entity)
    {
    }

    void PlayerInDialogState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        assert(target != entt::null);
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Talk);
        if (registry->any_of<sage::Animation>(target))
        {
            registry->get<sage::Animation>(target).ChangeAnimationById(lq::animation_ids::Talk);
        }

        auto& actorTrans = registry->get<sage::sgTransform>(entity);
        const auto& npcTrans = registry->get<sage::sgTransform>(target);
        const Vector3 direction =
            Vector3Normalize(Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos()));
        const float angle = atan2f(direction.x, direction.z);
        actorTrans.rotation.world = {actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z};

        sys->dialogSystem->StartConversation(npcTrans, target);
        sys->playerAbilitySystem->UnsubscribeFromUserInput();
    }

    void PlayerInDialogState::OnExit(PlayerStateMachine& machine, const entt::entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        if (registry->any_of<sage::Animation>(target))
        {
            registry->get<sage::Animation>(target).ChangeAnimationById(lq::animation_ids::Idle);
        }
        sys->playerAbilitySystem->SubscribeToUserInput();
    }

    void PlayerCombatState::OnEnter(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto* machinePtr = &machine;
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::AutoAttack);

        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
        registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);

        auto& combatable = registry->get<CombatableActor>(entity);
        assert(combatable.target != entt::null);
        auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);

        auto forwardEnemyDeath = [entity, registry](const entt::entity target) {
            registry->get<CombatableActor>(entity).onTargetDeath.Publish(entity, target);
        };
        auto onTargetDeath = [registry, machinePtr](const entt::entity e, entt::entity) {
            registry->get<CombatableActor>(e).target = entt::null;
            machinePtr->ChangeState(e, PlayerDefaultState{});
        };

        combatable.onTargetDeathSub = enemyCombatable.onDeath.Subscribe(forwardEnemyDeath);

        auto& state = registry->get<PlayerState>(entity);
        state.BindSubscription(combatable.onTargetDeath.Subscribe(onTargetDeath));
    }

    void PlayerCombatState::OnExit(PlayerStateMachine& machine, const entt::entity entity)
    {
        auto* registry = machine.registry;
        auto* sys = machine.sys;
        auto& combatable = registry->get<CombatableActor>(entity);
        combatable.onTargetDeathSub.UnSubscribe();
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
    }
} // namespace lq
