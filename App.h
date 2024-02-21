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
#include "Scene.hpp"

namespace sage
{
    class App
    {
        std::unique_ptr<sage::Scene> scene;
        std::unique_ptr<sage::Camera> sCamera;
        std::unique_ptr<sage::CollisionSystem> colSystem;
        std::unique_ptr<sage::Cursor> cursor;

        void init();
        void cleanup();
        void draw();

        Ray ray {0};
        std::string hitObjectName{};
    public:
        App()
        {
            init();

            sCamera = std::make_unique<sage::Camera>();
            cursor = std::make_unique<Cursor>();
            scene = std::make_unique<Scene>();
            colSystem = std::make_unique<sage::CollisionSystem>();
            
            sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png") };
            auto towerTransform1 = new Transform(
                (Transform){{ 0.0f, 0.0f, 0.0f }, 1.0f, {0}});
            auto towerTransform2 = new Transform(
                (Transform){{ 10.0f, 0.0f, 20.0f }, 1.0f, {0}});

            auto towerRenderable1 = new Renderable(LoadModel("resources/models/obj/turret.obj"), mat, towerTransform1);
            towerRenderable1->name = "Tower";

            auto towerRenderable2 = new Renderable(LoadModel("resources/models/obj/turret.obj"), mat, towerTransform2);
            towerRenderable2->name = "Tower 2";

            auto go1 = new GameObject(*towerRenderable1, *towerTransform1);
            auto go2 = new GameObject(*towerRenderable2, *towerTransform2);
            
            scene->AddGameObject(go1);
            scene->AddGameObject(go2);
            
            colSystem->AddCollideable(*go1->GetCollideable());
            colSystem->AddCollideable(*go2->GetCollideable());
        }
        
        ~App()
        {
            cleanup();
        }
        
        void Update();
    };
}

