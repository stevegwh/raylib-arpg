//
// Created by Steve Wheeler on 08/10/2024.
//

#include "InventorySystem.hpp"

#include "collision/RpgCollisionLayers.hpp"
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
#include "ItemFactory.hpp"
#include "Systems.hpp"
#include "ui/GameUI.hpp"

namespace lq
{

    void InventorySystem::inventoryUpdated() const
    {
        onInventoryUpdated.Publish();
    }

    bool InventorySystem::CheckWorldItemRange(bool hover)
    {
        const auto cursorPos = sys->engine.cursor->getMouseHitInfo().rlCollision.point;
        if (sage::AlmostEquals(cursorPos, lastWorldItemHovered.pos))
        {
            return lastWorldItemHovered.reachable;
        }

        lastWorldItemHovered.pos = cursorPos;
        lastWorldItemHovered.reachable = false;

        auto actorId = sys->selectionSystem->GetSelectedActor();
        const auto playerPos = registry->get<sage::sgTransform>(actorId).GetWorldPos();

        const auto dist = Vector3Distance(cursorPos, playerPos);
        if (dist > ItemComponent::MAX_ITEM_DROP_RANGE)
        {
            if (!hover)
            {
                sys->UI().CreateErrorMessage("Item out of range.");
            }
            return false;
        }

        const auto& collideable = registry->get<sage::Collideable>(actorId);
        sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
        if (sys->engine.navigationGridSystem->AStarPathfind(actorId, playerPos, cursorPos).empty())
        {
            if (!hover)
            {
                sys->UI().CreateErrorMessage("Item unreachable.");
            }
        }
        else
        {
            lastWorldItemHovered.reachable = true;
        }
        sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);

        return lastWorldItemHovered.reachable;
    }

    void InventorySystem::onWorldItemClicked(entt::entity entity)
    {
        if (!CheckWorldItemRange()) return;

        auto& inventoryComponent = registry->get<InventoryComponent>(sys->selectionSystem->GetSelectedActor());

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
        auto& subscriptions = inventorySubscriptions[entity];
        subscriptions.onItemAdded = component.onItemAdded.Subscribe([this]() { inventoryUpdated(); });
        subscriptions.onItemRemoved = component.onItemRemoved.Subscribe([this]() { inventoryUpdated(); });
    }

    void InventorySystem::onComponentRemoved(entt::entity entity)
    {
        const auto it = inventorySubscriptions.find(entity);
        if (it == inventorySubscriptions.end()) return;

        it->second.onItemAdded.UnSubscribe();
        it->second.onItemRemoved.UnSubscribe();
        inventorySubscriptions.erase(it);
    }

    InventorySystem::InventorySystem(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
        _sys->engine.cursor->onLeftClick.Subscribe([this](entt::entity itemId, sage::CollisionLayer layer) {
            if (layer != lq::collision_layers::Item) return;
            onWorldItemClicked(itemId);
        });

        registry->on_construct<InventoryComponent>().connect<&InventorySystem::onComponentAdded>(this);
        registry->on_destroy<InventoryComponent>().connect<&InventorySystem::onComponentRemoved>(this);
    }

} // namespace lq
