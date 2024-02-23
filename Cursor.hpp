//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "raylib.h"

#include "Camera.hpp"
#include "Collideable.hpp"
#include "CollisionSystem.hpp"
#include "RenderSystem.hpp"
#include "Event.hpp"

#include <string>

namespace sage
{
    class Cursor
    {
        Ray ray {0};

        std::string hitObjectName{};
        Color defaultColor = WHITE;
        Color hoverColor = LIME;
    public:
        std::unique_ptr<Event> OnClickEvent;
        std::unique_ptr<Event> OnCollisionHitEvent;

        RayCollision collision {0};
        CollisionInfo rayCollisionResultInfo;
        void OnClick(const CollisionSystem& colSystem);
        void GetMouseRayCollision(Camera3D raylibCamera, const CollisionSystem& colSystem, const RenderSystem& renderSystem);
        void Draw(const CollisionSystem& colSystem);
        void DrawDebugText() const;

        Cursor()
        {
            OnClickEvent = std::make_unique<Event>();
            OnCollisionHitEvent = std::make_unique<Event>();
        }
    };
}
