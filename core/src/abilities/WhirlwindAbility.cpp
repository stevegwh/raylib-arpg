//
// Created by Steve Wheeler on 21/07/2024.
//

#include "WhirlwindAbility.hpp"

namespace sage
{
    void WhirlwindAbility::Update()
    {
        cooldownTimer -= 1.0f;
    }
    
    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem) : 
    Ability(_registry, _collisionSystem)
    {
        cooldownLimit = 5.0f;
        initialDamage = 10.0f;
    }
}
