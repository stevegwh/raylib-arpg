//
// Created by steve on 18/02/2024.
//

#include "Application.hpp"

#include "scenes/ExampleScene.hpp"

#include "Camera.hpp"
#include "GameData.hpp"
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

        scene = std::make_unique<ExampleScene>(
            registry.get(),
            keyMapping.get(),
            settings.get(),
            ""); // TODO: Map path is ignored atm (just loads output.bin)

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
            scene->Update();
            draw();
        }
    }

    void Application::draw()
    {
        BeginDrawing();
        ClearBackground(BLUE);
        BeginMode3D(*scene->data->camera->getRaylibCam());
        scene->Draw3D();
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
    };

    void Application::cleanup()
    {
        CloseWindow();
    }
} // namespace sage
