//
// Created by Steve Wheeler on 03/06/2024.
//

#include "HealthBar.hpp"

namespace sage
{

void HealthBar::Decrement(entt::entity entity, int value)
{
    hp -= value;
    onHealthUpdate.publish(entity);
    if (value <= 0)
    {
        onHealthIsZero.publish(entity);
    }
}

void HealthBar::Increment(entt::entity entity, int value)
{
    hp += value;
    onHealthUpdate.publish(entity);
}

}