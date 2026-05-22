#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <memory>

namespace sage
{
    class AudioManager;
    class EngineSystems;
    class KeyMapping;
    struct Settings;
    class EditorScene;

    class EditorApplication
    {
        RenderTexture renderTexture{};
        RenderTexture renderTexture2d{};

        std::unique_ptr<entt::registry> registry;
        std::unique_ptr<KeyMapping> keyMapping;
        std::unique_ptr<Settings> settings;
        std::unique_ptr<AudioManager> audioManager;
        std::unique_ptr<EngineSystems> systems;
        std::unique_ptr<EditorScene> scene;

        bool exitWindowRequested = false;
        bool exitWindow = false;

        void init();
        void draw() const;
        void handleScreenUpdate();
        void handleWindowResize();
        void refreshViewportLayout(Vector2 previousViewport);

      public:
        void Update();
        EditorApplication();
        ~EditorApplication();

        EditorApplication(const EditorApplication&) = delete;
        EditorApplication& operator=(const EditorApplication&) = delete;
    };
} // namespace sage
