//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "Camera.hpp"

#include <entt/entt.hpp>

namespace sage
{
	enum class CursorState
	{
		DEFAULT,
		NPC_HOVER,
		BUILDING_HOVER
	};

	class Cursor
	{
		Texture2D* currentTex;
		Texture2D regulartex;
		Texture2D talktex;
		Texture2D movetex;
		Texture2D invalidmovetex;
		Texture2D combattex;

		Vector2 position;
		entt::registry* registry;
		Ray ray{};
		Color defaultColor = WHITE;
		Color hoverColor = LIME;
		Color invalidColor = RED;
		Color currentColor = WHITE;

		bool contextLocked = false;
		bool hideCursor = false;
		bool enabled = true;

		entt::entity controlledActor;

		CollisionSystem* collisionSystem;
		NavigationGridSystem* navigationGridSystem;
		Camera* sCamera;
		UserInput* userInput;

		void getMouseRayCollision();
		void onMouseClick();
		void changeCursors(CollisionLayer collisionLayer);

	public:
		std::string hitObjectName{};
		RayCollision collision{};
		CollisionInfo rayCollisionResultInfo;
		entt::sigh<void(entt::entity)> onCollisionHit{}; // Returns the hit entity (all layers)
		entt::sigh<void(entt::entity)> onNPCClick{};
		entt::sigh<void(entt::entity entity)> onFloorClick{};
		entt::sigh<void(entt::entity entity)> onAnyClick{};
		entt::sigh<void(entt::entity)> onEnemyClick{};

		Cursor(entt::registry* registry,
		       CollisionSystem* _collisionSystem,
		       NavigationGridSystem* _navigationGridSystem,
		       Camera* _sCamera,
		       UserInput* _userInput);

		void Update();
		void Draw3D();
		void Draw2D();
		void OnControlledActorChange(entt::entity entity);
		void DisableContextSwitching();
		void EnableContextSwitching();
		void Enable();
		void Disable();
		void Hide();
		void Show();
		bool isValidMove() const;
	};
}
