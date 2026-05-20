#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

namespace sage
{
    constexpr int MAX_COLLISION_LAYERS = 64;

    struct CollisionLayer
    {
        std::string_view layerName{};
        std::uint64_t bit{};

        constexpr CollisionLayer() = default;
        constexpr CollisionLayer(const std::string_view _layerName, const std::uint64_t _bit)
            : layerName(_layerName), bit(_bit)
        {
        }

        [[nodiscard]] constexpr bool IsValid() const
        {
            return bit != 0;
        }

        // The bit is the layer's identity. The name is metadata sourced from the
        // compile-time table at display time, not persisted on disk.
        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(bit);
        }

        friend constexpr bool operator==(CollisionLayer, CollisionLayer) = default;
    };

    [[nodiscard]] constexpr CollisionLayer MakeCollisionLayer(
        const std::string_view layerName, const std::uint8_t index)
    {
        assert(index < MAX_COLLISION_LAYERS && "Exceeded maximum allowed number of collision layers.");
        return CollisionLayer{layerName, 1ull << index};
    }

    struct CollisionMask
    {
        std::uint64_t bits{};

        constexpr explicit CollisionMask(const std::uint64_t _bits = 0) : bits(_bits)
        {
        }

        [[nodiscard]] constexpr bool Contains(const CollisionLayer layer) const
        {
            return (bits & layer.bit) != 0;
        }

        [[nodiscard]] constexpr bool IsEmpty() const
        {
            return bits == 0;
        }

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(bits);
        }

        friend constexpr bool operator==(CollisionMask, CollisionMask) = default;
    };

    [[nodiscard]] constexpr CollisionMask operator|(const CollisionLayer lhs, const CollisionLayer rhs)
    {
        return CollisionMask{lhs.bit | rhs.bit};
    }

    [[nodiscard]] constexpr CollisionMask operator|(const CollisionMask lhs, const CollisionLayer rhs)
    {
        return CollisionMask{lhs.bits | rhs.bit};
    }

    [[nodiscard]] constexpr CollisionMask operator|(const CollisionLayer lhs, const CollisionMask rhs)
    {
        return CollisionMask{lhs.bit | rhs.bits};
    }

    [[nodiscard]] constexpr CollisionMask operator|(const CollisionMask lhs, const CollisionMask rhs)
    {
        return CollisionMask{lhs.bits | rhs.bits};
    }

    namespace collision_layers
    {
        inline constexpr CollisionLayer Default = MakeCollisionLayer("Default", 0);
        inline constexpr CollisionLayer GeometrySimple = MakeCollisionLayer("GeometrySimple", 1);
        inline constexpr CollisionLayer GeometryComplex = MakeCollisionLayer("GeometryComplex", 2);
        inline constexpr CollisionLayer Background = MakeCollisionLayer("Background", 3);
        inline constexpr CollisionLayer Stairs = MakeCollisionLayer("Stairs", 4);
        inline constexpr CollisionLayer Obstacle = MakeCollisionLayer("Obstacle", 5);
    } // namespace collision_layers

    [[nodiscard]] const std::vector<CollisionLayer>& GetCollisionLayers();

    [[nodiscard]] inline std::string_view GetCollisionLayerName(const std::uint64_t bit)
    {
        for (const auto& layer : GetCollisionLayers())
        {
            if (layer.bit == bit) return layer.layerName;
        }
        return {};
    }

    namespace collision_masks
    {
        inline constexpr CollisionMask None{};
        inline constexpr CollisionMask Navigation =
            collision_layers::GeometrySimple | collision_layers::GeometryComplex | collision_layers::Stairs;
        inline constexpr CollisionMask DefaultQuery = Navigation | collision_layers::Obstacle;
    } // namespace collision_masks

    [[nodiscard]] constexpr bool IsNavigationLayer(const CollisionLayer& layer)
    {
        return collision_masks::Navigation.Contains(layer);
    }

    [[nodiscard]] constexpr bool RequiresMeshCollision(const CollisionLayer layer)
    {
        return layer == collision_layers::GeometryComplex || layer == collision_layers::Stairs;
    }

    [[nodiscard]] constexpr CollisionMask GetDefaultCollisionMask(const CollisionLayer layer)
    {
        if (layer == collision_layers::Default) return collision_masks::DefaultQuery;
        return collision_masks::None;
    }
} // namespace sage

template <>
struct std::hash<sage::CollisionLayer>
{
    std::size_t operator()(const sage::CollisionLayer layer) const noexcept
    {
        return std::hash<std::uint64_t>{}(layer.bit);
    }
};

// Pulls in the project's custom layer array and defines the runtime registry.
// Included last so MakeCollisionLayer is visible to the custom file, and so
// CustomCollisionLayers is visible when seeding the vector. The header is
// resolved against SAGE_PROJECT_INCLUDE_DIR (set by the consuming project),
// falling back to engine/defaults/ for standalone engine builds.
#include "project/CustomCollisionLayers.hpp"

namespace sage
{
    [[nodiscard]] inline const std::vector<CollisionLayer>& GetCollisionLayers()
    {
        static const std::vector<CollisionLayer> layers = []() {
            std::vector<CollisionLayer> v = {
                collision_layers::Default,
                collision_layers::GeometrySimple,
                collision_layers::GeometryComplex,
                collision_layers::Background,
                collision_layers::Stairs,
                collision_layers::Obstacle,
            };
            for (const auto& l : CustomCollisionLayers) v.push_back(l);
            return v;
        }();
        return layers;
    }
} // namespace sage
