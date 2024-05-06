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


        entt::delegate<void()> dKeyAPressed{};
        entt::delegate<void()> dKeyBPressed{};
        entt::delegate<void()> dKeyCPressed{};
        entt::delegate<void()> dKeyDPressed{};
        entt::delegate<void()> dKeyEPressed{};
        entt::delegate<void()> dKeyFPressed{};
        entt::delegate<void()> dKeyGPressed{};
        entt::delegate<void()> dKeyHPressed{};
        entt::delegate<void()> dKeyIPressed{};
        entt::delegate<void()> dKeyJPressed{};
        entt::delegate<void()> dKeyKPressed{};
        entt::delegate<void()> dKeyLPressed{};
        entt::delegate<void()> dKeyMPressed{};
        entt::delegate<void()> dKeyNPressed{};
        entt::delegate<void()> dKeyOPressed{};
        entt::delegate<void()> dKeyPPressed{};
        entt::delegate<void()> dKeyQPressed{};
        entt::delegate<void()> dKeyRPressed{};
        entt::delegate<void()> dKeySPressed{};
        entt::delegate<void()> dKeyTPressed{};
        entt::delegate<void()> dKeyUPressed{};
        entt::delegate<void()> dKeyVPressed{};
        entt::delegate<void()> dKeyWPressed{};
        entt::delegate<void()> dKeyXPressed{};
        entt::delegate<void()> dKeyYPressed{};
        entt::delegate<void()> dKeyZPressed{};
        entt::delegate<void()> dKeyEscapePressed{};
        entt::delegate<void()> dKeySpacePressed{};
        entt::delegate<void()> dKeyDeletePressed{};
        
        void ListenForInput();

        explicit UserInput(Cursor* _cursor, KeyMapping _keyMapping);
    };
}
