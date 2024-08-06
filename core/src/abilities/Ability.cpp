#include "Ability.hpp"

#include "raylib.h"

namespace sage
{
    void Ability::Update(entt::entity self)
    {
    }

    void Ability::Draw3D(entt::entity self)
    {
        // Draw the ability in 3D space
    }

    void Ability::Init(entt::entity self)
    {
        Execute(self);
    }

    Ability::Ability(
        entt::registry* _registry,
        float _cooldownDuration,
        CollisionSystem* _collisionSystem)
        : registry(_registry), collisionSystem(_collisionSystem)
    {
        cooldownTimer.SetMaxTime(_cooldownDuration);
    }
} // namespace sage