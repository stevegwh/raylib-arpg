#pragma once

#include <entt/entt.hpp>

#include "UserInput.hpp"
#include "Cursor.hpp"
#include "ActorMovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "Ability.hpp"

#include <vector>

namespace sage
{
	class AbilitySystem
	{
		entt::registry* registry;
		Cursor* cursor;
		UserInput* userInput;
		ActorMovementSystem* actorMovementSystem;
		CollisionSystem* collisionSystem;
		std::vector <Ability> abilityMap;
		std::array<int> currentAbilities;
		void abilityOnePressed();
		void abilityTwoPressed();
		void abilityThreePressed();
		void abilityFourPressed();
	public:
		void ChangeAbility(int abilitySlot, int newAbilityIndex);
		void Update();
		AbilitySystem(entt::registry* _registry, Cursor* _cursor, UserInput* _userInput, ActorMovementSystem* _actorMovementSystem, CollisionSystem* _collisionSystem);

	};
}