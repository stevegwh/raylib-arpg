#include "PlayerStateMachine.hpp"

#include "GameData.hpp"

#include "AbilityFactory.hpp"
#include "Camera.hpp"
#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/DialogComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "EntityReflectionSignalRouter.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/PartySystem.hpp"

#include "raylib.h"
#include <cassert>
#include <format>

// #define SAGE_DEBUG

namespace sage
{
    class PlayerStateController::DefaultState : public StateMachine
    {
        void onFloorClick(const entt::entity self, entt::entity x) const
        {
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;
            auto& state = registry->get<PlayerState>(self);
            state.ChangeState(self, PlayerStateEnum::MovingToLocation);
        }

        void onNPCLeftClick(entt::entity self, entt::entity target) const
        {
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;
            if (!registry->any_of<DialogComponent>(target)) return;

            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.emplace(registry, self, target);
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToTalkToNPC);
        }

        void onEnemyLeftClick(entt::entity self, entt::entity target) const
        {
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
        }

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered DefaultState. \n", static_cast<int>(entity));
#endif

            // Below are not disconnected in OnStateExit
            // Bridge was created in GameObjectFactory to connect controllable to cursor
            auto& controllable = registry->get<ControllableActor>(entity);
            entt::sink leftSink{controllable.onEnemyLeftClick};
            leftSink.connect<&DefaultState::onEnemyLeftClick>(this);
            entt::sink npcSink{controllable.onNPCLeftClick};
            npcSink.connect<&DefaultState::onNPCLeftClick>(this);
            entt::sink floorClickSink{controllable.onFloorClick};
            floorClickSink.connect<&DefaultState::onFloorClick>(this);
            // ----------------------------

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateExit(entt::entity entity) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited DefaultState. \n", static_cast<int>(entity));
#endif
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    static constexpr int FOLLOW_DISTANCE = 15;

    class PlayerStateController::FollowingLeaderState final : public StateMachine
    {

