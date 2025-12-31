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
    struct Settings;
    struct KeyMapping;
} // namespace sage

namespace lq
{
    class Scene;
    class Application
    {
        RenderTexture renderTexture{};
        RenderTexture renderTexture2d{};

      protected:
        std::unique_ptr<entt::registry> registry;
        std::unique_ptr<sage::CleanupSystem> cleanupSystem;
        std::unique_ptr<sage::AudioManager> audioManager;
        std::unique_ptr<sage::Settings> settings;
        std::unique_ptr<sage::KeyMapping> keyMapping;
        std::unique_ptr<Scene> scene;
        bool exitWindowRequested = false; // Flag to request window to exit
        bool exitWindow = false;          // Flag to set window to exit

        void handleScreenUpdate();
        virtual void init();
        static void cleanup();
        virtual void draw();

      public:
        void Quit();
        virtual void Update();
        Application();
        virtual ~Application();
        Application(const Application&) = delete;
        void operator=(const Application&) = delete;
    };
} // namespace lq
