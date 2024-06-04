//
// Created by Steve Wheeler on 03/06/2024.
//

#pragma once

#include <entt/entt.hpp>

#include "Cursor.hpp"
#include "ActorMovementSystem.hpp"
#include "components/Combatable.hpp"

namespace sage
{

class CombatSystem : public BaseSystem<Combatable>
{
    // Subscribe to collision with cursor on Enemy layer
    // React to click by updating HealthBar and animation
    Cursor* cursor;
    ActorMovementSystem* actorMovementSystem;

    void startCombat(entt::entity entity);
    void onEnemyClick(entt::entity entity);
    void onEnemyDead(entt::entity entity);
    void destroyEnemy(entt::entity entity);
public:
    CombatSystem(entt::registry* _registry, Cursor* _cursor, ActorMovementSystem* _actorMovementSystem);
    void Update();
};

} // sage
