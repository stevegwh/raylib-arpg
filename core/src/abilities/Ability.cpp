#include "Ability.hpp"

#include "raylib.h"

namespace sage
{
	void Ability::Update()
	{
		cooldownTimer -= GetFrameTime();
	}

	void Ability::Draw3D()
	{
		// Draw the ability in 3D space
	}
    
    Ability::Ability(entt::registry* _registry, CollisionSystem* _collisionSystem)
        : registry(_registry), collisionSystem(_collisionSystem) {}
}