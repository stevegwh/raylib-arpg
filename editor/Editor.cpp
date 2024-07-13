//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Editor.hpp"
#include "../src/scenes/ExampleScene.hpp"
#include "EditorScene.hpp"
#include "Serializer.hpp"
#include <memory>

#include <fstream>
#include <type_traits>
#include <vector>

#include "cereal/cereal.hpp"
//#include <cereal/archives/json.hpp>
#include "cereal/archives/xml.hpp"

namespace sage
{
	void Editor::enableEditMode()
	{
		stateChange = 2;
	}

	void Editor::enablePlayMode()
	{
		stateChange = 1;
	}

	void Editor::initEditorScene()
	{
		auto data = std::make_unique<GameData>(registry.get(), keyMapping.get(), settings.get());
		scene = std::make_unique<EditorScene>(registry.get(), std::move(data), "resources/models/obj/level-basic.obj");
		{
			entt::sink keyRPressed{scene->data->userInput->keyRPressed};
			keyRPressed.connect<&Editor::enablePlayMode>(this);
		}
		EnableCursor();
	}

	void Editor::SerializeEditorSettings(EditorSettings& settings, const char* path)
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
			output(settings);
		}
		storage.close();
		std::cout << "Save finished" << std::endl;
	}


	void Editor::DeserializeEditorSettings(EditorSettings& settings, const char* path)
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
			SerializeEditorSettings(settings, path);
		}
		std::cout << "Load finished" << std::endl;
	}

	void Editor::initGameScene()
	{
		auto data = std::make_unique<GameData>(registry.get(), keyMapping.get(), settings.get());
		scene = std::make_unique<ExampleScene>(registry.get(), std::move(data), "resources/models/obj/level-basic.obj");
		{
			entt::sink keyRPressed{scene->data->userInput->keyRPressed};
			keyRPressed.connect<&Editor::enableEditMode>(this);
		}
		HideCursor();
	}

	void Editor::init()
	{
		InitWindow(settings->screenWidth, settings->screenHeight, "Baldur's Raylib");
		SetConfigFlags(FLAG_MSAA_4X_HINT);
		initEditorScene();
	}

	void Editor::manageScenes()
	{
		if (stateChange > 0)
		{
			registry = std::make_unique<entt::registry>();
			switch (stateChange)
			{
			case 1:
				initGameScene();
				break;
			case 2:
				initEditorScene();
				break;
			}
			stateChange = 0;
		}
	}

	void Editor::Update()
	{
		init();
		SetTargetFPS(60);
		while (!WindowShouldClose()) // Detect window close button or ESC key
		{
			scene->Update();
			draw();
			manageScenes();
		}
	}

	void Editor::draw()
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

	Editor::Editor()
	{
		editorSettings = std::make_unique<EditorSettings>();
		EditorSettings _settings;
		DeserializeEditorSettings(_settings, "resources/editor-settings.xml");
		editorSettings = std::make_unique<EditorSettings>(_settings);
	}
} // sage
