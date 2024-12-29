//
// Created by Steve Wheeler on 05/11/2024.
//

#include "PartySystem.hpp"

#include "components/Collideable.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/States.hpp"
#include "ControllableActorSystem.hpp"
#include "GameData.hpp"
#include "TextureTerrainOverlay.hpp" // used for construction

#include <cassert>

namespace sage
{

    void PartySystem::SetLeader(entt::entity leader) const
    {
        // Combine with ControllableActorSystem and put function here
    }

    void PartySystem::NPCToMember(entt::entity npc)
    {
        registry->emplace<PartyMemberComponent>(npc, npc);
        registry->emplace<ControllableActor>(npc, npc);
        registry->emplace<InventoryComponent>(npc);
        registry->emplace<EquipmentComponent>(npc);
        auto& combatable = registry->emplace<CombatableActor>(npc);
        combatable.actorType = CombatableActorType::PLAYER;
        auto& moveable = registry->emplace<MoveableActor>(npc);
        moveable.movementSpeed = 0.35f;
        moveable.pathfindingBounds = 100;
        auto& col = registry->get<Collideable>(npc);
        col.collisionLayer = CollisionLayer::PLAYER;
        AddMember(npc);
    }

    void PartySystem::AddMember(entt::entity member)
    {
        assert(party.size() < PARTY_MEMBER_MAX);
        party.push_back(member);
        groups.at(0).push_back(member);
    }

    void PartySystem::RemoveMember(entt::entity entity)
    {
        for (auto it = party.begin(); it != party.end(); ++it)
        {
            if (*it == entity)
            {
                party.erase(it);
                return;
            }
        }
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
    }

    PartySystem::PartySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        groups.resize(1);

        entt::sink sink{gameData->controllableActorSystem->onSelectedActorChange};
        sink.connect<&PartySystem::SetLeader>(this);
    }
} // namespace sage