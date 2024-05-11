//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"
#include "components/Renderable.hpp"
#include "components/Actor.hpp"

#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111
namespace sage
{
void Cursor::getMouseRayCollision()
{
    // Display information about closest hit
    collision = {};
    hitObjectName = "None";
    collision.distance = FLT_MAX;
    collision.hit = false;

    // Get ray and test against objects
    ray = GetMouseRay(GetMousePosition(), *sCamera->getRaylibCam());

    auto collisions = collisionSystem->GetCollisionsWithRay(ray);
    rayCollisionResultInfo = collisions.empty() ? (CollisionInfo){{}, {}} : collisionSystem
        ->GetCollisionsWithRay(ray).at(0);

    if (rayCollisionResultInfo.rlCollision.hit)
    {
        auto layer = registry->get<Collideable>(rayCollisionResultInfo.collidedEntityId).collisionLayer;
        if (layer == FLOOR) // TODO: I was expecting this to be "NAVIGATION" not "FLOOR"
        {
            currentTex = &movetex;
            if ((rayCollisionResultInfo.rlCollision.distance < collision.distance))
            {
                collision = rayCollisionResultInfo.rlCollision;
                if (registry->valid(rayCollisionResultInfo.collidedEntityId))
                {
                    if (registry->all_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
                    {
                        hitObjectName = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId).name;
                    }
                }
                onCollisionHitEvent.publish(); // TODO: rename the event to something descriptive (onNavigationLayerHit?)
            }
        }
        else if (layer == BUILDING)
        {
            if ((rayCollisionResultInfo.rlCollision.distance < collision.distance))
            {
                collision = rayCollisionResultInfo.rlCollision;
                if (registry->valid(rayCollisionResultInfo.collidedEntityId))
                {
                    if (registry->all_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
                    {
                        hitObjectName = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId).name;
                    }
                }
                onCollisionHitEvent.publish(); // TODO: rename the event to something descriptive (onNavigationLayerHit?)
            }
        }
        else if (layer == NPC)
        {
            currentTex = &talktex;
        }
        else
        {
            currentTex = &regulartex;
        }

    }
}

void Cursor::Update()
{
    position = (Vector2){ .x = GetMousePosition().x, .y = GetMousePosition().y };
    getMouseRayCollision();
}

void Cursor::Draw3D()
{
    auto layer = registry->get<Collideable>(rayCollisionResultInfo.collidedEntityId).collisionLayer;
    if (collision.hit && layer == FLOOR) // TODO: States would be much cleaner
    {
        {
            Vector2 tmp;
            if (navigationGridSystem->WorldToGridSpace(collision.point, tmp)) // Out of map bounds
            {
                currentColor = hoverColor;
                currentTex = &movetex;
                if (registry->any_of<Actor>(controlledActor)) // Out of player bounds
                {
                    const auto& actor = registry->get<Actor>(controlledActor);
                    Vector2 minRange;
                    Vector2 maxRange;
                    navigationGridSystem->GetPathfindRange(controlledActor, actor.pathfindingBounds, minRange, maxRange);
                    Vector2 tmp;
                    if (!navigationGridSystem->WorldToGridSpace(collision.point, tmp, minRange, maxRange))
                    {
                        currentColor = RED;
                        currentTex = &invalidmovetex;
                    }
                }
            }
            else 
            {
                currentColor = RED;
                currentTex = &invalidmovetex;
            }
        }
        DrawCube(collision.point, 0.5f, 0.5f, 0.5f, currentColor);
        //DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);
//        Vector3 normalEnd;
//        normalEnd.x = collision.point.x + collision.normal.x;
//        normalEnd.y = collision.point.y + collision.normal.y;
//        normalEnd.z = collision.point.z + collision.normal.z;
        //DrawLine3D(collision.point, normalEnd, RED);
    }
    else
    {
        DrawCube(collision.point, 0.3f, 0.3f, 0.3f, defaultColor);
        DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);
    }
}

void Cursor::Draw2D()
{
    DrawTextureEx(*currentTex, position, 0.0, 0.6f, WHITE);
}

void Cursor::OnControlledActorChange(entt::entity entity)
{
    controlledActor = entity;
}

Cursor::Cursor(entt::registry* _registry,
               sage::CollisionSystem *_collisionSystem,
               sage::NavigationGridSystem* _navigationGridSystem,
               sage::Camera *_sCamera) :
                registry(_registry),
                collisionSystem(_collisionSystem),
                navigationGridSystem(_navigationGridSystem),
                sCamera(_sCamera)
{
    regulartex = LoadTexture("resources/textures/cursor/regular.png");
    talktex = LoadTexture("resources/textures/cursor/talk.png");
    movetex = LoadTexture("resources/textures/cursor/move.png");
    invalidmovetex = LoadTexture("resources/textures/cursor/deniedred.png");
    currentTex = &regulartex;
}
}

