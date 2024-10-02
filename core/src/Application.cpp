//
// Created by steve on 18/02/2024.
//

#include "Application.hpp"

#include "AssetManager.hpp"
#include "scenes/ExampleScene.hpp"

#include "Camera.hpp"
#include "components/Renderable.hpp"
#include "GameData.hpp"
#include "Serializer.hpp"
#include "slib.hpp"

namespace sage
{

    void Application::init()
    {
        Settings _settings;
        serializer::DeserializeSettings(_settings, "resources/settings.xml");
        settings = std::make_unique<Settings>(_settings);

        KeyMapping _keyMapping;
        serializer::DeserializeKeyMapping(_keyMapping, "resources/keybinding.xml");
        keyMapping = std::make_unique<KeyMapping>(_keyMapping);

        // SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(_settings.screenWidth, _settings.screenHeight, "Baldur's Raylib");

        AssetManager::GetInstance().LoadPaths(); // Init asset paths
        serializer::LoadBinFile(registry.get(), "resources/assets.bin");
        serializer::LoadMap(registry.get(), "resources/map2.bin");

        auto icon = ResourceManager::GetInstance().GetImage(AssetID::IMG_APPLICATIONICON);
        SetWindowIcon(icon.GetImage());
        ResourceManager::GetInstance().ImageUnload(AssetID::IMG_APPLICATIONICON);
        HideCursor();
        SetExitKey(KEY_NULL); // Disable KEY_ESCAPE to close window, X-button still works

        scene = std::make_unique<ExampleScene>(registry.get(), keyMapping.get(), settings.get());
    }

    void Application::handleScreenUpdate() const
    {
        if (settings->toggleFullScreenRequested)
        {
#ifdef __APPLE__
            if (!IsWindowFullscreen())
            {
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));

                ToggleFullscreen();
            }
            else
            {
                ToggleFullscreen();
                settings->ResetToUserDefined();
                SetWindowSize(settings->screenWidth, settings->screenHeight);
            }
#else
            bool maximized = GetScreenWidth() == GetMonitorWidth(GetCurrentMonitor()) &&
                             GetScreenHeight() == GetMonitorHeight(GetCurrentMonitor());
            if (!maximized)
            {

                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));

                ToggleBorderlessWindowed();
            }
            else
            {
                ToggleBorderlessWindowed();
                settings->ResetToUserDefined();
                SetWindowSize(settings->screenWidth, settings->screenHeight);
            }
#endif
            settings->toggleFullScreenRequested = false;
        }
    }

    void Application::Update()
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

            scene->Update();

            draw();
            handleScreenUpdate();
        }
    }

    void Application::draw()
    {
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(*scene->data->camera->getRaylibCam());
        scene->Draw3D();

        // scene->DrawDebug();
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

    Application::~Application()
    {
        cleanup();
    }

    Application::Application() : registry(std::make_unique<entt::registry>())
    {
    }
} // namespace sage
