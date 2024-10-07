//
// Created by steve on 22/02/2024.
//

#include "EditorScene.hpp"

#include "components/sgTransform.hpp"
#include "EditorGui.hpp"
#include "EditorSettings.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/LightSubSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "windows/FloatingWindow.hpp"

#include "raymath.h"
#include "Settings.hpp"
#include "UserInput.hpp"

// TODO: This shouldn't use "GameData", it should have its own "Data" class that only inits the systems that it
// needs.

namespace sage
{
    void EditorScene::moveSelectedObjectToCursorHit() const
    {
        // TODO: Confirm this works
        // (It does not. Set event to update the bb of the selected object)
        auto& selectedObjectTrans = registry->get<sgTransform>(selectedObject);
        selectedObjectTrans.SetPosition(data->cursor->getFirstCollision().point);
    }

    void EditorScene::OnCursorClick()
    {
        if (gui->focused) return;
        if (data->cursor->getFirstCollision().hit)
        {
            switch (registry->get<Collideable>(data->cursor->getMouseHitInfo().collidedEntityId).collisionLayer)
            {
            case CollisionLayer::DEFAULT:
                break;
            case CollisionLayer::FLOORSIMPLE:
                if (currentEditorMode == CREATE)
                {
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
        // data->Save();
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
        else if (currentEditorMode == WALK)
            mode = "MOVE";
        else if (currentEditorMode == CREATE)
            mode = "CREATE";

        gui->Draw(mode, data->cursor.get());
        // Do not draw2D the game cursor
    }

    void EditorScene::DrawDebug3D()
    {
        DrawGrid(data->navigationGridSystem->slices, data->navigationGridSystem->spacing);
        Scene::DrawDebug3D();
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
        entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, EditorSettings* _editorSettings)
        : Scene(_registry, _keyMapping, _settings),
          editorSettings(_editorSettings),
          gui(std::make_unique<editor::EditorGui>(
              _editorSettings, _settings, data->userInput.get(), data->camera.get()))
    {
        {
            entt::sink onClickEvent{data->cursor->onAnyLeftClick};
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
        lightSubSystem->lights[0] =
            CreateLight(LIGHT_POINT, {0, 25, 0}, Vector3Zero(), WHITE, lightSubSystem->shader);
    }
} // namespace sage
