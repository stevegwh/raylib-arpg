//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace sage
{
    class Systems;

    class ContextualDialogSystem
    {
        static constexpr float fontSize = 22;
        std::unordered_map<entt::entity, std::vector<std::string>> dialogTextMap;
        entt::registry* registry;
        Systems* sys;

      public:
        void Update() const;
        void Draw2D() const;

        void InitContextualDialogsFromDirectory();
        ContextualDialogSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
