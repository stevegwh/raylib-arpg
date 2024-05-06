//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Editor.hpp"
#include "EditorScene.hpp"
#include "Serializer.hpp"
#include <memory>

namespace sage
{

void Editor::enableEditMode()
{
    stateChange = 2;
}

void Editor::enablePlayMode()
{
    stateChange = 1;
}

void Editor::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Baldur's Raylib");
    scene = std::make_unique<EditorScene>(registry, ecs.get());

    //serializer::DeserializeKeyMapping(keyMapping, "resources/editorKeymapping.xml");

    ecs->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
}

void Editor::manageScenes()
{
    if (stateChange > 0)
    {
        delete registry;
        registry = new entt::registry();
        ecs = std::make_unique<ECSManager>(registry, keyMapping);

        switch (stateChange)
        {
        case 1:
            scene = std::make_unique<GameScene>(registry, ecs.get());
            ecs->userInput->dKeyRPressed.connect<&Editor::enableEditMode>(this);
            break;
        case 2:
            scene = std::make_unique<EditorScene>(registry, ecs.get());
            ecs->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
            break;
        }
        stateChange = 0;
    }
    
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
        ecs->camera->Update();
        ecs->userInput->ListenForInput();
        ecs->cursor->Update();
        toggleFullScreen(); // checks for full screen TODO: tmp

        scene->Update();

        //----------------------------------------------------------------------------------
        draw();
        manageScenes();
    }
}

void Editor::drawGrid()
{
    DrawGrid(100, 1.0f);
    for (const auto& gridSquareRow : ecs->navigationGridSystem->GetGridSquares())
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
}

void Editor::draw()
{
// Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode3D(*ecs->camera->getRaylibCam());

    // If we hit something, draw the cursor at the hit point
    ecs->cursor->Draw();

    scene->Draw3D();

    scene->lightSubSystem->DrawDebugLights();
    
    drawGrid();

    EndMode3D();

    ecs->cursor->DrawDebugText();

    scene->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
} // sage