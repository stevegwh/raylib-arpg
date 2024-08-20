//
// Created by Steve Wheeler on 04/05/2024.
//

#include "EditorApplication.hpp"
#include "EditorScene.hpp"
#include "scenes/ExampleScene.hpp"
#include "Serializer.hpp"

// TODO: To move GameData in the scene requires all of this. Should just make it here?
#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "KeyMapping.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

#include <fstream>
#include <memory>

#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"

namespace sage
{
    std::string EditorApplication::editorSettingsPath = "resources/editor-settings.xml";

    void EditorApplication::enableEditMode()
    {
        state = StateFlag::EDITOR;
    }

    void EditorApplication::enablePlayMode()
    {
        state = StateFlag::PLAY;
    }

    void EditorApplication::initEditorScene()
    {
        // "resources/models/obj/level-basic.obj"
        scene =
            std::make_unique<EditorScene>(registry.get(), keyMapping.get(), settings.get(), editorSettings.get());
        {
            entt::sink keyRPressed{scene->data->userInput->keyRPressed};
            keyRPressed.connect<&EditorApplication::enablePlayMode>(this);
        }
        {
            entt::sink sink{scene->sceneChange};
            sink.connect<&EditorApplication::enableEditMode>(this);
        }
        EnableCursor();
    }

    void EditorApplication::initGameScene()
    {
        scene = std::make_unique<ExampleScene>(
            registry.get(), keyMapping.get(), settings.get(), editorSettings->lastOpenedMap);
        {
            entt::sink keyRPressed{scene->data->userInput->keyRPressed};
            keyRPressed.connect<&EditorApplication::enableEditMode>(this);
        }
        HideCursor();
    }

    void EditorApplication::SerializeEditorSettings(EditorSettings* settings)
    {
        std::cout << "Save called" << std::endl;
        using namespace entt::literals;
        // std::stringstream storage;

        std::ofstream storage(EditorApplication::editorSettingsPath.c_str());
        if (!storage.is_open())
        {
            // Handle file opening error
            return;
        }

        {
            // output finishes flushing its contents when it goes out of scope
            cereal::XMLOutputArchive output{storage};
            output(*settings);
        }
        storage.close();
        std::cout << "Save finished" << std::endl;
    }

    void EditorApplication::DeserializeEditorSettings(EditorSettings& settings)
    {
        std::cout << "Load called" << std::endl;
        using namespace entt::literals;

        std::ifstream storage(EditorApplication::editorSettingsPath.c_str());
        if (storage.is_open())
        {
            cereal::XMLInputArchive input{storage};
            input(settings);
            storage.close();
        }
        else
        {
            // File doesn't exist, create a new file with the default key mapping
            std::cout << "Key mapping file not found. Creating a new file with the default key mapping."
                      << std::endl;
            SerializeEditorSettings(&settings);
        }
        std::cout << "Load finished" << std::endl;
    }

    void EditorApplication::manageEditorState()
    {
        if (state != StateFlag::VOID)
        {
            registry = std::make_unique<entt::registry>();
            switch (state)
            {
            case StateFlag::PLAY:
                SerializeEditorSettings(editorSettings.get());
                initGameScene();
                break;
            case StateFlag::EDITOR:
                initEditorScene();
                break;
            case StateFlag::VOID:
                break;
            }
            state = StateFlag::VOID;
        }
    }

    void EditorApplication::Update()
    {
        init();
        SetTargetFPS(60);
        while (!exitWindow) // Detect window close button or ESC key
        {

            if (WindowShouldClose() || IsKeyPressed(KEY_ESCAPE)) exitWindowRequested = true;

            if (exitWindowRequested)
            {
                // A request for close window has been issued, we can save data before closing
                // or just show a message asking for confirmation

                if (IsKeyPressed(KEY_Y))
                    exitWindow = true;
                else if (IsKeyPressed(KEY_N))
                    exitWindowRequested = false;
            }

            manageEditorState();
            scene->Update();
            draw();
        }
    }

    void EditorApplication::draw()
    {
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(*scene->data->camera->getRaylibCam());
        scene->Draw3D();
        scene->DrawDebug();
        EndMode3D();
        scene->Draw2D();
        DrawFPS(10, 10);

        if (exitWindowRequested)
        {
            DrawRectangle(0, 100, settings->screenWidth, 200, BLACK);
            auto textSize = MeasureText("Are you sure you want to exit program? [Y/N]", 30);
            DrawText(
                "Are you sure you want to exit program? [Y/N]",
                (settings->screenWidth - textSize) / 2,
                180,
                30,
                WHITE);
        }

        EndDrawing();
    }

    EditorApplication::EditorApplication()
    {
        EditorSettings _settings;
        DeserializeEditorSettings(_settings);
        editorSettings = std::make_unique<EditorSettings>(_settings);
    }
} // namespace sage
