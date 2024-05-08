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
    InitWindow(settings->SCREEN_WIDTH, settings->SCREEN_HEIGHT, "Baldur's Raylib");
    scene = std::make_unique<EditorScene>(registry.get(), data.get());
    data->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
}

void Editor::manageScenes()
{
    if (stateChange > 0)
    {
        registry = std::make_unique<entt::registry>();
        data = std::make_unique<sage::GameData>(registry.get(), keyMapping.get(), settings.get());

        switch (stateChange)
        {
        case 1:
            scene = std::make_unique<ExampleScene>(registry.get(), data.get());
            data->userInput->dKeyRPressed.connect<&Editor::enableEditMode>(this);
            break;
        case 2:
            scene = std::make_unique<EditorScene>(registry.get(), data.get());
            data->userInput->dKeyRPressed.connect<&Editor::enablePlayMode>(this);
            break;
        }
        stateChange = 0;
    }
    
}

void Editor::drawDebugCollisionText()
{
    // Draw some debug GUI text
    DrawText(TextFormat("Hit Object: %s", data->cursor->hitObjectName.c_str()), 10, 50, 10, BLACK);

    if (data->cursor->collision.hit)
    {
        int ypos = 70;

        DrawText(TextFormat("Distance: %3.2f", data->cursor->collision.distance), 10, ypos, 10, BLACK);

        DrawText(TextFormat("Hit Pos: %3.2f %3.2f %3.2f",
                            data->cursor->collision.point.x,
                            data->cursor->collision.point.y,
                            data->cursor->collision.point.z), 10, ypos + 15, 10, BLACK);

        DrawText(TextFormat("Hit Norm: %3.2f %3.2f %3.2f",
                            data->cursor->collision.normal.x,
                            data->cursor->collision.normal.y,
                            data->cursor->collision.normal.z), 10, ypos + 30, 10, BLACK);

        DrawText(TextFormat("Entity ID: %d", data->cursor->rayCollisionResultInfo.collidedEntityId), 10,
                 ypos + 45, 10, BLACK);

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
        data->camera->Update();
        data->userInput->ListenForInput();
        data->cursor->Update();
        scene->Update();

        //----------------------------------------------------------------------------------
        draw();
        manageScenes();
    }
}

void Editor::drawGrid(bool drawDebug)
{
    DrawGrid(data->navigationGridSystem->slices, data->navigationGridSystem->spacing);

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
    for (const auto& gridSquareRow : data->navigationGridSystem->GetGridSquares())
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

    BeginMode3D(*data->camera->getRaylibCam());

    // If we hit something, draw the cursor at the hit point
    data->cursor->Draw();

    scene->Draw3D();

    scene->lightSubSystem->DrawDebugLights();

    drawGrid(false);

    EndMode3D();

    drawDebugCollisionText();

    scene->Draw2D();

    DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
} // sage