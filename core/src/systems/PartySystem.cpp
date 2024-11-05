//
// Created by Steve Wheeler on 05/11/2024.
//

#include "PartySystem.hpp"

namespace sage
{

    void PartySystem::AddPartyMember(PartyMember member)
    {
        party.push_back(member);
    }

    void PartySystem::RemovePartyMember(entt::entity entity)
    {
        // TODO
    }

    void PartySystem::SetLeader(entt::entity entity)
    {
        leader = entity;
        // TODO: publish event? should react to event?
    }

    entt::entity PartySystem::GetLeader() const
    {
        return leader;
    }

    unsigned int PartySystem::GetSize() const
    {
        return party.size();
    }

    PartySystem::PartySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage