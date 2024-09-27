//
// Created by Steve Wheeler on 17/09/2024.
//

#pragma once

#include "raylib-cereal.hpp"
#include <entt/entt.hpp>
#include <vector>

namespace sage
{

    /**
     * Convenience class to wrap am entt view into a serializable collection (and back again)
     * @tparam Asset
     */
    template <typename Asset>
    class AssetSerializer
    {
        entt::registry* registry;

      public:
        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<Asset> assets;
            for (const auto view = registry->view<Asset>(); auto& entity : view)
            {
                auto& asset = registry->get<Asset>(entity);
                assets.push_back(asset);
            }
            archive(assets);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<Asset> assets;
            archive(assets);
            assert(!assets.empty());
            for (const auto& asset : assets)
            {
                const auto entity = registry->create();
                registry->emplace<Asset>(entity, asset);
            }
        }

        explicit AssetSerializer(entt::registry* _registry) : registry(_registry)
        {
        }
    };

} // namespace sage
