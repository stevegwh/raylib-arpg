//
// Created by steve on 20/05/2024.
//

#pragma once

#include "components/HealthBar.hpp"
#include "Camera.hpp"
#include "systems/BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
	class HealthBarSystem : public BaseSystem<HealthBar>
	{
		Camera* camera;

		void updateHealthBarTextures();

	public:
		HealthBarSystem(entt::registry* _registry, Camera* _camera);
		void Draw2D();
		void Draw3D();
		void Update();
	};
} // sage
