#pragma once

#include "raylib.h"

#include "entt/entt.hpp"

namespace sage
{
    void AOEAtPoint(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Vector3 point, float radius);

    void HitSingleTarget(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, entt::entity target);

} // namespace sage