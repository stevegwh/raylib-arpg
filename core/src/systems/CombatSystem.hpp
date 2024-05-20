//
// Created by steve on 20/05/2024.
//

#pragma once

#include "../components/Combat.hpp"
#include "Camera.hpp"
#include "BaseSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{
class CombatSystem : public BaseSystem<Combat>
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
    CombatSystem(entt::registry* _registry, sage::Camera* _camera);
    ~CombatSystem();
    void Draw2D();
    void Draw3D();
    void Update();
};

} // sage
