#include "PlayerStateMachine.hpp"

#include "GameData.hpp"

#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "EntityReflectionSignalRouter.hpp"

#include "AbilityFactory.hpp"
#include "Camera.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include <cassert>

#include "components/DialogComponent.hpp"
#include "raylib.h"
#include "systems/DialogSystem.hpp"
#include "systems/PartySystem.hpp"

namespace sage
{

    class PlayerStateController::DefaultState : public StateMachine
    {
        void onFloorClick(entt::entity self, entt::entity x) const
        {
            gameData->controllableActorSystem->PathfindToLocation(
                self, gameData->cursor->getFirstCollision().point);
        }

        void onNPCLeftClick(entt::entity self, entt::entity target) const
        {
            if (self != gameData->controllableActorSystem->GetSelectedActor()) return;
            if (!registry->any_of<DialogComponent>(target)) return;

            auto& moveable = registry->get<MoveableActor>(self);
            moveable.targetActor = target;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToTalkToNPC);
        }

        void onEnemyLeftClick(entt::entity self, entt::entity target) const
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
        }

        void onEnemyRightClick(entt::entity self, entt::entity target) const
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
        }

        void onMovementCancel(entt::entity self) const
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

      public:
        void Update(entt::entity entity) override
        {
            // Should check here if should be in combat
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
            // Below are not disconnected in OnStateExit
            // Bridge was created in GameObjectFactory to connect this to cursor
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

        ~DefaultState() override = default;

        DefaultState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
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
            auto& moveable = registry->get<MoveableActor>(self);
            auto& playerDiag = registry->get<DialogComponent>(self);
            playerDiag.dialogTarget = moveable.targetActor;
            const auto& pos = registry->get<DialogComponent>(playerDiag.dialogTarget).conversationPos;
            gameData->controllableActorSystem->PathfindToLocation(self, pos);

            entt::sink sink{moveable.onFinishMovement};
            sink.connect<&MovingToTalkToNPCState::onTargetReached>(this);

            entt::sink sink2{moveable.onMovementCancel};
            sink2.connect<&MovingToTalkToNPCState::onMovementCancelled>(this);
        }

        void OnStateExit(entt::entity self) override
        {
            auto& moveable = registry->get<MoveableActor>(self);
            entt::sink sink{moveable.onFinishMovement};
            sink.disconnect<&MovingToTalkToNPCState::onTargetReached>(this);
            entt::sink sink2{moveable.onMovementCancel};
            sink2.disconnect<&MovingToTalkToNPCState::onMovementCancelled>(this);
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
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::WALK);

            auto& moveableActor = registry->get<MoveableActor>(self);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.connect<&MovingToAttackEnemyState::onTargetReached>(this);

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            auto& controllable = registry->get<ControllableActor>(self);
            entt::sink attackCancelSink{controllable.onFloorClick};
            attackCancelSink.connect<&MovingToAttackEnemyState::onAttackCancelled>(this);

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 playerPos = registry->get<sgTransform>(self).GetWorldPos();
            Vector3 enemyPos = enemyTrans.GetWorldPos();
            Vector3 direction = Vector3Subtract(enemyPos, playerPos);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            gameData->controllableActorSystem->PathfindToLocation(self, targetPos);
        }

        void OnStateExit(entt::entity self) override
        {
            auto& moveableActor = registry->get<MoveableActor>(self);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.disconnect<&MovingToAttackEnemyState::onTargetReached>(this);

            auto& controllable = registry->get<ControllableActor>(self);
            entt::sink attackCancelSink{controllable.onFloorClick};
            attackCancelSink.disconnect<&MovingToAttackEnemyState::onAttackCancelled>(this);
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
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.publish(abilityEntity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);

            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);

            combatable.onTargetDeathHookId = gameData->reflectionSignalRouter->CreateHook<entt::entity>(
                entity, enemyCombatable.onDeath, combatable.onTargetDeath);

            entt::sink sink{combatable.onTargetDeath};
            sink.connect<&CombatState::onTargetDeath>(this);

            auto& controllable = registry->get<ControllableActor>(entity);
            entt::sink attackCancelSink{controllable.onFloorClick};
            attackCancelSink.connect<&CombatState::onAttackCancelled>(this);
        }

        void OnStateExit(entt::entity entity) override
        {
            auto& combatable = registry->get<CombatableActor>(entity);

            gameData->reflectionSignalRouter->RemoveHook(combatable.onTargetDeathHookId);
            entt::sink sink{combatable.onTargetDeath};
            sink.disconnect<&CombatState::onTargetDeath>(this);

            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.publish(abilityEntity);

            auto& controllable = registry->get<ControllableActor>(entity);
            entt::sink attackCancelSink{controllable.onFloorClick};
            attackCancelSink.disconnect<&CombatState::onAttackCancelled>(this);
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
    }
} // namespace sage