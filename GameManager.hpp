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

// Systems
#include "RenderSystem.hpp"
#include "CollisionSystem.hpp"
#include "TransformSystem.hpp"
#include "WorldSystem.hpp"
#include "NavigationGridSystem.hpp"
#include "ActorMovementSystem.hpp"

// States
#include "Game.hpp"
#include "Editor.hpp" // Friend

#define GM GameManager::GetInstance()

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

    std::unique_ptr<sage::CollisionSystem> collisionSystem;
    std::unique_ptr<sage::RenderSystem> renderSystem;
    std::unique_ptr<sage::TransformSystem> transformSystem;
    std::unique_ptr<sage::WorldSystem> worldSystem;
    std::unique_ptr<sage::NavigationGridSystem> navigationGridSystem;
    std::unique_ptr<sage::ActorMovementSystem> actorMovementSystem;
    std::unique_ptr<sage::Camera> sCamera;

    static GameManager& GetInstance()
    {
        static GameManager instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    GameManager(GameManager const&) = delete;
    void operator=(GameManager const&)  = delete;
    
    void Update();
    void DeserializeMap();
    void SerializeMap() const;
    
    friend class Editor;
};
}

