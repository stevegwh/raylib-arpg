#include "PlayerStateMachine.hpp"

#include "Systems.hpp"

#include "AbilityFactory.hpp"
#include "Camera.hpp"
#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/DialogComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "PartyMemberStateMachine.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "raylib.h"
#include "StateMachines.hpp"
#include "systems/LootSystem.hpp"
#include "systems/PartySystem.hpp"

#include <cassert>
#include <format>

namespace sage
{
    class PlayerStateMachine::DefaultState : public State
    {
        PlayerStateMachine* stateController;

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity) override
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnExit(entt::entity entity) override
        {
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* _registry, Systems* _sys) : State(_registry, _sys)
        {
        }

        friend class PlayerStateMachine;
    };

    // ----------------------------

    class PlayerStateMachine::MovingToLocationState : public State
    {
        PlayerStateMachine* stateController;

        void onMovementCancelled(entt::entity self) const
        {

            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnEnter(entt::entity self) override
        {
            sys->actorMovementSystem->CancelMovement(self);
            auto& moveable = registry->get<MoveableActor>(self);
            auto& state = registry->get<PlayerState>(self);

            auto party = sys->partySystem->GetAllMembers();

            for (const auto& member : party)
            {
                const auto& collideable = registry->get<Collideable>(member);
                sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
            }

            if (sys->actorMovementSystem->TryPathfindToLocation(self, sys->cursor->getFirstCollision().point))
            {
                auto& animation = registry->get<Animation>(self);
                animation.ChangeAnimationByEnum(AnimationEnum::RUN);
                auto syb = moveable.onDestinationReached.Subscribe(
                    [this](entt::entity _entity) { onTargetReached(_entity); });
                state.ManageSubscription(std::move(syb));
                auto syb1 = moveable.onMovementCancel.Subscribe(
                    [this](entt::entity _entity) { onMovementCancelled(_entity); });
                state.ManageSubscription(std::move(syb1));
                auto syb2 = moveable.onDestinationUnreachable.Subscribe(
                    [this](entt::entity _entity, Vector3) { onMovementCancelled(_entity); });
                state.ManageSubscription(std::move(syb2));
            }
            else
            {
                stateController->ChangeState(self, PlayerStateEnum::Default);
            }

            for (const auto& member : party)
            {
                const auto& collideable = registry->get<Collideable>(member);
                sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);
            }
        }

        void OnExit(entt::entity self) override
        {
        }

        ~MovingToLocationState() override = default;

        MovingToLocationState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::MovingToTalkToNPCState final : public State
    {
        PlayerStateMachine* stateController;
        void onMovementCancelled(const entt::entity self) const
        {
            auto& dialogComponent = registry->get<DialogComponent>(self);
            dialogComponent.dialogTarget = entt::null;
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(const entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::InDialog);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnEnter(entt::entity self, entt::entity target)
        {
            auto& moveable = registry->get<MoveableActor>(self);
            auto& playerDiag = registry->get<DialogComponent>(self);
            playerDiag.dialogTarget = target;
            const auto& pos = registry->get<DialogComponent>(playerDiag.dialogTarget).conversationPos;
            sys->actorMovementSystem->PathfindToLocation(self, pos);

            auto& state = registry->get<PlayerState>(self);
            auto syb = moveable.onDestinationReached.Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(syb));
            auto syb1 = moveable.onMovementCancel.Subscribe(
                [this](entt::entity _entity) { onMovementCancelled(_entity); });
            state.ManageSubscription(std::move(syb1));

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);
        }

        void OnExit(entt::entity self) override
        {
        }

        ~MovingToTalkToNPCState() override = default;

        MovingToTalkToNPCState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::DestinationUnreachableState : public State
    {
        PlayerStateMachine* stateController;
        struct StateData
        {
            Vector3 originalDestination{};
            PlayerStateEnum previousState{};
            double timeStart{};
            float threshold{};
            unsigned int tryCount{};
            unsigned int maxTries{};
        };
        std::unordered_map<entt::entity, StateData> data;

      public:
        void Update(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity, Vector3 originalDestination, PlayerStateEnum previousState)
        {
            data[entity] = {originalDestination, previousState, GetTime(), 1.5f, 0, 4};

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnExit(entt::entity self) override
        {
            data.erase(self);
        }

        ~DestinationUnreachableState() override = default;

        DestinationUnreachableState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::InDialogState : public State
    {
        PlayerStateMachine* stateController;

      public:
        void OnEnter(entt::entity self) override
        {
            auto& playerDiag = registry->get<DialogComponent>(self);
            registry->get<Animation>(self).ChangeAnimationByEnum(AnimationEnum::TALK);
            if (registry->any_of<Animation>(playerDiag.dialogTarget))
            {
                registry->get<Animation>(playerDiag.dialogTarget).ChangeAnimationByEnum(AnimationEnum::TALK);
            }

            // Rotate to look at NPC
            auto& actorTrans = registry->get<sgTransform>(self);
            const auto& npcTrans = registry->get<sgTransform>(playerDiag.dialogTarget);
            Vector3 direction = Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos());
            direction = Vector3Normalize(direction);
            const float angle = atan2f(direction.x, direction.z);
            actorTrans.SetRotation({actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});

            sys->dialogSystem->StartConversation(npcTrans, playerDiag.dialogTarget);
            sys->playerAbilitySystem->UnsubscribeFromUserInput();
        }

        void OnExit(entt::entity self) override
        {
            auto& playerDiag = registry->get<DialogComponent>(self);
            if (registry->any_of<Animation>(playerDiag.dialogTarget))
            {
                registry->get<Animation>(playerDiag.dialogTarget).ChangeAnimationByEnum(AnimationEnum::IDLE);
            }
            playerDiag.dialogTarget = entt::null;
            sys->playerAbilitySystem->SubscribeToUserInput();
            // TODO: Bug: Doesn't change back to default on dialog end
        }

        ~InDialogState() override = default;

        InDialogState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::MovingToLootState : public State
    {
        PlayerStateMachine* stateController;
        void onMovementCancelled(const entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(const entt::entity self) const
        {
            auto& moveable = registry->get<MoveableActor>(self);
            sys->lootSystem->OnChestClick(moveable.lootTarget.value());
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnEnter(entt::entity self, entt::entity target)
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.lootTarget = target;
            auto& trans = registry->get<sgTransform>(self);
            auto& chestTrans = registry->get<sgTransform>(target);
            Vector3 dest = Vector3Add(
                trans.GetWorldPos(),
                Vector3MultiplyByValue(Vector3Subtract(chestTrans.GetWorldPos(), trans.GetWorldPos()), 0.85));
            // TODO: N.B. Right now, its possible that 'dest' is outside of LOOT_RANGE
            sys->actorMovementSystem->PathfindToLocation(self, dest);

            auto& state = registry->get<PlayerState>(self);
            auto syb = moveable.onDestinationReached.Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(syb));
            auto syb1 = moveable.onMovementCancel.Subscribe(
                [this](entt::entity _entity) { onMovementCancelled(_entity); });
            state.ManageSubscription(std::move(syb1));

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);
        }

        void OnExit(entt::entity self) override
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.lootTarget.reset();
        }

        ~MovingToLootState() override = default;

        MovingToLootState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::MovingToAttackEnemyState : public State
    {
        PlayerStateMachine* stateController;
        void onAttackCancelled(entt::entity self, entt::entity) const
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Combat);
        }

      public:
        void OnEnter(entt::entity self) override
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);

            auto& moveableActor = registry->get<MoveableActor>(self);

            auto& state = registry->get<PlayerState>(self);
            auto syb = moveableActor.onDestinationReached.Subscribe(
                [this](const entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(syb));

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            auto& controllable = registry->get<ControllableActor>(self);
            auto syb1 = controllable.onFloorClick.Subscribe(
                [this](const entt::entity _self, entt::entity clicked) { onAttackCancelled(_self, clicked); });
            state.ManageSubscription(std::move(syb1));

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 playerPos = registry->get<sgTransform>(self).GetWorldPos();
            Vector3 enemyPos = enemyTrans.GetWorldPos();
            Vector3 direction = Vector3Subtract(enemyPos, playerPos);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            sys->actorMovementSystem->PathfindToLocation(self, targetPos);
        }

        void OnExit(entt::entity self) override
        {
        }

        ~MovingToAttackEnemyState() override = default;

        MovingToAttackEnemyState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    // TODO: Move combat to its own state machine
    class PlayerStateMachine::CombatState : public State
    {
        PlayerStateMachine* stateController;
        void onAttackCancelled(entt::entity self, entt::entity) const
        {
            // Both outcomes are the same
            onTargetDeath(self, entt::null);
        }

        void onTargetDeath(entt::entity self, entt::entity) const
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        bool checkInCombat(entt::entity entity)
        {
            // Might do more here later
            return true;
        }

      public:
        void Update(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity) override
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

            auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);

            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
            combatable.onTargetDeathSub =
                enemyCombatable.onDeath.Subscribe([entity, this](const entt::entity target) {
                    const auto& c = registry->get<CombatableActor>(entity);
                    c.onTargetDeath.Publish(entity, target);
                });

            auto& state = registry->get<PlayerState>(entity);
            auto syb = combatable.onTargetDeath.Subscribe(
                [this](entt::entity self, entt::entity target) { onTargetDeath(self, target); });
            state.ManageSubscription(std::move(syb));
        }

        void OnExit(entt::entity entity) override
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            combatable.onTargetDeathSub.UnSubscribe();
            auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
        }

        ~CombatState() override = default;
        CombatState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void PlayerStateMachine::onFloorClick(const entt::entity self, entt::entity)
    {
        auto& state = registry->get<PlayerState>(self);
        // We're not allowed to change to the same state, so change to default and then back again
        if (state.GetCurrentState() == PlayerStateEnum::MovingToLocation)
        {
            ChangeState(self, PlayerStateEnum::Default);
        }
        ChangeState(self, PlayerStateEnum::MovingToLocation);
    }

    void PlayerStateMachine::onChestClick(const entt::entity self, entt::entity target)
    {
        auto& state = registry->get<PlayerState>(self);
        // We're not allowed to change to the same state, so change to default and then back again
        if (state.GetCurrentState() == PlayerStateEnum::MovingToLoot)
        {
            ChangeState(self, PlayerStateEnum::Default);
        }
        ChangeState<MovingToLootState, entt::entity>(self, PlayerStateEnum::MovingToLoot, target);
    }

    void PlayerStateMachine::onNPCLeftClick(entt::entity self, entt::entity target)
    {
        if (!registry->any_of<DialogComponent>(target)) return;

        if (registry->any_of<MoveableActor>(target)) // Not all NPCs move
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.emplace(registry, self, target);
        }
        ChangeState<MovingToTalkToNPCState, entt::entity>(self, PlayerStateEnum::MovingToTalkToNPC, target);
    }

    void PlayerStateMachine::onEnemyLeftClick(entt::entity self, entt::entity target)
    {
        auto& combatable = registry->get<CombatableActor>(self);
        combatable.target = target;
        ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
    }

    void PlayerStateMachine::Update()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PartyMemberState>(entity));
            const auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetState(state)->Update(entity);
        }
    }

    void PlayerStateMachine::Draw3D()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PartyMemberState>(entity));
            const auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetState(state)->Draw3D(entity);
        }
    }

    void PlayerStateMachine::onComponentAdded(entt::entity entity)
    {
        // Cursor and controllable events are connected in ControllableActorSystem
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickSub = controllable.onEnemyLeftClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onEnemyLeftClick(self, target); });
        controllable.onNPCLeftClickSub = controllable.onNPCLeftClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onNPCLeftClick(self, target); });
        controllable.onFloorClickSub = controllable.onFloorClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onFloorClick(self, target); });
        controllable.onChestClickSub = controllable.onChestClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onChestClick(self, target); });
        // ----------------------------
    }

    void PlayerStateMachine::onComponentRemoved(entt::entity entity)
    {
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickSub.UnSubscribe();
        controllable.onChestClickSub.UnSubscribe();
        controllable.onNPCLeftClickSub.UnSubscribe();
        controllable.onFloorClickSub.UnSubscribe();
    }

    PlayerStateMachine::PlayerStateMachine(entt::registry* _registry, Systems* _sys) : StateMachine(_registry)
    {
        states[PlayerStateEnum::Default] = std::make_unique<DefaultState>(_registry, _sys);
        states[PlayerStateEnum::MovingToAttackEnemy] =
            std::make_unique<MovingToAttackEnemyState>(_registry, _sys, this);
        states[PlayerStateEnum::Combat] = std::make_unique<CombatState>(_registry, _sys, this);
        states[PlayerStateEnum::MovingToTalkToNPC] =
            std::make_unique<MovingToTalkToNPCState>(_registry, _sys, this);
        states[PlayerStateEnum::InDialog] = std::make_unique<InDialogState>(_registry, _sys, this);
        states[PlayerStateEnum::MovingToLocation] = std::make_unique<MovingToLocationState>(_registry, _sys, this);
        states[PlayerStateEnum::DestinationUnreachable] =
            std::make_unique<DestinationUnreachableState>(_registry, _sys, this);
        states[PlayerStateEnum::MovingToLoot] = std::make_unique<MovingToLootState>(_registry, _sys, this);

        registry->on_construct<PlayerState>().connect<&PlayerStateMachine::onComponentAdded>(this);
        registry->on_destroy<PlayerState>().connect<&PlayerStateMachine::onComponentRemoved>(this);
    }
} // namespace sage