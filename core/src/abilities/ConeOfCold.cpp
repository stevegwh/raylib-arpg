//
// Created by Steve Wheeler on 21/07/2024.
//

#include "ConeOfCold.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"
#include "raylib.h"

namespace sage
{
    void ConeOfCold::Execute(entt::entity actor)
    {
    }

    void ConeOfCold::Update(entt::entity actor)
    {
    }

    ConeOfCold::ConeOfCold(entt::registry* _registry, CollisionSystem* _collisionSystem)
        : Ability(_registry, {}, _collisionSystem)
    {
    }
} // namespace sage
