//
// Created by Steve Wheeler on 03/06/2024.
//

#include "HealthBar.hpp"

#include <cmath>

namespace sage
{
    void HealthBar::Decrement(int value)
    {
        damageTaken += value; // TODO: Can move to combatable?
    }

    void HealthBar::Increment(int value)
    {
        damageTaken = 0;
    }

    HealthBar::HealthBar()
    {
        healthBarTexture = LoadRenderTexture(200, 50);
    }

    HealthBar::~HealthBar()
    {
        UnloadRenderTexture(healthBarTexture);
    }
} // namespace sage
