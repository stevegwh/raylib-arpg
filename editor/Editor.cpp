//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Editor.hpp"
#include "EditorScene.hpp"
#include <memory>

namespace sage
{
void Editor::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;

    InitWindow(screenWidth, screenHeight, "Baldur's Raylib");
    // scene = std::make_unique<Editor>(registry, userInput.get());
    scene = std::make_unique<EditorScene>(registry, ecs.get());
}

void Editor::Update()
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
        // TODO: replace with SceneManager
        if (stateChange > 0)
        {
            delete registry;
            registry = new entt::registry();
            ecs = std::make_unique<ECSManager>(registry, sCamera.get());
            switch (stateChange) {
            case 1:
                scene = std::make_unique<Game>(registry, ecs.get());
                break;
            case 2:
//                scene = std::make_unique<Editor>(registry, userInput.get());
//                break;
            }
            stateChange = 0;
        }
    }
}
} // sage