//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "../components/Renderable.hpp"
#include "BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
	class RenderSystem : public BaseSystem<Renderable>
	{
	public:
		explicit RenderSystem(entt::registry* _registry);
		~RenderSystem();
		void Update();
		void Draw() const;
	};
}
