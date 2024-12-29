//
// Created by Steve Wheeler on 29/12/2024.
//

#include "DialogTriggerOnClickComponent.hpp"

#include "Cursor.hpp"
#include "DialogComponent.hpp"
#include "GameData.hpp"
#include "systems/DialogSystem.hpp"

namespace sage
{

    void DialogTriggerOnClickComponent::ExecuteBehavior(entt::entity clicked) const
    {
        if (clicked != self) return;
        // Trigger dialog here
        // gameData->dialogSystem->StartConversation({}, );
    }

    DialogTriggerOnClickComponent::~DialogTriggerOnClickComponent()
    {
        connection.release();
    }

    DialogTriggerOnClickComponent::DialogTriggerOnClickComponent(entt::entity _self, Cursor* _cursor) : self(_self)
    {
        entt::sink sink{_cursor->onInteractableClick};
        connection = sink.connect<&DialogTriggerOnClickComponent::ExecuteBehavior>(this);
    }
} // namespace sage