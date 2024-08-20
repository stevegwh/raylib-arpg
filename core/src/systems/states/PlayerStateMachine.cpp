#include "PlayerStateMachine.hpp"

#include "GameData.hpp"

#include "abilities/Abilities.hpp"
#include "components/Animation.hpp"
#include "components/ControllableActor.hpp"
#include "components/MovableActor.hpp"
#include "components/sgTransform.hpp"

#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include <cassert>

#include "raylib.h"

namespace sage
{
    namespace playerstates
    {
        // ----------------------------
        void DefaultState::Update(entt::entity entity)
        {
            // Should check here if should be in combat
        }

        void DefaultState::Draw3D(entt::entity entity)
        {
        }

        void DefaultState::onFloorClick(entt::entity self)
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;

            gameData->controllableActorSystem->CancelMovement(self); // Flush any previous commands
            gameData->controllableActorSystem->PathfindToLocation(self, gameData->cursor->collision().point);
        }

        void DefaultState::onEnemyClick(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {
            // TODO: Unsure if this causes problems if "connect" is called multiple times.
            // Might have to first disconnect

            // Below are not disconnected in OnStateExit
            auto& controllable = registry->get<ControllableActor>(entity);
            entt::sink sink{controllable.onEnemyClicked};
            sink.connect<&DefaultState::onEnemyClick>(this);

            auto& combatableActor = registry->get<CombatableActor>(entity);
            entt::sink floorClickSink{combatableActor.onAttackCancelled};
            floorClickSink.connect<&DefaultState::onFloorClick>(this);
            // ----------------------------

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        DefaultState::DefaultState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

        void MovingToAttackEnemyState::onTargetReached(entt::entity self)
        {
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Combat);
        }

        void MovingToAttackEnemyState::OnStateEnter(entt::entity self)
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

        void MovingToAttackEnemyState::OnStateExit(entt::entity self)
        {
            gameData->controllableActorSystem->CancelMovement(self);

            auto& moveableActor = registry->get<MoveableActor>(self);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.disconnect<&MovingToAttackEnemyState::onTargetReached>(this);
        }

        MovingToAttackEnemyState::MovingToAttackEnemyState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

        void CombatState::onTargetDeath(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            auto& playerState = registry->get<PlayerState>(self);
            playerState.ChangeState(self, PlayerStateEnum::Default);
        }

        bool CombatState::checkInCombat(entt::entity entity)
        {
            // Might do more here later
            return true;
        }

        void CombatState::Update(entt::entity entity)
        {
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Update(entity);
        }

        void CombatState::OnStateEnter(entt::entity entity)
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Init(entity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);
            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
            entt::sink sink{enemyCombatable.onDeath};
            sink.connect<&CombatableActor::TargetDeath>(combatable);

            entt::sink combatableSink{combatable.onTargetDeath};
            combatableSink.connect<&CombatState::onTargetDeath>(this);
        }

        void CombatState::OnStateExit(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            if (combatable.target != entt::null)
            {
                auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
                entt::sink sink{enemyCombatable.onDeath};
                sink.disconnect<&CombatableActor::TargetDeath>(combatable);
            }

            entt::sink combatableSink{combatable.onTargetDeath};
            combatableSink.disconnect<&CombatState::onTargetDeath>(this);

            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Cancel(entity);
        }

        CombatState::CombatState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------
    } // namespace playerstates

    StateMachine* PlayerStateController::GetSystem(PlayerStateEnum state)
    {
        switch (state)
        {
        case PlayerStateEnum::Default:
            return defaultState.get();
        case PlayerStateEnum::MovingToAttackEnemy:
            return approachingTargetState.get();
        case PlayerStateEnum::MovingToTalkToNPC:
            return nullptr;
        case PlayerStateEnum::Combat:
            return combatState.get();
        default:
            return defaultState.get();
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

    PlayerStateController::PlayerStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry),
          defaultState(std::make_unique<playerstates::DefaultState>(_registry, _gameData)),
          approachingTargetState(std::make_unique<playerstates::MovingToAttackEnemyState>(_registry, _gameData)),
          combatState(std::make_unique<playerstates::CombatState>(_registry, _gameData))
    {
    }
} // namespace sage