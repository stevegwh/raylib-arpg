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
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include <cassert>

#include "raylib.h"

namespace sage
{

    class PlayerStateController::DefaultState : public StateMachine
    {
        void onFloorClick(entt::entity self, entt::entity x)
        {
            gameData->controllableActorSystem->PathfindToLocation(self, gameData->cursor->getFirstCollision().point);
        }

        void onEnemyLeftClick(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
        }

        void onEnemyRightClick(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
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
            entt::sink leftSink{controllable.onEnemyLeftClick};
            leftSink.connect<&DefaultState::onEnemyLeftClick>(this);
            entt::sink rightSink{controllable.onEnemyRightClick};
            rightSink.connect<&DefaultState::onEnemyRightClick>(this);
            entt::sink floorClickSink{controllable.onFloorClick};
            floorClickSink.connect<&DefaultState::onFloorClick>(this);
            // ----------------------------

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    class PlayerStateController::MovingToTalkToNPCState : public StateMachine
    {
        void onTargetReached(entt::entity self);

      public:
        void Update(entt::entity entity) override;
        void OnStateEnter(entt::entity entity) override;
        void OnStateExit(entt::entity entity) override;
        ~MovingToTalkToNPCState() override = default;
        MovingToTalkToNPCState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::MovingToAttackEnemyState : public StateMachine
    {

        void onAttackCancelled(entt::entity self, entt::entity x)
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self)
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Combat);
        }

      public:
        void OnStateEnter(entt::entity self) override
        {
            gameData->actorMovementSystem->CancelMovement(self); // Flush queue

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
            float length = Vector3Length(direction);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            gameData->controllableActorSystem->PathfindToLocation(self, targetPos);
        }

        void OnStateExit(entt::entity self) override
        {
            gameData->controllableActorSystem->CancelMovement(self);

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
        int onTargetDeathBridge;

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

            onTargetDeathBridge = gameData->reflectionSignalRouter->CreateHook<entt::entity>(
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

            gameData->reflectionSignalRouter->RemoveHook(onTargetDeathBridge);
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
        auto view = registry->view<PlayerState>();
        for (const auto& entity : view)
        {
            auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void PlayerStateController::Draw3D()
    {
        auto view = registry->view<PlayerState>();
        for (const auto& entity : view)
        {
            auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    PlayerStateController::~PlayerStateController()
    {
    }

    PlayerStateController::PlayerStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry)
    {

        states[PlayerStateEnum::Default] = std::make_unique<DefaultState>(_registry, _gameData);
        states[PlayerStateEnum::MovingToAttackEnemy] =
            std::make_unique<MovingToAttackEnemyState>(_registry, _gameData);
        states[PlayerStateEnum::Combat] = std::make_unique<CombatState>(_registry, _gameData);
    }
} // namespace sage