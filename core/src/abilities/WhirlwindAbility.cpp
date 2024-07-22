//
// Created by Steve Wheeler on 21/07/2024.
//

#include "WhirlwindAbility.hpp"
#include "raylib.h"

namespace sage
{
    void WhirlwindAbility::Use()
    {
        if (cooldownTimer <= 0)
        {
            cooldownTimer = cooldownLimit;
            // Do something
            std::cout << "Whirlwind ability used \n";
            return;
        }
        std::cout << "Waiting for cooldown \n";
    }
    
    void WhirlwindAbility::Update()
    {
        cooldownTimer -= GetFrameTime();
    }
    
    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem) : 
    Ability(_registry, _collisionSystem)
    {
        cooldownLimit = 3.0f;
        initialDamage = 10.0f;
    }
}