        void onMovementCancelled(const entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetPosUpdate(const entt::entity self, const entt::entity target) const
        {
            const auto& trans = registry->get<sgTransform>(self);
            const auto& targetTrans = registry->get<sgTransform>(target);
            const auto& targetMoveable = registry->get<MoveableActor>(target);
            auto dest = targetMoveable.IsMoving() ? targetMoveable.GetDestination() : targetTrans.GetWorldPos();
            const auto dir = Vector3Normalize(Vector3Subtract(dest, trans.GetWorldPos()));
            dest = Vector3Subtract(dest, Vector3MultiplyByValue(dir, FOLLOW_DISTANCE));
            gameData->actorMovementSystem->PathfindToLocation(self, dest);
        }

        void onTargetReached(const entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
            if (self == gameData->controllableActorSystem->GetSelectedActor())
            {
                auto& state = registry->get<PlayerState>(self);
                state.ChangeState(self, PlayerStateEnum::Default);
            }

            const auto& moveable = registry->get<MoveableActor>(self);
            const auto& transform = registry->get<sgTransform>(self);
            const auto& followTrans = registry->get<sgTransform>(moveable.followTarget->targetActor);
            const auto& followMoveable = registry->get<MoveableActor>(moveable.followTarget->targetActor);

            // If we are closer to our destination than the leader is, then wait.
            if (followMoveable.IsMoving() &&
                Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE >
                    Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
            {
                registry->emplace_or_replace<FollowTargetParams>(
                    self, registry, self, moveable.followTarget->targetActor);
                auto& state = registry->get<PlayerState>(self);
                state.ChangeState(self, PlayerStateEnum::WaitingForLeader);
            }
        }

        void OnStateEnter(const entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered FollowingLeaderState. \n", static_cast<int>(self));
#endif
            const auto& followTargetParams = registry->get<FollowTargetParams>(self);

            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.emplace(followTargetParams);
            registry->erase<FollowTargetParams>(self);

            const auto target = moveable.followTarget->targetActor;
            auto& targetMoveable = registry->get<MoveableActor>(target);

            auto& state = registry->get<PlayerState>(self);
            entt::sink sink1{targetMoveable.onDestinationReached};
            state.currentStateConnections.push_back(sink1.connect<&FollowingLeaderState::onTargetReached>(this));
            entt::sink sink2{moveable.followTarget->onPathChanged};
            state.currentStateConnections.push_back(sink2.connect<&FollowingLeaderState::onTargetPosUpdate>(this));
            entt::sink sink3{moveable.onMovementCancel};
            state.currentStateConnections.push_back(
                sink3.connect<&FollowingLeaderState::onMovementCancelled>(this));

            onTargetPosUpdate(self, target);
        }

        void OnStateExit(const entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited FollowingLeaderState. \n", static_cast<int>(self));
#endif
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
            gameData->actorMovementSystem->CancelMovement(self);
        }

        ~FollowingLeaderState() override = default;

        FollowingLeaderState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };
    // ----------------------------

    class PlayerStateController::WaitingForLeaderState final : public StateMachine
    {

        void onMovementCancelled(const entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
            if (self == gameData->controllableActorSystem->GetSelectedActor())
            {
                auto& state = registry->get<PlayerState>(self);
                state.ChangeState(self, PlayerStateEnum::Default);
            }

            const auto& moveable = registry->get<MoveableActor>(self);
            const auto& transform = registry->get<sgTransform>(self);
            const auto& followTrans = registry->get<sgTransform>(moveable.followTarget->targetActor);
            const auto& followMoveable = registry->get<MoveableActor>(moveable.followTarget->targetActor);

            // Follow target is now closer to its destination than we are, so we can proceed.
            if (followMoveable.IsMoving() &&
                Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE <
                    Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
            {
                registry->emplace_or_replace<FollowTargetParams>(
                    self, registry, self, moveable.followTarget->targetActor);
                auto& state = registry->get<PlayerState>(self);
                state.ChangeState(self, PlayerStateEnum::FollowingLeader);
            }
        }

        void OnStateEnter(const entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered WaitingForLeaderState. \n", static_cast<int>(self));
#endif
            const auto& followTargetParams = registry->get<FollowTargetParams>(self);
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.emplace(followTargetParams);
            registry->erase<FollowTargetParams>(self);
            auto& state = registry->get<PlayerState>(self);
            entt::sink sink{moveable.onMovementCancel};
            state.currentStateConnections.push_back(
                sink.connect<&WaitingForLeaderState::onMovementCancelled>(this));
        }

        void OnStateExit(const entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited WaitingForLeaderState. \n", static_cast<int>(self));
#endif
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
        }

        ~WaitingForLeaderState() override = default;

        WaitingForLeaderState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };
    // ----------------------------

    class PlayerStateController::MovingToLocationState : public StateMachine
    {

        void onMovementCancelled(entt::entity self) const
        {
            auto& state = registry->get<PlayerState>(self);
            state.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnStateEnter(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered MovingToTalkToNPCState. \n", static_cast<int>(self));
#endif

            gameData->actorMovementSystem->CancelMovement(self);
            gameData->actorMovementSystem->PathfindToLocation(self, gameData->cursor->getFirstCollision().point);

            auto target = self;
            auto group = gameData->partySystem->GetGroup(self);
            for (const auto& entity : group)
            {
                if (entity == self) continue;
                registry->emplace_or_replace<FollowTargetParams>(entity, registry, entity, target);
                gameData->actorMovementSystem->CancelMovement(entity);
                auto& playerState = registry->get<PlayerState>(entity);
                playerState.ChangeState(entity, PlayerStateEnum::FollowingLeader);
            }

            auto& moveable = registry->get<MoveableActor>(self);
            auto& state = registry->get<PlayerState>(self);
            entt::sink sink{moveable.onDestinationReached};
            state.currentStateConnections.push_back(sink.connect<&MovingToLocationState::onTargetReached>(this));
            entt::sink sink2{moveable.onMovementCancel};
            state.currentStateConnections.push_back(
                sink2.connect<&MovingToLocationState::onMovementCancelled>(this));
        }

        void OnStateExit(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited MovingToTalkToNPCState. \n", static_cast<int>(self));
#endif
        }

        ~MovingToLocationState() override = default;

        MovingToLocationState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::MovingToTalkToNPCState : public StateMachine
    {

        void onMovementCancelled(const entt::entity self) const
        {
            auto& dialogComponent = registry->get<DialogComponent>(self);
            dialogComponent.dialogTarget = entt::null;
            auto& playerState = registry->get<PlayerState>(self);
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(const entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::InDialog);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnStateEnter(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered MovingToTalkToNPCState. \n", static_cast<int>(self));
#endif
            auto& moveable = registry->get<MoveableActor>(self);
            auto& playerDiag = registry->get<DialogComponent>(self);
            playerDiag.dialogTarget = moveable.followTarget->targetActor;
            const auto& pos = registry->get<DialogComponent>(playerDiag.dialogTarget).conversationPos;
            gameData->actorMovementSystem->PathfindToLocation(self, pos);

            auto& state = registry->get<PlayerState>(self);
            entt::sink sink{moveable.onDestinationReached};
            state.currentStateConnections.push_back(sink.connect<&MovingToTalkToNPCState::onTargetReached>(this));
            entt::sink sink2{moveable.onMovementCancel};
            state.currentStateConnections.push_back(
                sink2.connect<&MovingToTalkToNPCState::onMovementCancelled>(this));
        }

        void OnStateExit(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited MovingToTalkToNPCState. \n", static_cast<int>(self));
#endif
        }

        ~MovingToTalkToNPCState() override = default;

        MovingToTalkToNPCState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::DestinationUnreachableState : public StateMachine
    {

      public:
        void OnStateEnter(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered DestinationUnreachableState. \n", static_cast<int>(self));
#endif
        }

        void OnStateExit(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited DestinationUnreachableState. \n", static_cast<int>(self));
#endif
        }

        ~DestinationUnreachableState() override = default;

        DestinationUnreachableState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::InDialogState : public StateMachine
    {

      public:
        void Update(entt::entity self) override
        {
            // if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            // {
            //     auto& playerState = registry->get<PlayerState>(self);
            //     playerState.ChangeState(self, PlayerStateEnum::Default);
            // }
        }

        void OnStateEnter(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered InDialogState. \n", static_cast<int>(self));
#endif
            auto& playerDiag = registry->get<DialogComponent>(self);
            registry->get<Animation>(self).ChangeAnimationByEnum(AnimationEnum::TALK);
            registry->get<Animation>(playerDiag.dialogTarget).ChangeAnimationByEnum(AnimationEnum::TALK);

            // Rotate to look at NPC
            auto& actorTrans = registry->get<sgTransform>(self);
            const auto& npcTrans = registry->get<sgTransform>(playerDiag.dialogTarget);
            Vector3 direction = Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos());
            direction = Vector3Normalize(direction);
            const float angle = atan2f(direction.x, direction.z);
            actorTrans.SetRotation({actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});

            gameData->dialogSystem->StartConversation(npcTrans, playerDiag.dialogTarget);
        }

        void OnStateExit(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited InDialogState. \n", static_cast<int>(self));
#endif
            auto& playerDiag = registry->get<DialogComponent>(self);
            registry->get<Animation>(playerDiag.dialogTarget).ChangeAnimationByEnum(AnimationEnum::IDLE);
            playerDiag.dialogTarget = entt::null;
        }

        ~InDialogState() override = default;

        InDialogState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::MovingToAttackEnemyState : public StateMachine
    {

        void onAttackCancelled(entt::entity self, entt::entity) const
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Combat);
        }

      public:
        void OnStateEnter(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered MovingToAttackEnemyState. \n", static_cast<int>(self));
#endif
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::WALK);

            auto& moveableActor = registry->get<MoveableActor>(self);

            auto& state = registry->get<PlayerState>(self);
            entt::sink sink{moveableActor.onDestinationReached};
            state.currentStateConnections.push_back(
                sink.connect<&MovingToAttackEnemyState::onTargetReached>(this));

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            auto& controllable = registry->get<ControllableActor>(self);
            entt::sink attackCancelSink{controllable.onFloorClick};
            state.currentStateConnections.push_back(
                attackCancelSink.connect<&MovingToAttackEnemyState::onAttackCancelled>(this));

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 playerPos = registry->get<sgTransform>(self).GetWorldPos();
            Vector3 enemyPos = enemyTrans.GetWorldPos();
            Vector3 direction = Vector3Subtract(enemyPos, playerPos);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            gameData->actorMovementSystem->PathfindToLocation(self, targetPos);
        }

        void OnStateExit(entt::entity self) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited MovingToAttackEnemyState. \n", static_cast<int>(self));
#endif
        }

        ~MovingToAttackEnemyState() override = default;

        MovingToAttackEnemyState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::CombatState : public StateMachine
    {

        void onAttackCancelled(entt::entity self, entt::entity x)
        {
            // Both outcomes are the same
            onTargetDeath(self, entt::null);
        }

        void onTargetDeath(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
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

        void OnStateEnter(entt::entity entity) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Entered CombatState. \n", static_cast<int>(entity));
#endif
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.publish(abilityEntity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);

            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);

            combatable.onTargetDeathHookId = gameData->reflectionSignalRouter->CreateHook<entt::entity>(
                entity, enemyCombatable.onDeath, combatable.onTargetDeath);

            auto& state = registry->get<PlayerState>(entity);
            entt::sink sink{combatable.onTargetDeath};
            state.currentStateConnections.push_back(sink.connect<&CombatState::onTargetDeath>(this));

            auto& controllable = registry->get<ControllableActor>(entity);
            entt::sink attackCancelSink{controllable.onFloorClick};
            state.currentStateConnections.push_back(
                attackCancelSink.connect<&CombatState::onAttackCancelled>(this));
        }

        void OnStateExit(entt::entity entity) override
        {
#ifdef SAGE_DEBUG
            std::cout << std::format("Object {}: Exited CombatState. \n", static_cast<int>(entity));
#endif
            auto& combatable = registry->get<CombatableActor>(entity);
            gameData->reflectionSignalRouter->RemoveHook(combatable.onTargetDeathHookId);
            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.publish(abilityEntity);
        }

        ~CombatState() override = default;
        CombatState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    void PlayerStateController::Update()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            const auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void PlayerStateController::Draw3D()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            const auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    PlayerStateController::PlayerStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry)
    {

        states[PlayerStateEnum::Default] = std::make_unique<DefaultState>(_registry, _gameData);
        states[PlayerStateEnum::MovingToAttackEnemy] =
            std::make_unique<MovingToAttackEnemyState>(_registry, _gameData);
        states[PlayerStateEnum::Combat] = std::make_unique<CombatState>(_registry, _gameData);
        states[PlayerStateEnum::MovingToTalkToNPC] =
            std::make_unique<MovingToTalkToNPCState>(_registry, _gameData);
        states[PlayerStateEnum::InDialog] = std::make_unique<InDialogState>(_registry, _gameData);
        states[PlayerStateEnum::FollowingLeader] = std::make_unique<FollowingLeaderState>(_registry, _gameData);
        states[PlayerStateEnum::MovingToLocation] = std::make_unique<MovingToLocationState>(_registry, _gameData);
        states[PlayerStateEnum::WaitingForLeader] = std::make_unique<WaitingForLeaderState>(_registry, _gameData);
        states[PlayerStateEnum::DestinationUnreachable] =
            std::make_unique<DestinationUnreachableState>(_registry, _gameData);
    }
} // namespace sage