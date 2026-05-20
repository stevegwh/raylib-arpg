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

        template <class Inspector>
        void inspect(Inspector& inspector)
        {
            inspector.String("Name", name);
            inspector.Enum("Type", type);
            inspector.Vec3("Position", pos);
            inspector.Vec3("Rotation", rot);
        }
    };
} // namespace sage