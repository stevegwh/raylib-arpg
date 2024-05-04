//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "raylib.h"
#include "entt/entt.hpp"
#include <sstream>
#include "cereal/cereal.hpp"
//#include <cereal/archives/json.hpp>
#include "cereal/archives/xml.hpp"

#include <memory>
#include <queue>

namespace sage
{
struct Transform
{
    Vector3 position{};
    std::queue<Vector3> targets{};
    Vector3 direction{};
    Vector3 rotation{};
    float scale = 1.0f;

    template<class Archive>
    void save(Archive & archive) const
    {
        archive(
            CEREAL_NVP(position.x),
            CEREAL_NVP(position.y),
            CEREAL_NVP(position.z),
            CEREAL_NVP(rotation.x),
            CEREAL_NVP(rotation.y),
            CEREAL_NVP(rotation.z),
            CEREAL_NVP(scale));
    }

    template<class Archive>
    void load(Archive & archive)
    {
        archive(position.x,
                position.y,
                position.z,
                rotation.x,
                rotation.y,
                rotation.z,
                scale);
    }

    entt::delegate<void(entt::entity)> dOnPositionUpdate{};
    entt::delegate<void(entt::entity)> dOnStartMovement{};
    entt::delegate<void(entt::entity)> dOnFinishMovement{};
};
}

