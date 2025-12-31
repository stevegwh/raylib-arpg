//
// Created by Steve Wheeler on 08/10/2024.
//

#include "InventorySystem.hpp"

#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/QuestComponents.hpp"
#include "ControllableActorSystem.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "GameUI.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"

namespace lq
{

    void InventorySystem::inventoryUpdated() const
    {
        onInventoryUpdated.Publish();
    }

    bool InventorySystem::CheckWorldItemRange(bool hover)
    {
        const auto cursorPos = sys->cursor->getMouseHitInfo().rlCollision.point;
        if (sage::AlmostEquals(cursorPos, lastWorldItemHovered.pos))
        {
            return lastWorldItemHovered.reachable;
        }

        lastWorldItemHovered.pos = cursorPos;
        lastWorldItemHovered.reachable = false;

        auto actorId = sys->cursor->GetSelectedActor();
        const auto playerPos = registry->get<sage::sgTransform>(actorId).GetWorldPos();

        const auto dist = Vector3Distance(cursorPos, playerPos);
        if (dist > ItemComponent::MAX_ITEM_DROP_RANGE)
        {
            if (!hover)
            {
                sys->uiEngine->CreateErrorMessage("Item out of range.");
            }
            return false;
        }

        const auto& collideable = registry->get<sage::Collideable>(actorId);
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
        if (sys->navigationGridSystem->AStarPathfind(actorId, playerPos, cursorPos).empty())
        {
            if (!hover)
            {
                sys->uiEngine->CreateErrorMessage("Item unreachable.");
            }
        }
        else
        {
            lastWorldItemHovered.reachable = true;
        }
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);

        return lastWorldItemHovered.reachable;
    }

    void InventorySystem::onWorldItemClicked(entt::entity entity)
    {
        if (!CheckWorldItemRange()) return;

        auto& inventoryComponent = registry->get<InventoryComponent>(sys->cursor->GetSelectedActor());

        if (inventoryComponent.AddItem(entity))
        {
            if (registry->any_of<sage::Renderable>(entity))
            {
                registry->remove<sage::Renderable>(entity);
            }
            if (registry->any_of<sage::sgTransform>(entity))
            {
                registry->remove<sage::sgTransform>(entity);
            }
            if (registry->any_of<sage::Collideable>(entity))
            {
                registry->remove<sage::Collideable>(entity);
            }
            if (registry->any_of<QuestTaskComponent>(entity))
            {
                auto& questTask = registry->get<QuestTaskComponent>(entity);
                questTask.MarkComplete();
            }
        }
        else
        {
            inventoryComponent.onInventoryFull.Publish();
        }
    }

    void InventorySystem::onComponentAdded(entt::entity entity)
    {
        auto& component = registry->get<InventoryComponent>(entity);
        component.onItemAddedSub = component.onItemAdded.Subscribe([this]() { inventoryUpdated(); });
        component.onItemRemovedSub = component.onItemRemoved.Subscribe([this]() { inventoryUpdated(); });
    }

    void InventorySystem::onComponentRemoved(entt::entity entity)
    {
        auto& component = registry->get<InventoryComponent>(entity);
        component.onItemAddedSub.UnSubscribe();
        component.onItemRemovedSub.UnSubscribe();
    }

    InventorySystem::InventorySystem(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {

        _sys->cursor->onItemClick.Subscribe([this](entt::entity itemId) { onWorldItemClicked(itemId); });

        registry->on_construct<InventoryComponent>().connect<&InventorySystem::onComponentAdded>(this);
        registry->on_destroy<InventoryComponent>().connect<&InventorySystem::onComponentRemoved>(this);
    }

} // namespace lq