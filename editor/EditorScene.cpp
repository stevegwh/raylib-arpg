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
    newTransform.position = data->cursor->collision.point;
    const auto& renderable = registry->get<Renderable>(selectedObject);
    registry->patch<Transform>(selectedObject, [&newTransform] (auto& t) {
        t.position = newTransform.position;
    });
    Matrix mat = registry->get<Transform>(selectedObject).GetMatrixNoRot();
    data->collisionSystem->UpdateWorldBoundingBox(selectedObject, mat); // TODO: Would prefer to have this as an event
}

void EditorScene::OnCursorClick()
{
    if (data->cursor->collision.hit)
    {
        
        switch (registry->get<Collideable>(data->cursor->rayCollisionResultInfo.collidedEntityId).collisionLayer)
        {
        case DEFAULT:
            break;
        case FLOOR:
            if (currentEditorMode == CREATE)
            {
                GameObjectFactory::createBuilding(registry, data, data->cursor->collision.point, 
                                                  "Tower Instance",
                                                  "resources/models/obj/castle.obj",
                                                  "resources/models/obj/castle_diffuse.png");
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
            selectedObject = data->cursor->rayCollisionResultInfo.collidedEntityId;
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
    data->Save();
}

void EditorScene::OnSerializeLoad()
{
    data->Load();
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
    data->navigationGridSystem->PopulateGrid();
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
        data->collisionSystem->BoundingBoxDraw(selectedObject, ORANGE);
    }

    data->renderSystem->Draw();
}


void EditorScene::Draw2D()
{
    std::string mode = "NONE";
    if (currentEditorMode == IDLE) mode = "IDLE";
    else if (currentEditorMode == SELECT) mode = "SELECT";
    else if (currentEditorMode == MOVE) mode = "MOVE";
    else if (currentEditorMode == CREATE) mode = "CREATE";
    
    gui->Draw(mode, data->cursor.get());
}

EditorScene::EditorScene(entt::registry* _registry, GameData* _data) :
    Scene(_registry, _data), gui(std::make_unique<editor::GUI>(data->settings, data->userInput.get()))
{
    data->userInput->dOnClickEvent.connect<&EditorScene::OnCursorClick>(this);
    data->cursor->dOnCollisionHitEvent.connect<&EditorScene::OnCollisionHit>(this);
    data->userInput->dKeyDeletePressed.connect<&EditorScene::OnDeleteModeKeyPressed>(this);
    data->userInput->dKeyPPressed.connect<&EditorScene::OnCreateModeKeyPressed>(this);
    data->userInput->dKeyGPressed.connect<&EditorScene::OnGenGridKeyPressed>(this);
    data->userInput->dKeyMPressed.connect<&EditorScene::OnSerializeSave>(this);
    data->userInput->dKeyNPressed.connect<&EditorScene::OnSerializeLoad>(this);

    gui->saveButtonPressed.connect<&EditorScene::OnSerializeSave>(this);
    gui->loadButtonPressed.connect<&EditorScene::OnSerializeLoad>(this);

    BoundingBox bb = {
        .min = (Vector3){ -100.0f, 0.1f, -100.0f },
        .max = (Vector3){  100.0f, 0.1f,  100.0f }
    };
    GameObjectFactory::createFloor(registry, this, bb);

    data->Load();
    data->navigationGridSystem->Init(100, 1.0f);
    data->navigationGridSystem->PopulateGrid();
}

EditorScene::~EditorScene()
{

}
}
