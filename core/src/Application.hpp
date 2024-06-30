//
// Created by steve on 18/02/2024.
//

#pragma once

// Misc
#include "KeyMapping.hpp"
#include "scenes/Scene.hpp"
#include "Settings.hpp"

#include "raylib.h"
#include "entt/entt.hpp"
#include "Settings.hpp"

#include <stack>
#include <memory>
#include <unordered_map>

namespace sage
{
	class Application
	{
	protected:
		std::unique_ptr<Settings> settings;
		std::unique_ptr<KeyMapping> keyMapping;
		std::unique_ptr<entt::registry> registry;
		std::unique_ptr<Scene> scene;
		int stateChange = 0;
		virtual void init();
		static void cleanup();
		virtual void draw();

	public:
		Application();
		~Application();
		Application(const Application&) = delete;
		void operator=(const Application&) = delete;
		virtual void Update();
	};
}
