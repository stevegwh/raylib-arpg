//
// Created by steve on 18/02/2024.
//

#pragma once
#include "Camera.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Mesh.hpp"
#include "CollisionSystem.hpp"
#include <memory>
#include <string>

namespace sage
{
    class App
    {
        std::unique_ptr<sage::Camera> sCamera;
        std::unique_ptr<sage::Mesh> towerMesh;
        std::unique_ptr<sage::CollisionSystem> colSystem;

        void init();
        void cleanup();
        void draw();

        Ray ray {0};
        Color cursorColor{};
        RayCollision collision{};
        RayCollision boxHitInfo{};
        std::string hitObjectName{};
    public:
        App()
        {
            init();

            sCamera = std::make_unique<sage::Camera>();

            sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png") };
            towerMesh = std::make_unique<sage::Mesh>(LoadModel("resources/models/obj/turret.obj"), mat);
            towerMesh->SetPosition({ 0.0f, 0.0f, 0.0f });
            towerMesh->SetScale(1.0f);

            colSystem = std::make_unique<sage::CollisionSystem>();
            colSystem->AddCollideable(towerMesh->boundingBox);
        }
        ~App()
        {
            cleanup();
        }
        void Update();
    };
}

