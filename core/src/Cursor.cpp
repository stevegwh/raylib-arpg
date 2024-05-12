//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"
#include "UserInput.hpp"
#include "components/Renderable.hpp"
#include "components/Actor.hpp"

#ifndef WIN32
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
}

void Cursor::LockContext()
{
    lockContext = true;
}

void Cursor::UnlockContext()
{
    lockContext = false;
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
        rayCollisionResultInfo = empty;
        return;
    }

    // Collision hit
    rayCollisionResultInfo = collisions.at(0); // Closest collision
    collision = rayCollisionResultInfo.rlCollision;
    onCollisionHit.publish(rayCollisionResultInfo.collidedEntityId);

    auto layer = registry->get<Collideable>(rayCollisionResultInfo.collidedEntityId).collisionLayer;
    if (layer == CollisionLayer::FLOOR) // TODO: I was expecting this to be "NAVIGATION" not "FLOOR"
    {
        currentColor = hoverColor;
        currentTex = &movetex;
        Vector2 tmp;
        if (navigationGridSystem->WorldToGridSpace(collision.point,
                                                   tmp)) // Out of map bounds (TODO: Potentially pointless, if FLOOR is the same size as bounds.)
        {
            if (registry->any_of<Actor>(controlledActor))
            {
                const auto &actor = registry->get<Actor>(controlledActor);
                Vector2 minRange;
                Vector2 maxRange;
                navigationGridSystem->GetPathfindRange(controlledActor,
                                                       actor.pathfindingBounds,
                                                       minRange,
                                                       maxRange);
                if (!navigationGridSystem->WorldToGridSpace(collision.point, tmp, minRange, maxRange)) // Out of player's movement range
                {
                    currentColor = invalidColor;
                    currentTex = &invalidmovetex;
                }
            }
        }
    }
    else if (layer == CollisionLayer::BUILDING)
    {
        currentTex = &invalidmovetex;
        currentColor = invalidColor;
        if (registry->all_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
        {
            hitObjectName = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId).name;
        }
    }
    else if (layer == CollisionLayer::NPC)
    {
        currentTex = &talktex;
        currentColor = invalidColor;
    }
}

void Cursor::Update()
{
    position = { .x = GetMousePosition().x, .y = GetMousePosition().y };
    getMouseRayCollision();
}

void Cursor::Draw3D()
{
    if (!collision.hit) return;
    if (!lockContext)
    {
        DrawCube(collision.point, 0.5f, 0.5f, 0.5f, currentColor);
    }
}

void Cursor::Draw2D()
{
    Vector2 pos = position;
    if (currentTex != &regulartex)
    {
        pos = Vector2Subtract(position,
                              {static_cast<float>(currentTex->width/2),static_cast<float>(currentTex->height/2)});
    }
    DrawTextureEx(*currentTex, pos, 0.0,1.0f,WHITE);
}

void Cursor::OnControlledActorChange(entt::entity entity)
{
    controlledActor = entity;
}

Cursor::Cursor(entt::registry* _registry,
               sage::CollisionSystem *_collisionSystem,
               sage::NavigationGridSystem* _navigationGridSystem,
               sage::Camera *_sCamera,
               sage::UserInput* _userInput) :
                registry(_registry),
                collisionSystem(_collisionSystem),
                navigationGridSystem(_navigationGridSystem),
                sCamera(_sCamera)
{
    regulartex = LoadTexture("resources/textures/cursor/32/regular.png");
    talktex = LoadTexture("resources/textures/cursor/32/talk.png");
    movetex = LoadTexture("resources/textures/cursor/32/move.png");
    invalidmovetex = LoadTexture("resources/textures/cursor/32/denied.png");
    currentTex = &regulartex;
    {
        entt::sink sink{_userInput->onClickEvent};
        sink.connect<&Cursor::onMouseClick>(this);
    }
}
}

