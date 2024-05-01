//
// Created by steve on 18/02/2024.
//

#include "GameManager.hpp"

#include "Registry.hpp"
#include "WorldObject.hpp"


#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

namespace sage
{
GameManager::GameManager() :
    registry(new entt::registry()),
    sCamera(std::make_unique<sage::Camera>()),
    userInput(std::make_unique<sage::UserInput>(registry)),
    ecs(std::make_unique<sage::ECSManager>(registry, userInput.get()))
{}

GameManager::~GameManager()
{
    delete registry;
    cleanup();
}

void GameManager::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;

    InitWindow(screenWidth, screenHeight, "Baldur's Raylib");
    
#ifdef EDITOR_MODE
    scene = std::make_unique<Editor>(userInput.get());
#else
    scene = std::make_unique<Game>(registry, userInput.get());
#endif

}

//Model model;
//Light lights[MAX_LIGHTS] = { 0 };

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

        scene->Update();

        //----------------------------------------------------------------------------------
        draw();
        Registry::GetInstance().RunMaintainance();
        // temporary
        if (stateChange > 0)
        {
            if (stateChange == 1)
            {
                delete registry; // TODO: will completely break everything
                registry = new entt::registry();
                ecs = std::make_unique<ECSManager>(registry, userInput.get());
                scene = std::make_unique<Game>(registry, userInput.get());
            }
            else if (stateChange == 2)
            {
                delete registry;
                registry = new entt::registry();
                ecs = std::make_unique<ECSManager>(registry, userInput.get());
                scene = std::make_unique<Editor>(registry, userInput.get());
            }
            stateChange = 0;
        }
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

    scene->Draw3D();
    
    scene->lightSubSystem->DrawDebugLights();

    EndMode3D();

    userInput->DrawDebugText();

    scene->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
    
};

void GameManager::cleanup()
{
    CloseWindow();
}

void GameManager::SetStateEditor()
{
    stateChange = 2;

}

void GameManager::SetStateRun()
{
    stateChange = 1;
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