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
#include "GameUiEngine.hpp"
#include "ItemFactory.hpp"
#include "NavigationGridSystem.hpp"

namespace sage
{

    void InventorySystem::inventoryUpdated() const
    {
        onInventoryUpdated->Publish();
    }

    bool InventorySystem::CheckWorldItemRange(bool hover)
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
            if (!hover)
            {
                gameData->uiEngine->CreateErrorMessage("Item out of range.");
            }
            return false;
        }

        const auto& collideable = registry->get<Collideable>(actorId);
        gameData->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
        if (gameData->navigationGridSystem->AStarPathfind(actorId, playerPos, cursorPos).empty())
        {
            if (!hover)
            {
                gameData->uiEngine->CreateErrorMessage("Item unreachable.");
            }
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
            inventoryComponent.onInventoryFull->Publish();
        }
    }

    void InventorySystem::onComponentAdded(entt::entity entity)
    {
        auto& component = registry->get<InventoryComponent>(entity);
        component.onItemAddedCnx = component.onItemAdded->Subscribe([this]() { inventoryUpdated(); });
        component.onItemRemovedCnx = component.onItemRemoved->Subscribe([this]() { inventoryUpdated(); });
    }

    void InventorySystem::onComponentRemoved(entt::entity entity)
    {
        auto& component = registry->get<InventoryComponent>(entity);
        component.onItemAddedCnx->UnSubscribe();
        component.onItemRemovedCnx->UnSubscribe();
    }

    InventorySystem::InventorySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData), onInventoryUpdated(std::make_unique<Event<>>())
    {

        _gameData->cursor->onItemClick->Subscribe([this](entt::entity itemId) { onWorldItemClicked(itemId); });

        registry->on_construct<InventoryComponent>().connect<&InventorySystem::onComponentAdded>(this);
        registry->on_destroy<InventoryComponent>().connect<&InventorySystem::onComponentRemoved>(this);
    }

} // namespace sage