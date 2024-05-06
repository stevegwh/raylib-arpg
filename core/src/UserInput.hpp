//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once


#include "Cursor.hpp"
#include "KeyMapping.hpp"

namespace sage
{
    class UserInput
    {
        Cursor* cursor;
        KeyMapping keyMapping;
        
    public:
        entt::delegate<void()> dOnClickEvent{};
        entt::delegate<void()> dOnCollisionHitEvent{};
        entt::delegate<void()> dOnDeleteKeyPressedEvent{};
        entt::delegate<void()> dOnCreateKeyPressedEvent{};
        entt::delegate<void()> dOnGenGridKeyPressedEvent{};
        entt::delegate<void()> dOnSerializeSaveKeyPressedEvent{};
        entt::delegate<void()> dOnSerializeLoadKeyPressedEvent{};
        entt::delegate<void()> dOnRunModePressedEvent{};
        
        void ListenForInput();

        explicit UserInput(Cursor* _cursor, KeyMapping _keyMapping);
    };
}
