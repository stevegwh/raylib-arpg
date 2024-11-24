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
    }
} // namespace sage