//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"
#include "UserInput.hpp"
#include "components/Renderable.hpp"
#include "components/ControllableActor.hpp"

#ifndef MSVC
#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

#endif

namespace sage
{
	void Cursor::onMouseClick()
	{
		if (!enabled) return;
		
		const auto& layer = registry->get<Collideable>(rayCollisionResultInfo.collidedEntityId).collisionLayer;
		if (layer == CollisionLayer::NPC)
		{
			onNPCClick.publish(rayCollisionResultInfo.collidedEntityId);
		}
		else if (layer == CollisionLayer::FLOOR)
		{
			onFloorClick.publish(rayCollisionResultInfo.collidedEntityId);
		}
		else if (layer == CollisionLayer::ENEMY)
		{
			onEnemyClick.publish(rayCollisionResultInfo.collidedEntityId);
		}
		onAnyClick.publish(rayCollisionResultInfo.collidedEntityId);
	}

	void Cursor::DisableContextSwitching() // Lock mouse context? Like changing depending on collision.
	{
		contextLocked = true;
	}

	void Cursor::EnableContextSwitching()
	{
		contextLocked = false;
	}

	void Cursor::Enable()
	{
		enabled = true;
	}
	
	void Cursor::Disable()
	{
		enabled = false;
	}
	
	void Cursor::Hide()
	{
		hideCursor = true;
	}
	
	void Cursor::Show()
	{
		hideCursor = false;
	}

	bool Cursor::isValidMove() const
	{
		GridSquare clickedSquare{};
		if (navigationGridSystem->WorldToGridSpace(collision.point,
                                                   clickedSquare))
		// Out of map bounds (TODO: Potentially pointless, if FLOOR is the same size as bounds.)
		{
			if (registry->any_of<ControllableActor>(controlledActor))
			{
				const auto& actor = registry->get<ControllableActor>(controlledActor);
				GridSquare minRange;
				GridSquare maxRange;
				navigationGridSystem->GetPathfindRange(controlledActor,
				                                       actor.pathfindingBounds,
				                                       minRange,
				                                       maxRange);
				if (!navigationGridSystem->WorldToGridSpace(collision.point, clickedSquare, minRange, maxRange))
				// Out of player's movement range
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
        if (navigationGridSystem->GetGridSquare(clickedSquare.row, clickedSquare.col)->occupied)
        {
            return false;
        }
		return true;
	}

	void Cursor::changeCursors(CollisionLayer layer)
	{
		if (contextLocked) return;

		if (layer == CollisionLayer::FLOOR || layer == CollisionLayer::NAVIGATION)
		{
			if (isValidMove())
			{
				currentTex = &movetex;
				currentColor = GREEN;
			}
			else
			{
				currentTex = &invalidmovetex;
				currentColor = invalidColor;
			}
            if (registry->all_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
            {
                hitObjectName = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId).name;
            }
		}
		else if (layer == CollisionLayer::BUILDING)
		{
			currentTex = &regulartex;
			currentColor = invalidColor;
			if (registry->all_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
			{
				hitObjectName = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId).name;
			}
		}
		else if (layer == CollisionLayer::PLAYER)
		{
			currentTex = &regulartex;
            hitObjectName = "Player";
		}
		else if (layer == CollisionLayer::NPC)
		{
			currentTex = &talktex;
            hitObjectName = "NPC";
		}
		else if (layer == CollisionLayer::ENEMY)
		{
			currentTex = &combattex;
            hitObjectName = "NPC";
		}
	}

	void Cursor::getMouseRayCollision()
	{
		// Display information about the closest hit
		collision = {};
		hitObjectName = "None";
		collision.distance = FLT_MAX;
		collision.hit = false;
		currentTex = &regulartex;
		currentColor = defaultColor;

		// Get ray and test against objects
		ray = GetMouseRay(GetMousePosition(), *sCamera->getRaylibCam());

		auto collisions = collisionSystem->GetCollisionsWithRay(ray);
		if (collisions.empty())
		{
			CollisionInfo empty{};
			rayCollisionResultInfo = {};
			return;
		}

	
		// Collision hit
		rayCollisionResultInfo = collisions.at(0); // Closest collision

		for (auto coll : collisions) // Avoids the actors in the scene being obscured by the terrain's bounding boxes
		{
			if (coll.collisionLayer != CollisionLayer::FLOOR)
			{
				rayCollisionResultInfo = coll;
				break;
			}
		}

        // Get the floor's mesh hit point
        if (rayCollisionResultInfo.collisionLayer == CollisionLayer::FLOOR && registry->any_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
        {
            auto& renderable = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId);
            auto meshCollision = GetRayCollisionMesh(ray, *renderable.model.meshes, renderable.model.transform);
            if (meshCollision.hit)
            {
                rayCollisionResultInfo.rlCollision = meshCollision;
            }
        }
        
		collision = rayCollisionResultInfo.rlCollision;
		onCollisionHit.publish(rayCollisionResultInfo.collidedEntityId);

		auto layer = registry->get<Collideable>(rayCollisionResultInfo.collidedEntityId).collisionLayer;
		changeCursors(layer);
	}

	void Cursor::Update()
	{
		position = {.x = GetMousePosition().x, .y = GetMousePosition().y};
		getMouseRayCollision();
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			onMouseClick();
		}
	}

	void Cursor::Draw3D()
	{
		if (!collision.hit) return;
		if (contextLocked) return;
		DrawCube(collision.point, 0.5f, 0.5f, 0.5f, currentColor);
        Vector3 normalEnd;
        normalEnd.x = collision.point.x + collision.normal.x;
        normalEnd.y = collision.point.y + collision.normal.y;
        normalEnd.z = collision.point.z + collision.normal.z;

        DrawLine3D(collision.point, normalEnd, RED);
	}

	void Cursor::Draw2D()
	{
		if (hideCursor) return;
		Vector2 pos = position;
		if (currentTex != &regulartex)
		{
			pos = Vector2Subtract(position,
			                      {
				                      static_cast<float>(currentTex->width / 2),
				                      static_cast<float>(currentTex->height / 2)
			                      });
		}
		DrawTextureEx(*currentTex, pos, 0.0, 1.0f,WHITE);
	}

	void Cursor::OnControlledActorChange(entt::entity entity)
	{
		controlledActor = entity;
	}

	Cursor::Cursor(entt::registry* _registry,
	               CollisionSystem* _collisionSystem,
	               NavigationGridSystem* _navigationGridSystem,
	               Camera* _sCamera,
	               UserInput* _userInput) :
		registry(_registry),
		collisionSystem(_collisionSystem),
		navigationGridSystem(_navigationGridSystem),
		sCamera(_sCamera),
		userInput(_userInput)
	{
		regulartex = LoadTexture("resources/textures/cursor/32/regular.png");
		talktex = LoadTexture("resources/textures/cursor/32/talk.png");
		movetex = LoadTexture("resources/textures/cursor/32/move.png");
		invalidmovetex = LoadTexture("resources/textures/cursor/32/denied.png");
		combattex = LoadTexture("resources/textures/cursor/32/attack.png");
		currentTex = &regulartex;
		EnableContextSwitching();
	}
}
