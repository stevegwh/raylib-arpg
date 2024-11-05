//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    enum class AssetID;
    class GameData;

    struct PartyMember
    {
        const entt::entity entity;
        entt::entity leader = entt::null;
        AssetID portraitImage{};
        explicit PartyMember(entt::entity _entity) : entity(_entity){};
    };

    class PartySystem
    {
        entt::registry* registry;
        GameData* gameData;
        std::vector<PartyMember> party;
        entt::entity leader{};

      public:
        void AddPartyMember(PartyMember member);
        void RemovePartyMember(entt::entity entity);
        void SetLeader(entt::entity entity);
        [[nodiscard]] entt::entity GetLeader() const;
        [[nodiscard]] unsigned int GetSize() const;
        PartySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage
