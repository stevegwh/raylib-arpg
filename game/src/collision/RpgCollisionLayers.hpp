#pragma once

#include "engine/CollisionLayers.hpp"

// Layer definitions now live in engine/CustomCollisionLayers.hpp (the
// game-developer-edited project settings header). This alias keeps the
// historical `lq::collision_layers::*` spelling working at call sites.
namespace lq
{
    namespace collision_layers = sage::custom_collision_layers;
}

namespace lq::collision_masks
{
    inline constexpr sage::CollisionMask DefaultQuery =
        sage::collision_masks::Navigation | collision_layers::Player | collision_layers::Enemy |
        collision_layers::Npc | collision_layers::Item | collision_layers::Interactable | collision_layers::Chest |
        collision_layers::Building;

    inline constexpr sage::CollisionMask Player = collision_layers::Enemy | collision_layers::Building |
                                                  collision_layers::Interactable | collision_layers::Chest;

    inline constexpr sage::CollisionMask Enemy = collision_layers::Player | collision_layers::Building;

    inline constexpr sage::CollisionMask CursorHover = collision_layers::Npc | collision_layers::Enemy |
                                                       collision_layers::Item | collision_layers::Interactable |
                                                       collision_layers::Chest;

    [[nodiscard]] constexpr sage::CollisionMask ForLayer(const sage::CollisionLayer layer)
    {
        if (layer == collision_layers::Player) return Player;
        if (layer == collision_layers::Enemy) return Enemy;
        return sage::collision_masks::None;
    }
} // namespace lq::collision_masks
