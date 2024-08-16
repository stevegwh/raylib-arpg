//
// Created by Steve Wheeler on 26/07/2024.
//

#include "RainOfFireVFX.hpp"

#include "raylib.h"
#include "raymath.h"
#include "ResourceManager.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace sage
{
    void RainOfFireVFX::Draw3D() const
    {
        BeginShaderMode(shader);
        for (const auto& fireball : fireballs)
        {
            // DrawSphere(fireball->position, fireball->radius, RED);
            fireball->flameEffect->DrawOldestFirst();
        }
        EndShaderMode();
    }

    void RainOfFireVFX::Update(float dt)
    {

        for (auto& fireball : fireballs)
        {
            Vector3 previousPosition = fireball->position;
            fireball->position = Vector3Add(fireball->position, Vector3Scale(fireball->velocity, dt));
            // Calculate the direction of the fireball
            Vector3 fireballDirection = Vector3Normalize(Vector3Subtract(fireball->position, previousPosition));
            // Calculate the inverse direction
            Vector3 inverseDirection = Vector3Scale(fireballDirection, -1.0f);
            fireball->flameEffect->SetOrigin(fireball->position);
            fireball->flameEffect->SetDirection(inverseDirection);
            fireball->flameEffect->Update(dt);
            if (fireball->position.y < minHeight)
            {
                generateFireball(*fireball);
            }
        }
    }

    void RainOfFireVFX::generateFireball(Fireball& fireball)
    {

        // Add randomness to the spawn position within a semi-circular area
        float spawnAngle = ((float)rand() / RAND_MAX) * PI;
        float spawnRadius = ((float)rand() / RAND_MAX) * spawnAreaRadius;

        Vector3 spawnPoint = {
            baseSpawnPoint.x + spawnRadius * cos(spawnAngle),
            baseSpawnPoint.y + spawnRadius * sin(spawnAngle), // slight vertical variation
            baseSpawnPoint.z + spawnRadius * sin(spawnAngle)};

        // Randomize angle for initial trajectory within a 360-degree spread
        float angle = 200 * DEG2RAD;
        float radius = ((float)rand() / RAND_MAX) * impactRadius;

        // Calculate the landing point within the circle at the ground level
        Vector3 landingPoint = {
            target.x + radius * cos(angle),
            target.y, // Ground level
            target.z + radius * sin(angle)};

        // Set fireball spawn position
        fireball.position = spawnPoint;

        // Calculate the velocity vector towards the landing point
        Vector3 direction = Vector3Normalize(Vector3Subtract(landingPoint, spawnPoint));
        float speed = 1.0f + ((float)rand() / RAND_MAX) * 2.0f; // Speed of fireballs between 1 and 3
        fireball.velocity = {direction.x * speed, direction.y * speed, direction.z * speed};

        fireball.radius = 0.5f; // Radius of fireballs
        if (!fireball.flameEffect)
        {
            fireball.flameEffect = std::make_unique<FlamePartSys>(camera);
        }
    }

    void RainOfFireVFX::InitSystem(const Vector3& _target)
    {
        active = true;
        const int numFireballs = 20; // Total number of fireballs
        const float height = 3.0f;   // Base height above the target
        initialHeight = height;
        minHeight = 0.0f;
        impactRadius = 1.0f;
        target = _target;
        // Base spawn point slightly behind and above the target
        baseSpawnPoint = {target.x + initialOffset, target.y + height, target.z + initialOffset};

        if (!fireballs.empty())
        {
            for (auto& fireball : fireballs)
            {
                generateFireball(*fireball);
            }
        }
        else
        {
            for (int i = 0; i < numFireballs; ++i)
            {
                Fireball fireball;
                generateFireball(fireball);
                fireballs.push_back(std::make_unique<Fireball>(std::move(fireball)));
            }
        }
    }

    RainOfFireVFX::~RainOfFireVFX()
    {
        UnloadShader(shader);
        std::cout << "RainOfFireVFX destroyed" << std::endl;
    }

    RainOfFireVFX::RainOfFireVFX(Camera3D* _camera) : VisualFX(_camera)
    {
        shader = ResourceManager::ShaderLoad(nullptr, "resources/shaders/glsl330/billboard.fs");
    }
} // namespace sage