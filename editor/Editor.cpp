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
    scene = std::make_unique<EditorScene>(registry, game.get());

    //serializer::DeserializeKeyMapping(keyMapping, "resources/editorKeymapping.xml");

    game->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
}

void Editor::manageScenes()
{
    if (stateChange > 0)
    {
        delete registry;
        registry = new entt::registry();
        game = std::make_unique<Game>(registry, keyMapping);

        switch (stateChange)
        {
        case 1:
            scene = std::make_unique<GameScene>(registry, game.get());
            game->userInput->dKeyRPressed.connect<&Editor::enableEditMode>(this);
            break;
        case 2:
            scene = std::make_unique<EditorScene>(registry, game.get());
            game->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
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
        game->camera->Update();
        game->userInput->ListenForInput();
        game->cursor->Update();
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
    for (const auto& gridSquareRow : game->navigationGridSystem->GetGridSquares())
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

    BeginMode3D(*game->camera->getRaylibCam());

    // If we hit something, draw the cursor at the hit point
    game->cursor->Draw();

    scene->Draw3D();

    scene->lightSubSystem->DrawDebugLights();
    
    drawGrid();

    EndMode3D();

    game->cursor->DrawDebugText();

    scene->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
} // sage