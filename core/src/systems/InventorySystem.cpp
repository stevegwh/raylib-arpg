//
// Created by Steve Wheeler on 08/10/2024.
//

#include "InventorySystem.hpp"

#include "Camera.hpp"
#include "components/Collideable.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "ControllableActorSystem.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameUiElements.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"

namespace sage
{

    void InventorySystem::inventoryUpdated() const
    {
        onInventoryUpdated.publish();
    }

    bool InventorySystem::checkWorldItemRange() const
    {
        const auto playerPos =
            registry->get<sgTransform>(gameData->controllableActorSystem->GetSelectedActor()).GetWorldPos();
        const auto cursorPos = gameData->cursor->getMouseHitInfo().rlCollision.point;
        const auto dist = Vector3Distance(cursorPos, playerPos);
        if (dist > ItemComponent::MAX_ITEM_DROP_RANGE)
        {
            // TODO: Say to player it's out of range
            std::cout << "Item out of pick up range \n";
            return false;
        }
        return true;
    }

    void InventorySystem::onWorldItemStopHover() const
    {
        if (tooltipWindow)
        {
            tooltipWindow->markForRemoval = true;
        }
    }

    void InventorySystem::onWorldItemHovered(entt::entity entity)
    {
        if (!checkWorldItemRange()) return;
        auto& item = registry->get<ItemComponent>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        tooltipWindow = GameUiFactory::CreateItemTooltip(gameData->uiEngine.get(), item, pos);
    }

    void InventorySystem::onWorldItemClicked(entt::entity entity) const
    {
        if (!checkWorldItemRange()) return;

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
        entt::sink sink2{_gameData->cursor->onItemHover};
        sink2.connect<&InventorySystem::onWorldItemHovered>(this);
        entt::sink sink3{_gameData->cursor->onStopHover};
        sink3.connect<&InventorySystem::onWorldItemStopHover>(this);

        registry->on_construct<InventoryComponent>().connect<&InventorySystem::onComponentAdded>(this);
        registry->on_destroy<InventoryComponent>().connect<&InventorySystem::onComponentRemoved>(this);
    }

} // namespace sage