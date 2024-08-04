#include "PlayerCombatStateSystem.hpp"
//
// Created by Steve on 05/06/24.
//

#include "PlayerCombatStateSystem.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"
#include "components/sgTransform.hpp"

#include "raylib.h"
#include "raymath.h"

namespace sage
{
    void PlayerCombatStateSystem::Update()
    {
        auto view = registry->view<CombatableActor, StatePlayerCombat>();

        for (const auto& entity : view)
        {
            auto& c = registry->get<CombatableActor>(entity);
            if (!checkInCombat(entity))
                continue;

            autoAttackAbility->Update(entity);
        }
    }

    void PlayerCombatStateSystem::Draw3D()
    {
    }

    bool PlayerCombatStateSystem::checkInCombat(entt::entity entity)
    {
        // If the entity is not the target of any other combatable.
        // If no current target
        // Have a timer for aggro and if the player is not within that range for a certain amount of time they resume
        // their regular task (tasks TBC)

        // TODO: Should only go out of combat if no enemies are targetting the player

        auto& combatable = registry->get<CombatableActor>(entity);
        if (combatable.target == entt::null)
        {
            ChangeState<StatePlayerDefault, PlayerStates>(entity);
            return false;
        }
        return true;
    }

    void PlayerCombatStateSystem::onDeath(entt::entity entity)
    {
    }

    void PlayerCombatStateSystem::onTargetDeath(entt::entity self, entt::entity target)
    {
        auto& enemyCombatable = registry->get<CombatableActor>(target);
        auto& playerCombatable = registry->get<CombatableActor>(self);
        {
            entt::sink sink{playerCombatable.onTargetDeath};
            sink.disconnect<&PlayerCombatStateSystem::onTargetDeath>(this);
        }
        {
            entt::sink sink{playerCombatable.onAttackCancelled};
            sink.disconnect<&PlayerCombatStateSystem::onAttackCancel>(this);
        }
        playerCombatable.target = entt::null;
    }

    void PlayerCombatStateSystem::onAttackCancel(entt::entity entity)
    {
        auto& playerCombatable = registry->get<CombatableActor>(entity);
        playerCombatable.target = entt::null;
        auto& playerTrans = registry->get<sgTransform>(entity);
        {
            entt::sink sink{playerTrans.onFinishMovement};
            sink.disconnect<&PlayerCombatStateSystem::startCombat>(this);
        }
        controllableActorSystem->CancelMovement(entity);
        autoAttackAbility->Cancel();
    }

    void PlayerCombatStateSystem::OnHit(AttackData attackData)
    {
        if (!checkInCombat(attackData.hit))
        {
            // OnEnemyClick(attackData.hit, attackData.attacker);
        }
    }

    void PlayerCombatStateSystem::OnEnemyClick(entt::entity self, entt::entity target)
    {
        auto& combatable = registry->get<CombatableActor>(self);
        combatable.target = target;
        {
            entt::sink sink{combatable.onAttackCancelled};
            sink.connect<&PlayerCombatStateSystem::onAttackCancel>(this);
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
            sink.connect<&PlayerCombatStateSystem::startCombat>(this);
        }
    }

    void PlayerCombatStateSystem::startCombat(entt::entity self)
    {
        // Remember to set target
        {
            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.disconnect<&PlayerCombatStateSystem::startCombat>(this);
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
            sink.connect<&PlayerCombatStateSystem::onTargetDeath>(this);
        }
    }

    void PlayerCombatStateSystem::Enable()
    {
        auto view = registry->view<CombatableActor>();
        for (const auto& entity : view)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            if (combatable.actorType == CombatableActorType::PLAYER)
            {
                {
                    entt::sink sink{combatable.onEnemyClicked};
                    sink.connect<&PlayerCombatStateSystem::OnEnemyClick>(this);
                }
                {
                    entt::sink sink{combatable.onAttackCancelled};
                    sink.connect<&PlayerCombatStateSystem::onAttackCancel>(this);
                }
                {
                    entt::sink sink{combatable.onHit};
                    sink.connect<&PlayerCombatStateSystem::OnHit>(this);
                }
            }
        }
    }

    void PlayerCombatStateSystem::Disable()
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
                    sink.disconnect<&PlayerCombatStateSystem::OnEnemyClick>(this);
                }
                {
                    entt::sink sink{combatable.onAttackCancelled};
                    sink.disconnect<&PlayerCombatStateSystem::onAttackCancel>(this);
                }
                {
                    entt::sink sink{combatable.onHit};
                    sink.disconnect<&PlayerCombatStateSystem::OnHit>(this);
                }
            }
        }
    }

    void PlayerCombatStateSystem::OnStateEnter(entt::entity self)
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK); // TODO: Change to "combat move" animation
        autoAttackAbility->Init();
    }

    void PlayerCombatStateSystem::OnStateExit(entt::entity self)
    {
        controllableActorSystem->CancelMovement(self);
        autoAttackAbility->Cancel();
    }

    PlayerCombatStateSystem::PlayerCombatStateSystem(entt::registry* _registry,
                                                     ControllableActorSystem* _controllableActorSystem,
                                                     CollisionSystem* _collisionSystem, 
                                                     TimerManager* _timerManager)
        : StateMachineSystem(_registry), controllableActorSystem(_controllableActorSystem),
        autoAttackAbility(std::make_unique<PlayerAutoAttack>(registry, _collisionSystem, _timerManager))
    {
    }
} // namespace sage
