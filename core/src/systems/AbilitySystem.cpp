#include "AbilitySystem.hpp"

namespace sage
{
	void AbilitySystem::abilityOnePressed()
	{
		abilityMap[currentAbilities[0]();
	}

	void AbilitySystem::abilityTwoPressed()
	{
		abilityMap[currentAbilities[1]();
	}

	void AbilitySystem::abilityThreePressed()
	{
		abilityMap[currentAbilities[2]();
	}

	void AbilitySystem::abilityFourPressed()
	{
		abilityMap[currentAbilities[3]();
	}

	void sage::AbilitySystem::ChangeAbility(int abilitySlot, int newAbilityIndex)
	{
		currentAbilities[abilitySlot] = newAbilityIndex;
	}

	void AbilitySystem::Update()
	{
		// Grab the  ability from the map? Update each one?
	}

	AbilitySystem::AbilitySystem(entt::registry* _registry, Cursor* _cursor, UserInput* _userInput, ActorMovementSystem* _actorMovementSystem, CollisionSystem* _collisionSystem) :
		registry(_registry), cursor(_cursor), userInput(_userInput), actorMovementSystem(_actorMovementSystem), collisionSystem(_collisionSystem)
	{
		// Subscribe to user input events
		// Need a way to change which event corresponds to which ability. Probably needs a map of some sort.
		{
			entt::sink sink userInput->keyOnePressed;
			sink.connect<&AbilitySystem::abilityOnePressed>(this);
		}
		{
			entt::sink sink userInput->keyTwoPressed;
			sink.connect<&AbilitySystem::abilityTwoPressed>(this);
		}
		{
			entt::sink sink userInput->keyThreePressed;
			sink.connect<&AbilitySystem::abilityThreePressed>(this);
		}
		{
			entt::sink sink userInput->keyFourPressed;
			sink.connect<&AbilitySystem::abilityFourPressed>(this);
		}



	}
}