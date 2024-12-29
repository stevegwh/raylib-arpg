
//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "cereal/types/tuple.hpp"
#include "raylib-cereal.hpp"
#include <entt/entt.hpp>
#include <tuple>
#include <vector>

namespace sage
{
    /**
     * Convenience class to wrap an entt view into a serializable collection (and back again)
     * Supports serializing/deserializing multiple components per entity
     * @tparam Components Variable number of component types to serialize
     */
    template <typename... Components>
    class ViewSerializer
    {
        entt::registry* registry;

        // Helper struct to store multiple components for an entity
        struct ComponentTuple
        {
            std::tuple<Components...> data;

            template <class Archive>
            void serialize(Archive& archive)
            {
                archive(data);
            }
        };

      public:
        template <class Archive>
        void save(Archive& archive) const
        {
            // TODO: This only works for copiable components
            std::vector<ComponentTuple> components;

            auto view = registry->view<Components...>();

            view.each([&components](Components&... componentRefs) {
                ComponentTuple tuple{std::make_tuple(componentRefs...)};
                components.push_back(tuple);
            });

            archive(components);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<ComponentTuple> components;
            archive(components);

            for (const auto& componentTuple : components)
            {
                const auto entity = registry->create();

                // Helper lambda to unpack tuple and emplace components
                auto emplacer = [this, entity](const auto&... component) {
                    (registry->emplace<Components>(entity, component), ...);
                };

                // Apply the emplacer to the tuple
                std::apply(emplacer, componentTuple.data);
            }
        }

        explicit ViewSerializer(entt::registry* _registry) : registry(_registry)
        {
        }
    };
} // namespace sage

// //
// // Created by Steve Wheeler on 17/09/2024.
// //
//
// #pragma once
//
// #include "raylib-cereal.hpp"
// #include <entt/entt.hpp>
// #include <vector>
//
// namespace sage
// {
//
//     /**
//      * Convenience class to wrap am entt view into a serializable collection (and back again)
//      * @tparam ViewName
//      */
//     template <typename ViewName>
//     class ViewSerializer
//     {
//         entt::registry* registry;
//
//       public:
//         template <class Archive>
//         void save(Archive& archive) const
//         {
//             std::vector<ViewName> components;
//             for (const auto view = registry->view<ViewName>(); auto& entity : view)
//             {
//                 auto& asset = registry->get<ViewName>(entity);
//                 components.push_back(asset);
//             }
//             archive(components);
//         }
//
//         template <class Archive>
//         void load(Archive& archive)
//         {
//             std::vector<ViewName> components;
//             archive(components);
//             assert(!components.empty());
//             for (const auto& asset : components)
//             {
//                 const auto entity = registry->create();
//                 registry->emplace<ViewName>(entity, asset);
//             }
//         }
//
//         explicit ViewSerializer(entt::registry* _registry) : registry(_registry)
//         {
//         }
//     };
//
// } // namespace sage
