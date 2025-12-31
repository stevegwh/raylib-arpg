//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace lq
{
    class Systems;

    class ContextualDialogSystem
    {
        static constexpr float fontSize = 22;
        std::unordered_map<entt::entity, std::vector<std::pair<std::string, std::function<bool()>>>> dialogTextMap;
        entt::registry* registry;
        Systems* sys;

      public:
        void Update() const;
        void Draw2D() const;

        void InitContextualDialogsFromDirectory();
        ContextualDialogSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace lq
