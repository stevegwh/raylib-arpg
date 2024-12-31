//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    class GameData;
    struct PartyMemberComponent;

    static constexpr unsigned int PARTY_MEMBER_MAX = 4;

    class PartySystem
    {
        entt::registry* registry;
        GameData* gameData;
        std::vector<entt::entity> party;
        std::vector<std::vector<entt::entity>> groups;

      public:
        void GiveItemToSelected(const std::string& itemName) const;
        void RemoveItemFromParty(const std::string& itemName) const;
        void RemoveItemFromParty(entt::entity itemId) const;
        void SetLeader(entt::entity leader) const;
        void NPCToMember(entt::entity npc);
        void AddMember(entt::entity member);
        void RemoveMember(entt::entity entity);
        [[nodiscard]] entt::entity GetMember(unsigned int memberNumber) const;
        [[nodiscard]] unsigned int GetSize() const;
        [[nodiscard]] bool CheckSameGroup(entt::entity a, entt::entity b) const;
        [[nodiscard]] std::vector<entt::entity> GetGroup(entt::entity entity) const;
        PartySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage
