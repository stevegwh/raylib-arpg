//
// Created by steve on 18/02/2024.
//

#include "App.h"

#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

namespace sage
{
    void App::init()
    {
        // Initialization
        //--------------------------------------------------------------------------------------
        const int screenWidth = 1280;
        const int screenHeight = 720;

        InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh picking");

        // Display information about closest hit
        hitObjectName = "None";
        collision.distance = FLT_MAX;
        collision.hit = false;
        cursorColor = WHITE;
    }

    void App::cleanup()
    {
        CloseWindow();
    }

    void App::Update()
    {
        SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
        //--------------------------------------------------------------------------------------
        // Main game loop
        while (!WindowShouldClose())        // Detect window close button or ESC key
        {

            // Update
            //----------------------------------------------------------------------------------
            sCamera->HandleInput();
            sCamera->Update();
            // Get ray and test against objects
            ray = GetMouseRay(GetMousePosition(), *sCamera->getCamera());


//        // Check ray collision against ground quad
//        RayCollision groundHitInfo = GetRayCollisionQuad(ray, g0, g1, g2, g3);
//
//        if ((groundHitInfo.hit) && (groundHitInfo.distance < collision.distance))
//        {
//            collision = groundHitInfo;
//            cursorColor = GREEN;
//            hitObjectName = "Ground";
//        }
//

            boxHitInfo = colSystem->CheckRayCollision(ray);

            if ((boxHitInfo.hit) && (boxHitInfo.distance < collision.distance))
            {
                collision = boxHitInfo;
                cursorColor = ORANGE;
                hitObjectName = "Box";

                // Check ray collision against model meshes
                RayCollision meshHitInfo = {false};
                for (int m = 0; m < towerMesh->model.meshCount; m++)
                {
                    // NOTE: We consider the model.transform for the collision check but
                    // it can be checked against any transform Matrix, used when checking against same
                    // model drawn multiple times with multiple transforms
                    meshHitInfo = GetRayCollisionMesh(ray, towerMesh->model.meshes[m], towerMesh->model.transform);
                    if (meshHitInfo.hit)
                    {
                        // Save the closest hit mesh
                        if ((!collision.hit) || (collision.distance > meshHitInfo.distance))
                            collision = meshHitInfo;

                        break;  // Stop once one mesh collision is detected, the colliding mesh is m
                    }
                }

                if (meshHitInfo.hit)
                {
                    collision = meshHitInfo;
                    cursorColor = ORANGE;
                    hitObjectName = "Mesh";
                }
            }
            //----------------------------------------------------------------------------------
            draw();
        }

    }

    void App::draw()
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(*sCamera->getCamera());

        // Draw the tower
        // WARNING: If scale is different than 1.0f,
        // not considered by GetRayCollisionModel()
        towerMesh->Draw();

        // Draw the mesh bbox if we hit it
        if (boxHitInfo.hit) DrawBoundingBox(towerMesh->boundingBox, LIME);

        // If we hit something, draw the cursor at the hit point
        if (collision.hit)
        {
            DrawCube(collision.point, 0.3f, 0.3f, 0.3f, cursorColor);
            DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);

            Vector3 normalEnd;
            normalEnd.x = collision.point.x + collision.normal.x;
            normalEnd.y = collision.point.y + collision.normal.y;
            normalEnd.z = collision.point.z + collision.normal.z;

            DrawLine3D(collision.point, normalEnd, RED);
        }

        DrawRay(ray, MAROON);

        DrawGrid(10, 10.0f);

        EndMode3D();

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

        DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    };


}