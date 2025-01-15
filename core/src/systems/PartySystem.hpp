//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Event.hpp"

#include "entt/entt.hpp"

#include <vector>

namespace sage
{
    class Systems;
    struct PartyMemberComponent;

    static constexpr unsigned int PARTY_MEMBER_MAX = 4;

    class PartySystem
    {
        entt::registry* registry;
        Systems* sys;
        std::vector<entt::entity> party;
        std::vector<std::vector<entt::entity>> groups;

      public:
        Event<> onPartyChange;

        [[nodiscard]] bool CheckPartyHasItem(entt::entity targetItemId) const;
        [[nodiscard]] bool CheckPartyHasItem(const std::string& itemName) const;
        void GiveItemToSelected(const std::string& itemName) const;
        void RemoveItemFromParty(const std::string& itemName) const;
        void RemoveItemFromParty(entt::entity itemId) const;
        void SetLeader(entt::entity leader) const;
        void NPCToMember(entt::entity npc);
        void AddMember(entt::entity member);
        void RemoveMember(entt::entity entity);
        const std::vector<entt::entity>& GetAllMembers();
        [[nodiscard]] entt::entity GetMember(unsigned int memberNumber) const;
        [[nodiscard]] unsigned int GetSize() const;
        [[nodiscard]] bool CheckSameGroup(entt::entity a, entt::entity b) const;
        [[nodiscard]] std::vector<entt::entity> GetGroup(entt::entity entity) const;
        PartySystem(entt::registry* _registry, Systems* _sys);
    };
} // namespace sage
