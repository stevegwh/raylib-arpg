//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "FlameEffect.hpp"

#include "raylib.h"

#include <vector>
#include <memory>

namespace sage
{
	struct Fireball
	{
		Vector3 position;
		Vector3 velocity;
		float radius;
		std::unique_ptr<FlameEffect> flameEffect;
	};

	class RainOfFireVFX
	{
		Shader shader{};
		Camera3D* camera;
		Vector3 target{};
		Vector3 baseSpawnPoint{};
		float initialHeight{};
		float minHeight{};
		float impactRadius{};
		const float initialOffset = 1.0f;  // Initial diagonal offset from the target
		const float spawnAreaRadius = 2.0f;  // Radius around the spawn point to vary starting positions
		std::vector<std::unique_ptr<Fireball>> fireballs;
		void generateFireball(Fireball& fireball);
	public:
		bool active = false;
		~RainOfFireVFX();
		explicit RainOfFireVFX(Camera3D* camera);
		void InitSystem(const Vector3& _target);
		void Update(float dt);
		void Draw3D() const;
	};
}