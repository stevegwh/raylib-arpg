#include "EditorApplication.hpp"

#include "EditorMapLoader.hpp"
#include "EditorScene.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/KeyMapping.hpp"
#include "engine/Serializer.hpp"
#include "engine/Settings.hpp"
#include "engine/UserInput.hpp"

#include "raylib.h"

namespace sage
{
    void EditorApplication::init()
    {
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        const auto screenSize = settings->GetScreenSize();
        InitWindow(static_cast<int>(screenSize.x), static_cast<int>(screenSize.y), "BG Raylib Editor");
        settings->UpdateViewport();
        SetExitKey(KEY_NULL);
        EnableCursor();

        serializer::LoadAssetBinFile(registry.get(), "resources/assets.bin");
        editor::LoadMap(registry.get(), "resources/dungeon-map.bin");
        systems = std::make_unique<EngineSystems>(registry.get(), keyMapping.get(), settings.get(), audioManager.get());
        scene = std::make_unique<EditorScene>(systems.get());
    }

    void EditorApplication::draw() const
    {
        BeginDrawing();
        ClearBackground(Color{232, 235, 238, 255});

        BeginMode3D(*systems->camera->getRaylibCam());
        scene->Draw3D();
        EndMode3D();

        scene->Draw2D();
        DrawFPS(12, 12);

        if (exitWindowRequested)
        {
            const auto viewport = settings->GetViewPort();
            DrawRectangle(0, 100, static_cast<int>(viewport.x), 200, BLACK);
            const auto text = "Are you sure you want to exit program? [Y/N]";
            const auto textSize = MeasureText(text, 30);
            DrawText(text, static_cast<int>((viewport.x - textSize) / 2), 180, 30, WHITE);
        }

        EndDrawing();
    }

    void EditorApplication::handleScreenUpdate() const
    {
        if (!settings->toggleFullScreenRequested) return;

        const auto prev = settings->GetViewPort();

#ifdef __APPLE__
        if (!IsWindowFullscreen())
        {
            const int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            settings->SetScreenSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            ToggleFullscreen();
        }
        else
        {
            ToggleFullscreen();
            settings->ResetToUserDefined();
            const auto screen = settings->GetScreenSize();
            SetWindowSize(static_cast<int>(screen.x), static_cast<int>(screen.y));
        }
#else
        const bool maximized = GetScreenWidth() == GetMonitorWidth(GetCurrentMonitor()) &&
                               GetScreenHeight() == GetMonitorHeight(GetCurrentMonitor());
        if (!maximized)
        {
            const int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            settings->SetScreenSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            ToggleBorderlessWindowed();
        }
        else
        {
            ToggleBorderlessWindowed();
            settings->ResetToUserDefined();
            const auto screen = settings->GetScreenSize();
            SetWindowSize(static_cast<int>(screen.x), static_cast<int>(screen.y));
        }
#endif

        settings->toggleFullScreenRequested = false;
        systems->userInput->onWindowUpdate.Publish(prev, settings->GetViewPort());
    }

    void EditorApplication::Update()
    {
        init();
        SetTargetFPS(60);

        while (!exitWindow)
        {
            if (WindowShouldClose() || IsKeyPressed(KEY_ESCAPE)) exitWindowRequested = true;

            if (exitWindowRequested)
            {
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

    EditorApplication::EditorApplication()
        : registry(std::make_unique<entt::registry>()),
          keyMapping(std::make_unique<KeyMapping>()),
          settings(std::make_unique<Settings>(&exitWindow)),
          audioManager(std::make_unique<AudioManager>())
    {
    }

    EditorApplication::~EditorApplication()
    {
        CloseWindow();
    }
} // namespace sage
