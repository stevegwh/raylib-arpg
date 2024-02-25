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
    class UserInput
    {
        Ray ray {0};

        std::string hitObjectName{};
        Color defaultColor = WHITE;
        Color hoverColor = LIME;
        void OnClick() const;
        void OnDeleteKeyPressed() const;
        void OnCreateKeyPressed() const;
    public:
        std::unique_ptr<Event> OnClickEvent;
        std::unique_ptr<Event> OnCollisionHitEvent;
        std::unique_ptr<Event> OnDeleteKeyPressedEvent;
        std::unique_ptr<Event> OnCreateKeyPressedEvent;

        RayCollision collision {0};
        CollisionInfo rayCollisionResultInfo;

        void GetMouseRayCollision();
        void Draw();
        void DrawDebugText() const;
        void ListenForInput();

        UserInput()
        :
        OnClickEvent(std::make_unique<Event>()), OnCollisionHitEvent(std::make_unique<Event>()),
        OnDeleteKeyPressedEvent(std::make_unique<Event>()), OnCreateKeyPressedEvent(std::make_unique<Event>())
        {
        }
    };
}
