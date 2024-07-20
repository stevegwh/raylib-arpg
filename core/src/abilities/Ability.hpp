#pragma once


namespace sage
{
	struct Ability
	{
		float cooldownTimer;
		float duration;
		float initialDamage;
		float damageOverTime; // 0 if no damage over time
		// A DoT should probably push a function to the affected unit and the ability's "DoT update" should be called every frame
		void Update();
		void Draw3D();
	};
}