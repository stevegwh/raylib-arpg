//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Editor.hpp"
#include "../src/scenes/ExampleScene.hpp"
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

void Editor::initEditorScene()
{
    auto data = std::make_unique<sage::GameData>(registry.get(), keyMapping.get(), settings.get());
    scene = std::make_unique<EditorScene>(registry.get(), std::move(data));
    scene->data->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
}

void Editor::initGameScene()
{
    auto data = std::make_unique<sage::GameData>(registry.get(), keyMapping.get(), settings.get());
    data->userInput->dKeyRPressed.connect<&Editor::enableEditMode>(this);
    scene = std::make_unique<ExampleScene>(registry.get(), std::move(data));
}

void Editor::init()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(settings->SCREEN_WIDTH, settings->SCREEN_HEIGHT, "Baldur's Raylib");
    initEditorScene();
}

void Editor::manageScenes()
{
    if (stateChange > 0)
    {
        registry = std::make_unique<entt::registry>();
        switch (stateChange)
        {
        case 1:
            initGameScene();
            break;
        case 2:
            initEditorScene();
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
        scene->Update();
        //----------------------------------------------------------------------------------
        draw();
        manageScenes();
    }
}

void Editor::drawGrid(bool drawDebug)
{
    DrawGrid(scene->data->navigationGridSystem->slices, scene->data->navigationGridSystem->spacing);

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
    for (const auto& gridSquareRow : scene->data->navigationGridSystem->GetGridSquares())
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

    BeginMode3D(*scene->data->camera->getRaylibCam());

    scene->Draw3D();

    drawGrid(false);

    EndMode3D();

    scene->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
} // sage