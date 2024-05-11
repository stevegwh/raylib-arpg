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
    if (gui->focused) return;
    if (data->cursor->collision.hit)
    {
        switch (registry->get<Collideable>(data->cursor->rayCollisionResultInfo.collidedEntityId).collisionLayer)
        {
        case DEFAULT:
            break;
        case FLOOR:
            if (currentEditorMode == CREATE)
            {
                GameObjectFactory::createBuilding(registry, data.get(), data->cursor->collision.point,
                                                  "Tower Instance",
                                                  "resources/models/obj/turret.obj",
                                                  "resources/models/obj/turret_diffuse.png");
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
    std::cout << "Delete pressed" << std::endl;
    currentEditorMode = IDLE;
    registry->destroy(selectedObject);
    selectedObject = {};
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
    if (gui->focused) return;
    // Draw the mesh bbox if we hit it
    if (data->cursor->rayCollisionResultInfo.rlCollision.hit && registry->valid(data->cursor->rayCollisionResultInfo.collidedEntityId))
    {
        const auto& col = registry->get<Collideable>(data->cursor->rayCollisionResultInfo.collidedEntityId);
        boundingBoxHighlight = data->cursor->rayCollisionResultInfo.collidedEntityId;
    }
}

void EditorScene::Draw3D()
{
    if (currentEditorMode == SELECT)
    {
        data->collisionSystem->BoundingBoxDraw(selectedObject, ORANGE);
    }

    if (currentEditorMode == IDLE)
    {
        if (boundingBoxHighlight != entt::null)
        {
            data->collisionSystem->BoundingBoxDraw(boundingBoxHighlight);
            boundingBoxHighlight = entt::null;
        }
    }

    Scene::Draw3D();
}


void EditorScene::Draw2D()
{
    std::string mode = "NONE";
    if (currentEditorMode == IDLE) mode = "IDLE";
    else if (currentEditorMode == SELECT) mode = "SELECT";
    else if (currentEditorMode == MOVE) mode = "MOVE";
    else if (currentEditorMode == CREATE) mode = "CREATE";
    
    gui->Draw(mode, data->cursor.get());
    // Do not draw2D the game cursor
}

EditorScene::EditorScene(entt::registry* _registry, std::unique_ptr<GameData> _data) :
    Scene(_registry, std::move(_data)), gui(std::make_unique<editor::GUI>(data->settings, data->userInput.get(), data->camera.get()))
{
    {
        entt::sink onClickEvent{data->userInput->onClickEvent};
        onClickEvent.connect<&EditorScene::OnCursorClick>(this);
    }
    {
        entt::sink onCollisionHitEvent{data->cursor->onCollisionHitEvent};
        onCollisionHitEvent.connect<&EditorScene::OnCollisionHit>(this);
    }
    {
        entt::sink keyDeletePressed{data->userInput->keyDeletePressed};
        keyDeletePressed.connect<&EditorScene::OnDeleteModeKeyPressed>(this);
    }
    {
        entt::sink keyPPressed{data->userInput->keyPPressed};
        keyPPressed.connect<&EditorScene::OnCreateModeKeyPressed>(this);
    }
    {
        entt::sink keyGPressed{data->userInput->keyGPressed};
        keyGPressed.connect<&EditorScene::OnGenGridKeyPressed>(this);
    }
    {
        entt::sink keyMPressed{data->userInput->keyMPressed};
        keyMPressed.connect<&EditorScene::OnSerializeSave>(this);
    }
    {
        entt::sink keyNPressed{data->userInput->keyNPressed};
        keyNPressed.connect<&EditorScene::OnSerializeLoad>(this);
    }
    {
        entt::sink saveButton{gui->saveButtonPressed};
        saveButton.connect<&EditorScene::OnSerializeSave>(this);
    }
    {
        entt::sink loadButton{gui->loadButtonPressed};
        loadButton.connect<&EditorScene::OnSerializeLoad>(this);
    }

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
