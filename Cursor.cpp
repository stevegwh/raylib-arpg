//
// Created by Steve Wheeler on 18/02/2024.
//

#include "Cursor.hpp"

namespace sage
{

    void Cursor::Draw(const RayCollision& collision)
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
        
    }

}
