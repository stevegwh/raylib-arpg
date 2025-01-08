//
// Created by steve on 18/02/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class AudioManager;
    class CleanupSystem;
    class Scene;
    struct Settings;
    struct KeyMapping;

    class Application
    {
        RenderTexture renderTexture{};
        RenderTexture renderTexture2d{};

      protected:
        std::unique_ptr<entt::registry> registry;
        std::unique_ptr<CleanupSystem> cleanupSystem;
        std::unique_ptr<AudioManager> audioManager;
        std::unique_ptr<Settings> settings;
        std::unique_ptr<KeyMapping> keyMapping;
        std::unique_ptr<Scene> scene;
        bool exitWindowRequested = false; // Flag to request window to exit
        bool exitWindow = false;          // Flag to set window to exit

        void handleScreenUpdate();
        virtual void init();
        static void cleanup();
        virtual void draw();

      public:
        Application();
        virtual ~Application();
        Application(const Application&) = delete;
        void operator=(const Application&) = delete;
        virtual void Update();
    };
} // namespace sage
