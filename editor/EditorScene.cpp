//
// Created by steve on 22/02/2024.
//

#include "EditorScene.hpp"

#include "components/sgTransform.hpp"
#include "GameObjectFactory.hpp"

namespace sage
{
    void EditorScene::moveSelectedObjectToCursorHit() const
    {
        // TODO: Confirm this works
        // (It does not. Set event to update the bb of the selected object)
        auto& selectedObjectTrans = registry->get<sgTransform>(selectedObject);
        selectedObjectTrans.SetPosition(data->cursor->collision().point, selectedObject);
    }

    void EditorScene::OnCursorClick()
    {
        if (gui->focused) return;
        if (data->cursor->collision().hit)
        {
            switch (
                registry
                    ->get<Collideable>(data->cursor->getMouseHitInfo().collidedEntityId)
                    .collisionLayer)
            {
            case CollisionLayer::DEFAULT:
                break;
            case CollisionLayer::FLOOR:
                if (currentEditorMode == CREATE)
                {
                    GameObjectFactory::createBuilding(
                        registry,
                        data.get(),
                        data->cursor->collision().point,
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
            case CollisionLayer::BUILDING:
                currentEditorMode = SELECT;
                selectedObject = data->cursor->getMouseHitInfo().collidedEntityId;
                break;
            }
        }
        else
        {
            if (currentEditorMode != SELECT)
            {
                selectedObject = {};
            }
        }
    }

    void EditorScene::OnSerializeSave()
    {
        data->Save();
    }

    void EditorScene::OnOpenPressed()
    {
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        {
            // data->Load();
            gui->OpenFileDialog();
        }
    }

    void EditorScene::OnFileOpened()
    {
        sceneChange.publish(); // TODO: Make scene files?
    }

    void EditorScene::OnDeleteModeKeyPressed()
    {
        if (currentEditorMode != SELECT) return;
        currentEditorMode = IDLE;
        registry->destroy(selectedObject);
        selectedObject = {};
    }

    void EditorScene::OnCreateModeKeyPressed()
    {
        if (currentEditorMode == CREATE)
            currentEditorMode = IDLE;
        else
            currentEditorMode = CREATE;
    }

    void EditorScene::OnGenGridKeyPressed()
    {
        // data->navigationGridSystem->PopulateGrid();
    }

    void EditorScene::OnCollisionHit(entt::entity entity)
    {
        if (gui->focused) return;
        // Draw the mesh bbox if we hit it
        boundingBoxHighlight = entity;
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
        if (currentEditorMode == IDLE)
            mode = "VOID";
        else if (currentEditorMode == SELECT)
            mode = "SELECT";
        else if (currentEditorMode == MOVE)
            mode = "MOVE";
        else if (currentEditorMode == CREATE)
            mode = "CREATE";

        gui->Draw(mode, data->cursor.get());
        // Do not draw2D the game cursor
    }

    void EditorScene::DrawDebug()
    {
        DrawGrid(data->navigationGridSystem->slices, data->navigationGridSystem->spacing);
    }

    void EditorScene::Update()
    {
        Scene::Update();
        gui->Update();
    }

    EditorScene::~EditorScene()
    {
    }

    EditorScene::EditorScene(
        entt::registry* _registry,
        std::unique_ptr<GameData> _data,
        EditorSettings* _editorSettings)
        : Scene(_registry, std::move(_data), _editorSettings->lastOpenedMap),
          editorSettings(_editorSettings),
          gui(std::make_unique<editor::EditorGui>(
              _editorSettings, data->settings, data->userInput.get(), data->camera.get()))
    {
        {
            entt::sink onClickEvent{data->cursor->onAnyClick};
            onClickEvent.connect<&EditorScene::OnCursorClick>(this);
        }
        {
            entt::sink onCollisionHitEvent{data->cursor->onCollisionHit};
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
            entt::sink keyOPressed{data->userInput->keyOPressed};
            keyOPressed.connect<&EditorScene::OnOpenPressed>(this);
        }
        {
            entt::sink saveButton{gui->saveButtonPressed};
            saveButton.connect<&EditorScene::OnSerializeSave>(this);
        }
        {
            entt::sink loadButton{gui->onFileOpened};
            loadButton.connect<&EditorScene::OnFileOpened>(this);
        }
        lightSubSystem->lights[0] = CreateLight(
            LIGHT_POINT, {0, 25, 0}, Vector3Zero(), WHITE, lightSubSystem->shader);
        data->controllableActorSystem->Disable();
    }
} // namespace sage
