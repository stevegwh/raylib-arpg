//
// Created by steve on 05/06/24.
//

#include "raylib.h"
#include "raymath.h"

#include "WaveMobCombatStateSystem.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/HealthBar.hpp"
#include "components/sgTransform.hpp"

namespace sage
{
    void WaveMobCombatStateSystem::Update()
    {
        auto view = registry->view<CombatableActor, StateEnemyCombat>();

        for (const auto& entity : view)
        {
            auto& c = registry->get<CombatableActor>(entity);
            if (!checkInCombat(entity))
                continue;

            // Player is out of combat if no enemy is targetting them?
            if (c.autoAttackTick >= c.autoAttackTickThreshold)
            // Maybe can count time since last autoattack to time out combat?
            {
                autoAttack(entity);
            }
            else
            {
                c.autoAttackTick += GetFrameTime();
            }
        }
    }

    bool WaveMobCombatStateSystem::checkInCombat(entt::entity self)
    {
        // If the entity is not the target of any other combatable.
        // If no current target
        // Have a timer for aggro and if the player is not within that range for a certain amount of time they resume
        // their regular task (tasks TBC)
        auto& combatable = registry->get<CombatableActor>(self);
        if (combatable.dying)
            return false;
        if (combatable.target == entt::null)
        {
            ChangeState<StateEnemyDefault, EnemyStates>(self);
            return false;
        }
        return true;
    }

    void WaveMobCombatStateSystem::destroyEnemy(entt::entity self)
    {
        navigationGridSystem->MarkSquareAreaOccupied(registry->get<Collideable>(self).worldBoundingBox, false);
        {
            auto& animation = registry->get<Animation>(self);
            entt::sink sink{animation.onAnimationEnd};
            sink.disconnect<&WaveMobCombatStateSystem::destroyEnemy>(this);
        }
        registry->destroy(self);
    }

    void WaveMobCombatStateSystem::OnStateEnter(entt::entity self)
    {
        actorMovementSystem->CancelMovement(self);
        auto& combatable = registry->get<CombatableActor>(self);
        {
            entt::sink sink{combatable.onDeath};
            sink.connect<&WaveMobCombatStateSystem::onDeath>(this);
        }
    }

    void WaveMobCombatStateSystem::OnStateExit(entt::entity entity)
    {
    }

    void WaveMobCombatStateSystem::onDeath(entt::entity self)
    {
        auto& combatable = registry->get<CombatableActor>(self);
        combatable.target = entt::null;
        combatable.dying = true;

        {
            entt::sink sink{combatable.onHit};
            sink.disconnect<&WaveMobCombatStateSystem::OnHit>(this);
        }
        {
            entt::sink sink{combatable.onDeath};
            sink.disconnect<&WaveMobCombatStateSystem::onDeath>(this);
        }

        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
        {
            entt::sink sink{animation.onAnimationEnd};
            sink.connect<&WaveMobCombatStateSystem::destroyEnemy>(this);
        }
        actorMovementSystem->CancelMovement(self);
    }

	bool WaveMobCombatStateSystem::isTargetInLineOfSight(entt::entity self, Vector3& normDirection,
                                                             float distance) const
	{
		auto& combatableActor = registry->get<CombatableActor>(self);
		auto& actorTrans = registry->get<sgTransform>(self);
		auto& collideable = registry->get<Collideable>(self);

		Ray ray;
        ray.position = actorTrans.position();
        ray.direction = Vector3Scale(normDirection, distance);
        ray.position.y = 0.5f;
        ray.direction.y = 0.5f;
        actorTrans.movementDirectionDebugLine = ray;
        auto collisions = collisionSystem->GetCollisionsWithRay(self, ray, collideable.collisionLayer);

        if (!collisions.empty() && collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
        {
            // Lost line of sight, out of combat
            actorMovementSystem->CancelMovement(self);
            combatableActor.target = entt::null;
            actorTrans.movementDirectionDebugLine = {};
            return false;
        }
		return true;
	}

    inline void WaveMobCombatStateSystem::onTargetOutOfRange(entt::entity self, Vector3& normDirection,
                                                             float distance) const
    {
        auto& animation = registry->get<Animation>(self);
		auto& combatableActor = registry->get<CombatableActor>(self);
        auto& target = registry->get<sgTransform>(combatableActor.target).position();

        animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

		if (!isTargetInLineOfSight(self, normDirection, distance))
		{
			return;
		}
        
        const auto& moveableActor = registry->get<MoveableActor>(self);
        if (!moveableActor.isMoving())
        {
            actorMovementSystem->PathfindToLocation(self, target, true);
        }
    }

    void WaveMobCombatStateSystem::autoAttack(entt::entity self) const
    {
        auto& combatableActor = registry->get<CombatableActor>(self);
        auto& actorTrans = registry->get<sgTransform>(self);
        auto& animation = registry->get<Animation>(self);

        auto target = registry->get<sgTransform>(combatableActor.target).position();

        Vector3 direction = Vector3Subtract(target, actorTrans.position());
        float distance = Vector3Distance(actorTrans.position(), target);
        Vector3 normDirection = Vector3Normalize(direction);

		// TODO: Arbitrary number. Should probably use the navigation system to find the "next best square" from current position
        if (distance >= 8.0f) 
        {
            onTargetOutOfRange(self, normDirection, distance);
            return;
        }

        float angle = atan2f(direction.x, direction.z) * RAD2DEG;
        actorTrans.SetRotation({0, angle, 0}, self);
        combatableActor.autoAttackTick = 0;

        auto& targetCombatable = registry->get<CombatableActor>(combatableActor.target);

        // TODO: Need to move all of these things to an ability class
        AttackData attackData;
        attackData.attacker = self;
        attackData.hit = combatableActor.target;
        attackData.damage = 0;
        targetCombatable.onHit.publish(attackData);

        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
    }

    void WaveMobCombatStateSystem::Draw3D()
    {
        return;
        auto view = registry->view<CombatableActor>();
        for (auto& entity : view)
        {
            auto& c = registry->get<CombatableActor>(entity);
            if (c.actorType == CombatableActorType::WAVEMOB)
            {
                // Draw stuff here
            }
        }
    }

    void WaveMobCombatStateSystem::startCombat(entt::entity entity)
    {
    }

    void WaveMobCombatStateSystem::OnHit(AttackData attackData)
    {
        ChangeState<StateEnemyCombat, EnemyStates>(attackData.hit);
        auto& c = registry->get<CombatableActor>(attackData.hit);
        c.target = attackData.attacker;
    }

    WaveMobCombatStateSystem::WaveMobCombatStateSystem(entt::registry* _registry,
                                                       ActorMovementSystem* _actorMovementSystem,
                                                       CollisionSystem* _collisionSystem,
                                                       NavigationGridSystem* _navigationGridSystem)
        : StateMachineSystem(_registry), navigationGridSystem(_navigationGridSystem),
          actorMovementSystem(_actorMovementSystem), collisionSystem(_collisionSystem)
    {
    }
} // namespace sage
