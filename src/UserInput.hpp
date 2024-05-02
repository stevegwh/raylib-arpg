//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Camera.hpp"
#include "Collideable.hpp"
#include "CollisionSystem.hpp"
#include "RenderSystem.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

#include <string>

namespace sage
{
    class UserInput
    {
        Ray ray {0};
        entt::registry* registry;

        std::string hitObjectName{};
        Color defaultColor = WHITE;
        Color hoverColor = LIME;
        void OnClick() const;
        void OnDeleteKeyPressed() const;
        void OnCreateKeyPressed() const;
        void OnGenGridKeyPressed() const;
        void OnSerializeKeyPressed() const;
    public:
        entt::delegate<void()> dOnClickEvent{};
        entt::delegate<void()> dOnCollisionHitEvent{};
        entt::delegate<void()> dOnDeleteKeyPressedEvent{};
        entt::delegate<void()> dOnCreateKeyPressedEvent{};
        entt::delegate<void()> dOnGenGridKeyPressedEvent{};
        entt::delegate<void()> dOnSerializeKeyPressedEvent{};
        entt::delegate<void()> dOnRunModePressedEvent{};

        RayCollision collision {0};
        CollisionInfo rayCollisionResultInfo;

        void GetMouseRayCollision();
        void Draw();
        void DrawDebugText() const;
        void ListenForInput();

        explicit UserInput(entt::registry* _registry);
    };
}
