#include "PlayerCombatLogicSubSystem.hpp"
//
// Created by Steve on 05/06/24.
//


#include "PlayerCombatLogicSubSystem.hpp"
#include "components/states/PlayerStateComponents.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/sgTransform.hpp"
#include "components/HealthBar.hpp"

#include "raylib.h"
#include "raymath.h"

namespace sage
{
	void PlayerCombatLogicSubSystem::Update()
	{
		auto view = registry->view<CombatableActor, StatePlayerCombat>();

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

	void PlayerCombatLogicSubSystem::Draw3D(entt::entity entity)
	{
	}

	bool PlayerCombatLogicSubSystem::checkInCombat(entt::entity entity)
	{
		// If the entity is not the target of any other combatable.
		// If no current target
		// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
		auto& combatable = registry->get<CombatableActor>(entity);
		if (combatable.target == entt::null)
		{
			ChangeState<StatePlayerDefault, StateComponents>(entity);
			return false;
		}
		return true;
	}

	void PlayerCombatLogicSubSystem::onDeath(entt::entity entity)
	{
	}

	void PlayerCombatLogicSubSystem::onTargetDeath(entt::entity actor, entt::entity target)
	{
		auto& enemyCombatable = registry->get<CombatableActor>(target);
		auto& playerCombatable = registry->get<CombatableActor>(actor);
        {
            entt::sink sink{ playerCombatable.onTargetDeath };
            sink.disconnect<&PlayerCombatLogicSubSystem::onTargetDeath>(this);
        }
		{
			entt::sink sink{ playerCombatable.onAttackCancelled };
			sink.disconnect<&PlayerCombatLogicSubSystem::onAttackCancel>(this);
		}
		playerCombatable.target = entt::null;
	}

	void PlayerCombatLogicSubSystem::onAttackCancel(entt::entity entity)
	{
		auto& playerCombatable = registry->get<CombatableActor>(entity);
		playerCombatable.target = entt::null;
		auto& playerTrans = registry->get<sgTransform>(entity);
		{
			entt::sink sink{ playerTrans.onFinishMovement };
			sink.disconnect<&PlayerCombatLogicSubSystem::startCombat>(this);
		}
		controllableActorSystem->CancelMovement(entity);
	}

	
	void PlayerCombatLogicSubSystem::autoAttack(entt::entity entity) const
	{
		// TODO: Check if unit is still within our attack range?
		auto& c = registry->get<CombatableActor>(entity);

		auto& t = registry->get<sgTransform>(entity);
		auto& enemyPos = registry->get<sgTransform>(c.target).position();
		Vector3 direction = Vector3Subtract(enemyPos, t.position());
		float angle = atan2f(direction.x, direction.z) * RAD2DEG;
		t.SetRotation({ 0, angle, 0 }, entity);
		c.autoAttackTick = 0;

		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
		if (registry->any_of<CombatableActor>(c.target))
		{
			auto& enemyCombatable = registry->get<CombatableActor>(c.target);
			enemyCombatable.onHit.publish(c.target, entity, 10); // TODO: tmp dmg
		}
	}

	void PlayerCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker)
	{
	}

	void PlayerCombatLogicSubSystem::OnEnemyClick(entt::entity actor, entt::entity target)
	{
		auto& combatable = registry->get<CombatableActor>(actor);
		{
			entt::sink sink{ combatable.onAttackCancelled };
			sink.connect<&PlayerCombatLogicSubSystem::onAttackCancel>(this);
		}
		auto& playerTrans = registry->get<sgTransform>(actor);
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

		controllableActorSystem->PathfindToLocation(actor, targetPos);
		{
			entt::sink sink{playerTrans.onFinishMovement};
			sink.connect<&PlayerCombatLogicSubSystem::startCombat>(this);
		}
	}

	void PlayerCombatLogicSubSystem::startCombat(entt::entity entity)
	{
		{
			auto& playerTrans = registry->get<sgTransform>(entity);
			entt::sink sink{ playerTrans.onFinishMovement };
			sink.disconnect<&PlayerCombatLogicSubSystem::startCombat>(this);
		}

		auto& playerCombatable = registry->get<CombatableActor>(entity);
		ChangeState<StatePlayerCombat, StateComponents>(entity);

		auto& enemyCombatable = registry->get<CombatableActor>(playerCombatable.target);
		{
			entt::sink sink{ enemyCombatable.onDeath };
			sink.connect<&CombatableActor::TargetDeath>(playerCombatable);
		}
		{
			entt::sink sink{ playerCombatable.onTargetDeath };
			sink.connect<&PlayerCombatLogicSubSystem::onTargetDeath>(this);
		}
	}

	void PlayerCombatLogicSubSystem::Enable()
	{
		// Add checks to see if the player should be in combat
		auto view = registry->view<CombatableActor>();
		for (const auto& entity : view)
		{
			auto& combatable = registry->get<CombatableActor>(entity);
			if (combatable.actorType == CombatableActorType::PLAYER)
			{
				{
					entt::sink sink{ combatable.onEnemyClicked };
					sink.connect<&PlayerCombatLogicSubSystem::OnEnemyClick>(this);
				}
				{
					entt::sink sink{ combatable.onAttackCancelled };
					sink.connect<&PlayerCombatLogicSubSystem::onAttackCancel>(this);
				}
			}
		}
	}

	void PlayerCombatLogicSubSystem::Disable()
	{
		// Remove checks to see if the player should be in combat
		auto view = registry->view<CombatableActor>();
		for (const auto& entity : view)
		{
			auto& combatable = registry->get<CombatableActor>(entity);
			if (combatable.actorType == CombatableActorType::PLAYER)
			{
				{
					entt::sink sink{ combatable.onEnemyClicked };
					sink.disconnect<&PlayerCombatLogicSubSystem::OnEnemyClick>(this);
				}
				{
					entt::sink sink{ combatable.onAttackCancelled };
					sink.disconnect<&PlayerCombatLogicSubSystem::onAttackCancel>(this);
				}
			}
		}
	}

	void PlayerCombatLogicSubSystem::OnStateEnter(entt::entity entity)
	{
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK); // TODO: Change to "combat move" animation
	}

	void PlayerCombatLogicSubSystem::OnStateExit(entt::entity entity)
	{
		controllableActorSystem->CancelMovement(entity);
	}

	PlayerCombatLogicSubSystem::PlayerCombatLogicSubSystem(
		entt::registry* _registry,
		ControllableActorSystem* _controllableActorSystem) :
		StateMachineSystem(_registry),
		controllableActorSystem(_controllableActorSystem)
	{
		registry->on_construct<StatePlayerCombat>().connect<&PlayerCombatLogicSubSystem::OnStateEnter>(this);
		registry->on_destroy<StatePlayerCombat>().connect<&PlayerCombatLogicSubSystem::OnStateExit>(this);
	}
} // sage
