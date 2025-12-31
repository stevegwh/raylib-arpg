//
// Created by Steve Wheeler on 05/11/2024.
//

#include "PartySystem.hpp"

#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/States.hpp"
#include "ControllableActorSystem.hpp"
#include "InventorySystem.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"

#include "engine/components/Collideable.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/RenderSystem.hpp"
#include "engine/TextureTerrainOverlay.hpp" // used for construction

#include <cassert>

namespace lq
{

    bool PartySystem::CheckPartyHasItem(entt::entity targetItemId) const
    {
        for (auto entity : party)
        {
            auto& inventory = registry->get<InventoryComponent>(entity);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto itemId = inventory.GetItem(row, col);
                    if (itemId == entt::null) continue;
                    if (itemId == targetItemId)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool PartySystem::CheckPartyHasItem(const std::string& itemName) const
    {
        for (auto entity : party)
        {
            auto& inventory = registry->get<InventoryComponent>(entity);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto itemId = inventory.GetItem(row, col);
                    if (itemId == entt::null) continue;
                    auto& item = registry->get<ItemComponent>(itemId);
                    if (item.name == itemName)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void PartySystem::GiveItemToSelected(const std::string& itemName) const
    {
        auto& inventory = registry->get<InventoryComponent>(sys->cursor->GetSelectedActor());
        auto itemId = sys->itemFactory->GetItem(itemName);
        auto success = inventory.AddItem(itemId);
    }

    // Removes ALL instances of itemName from party's inventory
    void PartySystem::RemoveItemFromParty(const std::string& itemName) const
    {
        for (auto entity : party)
        {
            auto& inventory = registry->get<InventoryComponent>(entity);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto itemId = inventory.GetItem(row, col);
                    if (itemId == entt::null) continue;
                    auto& item = registry->get<ItemComponent>(itemId);
                    if (item.name == itemName)
                    {
                        inventory.RemoveItem(row, col);
                    }
                }
            }
        }
    }

    void PartySystem::RemoveItemFromParty(entt::entity itemId) const
    {
        for (auto entity : party)
        {
            auto& inventory = registry->get<InventoryComponent>(entity);
            inventory.RemoveItem(itemId);
        }
    }

    void PartySystem::SetLeader(entt::entity leader) const
    {
        // Combine with ControllableActorSystem and put function here
    }

    void PartySystem::NPCToMember(entt::entity npc)
    {
        // Taken from ControllableActorSystem (more grounds to merge both)
        auto& moveable = registry->emplace<sage::MoveableActor>(npc);
        moveable.movementSpeed = 0.35f;
        moveable.pathfindingBounds = 100;
        registry->emplace<ControllableActor>(npc);
        registry->emplace<InventoryComponent>(npc);
        registry->emplace<EquipmentComponent>(npc);
        auto& combatable = registry->emplace<CombatableActor>(npc);
        combatable.actorType = CombatableActorType::PLAYER;
        auto& col = registry->get<sage::Collideable>(npc);
        col.collisionLayer = sage::CollisionLayer::PLAYER;
        registry->emplace<PartyMemberComponent>(npc, npc);
        registry->emplace<PartyMemberState>(npc);

        AddMember(npc);
    }

    void PartySystem::AddMember(entt::entity member)
    {
        assert(party.size() < PARTY_MEMBER_MAX);
        party.push_back(member);
        groups.at(0).push_back(member);
        onPartyChange.Publish();
    }

    void PartySystem::RemoveMember(entt::entity entity)
    {
        for (auto it = party.begin(); it != party.end(); ++it)
        {
            if (*it == entity)
            {
                party.erase(it);
                onPartyChange.Publish();
                return;
            }
        }
    }

    const std::vector<entt::entity>& PartySystem::GetAllMembers()
    {
        return party;
    }

    entt::entity PartySystem::GetMember(unsigned int memberNumber) const
    {
        if (memberNumber < party.size())
        {
            return party.at(memberNumber);
        }
        return entt::null;
    }

    unsigned int PartySystem::GetSize() const
    {
        return party.size();
    }

    bool PartySystem::CheckSameGroup(entt::entity a, entt::entity b) const
    {
        for (const auto& group : groups)
        {
            if (std::find(group.begin(), group.end(), a) != std::end(group) &&
                std::find(group.begin(), group.end(), b) != std::end(group))
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::vector<entt::entity> PartySystem::GetGroup(entt::entity entity) const
    {
        for (const auto& group : groups)
        {
            if (std::find(group.begin(), group.end(), entity) != std::end(group))
            {
                return group;
            }
        }
        assert(0);
        return {};
    }

    PartySystem::PartySystem(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
        groups.resize(1);

        sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, const entt::entity actor) { SetLeader(actor); });
    }
} // namespace lq