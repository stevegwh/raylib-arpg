//
// Created by steve on 18/02/2024.
//

#pragma once

// Misc
#include "KeyMapping.hpp"
#include "scenes/Scene.hpp"
#include "Camera.hpp"
#include "UserInput.hpp"
#include "GameData.hpp"
// Scenes
#include "scenes/ExampleScene.hpp"

#include "raylib.h"
#include "entt/entt.hpp"
#include "Settings.hpp"

#include <stack>
#include <memory>
#include <unordered_map>

namespace sage
{

class Application
{
protected:
    std::unique_ptr<Settings> settings;
    std::unique_ptr<KeyMapping> keyMapping;
    std::unique_ptr<entt::registry> registry;
    std::unique_ptr<sage::Scene> scene;
    int stateChange = 0;
    virtual void init();
    static void cleanup();
    virtual void draw();
public:
    Application();
    ~Application();
    Application(Application const&) = delete;
    void operator=(Application const&)  = delete;
    std::unique_ptr<sage::GameData> data;
    virtual void Update();
};
}

