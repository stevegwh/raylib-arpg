#include "PlayerStateMachine.hpp"

#include "GameData.hpp"

#include "abilities/Abilities.hpp"
#include "components/Animation.hpp"
#include "components/ControllableActor.hpp"
#include "components/MovableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "EntityReflectionSignalRouter.hpp"

#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include <cassert>

#include "raylib.h"

namespace sage
{
    class PlayerStateController::DefaultState : public StateMachine
    {
        GameData* gameData;

        void onFloorClick(entt::entity self, entt::entity x)
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;

            gameData->controllableActorSystem->CancelMovement(self); // Flush any previous commands
            gameData->controllableActorSystem->PathfindToLocation(self, gameData->cursor->collision().point);
        }

        void onEnemyClick(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
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
            auto& controllable = registry->get<ControllableActor>(entity);
            entt::sink sink{controllable.onEnemyClicked};
            sink.connect<&DefaultState::onEnemyClick>(this);

            auto& combatableActor = registry->get<CombatableActor>(entity);

            // Bridge was created in GameObjectFactory to connect this to cursor
            entt::sink floorClickSink{combatableActor.onAttackCancelled};
            floorClickSink.connect<&DefaultState::onFloorClick>(this);
            // ----------------------------

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        virtual ~DefaultState() = default;

        DefaultState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry), gameData(_gameData)
        {
        }
    };

    class PlayerStateController::MovingToTalkToNPCState : public StateMachine
    {
        GameData* gameData;
        void onTargetReached(entt::entity self);

      public:
        void Update(entt::entity entity) override;
        void OnStateEnter(entt::entity entity) override;
        void OnStateExit(entt::entity entity) override;
        virtual ~MovingToTalkToNPCState() = default;
        MovingToTalkToNPCState(entt::registry* _registry, GameData* gameData);
    };

    // ----------------------------

    class PlayerStateController::MovingToAttackEnemyState : public StateMachine
    {
        GameData* gameData;

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
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

            auto& moveableActor = registry->get<MoveableActor>(self);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.connect<&MovingToAttackEnemyState::onTargetReached>(this);

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 playerPos = registry->get<sgTransform>(self).position();
            Vector3 enemyPos = enemyTrans.position();
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
        }

        virtual ~MovingToAttackEnemyState() = default;

        MovingToAttackEnemyState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::CombatState : public StateMachine
    {
        GameData* gameData;
        int onTargetDeathBridge;

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
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Update(entity);
        }

        void OnStateEnter(entt::entity entity) override
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Init(entity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);

            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);

            onTargetDeathBridge = gameData->signalReflectionManager->CreateHook<entt::entity>(
                entity, enemyCombatable.onDeath, combatable.onTargetDeath);
            entt::sink sink{combatable.onTargetDeath};
            sink.connect<&CombatState::onTargetDeath>(this);
        }

        void OnStateExit(entt::entity entity) override
        {
            auto& combatable = registry->get<CombatableActor>(entity);

            gameData->signalReflectionManager->RemoveHook(onTargetDeathBridge);
            entt::sink sink{combatable.onTargetDeath};
            sink.disconnect<&CombatState::onTargetDeath>(this);

            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Cancel(entity);
        }

        virtual ~CombatState() = default;
        CombatState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry), gameData(_gameData)
        {
        }
    };

    // ----------------------------

    StateMachine* PlayerStateController::GetSystem(PlayerStateEnum state)
    {
        switch (state)
        {
        case PlayerStateEnum::Default:
            return defaultState;
        case PlayerStateEnum::MovingToAttackEnemy:
            return approachingTargetState;
        case PlayerStateEnum::MovingToTalkToNPC:
            return nullptr;
        case PlayerStateEnum::Combat:
            return combatState;
        default:
            return defaultState;
        }
    }

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
        delete defaultState;
        delete approachingTargetState;
        delete combatState;
    }

    PlayerStateController::PlayerStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry),
          defaultState(new DefaultState(_registry, _gameData)),
          approachingTargetState(new MovingToAttackEnemyState(_registry, _gameData)),
          combatState(new CombatState(_registry, _gameData))
    {
    }
} // namespace sage