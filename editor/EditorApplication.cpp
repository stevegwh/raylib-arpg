#include "EditorApplication.hpp"

#include "EditorMapLoader.hpp"
#include "EditorScene.hpp"

#include <filesystem>
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/KeyMapping.hpp"
#include "engine/Serializer.hpp"
#include "engine/Settings.hpp"
#include "engine/UserInput.hpp"

#include "raylib.h"

#include <algorithm>

namespace sage
{
    namespace
    {
        constexpr float EDITOR_LEFT_DOCK_WIDTH = 340.0f;
        constexpr float EDITOR_RIGHT_DOCK_WIDTH = 440.0f;
        constexpr float EDITOR_ASSET_BROWSER_HEIGHT = 344.0f;
        constexpr float EDITOR_SCENE_VIEW_PADDING = 18.0f;
        constexpr float EDITOR_SCENE_ASPECT = 16.0f / 9.0f;

        Rectangle CalculateSceneViewport(Settings& settings)
        {
            const auto appViewport = settings.GetViewPort();
            const float left = settings.ScaleValueWidth(EDITOR_LEFT_DOCK_WIDTH + EDITOR_SCENE_VIEW_PADDING);
            const float right = settings.ScaleValueWidth(EDITOR_RIGHT_DOCK_WIDTH + EDITOR_SCENE_VIEW_PADDING);
            const float bottom =
                settings.ScaleValueHeight(EDITOR_ASSET_BROWSER_HEIGHT + EDITOR_SCENE_VIEW_PADDING * 2.0f);
            const float top = settings.ScaleValueHeight(EDITOR_SCENE_VIEW_PADDING);

            const float availableWidth = std::max(1.0f, appViewport.x - left - right);
            const float availableHeight = std::max(1.0f, appViewport.y - top - bottom);
            float viewportWidth = availableWidth;
            float viewportHeight = viewportWidth / EDITOR_SCENE_ASPECT;
            if (viewportHeight > availableHeight)
            {
                viewportHeight = availableHeight;
                viewportWidth = viewportHeight * EDITOR_SCENE_ASPECT;
            }

            return {
                left + (availableWidth - viewportWidth) * 0.5f,
                top + (availableHeight - viewportHeight) * 0.5f,
                viewportWidth,
                viewportHeight};
        }

        void ConfigureEditorSceneViewport(Settings& settings)
        {
            const Rectangle viewport = CalculateSceneViewport(settings);
            settings.SetRenderViewport(
                static_cast<int>(viewport.width),
                static_cast<int>(viewport.height),
                {viewport.x, viewport.y});
        }
    } // namespace

    void EditorApplication::init()
    {
        SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
        const auto screenSize = settings->GetScreenSize();
        InitWindow(static_cast<int>(screenSize.x), static_cast<int>(screenSize.y), "BG Raylib Editor");
        settings->UpdateViewport();
        ConfigureEditorSceneViewport(*settings);
        SetExitKey(KEY_NULL);
        EnableCursor();

        serializer::LoadAssetBinFile(registry.get(), "resources/assets.bin");
        constexpr const char* mapPath = "resources/dungeon-map.bin";
        editor::LoadMap(registry.get(), mapPath);
        systems = std::make_unique<EngineSystems>(registry.get(), keyMapping.get(), settings.get(), audioManager.get());
        scene = std::make_unique<EditorScene>(systems.get());
        scene->SetSceneName(std::filesystem::path{mapPath}.stem().string());

        const auto renderViewport = settings->GetRenderViewPort();
        renderTexture = LoadRenderTexture(static_cast<int>(renderViewport.x), static_cast<int>(renderViewport.y));
        const auto appViewport = settings->GetViewPort();
        renderTexture2d = LoadRenderTexture(static_cast<int>(appViewport.x), static_cast<int>(appViewport.y));
    }

    void EditorApplication::draw() const
    {
        BeginTextureMode(renderTexture);
        ClearBackground(BLANK);
        BeginMode3D(*systems->camera->getRaylibCam());
        scene->Draw3D();
        EndMode3D();
        EndTextureMode();

        BeginTextureMode(renderTexture2d);
        ClearBackground(BLANK);
        scene->Draw2D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        const auto appViewport = settings->GetViewPort();
        const auto appViewportOffset = settings->GetViewportOffset();
        const auto renderViewport = settings->GetRenderViewPort();
        const auto renderViewportOffset = settings->GetRenderViewportOffset();

        DrawTextureRec(
            renderTexture.texture,
            {0, 0, renderViewport.x, -renderViewport.y},
            {appViewportOffset.x + renderViewportOffset.x, appViewportOffset.y + renderViewportOffset.y},
            WHITE);

        DrawTextureRec(
            renderTexture2d.texture,
            {0, 0, appViewport.x, -appViewport.y},
            appViewportOffset,
            WHITE);

        DrawFPS(12, 12);

        if (exitWindowRequested)
        {
            DrawRectangle(
                static_cast<int>(appViewportOffset.x),
                static_cast<int>(appViewportOffset.y + 100.0f),
                static_cast<int>(appViewport.x),
                200,
                BLACK);
            const auto text = "Are you sure you want to exit program? [Y/N]";
            const auto textSize = MeasureText(text, 30);
            DrawText(
                text,
                static_cast<int>(appViewportOffset.x + (appViewport.x - textSize) / 2.0f),
                static_cast<int>(appViewportOffset.y + 180.0f),
                30,
                WHITE);
        }

        EndDrawing();
    }

    void EditorApplication::handleScreenUpdate()
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
        refreshViewportLayout(prev);
    }

    void EditorApplication::refreshViewportLayout(const Vector2 previousViewport)
    {
        settings->SetScreenSize(GetScreenWidth(), GetScreenHeight());
        ConfigureEditorSceneViewport(*settings);
        const auto appViewport = settings->GetViewPort();
        systems->userInput->onWindowUpdate.Publish(previousViewport, appViewport);

        UnloadRenderTexture(renderTexture);
        const auto renderViewport = settings->GetRenderViewPort();
        renderTexture = LoadRenderTexture(static_cast<int>(renderViewport.x), static_cast<int>(renderViewport.y));

        UnloadRenderTexture(renderTexture2d);
        renderTexture2d = LoadRenderTexture(static_cast<int>(appViewport.x), static_cast<int>(appViewport.y));
    }

    void EditorApplication::handleWindowResize()
    {
        const auto screen = settings->GetScreenSize();
        if (static_cast<int>(screen.x) == GetScreenWidth() && static_cast<int>(screen.y) == GetScreenHeight())
        {
            return;
        }

        refreshViewportLayout(settings->GetViewPort());
    }

    void EditorApplication::Update()
    {
        init();
        SetTargetFPS(60);

        while (!exitWindow)
        {
            if (WindowShouldClose()) exitWindowRequested = true;
            if (IsKeyPressed(KEY_ESCAPE) && !scene->HandleEscapePressed()) exitWindowRequested = true;

            if (exitWindowRequested)
            {
                if (IsKeyPressed(KEY_Y))
                    exitWindow = true;
                else if (IsKeyPressed(KEY_N))
                    exitWindowRequested = false;
            }

            handleWindowResize();
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
        UnloadRenderTexture(renderTexture);
        UnloadRenderTexture(renderTexture2d);
        CloseWindow();
    }
} // namespace sage
