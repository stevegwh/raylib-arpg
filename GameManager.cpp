//
// Created by steve on 18/02/2024.
//

#include "GameManager.hpp"

#include "Registry.hpp"
#include "WorldObject.hpp"
#include "GameObjectFactory.hpp"
#include "Serializer.hpp"


namespace sage
{
GameManager::GameManager() :
sCamera(std::make_unique<sage::Camera>()),
userInput(std::make_unique<sage::UserInput>()),
ecs(std::make_unique<sage::ECSManager>(userInput.get()))
{}

GameManager::~GameManager()
{
    cleanup();
}

void GameManager::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh picking");
    
    // Should each state have its own set of systems?
    // Or, should the destructor just make sure to delete all its entities from the systems?
#ifdef EDITOR_MODE
    states.push(std::make_unique<sage::Editor>(userInput.get()));
#else
    states.push(std::make_unique<sage::Game>());
#endif

}

void GameManager::Update()
{
    init();
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

        states.top()->Update();

        //----------------------------------------------------------------------------------
        draw();
        Registry::GetInstance().RunMaintainance();
        
    }
}

void GameManager::draw()
{
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode3D(*sCamera->getCamera());

    // If we hit something, draw the cursor at the hit point
    userInput->Draw();
    
    states.top()->Draw3D();

    EndMode3D();

    userInput->DrawDebugText();
    
    states.top()->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
    
};

void GameManager::cleanup()
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