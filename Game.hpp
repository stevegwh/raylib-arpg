//
// Created by steve on 18/02/2024.
//

#pragma once

#define EDITOR_MODE

#include "raylib.h"
#include "raymath.h"

#include <memory>
#include <string>

#include "Camera.hpp"
#include "UserInput.hpp"

#include "Registry.hpp"
#include "Entity.hpp"

#include "Collideable.hpp"
#include "WorldObject.hpp"

#include "GameObjectFactory.hpp"

#include "RenderSystem.hpp"
#include "CollisionSystem.hpp"
#include "TransformSystem.hpp"
#include "WorldSystem.hpp"
#include "NavigationGridSystem.hpp"
#include "ActorMovementSystem.hpp"

#ifdef EDITOR_MODE
#include "Editor.hpp"
#endif

namespace sage
{
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;
    class Game
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

        Game() :
        sCamera(std::make_unique<sage::Camera>()),
        renderSystem(std::make_unique<RenderSystem>()),
        collisionSystem(std::make_unique<sage::CollisionSystem>()),
        transformSystem(std::make_unique<sage::TransformSystem>()),
        userInput(std::make_unique<sage::UserInput>())
        {
            EntityID rootNodeId = Registry::GetInstance().CreateEntity();
            auto rootNodeObject = std::make_unique<WorldObject>(rootNodeId);
            worldSystem = std::make_unique<sage::WorldSystem>(rootNodeId);
            worldSystem->AddComponent(std::move(rootNodeObject));
            navigationGridSystem = std::make_unique<sage::NavigationGridSystem>(100, 1.0f);
            
#ifdef EDITOR_MODE
            gameEditor = std::make_unique<sage::Editor>(userInput.get());
#endif
        }

        ~Game()
        {
            cleanup();
        }
        
    public:

        std::unique_ptr<sage::CollisionSystem> collisionSystem;
        std::unique_ptr<sage::RenderSystem> renderSystem;
        std::unique_ptr<sage::TransformSystem> transformSystem;
        std::unique_ptr<sage::WorldSystem> worldSystem;
        std::unique_ptr<sage::NavigationGridSystem> navigationGridSystem;
        std::unique_ptr<sage::ActorMovementSystem> actorMovementSystem;
        std::unique_ptr<sage::Camera> sCamera;

        static Game& GetInstance()
        {
            static Game instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }
        Game(Game const&) = delete;
        void operator=(Game const&)  = delete;
        
        void Update();
        
        friend class Editor;
    };
}

