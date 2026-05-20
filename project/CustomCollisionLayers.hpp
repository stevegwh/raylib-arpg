#pragma once

// Project-specific collision layers. Edit this file to declare layers your
// project needs beyond the engine's built-in set. The engine concatenates the
// array below into its runtime layer list at compile time — engine code never
// references these by name, so the engine→project dependency direction is
// preserved.
//
// Bits 0–15 are reserved for engine layers (see engine/CollisionLayers.hpp);
// use indices 16+ here.

#include "engine/CollisionLayers.hpp"

namespace sage::custom_collision_layers
{
    inline constexpr CollisionLayer Player = MakeCollisionLayer("Player", 16);
    inline constexpr CollisionLayer Npc = MakeCollisionLayer("NPC", 17);
    inline constexpr CollisionLayer Enemy = MakeCollisionLayer("Enemy", 18);
    inline constexpr CollisionLayer Building = MakeCollisionLayer("Building", 19);
    inline constexpr CollisionLayer Item = MakeCollisionLayer("Item", 20);
    inline constexpr CollisionLayer Interactable = MakeCollisionLayer("Interactable", 21);
    inline constexpr CollisionLayer Chest = MakeCollisionLayer("Chest", 22);
} // namespace sage::custom_collision_layers

namespace sage
{
    inline constexpr std::array CustomCollisionLayers = {
        custom_collision_layers::Player,
        custom_collision_layers::Npc,
        custom_collision_layers::Enemy,
        custom_collision_layers::Building,
        custom_collision_layers::Item,
        custom_collision_layers::Interactable,
        custom_collision_layers::Chest,
    };
} // namespace sage
