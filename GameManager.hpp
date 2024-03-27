//
// Created by steve on 18/02/2024.
//

#pragma once

#define EDITOR_MODE

#include "raylib.h"
#include "raymath.h"

#include <memory>
#include <string>
#include <stack>

// Entities
#include "Registry.hpp"
#include "Entity.hpp"

// Components
#include "Collideable.hpp"
#include "WorldObject.hpp"

// Misc
#include "GameObjectFactory.hpp"
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

// Friends
#ifdef EDITOR_MODE
#include "Editor.hpp"
#endif


#define GM GameManager::GetInstance()

namespace sage
{
static constexpr int SCREEN_WIDTH = 1280;
static constexpr int SCREEN_HEIGHT = 720;

class GameManager
{

    std::unique_ptr<sage::UserInput> userInput;
#ifdef EDITOR_MODE
    std::unique_ptr<sage::Editor> gameEditor;
#endif
    std::vector<Vector3> grid;

    void init();
    static void cleanup();
    void draw();
    void removeTower(EntityID entityId);

    GameManager() :
    sCamera(std::make_unique<sage::Camera>()),
    renderSystem(std::make_unique<RenderSystem>()),
    collisionSystem(std::make_unique<sage::CollisionSystem>()),
    transformSystem(std::make_unique<sage::TransformSystem>()),
    userInput(std::make_unique<sage::UserInput>()),
    navigationGridSystem(std::make_unique<sage::NavigationGridSystem>())
    {
        EntityID rootNodeId = Registry::GetInstance().CreateEntity();
        auto rootNodeObject = std::make_unique<WorldObject>(rootNodeId);
        worldSystem = std::make_unique<sage::WorldSystem>(rootNodeId);
        worldSystem->AddComponent(std::move(rootNodeObject));
        actorMovementSystem = std::make_unique<sage::ActorMovementSystem>(userInput.get());
        
#ifdef EDITOR_MODE
        gameEditor = std::make_unique<sage::Editor>(userInput.get());
#endif
    }

    ~GameManager()
    {
        cleanup();
    }
    
    std::stack<State> states;
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

