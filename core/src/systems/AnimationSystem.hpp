//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "raylib.h"
#include "entt/entt.hpp"

#include "BaseSystem.hpp"
#include "../components/Animation.hpp"

namespace sage
{

class AnimationSystem : public BaseSystem<Animation>
{
public:
    void Update();
    void Draw();
    explicit AnimationSystem(entt::registry* _registry);
};

} // sage
