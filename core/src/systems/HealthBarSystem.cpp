//
// Created by steve on 20/05/2024.
//

#include "HealthBarSystem.hpp"
#include "components/Collideable.hpp"

#include "raylib.h"


namespace sage
{
	void HealthBarSystem::Draw2D()
	{
	}

	void HealthBarSystem::updateHealthBarTextures()
	{
		const auto& view = registry->view<HealthBar>();
		view.each([this](const auto& c)
		{
			c.UpdateTexture();
		});
	}

	void HealthBarSystem::Draw3D()
	{
		const auto& view = registry->view<HealthBar, Collideable>();
		view.each([this](const auto& c, const auto& col)
		{
			const Vector3& min = col.worldBoundingBox.min;
			const Vector3& max = col.worldBoundingBox.max;

			Vector3 modelCenter = {
				min.x + (max.x - min.x) / 2,
				max.y,
				min.z + (max.z - min.z) / 2
			};

			Vector3 billboardPos = modelCenter;
			billboardPos.y += 1.0f;
			Rectangle sourceRec = {
				0.0f, 0.0f, static_cast<float>(c.healthBarTexture.texture.width),
				static_cast<float>(-c.healthBarTexture.texture.height)
			};
			DrawBillboardRec(*camera->getRaylibCam(), c.healthBarTexture.texture, sourceRec, billboardPos, {1.0f, 1.0f},
			                 WHITE);
		});
	}

	void HealthBarSystem::Update()
	{
		updateHealthBarTextures();
	}


	HealthBarSystem::HealthBarSystem(entt::registry* _registry, Camera* _camera) :
		BaseSystem(_registry), camera(_camera)
	{
	}
} // sage
