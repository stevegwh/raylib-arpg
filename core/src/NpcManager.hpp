//
// Created by Steve Wheeler on 02/12/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <string>
#include <unordered_map>

namespace sage
{
    class Systems;

    class NPCManager
    {
        entt::registry* registry;
        Systems* sys;
        std::unordered_map<std::string, entt::entity> npcMap;

      public:
        entt::entity CreateNPC(const std::string& name, Vector3 pos, Vector3 rot);

        explicit NPCManager(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
