//
// Created by Steve Wheeler on 05/11/2024.
//

#include "PartySystem.hpp"
#include "components/PartyMemberComponent.hpp"
#include <cassert>

namespace sage
{

    void PartySystem::AddMember(entt::entity member)
    {
        assert(party.size() < PARTY_MEMBER_MAX);
        party.push_back(member);
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

    PartySystem::PartySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage