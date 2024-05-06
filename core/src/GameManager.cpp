//
// Created by steve on 18/02/2024.
//

#include "GameManager.hpp"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

namespace sage
{
GameManager::GameManager() :
    registry(new entt::registry()),
    sCamera(std::make_unique<sage::Camera>())
{
    ecs = std::make_unique<sage::ECSManager>(registry, sCamera.get(), keyMapping);
}

GameManager::~GameManager()
{
    delete registry;
    cleanup();
}

void GameManager::toggleFullScreen() 
{
    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        if (!IsWindowFullscreen())
        {
            const int current_screen = GetCurrentMonitor();
            SetWindowSize(
                GetMonitorWidth(current_screen),
                GetMonitorHeight(current_screen));
            ToggleFullscreen();
        }
        else if (IsWindowFullscreen())
        {
            ToggleFullscreen();
            SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        };
    }
}

void GameManager::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Baldur's Raylib");
    scene = std::make_unique<Game>(registry, ecs.get());
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
        sCamera->Update();
        ecs->userInput->ListenForInput();

        scene->Update();
        //----------------------------------------------------------------------------------
        draw();
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
    ecs->cursor->Draw();

    scene->Draw3D();

    EndMode3D();

    scene->Draw2D();

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