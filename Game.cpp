//
// Created by steve on 18/02/2024.
//

#include "Game.hpp"

#define FLT_MAX     340282346638528859811704183484516925440.0f     // Maximum value of a float, from bit pattern 01111111011111111111111111111111

namespace sage
{
    void Game::init()
    {
        // Initialization
        //--------------------------------------------------------------------------------------
        const int screenWidth = 1280;
        const int screenHeight = 720;

        InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh picking");
        
    }

    void Game::Update()
    {
        Ray ray = { 0 };        // Picking ray

        // Ground quad
        Vector3 g0 = (Vector3){ -50.0f, 0.0f, -50.0f };
        Vector3 g1 = (Vector3){ -50.0f, 0.0f,  50.0f };
        Vector3 g2 = (Vector3){  50.0f, 0.0f,  50.0f };
        Vector3 g3 = (Vector3){  50.0f, 0.0f, -50.0f };

        SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
        //--------------------------------------------------------------------------------------
        // Main game loop
        while (!WindowShouldClose())        // Detect window close button or ESC key
        {
            // Update
            //----------------------------------------------------------------------------------
            sCamera->HandleInput();
            sCamera->Update();

            // Display information about closest hit
            RayCollision collision = { 0 };
            hitObjectName = "None";
            collision.distance = FLT_MAX;
            collision.hit = false;
            Color cursorColor = WHITE;


//            // Check ray collision against ground quad
//            RayCollision groundHitInfo = GetRayCollisionQuad(ray, g0, g1, g2, g3);
//
//            if ((groundHitInfo.hit) && (groundHitInfo.distance < collision.distance))
//            {
//                collision = groundHitInfo;
//                cursorColor = GREEN;
//                hitObjectName = "Ground";
//            }

            // Get ray and test against objects
            ray = GetMouseRay(GetMousePosition(), *sCamera->getCamera());


            CollisionInfo boxHitInfo = collisionSystem->CheckRayCollision(ray);

            if ((boxHitInfo.rayCollision.hit) && (boxHitInfo.rayCollision.distance < collision.distance))
            {
                collision = boxHitInfo.rayCollision;
                cursorColor = ORANGE;
                
                if (renderSystem->EntityExists(boxHitInfo.collidedObject))
                {
                    hitObjectName = renderSystem->GetComponent(boxHitInfo.collidedObject).name;
                }
                
                
                /*
                // Check ray collision against model meshes
                RayCollision meshHitInfo = { 0 };
                for (int m = 0; m < tower->model.meshCount; m++)
                {
                    // NOTE: We consider the model.transform for the collision check but 
                    // it can be checked against any transform Matrix, used when checking against same
                    // model drawn multiple times with multiple transforms
                    meshHitInfo = GetRayCollisionMesh(ray, tower->model.meshes[m], tower->model.transform);
                    if (meshHitInfo.hit)
                    {
                        // Save the closest hit mesh
                        if ((!collision.hit) || (collision.distance > meshHitInfo.distance)) collision = meshHitInfo;

                        break;  // Stop once one mesh collision is detected, the colliding mesh is m
                    }
                }

                if (meshHitInfo.hit)
                {
                    collision = meshHitInfo;
                    cursorColor = ORANGE;
                    hitObjectName = "Renderable";
                }
                 */
            }
            //----------------------------------------------------------------------------------

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(*sCamera->getCamera());

            // Draw the tower
            // WARNING: If scale is different than 1.0f,
            // not considered by GetRayCollisionModel()
            renderSystem->Draw();

            // Draw the mesh bbox if we hit it
            if (boxHitInfo.rayCollision.hit) 
            {
                BoundingBox boundingBox = collisionSystem->GetComponent(boxHitInfo.collidedObject).boundingBox;
                DrawBoundingBox(boundingBox, LIME);
            }

            // If we hit something, draw the cursor at the hit point
            cursor->Draw(collision);
//            if (collision.hit)
//            {
//                DrawCube(collision.point, 0.3f, 0.3f, 0.3f, cursorColor);
//                DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);
//
//                Vector3 normalEnd;
//                normalEnd.x = collision.point.x + collision.normal.x;
//                normalEnd.y = collision.point.y + collision.normal.y;
//                normalEnd.z = collision.point.z + collision.normal.z;
//
//                DrawLine3D(collision.point, normalEnd, RED);
//            }

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
        }

    }

    void Game::draw()
    {
        
    };

    void Game::cleanup()
    {
        CloseWindow();
    }
}