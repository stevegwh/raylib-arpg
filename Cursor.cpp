//
// Created by Steve Wheeler on 18/02/2024.
//

#include "Cursor.hpp"
#include <iostream>

#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

namespace sage
{

    void Cursor::GetMouseRayCollision(Camera3D raylibCamera, const CollisionSystem& colSystem, const RenderSystem& renderSystem)
    {
        // Display information about closest hit
        collision = {};
        hitObjectName = "None";
        collision.distance = FLT_MAX;
        collision.hit = false;

        // Get ray and test against objects
        ray = GetMouseRay(GetMousePosition(), raylibCamera);

        auto collisions = colSystem.CheckRayCollision(ray);
        rayCollisionResultInfo = collisions.empty() ? (CollisionInfo){0, {}} : colSystem.CheckRayCollision(ray).at(0);


        if ((rayCollisionResultInfo.rayCollision.hit) && (rayCollisionResultInfo.rayCollision.distance < collision.distance))
        {
            collision = rayCollisionResultInfo.rayCollision;

            if (renderSystem.FindEntity(rayCollisionResultInfo.collidedEntityId))
            {
                hitObjectName = renderSystem.GetComponent(rayCollisionResultInfo.collidedEntityId)->name;
            }

            OnCollisionHitEvent->InvokeAllCallbacks();
        }
    }

    void Cursor::OnClick(const CollisionSystem& colSystem)
    {
        OnClickEvent->InvokeAllCallbacks();
        //std::cout << "Hit object position: " << collision.point.x << ", " << collision.point.y << ", " << collision.point.z << "\n";
    }

    void Cursor::Draw(const CollisionSystem& colSystem)
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
        if (rayCollisionResultInfo.rayCollision.hit)
        {
            auto col = colSystem.GetComponent(rayCollisionResultInfo.collidedEntityId);
            if (col->collisionLayer == FLOOR)
            {
                colSystem.BoundingBoxDraw(rayCollisionResultInfo.collidedEntityId, ORANGE);
            }
            else
            {
                colSystem.BoundingBoxDraw(rayCollisionResultInfo.collidedEntityId);
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
        }
    }

}
