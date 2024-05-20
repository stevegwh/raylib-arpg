//
// Created by steve on 20/05/2024.
//

#include "CombatSystem.hpp"
#include "../components/Transform.hpp"

#include "raylib.h"

namespace sage
{
void CombatSystem::Draw2D()
{

}

void CombatSystem::Draw3D()
{
    const auto& view = registry->view<Combat, sage::Transform>();
    view.each([](const auto& c, const auto& t)
    {
        DrawText("Enemy: 100 / 100", (int)t.position.x - MeasureText("Enemy: 100/100", 20)/2, (int)t.position.y, 20, BLACK);
    });
}

void CombatSystem::Update()
{
//    const auto& view = registry->view<Combat>();
//    view.each([](const auto& c)
//              {
//              });

}

CombatSystem::CombatSystem(entt::registry* _registry) : BaseSystem<Combat>(_registry)
{

}
} // sage