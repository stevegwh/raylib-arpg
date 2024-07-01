//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "systems/LightSubSystem.hpp"
#include "entt/entt.hpp"
#include "GameData.hpp"

#include <memory>

namespace sage
{
	class Scene
	{
	protected:
		entt::registry* registry;

	public:
		std::unique_ptr<GameData> data;
		std::unique_ptr<LightSubSystem> lightSubSystem;

		explicit Scene(entt::registry* _registry, std::unique_ptr<GameData> _data) :
			registry(_registry),
			data(std::move(_data)),
			lightSubSystem(std::make_unique<LightSubSystem>())
		{
		};
		virtual ~Scene() = default;

		virtual void Update()
		{
			data->renderSystem->Update();
			data->camera->Update();
			data->userInput->ListenForInput();
			data->cursor->Update();
		}

		virtual void Draw3D()
		{
			data->renderSystem->Draw();
			// If we hit something, draw the cursor at the hit point
			data->cursor->Draw3D();
			lightSubSystem->DrawDebugLights();
		};

		virtual void Draw2D()
		{
			data->cursor->Draw2D();
		}

		virtual void DrawDebug()
		{
		}
	};
} // sage
