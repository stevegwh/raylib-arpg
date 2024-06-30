//
// Created by steve on 18/02/2024.
//

#include "Application.hpp"
#include "Serializer.hpp"
#include "scenes/ExampleScene.hpp"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

namespace sage
{
	Application::Application() :
		registry(std::make_unique<entt::registry>())
	{
		Settings _settings;
		serializer::DeserializeSettings(_settings, "resources/settings.xml");
		settings = std::make_unique<Settings>(_settings);

		KeyMapping _keyMapping;
		serializer::DeserializeKeyMapping(_keyMapping, "resources/keybinding.xml");
		keyMapping = std::make_unique<KeyMapping>(_keyMapping);
	}

	Application::~Application()
	{
		cleanup();
	}

	void Application::init()
	{
		InitWindow(settings->screenWidth, settings->screenHeight, "Baldur's Raylib");
		SetConfigFlags(FLAG_MSAA_4X_HINT);
		scene = std::make_unique<ExampleScene>(registry.get(),
		                                       std::make_unique<GameData>(
			                                       registry.get(), keyMapping.get(), settings.get()));
		HideCursor();
	}

	void Application::Update()
	{
		init();
		SetTargetFPS(60);
		while (!WindowShouldClose()) // Detect window close button or ESC key
		{
			scene->Update();
			draw();
		}
	}

	void Application::draw()
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode3D(*scene->data->camera->getRaylibCam());
		scene->Draw3D();
		EndMode3D();
		scene->Draw2D();
		EndDrawing();
	};

	void Application::cleanup()
	{
		CloseWindow();
	}
}


/*
// Check ray collision against model meshes
RayCollision meshHitInfo = { 0 };
for (int m = 0; m < tower->model.meshCount; m++)
{
    // NOTE: We consider the model.transform for the collision check but
    // it can be checked against any transform Matrix, used when checking against same
    // model drawn multiple times with multiple transforms
    meshHitInfo = GetRayCollisionMesh(ray, tower->model.meshes[m], tower->model.transform);
    if (meshHitInfo.hit)
    {
        // Save the closest hit mesh
        if ((!collision.hit) || (collision.distance > meshHitInfo.distance)) collision = meshHitInfo;

        break;  // Stop once one mesh collision is detected, the colliding mesh is m
    }
}

if (meshHitInfo.hit)
{
    collision = meshHitInfo;
    cursorColor = ORANGE;
    hitObjectName = "Renderable";
}
 */
