
//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib-cereal.hpp"
#include <vector>

#pragma once

#include "entt/entt.hpp"
#include "raylib-cereal.hpp"
#include <vector>

namespace sage
{

    /**
     * Convenience class to wrap am entt view into a serializable collection (and back again)
     * @tparam ViewName
     */
    template <typename ViewName>
    class ViewSerializer
    {
        entt::registry* registry;

      public:
        template <class Archive>
        void save(Archive& archive) const
        {

            std::vector<ViewName> components;
            for (const auto view = registry->view<ViewName>(); auto& entity : view)
            {
                auto& asset = registry->get<ViewName>(entity);
                components.push_back(asset);
            }
            archive(components);
        }

        template <class Archive>
        void load(Archive& archive)
        {

            std::vector<ViewName> components;
            archive(components);
            assert(!components.empty());
            for (const auto& asset : components)
            {
                const auto entity = registry->create();
                registry->emplace<ViewName>(entity, asset);
            }
        }

        explicit ViewSerializer(entt::registry* _registry) : registry(_registry)
        {
        }
    };

} // namespace sage
