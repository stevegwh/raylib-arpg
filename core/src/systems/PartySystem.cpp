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

    PartyMemberComponent PartySystem::GetMember(unsigned int memberNumber)
    {
        assert(memberNumber < party.size());
        return registry->get<PartyMemberComponent>(party.at(memberNumber));
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