//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "engine/raylib-cereal.hpp"
#include "raylib.h"

namespace sage
{
    enum class SpawnerType
    {
        PLAYER,
        ENEMY,
        DIALOG_CUTSCENE,
        NPC
    };

    struct Spawner
    {
        // Can add a name for named/important mobs
        std::string name;
        SpawnerType type;
        Vector3 pos;
        Vector3 rot;
        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(type, name, pos, rot);
        }
    };
} // namespace sage