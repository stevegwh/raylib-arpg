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
        KeyMapping keyMapping;
        void toggleFullScreen() const;
    public:
        entt::delegate<void()> dOnClickEvent{};

        entt::delegate<void()> dKeyAPressed{};
        entt::delegate<void()> dKeyAUp{};
        entt::delegate<void()> dKeyBPressed{};
        entt::delegate<void()> dKeyBUp{};
        entt::delegate<void()> dKeyCPressed{};
        entt::delegate<void()> dKeyCUp{};
        entt::delegate<void()> dKeyDPressed{};
        entt::delegate<void()> dKeyDUp{};
        entt::delegate<void()> dKeyEPressed{};
        entt::delegate<void()> dKeyEUp{};
        entt::delegate<void()> dKeyFPressed{};
        entt::delegate<void()> dKeyFUp{};
        entt::delegate<void()> dKeyGPressed{};
        entt::delegate<void()> dKeyGUp{};
        entt::delegate<void()> dKeyHPressed{};
        entt::delegate<void()> dKeyHUp{};
        entt::delegate<void()> dKeyIPressed{};
        entt::delegate<void()> dKeyIUp{};
        entt::delegate<void()> dKeyJPressed{};
        entt::delegate<void()> dKeyJUp{};
        entt::delegate<void()> dKeyKPressed{};
        entt::delegate<void()> dKeyKUp{};
        entt::delegate<void()> dKeyLPressed{};
        entt::delegate<void()> dKeyLUp{};
        entt::delegate<void()> dKeyMPressed{};
        entt::delegate<void()> dKeyMUp{};
        entt::delegate<void()> dKeyNPressed{};
        entt::delegate<void()> dKeyNUp{};
        entt::delegate<void()> dKeyOPressed{};
        entt::delegate<void()> dKeyOUp{};
        entt::delegate<void()> dKeyPPressed{};
        entt::delegate<void()> dKeyPUp{};
        entt::delegate<void()> dKeyQPressed{};
        entt::delegate<void()> dKeyQUp{};
        entt::delegate<void()> dKeyRPressed{};
        entt::delegate<void()> dKeyRUp{};
        entt::delegate<void()> dKeySPressed{};
        entt::delegate<void()> dKeySUp{};
        entt::delegate<void()> dKeyTPressed{};
        entt::delegate<void()> dKeyTUp{};
        entt::delegate<void()> dKeyUPressed{};
        entt::delegate<void()> dKeyUUp{};
        entt::delegate<void()> dKeyVPressed{};
        entt::delegate<void()> dKeyVUp{};
        entt::delegate<void()> dKeyWPressed{};
        entt::delegate<void()> dKeyWUp{};
        entt::delegate<void()> dKeyXPressed{};
        entt::delegate<void()> dKeyXUp{};
        entt::delegate<void()> dKeyYPressed{};
        entt::delegate<void()> dKeyYUp{};
        entt::delegate<void()> dKeyZPressed{};
        entt::delegate<void()> dKeyZUp{};
        entt::delegate<void()> dKeyEscapePressed{};
        entt::delegate<void()> dKeyEscapeUp{};
        entt::delegate<void()> dKeySpacePressed{};
        entt::delegate<void()> dKeySpaceUp{};
        entt::delegate<void()> dKeyDeletePressed{};
        entt::delegate<void()> dKeyDeleteUp{};
        
        void ListenForInput() const;

        explicit UserInput(KeyMapping _keyMapping);
    };
}
