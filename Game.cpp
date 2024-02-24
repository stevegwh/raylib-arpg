//
// Created by steve on 18/02/2024.
//

#include "Game.hpp"

#include <utility>

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
    
    void Game::createTower(Vector3 position, const char* name) const
    {
        EntityID newTowerId = Registry::GetInstance().CreateEntity();
        sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png") };

        auto towerRenderable1 = new Renderable(newTowerId, LoadModel("resources/models/obj/turret.obj"), mat);
        towerRenderable1->name = name;

        auto towerTransform1 = new Transform(newTowerId);
        towerTransform1->position = position;
        towerTransform1->scale = 1.0f;

        auto towerCollidable1 = new Collideable(newTowerId, towerRenderable1->meshBoundingBox);
        towerCollidable1->worldBoundingBox.min = Vector3Add(towerCollidable1->worldBoundingBox.min, towerTransform1->position);
        towerCollidable1->worldBoundingBox.max = Vector3Add(towerCollidable1->worldBoundingBox.max, towerTransform1->position);
        towerCollidable1->collisionLayer = BUILDING;

        transformSystem->AddComponent(*towerTransform1);
        renderSystem->AddComponent(*towerRenderable1);
        collisionSystem->AddComponent(*towerCollidable1);
        auto towerWorldObject1 = new WorldObject(newTowerId);
        worldSystem->AddComponent(*towerWorldObject1);
        
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
                cursor->OnClick(*collisionSystem);
            }
            //----------------------------------------------------------------------------------
            draw();
        }

    }

    void Game::draw()
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(*sCamera->getCamera());

        // If we hit something, draw the cursor at the hit point
        cursor->Draw(*collisionSystem);

        renderSystem->Draw();

        gameEditor->Draw();

        DrawGrid(100, 1.0f);

        EndMode3D();

        cursor->DrawDebugText();

        DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
        
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