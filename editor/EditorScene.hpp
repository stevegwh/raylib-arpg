//
// Created by steve on 22/02/2024.
//

#pragma once

#include "EditorGui.hpp"
#include "EditorSettings.hpp"
#include "scenes/Scene.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

#include "entt/entt.hpp"
#include "windows/FloatingWindow.hpp"

#include <memory>
#include <string>
#include <vector>

namespace sage
{
    enum EditorMode
    {
        IDLE,
        SELECT,
        MOVE,
        CREATE
    };

    class EditorScene : public Scene
    {
        EditorMode currentEditorMode = IDLE;

        entt::entity boundingBoxHighlight;

        bool destroySelected = false;

        std::unique_ptr<editor::EditorGui> gui;
        EditorSettings* editorSettings;

        // Event responses
        void OnCursorClick();
        void OnCollisionHit(entt::entity entity);
        void OnSerializeSave();
        void OnOpenPressed();
        void OnOpenClicked();
        void OnFileOpened();
        void OnDeleteModeKeyPressed();
        void OnCreateModeKeyPressed();
        void OnGenGridKeyPressed();

        entt::entity selectedObject{};
        void moveSelectedObjectToCursorHit() const;

      public:
        void Draw3D() override;
        void Draw2D() override;
        void DrawDebug() override;
        void Update() override;
        ~EditorScene() override;
        EditorScene(
            entt::registry* _registry,
            std::unique_ptr<GameData> _data,
            EditorSettings* _editorSettings);
    };
} // namespace sage
