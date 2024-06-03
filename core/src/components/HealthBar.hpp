//
// Created by steve on 20/05/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
struct HealthBar
{
    int hp = 100;
    void Decrement(entt::entity entity, int value);
    void Increment(entt::entity entity, int value);
    entt::sigh<void(entt::entity)> onHealthUpdate{};
    entt::sigh<void(entt::entity)> onHealthIsZero{};
};
}
