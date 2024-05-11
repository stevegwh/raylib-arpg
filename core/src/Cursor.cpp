//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"
#include "components/Renderable.hpp"

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


    if ((rayCollisionResultInfo.rayCollision.hit) && (rayCollisionResultInfo.rayCollision.distance < collision.distance))
    {
        collision = rayCollisionResultInfo.rayCollision;
        if (registry->valid(rayCollisionResultInfo.collidedEntityId))
        {
            if (registry->all_of<Renderable>(rayCollisionResultInfo.collidedEntityId))
            {
                hitObjectName = registry->get<Renderable>(rayCollisionResultInfo.collidedEntityId).name;
            }
        }
        onCollisionHitEvent.publish();
    }
}

void Cursor::Update()
{
    getMouseRayCollision();
}

void Cursor::Draw()
{

    if (collision.hit)
    {
        {
            Vector2 tmp;
            currentColor = navigationGridSystem->WorldToGridSpace(collision.point, tmp) ? hoverColor : invalidColor;
        }
        DrawCube(collision.point, 0.3f, 0.3f, 0.3f, currentColor);
        DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);

        Vector3 normalEnd;
        normalEnd.x = collision.point.x + collision.normal.x;
        normalEnd.y = collision.point.y + collision.normal.y;
        normalEnd.z = collision.point.z + collision.normal.z;

        DrawLine3D(collision.point, normalEnd, RED);
    }
    else
    {
        DrawCube(collision.point, 0.3f, 0.3f, 0.3f, defaultColor);
        DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);
    }
}

Cursor::Cursor(entt::registry* _registry,
               sage::CollisionSystem *_collisionSystem,
               sage::NavigationGridSystem* _navigationGridSystem,
               sage::Camera *_sCamera) :
                registry(_registry),
                collisionSystem(_collisionSystem),
                navigationGridSystem(_navigationGridSystem),
                sCamera(_sCamera)
{}
}

