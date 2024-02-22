//
// Created by steve on 18/02/2024.
//

#include "Game.hpp"

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

        SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
        //--------------------------------------------------------------------------------------
        // Main game loop
        while (!WindowShouldClose())        // Detect window close button or ESC key
        {
            // Update
            //----------------------------------------------------------------------------------
            sCamera->HandleInput();
            sCamera->Update();
            cursor->GetMouseRayCollision(*sCamera->getCamera(), *collisionSystem, *renderSystem);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                cursor->OnClick();

                // Place a model when mouse clicked
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

            // If we hit something, draw the cursor at the hit point
            cursor->Draw(*collisionSystem);

            DrawGrid(10, 10.0f);

            EndMode3D();

            cursor->DrawDebugText();

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