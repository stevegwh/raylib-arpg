//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Editor.hpp"
#include "../src/scenes/ExampleScene.hpp"
#include "EditorScene.hpp"
#include "Serializer.hpp"
#include <memory>

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
} // sage
