//
// Created by Steve Wheeler on 03/06/2024.
//

#pragma once

#include <entt/entt.hpp>

#include "Cursor.hpp"
#include "ActorMovementSystem.hpp"

namespace sage
{

class CombatSystem // SubSystem?
{
    // Subscribe to collision with cursor on Enemy layer
    // React to click by updating HealthBar and animation
    entt::registry* registry;
    Cursor* cursor;
    ActorMovementSystem* actorMovementSystem;
    
    entt::entity targetEnemy;

    void startCombat(entt::entity entity);
    void onEnemyClick(entt::entity entity);
    void onEnemyDead(entt::entity entity);
public:
    CombatSystem(entt::registry* _registry, Cursor* _cursor, ActorMovementSystem* _actorMovementSystem);
};

} // sage
