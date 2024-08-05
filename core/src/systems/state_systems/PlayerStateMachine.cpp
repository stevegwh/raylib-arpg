#include "PlayerStateMachine.hpp"

#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include "Cursor.hpp"
#include "TimerManager.hpp"

#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"
#include "components/sgTransform.hpp"
#include "abilities/PlayerAutoAttack.hpp"

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

        void DefaultState::OnEnemyClick(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            ChangeState<StatePlayerApproachingTarget, PlayerStates>(self);
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {
            auto& combatableActor = registry->get<CombatableActor>(entity);
            entt::sink sink {combatableActor.onEnemyClicked};
            sink.connect<&DefaultState::OnEnemyClick>(this);

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void DefaultState::OnStateExit(entt::entity entity)
        {
            auto& combatableActor = registry->get<CombatableActor>(entity);
            entt::sink sink {combatableActor.onEnemyClicked};
            sink.disconnect<&DefaultState::OnEnemyClick>(this);
        }

        DefaultState::DefaultState(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem)
            : StateMachine(_registry), actorMovementSystem(_actorMovementSystem)
        {
        }

// ----------------------------

        void ApproachingTargetState::Update()
        {
            auto view = registry->view<CombatableActor, StatePlayerApproachingTarget>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (combatable.target == entt::null)
                {
                    ChangeState<StatePlayerDefault, PlayerStates>(entity);
                    continue;
                }
            }
        }

        void ApproachingTargetState::OnEnemyClick(entt::entity self, entt::entity target)
        {

        }

        void ApproachingTargetState::OnReachedTarget(entt::entity self)
        {
            ChangeState<StatePlayerEngagedInCombat, PlayerStates>(self);
        }

        void ApproachingTargetState::onAttackCancel(entt::entity self)
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            
            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.disconnect<&ApproachingTargetState::OnReachedTarget>(this);
            
            controllableActorSystem->CancelMovement(self);
            ChangeState<StatePlayerDefault, PlayerStates>(self);
        }

        void ApproachingTargetState::OnStateEnter(entt::entity self)
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.connect<&ApproachingTargetState::OnReachedTarget>(this);

            auto& combatable = registry->get<CombatableActor>(self);
            entt::sink combatableSink{combatable.onAttackCancelled};
            combatableSink.connect<&ApproachingTargetState::onAttackCancel>(this);

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 enemyPos = enemyTrans.position();
            Vector3 direction = Vector3Subtract(enemyPos, playerTrans.position());
            float length = Vector3Length(direction);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            controllableActorSystem->PathfindToLocation(self, targetPos);
        }

        void ApproachingTargetState::OnStateExit(entt::entity self)
        {
            controllableActorSystem->CancelMovement(self);

            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.disconnect<&ApproachingTargetState::OnReachedTarget>(this);

            auto& combatable = registry->get<CombatableActor>(self);
            entt::sink combatableSink{combatable.onAttackCancelled};
            combatableSink.disconnect<&ApproachingTargetState::onAttackCancel>(this);
        }

        ApproachingTargetState::ApproachingTargetState(entt::registry* _registry, ControllableActorSystem* _controllableActorSystem)
            : StateMachine(_registry), controllableActorSystem(_controllableActorSystem) {}

// ----------------------------

        void EngagedInCombatState::Update()
        {
            auto view = registry->view<CombatableActor, StatePlayerEngagedInCombat>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (!checkInCombat(entity))
                {
                    auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
                    autoAttackAbility.Cancel();
                    ChangeState<StatePlayerDefault, PlayerStates>(entity);
                    continue;
                }

                auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
                autoAttackAbility.Update(entity);
            }
        }

        void EngagedInCombatState::onTargetDeath(entt::entity self, entt::entity target)
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            ChangeState<StatePlayerDefault, PlayerStates>(self);
        }

        void EngagedInCombatState::onAttackCancel(entt::entity self)
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(self);
            autoAttackAbility.Cancel();
            ChangeState<StatePlayerDefault, PlayerStates>(self);
        }

        bool EngagedInCombatState::checkInCombat(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            return combatable.target != entt::null;
        }

        void EngagedInCombatState::OnStateEnter(entt::entity entity)
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(entity);
            autoAttackAbility.Init(entity);

            auto& combatable = registry->get<CombatableActor>(entity);
            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
            entt::sink sink{enemyCombatable.onDeath};
            sink.connect<&CombatableActor::TargetDeath>(combatable);

            entt::sink combatableSink{combatable.onTargetDeath};
            combatableSink.connect<&EngagedInCombatState::onTargetDeath>(this);

            entt::sink attackCancelSink{combatable.onAttackCancelled};
            attackCancelSink.connect<&EngagedInCombatState::onAttackCancel>(this);
        }

        void EngagedInCombatState::OnStateExit(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            if (combatable.target != entt::null)
            {
                auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
                entt::sink sink{enemyCombatable.onDeath};
                sink.disconnect<&CombatableActor::TargetDeath>(combatable);
            }

            entt::sink combatableSink{combatable.onTargetDeath};
            combatableSink.disconnect<&EngagedInCombatState::onTargetDeath>(this);

            entt::sink attackCancelSink{combatable.onAttackCancelled};
            attackCancelSink.disconnect<&EngagedInCombatState::onAttackCancel>(this);
        }

        EngagedInCombatState::EngagedInCombatState(entt::registry* _registry) : StateMachine(_registry) {}
        
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

    PlayerStateController::PlayerStateController(entt::registry* _registry, Cursor* _cursor,
                                                 ControllableActorSystem* _controllableActorSystem,
                                                 ActorMovementSystem* _actorMovementSystem,
                                                 CollisionSystem* _collisionSystem,
                                                 NavigationGridSystem* _navigationGridSystem,
                                                 TimerManager* _timerManager)
        : registry(_registry), cursor(_cursor), controllableActorSystem(_controllableActorSystem)
    {
        defaultState = std::make_unique<playerstates::DefaultState>(_registry, _actorMovementSystem);
        approachingTargetState = std::make_unique<playerstates::ApproachingTargetState>(_registry, _controllableActorSystem);
        engagedInCombatState = std::make_unique<playerstates::EngagedInCombatState>(_registry);

        systems.push_back(defaultState.get());
        systems.push_back(approachingTargetState.get());
        systems.push_back(engagedInCombatState.get());
    }
} // namespace sage