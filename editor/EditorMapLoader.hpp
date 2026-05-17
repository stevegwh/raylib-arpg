#pragma once

#include "entt/entt.hpp"

namespace sage::editor
{
    // Loads an engine-only subset of a map .bin file: Renderable, sgTransform, Collideable
    // (plus the ResourceManager payload required to resolve renderables). Spawners and Lights
    // are read from the stream and emplaced to keep wire-format compatibility with the game's
    // MapLoader, but no game-specific components (Item/Dialog/Inventory/Door) are attached.
    void LoadMap(entt::registry* destination, const char* path);
} // namespace sage::editor
