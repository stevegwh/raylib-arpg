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

#define SAGE_DEBUG

namespace sage
{

    class PlayerStateController::DefaultState : public StateMachine
    {
        void onFloorClick(const entt::entity self, entt::entity x) const
        {
            // TODO: Should have its own state (moving to target), or something
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;

            // Necessary because this function is active in all states.
            // TODO: *Must* remove "hanging" events so that it falls more in line with a proper FSM.
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);

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

        void onEnemyRightClick(entt::entity self, entt::entity target) const
        {
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
        }

        void onMovementCancel(entt::entity self) const
        {
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
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
            auto& moveable = registry->get<MoveableActor>(entity);
            entt::sink leftSink{controllable.onEnemyLeftClick};
            leftSink.connect<&DefaultState::onEnemyLeftClick>(this);
            entt::sink rightSink{controllable.onEnemyRightClick};
            rightSink.connect<&DefaultState::onEnemyRightClick>(this);
            entt::sink npcSink{controllable.onNPCLeftClick};
            npcSink.connect<&DefaultState::onNPCLeftClick>(this);
            entt::sink floorClickSink{controllable.onFloorClick};
            floorClickSink.connect<&DefaultState::onFloorClick>(this);
            entt::sink movementCancelSink{moveable.onMovementCancel};
            movementCancelSink.connect<&DefaultState::onMovementCancel>(this);
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

    class PlayerStateController::FollowingLeaderState final : public StateMachine
    {

        void onMovementCancelled(const entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetPosUpdate(const entt::entity self, const entt::entity target) const
        {
            const auto& targetPos = registry->get<sgTransform>(target).GetWorldPos();
            gameData->actorMovementSystem->PathfindToLocation(self, targetPos);
        }

        void onTargetReached(const entt::entity self) const
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
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
            const auto& targetPos = registry->get<sgTransform>(target).GetWorldPos();
            gameData->actorMovementSystem->PathfindToLocation(self, targetPos);

            auto& state = registry->get<PlayerState>(self);
            entt::sink sink1{moveable.onDestinationReached};
            state.currentStateConnections.push_back(sink1.connect<&FollowingLeaderState::onTargetReached>(this));
            entt::sink sink2{moveable.followTarget->onPositionUpdate};
            state.currentStateConnections.push_back(sink2.connect<&FollowingLeaderState::onTargetPosUpdate>(this));
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

    class PlayerStateController::MovingToTalkToNPCState : public StateMachine
    {

        void onMovementCancelled(entt::entity self) const
        {
            auto& dialogComponent = registry->get<DialogComponent>(self);
            dialogComponent.dialogTarget = entt::null;
            auto& playerState = registry->get<PlayerState>(self);
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
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
    }
} // namespace sage