//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

    struct EquipmentComponent
    {
        entt::entity helm = entt::null;
        entt::entity torso = entt::null;
        entt::entity legs = entt::null;
        entt::entity arms = entt::null;
        entt::entity boots = entt::null;
        entt::entity belt = entt::null;
        entt::entity leftHand = entt::null;
        entt::entity rightHand = entt::null;
    };

} // namespace sage
