//
// Created by steve on 20/05/2024.
//

#pragma once

#include "../components/HealthBar.hpp"
#include "Camera.hpp"
#include "BaseSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{
class HealthBarSystem : public BaseSystem<HealthBar>
{
    sage::Camera* camera;

    RenderTexture2D healthBarTexture;
    const int healthBarWidth = 200;
    const int healthBarHeight = 20;
    const Color healthBarColor = RED;
    const Color healthBarBgColor = BLACK;
    const Color healthBarBorderColor = MAROON;

    void updateHealthBarTexture();
public:
    HealthBarSystem(entt::registry* _registry, sage::Camera* _camera);
    ~HealthBarSystem();
    void Draw2D();
    void Draw3D();
    void Update();
    void Decrement(entt::entity entity, int value);
    void Increment(entt::entity entity, int value);
};

} // sage
