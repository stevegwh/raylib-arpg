//
// Created by steve on 18/02/2024.
//

#include "GameManager.hpp"
#include "Serializer.hpp"
#include "Settings.hpp"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

namespace sage
{
GameManager::GameManager() :
    registry(new entt::registry())
{
    serializer::DeserializeSettings(settings, "resources/settings.xml");
    serializer::DeserializeKeyMapping(keyMapping, "resources/keybinding.xml");
    game = std::make_unique<sage::Game>(registry, keyMapping);
}

GameManager::~GameManager()
{
    delete registry;
    cleanup();
}

void GameManager::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(settings.SCREEN_WIDTH, settings.SCREEN_HEIGHT, "Baldur's Raylib");
    scene = std::make_unique<GameScene>(registry, game.get(), settings);
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
        
        // TODO: rename game to "game", rename "GameScene" to "GameScene"
        // TODO: make an "update" loop for "Game" (now called GameScene)
        game->camera->Update();
        game->userInput->ListenForInput();
        game->cursor->Update();

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

    BeginMode3D(*game->camera->getRaylibCam());

    // If we hit something, draw the cursor at the hit point
    game->cursor->Draw();

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