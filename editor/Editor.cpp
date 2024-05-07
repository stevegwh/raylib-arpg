//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Editor.hpp"
#include "EditorScene.hpp"
#include "Serializer.hpp"
#include "Settings.hpp"
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
    InitWindow(settings.SCREEN_WIDTH, settings.SCREEN_HEIGHT, "Baldur's Raylib");
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
        game = std::make_unique<Game>(registry, keyMapping, settings);

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
        scene->Update();

        //----------------------------------------------------------------------------------
        draw();
        manageScenes();
    }
}

void Editor::drawGrid(bool drawDebug)
{
    DrawGrid(game->navigationGridSystem->slices, game->navigationGridSystem->spacing);

//    for (int i = 0; i  < game->navigationGridSystem->gridSquares.size(); i++)
//    {
//        for (int j = 0; j < game->navigationGridSystem->gridSquares.at(0).size(); j++)
//        {
//            if (game->navigationGridSystem->gridSquares[i][j]->debugColor)
//            {
//                auto color = YELLOW;
//
//                DrawCubeWires(game->navigationGridSystem->gridSquares[i][j]->worldPosCentre,
//                              game->navigationGridSystem->gridSquares[i][j]->debugBox.x,
//                              game->navigationGridSystem->gridSquares[i][j]->debugBox.y,
//                              game->navigationGridSystem->gridSquares[i][j]->debugBox.z,
//                              color);
//            }
//        }
//    }

    if (!drawDebug) return;
    for (const auto& gridSquareRow : game->navigationGridSystem->GetGridSquares())
    {
        for (const auto& gridSquare : gridSquareRow)
        {
            auto color = gridSquare->occupied ? RED : GREEN;

            DrawCubeWires(gridSquare->worldPosCentre, 
                          gridSquare->debugBox.x, 
                          gridSquare->debugBox.y, 
                          gridSquare->debugBox.z, 
                          color);
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
    
    drawGrid(false);

    EndMode3D();

    game->cursor->DrawDebugText();

    scene->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
} // sage