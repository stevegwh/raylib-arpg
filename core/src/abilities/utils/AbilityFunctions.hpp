#pragma once

#include "Ability.hpp"
#include "components/CombatableActor.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{

    void Hit360AroundPoint(
        entt::registry* registry,
        entt::entity self,
        AbilityData abilityData,
        Vector3 point,
        float radius);

    void HitSingleTarget(
        entt::registry* registry,
        entt::entity self,
        AbilityData abilityData,
        entt::entity target);
} // namespace sage