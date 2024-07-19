//
// Created by steve on 05/06/24.
//

#include "raylib.h"
#include "raymath.h"

#include "WaveMobCombatLogicSubSystem.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/sgTransform.hpp"
#include "components/HealthBar.hpp"
#include "components/states/EnemyStateComponents.hpp"

namespace sage
{
	void WaveMobCombatLogicSubSystem::Update()
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

	bool WaveMobCombatLogicSubSystem::checkInCombat(entt::entity entity)
	{
		// If the entity is not the target of any other combatable.
		// If no current target
		// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
		auto& combatable = registry->get<CombatableActor>(entity);
		if (combatable.dying) return false;
		if (combatable.target == entt::null)
		{
			ChangeState<StateEnemyDefault, StateComponents>(entity);
			return false;
		}
		return true;
	}

	void WaveMobCombatLogicSubSystem::destroyEnemy(entt::entity entity)
	{
		navigationGridSystem->MarkSquareAreaOccupied(registry->get<Collideable>(entity).worldBoundingBox, false);
		{
			auto& animation = registry->get<Animation>(entity);
			entt::sink sink{animation.onAnimationEnd};
			sink.disconnect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
		}
		registry->destroy(entity);
	}

	void WaveMobCombatLogicSubSystem::OnStateEnter(entt::entity entity)
	{
		actorMovementSystem->CancelMovement(entity);
	}

	void WaveMobCombatLogicSubSystem::OnStateExit(entt::entity entity)
	{
	}

	void WaveMobCombatLogicSubSystem::onDeath(entt::entity entity)
	{
		auto& combatable = registry->get<CombatableActor>(entity);
		combatable.onDeath.publish(entity);
		combatable.target = entt::null;
		combatable.dying = true;

		{
			entt::sink sink{combatable.onHit};
			sink.disconnect<&WaveMobCombatLogicSubSystem::OnHit>(this);
		}

		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
		{
			entt::sink sink{animation.onAnimationEnd};
			sink.connect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
		}
	}

	inline void WaveMobCombatLogicSubSystem::onTargetOutOfRange(entt::entity entity,
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

	void WaveMobCombatLogicSubSystem::autoAttack(entt::entity entity) const
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
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
	}

	void WaveMobCombatLogicSubSystem::Draw3D(entt::entity entity)
	{
		auto& t = registry->get<sgTransform>(entity);
		//DrawLine3D(t.movementDirectionDebugLine.position, t.movementDirectionDebugLine.direction, RED);
	}

	void WaveMobCombatLogicSubSystem::startCombat(entt::entity entity)
	{
	}

	void WaveMobCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker, float damage)
	{
		ChangeState<StateEnemyCombat, StateComponents>(entity);
		// Aggro when player hits
		auto& c = registry->get<CombatableActor>(entity);
		c.target = attacker;

		auto& healthbar = registry->get<HealthBar>(entity);
		healthbar.Decrement(entity, damage);
		if (healthbar.hp <= 0)
		{
			c.target = entt::null;
			onDeath(entity);
		}
	}

	WaveMobCombatLogicSubSystem::WaveMobCombatLogicSubSystem(entt::registry* _registry,
	                                                         ActorMovementSystem* _actorMovementSystem,
	                                                         CollisionSystem* _collisionSystem,
	                                                         NavigationGridSystem* _navigationGridSystem) :
		StateMachineSystem(_registry),
		navigationGridSystem(_navigationGridSystem),
		actorMovementSystem(_actorMovementSystem),
		collisionSystem(_collisionSystem)
	{
		registry->on_construct<StateEnemyCombat>().connect<&WaveMobCombatLogicSubSystem::OnStateEnter>(this);
		registry->on_destroy<StateEnemyCombat>().connect<&WaveMobCombatLogicSubSystem::OnStateExit>(this);
	}
} // sage
