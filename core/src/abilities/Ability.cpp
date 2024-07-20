#include "Ability.hpp"

namespace sage
{
	void Ability::Update()
	{
		cooldownTimer -= 1.0f;
	}

	void Ability::Draw3D()
	{
		// Draw the ability in 3D space
	}
}