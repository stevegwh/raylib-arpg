//
// Created by steve on 20/05/2024.
//

#pragma once

#include "../components/Combat.hpp"
#include "BaseSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{

class CombatSystem : public BaseSystem<Combat>
{
public:
    explicit CombatSystem(entt::registry* _registry);
    void Draw2D();
    void Draw3D();
    void Update();

};

} // sage
