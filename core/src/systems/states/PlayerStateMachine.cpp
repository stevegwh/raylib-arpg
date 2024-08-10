#include "PlayerStateMachine.hpp"

#include "abilities/PlayerAutoAttack.hpp"
#include "components/Animation.hpp"
#include "components/sgTransform.hpp"
#include "GameData.hpp"

#include <cassert>

#include "raylib.h"

namespace sage
{
    namespace playerstates
    {
        // ----------------------------
        void DefaultState::Update()
        {
            // I think having the player "aggro" when the enemy hits them is bad.
        }

        void DefaultState::Draw3D()
        {
        }

        void DefaultState::onEnemyClick(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            ChangeState<StatePlayerApproachingTarget, PlayerStates>(self);
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {
            auto& combatableActor = registry->get<CombatableActor>(entity);
            entt::sink sink{combatableActor.onEnemyClicked};
            sink.connect<&DefaultState::onEnemyClick>(this);

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void DefaultState::OnStateExit(entt::entity entity)
        {
            auto& combatableActor = registry->get<CombatableActor>(entity);
            entt::sink sink{combatableActor.onEnemyClicked};
            sink.disconnect<&DefaultState::onEnemyClick>(this);
        }

        DefaultState::DefaultState(entt::registry* _registry) : StateMachine(_registry)
        {
        }

        // ----------------------------

        void ApproachingTargetState::onTargetReached(entt::entity self)
        {
            ChangeState<StatePlayerCombat, PlayerStates>(self);
        }

        void ApproachingTargetState::onAttackCancel(entt::entity self)
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            ChangeState<StatePlayerDefault, PlayerStates>(self);
        }

        void ApproachingTargetState::onEnemyClick(entt::entity self, entt::entity target)
        {
            ChangeState<StatePlayerDefault, PlayerStates>(self);
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            ChangeState<StatePlayerApproachingTarget, PlayerStates>(self);
        }

        void ApproachingTargetState::Update()
        {
        }

        void ApproachingTargetState::OnStateEnter(entt::entity self)
        {
            // TODO: Below is being set in ControllableActorSystem. State should only be
            // managed here.
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.connect<&ApproachingTargetState::onTargetReached>(this);

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            entt::sink attackCancelledSink{combatable.onAttackCancelled};
            attackCancelledSink.connect<&ApproachingTargetState::onAttackCancel>(this);

            entt::sink enemyClickedSink{combatable.onEnemyClicked};
            enemyClickedSink.disconnect<&ApproachingTargetState::onEnemyClick>(this);

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 enemyPos = enemyTrans.position();
            Vector3 direction = Vector3Subtract(enemyPos, playerTrans.position());
            float length = Vector3Length(direction);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            gameData->controllableActorSystem->PathfindToLocation(self, targetPos);
        }

        void ApproachingTargetState::OnStateExit(entt::entity self)
        {
            gameData->controllableActorSystem->CancelMovement(self);

            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.disconnect<&ApproachingTargetState::onTargetReached>(this);

            auto& combatable = registry->get<CombatableActor>(self);
            entt::sink combatableSink{combatable.onAttackCancelled};
            combatableSink.disconnect<&ApproachingTargetState::onAttackCancel>(this);
        }

        ApproachingTargetState::ApproachingTargetState(
            entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

        void CombatState::onTargetDeath(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            ChangeState<StatePlayerDefault, PlayerStates>(self);
        }

        void CombatState::onAttackCancel(entt::entity self)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            ChangeState<StatePlayerDefault, PlayerStates>(self);
        }

        bool CombatState::checkInCombat(entt::entity entity)
        {
            // Might do more here later
            return true;
        }

        void CombatState::onEnemyClick(entt::entity self, entt::entity target)
        {
            ChangeState<StatePlayerDefault, PlayerStates>(self);
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            ChangeState<StatePlayerApproachingTarget, PlayerStates>(self);
        }

        void CombatState::Update()
        {
            auto view = registry->view<CombatableActor, StatePlayerCombat>();
            for (const auto& entity : view)
            {
                // auto& combatable = registry->get<CombatableActor>(entity);
                // if (!checkInCombat(entity))
                // {
                //     ChangeState<StatePlayerDefault, PlayerStates>(entity);
                //     continue;
                // }

                auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
                autoAttackAbility.Update(entity);
            }
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

            entt::sink attackCancelSink{combatable.onAttackCancelled};
            attackCancelSink.connect<&CombatState::onAttackCancel>(this);

            entt::sink enemyClickedSink{combatable.onEnemyClicked};
            enemyClickedSink.connect<&CombatState::onEnemyClick>(this);
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

            entt::sink attackCancelSink{combatable.onAttackCancelled};
            attackCancelSink.disconnect<&CombatState::onAttackCancel>(this);

            entt::sink enemyClickedSink{combatable.onEnemyClicked};
            enemyClickedSink.disconnect<&CombatState::onEnemyClick>(this);

            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Cancel(entity);
        }

        CombatState::CombatState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------
    } // namespace playerstates

    void PlayerStateController::Update()
    {
        for (auto& system : systems)
        {
            system->Update();
        }
    }

    void PlayerStateController::Draw3D()
    {
        for (auto& system : systems)
        {
            system->Draw3D();
        }
    }

    PlayerStateController::PlayerStateController(
        entt::registry* _registry, GameData* _gameData)
    {
        defaultState = std::make_unique<playerstates::DefaultState>(_registry);
        approachingTargetState =
            std::make_unique<playerstates::ApproachingTargetState>(_registry, _gameData);
        engagedInCombatState =
            std::make_unique<playerstates::CombatState>(_registry, _gameData);

        systems.push_back(defaultState.get());
        systems.push_back(approachingTargetState.get());
        systems.push_back(engagedInCombatState.get());
    }
} // namespace sage