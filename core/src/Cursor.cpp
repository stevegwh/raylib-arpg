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

	void Cursor::LockCursor() // Lock mouse context? Like changing depending on collision.
	{
		lockContext = true;
	}

	void Cursor::UnlockCursor()
	{
		lockContext = false;
	}

	bool Cursor::isValidMove() const
	{
		GridSquare tmp;
		if (navigationGridSystem->WorldToGridSpace(collision.point,
		                                           tmp))
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
				if (!navigationGridSystem->WorldToGridSpace(collision.point, tmp, minRange, maxRange))
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
		return true;
	}

	void Cursor::changeCursors(CollisionLayer layer)
	{
		if (lockContext) return;

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


		// TODO: Currently, the bounding box of the environment obstructs the NPCs or other actor's bounding boxes. Currently using a bit of a hack, for this
		// Could use a priority queue?
		
		// Collision hit
		rayCollisionResultInfo = collisions.at(0); // Closest collision

		for (auto coll : collisions) // First NPC or Enemy collision
		{
			if (coll.collisionLayer == CollisionLayer::NPC || coll.collisionLayer == CollisionLayer::ENEMY)
			{
				rayCollisionResultInfo = coll;
			}
		}

        // If collided entity has a mesh, use this position as opposed to its bounding box
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
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			onMouseClick();
		}
	}

	void Cursor::Draw3D()
	{
		if (!collision.hit) return;
		if (lockContext) return;
		DrawCube(collision.point, 0.5f, 0.5f, 0.5f, currentColor);
        Vector3 normalEnd;
        normalEnd.x = collision.point.x + collision.normal.x;
        normalEnd.y = collision.point.y + collision.normal.y;
        normalEnd.z = collision.point.z + collision.normal.z;

        DrawLine3D(collision.point, normalEnd, RED);
	}

	void Cursor::Draw2D()
	{
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
		UnlockCursor();
	}
}
