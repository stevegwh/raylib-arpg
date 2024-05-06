//
// Created by steve on 18/02/2024.
//

#pragma once

// Misc
#include "KeyMapping.hpp"
#include "scenes/Scene.hpp"
#include "Camera.hpp"
#include "UserInput.hpp"
#include "ECSManager.hpp"
// Scenes
#include "scenes/GameScene.hpp"

#include "raylib.h"
#include "entt/entt.hpp"

#include <stack>
#include <memory>
#include <unordered_map>

namespace sage
{
static constexpr int SCREEN_WIDTH = 1280;
static constexpr int SCREEN_HEIGHT = 720;

class GameManager
{
protected:
    KeyMapping keyMapping;
    entt::registry* registry;
    std::unique_ptr<sage::Scene> scene;
    int stateChange = 0;
    virtual void init();
    static void cleanup();
    virtual void draw();
    void toggleFullScreen();
public:
    GameManager();
    ~GameManager();
    GameManager(GameManager const&) = delete;
    void operator=(GameManager const&)  = delete;
    std::unique_ptr<sage::ECSManager> ecs;
    virtual void Update();
};
}

