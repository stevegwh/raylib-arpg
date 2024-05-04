//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"
#include "components/Renderable.hpp"

#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111
namespace sage
{
bool Cursor::GetMouseRayCollision()
{
    // Display information about closest hit
    collision = {};
    hitObjectName = "None";
    collision.distance = FLT_MAX;
    collision.hit = false;

    // Get ray and test against objects
    ray = GetMouseRay(GetMousePosition(), *sCamera->getCamera());

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
        return true;
    }
    return false;
}

void Cursor::Draw()
{

    if (collision.hit)
    {
        DrawCube(collision.point, 0.3f, 0.3f, 0.3f, hoverColor);
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


    // Draw the mesh bbox if we hit it
    if (rayCollisionResultInfo.rayCollision.hit && registry->valid(rayCollisionResultInfo.collidedEntityId))
    {
        const auto& col = registry->get<Collideable>(rayCollisionResultInfo.collidedEntityId);
        if (col.collisionLayer == FLOOR)
        {
            collisionSystem->BoundingBoxDraw(rayCollisionResultInfo.collidedEntityId, ORANGE);
        }
        else
        {
            collisionSystem->BoundingBoxDraw(rayCollisionResultInfo.collidedEntityId);
        }

    }
}

void Cursor::DrawDebugText() const
{
    // Draw some debug GUI text
    DrawText(TextFormat("Hit Object: %s", hitObjectName.c_str()), 10, 50, 10, BLACK);

    if (collision.hit)
    {
        int ypos = 70;

        DrawText(TextFormat("Distance: %3.2f", collision.distance), 10, ypos, 10, BLACK);

        DrawText(TextFormat("Hit Pos: %3.2f %3.2f %3.2f",
                            collision.point.x,
                            collision.point.y,
                            collision.point.z), 10, ypos + 15, 10, BLACK);

        DrawText(TextFormat("Hit Norm: %3.2f %3.2f %3.2f",
                            collision.normal.x,
                            collision.normal.y,
                            collision.normal.z), 10, ypos + 30, 10, BLACK);

        DrawText(TextFormat("Entity ID: %d", rayCollisionResultInfo.collidedEntityId), 10,
                 ypos + 45, 10, BLACK);



    }
}
Cursor::Cursor(entt::registry* _registry,
               sage::CollisionSystem *_collisionSystem, 
               sage::Camera *_sCamera) :
                registry(_registry),
                collisionSystem(_collisionSystem), 
                sCamera(_sCamera)
{}
}

