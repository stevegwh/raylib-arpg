//
// Created by steve on 18/02/2024.
//

#pragma once

// Misc
#include "scenes/Scene.hpp"
#include "Camera.hpp"
#include "UserInput.hpp"
#include "ECSManager.hpp"
// Scenes
#include "scenes/Game.hpp"

#include "raylib.h"
#include "entt/entt.hpp"

#include <stack>
#include <memory>
#include <unordered_map>

//#define GM GameManager::GetInstance()
//#define ECS GameManager::GetInstance().ecs
namespace sage
{
static constexpr int SCREEN_WIDTH = 1280;
static constexpr int SCREEN_HEIGHT = 720;

class GameManager
{
protected:
    entt::registry* registry;
    std::unique_ptr<sage::Scene> scene;
    int stateChange = 0;

    virtual void init();
    static void cleanup();
    void draw();

    GameManager();
    ~GameManager();
public:
    static GameManager& GetInstance()
    {
        static GameManager instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    GameManager(GameManager const&) = delete;
    void operator=(GameManager const&)  = delete;
    
    std::unique_ptr<sage::ECSManager> ecs;
    std::unique_ptr<sage::Camera> sCamera;
    virtual void Update();
    void SetState(int stateId);
};
}

