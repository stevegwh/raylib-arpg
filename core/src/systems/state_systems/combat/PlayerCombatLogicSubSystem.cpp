//
// Created by Steve on 05/06/24.
//


#include "PlayerCombatLogicSubSystem.hpp"
#include "components/states/PlayerStateComponents.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/Transform.hpp"
#include "components/HealthBar.hpp"

#include "raylib.h"
#include "raymath.h"

namespace sage
{
	void PlayerCombatLogicSubSystem::Update() const
	{
		auto view = registry->view<CombatableActor, StatePlayerCombat>();

		for (const auto& entity : view)
		{
			auto& c = registry->get<CombatableActor>(entity);
			if (!CheckInCombat(entity)) continue;

			// Player is out of combat if no enemy is targetting them?
			if (c.autoAttackTick >= c.autoAttackTickThreshold)
			// Maybe can count time since last autoattack to time out combat?
			{
				AutoAttack(entity);
			}
			else
			{
				c.autoAttackTick += GetFrameTime();
			}
		}
	}

	bool PlayerCombatLogicSubSystem::CheckInCombat(entt::entity entity) const
	{
		// If the entity is not the target of any other combatable.
		// If no current target
		// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
		auto& combatable = registry->get<CombatableActor>(entity);
		if (combatable.target == entt::null)
		{
			stateMachineSystem->ChangeState<StatePlayerDefault, StateComponents>(entity);
			return false;
		}
		return true;
	}

	void PlayerCombatLogicSubSystem::OnDeath(entt::entity entity)
	{
	}

	void PlayerCombatLogicSubSystem::OnTargetDeath(entt::entity entity)
	{
		auto& enemyCombatable = registry->get<CombatableActor>(entity);
		{
			entt::sink sink{enemyCombatable.onDeath};
			sink.disconnect<&PlayerCombatLogicSubSystem::OnTargetDeath>(this);
		}
		{
			entt::sink sink{cursor->onFloorClick};
			sink.disconnect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
		}
		auto& playerCombatable = registry->get<CombatableActor>(controllableActorSystem->GetControlledActor());
		playerCombatable.target = entt::null;
	}

	void PlayerCombatLogicSubSystem::OnAttackCancel(entt::entity entity)
	{
		// TODO: What is entity?
		auto& playerCombatable = registry->get<CombatableActor>(controllableActorSystem->GetControlledActor());
		playerCombatable.target = entt::null;
		auto& playerTrans = registry->get<Transform>(controllableActorSystem->GetControlledActor());
		{
			entt::sink sink{playerTrans.onFinishMovement};
			sink.disconnect<&PlayerCombatLogicSubSystem::StartCombat>(this);
		}
		controllableActorSystem->CancelMovement(controllableActorSystem->GetControlledActor());
	}

	void PlayerCombatLogicSubSystem::StartCombat(entt::entity entity)
	{
		// TODO: What is "entity"?
		{
			auto& playerTrans = registry->get<Transform>(controllableActorSystem->GetControlledActor());
			entt::sink sink{playerTrans.onFinishMovement};
			sink.disconnect<&PlayerCombatLogicSubSystem::StartCombat>(this);
		}

		auto& playerCombatable = registry->get<CombatableActor>(controllableActorSystem->GetControlledActor());
		stateMachineSystem->ChangeState<StatePlayerCombat, StateComponents>(
			controllableActorSystem->GetControlledActor());

		auto& enemyCombatable = registry->get<CombatableActor>(playerCombatable.target);
		{
			entt::sink sink{enemyCombatable.onDeath};
			sink.connect<&PlayerCombatLogicSubSystem::OnTargetDeath>(this);
		}
	}

	void PlayerCombatLogicSubSystem::onEnemyClick(entt::entity entity)
	{
		{
			entt::sink sink{cursor->onFloorClick};
			sink.connect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
		}
		auto& combatable = registry->get<CombatableActor>(controllableActorSystem->GetControlledActor());
		combatable.target = entity;
		auto& playerTrans = registry->get<Transform>(controllableActorSystem->GetControlledActor());
		const auto& enemyTrans = registry->get<Transform>(entity);

		const auto& enemyCollideable = registry->get<Collideable>(combatable.target);
		Vector3 enemyPos = enemyTrans.position;

		// Calculate the direction vector from player to enemy
		Vector3 direction = Vector3Subtract(enemyPos, playerTrans.position);

		// Normalize the direction vector
		float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		direction.x = (direction.x / length) * combatable.attackRange;
		direction.y = (direction.y / length) * combatable.attackRange;
		direction.z = (direction.z / length) * combatable.attackRange;

		// Calculate the target position by subtracting the normalized direction vector
		// multiplied by the attack range from the enemy position
		Vector3 targetPos = Vector3Subtract(enemyPos, direction);

		controllableActorSystem->PathfindToLocation(controllableActorSystem->GetControlledActor(), targetPos);
		{
			entt::sink sink{playerTrans.onFinishMovement};
			sink.connect<&PlayerCombatLogicSubSystem::StartCombat>(this);
		}
	}

	void PlayerCombatLogicSubSystem::AutoAttack(entt::entity entity) const
	{
		// TODO: Check if unit is still within our attack range?
		auto& c = registry->get<CombatableActor>(entity);

		auto& t = registry->get<Transform>(entity);
		auto& enemyPos = registry->get<Transform>(c.target).position;
		Vector3 direction = Vector3Subtract(enemyPos, t.position);
		float angle = atan2f(direction.x, direction.z) * RAD2DEG;
		t.rotation.y = angle;
		c.autoAttackTick = 0;

		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
		if (registry->any_of<CombatableActor>(c.target))
		{
			auto& enemyCombatable = registry->get<CombatableActor>(c.target);
			enemyCombatable.onHit.publish(c.target, controllableActorSystem->GetControlledActor(), 10); // TODO: tmp dmg
		}
	}

	void PlayerCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker)
	{
	}

	void PlayerCombatLogicSubSystem::Enable()
	{
		{
			entt::sink sink{cursor->onEnemyClick};
			sink.connect<&PlayerCombatLogicSubSystem::onEnemyClick>(this);
		}
	}

	void PlayerCombatLogicSubSystem::Disable()
	{
		{
			entt::sink sink{cursor->onEnemyClick};
			sink.disconnect<&PlayerCombatLogicSubSystem::onEnemyClick>(this);
		}
		{
			entt::sink sink{cursor->onFloorClick};
			sink.disconnect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
		}
	}

	void PlayerCombatLogicSubSystem::OnComponentEnabled(entt::entity entity) const
	{
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK); // TODO: Change to "combat move" animation
	}

	void PlayerCombatLogicSubSystem::OnComponentDisabled(entt::entity entity) const
	{
		controllableActorSystem->CancelMovement(entity);
	}

	PlayerCombatLogicSubSystem::PlayerCombatLogicSubSystem(entt::registry* _registry,
	                                                       StateMachineSystem* _stateMachineSystem,
	                                                       ControllableActorSystem* _controllableActorSystem,
	                                                       Cursor* _cursor) :
		registry(_registry),
		cursor(_cursor),
		stateMachineSystem(_stateMachineSystem),
		controllableActorSystem(_controllableActorSystem)
	{
		registry->on_construct<StatePlayerCombat>().connect<&PlayerCombatLogicSubSystem::OnComponentEnabled>(this);
		registry->on_destroy<StatePlayerCombat>().connect<&PlayerCombatLogicSubSystem::OnComponentDisabled>(this);
	}
} // sage
