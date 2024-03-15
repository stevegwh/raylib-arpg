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
        const int screenWidth = SCREEN_WIDTH;
        const int screenHeight = SCREEN_HEIGHT;

        InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh picking");
    }
    
    void Game::removeTower(EntityID entityId)
    {
        Registry::GetInstance().DeleteEntity(entityId);
    }
    
    EntityID Game::createPlayer(Vector3 position, const char* name) const
    {
        EntityID id = Registry::GetInstance().CreateEntity();
        sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png") };

        auto renderable = std::make_unique<Renderable>(id, LoadModel("resources/models/obj/cube_steve.obj"), mat);
        renderable->name = name;

        auto transform = std::make_unique<Transform>(id);
        transform->position = position;
        transform->scale = 1.0f;

        auto collideable = std::make_unique<Collideable>(id, renderable->meshBoundingBox);
        collideable->worldBoundingBox.min = Vector3Add(collideable->worldBoundingBox.min, transform->position);
        collideable->worldBoundingBox.max = Vector3Add(collideable->worldBoundingBox.max, transform->position);
        collideable->collisionLayer = PLAYER;

        auto towerWorldObject1 = std::make_unique<WorldObject>(id);

        renderSystem->AddComponent(std::move(renderable));
        transformSystem->AddComponent(std::move(transform));
        collisionSystem->AddComponent(std::move(collideable));
        worldSystem->AddComponent(std::move(towerWorldObject1));
        return id;
    }
    
    void Game::createTower(Vector3 position, const char* name) const
    {
        EntityID newTowerId = Registry::GetInstance().CreateEntity();
        sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png") };

        auto towerRenderable1 = std::make_unique<Renderable>(newTowerId, LoadModel("resources/models/obj/turret.obj"), mat);
        towerRenderable1->name = name;

        auto towerTransform1 = std::make_unique<Transform>(newTowerId);
        towerTransform1->position = position;
        towerTransform1->scale = 1.0f;

        auto towerCollidable1 = std::make_unique<Collideable>(newTowerId, towerRenderable1->meshBoundingBox);
        towerCollidable1->worldBoundingBox.min = Vector3Add(towerCollidable1->worldBoundingBox.min, towerTransform1->position);
        towerCollidable1->worldBoundingBox.max = Vector3Add(towerCollidable1->worldBoundingBox.max, towerTransform1->position);
        towerCollidable1->collisionLayer = BUILDING;

        auto towerWorldObject1 = std::make_unique<WorldObject>(newTowerId);

        renderSystem->AddComponent(std::move(towerRenderable1));
        transformSystem->AddComponent(std::move(towerTransform1));
        collisionSystem->AddComponent(std::move(towerCollidable1));
        worldSystem->AddComponent(std::move(towerWorldObject1));
    }

    void Game::Update()
    {
        navigationGridSystem->PopulateGrid();
        SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
        //--------------------------------------------------------------------------------------
        // Main game loop
        while (!WindowShouldClose())        // Detect window close button or ESC key
        {
            // Update
            //----------------------------------------------------------------------------------
            sCamera->HandleInput(); // TODO: Should merge this with userInput
            sCamera->Update();
            userInput->ListenForInput();
            transformSystem->Update();

            //----------------------------------------------------------------------------------
            draw();
            Registry::GetInstance().RunMaintainance();
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
        userInput->Draw();

        renderSystem->Draw();

#ifdef EDITOR_MODE
        gameEditor->Draw();
#endif

        DrawGrid(100, 1.0f);
        
//        for (const auto& gridSquare : navigationGridSystem->GetGridSquares())
//        {
//            auto bb = collisionSystem->GetComponent(gridSquare->entityId)->worldBoundingBox;
//            bb.max.y = 0.1f;
//            Color color = gridSquare->occupied ? RED : GREEN;
//            DrawBoundingBox(bb, color);
//        }
        for (const auto& gridSquareRow : navigationGridSystem->GetGridSquares())
        {
            for (const auto& gridSquare : gridSquareRow) 
            {
                BoundingBox bb;
                bb.min = gridSquare->worldPosMin;
                bb.max = gridSquare->worldPosMax;
                bb.max.y = 0.1f;
                Color color = gridSquare->occupied ? RED : GREEN;
                DrawBoundingBox(bb, color);
            }
        }

        EndMode3D();

        userInput->DrawDebugText();
        
#ifdef EDITOR_MODE
        gameEditor->DrawDebugText();
#endif

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