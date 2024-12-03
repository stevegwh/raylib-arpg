//
// Created by Steve Wheeler on 02/12/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

namespace sage
{
    class NPCManager
    {
        entt::registry* registry;
        std::unordered_map<std::string, entt::entity> npcMap;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        entt::entity GetNPC(const std::string& name);

        explicit NPCManager(entt::registry* _registry);
    };

} // namespace sage
