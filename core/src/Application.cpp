//
// Created by steve on 18/02/2024.
//

#include "Application.hpp"

#include "scenes/ExampleScene.hpp"

#include "Camera.hpp"
#include "Serializer.hpp"

namespace sage
{
    Application::Application() : registry(std::make_unique<entt::registry>())
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
        // SetConfigFlags(FLAG_MSAA_4X_HINT);
        Image icon = LoadImage("resources/icon.png");
        SetWindowIcon(icon);
        HideCursor();
        SetExitKey(KEY_NULL); // Disable KEY_ESCAPE to close window, X-button still works
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
} // namespace sage
