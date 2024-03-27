//
// Created by steve on 18/02/2024.
//

#pragma once

//#define EDITOR_MODE

#include "raylib.h"

#include <stack>
#include <memory>
#include <unordered_map>

// Misc
#include "State.hpp"
#include "Camera.hpp"
#include "UserInput.hpp"

// States
#include "Game.hpp"
#include "Editor.hpp" // Friend
#include "ECSManager.hpp"


#define GM GameManager::GetInstance()
#define ECS GameManager::GetInstance().ecs
namespace sage
{
static constexpr int SCREEN_WIDTH = 1280;
static constexpr int SCREEN_HEIGHT = 720;

class GameManager
{
    std::unique_ptr<sage::UserInput> userInput;
    std::vector<Vector3> grid;
    std::stack<std::unique_ptr<sage::State>> states;

    void init();
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
    void Update();
    
    friend class Editor;
};
}

