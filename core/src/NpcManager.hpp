//
// Created by Steve Wheeler on 02/12/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

namespace sage
{
    class GameData;
    
    class NPCManager
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<std::string, entt::entity> npcMap;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        entt::entity CreateNPC(const std::string& name, Vector3 pos, Vector3 rot);
        entt::entity GetNPC(const std::string& name);

        explicit NPCManager(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
