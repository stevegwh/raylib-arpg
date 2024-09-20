//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "components/Spawner.hpp"

#include "raylib-cereal.hpp"
#include <entt/entt.hpp>
#include <vector>

namespace sage
{

    class SpawnerLoader
    {
        entt::registry* registry;

      public:
        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<Spawner> spawners;
            for (const auto view = registry->view<Spawner>(); auto& entity : view)
            {
                auto& spawner = registry->get<Spawner>(entity);
                spawners.push_back(spawner);
            }
            archive(spawners);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<Spawner> spawners;
            archive(spawners);
            assert(!spawners.empty());
            for (const auto& spawner : spawners)
            {
                const auto entity = registry->create();
                registry->emplace<Spawner>(entity, spawner);
            }
        }

        explicit SpawnerLoader(entt::registry* _registry);
    };

} // namespace sage
