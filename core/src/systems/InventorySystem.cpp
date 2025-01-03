//
// Created by Steve Wheeler on 08/10/2024.
//

#include "InventorySystem.hpp"

#include "Camera.hpp"
#include "components/Collideable.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/QuestComponents.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "ControllableActorSystem.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "ItemFactory.hpp"
#include "NavigationGridSystem.hpp"

namespace sage
{

    void InventorySystem::inventoryUpdated() const
    {
        onInventoryUpdated.publish();
    }

    bool InventorySystem::CheckWorldItemRange()
    {
        const auto cursorPos = gameData->cursor->getMouseHitInfo().rlCollision.point;
        if (AlmostEquals(cursorPos, lastWorldItemHovered.pos))
        {
            return lastWorldItemHovered.reachable;
        }

        lastWorldItemHovered.pos = cursorPos;
        lastWorldItemHovered.reachable = false;

        auto actorId = gameData->controllableActorSystem->GetSelectedActor();
        const auto playerPos = registry->get<sgTransform>(actorId).GetWorldPos();

        const auto dist = Vector3Distance(cursorPos, playerPos);
        if (dist > ItemComponent::MAX_ITEM_DROP_RANGE)
        {
            // TODO: Say to player it's out of range
            std::cout << "Item out of pick up range. \n";
            return false;
        }

        const auto& collideable = registry->get<Collideable>(actorId);
        gameData->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
        if (gameData->navigationGridSystem->AStarPathfind(actorId, playerPos, cursorPos).empty())
        {
            // TODO: Say to player
            std::cout << "Item unreachable \n";
        }
        else
        {
            lastWorldItemHovered.reachable = true;
        }
        gameData->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);

        return lastWorldItemHovered.reachable;
    }

    void InventorySystem::onWorldItemClicked(entt::entity entity)
    {
        if (!CheckWorldItemRange()) return;

        auto& inventoryComponent =
            registry->get<InventoryComponent>(gameData->controllableActorSystem->GetSelectedActor());

        if (inventoryComponent.AddItem(entity))
        {
            if (registry->any_of<Renderable>(entity))
            {
                registry->remove<Renderable>(entity);
            }
            if (registry->any_of<sgTransform>(entity))
            {
                registry->remove<sgTransform>(entity);
            }
            if (registry->any_of<Collideable>(entity))
            {
                registry->remove<Collideable>(entity);
            }
            if (registry->any_of<QuestTaskComponent>(entity))
            {
                auto& questTask = registry->get<QuestTaskComponent>(entity);
                questTask.MarkComplete();
            }
        }
        else
        {
            inventoryComponent.onInventoryFull.publish();
        }
    }

    void InventorySystem::onComponentAdded(entt::entity entity)
    {
        auto& component = registry->get<InventoryComponent>(entity);
        entt::sink sink1{component.onItemAdded};
        entt::sink sink2{component.onItemRemoved};
        sink1.connect<&InventorySystem::inventoryUpdated>(this);
        sink2.connect<&InventorySystem::inventoryUpdated>(this);
    }

    void InventorySystem::onComponentRemoved(entt::entity entity)
    {
        auto& component = registry->get<InventoryComponent>(entity);
        entt::sink sink1{component.onItemAdded};
        entt::sink sink2{component.onItemRemoved};
        sink1.disconnect<&InventorySystem::inventoryUpdated>(this);
        sink2.disconnect<&InventorySystem::inventoryUpdated>(this);
    }

    InventorySystem::InventorySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        entt::sink sink{_gameData->cursor->onItemClick};
        sink.connect<&InventorySystem::onWorldItemClicked>(this);

        registry->on_construct<InventoryComponent>().connect<&InventorySystem::onComponentAdded>(this);
        registry->on_destroy<InventoryComponent>().connect<&InventorySystem::onComponentRemoved>(this);
    }

} // namespace sage