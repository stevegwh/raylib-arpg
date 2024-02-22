//
// Created by steve on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include "raymath.h"

#include <memory>
#include <string>

#include "Renderable.hpp"
#include "CollisionSystem.hpp"
#include "Cursor.hpp"
#include "Camera.hpp"
#include "RenderSystem.hpp"
#include "Entity.hpp"
#include "TransformSystem.hpp"

namespace sage
{
    class Game
    {

        std::unique_ptr<sage::Camera> sCamera;
        std::unique_ptr<Entity> tower;
        std::unique_ptr<Entity> tower2;
        std::unique_ptr<Entity> floor;
        std::unique_ptr<sage::Cursor> cursor;

        static void init();
        static void cleanup();
        void draw();

        Ray ray {0};
        std::string hitObjectName{};

        Game()
        {
            init();

            sCamera = std::make_unique<sage::Camera>();
            cursor = std::make_unique<Cursor>();

            // init systems
            renderSystem = std::make_unique<RenderSystem>();
            collisionSystem = std::make_unique<sage::CollisionSystem>();
            transformSystem = std::make_unique<sage::TransformSystem>();

            // init entities
            tower = std::make_unique<Entity>();
            tower2 = std::make_unique<Entity>();
            floor = std::make_unique<Entity>();
            
            

            // init components
            sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png") };

            auto towerRenderable1 = new Renderable(tower->entityId, LoadModel("resources/models/obj/turret.obj"), mat);
            towerRenderable1->name = "Tower";

            auto towerRenderable2 = new Renderable(tower2->entityId, LoadModel("resources/models/obj/turret.obj"), mat);
            towerRenderable2->name = "Tower 2";

            auto towerTransform1 = new Transform(tower->entityId);
            towerTransform1->position = { 0.0f, 0.0f, 0.0f };
            towerTransform1->scale = 1.0f;

            auto towerTransform2 = new Transform(tower2->entityId);
            towerTransform2->position = { 10.0f, 0.0f, 20.0f };
            towerTransform2->scale = 1.0f;

            // TODO: Below needs to be moved to the collision system update or something.
            auto towerCollidable1 = new Collideable(tower->entityId);
            towerCollidable1->boundingBox = GetMeshBoundingBox(towerRenderable1->model.meshes[0]);
            towerCollidable1->boundingBox.min = Vector3Add(towerCollidable1->boundingBox.min, towerTransform1->position);
            towerCollidable1->boundingBox.max = Vector3Add(towerCollidable1->boundingBox.max, towerTransform1->position);

            auto towerCollidable2 = new Collideable(tower2->entityId);
            towerCollidable2->boundingBox = GetMeshBoundingBox(towerRenderable2->model.meshes[0]);
            towerCollidable2->boundingBox.min = Vector3Add(towerCollidable2->boundingBox.min, towerTransform2->position);
            towerCollidable2->boundingBox.max = Vector3Add(towerCollidable2->boundingBox.max, towerTransform2->position);
            
            // Ground quad
            Vector3 g0 = (Vector3){ -50.0f, 0.1f, -50.0f };
            Vector3 g2 = (Vector3){  50.0f, 0.1f,  50.0f };
            auto floorCollidable = new Collideable(floor->entityId);
            floorCollidable->boundingBox.min = g0;
            floorCollidable->boundingBox.max = g2;
            floorCollidable->collisionLayer = FLOOR;

            // add components to systems
            transformSystem->AddComponent(*towerTransform1);
            transformSystem->AddComponent(*towerTransform2);
            
            renderSystem->AddComponent(*towerRenderable1);
            renderSystem->AddComponent(*towerRenderable2);
            
            collisionSystem->AddComponent(*towerCollidable1);
            collisionSystem->AddComponent(*towerCollidable2);
            collisionSystem->AddComponent(*floorCollidable);
        }

        ~Game()
        {
            cleanup();
        }
        
    public:
        std::unique_ptr<sage::CollisionSystem> collisionSystem;
        std::unique_ptr<sage::RenderSystem> renderSystem;
        std::unique_ptr<sage::TransformSystem> transformSystem;

        static Game& GetInstance()
        {
            static Game instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }
        Game(Game const&) = delete;
        void operator=(Game const&)  = delete;
        
        void Update();
    };
}

