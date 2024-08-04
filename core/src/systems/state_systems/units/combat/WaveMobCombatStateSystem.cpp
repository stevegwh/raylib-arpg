//
// Created by steve on 05/06/24.
//

#include "raylib.h"
#include "raymath.h"

#include "WaveMobCombatStateSystem.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/sgTransform.hpp"
#include "components/HealthBar.hpp"

namespace sage
{
	void WaveMobCombatStateSystem::Update()
	{
		auto view = registry->view<CombatableActor, StateEnemyCombat>();

		for (const auto& entity : view)
		{
			auto& c = registry->get<CombatableActor>(entity);
			if (!checkInCombat(entity)) continue;

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

	bool WaveMobCombatStateSystem::checkInCombat(entt::entity entity)
	{
		// If the entity is not the target of any other combatable.
		// If no current target
		// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
		auto& combatable = registry->get<CombatableActor>(entity);
		if (combatable.dying) return false;
		if (combatable.target == entt::null)
		{
			ChangeState<StateEnemyDefault, EnemyStates>(entity);
			return false;
		}
		return true;
	}

	void WaveMobCombatStateSystem::destroyEnemy(entt::entity entity)
	{
		navigationGridSystem->MarkSquareAreaOccupied(registry->get<Collideable>(entity).worldBoundingBox, false);
		{
			auto& animation = registry->get<Animation>(entity);
			entt::sink sink{animation.onAnimationEnd};
			sink.disconnect<&WaveMobCombatStateSystem::destroyEnemy>(this);
		}
		registry->destroy(entity);
	}

	void WaveMobCombatStateSystem::OnStateEnter(entt::entity entity)
	{
		actorMovementSystem->CancelMovement(entity);
		auto& combatable = registry->get<CombatableActor>(entity);
		{
			entt::sink sink{ combatable.onDeath };
			sink.connect<&WaveMobCombatStateSystem::onDeath>(this);
		}
	}

	void WaveMobCombatStateSystem::OnStateExit(entt::entity entity)
	{
	}

	void WaveMobCombatStateSystem::onDeath(entt::entity entity)
	{
		auto& combatable = registry->get<CombatableActor>(entity);
		combatable.target = entt::null;
		combatable.dying = true;

		{
			entt::sink sink{combatable.onHit};
			sink.disconnect<&WaveMobCombatStateSystem::OnHit>(this);
		}
		{
			entt::sink sink{ combatable.onDeath };
			sink.disconnect<&WaveMobCombatStateSystem::onDeath>(this);
		}

		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
		{
			entt::sink sink{animation.onAnimationEnd};
			sink.connect<&WaveMobCombatStateSystem::destroyEnemy>(this);
		}
	}

	inline void WaveMobCombatStateSystem::onTargetOutOfRange(entt::entity entity,
	                                                            Vector3& normDirection,
	                                                            float distance) const
	{
		auto& combatableActor = registry->get<CombatableActor>(entity);
		auto& animation = registry->get<Animation>(entity);
		auto& actorTrans = registry->get<sgTransform>(entity);
		auto& collideable = registry->get<Collideable>(entity);
		auto& target = registry->get<sgTransform>(combatableActor.target).position();

		animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
		Ray ray;
		ray.position = actorTrans.position();
		ray.direction = Vector3Scale(normDirection, distance);
		ray.position.y = 0.5f;
		ray.direction.y = 0.5f;
		actorTrans.movementDirectionDebugLine = ray;
		auto collisions = collisionSystem->GetCollisionsWithRay(entity, ray, collideable.collisionLayer);

		if (!collisions.empty() && collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
		{
			// Lost line of sight, out of combat
			actorMovementSystem->CancelMovement(entity);
			combatableActor.target = entt::null;
			actorTrans.movementDirectionDebugLine = {};
			return;
		}
		const auto& moveableActor = registry->get<MoveableActor>(entity);
		if (!moveableActor.destination.has_value())
		{
			actorMovementSystem->PathfindToLocation(entity, target, true);
		}
	}

	void WaveMobCombatStateSystem::autoAttack(entt::entity entity) const
	{
		auto& combatableActor = registry->get<CombatableActor>(entity);
		auto& actorTrans = registry->get<sgTransform>(entity);
		auto& animation = registry->get<Animation>(entity);

		auto target = registry->get<sgTransform>(combatableActor.target).position();

		Vector3 direction = Vector3Subtract(target, actorTrans.position());
		float distance = Vector3Distance(actorTrans.position(), target);
		Vector3 normDirection = Vector3Normalize(direction);

		if (distance >= 15)
		{
			onTargetOutOfRange(entity, normDirection, distance);
			return;
		}

		float angle = atan2f(direction.x, direction.z) * RAD2DEG;
		actorTrans.SetRotation({0, angle, 0}, entity);
		combatableActor.autoAttackTick = 0;

		auto& targetCombatable = registry->get<CombatableActor>(combatableActor.target);

		// TODO: Need to move all of these things to an ability class
		AttackData attackData;
		attackData.attacker = entity;
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
	                                                         NavigationGridSystem* _navigationGridSystem) :
		StateMachineSystem(_registry),
		navigationGridSystem(_navigationGridSystem),
		actorMovementSystem(_actorMovementSystem),
		collisionSystem(_collisionSystem)
	{
	}
} // sage
