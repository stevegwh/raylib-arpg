//
// Created by Steve Wheeler on 25/07/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <vector>

#include "components/NavigationGridSquare.hpp"
#include "systems/NavigationGridSystem.hpp"

namespace sage
{

	class TextureTerrainOverlay
	{
		entt::registry* registry;
		entt::entity entity;
		bool m_active = false;
		bool initialised = false;
		NavigationGridSystem* navigationGridSystem;
		Texture2D texture{};
		Model generateTerrainPolygon();
		void updateTerrainPolygon();
		GridSquare minRange{}; // Top left corner of the texture
		GridSquare maxRange{}; // bottom right corner of the texture
		GridSquare lastHit{};
	public:
		~TextureTerrainOverlay();
		// TODO: NavigationGridSystem needs to return a square portion of the grid
		TextureTerrainOverlay(entt::registry* _registry,
				NavigationGridSystem* _navigationGridSystem,
				const char* texturePath
		);
		void Init(Vector3 mouseRayHit);
		void Update(Vector3 mouseRayHit);
		void Enable(bool enable);
		bool active() const;
	};

} // sage
