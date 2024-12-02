//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <string>

namespace sage
{

    class DialogFactory
    {
        entt::registry* registry;

      public:
        void GetDialog(const std::string& npcName, entt::entity entity) const;

        explicit DialogFactory(entt::registry* _registry);
    };

} // namespace sage
