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
    class App
    {

        std::unique_ptr<sage::Camera> sCamera;
        std::unique_ptr<Entity> tower;
        std::unique_ptr<Entity> tower2;
        std::unique_ptr<sage::Cursor> cursor;

        void init();
        void cleanup();
        void draw();

        Ray ray {0};
        std::string hitObjectName{};
    public:
        std::unique_ptr<sage::CollisionSystem> collisionSystem;
        std::unique_ptr<sage::RenderSystem> renderSystem;
        std::unique_ptr<sage::TransformSystem> transformSystem;
        
        App()
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
            
            // init components
            sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png") };
            
            auto towerRenderable1 = new Renderable(0, LoadModel("resources/models/obj/turret.obj"), mat);
            towerRenderable1->name = "Tower";

            auto towerRenderable2 = new Renderable(0, LoadModel("resources/models/obj/turret.obj"), mat);
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
            
            // add components to systems
            renderSystem->AddRenderable(*towerRenderable1);
            renderSystem->AddRenderable(*towerRenderable2);
            
            transformSystem->AddTransform(*towerTransform1);
            transformSystem->AddTransform(*towerTransform2);

            collisionSystem->AddCollideable(*towerCollidable1);
            collisionSystem->AddCollideable(*towerCollidable2);
        }
        
        ~App()
        {
            cleanup();
        }
        
        void Update();
    };
}

