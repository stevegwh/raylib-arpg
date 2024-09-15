//
// Created by steve on 22/02/2024.
//

#pragma once

#include "scenes/Scene.hpp"

#include "entt/entt.hpp"

#include <memory>
#include <string>
#include <vector>

namespace sage
{
    struct Settings;
    struct EditorSettings;
    class UserInput;

    namespace editor
    {
        class EditorGui;
        class FloatingWindow;
    } // namespace editor

    enum EditorMode
    {
        IDLE,
        SELECT,
        WALK,
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
            KeyMapping* _keyMapping,
            Settings* _settings,
            EditorSettings* _editorSettings);
    };
} // namespace sage
