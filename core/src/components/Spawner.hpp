//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "raylib-cereal.hpp"
#include "raylib.h"

namespace sage
{
    enum class SpawnerType
    {
        PLAYER,
        ENEMY,
        NPC
    };

    struct Spawner
    {
        // Can add a name for named/important mobs
        SpawnerType spawnerType;
        std::string spawnerName;
        Vector3 pos;
        Vector3 rot;
        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(spawnerType, spawnerName, pos, rot);
        }
    };
} // namespace sage