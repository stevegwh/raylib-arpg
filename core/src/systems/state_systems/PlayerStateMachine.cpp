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
        void DefaultState::Update()
        {
        }

        void DefaultState::Draw3D()
        {
        }

        void DefaultState::OnComponentAdded(entt::entity entity)
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void DefaultState::OnComponentRemoved(entt::entity entity)
        {
            actorMovementSystem->CancelMovement(entity);
        }

        DefaultState::DefaultState(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem)
            : StateMachine(_registry), actorMovementSystem(_actorMovementSystem)
        {
        }

        // TODO: Does not account for an enemy being out of range of the player when auto attacking
        // TODO: Click enemy -> Attack Cancel -> click enemy causes as a crash once the player reaches the enemy

        void InCombatState::Update()
        {
            auto view = registry->view<CombatableActor, StatePlayerCombat>();

            for (const auto& entity : view)
            {
                auto& c = registry->get<CombatableActor>(entity);
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

        void InCombatState::Draw3D()
        {
        }

        bool InCombatState::checkInCombat(entt::entity entity)
        {
            // If the entity is not the target of any other combatable.
            // If no current target
            // Have a timer for aggro and if the player is not within that range for a certain amount of time they
            // resume their regular task (tasks TBC)

            // TODO: Should only go out of combat if no enemies are targetting the player

            auto& combatable = registry->get<CombatableActor>(entity);
            if (combatable.target == entt::null)
            {
                return false;
            }
            return true;
        }

        void InCombatState::onDeath(entt::entity entity)
        {
        }

        void InCombatState::onTargetDeath(entt::entity self, entt::entity target)
        {
            auto& enemyCombatable = registry->get<CombatableActor>(target);
            auto& playerCombatable = registry->get<CombatableActor>(self);
            {
                entt::sink sink{playerCombatable.onTargetDeath};
                sink.disconnect<&InCombatState::onTargetDeath>(this);
            }
            {
                entt::sink sink{playerCombatable.onAttackCancelled};
                sink.disconnect<&InCombatState::onAttackCancel>(this);
            }
            playerCombatable.target = entt::null;
        }

        void InCombatState::onAttackCancel(entt::entity self)
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            auto& playerTrans = registry->get<sgTransform>(self);
            {
                entt::sink sink{playerTrans.onFinishMovement};
                sink.disconnect<&InCombatState::startCombat>(this);
            }
            controllableActorSystem->CancelMovement(self);
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(self);
            autoAttackAbility.Cancel();
        }

        void InCombatState::OnHit(AttackData attackData)
        {
            if (!checkInCombat(attackData.hit))
            {
                // OnEnemyClick(attackData.hit, attackData.attacker);
            }
        }

        void InCombatState::OnEnemyClick(entt::entity self, entt::entity target)
        {
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(self);
            autoAttackAbility.Cancel();

            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            {
                entt::sink sink{combatable.onAttackCancelled};
                sink.connect<&InCombatState::onAttackCancel>(this);
            }
            auto& playerTrans = registry->get<sgTransform>(self);
            const auto& enemyTrans = registry->get<sgTransform>(target);

            const auto& enemyCollideable = registry->get<Collideable>(combatable.target);
            Vector3 enemyPos = enemyTrans.position();

            // Calculate the direction vector from player to enemy
            Vector3 direction = Vector3Subtract(enemyPos, playerTrans.position());

            // Normalize the direction vector
            float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
            direction.x = (direction.x / length) * combatable.attackRange;
            direction.y = (direction.y / length) * combatable.attackRange;
            direction.z = (direction.z / length) * combatable.attackRange;

            // Calculate the target position by subtracting the normalized direction vector
            // multiplied by the attack range from the enemy position
            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            controllableActorSystem->PathfindToLocation(self, targetPos);
            {
                entt::sink sink{playerTrans.onFinishMovement};
                sink.connect<&InCombatState::startCombat>(this);
            }
        }

        void InCombatState::startCombat(entt::entity self)
        {
            // Remember to set target
            {
                auto& playerTrans = registry->get<sgTransform>(self);
                entt::sink sink{playerTrans.onFinishMovement};
                sink.disconnect<&InCombatState::startCombat>(this);
            }
            ChangeState<StatePlayerCombat, PlayerStates>(self);

            auto& playerCombatable = registry->get<CombatableActor>(self);
            auto& enemyCombatable = registry->get<CombatableActor>(playerCombatable.target);
            {
                entt::sink sink{enemyCombatable.onDeath};
                sink.connect<&CombatableActor::TargetDeath>(playerCombatable);
            }
            {
                entt::sink sink{playerCombatable.onTargetDeath};
                sink.connect<&InCombatState::onTargetDeath>(this);
            }
        }

        void InCombatState::Enable()
        {
            auto view = registry->view<CombatableActor>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (combatable.actorType == CombatableActorType::PLAYER)
                {
                    {
                        entt::sink sink{combatable.onEnemyClicked};
                        sink.connect<&InCombatState::OnEnemyClick>(this);
                    }
                    {
                        entt::sink sink{combatable.onAttackCancelled};
                        sink.connect<&InCombatState::onAttackCancel>(this);
                    }
                    {
                        entt::sink sink{combatable.onHit};
                        sink.connect<&InCombatState::OnHit>(this);
                    }
                }
            }
        }

        void InCombatState::Disable()
        {
            // Remove checks to see if the player should be in combat
            auto view = registry->view<CombatableActor>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (combatable.actorType == CombatableActorType::PLAYER)
                {
                    {
                        entt::sink sink{combatable.onEnemyClicked};
                        sink.disconnect<&InCombatState::OnEnemyClick>(this);
                    }
                    {
                        entt::sink sink{combatable.onAttackCancelled};
                        sink.disconnect<&InCombatState::onAttackCancel>(this);
                    }
                    {
                        entt::sink sink{combatable.onHit};
                        sink.disconnect<&InCombatState::OnHit>(this);
                    }
                }
            }
        }

        void InCombatState::OnComponentAdded(entt::entity self)
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK); // TODO: Change to "combat move" animation
            auto& autoAttackAbility = registry->get<PlayerAutoAttack>(self);
            autoAttackAbility.Init(self);
        }

        void InCombatState::OnComponentRemoved(entt::entity self)
        {
            controllableActorSystem->CancelMovement(self);
        }

        InCombatState::InCombatState(entt::registry* _registry, ControllableActorSystem* _controllableActorSystem, CollisionSystem* _collisionSystem, TimerManager* _timerManager)
            : StateMachine(_registry), controllableActorSystem(_controllableActorSystem)
        {
        }
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
        inCombatState = std::make_unique<playerstates::InCombatState>(_registry, _controllableActorSystem, _collisionSystem, _timerManager);
        systems.push_back(defaultState.get());
        systems.push_back(inCombatState.get());
    }
} // namespace sage