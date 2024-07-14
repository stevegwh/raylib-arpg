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
	void EditorApplication::enableEditMode()
	{
		state = EditorState::EDITOR;
	}

	void EditorApplication::enablePlayMode()
	{
		state = EditorState::PLAY;
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

	void EditorApplication::SerializeEditorSettings(EditorSettings* settings, const char* path)
	{
		std::cout << "Save called" << std::endl;
		using namespace entt::literals;
		//std::stringstream storage;

		std::ofstream storage(path);
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


	void EditorApplication::DeserializeEditorSettings(EditorSettings& settings, const char* path)
	{
		std::cout << "Load called" << std::endl;
		using namespace entt::literals;

		std::ifstream storage(path);
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
            SerializeEditorSettings(&settings, path);
		}
		std::cout << "Load finished" << std::endl;
	}

	void EditorApplication::init()
	{
		InitWindow(settings->screenWidth, settings->screenHeight, "Baldur's Raylib");
		SetConfigFlags(FLAG_MSAA_4X_HINT);
		initEditorScene();
	}

	void EditorApplication::manageStates()
	{
		if (state != EditorState::IDLE)
		{
			registry = std::make_unique<entt::registry>();
			switch (state)
			{
			case EditorState::PLAY:SerializeEditorSettings(editorSettings.get(), "resources/editor-settings.xml");
				initGameScene();
				break;
            case EditorState::EDITOR:
				initEditorScene();
				break;
            case EditorState::IDLE:break;
            }
			state = EditorState::IDLE;
		}
	}

	void EditorApplication::Update()
	{
		init();
		SetTargetFPS(60);
		while (!WindowShouldClose()) // Detect window close button or ESC key
		{
			scene->Update();
			draw();
            manageStates();
		}
	}

	void EditorApplication::draw()
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);
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
        DeserializeEditorSettings(_settings, "resources/editor-settings.xml");
		editorSettings = std::make_unique<EditorSettings>(_settings);
	}
} // sage
