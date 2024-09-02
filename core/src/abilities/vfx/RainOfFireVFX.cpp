//
// Created by Steve Wheeler on 26/07/2024.
//

#include "RainOfFireVFX.hpp"

#include "GameData.hpp"

#include "Camera.hpp"
#include "components/sgTransform.hpp"

#include "FlamePartSys.hpp"
#include "ResourceManager.hpp"

#include "raylib.h"
#include "raymath.h"
#include "slib.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace sage
{

    struct Fireball
    {
        Vector3 position;
        Vector3 velocity;
        std::unique_ptr<FlamePartSys> flameEffect;
    };

    void RainOfFireVFX::Draw3D() const
    {
        BeginShaderMode(shader);
        for (const auto& fireball : fireballs)
        {
            fireball.flameEffect->DrawOldestFirst();
        }
        EndShaderMode();
    }

    void RainOfFireVFX::Update(float dt)
    {
        for (auto& fireball : fireballs)
        {
            Vector3 previousPosition = fireball.position;
            fireball.position = Vector3Add(fireball.position, Vector3Scale(fireball.velocity, dt));

            // fireball->flameEffect->SetDirection(fireball->velocity);
            fireball.flameEffect->SetOrigin(fireball.position);

            fireball.flameEffect->Update(dt);
            if (fireball.position.y < minHeight)
            {
                generateFireball(fireball);
            }
        }
    }

    void RainOfFireVFX::generateFireball(Fireball& fireball)
    {
        int maxRadius = 5; // TODO: temporary. Should be the radius of the ability's cursor

        auto right = gameData->camera->GetRight();
        auto aerialSpawn = Vector3MultiplyByValue(right, 3);

        // Calculate a random point in the circle around the target
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        int radius = GetRandomValue(1, maxRadius);
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        Vector3 randomGroundPoint{.x = x, .y = 0, .z = y};
        randomGroundPoint = Vector3Add(target, randomGroundPoint);
        aerialSpawn = Vector3Add(randomGroundPoint, aerialSpawn);
        aerialSpawn.y = initialHeight;

        fireball.position = aerialSpawn;

        // Calculate the velocity vector towards the landing point
        Vector3 direction = Vector3Normalize(Vector3Subtract(randomGroundPoint, aerialSpawn));
        float speed = GetRandomValue(4, 8);
        fireball.velocity = {direction.x * speed, direction.y * speed, direction.z * speed};

        if (!fireball.flameEffect)
        {
            fireball.flameEffect = std::make_unique<FlamePartSys>(gameData->camera->getRaylibCam());
            fireball.flameEffect->SetOrigin(fireball.position);
            fireball.flameEffect->SetDirection(fireball.velocity);
        }
    }

    void RainOfFireVFX::InitSystem()
    {
        active = true;
        const int numFireballs = 20; // Total number of fireballs
        const float height = 15.0f;  // Base height above the target
        initialHeight = height;
        minHeight = 0.0f;
        impactRadius = 1.0f;
        target = transform->GetWorldPos();
        // Base spawn point slightly behind and above the target
        baseSpawnPoint = {target.x + initialOffset, target.y + height, target.z + initialOffset};

        if (!fireballs.empty())
        {
            for (auto& fireball : fireballs)
            {
                generateFireball(fireball);
            }
        }
        else
        {
            for (int i = 0; i < numFireballs; ++i)
            {
                Fireball fireball;
                generateFireball(fireball);
                fireballs.push_back(std::move(fireball));
            }
        }
    }

    RainOfFireVFX::~RainOfFireVFX()
    {
        UnloadShader(shader);
        std::cout << "RainOfFireVFX destroyed" << std::endl;
    }

    RainOfFireVFX::RainOfFireVFX(GameData* _gameData, sgTransform* _transform) : VisualFX(_gameData, _transform)
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/billboard.fs");
    }
} // namespace sage