//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "Scene.hpp"
#include "../UserInput.hpp"

#include "entt/entt.hpp"
#include "GameData.hpp"

#include <vector>
#include <memory>
#include <string>

namespace sage
{
	class ExampleScene : public Scene
	{
	public:
		ExampleScene(entt::registry* _registry, std::unique_ptr<GameData> _data, const std::string& mapPath);
		~ExampleScene() override;
		void Update() override;
		void Draw2D() override;
		void Draw3D() override;
		void DrawDebug() override;
	};
} // sage
