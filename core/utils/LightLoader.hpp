//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "components/Light.hpp"

#include "raylib-cereal.hpp"
#include <entt/entt.hpp>
#include <vector>

namespace sage
{

    class LightLoader
    {
        entt::registry* registry;

      public:
        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<Light> spawners;
            for (const auto view = registry->view<Light>(); auto& entity : view)
            {
                auto& spawner = registry->get<Light>(entity);
                spawners.push_back(spawner);
            }
            archive(spawners);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<Light> spawners;
            archive(spawners);
            assert(!spawners.empty());
            for (const auto& spawner : spawners)
            {
                const auto entity = registry->create();
                registry->emplace<Light>(entity, spawner);
            }
        }

        explicit LightLoader(entt::registry* _registry) : registry(_registry)
        {
        }
    };

} // namespace sage
