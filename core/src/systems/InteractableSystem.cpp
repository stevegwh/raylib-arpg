//
// Created by Steve Wheeler on 29/12/2024.
//

#include "InteractableSystem.hpp"

#include "components/DialogTriggerOnClickComponent.hpp"
#include "components/DoorBehaviorComponent.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"

namespace sage
{

    void InteractableSystem::onComponentAdded(entt::entity entity)
    {
    }

    void InteractableSystem::onComponentRemoved(entt::entity entity)
    {
    }

    void InteractableSystem::OnInteractableClick(entt::entity entity) const
    {
        if (registry->any_of<DialogTriggerOnClickComponent>(entity))
        {
            auto& trigger = registry->get<DialogTriggerOnClickComponent>(entity);
            trigger.ExecuteBehavior(entity);
        }
        else if (registry->any_of<DoorBehaviorComponent>(entity))
        {
            auto& door = registry->get<DoorBehaviorComponent>(entity);
            door.ExecuteBehavior(entity);
        }
    }

    InteractableSystem::InteractableSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        entt::sink sink{gameData->cursor->onInteractableClick};
        sink.connect<&InteractableSystem::OnInteractableClick>(this);
    }
} // namespace sage