//
// Created by steve on 18/02/2024.
//

#pragma once

// Misc
#include "KeyMapping.hpp"
#include "scenes/Scene.hpp"
#include "Settings.hpp"

#include "entt/entt.hpp"

#include <stack>

namespace sage
{
    class Application
    {
        RenderTexture renderTexture;

      protected:
        std::unique_ptr<Settings> settings;
        std::unique_ptr<KeyMapping> keyMapping;
        std::unique_ptr<entt::registry> registry;
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
