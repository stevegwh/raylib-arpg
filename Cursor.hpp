//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "raylib.h"

namespace sage
{
    class Cursor
    {
        Color defaultColor = WHITE;
        Color hoverColor = LIME;
    public:
        void Draw(const RayCollision& collision);
    
    };
}
