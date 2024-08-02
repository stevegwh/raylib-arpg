//
// Created by Steve Wheeler on 26/07/2024.
//

#pragma once

#include "FountainEffect.hpp"

#include "raylib.h"

#include <vector>
#include <memory>

namespace sage
{

	class SpiralFountainVFX
	{
		Shader shader{};
		Camera3D* camera;

		Vector3 spiralCentre{};
		float spiralRadius = 0.0f;    // Starting radius of the spiral
		float spiralSpeed = 10.0f;     // Speed of rotation (in radians per second)
		float spiralAngle = 0.0f;     // Current angle of rotation
		float spiralGrowth = 0.5f;    // How much the radius grows per second
		float maxSpiralRadius = 1.0f; // Maximum radius of the spiral


		std::unique_ptr<FountainEffect> fountain;
	public:
		bool active = false;
		~SpiralFountainVFX();
		explicit SpiralFountainVFX(Camera3D* camera);
		void InitSystem(const Vector3& _target);
		void Update(float dt);
		void Draw3D() const;
	};
}