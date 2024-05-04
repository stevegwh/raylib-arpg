//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once


#include "Cursor.hpp"
#include "entt/entt.hpp"

namespace sage
{
    class UserInput
    {
        entt::registry* registry;
        Cursor* cursor;
        
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
        
        void ListenForInput();

        explicit UserInput(Cursor* _cursor);
    };
}
