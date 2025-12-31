//
// Created by Steve Wheeler on 26/07/2024.
//

#include "RainOfFireVFX.hpp"

#include "components/Ability.hpp"
#include "Systems.hpp"

#include "engine/Camera.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/slib.hpp"

#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace lq
{

    void RainOfFireVFX::Draw3D() const
    {

        for (const auto& fireball : fireballs)
        {
            rlDisableBackfaceCulling();
            // TODO: Fireball is not centred
            fireball.fireball->GetModel().Draw(fireball.position, Vector3{0, 1, 0}, 0, Vector3{1, 1, 1}, WHITE);
            rlEnableBackfaceCulling();
            fireball.flameEffect->DrawOldestFirst(shader);
        }
    }

    void RainOfFireVFX::Update(float dt)
    {
        for (auto& fireball : fireballs)
        {
            // fireball.fireball->Update(dt);
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

        auto right = sys->camera->GetRight();
        auto aerialSpawn = sage::Vector3MultiplyByValue(right, 3);

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
        float speed = GetRandomValue(25, 30);
        fireball.velocity = {direction.x * speed, direction.y * speed, direction.z * speed};

        if (!fireball.fireball)
        {
            fireball.fireball = std::make_unique<FireballVFX>(sys, ability);
            fireball.fireball->InitSystem();
        }

        if (!fireball.flameEffect)
        {
            fireball.flameEffect = std::make_unique<FlamePartSys>(sys->camera->getRaylibCam());
            fireball.flameEffect->SetOrigin(fireball.position);
            fireball.flameEffect->SetDirection(direction);
        }
    }

    void RainOfFireVFX::InitSystem()
    {
        active = true;
        const int numFireballs = 1; // Total number of fireballs
        const float height = 15.0f; // Base height above the target
        initialHeight = height;
        minHeight = 0.0f;
        impactRadius = 1.0f;
        auto& transform = sys->registry->get<sage::sgTransform>(ability->self);
        target = transform.GetWorldPos();
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

    RainOfFireVFX::RainOfFireVFX(Systems* _sys, Ability* _ability) : VisualFX(_sys, _ability)
    {
        shader =
            sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/billboard.fs");
    }
} // namespace lq