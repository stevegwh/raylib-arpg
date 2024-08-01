//
// Created by Steve Wheeler on 04/05/2024.
//

#include "EditorApplication.hpp"
#include "../src/scenes/ExampleScene.hpp"
#include "EditorScene.hpp"
#include "Serializer.hpp"

#include <memory>
#include <fstream>

#include "cereal/cereal.hpp"
//#include <cereal/archives/json.hpp>
#include "cereal/archives/xml.hpp"

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
		auto data = std::make_unique<GameData>(registry.get(), keyMapping.get(), settings.get());
		scene = std::make_unique<EditorScene>(registry.get(), std::move(data), editorSettings.get());
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
        auto data = std::make_unique<GameData>(registry.get(), keyMapping.get(), settings.get());
        scene = std::make_unique<ExampleScene>(registry.get(), std::move(data), editorSettings->lastOpenedMap);
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
		//std::stringstream storage;

		std::ofstream storage(EditorApplication::editorSettingsPath.c_str());
		if (!storage.is_open())
		{
			// Handle file opening error
			return;
		}

		{
			// output finishes flushing its contents when it goes out of scope
			cereal::XMLOutputArchive output{ storage };
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
			cereal::XMLInputArchive input{ storage };
			input(settings);
			storage.close();
		}
		else
		{
			// File doesn't exist, create a new file with the default key mapping
			std::cout << "Key mapping file not found. Creating a new file with the default key mapping." << std::endl;
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
            case StateFlag::VOID:break;
            }
			state = StateFlag::VOID;
		}
	}

	void EditorApplication::Update()
	{
		init();
		SetTargetFPS(60);
		while (!WindowShouldClose()) // Detect window close button or ESC key
		{
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
		EndDrawing();
	}

	EditorApplication::EditorApplication()
	{
		EditorSettings _settings;
        DeserializeEditorSettings(_settings);
		editorSettings = std::make_unique<EditorSettings>(_settings);
	}
} // sage
