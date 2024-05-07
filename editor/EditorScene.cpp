//
// Created by steve on 22/02/2024.
//

#include "EditorScene.hpp"
#include "../core/src/GameObjectFactory.hpp"
#include "raygui.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <iostream>

namespace sage
{

void EditorScene::moveSelectedObjectToCursorHit()
{
    Transform newTransform;
    newTransform.position = game->cursor->collision.point;
    const auto& renderable = registry->get<Renderable>(selectedObject);
    registry->patch<Transform>(selectedObject, [&newTransform] (auto& t) {
        t.position = newTransform.position;
    });
    Matrix mat = registry->get<Transform>(selectedObject).GetMatrixNoRot();
    game->collisionSystem->UpdateWorldBoundingBox(selectedObject, mat); // TODO: Would prefer to have this as an event
}

void EditorScene::OnCursorClick()
{
    if (game->cursor->collision.hit)
    {
        
        switch (registry->get<Collideable>(game->cursor->rayCollisionResultInfo.collidedEntityId).collisionLayer)
        {
        case DEFAULT:
            break;
        case FLOOR:
            if (currentEditorMode == CREATE)
            {
                GameObjectFactory::createTower(registry, game, game->cursor->collision.point, "Tower Instance");
            }
            else if (currentEditorMode == SELECT)
            {
                moveSelectedObjectToCursorHit();
                selectedObject = {};
                currentEditorMode = IDLE;
            }
            break;
        case BUILDING:
            currentEditorMode = SELECT;
            selectedObject = game->cursor->rayCollisionResultInfo.collidedEntityId;
            break;
        }
    }
    else
    {
        selectedObject = {};
    }
}

void EditorScene::OnSerializeSave()
{
    game->Save();
}

void EditorScene::OnSerializeLoad()
{
    game->Load();
}

void EditorScene::OnDeleteModeKeyPressed()
{
    if (currentEditorMode != SELECT) return;
    registry->destroy(selectedObject);
    selectedObject = {};
    currentEditorMode = IDLE;
}

void EditorScene::OnCreateModeKeyPressed()
{
    if (currentEditorMode == CREATE) currentEditorMode = IDLE;
    else currentEditorMode = CREATE;
}

void EditorScene::OnGenGridKeyPressed()
{
    game->navigationGridSystem->PopulateGrid();
}

void EditorScene::OnCollisionHit()
{
    //std::cout << "Collision detected. \n";
    //    if (colSystem.GetComponent(rayCollisionResultInfo.collidedEntityId).collisionLayer == FLOOR)
    //    {
    //        // Place model
    //    }
    //    else
    //    {
    //        // Select model
    //        // Store entityID of selected model
    //        // Change bounding box colour
    //    }
}

void EditorScene::Update()
{
    
} 

void EditorScene::Draw3D()
{
    if (currentEditorMode == SELECT)
    {
        game->collisionSystem->BoundingBoxDraw(selectedObject, ORANGE);
    }

    game->renderSystem->Draw();
}


void EditorScene::Draw2D()
{
    std::string mode = "NONE";
    if (currentEditorMode == IDLE) mode = "IDLE";
    else if (currentEditorMode == SELECT) mode = "SELECT";
    else if (currentEditorMode == MOVE) mode = "MOVE";
    else if (currentEditorMode == CREATE) mode = "CREATE";

    DrawText(TextFormat("Editor Mode: %s", mode.c_str()), settings.SCREEN_WIDTH - 150, 50, 10, BLACK);
    
    gui->Draw();
}

EditorScene::EditorScene(entt::registry* _registry, Game* _game, Settings _settings) :
    Scene(_registry, _game, _settings), gui(std::make_unique<editor::GUI>())
{
    game->userInput->dOnClickEvent.connect<&EditorScene::OnCursorClick>(this);
    game->cursor->dOnCollisionHitEvent.connect<&EditorScene::OnCollisionHit>(this);
    game->userInput->dKeyDeletePressed.connect<&EditorScene::OnDeleteModeKeyPressed>(this);
    game->userInput->dKeyPPressed.connect<&EditorScene::OnCreateModeKeyPressed>(this);
    game->userInput->dKeyGPressed.connect<&EditorScene::OnGenGridKeyPressed>(this);
    game->userInput->dKeyMPressed.connect<&EditorScene::OnSerializeSave>(this);
    game->userInput->dKeyNPressed.connect<&EditorScene::OnSerializeLoad>(this);

    gui->saveButtonPressed.connect<&EditorScene::OnSerializeSave>(this);
    gui->loadButtonPressed.connect<&EditorScene::OnSerializeLoad>(this);

//    BoundingBox bb = {
//        .min = (Vector3){ -50.0f, 0.1f, -50.0f },
//        .max = (Vector3){  50.0f, 0.1f,  50.0f }
//    };

    BoundingBox bb = {
        .min = (Vector3){ -100.0f, 0.1f, -100.0f },
        .max = (Vector3){  100.0f, 0.1f,  100.0f }
    };
    GameObjectFactory::createFloor(registry, this, bb);

    game->Load();
    game->navigationGridSystem->Init(100, 1.0f);
    game->navigationGridSystem->PopulateGrid();
}

EditorScene::~EditorScene()
{

}
}
